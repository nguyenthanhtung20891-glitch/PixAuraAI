import XCTest
import Foundation
import Dispatch
import CryptoKit
import CPixAuraCore
@testable import PixAuraCore

private final class ReviewRun: @unchecked Sendable {
    let editor: ManualEditor, review: ManualEditor.AutoReview
    private let lock = NSLock()
    private var code: Int32 = -1
    init(_ e: ManualEditor, _ r: ManualEditor.AutoReview) { editor = e; review = r }
    func run() { var result: Int32 = 0; do { _ = try editor.applyAutoFromHuman(review) } catch { result = (error as? ManualEditorFailure)?.status ?? -1 }; lock.lock(); code = result; lock.unlock() }
    var status: Int32 { lock.lock(); defer { lock.unlock() }; return code }
}
final class AutoReviewTests: XCTestCase {
    private func fixture(_ body: (ManualEditor) throws -> Void) throws {
        let root = FileManager.default.temporaryDirectory.appendingPathComponent("review-\(UUID().uuidString)")
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
    private func status(_ expected: Int32, _ body: () throws -> Void) {
        XCTAssertThrowsError(try body()) { XCTAssertEqual(($0 as? ManualEditorFailure)?.status, expected) }
    }
    func testExactHumanApplyOneRevisionReplayAndOrdinaryManualHistory() throws {
        try fixture { editor in
            try editor.select(4); try editor.update([100]); try editor.action(1)
            let source = try editor.state().revisionID, count = try editor.state().revisionCount
            let proposal = try editor.requestAuto(2), review = try editor.prepareAutoReview(proposal)
            let reviewed = review.canonicalBytes
            XCTAssertEqual(reviewed, try editor.inspectAuto(proposal)); XCTAssertEqual(review.sourceRevision, source)
            status(15) { _ = try editor.executeAgentPlan(editor.validateAgentPlan(reviewed)) }
            XCTAssertEqual(try editor.state().revisionCount, count)
            var changedCopy = review.canonicalBytes; changedCopy[0] = 120
            XCTAssertEqual(review.canonicalBytes, reviewed)
            let receipt = try editor.applyAutoFromHuman(review)
            XCTAssertEqual(receipt.planSHA256, SHA256.hash(data: reviewed).map { String(format: "%02x", $0) }.joined())
            XCTAssertEqual(receipt.sourceRevision, source); XCTAssertEqual(try editor.state().revisionID, receipt.resultingRevision)
            XCTAssertEqual(try editor.state().revisionCount, count + 1)
            status(15) { _ = try editor.applyAutoFromHuman(review) }; status(15) { _ = try editor.prepareAutoReview(proposal) }
            try editor.action(2); XCTAssertEqual(try editor.state().revisionID, source)
            try editor.action(3); XCTAssertEqual(try editor.state().revisionID, receipt.resultingRevision)
            try editor.action(2); try editor.select(4); try editor.update([250]); try editor.action(1)
            XCTAssertFalse(try editor.state().canRedo); XCTAssertEqual(try editor.state().revisionCount, count + 2)
        }
    }
    func testWrongOwnerChangedProposalStaleSourceAndPreAdmissionCancellation() throws {
        try fixture { editor in
            let old = try editor.prepareAutoReview(editor.requestAuto(0))
            _ = try editor.requestAuto(2)
            status(9) { _ = try editor.applyAutoFromHuman(old) }
            let current = try editor.prepareAutoReview(editor.requestAuto(0))
            let originalPlan = try editor.validateAgentPlan(current.canonicalBytes)
            try fixture { other in self.status(9) { _ = try other.applyAutoFromHuman(current) } }
            try editor.select(4); try editor.update([100]); try editor.action(1)
            status(9) { _ = try editor.approveAgentPlanFromHuman(originalPlan) }
            status(9) { _ = try editor.applyAutoFromHuman(current) }
            let proposal = try editor.requestAuto(0), cancelled = try editor.prepareAutoReview(proposal)
            let count = try editor.state().revisionCount
            status(15) { _ = try editor.applyAutoFromHuman(cancelled) { try? editor.cancelAutoReview(cancelled) } }
            XCTAssertEqual(try editor.state().revisionCount, count)
            status(15) { _ = try editor.prepareAutoReview(proposal) }
            let fresh = try editor.prepareAutoReview(editor.requestAuto(0)); _ = try editor.applyAutoFromHuman(fresh)
            XCTAssertEqual(try editor.state().revisionCount, count + 1)
        }
    }
    func testDoubleApplyCancelManualUndoRedoAndStopRaces() throws {
        for mode in 0..<5 {
            try fixture { editor in
                try editor.select(4); try editor.update([100]); try editor.action(1)
                let count = try editor.state().revisionCount
                let review = try editor.prepareAutoReview(editor.requestAuto(2))
                XCTAssertEqual(editor.reviewTestControl(0, 1), 0)
                let run = ReviewRun(editor, review), done = DispatchGroup(); done.enter()
                DispatchQueue.global().async { run.run(); done.leave() }
                let end = Date().addingTimeInterval(10)
                while editor.reviewTestControl(1) != 1 && Date() < end { Thread.sleep(forTimeInterval: 0.001) }
                XCTAssertEqual(editor.reviewTestControl(1), 1)
                status(15) { _ = try editor.applyAutoFromHuman(review) }
                status(8) { _ = try editor.requestAuto(0) }
                if mode == 0 { try editor.cancelAutoReview(review) }
                if mode == 1 { try editor.select(4); try editor.update([250]); try editor.action(1) }
                if mode == 2 { try editor.action(2); try editor.action(3) }
                let closed = DispatchGroup()
                if mode == 3 {
                    closed.enter(); DispatchQueue.global().async { run.editor.close(); closed.leave() }
                    let limit = Date().addingTimeInterval(10)
                    while Date() < limit { do { _ = try editor.state() } catch { break }; Thread.sleep(forTimeInterval: 0.001) }
                }
                XCTAssertEqual(editor.reviewTestControl(2), 0); XCTAssertEqual(done.wait(timeout: .now() + 10), .success)
                if mode == 3 { XCTAssertEqual(closed.wait(timeout: .now() + 10), .success) }
                else {
                    XCTAssertEqual(run.status, mode == 0 ? 13 : mode == 4 ? 0 : 9)
                    XCTAssertEqual(try editor.state().revisionCount, count + (mode == 1 || mode == 4 ? 1 : 0))
                    if mode == 4 { try editor.cancelAutoReview(review); XCTAssertEqual(try editor.state().revisionCount, count + 1) }
                    status(15) { _ = try editor.applyAutoFromHuman(review) }
                }
            }
        }
    }
    func testMalformedReceiptCannotBecomeCompletion() throws {
        try fixture { editor in
            var receipt = pixaura_agent_execution_receipt()
            status(14) { _ = try editor.decodeAgentReceipt(receipt) }
            receipt.api_version = 1; receipt.struct_size = 144; receipt.approval_consumed = 1
            withUnsafeMutableBytes(of: &receipt) { bytes in for i in 16..<144 { bytes[i] = 48 }; bytes[112] = 49 }
            XCTAssertNoThrow(try editor.decodeAgentReceipt(receipt))
            receipt.reserved = 1; status(14) { _ = try editor.decodeAgentReceipt(receipt) }; receipt.reserved = 0
            receipt.approval_consumed = 0; status(14) { _ = try editor.decodeAgentReceipt(receipt) }; receipt.approval_consumed = 1
            withUnsafeMutableBytes(of: &receipt) { $0[16] = 255 }; status(14) { _ = try editor.decodeAgentReceipt(receipt) }
            XCTAssertEqual(try editor.state().revisionCount, 1)
        }
    }
}
