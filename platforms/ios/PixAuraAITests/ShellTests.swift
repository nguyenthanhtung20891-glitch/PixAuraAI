import XCTest
import Combine
@testable import PixAuraCore
import CPixAuraCore
import CoreGraphics
import ImageIO
import Foundation
import Darwin
@testable import PixAuraAI

@MainActor private final class ModelLifetimeProbe {
    weak var value: ManualEditorModel?
    init(_ value: ManualEditorModel) { self.value = value }
}


// Device-only facts are observations, never extra admission allowances.
private func phase11ResourceReading() -> [String: Any] {
    var reading = mach_task_basic_info_data_t()
    var words = mach_msg_type_number_t(MemoryLayout.size(ofValue: reading) / MemoryLayout<integer_t>.size)
    let status = withUnsafeMutablePointer(to: &reading) { pointer in
        pointer.withMemoryRebound(to: integer_t.self, capacity: Int(words)) {
            task_info(mach_task_self_, task_flavor_t(MACH_TASK_BASIC_INFO), $0, &words)
        }
    }
    return ["kernel_status": status, "resident_bytes": reading.resident_size,
            "process_high_water_resident_bytes": reading.resident_size_max,
            "thermal_state": ProcessInfo.processInfo.thermalState.rawValue,
            "low_power_mode": ProcessInfo.processInfo.isLowPowerModeEnabled]
}
private func phase11Check(_ code: Int32, _ boundary: String) throws {
    guard code == 0 else { throw ManualEditorFailure(status: code, boundary: boundary) }
}
private func phase11Milliseconds<T>(_ work: () throws -> T) throws -> (T, Double) {
    let start = DispatchTime.now().uptimeNanoseconds
    let result = try work()
    return (result, Double(DispatchTime.now().uptimeNanoseconds - start) / 1_000_000)
}
private final class Phase11DeviceInput {
    let file: FileHandle
    init(_ url: URL) throws { file = try FileHandle(forReadingFrom: url) }
    deinit { try? file.close() }
}
private func phase11DeviceRead(_ raw: UnsafeMutableRawPointer?, _ output: UnsafeMutablePointer<UInt8>?, _ capacity: UInt64) -> Int64 {
    guard let raw, let output, capacity > 0, capacity <= 65536 else { return -1 }
    do {
        let data = try Unmanaged<Phase11DeviceInput>.fromOpaque(raw).takeUnretainedValue().file.read(upToCount: Int(capacity)) ?? Data()
        data.withUnsafeBytes { if let base = $0.baseAddress { UnsafeMutableRawPointer(output).copyMemory(from: base, byteCount: data.count) } }
        return Int64(data.count)
    } catch { return -1 }
}
private final class Phase11DeviceContext {
    let pointer = UnsafeMutablePointer<pixaura_editor_context>.allocate(capacity: 1)
    private var opened = false
    init() { pointer.initialize(to: pixaura_editor_context()) }
    deinit {
        _ = close()
        // Retain storage rather than free an owner if quiescence ever fails.
        if !opened { pointer.deinitialize(count: 1); pointer.deallocate() }
    }
    func close() -> [String: Any] {
        guard opened else { return ["stop_status": Int32(0), "destroy_status": Int32(0)] }
        let stop = pixaura_editor_stop(pointer), destroy = pixaura_editor_destroy(pointer)
        if destroy == 0 { opened = false }
        return ["stop_status": stop, "destroy_status": destroy]
    }
    func open(root: URL, input: URL, capability: AppleCapabilitySnapshot) throws -> pixaura_execution_strategy {
        let canonical: String = try root.withUnsafeFileSystemRepresentation { path in
            let resolved = try XCTUnwrap(path.flatMap { realpath($0, nil) })
            defer { free(resolved) }; return String(cString: resolved)
        }
        let path = Array(canonical.utf8), identity = Array(UUID().uuidString.replacingOccurrences(of: "-", with: "").lowercased().utf8)
        let reader = try Phase11DeviceInput(input)
        let count = UInt64(try input.resourceValues(forKeys: [.fileSizeKey]).fileSize ?? 0)
        var profile = capability.profile, observation = capability.observation, strategy = pixaura_execution_strategy()
        let code = path.withUnsafeBufferPointer { path in identity.withUnsafeBufferPointer { identity in
            pixaura_editor_open_policy(1, pointer, path.baseAddress, UInt64(path.count), identity.baseAddress, UInt64(identity.count),
                phase11DeviceRead, Unmanaged.passUnretained(reader).toOpaque(), count, &profile, &observation, &strategy)
        } }
        withExtendedLifetime(reader) {}
        try phase11Check(code, "Benchmark import"); opened = true; return strategy
    }
    func state() throws -> ManualEditorState {
        var size: UInt64 = 0
        try phase11Check(pixaura_editor_state(pointer, nil, 0, &size), "Benchmark state size")
        guard size > 0, size <= 1048576 else { throw ManualEditorFailure(status: 14, boundary: "Benchmark state bounds") }
        var data = Data(count: Int(size))
        try phase11Check(data.withUnsafeMutableBytes { pixaura_editor_state(pointer, $0.bindMemory(to: UInt8.self).baseAddress, size, &size) }, "Benchmark state")
        return try JSONDecoder().decode(ManualEditorState.self, from: data)
    }
}
private func phase11SyntheticPNG(_ url: URL, width: Int, height: Int) throws {
    var pixels = Data(count: width * height * 4)
    pixels.withUnsafeMutableBytes { (buffer: UnsafeMutableRawBufferPointer) in
        for y in 0..<height { for x in 0..<width {
            let at = (y * width + x) * 4
            buffer[at] = UInt8((x * 13 + y * 7) & 255)
            buffer[at + 1] = UInt8((x * 3 + y * 11) & 255)
            buffer[at + 2] = UInt8((x + y * 5) & 255); buffer[at + 3] = 255
        } }
    }
    let space = try XCTUnwrap(CGColorSpace(name: CGColorSpace.sRGB))
    let provider = try XCTUnwrap(CGDataProvider(data: pixels as CFData))
    let image = try XCTUnwrap(CGImage(width: width, height: height, bitsPerComponent: 8, bitsPerPixel: 32, bytesPerRow: width * 4,
        space: space, bitmapInfo: CGBitmapInfo(rawValue: CGImageAlphaInfo.last.rawValue), provider: provider,
        decode: nil, shouldInterpolate: false, intent: .defaultIntent))
    let destination = try XCTUnwrap(CGImageDestinationCreateWithURL(url as CFURL, "public.png" as CFString, 1, nil))
    CGImageDestinationAddImage(destination, image, nil)
    guard CGImageDestinationFinalize(destination) else { throw ManualEditorFailure(status: 12, boundary: "Benchmark fixture") }
}

final class ShellTests: XCTestCase {
    func testNativeDiagnosticPresentationPreservesRawStatusAndLocalizedMeaning() throws {
        let bundle = try XCTUnwrap(Bundle.main.path(forResource: "vi", ofType: "lproj").flatMap(Bundle.init(path:)))
        let failure = ManualEditorFailure(status: 7, boundary: "Preview")
        let diagnostic = try XCTUnwrap(failure.errorDescription)
        XCTAssertEqual(diagnostic, "Preview failure: Invalid parameter (core 7)")
        XCTAssertEqual(presented(diagnostic, bundle: bundle), "Lỗi bản xem trước: Tham số không hợp lệ (core 7)")
        XCTAssertEqual(presented("Unknown diagnostic (core 999)", bundle: bundle), "Unknown diagnostic (core 999)")
        XCTAssertEqual(presented("Invalid parameter (core forged)", bundle: bundle), "Tham số không hợp lệ (core forged)")
        XCTAssertEqual(presented("Preview failure: Preview failure: Invalid parameter (core 7)", bundle: bundle), "Lỗi bản xem trước: Preview failure: Invalid parameter (core 7)")
    }

    @MainActor private func waitFor<T>(_ publisher: Published<T>.Publisher, _ predicate: @escaping (T) -> Bool, file: StaticString = #filePath, line: UInt = #line) async {
        let ready = expectation(description: "authoritative interaction state at \(file):\(line)")
        let subscription = publisher.first(where: predicate).sink { _ in ready.fulfill() }
        await fulfillment(of: [ready], timeout: 20)
        withExtendedLifetime(subscription) {}
    }
    func testShareCacheRejectsPathNamesBeforeRemovingCurrentExport() throws {
        let token = UUID().uuidString
        let safe = try ExportDestination.cacheFile(name: "phase11-\(token).png", bytes: Data([1, 2, 3]))
        let outside = safe.deletingLastPathComponent().deletingLastPathComponent().appendingPathComponent("phase11-escape-\(token).png")
        defer { try? FileManager.default.removeItem(at: safe); try? FileManager.default.removeItem(at: outside) }
        let invalid = ["../phase11-escape-\(token).png", "", ".", "..", "/absolute.png", "..\\escape.png", "bad\0.png", "bad\n.png", String(repeating: "a", count: 256)]
        for name in invalid {
            XCTAssertThrowsError(try ExportDestination.cacheFile(name: name, bytes: Data([9])))
            XCTAssertTrue(FileManager.default.fileExists(atPath: safe.path), "invalid name must not remove current export")
            XCTAssertFalse(FileManager.default.fileExists(atPath: outside.path), "cache name must not escape its directory")
        }
    }
    @MainActor func testFailedImportsRemoveOnlyUnpublishedRootsAndKeepManualHistory() async throws {
        let fixture = try XCTUnwrap(Bundle(for: ShellTests.self).url(forResource: "fixtures", withExtension: "json"))
        let entries = try XCTUnwrap(JSONSerialization.jsonObject(with: Data(contentsOf: fixture)) as? [String: [String: Any]])
        let input = FileManager.default.temporaryDirectory.appendingPathComponent("phase11-\(UUID()).png")
        let invalid = FileManager.default.temporaryDirectory.appendingPathComponent("phase11-invalid-\(UUID()).png")
        try Data(try XCTUnwrap(entries["png"]?["bytes"] as? [UInt8])).write(to: input)
        try Data([0, 1, 2, 3]).write(to: invalid)
        defer { try? FileManager.default.removeItem(at: input); try? FileManager.default.removeItem(at: invalid) }
        let capability = AppleCapabilityOwner(signals: AppTestAppleSignals()); defer { capability.close() }
        let model = ManualEditorModel(capabilities: capability)
        model.importImage(input); await waitFor(model.$core) { $0?.revisionCount == 1 }
        model.select(4); await waitFor(model.$core) { $0?.active == true }
        model.update([100]); model.action(1); await waitFor(model.$core) { $0?.revisionCount == 2 }
        let revision = try XCTUnwrap(model.core?.revisionID)
        let support = try FileManager.default.url(for: .applicationSupportDirectory, in: .userDomainMask, appropriateFor: nil, create: false)
        let projects = support.appendingPathComponent("manual-projects", isDirectory: true)
        let before = Set(try FileManager.default.contentsOfDirectory(atPath: projects.path))
        for _ in 0..<3 {
            model.importImage(invalid); await waitFor(model.$error) { $0 != nil }
            XCTAssertEqual(Set(try FileManager.default.contentsOfDirectory(atPath: projects.path)), before)
            XCTAssertEqual(model.core?.revisionID, revision); XCTAssertEqual(model.core?.revisionCount, 2)
            XCTAssertEqual(try Data(contentsOf: invalid), Data([0, 1, 2, 3]))
        }
        model.select(4); await waitFor(model.$core) { $0?.active == true }
        model.update([200]); model.action(1); await waitFor(model.$core) { $0?.revisionCount == 3 }
        model.close(); await waitFor(model.$core) { $0 == nil }
    }
    @MainActor func testExportFileAndShareDoNotMutateHistory() async throws {
        let file = try XCTUnwrap(Bundle(for: ShellTests.self).url(forResource: "fixtures", withExtension: "json"))
        let fixtures = try XCTUnwrap(JSONSerialization.jsonObject(with: Data(contentsOf: file)) as? [String: [String: Any]])
        let input = FileManager.default.temporaryDirectory.appendingPathComponent("export-\(UUID()).png")
        try Data(try XCTUnwrap(fixtures["png"]?["bytes"] as? [UInt8])).write(to: input)
        defer { try? FileManager.default.removeItem(at: input) }
        let capability = AppleCapabilityOwner(signals: AppTestAppleSignals()); defer { capability.close() }
        let model = ManualEditorModel(capabilities: capability)
        model.importImage(input); await waitFor(model.$core) { $0?.revisionCount == 1 }
        let revision = try XCTUnwrap(model.core?.revisionID)
        model.shareExport(format: 1, quality: 0)
        XCTAssertEqual(model.error, "JPEG quality must be from 1 through 100")
        XCTAssertFalse(model.exporting)
        model.shareExport(format: 2, quality: 0)
        model.shareExport(format: 2, quality: 0)
        await waitFor(model.$exportStatus) { $0?.contains("not confirmed") == true }
        XCTAssertEqual(model.core?.revisionCount, 1)
        XCTAssertEqual(model.core?.revisionID, revision)
        let shared = try XCTUnwrap(model.shareURL)
        XCTAssertEqual(try Data(contentsOf: shared).prefix(4), Data([0x89, 0x50, 0x4E, 0x47]))
        model.finishShare(completed: true)
        XCTAssertEqual(model.exportStatus, "Share sheet closed. Delivery to another app is not confirmed.")
        XCTAssertFalse(FileManager.default.fileExists(atPath: shared.path))
        model.finishShare(completed: false)
        XCTAssertEqual(model.exportStatus, "Share sheet closed. Delivery to another app is not confirmed.")
        model.prepareFileExport(format: 1, quality: 90)
        await waitFor(model.$exportFile) { $0?.starts(with: Data([0xFF, 0xD8])) == true }
        XCTAssertEqual(model.core?.revisionID, revision)
        model.finishFileExport(.failure(NSError(domain: NSCocoaErrorDomain, code: NSUserCancelledError)))
        XCTAssertEqual(model.exportStatus, "Save cancelled")
        XCTAssertFalse(ExportDestination.libraryAccess(.denied))
        XCTAssertFalse(ExportDestination.libraryAccess(.restricted))
        XCTAssertTrue(ExportDestination.libraryAccess(.authorized))
        let parent = FileManager.default.temporaryDirectory.appendingPathComponent("export-parent-\(UUID())", isDirectory: true)
        try FileManager.default.createDirectory(at: parent, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: parent) }
        let destination = parent.appendingPathComponent("blocked", isDirectory: true)
        try FileManager.default.createDirectory(at: destination, withIntermediateDirectories: true)
        XCTAssertThrowsError(try ExportDestination.writeAtomically(destination, Data([1, 2, 3])))
        let leftovers = try FileManager.default.contentsOfDirectory(at: parent, includingPropertiesForKeys: nil)
        XCTAssertTrue(leftovers.allSatisfy { !$0.lastPathComponent.contains(".partial") })
        model.close(); await waitFor(model.$core) { $0 == nil }
    }
    @MainActor func testInteractionModeMatrixCloseReopenAndOrdinaryPublishedHistory() async throws {
        let file = try XCTUnwrap(Bundle(for: ShellTests.self).url(forResource: "fixtures", withExtension: "json"))
        let fixtures = try XCTUnwrap(JSONSerialization.jsonObject(with: Data(contentsOf: file)) as? [String: [String: Any]])
        let input = FileManager.default.temporaryDirectory.appendingPathComponent("lifecycle-\(UUID()).png")
        try Data(try XCTUnwrap(fixtures["png"]?["bytes"] as? [UInt8])).write(to: input)
        defer { try? FileManager.default.removeItem(at: input) }
        let capability = AppleCapabilityOwner(signals: AppTestAppleSignals()); defer { capability.close() }
        let model = ManualEditorModel(capabilities: capability)
        model.importImage(input); await waitFor(model.$core) { $0?.revisionCount == 1 }
        let source = try XCTUnwrap(model.core?.revisionID)
        let edges = [(0,1),(1,0),(0,2),(2,0),(1,2),(2,1)]
        func mode(_ value: Int) {
            if value == 1 { model.setAssistMode(true) }
            else if value == 2 { model.setAutoMode(true) }
            else { model.setAutoMode(false); model.setAssistMode(false) }
        }
        for (from,to) in edges {
            mode(from); mode(to)
            XCTAssertEqual(model.core?.revisionID, source)
            XCTAssertNil(model.autoProposal.review); XCTAssertNil(model.assistance.advice)
        }
        mode(2); await waitFor(model.$autoProposal) { $0.choices.count == 11 }
        model.chooseAuto(0); model.requestAuto(); await waitFor(model.$autoProposal) { $0.status == .reviewReady && !$0.inFlight }
        model.close(); await waitFor(model.$core) { $0 == nil }
        XCTAssertNil(model.autoProposal.review); XCTAssertFalse(model.autoProposal.enabled)
        model.reopen(); await waitFor(model.$core) { $0?.revisionID == source }
        mode(2); model.chooseAuto(0); model.requestAuto()
        await waitFor(model.$autoProposal) { $0.status == .reviewReady && !$0.inFlight }
        let review = try XCTUnwrap(model.autoProposal.review)
        model.applyAutoFromHuman(review); model.applyAutoFromHuman(review)
        await waitFor(model.$autoProposal) { $0.status == .completed && !$0.inFlight }
        await waitFor(model.$core) { $0?.revisionCount == 2 }
        let published = try XCTUnwrap(model.core?.revisionID)
        model.close(); await waitFor(model.$core) { $0 == nil }
        model.reopen(); await waitFor(model.$core) { $0?.revisionID == published }
        XCTAssertNil(model.autoProposal.receipt); XCTAssertNil(model.autoProposal.review)
        mode(0); model.action(2); await waitFor(model.$core) { $0?.revisionID == source }
        model.action(3); await waitFor(model.$core) { $0?.revisionID == published }
        model.close(); await waitFor(model.$core) { $0 == nil }
    }

    @MainActor func testInterruptDropsAutoAuthorityAndManualGestureWithoutRevision() async throws {
        let file = try XCTUnwrap(Bundle(for: ShellTests.self).url(forResource: "fixtures", withExtension: "json"))
        let fixtures = try XCTUnwrap(JSONSerialization.jsonObject(with: Data(contentsOf: file)) as? [String: [String: Any]])
        let input = FileManager.default.temporaryDirectory.appendingPathComponent("interrupt-\(UUID()).png")
        try Data(try XCTUnwrap(fixtures["png"]?["bytes"] as? [UInt8])).write(to: input)
        defer { try? FileManager.default.removeItem(at: input) }
        let capability = AppleCapabilityOwner(signals: AppTestAppleSignals()); defer { capability.close() }
        let model = ManualEditorModel(capabilities: capability)
        model.importImage(input); await waitFor(model.$core) { $0?.revisionCount == 1 }
        let source = try XCTUnwrap(model.core?.revisionID)
        model.setAutoMode(true)
        await waitFor(model.$autoProposal) { $0.choices.count == 11 }
        model.chooseAuto(0)
        model.requestAuto()
        model.interrupt()
        await waitFor(model.$busy) { !$0 }
        await waitFor(model.$autoProposal) { !$0.inFlight }
        XCTAssertNil(model.autoProposal.proposal)
        XCTAssertNil(model.autoProposal.review)
        XCTAssertEqual(model.core?.revisionID, source)
        model.requestAuto()
        await waitFor(model.$autoProposal) { $0.status == .reviewReady && $0.review != nil && !$0.inFlight }
        model.interrupt()
        XCTAssertNil(model.autoProposal.proposal)
        XCTAssertNil(model.autoProposal.review)
        XCTAssertEqual(model.core?.revisionCount, 1)
        // A new proposal is admitted only after queued interruption retires.
        await waitFor(model.$busy) { !$0 }
        model.requestAuto()
        await waitFor(model.$autoProposal) { $0.status == .reviewReady && $0.review != nil }
        model.setAssistMode(true)
        XCTAssertTrue(model.assistance.enabled)
        XCTAssertFalse(model.autoProposal.enabled)
        XCTAssertNil(model.autoProposal.review)
        XCTAssertNil(model.autoProposal.proposal)
        XCTAssertEqual(model.core?.revisionID, source)
        model.setAssistMode(false)
        await waitFor(model.$tools) { $0.contains { $0.toolID == "pixaura.rotate" } }
        let rotate = try XCTUnwrap(model.tools.firstIndex { $0.toolID == "pixaura.rotate" })
        model.select(rotate)
        await waitFor(model.$core) { $0?.active == true }
        model.update([1])
        model.interrupt()
        await waitFor(model.$busy) { $0 == false }
        await waitFor(model.$core) { $0?.active == false && $0?.revisionID == source }
        XCTAssertEqual(model.core?.revisionCount, 1)
        model.action(2)
        await waitFor(model.$busy) { $0 == false }
        XCTAssertEqual(model.core?.revisionCount, 1)
        XCTAssertEqual(model.core?.revisionID, source)
        model.close(); await waitFor(model.$core) { $0 == nil }
    }

    @MainActor func testAutoRequestWhileInterruptRetiresFailsBusyThenRecovers() async throws {
        let file = try XCTUnwrap(Bundle(for: ShellTests.self).url(forResource: "fixtures", withExtension: "json"))
        let fixtures = try XCTUnwrap(JSONSerialization.jsonObject(with: Data(contentsOf: file)) as? [String: [String: Any]])
        let input = FileManager.default.temporaryDirectory.appendingPathComponent("interrupt-admission-\(UUID()).png")
        try Data(try XCTUnwrap(fixtures["png"]?["bytes"] as? [UInt8])).write(to: input)
        defer { try? FileManager.default.removeItem(at: input) }
        let capability = AppleCapabilityOwner(signals: AppTestAppleSignals()); defer { capability.close() }
        let model = ManualEditorModel(capabilities: capability)
        model.importImage(input); await waitFor(model.$core) { $0?.revisionCount == 1 }
        let source = try XCTUnwrap(model.core?.revisionID)
        model.setAutoMode(true); await waitFor(model.$autoProposal) { $0.choices.count == 11 }
        model.chooseAuto(0); model.requestAuto()
        await waitFor(model.$autoProposal) { $0.status == .reviewReady && !$0.inFlight }
        // Hold retirement deterministically; inspect the existing queue without
        // adding a production hook, changing admission, or adding a retry.
        let worker = try XCTUnwrap(Mirror(reflecting: model).children.first { $0.label == "worker" }?.value)
        let queue = try XCTUnwrap(Mirror(reflecting: worker).children.first { $0.label == "commands" }?.value as? DispatchQueue)
        let gate = DispatchSemaphore(value: 0)
        defer { gate.signal() }
        let entered = expectation(description: "command retirement held")
        queue.async { entered.fulfill(); gate.wait() }
        await fulfillment(of: [entered], timeout: 20)
        model.interrupt()
        XCTAssertTrue(model.busy)
        XCTAssertNil(model.autoProposal.review)
        model.requestAuto()
        await waitFor(model.$autoProposal) { $0.status == .failed && !$0.inFlight }
        XCTAssertEqual(model.autoProposal.error, "Provider busy or resources limited. Manual editing is available.")
        XCTAssertNil(model.autoProposal.proposal); XCTAssertNil(model.autoProposal.review)
        XCTAssertEqual(model.core?.revisionID, source); XCTAssertEqual(model.core?.revisionCount, 1)
        gate.signal()
        await waitFor(model.$busy) { !$0 }
        model.requestAuto()
        await waitFor(model.$autoProposal) { $0.status == .reviewReady && $0.review != nil && !$0.inFlight }
        XCTAssertEqual(model.core?.revisionID, source); XCTAssertEqual(model.core?.revisionCount, 1)
        model.setAutoMode(false)
        let rotate = try XCTUnwrap(model.tools.firstIndex { $0.toolID == "pixaura.rotate" })
        model.select(rotate); await waitFor(model.$core) { $0?.active == true }
        model.update([1]); model.action(1)
        await waitFor(model.$core) { $0?.revisionCount == 2 }
        XCTAssertNotEqual(model.core?.revisionID, source)
        model.close(); await waitFor(model.$core) { $0 == nil }
    }

    @MainActor func testObservableOwnerDestructionClosesNativeStateWithoutViewCallback() async throws {
        let capability = AppleCapabilityOwner(signals: AppTestAppleSignals()); defer { capability.close() }
        var model: ManualEditorModel? = ManualEditorModel(capabilities: capability)
        let file = try XCTUnwrap(Bundle(for: ShellTests.self).url(forResource: "fixtures", withExtension: "json"))
        let fixtures = try XCTUnwrap(JSONSerialization.jsonObject(with: Data(contentsOf: file)) as? [String: [String: Any]])
        let input = FileManager.default.temporaryDirectory.appendingPathComponent("owner-lifecycle-\(UUID()).png")
        try Data(try XCTUnwrap(fixtures["png"]?["bytes"] as? [UInt8])).write(to: input)
        defer { try? FileManager.default.removeItem(at: input) }
        model!.importImage(input); await waitFor(model!.$core) { $0?.revisionCount == 1 }
        let source = try XCTUnwrap(model!.core?.revisionID)
        // Read-only test inspection; no lifecycle hook or production API added.
        let worker = try XCTUnwrap(Mirror(reflecting: model!).children.first { $0.label == "worker" }?.value)
        let queue = try XCTUnwrap(Mirror(reflecting: worker).children.first { $0.label == "commands" }?.value as? DispatchQueue)
        let optionalOwner = try XCTUnwrap(Mirror(reflecting: worker).children.first { $0.label == "owner" }?.value)
        let editor = try XCTUnwrap(Mirror(reflecting: optionalOwner).children.first?.value as? ManualEditor)
        let released = ModelLifetimeProbe(model!)
        model = nil
        XCTAssertNil(released.value)
        await withCheckedContinuation { (done: CheckedContinuation<Void, Never>) in
            queue.async { done.resume() }
        }
        XCTAssertThrowsError(try editor.state()) { XCTAssertEqual(($0 as? ManualEditorFailure)?.status, 13) }
        let reopened = ManualEditorModel(capabilities: capability)
        reopened.reopen(); await waitFor(reopened.$core) { $0?.revisionID == source }
        XCTAssertNil(reopened.autoProposal.review); XCTAssertNil(reopened.assistance.advice)
        reopened.close(); await waitFor(reopened.$core) { $0 == nil }
    }
    func testAppleOwnerPressureDuringGesturePreservesCommitCancelAndDisplay() async throws {
        try await Task.detached {
            let signals = AppTestAppleSignals()
            let live = AppleCapabilityOwner(signals: signals); defer { live.close() }
            let fixtureURL = try XCTUnwrap(Bundle(for: ShellTests.self).url(forResource: "fixtures", withExtension: "json"))
            let fixtures = try XCTUnwrap(JSONSerialization.jsonObject(with: Data(contentsOf: fixtureURL)) as? [String: [String: Any]])
            let root = FileManager.default.temporaryDirectory.appendingPathComponent("apple-app-\(UUID())")
            try FileManager.default.createDirectory(at: root, withIntermediateDirectories: true)
            defer { try? FileManager.default.removeItem(at: root) }
            let input = root.appendingPathComponent("input.png")
            try Data(try XCTUnwrap(fixtures["png"]?["bytes"] as? [UInt8])).write(to: input)
            let editor = try ManualEditor(root: root, importing: input, capability: live.snapshot()); defer { editor.close() }
            let exposure = try XCTUnwrap(editor.tools.firstIndex { $0.name == "Exposure" })
            let original = try XCTUnwrap(editor.render(editor.request())); XCTAssertTrue(editor.install(original))
            try editor.select(exposure); try editor.update([1234])
            let late = try XCTUnwrap(editor.render(editor.request()))
            signals.memory = 4
            XCTAssertEqual(try editor.applyCapability(live.snapshot()).previewEdge, 360)
            XCTAssertFalse(editor.install(late)); XCTAssertTrue(editor.displayedImage() === original.image)
            let next = try XCTUnwrap(editor.render(editor.request())); XCTAssertTrue(editor.install(next))
            XCTAssertFalse(editor.install(late)); XCTAssertEqual(try editor.state().values, [1234])
            XCTAssertEqual(try editor.state().revisionCount, 1)
            try editor.action(1); XCTAssertEqual(try editor.state().revisionCount, 2)
            try editor.select(exposure); try editor.update([-1234])
            signals.constrained = true; try editor.applyCapability(live.snapshot())
            try editor.action(0); XCTAssertEqual(try editor.state().revisionCount, 2)
            editor.close()
            // Initial decode reserves the frozen codec scratch envelope. A
            // critical 32 MiB cap cannot admit that envelope plus control and
            // metadata, even though already-open preview/commit stayed safe.
            XCTAssertThrowsError(try ManualEditor(root: root, capability: live.snapshot())) {
                XCTAssertEqual(($0 as? ManualEditorFailure)?.status, 8)
                XCTAssertEqual(($0 as? ManualEditorFailure)?.boundary, "Open")
            }
            signals.memory = 1; signals.constrained = false
            let fresh = AppleCapabilityOwner(signals: signals); defer { fresh.close() }
            let reopened = try ManualEditor(root: root, capability: fresh.snapshot()); defer { reopened.close() }
            XCTAssertEqual(try reopened.state().revisionCount, 2); XCTAssertEqual(try reopened.strategy().epoch, 1)
        }.value
    }
    func testBoundedPreviewPlatformConsumer() async throws {
        try await Task.detached {
            let fixtureURL = try XCTUnwrap(Bundle(for: ShellTests.self).url(forResource: "fixtures", withExtension: "json"))
            let fixtures = try XCTUnwrap(JSONSerialization.jsonObject(with: Data(contentsOf: fixtureURL)) as? [String: [String: Any]])
            let png = try XCTUnwrap(fixtures["png"])
            let bytes = try XCTUnwrap(png["bytes"] as? [UInt8])
            let hash = try XCTUnwrap(png["sha256"] as? String)
            let root = FileManager.default.temporaryDirectory.appendingPathComponent("preview-\(UUID().uuidString)")
            let assets = root.appendingPathComponent("assets/sha256")
            try FileManager.default.createDirectory(at: assets, withIntermediateDirectories: true)
            defer { try? FileManager.default.removeItem(at: root) }
            try Data(bytes).write(to: assets.appendingPathComponent(hash))
            // Native storage deliberately rejects symlink traversal. Foundation
            // URL normalization need not produce the POSIX canonical device path.
            let canonical: String = try root.withUnsafeFileSystemRepresentation { pointer in
                let resolved = try XCTUnwrap(pointer.flatMap { realpath($0, nil) })
                defer { free(resolved) }
                return String(cString: resolved)
            }
            let foundation = root.resolvingSymlinksInPath().path
            let readiness = try JSONSerialization.data(withJSONObject: ["foundation_matches_posix": foundation == canonical, "canonical_via_realpath": true], options: [.sortedKeys])
            let attachment = XCTAttachment(data: readiness, uniformTypeIdentifier: "public.json")
            attachment.name = "pixaura-preview-path-readiness.json"; attachment.lifetime = .keepAlways; self.add(attachment)
            let path = Array(canonical.utf8), digest = Array(hash.utf8)
            let id = Array(UUID().uuidString.replacingOccurrences(of: "-", with: "").lowercased().utf8)
            var limits = pixaura_decode_limits(), context = pixaura_decode_context()
            XCTAssertEqual(pixaura_decode_default_limits(1, &limits), 0)
            XCTAssertEqual(id.withUnsafeBufferPointer { pixaura_decode_context_init(1, &context, UInt32(MemoryLayout<pixaura_decode_context>.size), $0.baseAddress, 32, &limits) }, 0)
            defer { XCTAssertEqual(pixaura_decode_context_destroy(&context), 0) }
            var source = pixaura_decode_handle(), decoded = pixaura_decode_handle(), working = pixaura_decode_handle(), evaluated = pixaura_decode_handle(), preview = pixaura_decode_handle()
            XCTAssertEqual(path.withUnsafeBufferPointer { p in digest.withUnsafeBufferPointer { d in pixaura_decode_open(&context, p.baseAddress, UInt64(p.count), d.baseAddress, 64, UInt64(bytes.count), &source) } }, 0)
            XCTAssertEqual(pixaura_decode_image(&context, &source, &decoded), 0)
            var wl = pixaura_working_limits()
            XCTAssertEqual(pixaura_working_default_limits(1, &wl), 0)
            XCTAssertEqual(pixaura_working_normalize(&context, &decoded, &wl, &working), 0)
            let operations = Array("{\"operations\":[]}".utf8)
            XCTAssertEqual(operations.withUnsafeBufferPointer { pixaura_working_evaluate(&context, &working, 1, $0.baseAddress, UInt64($0.count), &wl, &evaluated) }, 0)
            var fit = pixaura_preview_request(version: 1, struct_size: UInt32(MemoryLayout<pixaura_preview_request>.size), mode: 1, max_width: 1, max_height: 2, reserved: 0)
            XCTAssertEqual(pixaura_preview_render(&context, &evaluated, &fit, nil, &preview), 0)
            let image = try XCTUnwrap(ReferencePreview.copyImage(context: &context, preview: &preview))
            XCTAssertEqual(pixaura_preview_release(&context, &preview), 0)
            XCTAssertEqual(image.width, 1)
            XCTAssertEqual(image.height, 1)
            XCTAssertEqual(image.alphaInfo, .last)
            XCTAssertEqual(image.dataProvider?.data as Data?, Data([188, 188, 255, 255]))
            var displayed = [UInt8](repeating: 0, count: 4)
            let drew = displayed.withUnsafeMutableBytes { storage -> Bool in
                guard let space = CGColorSpace(name: CGColorSpace.sRGB),
                      let display = CGContext(data: storage.baseAddress, width: 1, height: 1,
                                              bitsPerComponent: 8, bytesPerRow: 4, space: space,
                                              bitmapInfo: CGBitmapInfo.byteOrder32Big.rawValue | CGImageAlphaInfo.premultipliedLast.rawValue) else { return false }
                display.draw(image, in: CGRect(x: 0, y: 0, width: 1, height: 1))
                return true
            }
            XCTAssertTrue(drew)
            XCTAssertEqual(displayed, [188, 188, 255, 255])
        }.value
    }


    func testReferenceWorkloadMeasurementHarness() async throws {
        let data = try await Task.detached(priority: .userInitiated) { () throws -> Data in
            let requested = ProcessInfo.processInfo.environment["PIXAURA_BENCHMARK_SAMPLES"]
            guard requested == nil || requested == "100" else { throw ManualEditorFailure(status: 7, boundary: "Benchmark sample configuration") }
            let samples = requested == nil ? 1 : 100, warmups = requested == nil ? 0 : 5
            let directory = FileManager.default.temporaryDirectory.appendingPathComponent("reference-measurements-\(UUID())", isDirectory: true)
            try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
            defer { try? FileManager.default.removeItem(at: directory) }
            let capabilities = AppleCapabilityOwner(); defer { capabilities.close() }
            var registrySize: UInt64 = 0
            XCTAssertEqual(pixaura_manual_registry_json(1, nil, 0, &registrySize), 11)
            var registry = Data(count: Int(registrySize))
            try phase11Check(registry.withUnsafeMutableBytes { pixaura_manual_registry_json(1, $0.bindMemory(to: UInt8.self).baseAddress, registrySize, &registrySize) }, "Benchmark registry")
            struct Registry: Decodable { let tools: [ManualTool] }
            let tools = try JSONDecoder().decode(Registry.self, from: registry).tools
            let exposure = try XCTUnwrap(tools.firstIndex { $0.toolID == "pixaura.exposure" })
            var cases: [[String: Any]] = [], unexpectedFailures = 0
            for (width, height) in [(1024, 768), (3000, 2000), (4000, 3000)] {
                let input = directory.appendingPathComponent("synthetic-\(width)x\(height).png")
                try autoreleasepool { try phase11SyntheticPNG(input, width: width, height: height) }
                var records: [[String: Any]] = [], resourceFailure = false
                for index in 0..<(samples + warmups) {
                    var record: [String: Any] = ["sample": index, "warmup": index < warmups, "before": phase11ResourceReading()]
                    let project = directory.appendingPathComponent("project-\(UUID())", isDirectory: true)
                    try FileManager.default.createDirectory(at: project, withIntermediateDirectories: true)
                    var phase = "import"
                    do {
                        try autoreleasepool {
                            let owner = Phase11DeviceContext()
                            defer { record["cleanup"] = owner.close() }
                            let capability = try capabilities.snapshot()
                            record["policy_observation"] = ["process_budget_bytes": capability.observation.process_budget_bytes, "thermal": capability.observation.thermal, "memory_pressure": capability.observation.memory_pressure, "gpu_validation": capability.observation.gpu_validation]
                            let (strategy, openTime) = try phase11Milliseconds { try owner.open(root: project, input: input, capability: capability) }
                            record["import_ms"] = openTime
                            record["strategy"] = ["tier": strategy.tier, "preview_edge": strategy.preview_edge, "runtime_cap_bytes": strategy.runtime_cap_bytes, "backend": strategy.backend]
                            XCTAssertEqual(strategy.backend, 0, "Only existing CPU authority is permitted")
                            let original = try owner.state(); XCTAssertEqual(original.revisionCount, 1)
                            try phase11Check(pixaura_editor_select(owner.pointer, UInt32(exposure)), "Benchmark select")
                            var value: Int32 = 1233
                            try phase11Check(pixaura_editor_update(owner.pointer, &value, 1), "Benchmark update")
                            var pixels = [UInt8](repeating: 0, count: 4 * 1024 * 1024)
                            var previewWidth: UInt32 = 0, previewHeight: UInt32 = 0
                            func render() throws {
                                var request = pixaura_editor_request()
                                try phase11Check(pixaura_editor_request_preview(owner.pointer, &request), "Benchmark preview request")
                                try phase11Check(pixels.withUnsafeMutableBufferPointer { pixaura_editor_render(owner.pointer, &request, 720, 720, $0.baseAddress, UInt64($0.count), &previewWidth, &previewHeight) }, "Benchmark preview render")
                                try phase11Check(pixaura_editor_current(owner.pointer, &request), "Benchmark preview current")
                            }
                            phase = "preview priming"; try render()
                            // A rendered ticket is not reusable. A fresh gesture update
                            // obtains a new native generation, as in ordinary dragging.
                            value = 1234
                            try phase11Check(pixaura_editor_update(owner.pointer, &value, 1), "Benchmark fresh update")
                            phase = "warm preview"
                            let (_, previewTime) = try phase11Milliseconds { try render() }
                            record["warm_preview_ms"] = previewTime; record["preview_dimensions"] = [previewWidth, previewHeight]
                            XCTAssertEqual(try owner.state().revisionID, original.revisionID)
                            phase = "manual apply"
                            let (_, applyTime) = try phase11Milliseconds { try phase11Check(pixaura_editor_action(owner.pointer, 1), "Benchmark manual Apply") }
                            record["manual_apply_ms"] = applyTime
                            let applied = try owner.state(); XCTAssertEqual(applied.revisionCount, 2); XCTAssertNotEqual(applied.revisionID, original.revisionID)
                            var output = [UInt8](repeating: 0, count: 32 * 1024 * 1024)
                            for (format, label) in [(UInt32(1), "jpeg"), (UInt32(2), "png")] {
                                phase = label + " export"
                                var request = pixaura_export_request(); try phase11Check(pixaura_export_bind(1, owner.pointer, &request), "Benchmark export binding")
                                request.format = format; request.quality = format == 1 ? 90 : 0
                                var count: UInt64 = 0, exportWidth: UInt32 = 0, exportHeight: UInt32 = 0, exportState: UInt32 = 0
                                let (_, exportTime) = try phase11Milliseconds {
                                    try phase11Check(output.withUnsafeMutableBufferPointer { pixaura_export_encode(1, owner.pointer, &request, nil, $0.baseAddress, UInt64($0.count), &count, &exportWidth, &exportHeight, &exportState) }, "Benchmark export")
                                }
                                XCTAssertEqual(exportState, 6); XCTAssertEqual(exportWidth, UInt32(width)); XCTAssertEqual(exportHeight, UInt32(height))
                                XCTAssertEqual(Array(output.prefix(format == 1 ? 2 : 4)), format == 1 ? [0xff, 0xd8] : [0x89, 0x50, 0x4e, 0x47])
                                record[label + "_export_ms"] = exportTime; record[label + "_bytes"] = count
                                XCTAssertEqual(try owner.state().revisionID, applied.revisionID)
                            }
                            phase = "cancel"
                            try phase11Check(pixaura_editor_select(owner.pointer, UInt32(exposure)), "Benchmark cancel select")
                            try phase11Check(pixaura_editor_update(owner.pointer, &value, 1), "Benchmark cancel update")
                            let (_, cancelTime) = try phase11Milliseconds { try phase11Check(pixaura_editor_action(owner.pointer, 0), "Benchmark cancel") }
                            record["native_cancel_ms"] = cancelTime
                            XCTAssertEqual(try owner.state().revisionID, applied.revisionID)
                            record["status"] = "MEASURED"
                        }
                    } catch {
                        let status = (error as? ManualEditorFailure)?.status ?? 14
                        record["native_status"] = status; record["failure_phase"] = phase
                        if status == 8 && width >= 3000 { record["status"] = "RESOURCE_REJECTED_NOT_PERFORMANCE_PASS"; resourceFailure = true }
                        else { record["status"] = "FAILED"; unexpectedFailures += 1 }
                    }
                    record["after"] = phase11ResourceReading()
                    if let cleanup = record["cleanup"] as? [String: Any] {
                        XCTAssertEqual(cleanup["stop_status"] as? Int32, 0); XCTAssertEqual(cleanup["destroy_status"] as? Int32, 0)
                    }
                    records.append(record); try FileManager.default.removeItem(at: project)
                    if record["status"] as? String != "MEASURED" { break }
                }
                if width == 4000 { XCTAssertTrue(resourceFailure, "12MP cannot bypass frozen working raster limits") }
                cases.append(["width": width, "height": height, "encoded_bytes": try input.resourceValues(forKeys: [.fileSizeKey]).fileSize ?? 0,
                              "resource_rejected": resourceFailure, "records": records])
                if resourceFailure {
                    // A denied larger request cannot disable an ordinary small manual edit.
                    let recoveryInput = directory.appendingPathComponent("synthetic-1024x768.png")
                    let recoveryRoot = directory.appendingPathComponent("recovery-\(UUID())", isDirectory: true)
                    try FileManager.default.createDirectory(at: recoveryRoot, withIntermediateDirectories: true)
                    let recovery = Phase11DeviceContext(); defer { _ = recovery.close() }
                    _ = try recovery.open(root: recoveryRoot, input: recoveryInput, capability: capabilities.snapshot())
                    try phase11Check(pixaura_editor_select(recovery.pointer, UInt32(exposure)), "Benchmark manual recovery select")
                    var value: Int32 = 100; try phase11Check(pixaura_editor_update(recovery.pointer, &value, 1), "Benchmark manual recovery update")
                    try phase11Check(pixaura_editor_action(recovery.pointer, 1), "Benchmark manual recovery Apply")
                    let recoveryValid = try recovery.state().revisionCount == 2
                    let cleanup = recovery.close()
                    let closed = cleanup["stop_status"] as? Int32 == 0 && cleanup["destroy_status"] as? Int32 == 0
                    XCTAssertTrue(recoveryValid && closed)
                    cases[cases.count - 1]["manual_recovery_validated"] = recoveryValid && closed
                }
            }
            var model = utsname(); uname(&model)
            let capacity = MemoryLayout.size(ofValue: model.machine)
            let machine = withUnsafePointer(to: &model.machine) { $0.withMemoryRebound(to: CChar.self, capacity: capacity) { String(cString: $0) } }
            #if targetEnvironment(simulator)
            let physical = false
            #else
            let physical = true
            #endif
            #if DEBUG
            let debug = true
            #else
            let debug = false
            #endif
            XCTAssertEqual(unexpectedFailures, 0)
            return try JSONSerialization.data(withJSONObject: ["version": 1, "hardware": physical, "model": machine,
                "os": ProcessInfo.processInfo.operatingSystemVersionString, "debug_compilation": debug,
                "sampling_mode": samples == 100 ? "MEASUREMENT" : "SMOKE_NO_PERCENTILES", "samples_per_admitted_case": samples,
                "warmups_per_admitted_case": warmups, "requested_preview_edge": 720,
                "caller_preview_buffer_bytes": 4 * 1024 * 1024, "caller_export_buffer_bytes": 32 * 1024 * 1024,
                "scope": "native CPU app-service operations; caller buffers and fixture generation included in process high-water; no UI frame/dispatch or new RSS ceiling",
                "resource_policy": "actual AppleCapabilityOwner observations; no forced tier or enlarged budget", "cases": cases,
                "unexpected_failures": unexpectedFailures], options: [.sortedKeys])
        }.value
        let attachment = XCTAttachment(data: data, uniformTypeIdentifier: "public.json")
        attachment.name = "pixaura-reference-workload-measurements.json"; attachment.lifetime = .keepAlways; add(attachment)
    }

    func testNavigationPreservesSession() {
        let original = ShellState()
        var state = original
        state.openEditor(.assisted)
        state.navigate(.settings)
        state.back()
        XCTAssertEqual(original, ShellState())
        XCTAssertEqual(state.destination, .home)
        XCTAssertEqual(state.mode, .assisted)
        state.openEditor(.ai)
        XCTAssertEqual(state.destination, .editor)
        XCTAssertEqual(state.mode, .ai)
    }

    func testRealCoreContract() {
        XCTAssertEqual(CoreProbe.abiVersion(), 1)
    }

    func testRestorationRejectsUnknownValues() {
        XCTAssertEqual(ShellState.restore(destination: "Import", mode: "unknown"), ShellState())
        XCTAssertEqual(ShellState.restore(destination: "Editor", mode: "AI"),
                       ShellState(destination: .editor, mode: .ai))
    }

    @MainActor func testProbePublishesSuccessWithoutChangingSession() async {
        let model = ShellModel()
        model.openEditor(.manual)
        await model.checkCore()
        XCTAssertEqual(model.probe, "Shared core ready · ABI 1")
        XCTAssertEqual(model.state.destination, .editor)
    }
}

private final class AppTestAppleSignals: AppleCapabilitySignals {
    var memory: UInt32 = 1, constrained = false
    func facts() -> pixaura_capability_profile {
        var value = pixaura_capability_profile(); _ = pixaura_capability_baseline(1, &value)
        value.hardware.logical_cpu_count = 8; value.hardware.physical_memory_bytes = 8 << 30
        value.hardware.gpu_backend = 2; value.hardware.gpu_availability = 2; value.hardware.gpu_validation = 0
        value.hardware.provenance = 1
        value.runtime.thermal = 1; value.runtime.memory_pressure = 1; value.runtime.constrained = 0
        value.runtime.gpu_availability = 2; value.runtime.process_budget_bytes = 512 << 20; value.runtime.sequence = 1
        return value
    }
    func readings() -> (UInt32, UInt32, Bool, UInt32, UInt64) { (1, memory, constrained, 2, 512 << 20) }
    func start(_ changed: @escaping () -> Void) {}
    func stop() {}
}
