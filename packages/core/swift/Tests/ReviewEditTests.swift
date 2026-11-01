import XCTest
import Foundation
import CPixAuraCore
@testable import PixAuraCore

final class ReviewEditTests: XCTestCase {
    private func fixture(_ body: (ManualEditor) throws -> Void) throws {
        let root = FileManager.default.temporaryDirectory.appendingPathComponent("inspection-\(UUID().uuidString)")
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
    func testEveryToolEditsNewReviewWithoutMutationAndRestoreDoesNotResurrectIdentity() throws {
        try fixture { editor in
            for choice in try editor.adviceChoices() {
                let original = try editor.prepareAutoReview(editor.requestAuto(choice.index))
                let bytes = original.canonicalBytes
                let pair = try editor.renderReviewPreview(editor.beginReviewPreview(original))
                let session = try editor.beginReviewEditing(original)
                let p = original.inspection.actions[0].parameters[0]
                let value: Int32 = original.inspection.actions[0].toolID == "pixaura.crop" ? 700000 : (p.raw == p.minimum ? p.maximum : p.minimum)
                let edited = try XCTUnwrap(editor.editReview(session, operation: 1, value: value))
                XCTAssertFalse(edited === original); XCTAssertEqual(edited.inspection.actions[0].parameters[0].raw, value)
                XCTAssertEqual(original.canonicalBytes, bytes); XCTAssertThrowsError(try editor.inspectReview(original))
                XCTAssertFalse(editor.installReviewPreview(pair) {}); XCTAssertThrowsError(try editor.applyAutoFromHuman(original))
                XCTAssertEqual(try editor.state().revisionCount, 1)
                let restored = try XCTUnwrap(editor.editReview(session, operation: 4))
                XCTAssertEqual(restored.canonicalBytes, bytes); XCTAssertFalse(restored === original)
                XCTAssertFalse(editor.installReviewPreview(pair) {}); XCTAssertThrowsError(try editor.inspectReview(edited))
                XCTAssertThrowsError(try editor.editReview(session, operation: 1, value: Int32.max))
                XCTAssertNil(session.review); XCTAssertEqual(session.state, 4)
                _ = try editor.editReview(session, operation: 4)
                XCTAssertNil(try editor.editReview(session, operation: 2)); XCTAssertEqual(session.state, 3)
                XCTAssertEqual(try editor.state().revisionCount, 1)
                _ = try editor.editReview(session, operation: 4)
                try editor.cancelAuto()
            }
        }
    }
    func testEditedPlanAloneIsAppliedOnceAndMultiActionOrderIsNative() throws {
        try fixture { editor in
            let binding = String(decoding: try editor.agentBinding(), as: UTF8.self).trimmingCharacters(in: .whitespacesAndNewlines)
            let actions = (0..<8).map { i in
                i == 3 ? "{\"action_index\":3,\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"height_ppm\":800000,\"width_ppm\":800000,\"x_ppm\":100000,\"y_ppm\":100000},\"tool_id\":\"pixaura.crop\"}" : "{\"action_index\":\(i),\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":\(101+i)},\"tool_id\":\"pixaura.exposure\"}"
            }.joined(separator: ",")
            let original = try editor.prepareAutoReview(editor.inspectionCandidateTest(Data("{\"actions\":[\(actions)],\"binding\":\(binding),\"plan_version\":1}\n".utf8)))
            let session = try editor.beginReviewEditing(original)
            let moved = try XCTUnwrap(editor.editReview(session, operation: 3, action: 0, destination: 7))
            XCTAssertEqual(moved.inspection.actions.map { $0.parameters[0].raw }, [102,103,800000,105,106,107,108,101])
            let removed = try XCTUnwrap(editor.editReview(session, operation: 2, action: 2))
            XCTAssertEqual(removed.inspection.actions.count, 7)
            XCTAssertFalse(removed.inspection.actions.contains { $0.geometry })
            let restored = try XCTUnwrap(editor.editReview(session, operation: 4)); XCTAssertEqual(restored.canonicalBytes, original.canonicalBytes)
            _ = try editor.editReview(session, operation: 1, value: -250)
            XCTAssertThrowsError(try editor.applyAutoFromHuman(original)); XCTAssertEqual(try editor.state().revisionCount, 1)
            let final = try XCTUnwrap(session.review)
            _ = try editor.applyAutoFromHuman(final); XCTAssertEqual(try editor.state().revisionCount, 2)
            XCTAssertThrowsError(try editor.applyAutoFromHuman(final)); XCTAssertThrowsError(try editor.editReview(session, operation: 4))
            try editor.action(2); try editor.action(3); XCTAssertEqual(try editor.state().revisionCount, 2)
        }
    }
}
