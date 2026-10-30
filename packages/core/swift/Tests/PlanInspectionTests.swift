import XCTest
import Foundation
import CPixAuraCore
@testable import PixAuraCore

final class PlanInspectionTests: XCTestCase {
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
    func testAllElevenNativeDescriptionsAreExactInertAndStaleAfterManualChange() throws {
        try fixture { editor in
            let source = try editor.state().revisionID
            for choice in try editor.adviceChoices() {
                let review = try editor.prepareAutoReview(editor.requestAuto(choice.index))
                let description = try editor.inspectReview(review)
                XCTAssertEqual(description.actions.count, 1)
                let a = try XCTUnwrap(description.actions.first)
                let tool = editor.tools[choice.toolIndex]
                XCTAssertEqual(a.index, 0); XCTAssertEqual(a.toolID, tool.toolID)
                XCTAssertEqual(a.parameters.map(\.raw), choice.values)
                XCTAssertEqual(a.parameters.map(\.name), tool.parameters.map(\.name))
                XCTAssertEqual(a.parameters.map(\.unit), tool.parameters.map(\.unit))
                XCTAssertEqual(a.operationVersion, 1); XCTAssertEqual(a.parameterVersion, 1)
                XCTAssertEqual(try editor.inspectReview(review).actions.map(\.toolID), [a.toolID])
                try fixture { other in
                    _ = try other.prepareAutoReview(other.requestAuto(choice.index))
                    XCTAssertThrowsError(try other.inspectReview(review))
                }
                XCTAssertEqual(try editor.state().revisionID, source); XCTAssertEqual(try editor.state().revisionCount, 1)
                try editor.cancelAuto(); XCTAssertThrowsError(try editor.inspectReview(review))
            }
            let review = try editor.prepareAutoReview(editor.requestAuto(2))
            try editor.select(4); try editor.update([250]); try editor.action(1)
            XCTAssertThrowsError(try editor.inspectReview(review)); try editor.action(2); try editor.action(3)
            XCTAssertThrowsError(try editor.inspectReview(review))
        }
    }
    func testEightActionOrderRawPrecisionAndPreviewAssociation() throws {
        try fixture { editor in
            let binding = String(decoding: try editor.agentBinding(), as: UTF8.self).trimmingCharacters(in: .whitespacesAndNewlines)
            let actions = (0..<8).map { i in
                "{\"action_index\":\(i),\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":\(101+i)},\"tool_id\":\"pixaura.exposure\"}"
            }.joined(separator: ",")
            let bytes = Data("{\"actions\":[\(actions)],\"binding\":\(binding),\"plan_version\":1}\n".utf8)
            let review = try editor.prepareAutoReview(editor.inspectionCandidateTest(bytes))
            let value = try editor.inspectReview(review)
            XCTAssertEqual(value.actions.map(\.index), Array(0..<8)); XCTAssertEqual(value.actions.map { $0.parameters[0].raw }, Array(101...108).map { Int32($0) })
            let pair = try editor.renderReviewPreview(editor.beginReviewPreview(review)); XCTAssertTrue(editor.installReviewPreview(pair) {})
            XCTAssertEqual(try editor.state().revisionCount, 1)
            let newer = try editor.prepareAutoReview(editor.requestAuto(1))
            XCTAssertThrowsError(try editor.inspectReview(review)); XCTAssertFalse(editor.installReviewPreview(pair) {})
            XCTAssertEqual(try editor.inspectReview(newer).actions.count, 1)
        }
    }
}
