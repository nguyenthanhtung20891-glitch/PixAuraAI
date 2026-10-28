import XCTest
import Foundation
import Dispatch
import CPixAuraCore
@testable import PixAuraCore

private final class PreviewCall: @unchecked Sendable {
    let editor: ManualEditor, request: ManualEditor.ReviewPreviewRequest
    private let lock = NSLock()
    private var code: Int32 = -1
    init(_ editor: ManualEditor, _ request: ManualEditor.ReviewPreviewRequest) { self.editor = editor; self.request = request }
    func run() { var result: Int32 = 0; do { _ = try editor.renderReviewPreview(request) } catch { result = (error as? ManualEditorFailure)?.status ?? -1 }; lock.lock(); code = result; lock.unlock() }
    var status: Int32 { lock.lock(); defer { lock.unlock() }; return code }
}
final class ReviewPreviewTests: XCTestCase {
    private func fixture(_ body: (ManualEditor) throws -> Void) throws {
        let root = FileManager.default.temporaryDirectory.appendingPathComponent("preview-\(UUID().uuidString)")
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
    func testDetachedComparisonDoesNotApproveOrCommitAndExactApplyStillWorks() throws {
        try fixture { editor in
            try editor.select(4); try editor.update([100]); try editor.action(1)
            let source = try editor.state().revisionID, count = try editor.state().revisionCount
            let review = try editor.prepareAutoReview(editor.requestAuto(2))
            let first = try editor.renderReviewPreview(editor.beginReviewPreview(review))
            XCTAssertTrue(editor.installReviewPreview(first) {})
            XCTAssertEqual(first.before.width, 2); XCTAssertEqual(first.before.height, 3); XCTAssertFalse(first.geometryChanged)
            XCTAssertEqual(editor.reviewPreviewTestControl(5), 0)
            let second = try editor.renderReviewPreview(editor.beginReviewPreview(review))
            XCTAssertEqual(first.after.dataProvider?.data as Data?, second.after.dataProvider?.data as Data?)
            XCTAssertFalse(editor.installReviewPreview(first) {})
            XCTAssertTrue(editor.installReviewPreview(second) {})
            XCTAssertEqual(try editor.state().revisionID, source); XCTAssertEqual(try editor.state().revisionCount, count)
            try fixture { other in XCTAssertFalse(other.installReviewPreview(second) {}) }
            _ = try editor.applyAutoFromHuman(review)
            XCTAssertEqual(try editor.state().revisionCount, count + 1); XCTAssertFalse(editor.installReviewPreview(second) {})
            try editor.action(2); XCTAssertEqual(try editor.state().revisionID, source)
            try editor.action(3); XCTAssertEqual(try editor.state().revisionCount, count + 1)
        }
    }
    func testCancellationModeAbandonmentManualChangeAndStopDrainBeforeDestroy() throws {
        for scenario in 0..<4 {
            try fixture { editor in
                let review = try editor.prepareAutoReview(editor.requestAuto(2))
                let request = try editor.beginReviewPreview(review)
                XCTAssertEqual(editor.reviewPreviewTestControl(0, 0), 0)
                let call = PreviewCall(editor, request), done = expectation(description: "native preview drained")
                DispatchQueue.global().async { call.run(); done.fulfill() }
                let limit = Date().addingTimeInterval(10)
                while editor.reviewPreviewTestControl(1) != 1 && Date() < limit { Thread.sleep(forTimeInterval: 0.001) }
                XCTAssertEqual(editor.reviewPreviewTestControl(1), 1)
                defer { _ = editor.reviewPreviewTestControl(2) }
                if scenario == 0 { try editor.cancelReviewPreview(request) }
                if scenario == 1 { try editor.cancelAuto() }
                if scenario == 2 { try editor.select(4); try editor.update([100]); try editor.action(1) }
                if scenario == 3 {
                    let closed = expectation(description: "owner joined")
                    DispatchQueue.global().async { editor.close(); closed.fulfill() }
                    // stop need not be presented as publication/cancellation success.
                    _ = editor.reviewPreviewTestControl(2); wait(for: [done, closed], timeout: 10)
                    XCTAssertTrue([Int32(9),13,0].contains(call.status)); return
                }
                _ = editor.reviewPreviewTestControl(2); wait(for: [done], timeout: 10)
                XCTAssertTrue([Int32(9),13].contains(call.status))
                XCTAssertEqual(try editor.state().revisionCount, scenario == 2 ? 2 : 1)
            }
        }
    }
    func testFailureAndGeometryNeverCreateHistory() throws {
        try fixture { editor in
            let review = try editor.prepareAutoReview(editor.requestAuto(2)), request = try editor.beginReviewPreview(review)
            XCTAssertEqual(editor.reviewPreviewTestControl(3, 2), 0)
            XCTAssertThrowsError(try editor.renderReviewPreview(request)) { XCTAssertEqual(($0 as? ManualEditorFailure)?.status, 8) }
            XCTAssertEqual(try editor.state().revisionCount, 1); _ = editor.reviewPreviewTestControl(3, 0)
            let geometry = try editor.prepareAutoReview(editor.requestAuto(10))
            let pair = try editor.renderReviewPreview(editor.beginReviewPreview(geometry))
            XCTAssertTrue(pair.geometryChanged); XCTAssertTrue(editor.installReviewPreview(pair) {})
            try editor.select(4); try editor.update([250]); try editor.action(1)
            XCTAssertFalse(editor.installReviewPreview(pair) {})
        }
    }
}
