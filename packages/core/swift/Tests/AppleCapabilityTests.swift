import XCTest
import Foundation
import Dispatch
import CoreGraphics
import ImageIO
import CPixAuraCore
@testable import PixAuraCore

final class TestAppleSignals: AppleCapabilitySignals {
    var cpu: UInt32 = 8, ram: UInt64 = 8 << 30, budget: UInt64 = 512 << 20
    var thermal: UInt32 = 1, memory: UInt32 = 1, constrained = false, backend: UInt32 = 2
    var callback: (() -> Void)?, registrations = 0, removals = 0
    func facts() -> pixaura_capability_profile {
        var value = pixaura_capability_profile(); _ = pixaura_capability_baseline(1, &value)
        value.hardware.platform = 4; value.hardware.logical_cpu_count = cpu
        value.hardware.physical_memory_bytes = ram; value.hardware.provenance = 1
        value.hardware.gpu_backend = 2; value.hardware.gpu_availability = backend; value.hardware.gpu_validation = 0
        value.runtime.thermal = 1; value.runtime.memory_pressure = 1; value.runtime.constrained = 0
        value.runtime.gpu_availability = backend; value.runtime.process_budget_bytes = budget; value.runtime.sequence = 1
        return value
    }
    func readings() -> (UInt32, UInt32, Bool, UInt32, UInt64) { (thermal, memory, constrained, backend, budget) }
    func start(_ changed: @escaping () -> Void) { registrations += 1; callback = changed }
    func stop() { if callback != nil { removals += 1 }; callback = nil }
    func setConstrained(_ value: Bool) { constrained = value; callback?() }
}
private final class AppleTestCounter: @unchecked Sendable {
    private let lock = NSLock()
    private var value = 0
    func increment() { lock.lock(); value += 1; lock.unlock() }
    var count: Int { lock.lock(); defer { lock.unlock() }; return value }
}
private final class AppleWeakOwner {
    weak var value: AppleCapabilityOwner?
    init(_ owner: AppleCapabilityOwner?) { value = owner }
}
// Test fixture owns immutable inputs. ManualEditor's existing gate serializes
// native transitions; the counter synchronizes failures. No production Sendable
// conformance or warning suppression is added for the test.
private final class ApplePressureRace: @unchecked Sendable {
    let editor: ManualEditor, observation: AppleCapabilitySnapshot, action: UInt32
    let failures = AppleTestCounter()
    init(_ editor: ManualEditor, _ observation: AppleCapabilitySnapshot, _ action: UInt32) {
        self.editor = editor; self.observation = observation; self.action = action
    }
    func run() {
        let group = DispatchGroup()
        group.enter(); DispatchQueue.global().async { [self] in
            defer { group.leave() }
            do { try editor.applyCapability(observation) } catch { failures.increment() }
        }
        group.enter(); DispatchQueue.global().async { [self] in
            defer { group.leave() }
            do { try editor.action(action) } catch { failures.increment() }
        }
        group.wait()
    }
}

final class AppleCapabilityTests: XCTestCase {
    func testMappingsAndActualProbeNeverCertifies() throws {
        XCTAssertEqual([ProcessInfo.ThermalState.nominal, .fair, .serious, .critical].map(SystemAppleCapabilitySignals.thermal), [1, 2, 3, 4])
        XCTAssertEqual(SystemAppleCapabilitySignals.memory(.normal), 1)
        XCTAssertEqual(SystemAppleCapabilitySignals.memory(.warning), 3)
        XCTAssertEqual(SystemAppleCapabilitySignals.memory([.warning, .critical]), 4)
        XCTAssertEqual(SystemAppleCapabilitySignals.memory([]), 0)
        let owner = AppleCapabilityOwner(); defer { owner.close() }
        var value = try owner.snapshot()
        XCTAssertEqual(pixaura_capability_validate(1, &value.profile), 0)
        XCTAssertEqual(value.profile.hardware.gpu_validation, 0)
        XCTAssertEqual(value.observation.gpu_validation, 0)
        XCTAssertLessThanOrEqual(value.profile.hardware.logical_cpu_count, 1024)
        #if targetEnvironment(simulator)
        XCTAssertEqual(value.profile.hardware.provenance, 2)
        #else
        XCTAssertEqual(value.profile.hardware.provenance, 0)
        #endif
    }
    func testBoundedSubscriptionsReconstructionAndStaleCallbacks() throws {
        let signals = TestAppleSignals()
        var owner: AppleCapabilityOwner? = AppleCapabilityOwner(signals: signals)
        let weakOwner = AppleWeakOwner(owner)
        var calls = 0
        let first = try owner!.subscribe { _ in calls += 1 }
        let second = try owner!.subscribe { _ in calls += 1 }
        XCTAssertEqual(signals.registrations, 1)
        let stale = signals.callback
        for _ in 0..<10000 { signals.callback?() }
        XCTAssertEqual(calls, 20000)
        owner!.unsubscribe(first); XCTAssertEqual(signals.removals, 0)
        owner!.unsubscribe(second); XCTAssertEqual(signals.removals, 1)
        stale?(); XCTAssertEqual(calls, 20000)
        let replacement = try owner!.subscribe { _ in calls += 1 }
        stale?(); XCTAssertEqual(calls, 20000)
        signals.callback?(); XCTAssertEqual(calls, 20001)
        var ids = [replacement]
        for _ in 0..<7 { ids.append(try owner!.subscribe { _ in }) }
        XCTAssertThrowsError(try owner!.subscribe { _ in })
        owner!.close(); stale?(); XCTAssertEqual(calls, 20001)
        XCTAssertEqual(signals.removals, 2)
        owner = nil; XCTAssertNil(weakOwner.value)
        let fresh = AppleCapabilityOwner(signals: signals); defer { fresh.close() }
        XCTAssertEqual(try fresh.snapshot().observation.sequence, 1)
    }
    func testPressureRecoveryGestureHistoryAndLatePublication() async throws {
        try await Task.detached {
            for tier in 0..<3 {
                let signals = TestAppleSignals()
                signals.cpu = [2, 4, 8][tier]; signals.ram = [2, 4, 8][tier] << 30
                signals.budget = [128, 256, 512][tier] << 20
                let capabilities = AppleCapabilityOwner(signals: signals); defer { capabilities.close() }
                let root = FileManager.default.temporaryDirectory.appendingPathComponent("apple-policy-\(UUID())")
                try FileManager.default.createDirectory(at: root, withIntermediateDirectories: true)
                defer { try? FileManager.default.removeItem(at: root) }
                let input = root.appendingPathComponent("input.png")
                let bitmap = try XCTUnwrap(CGContext(data: nil, width: 1024, height: 512, bitsPerComponent: 8,
                    bytesPerRow: 4096, space: CGColorSpaceCreateDeviceRGB(), bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue))
                bitmap.setFillColor(CGColor(gray: 0.4, alpha: 1)); bitmap.fill(CGRect(x: 0, y: 0, width: 1024, height: 512))
                let png = try XCTUnwrap(CGImageDestinationCreateWithURL(input as CFURL, "public.png" as CFString, 1, nil))
                CGImageDestinationAddImage(png, try XCTUnwrap(bitmap.makeImage()), nil)
                XCTAssertTrue(CGImageDestinationFinalize(png))
                let editor = try ManualEditor(root: root, importing: input, capability: capabilities.snapshot())
                defer { editor.close() }
                let baseline = try editor.strategy(); XCTAssertEqual(baseline.tier, UInt32(tier))
                XCTAssertLessThanOrEqual(baseline.runtimeCapBytes, 256 << 20)
                XCTAssertTrue(baseline.forcedCPU)
                let initial = try XCTUnwrap(editor.render(editor.request())); XCTAssertTrue(editor.install(initial))
                XCTAssertEqual(initial.image.width, 512); XCTAssertEqual(initial.image.height, 256)
                let exposure = try XCTUnwrap(editor.tools.firstIndex { $0.name == "Exposure" })
                try editor.select(exposure); try editor.update([1234])
                let late = try XCTUnwrap(editor.render(editor.request()))
                signals.thermal = 4
                let critical = try editor.applyCapability(capabilities.snapshot())
                XCTAssertEqual(critical.tier, 0); XCTAssertEqual(critical.previewEdge, 360)
                XCTAssertLessThanOrEqual(critical.runtimeCapBytes, 32 << 20)
                XCTAssertFalse(editor.install(late)); XCTAssertNotNil(editor.displayedImage())
                XCTAssertEqual(try editor.state().values, [1234]); XCTAssertEqual(try editor.state().revisionCount, 1)
                let newest = try XCTUnwrap(editor.render(editor.request())); XCTAssertTrue(editor.install(newest))
                XCTAssertEqual(newest.image.width, 360); XCTAssertEqual(newest.image.height, 180)
                XCTAssertFalse(editor.install(late))
                signals.thermal = 1
                for index in 1...3 {
                    let recovered = try editor.applyCapability(capabilities.snapshot())
                    XCTAssertEqual(recovered.tier, index == 3 ? UInt32(tier) : 0)
                    XCTAssertLessThanOrEqual(recovered.runtimeCapBytes, baseline.runtimeCapBytes)
                }
                for severity: UInt32 in [2, 3, 4] {
                    signals.memory = severity
                    let strategy = try editor.applyCapability(capabilities.snapshot())
                    XCTAssertEqual(strategy.tier, 0); XCTAssertLessThanOrEqual(strategy.runtimeCapBytes, baseline.runtimeCapBytes)
                }
                try editor.action(1); XCTAssertEqual(try editor.state().revisionCount, 2)
                try editor.select(exposure); try editor.update([-1234]); signals.backend = 1
                let lost = try editor.applyCapability(capabilities.snapshot()); XCTAssertTrue(lost.forcedCPU)
                try editor.action(0); XCTAssertEqual(try editor.state().revisionCount, 2)
                signals.memory = 1; signals.constrained = true
                XCTAssertEqual(try editor.applyCapability(capabilities.snapshot()).tier, 0)
                signals.constrained = false; signals.backend = 2
                for _ in 0..<3 { try editor.applyCapability(capabilities.snapshot()) }
                let stable = try editor.strategy().epoch
                for _ in 0..<1000 { try editor.applyCapability(capabilities.snapshot()) }
                XCTAssertEqual(try editor.strategy().epoch, stable)
                var malformed = try capabilities.snapshot(); malformed.observation.thermal = 99
                XCTAssertThrowsError(try editor.applyCapability(malformed))
                malformed = try capabilities.snapshot(); malformed.observation.gpu_validation = 2
                XCTAssertThrowsError(try editor.applyCapability(malformed))
                malformed = try capabilities.snapshot(); malformed.observation.version = 2
                XCTAssertThrowsError(try editor.applyCapability(malformed))
                for iteration in 0..<16 {
                    try editor.select(exposure); try editor.update([1234])
                    var next = try capabilities.snapshot(); next.observation.memory_pressure = 3
                    let race = ApplePressureRace(editor, next, iteration % 2 == 0 ? 0 : 1)
                    race.run(); XCTAssertEqual(race.failures.count, 0)
                }
                let revisions = try editor.state().revisionCount
                editor.close(); XCTAssertThrowsError(try editor.applyCapability(capabilities.snapshot()))
                let reopened = try ManualEditor(root: root, capability: capabilities.snapshot()); defer { reopened.close() }
                XCTAssertEqual(try reopened.state().revisionCount, revisions); XCTAssertEqual(try reopened.strategy().epoch, 1)
            }
        }.value
    }
    func testBusyLanePreservesCriticalObservation() throws {
        let signals = TestAppleSignals(), owner: AppleCapabilityOwner
        owner = AppleCapabilityOwner(signals: signals); defer { owner.close() }
        signals.memory = 4; signals.backend = 1; let critical = try owner.snapshot()
        signals.memory = 1; signals.backend = 2
        let merged = try owner.snapshot().retainingPressure(from: critical)
        XCTAssertEqual(merged.observation.memory_pressure, 4)
        XCTAssertEqual(merged.observation.gpu_availability, 1)
        XCTAssertEqual(merged.observation.gpu_validation, 0)
    }
    func testConcurrentCallbacksAndCloseAreBounded() throws {
        let signals = TestAppleSignals()
        // Immutable fake readings; synchronization and sequence authority reside in owner.
        let live = AppleCapabilityOwner(signals: signals)
        let counter = AppleTestCounter()
        _ = try live.subscribe { _ in counter.increment() }
        let callback = try XCTUnwrap(signals.callback)
        DispatchQueue.concurrentPerform(iterations: 10000) { _ in callback() }
        XCTAssertEqual(counter.count, 10000)
        DispatchQueue.concurrentPerform(iterations: 100) { index in if index == 50 { live.close() } else { callback() } }
        let final = counter.count; callback(); XCTAssertEqual(counter.count, final)
        XCTAssertThrowsError(try live.snapshot())
    }
}
