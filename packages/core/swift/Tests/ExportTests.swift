import XCTest
import Foundation
@testable import PixAuraCore

final class ExportTests: XCTestCase {
    private func fixture(_ body: (ManualEditor) throws -> Void) throws {
        let root = FileManager.default.temporaryDirectory.appendingPathComponent("export-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: root, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: root) }
        let file = try XCTUnwrap(Bundle.module.url(forResource: "decode", withExtension: "json", subdirectory: "Fixtures"))
        let json = try XCTUnwrap(JSONSerialization.jsonObject(with: Data(contentsOf: file)) as? [String: [String: Any]])
        let bytes = try XCTUnwrap(json["png"]?["bytes"] as? [UInt8])
        let input = root.appendingPathComponent("input.png"); try Data(bytes).write(to: input)
        let capability = AppleCapabilityOwner(signals: TestAppleSignals()); defer { capability.close() }
        let editor = try ManualEditor(root: root, importing: input, capability: capability.snapshot())
        defer { editor.close() }; try body(editor)
    }
    func testExportReadsTheCommittedRevisionAndLeavesHistoryUnchanged() throws {
        try fixture { editor in
            let before = try editor.state()
            let png = try editor.exportEncoded(format: 2, quality: 0)
            let again = try editor.exportEncoded(format: 2, quality: 0)
            XCTAssertEqual(png.bytes.prefix(8), Data([0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A]))
            XCTAssertEqual(png.bytes, again.bytes)
            XCTAssertGreaterThan(png.width, 0); XCTAssertGreaterThan(png.height, 0); XCTAssertEqual(png.state, 6)
            let jpeg = try editor.exportEncoded(format: 1, quality: 90)
            XCTAssertEqual(jpeg.bytes.prefix(2), Data([0xFF, 0xD8]))
            let after = try editor.state()
            XCTAssertEqual(after.revisionCount, before.revisionCount)
            XCTAssertEqual(after.revisionID, before.revisionID)
            XCTAssertThrowsError(try editor.exportEncoded(format: 2, quality: 0, revisionID: "0123456789abcdef0123456789abcdef")) { error in
                XCTAssertEqual((error as? ManualEditorFailure)?.status, 9)
            }
            XCTAssertEqual(try editor.state().revisionID, before.revisionID)
            XCTAssertNoThrow(try editor.exportCancel())
        }
    }
}
