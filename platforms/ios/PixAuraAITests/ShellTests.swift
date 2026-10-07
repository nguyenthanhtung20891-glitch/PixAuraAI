import XCTest
import PixAuraCore
import CPixAuraCore
import CoreGraphics
import Foundation
@testable import PixAuraAI

final class ShellTests: XCTestCase {
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
            let path = Array(root.resolvingSymlinksInPath().path.utf8), digest = Array(hash.utf8)
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
