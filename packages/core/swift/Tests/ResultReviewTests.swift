import XCTest
import Foundation
import CPixAuraCore
@testable import PixAuraCore

final class ResultReviewTests: XCTestCase {
    private func fixture(_ body: (ManualEditor) throws -> Void) throws {
        let root = FileManager.default.temporaryDirectory.appendingPathComponent("result-\(UUID().uuidString)")
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
    func testApplyBindsOneOrdinaryRevisionAndUndoRedoRefreshNativeHistory() throws {
        try fixture { editor in
            let review = try editor.prepareAutoReview(editor.requestAuto(2))
            XCTAssertEqual(try editor.state().revisionCount, 1)
            let receipt = try editor.applyAutoFromHuman(review)
            let described = try editor.resultReview(receipt)
            XCTAssertEqual(described.state, 1); XCTAssertEqual(described.count, 2)
            XCTAssertTrue(described.publicationVisible); XCTAssertTrue(described.canUndo)
            XCTAssertEqual(described.result, receipt.resultingRevision)
            XCTAssertEqual(described.live, receipt.resultingRevision)
            XCTAssertFalse(described.previous.isEmpty)
            let unconfirmed = try editor.resultReview(nil)
            XCTAssertEqual(unconfirmed.state, 3); XCTAssertFalse(unconfirmed.publicationVisible)
            XCTAssertEqual(unconfirmed.live, described.live); XCTAssertEqual(unconfirmed.count, 2)
            let previous = try editor.renderResult(described, revision: described.previous)
            let current = try editor.renderResult(described, revision: described.result)
            XCTAssertGreaterThan(previous.width, 0); XCTAssertGreaterThan(current.width, 0)
            try editor.action(2)
            let stale = try editor.resultReview(receipt)
            XCTAssertEqual(stale.state, 2); XCTAssertTrue(stale.canRedo); XCTAssertEqual(stale.live, described.previous)
            XCTAssertThrowsError(try editor.renderResult(described, revision: described.result))
            try editor.action(3)
            let restored = try editor.resultReview(receipt)
            XCTAssertEqual(restored.state, 1); XCTAssertEqual(restored.live, receipt.resultingRevision)
        }
    }
}
