import XCTest
import Foundation
import PixAuraCore

final class ManualEditorTests: XCTestCase {
    func testRealOwnerToolsIntegerChurnApprovalCancelHistoryAndReopen() async throws {
        try await Task.detached {
            let root = FileManager.default.temporaryDirectory.appendingPathComponent("editor-\(UUID().uuidString)")
            try FileManager.default.createDirectory(at: root, withIntermediateDirectories: true)
            let fixture = try XCTUnwrap(Bundle.module.url(forResource: "decode", withExtension: "json", subdirectory: "Fixtures"))
            let json = try XCTUnwrap(JSONSerialization.jsonObject(with: Data(contentsOf: fixture)) as? [String: [String: Any]])
            let png = try XCTUnwrap(json["png"]), bytes = try XCTUnwrap(png["bytes"] as? [UInt8])
            let input = root.appendingPathComponent("picker.png"); try Data(bytes).write(to: input)
            defer { try? FileManager.default.removeItem(at: root) }
            let editor = try ManualEditor(root: root, importing: input)
            defer { editor.close() }
            XCTAssertEqual(editor.tools.map(\.toolID), ["pixaura.blur", "pixaura.brightness", "pixaura.contrast", "pixaura.crop", "pixaura.exposure", "pixaura.highlights", "pixaura.rotate", "pixaura.saturation", "pixaura.shadows", "pixaura.sharpen", "pixaura.temperature"])
            func render() throws -> ManualEditor.Candidate {
                let candidate = try XCTUnwrap(editor.render(editor.request()))
                XCTAssertTrue(editor.install(candidate)); return candidate
            }
            func pixels(_ c: ManualEditor.Candidate) throws -> Data { try XCTUnwrap(c.image.dataProvider?.data as Data?) }
            let base = try render()
            for (index, tool) in editor.tools.enumerated() {
                try editor.select(index); try editor.update(tool.parameters.map { Int32($0.defaultValue) }); try editor.action(1)
                XCTAssertEqual(try editor.state().revisionCount, 1)
            }
            let exposure = try XCTUnwrap(editor.tools.firstIndex { $0.name == "Exposure" })
            let parameter = editor.tools[exposure].parameters[0]
            XCTAssertEqual(parameter.integerPosition(-0.5), 0)
            XCTAssertEqual(parameter.integerPosition(-0.51), -1)
            XCTAssertEqual(parameter.integerPosition(1000.49), 1000)
            XCTAssertEqual(parameter.integerPosition(1000.5), 1001)
            XCTAssertEqual(parameter.displayValue(1000), "1.0 EV")
            try editor.select(exposure)
            for i in 0...10000 { try editor.update([Int32(i == 10000 ? 1000 : i % 5001)]) }
            XCTAssertEqual(try editor.state().revisionCount, 1); XCTAssertEqual(try editor.state().values, [1000])
            let changed = try render(); XCTAssertNotEqual(try pixels(changed), try pixels(base))
            try editor.action(1); XCTAssertEqual(try editor.state().revisionCount, 2); XCTAssertFalse(editor.install(changed))
            let committed = try render()
            try editor.action(2); XCTAssertEqual(try pixels(render()), try pixels(base))
            try editor.action(3); XCTAssertEqual(try pixels(render()), try pixels(committed))
            try editor.select(exposure); try editor.update([-1000]); let cancelled = try render()
            try editor.action(0); XCTAssertFalse(editor.install(cancelled)); XCTAssertEqual(try editor.state().revisionCount, 2)
            XCTAssertEqual(editor.displayedImage()?.dataProvider?.data as Data?, try pixels(committed))
            try editor.select(exposure); try editor.update([10]); let switched = try render()
            let blur = try XCTUnwrap(editor.tools.firstIndex { $0.name == "Blur" }); try editor.select(blur)
            XCTAssertFalse(editor.install(switched)); try editor.update([500]); let interrupted = try render()
            try editor.action(4); XCTAssertFalse(editor.install(interrupted))
            let crop = try XCTUnwrap(editor.tools.firstIndex { $0.name == "Crop" }); try editor.select(crop)
            XCTAssertThrowsError(try editor.update([1000000, 500000, 600000, 0])) { XCTAssertEqual(($0 as? ManualEditorFailure)?.status, 7) }
            try editor.update([1000000, 500000, 0, 0]); try editor.action(0)
            editor.close(); XCTAssertGreaterThan(committed.image.width, 0)
            let reopened = try ManualEditor(root: root); defer { reopened.close() }
            XCTAssertEqual(try reopened.state().revisionCount, 2); XCTAssertTrue(try reopened.state().canUndo)
            let restored = try XCTUnwrap(reopened.render(reopened.request()))
            XCTAssertEqual(try pixels(restored), try pixels(committed))
            let invalidRoot = root.appendingPathComponent("invalid")
            try FileManager.default.createDirectory(at: invalidRoot, withIntermediateDirectories: false)
            let invalidInput = invalidRoot.appendingPathComponent("malformed.png")
            try Data([0, 1, 2, 3]).write(to: invalidInput)
            XCTAssertThrowsError(try ManualEditor(root: invalidRoot, importing: invalidInput))
        }.value
    }
}
