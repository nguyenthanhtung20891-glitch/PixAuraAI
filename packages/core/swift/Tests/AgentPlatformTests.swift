import XCTest
import Foundation
import Dispatch
@testable import PixAuraCore

private final class AgentRun: @unchecked Sendable {
    let editor: ManualEditor, plan: ManualEditor.AgentPlan, approval: ManualEditor.AgentApproval
    private let lock = NSLock()
    private var result: Int32 = -100
    init(_ editor: ManualEditor, _ plan: ManualEditor.AgentPlan, _ approval: ManualEditor.AgentApproval) {
        self.editor = editor; self.plan = plan; self.approval = approval
    }
    func run() {
        var code: Int32 = 0
        do { _ = try editor.executeAgentPlan(plan, approval: approval) }
        catch { code = (error as? ManualEditorFailure)?.status ?? -1 }
        lock.lock(); result = code; lock.unlock()
    }
    var status: Int32 { lock.lock(); defer { lock.unlock() }; return result }
}
final class AgentPlatformTests: XCTestCase {
    private func fixture(_ body: (ManualEditor, URL) throws -> Void) throws {
        let root = FileManager.default.temporaryDirectory.appendingPathComponent("agent-platform-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: root, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: root) }
        let fixture = try XCTUnwrap(Bundle.module.url(forResource: "decode", withExtension: "json", subdirectory: "Fixtures"))
        let json = try XCTUnwrap(JSONSerialization.jsonObject(with: Data(contentsOf: fixture)) as? [String: [String: Any]])
        let bytes = try XCTUnwrap(json["png"]?["bytes"] as? [UInt8])
        let input = root.appendingPathComponent("input.png"); try Data(bytes).write(to: input)
        let capabilities = AppleCapabilityOwner(signals: TestAppleSignals()); defer { capabilities.close() }
        let editor = try ManualEditor(root: root, importing: input, capability: capabilities.snapshot())
        defer { editor.close() }; try body(editor, root)
    }
    private func plan(_ editor: ManualEditor, _ value: Int = 100) throws -> ManualEditor.AgentPlan {
        let binding = String(decoding: try editor.agentBinding().dropLast(), as: UTF8.self)
        let input = "{\"binding\":\(binding),\"goal_id\":\"adjust.exposure\",\"parameters\":{\"milli_ev\":\(value)},\"structured_intent_version\":1}\n"
        let output = try editor.referenceAgentPlan(Data(input.utf8))
        let expected = "{\"actions\":[{\"action_index\":0,\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":\(value)},\"tool_id\":\"pixaura.exposure\"}],\"binding\":\(binding),\"plan_version\":1}\n"
        XCTAssertEqual(output.canonicalBytes, Data(expected.utf8))
        return output
    }
    private func status(_ expected: Int32, _ body: () throws -> Void) {
        XCTAssertThrowsError(try body()) { XCTAssertEqual(($0 as? ManualEditorFailure)?.status, expected) }
    }
    func testHumanAuthorityExactPlanRevocationReplayAndOrdinaryHistory() throws {
        try fixture { editor, root in
            let original = try editor.state().revisionID, p = try plan(editor)
            XCTAssertEqual(try editor.validateAgentPlan(p.canonicalBytes).canonicalBytes, p.canonicalBytes)
            status(15) { _ = try editor.executeAgentPlan(p) }
            let a = try editor.approveAgentPlanFromHuman(p); defer { a.close() }
            XCTAssertEqual(try editor.state().revisionCount, 1)
            status(15) { _ = try editor.executeAgentPlan(plan(editor, 200), approval: a) }
            let receipt = try editor.executeAgentPlan(p, approval: a)
            XCTAssertEqual(receipt.sourceRevision, original); XCTAssertEqual(receipt.planSHA256.count, 64)
            XCTAssertEqual(try editor.state().revisionCount, 2)
            status(15) { _ = try editor.executeAgentPlan(p, approval: a) }
            try editor.action(2); XCTAssertEqual(try editor.state().revisionID, original)
            try editor.action(3); XCTAssertEqual(try editor.state().revisionID, receipt.resultingRevision)
            let fresh = try plan(editor), revoked = try editor.approveAgentPlanFromHuman(fresh)
            try editor.revokeAgentApproval(revoked)
            status(15) { _ = try editor.executeAgentPlan(fresh, approval: revoked) }; revoked.close(); revoked.close()
            try editor.select(1); try editor.update([100]); try editor.action(1)
            XCTAssertEqual(try editor.state().revisionCount, 3)
            editor.close(); editor.close()
            let reopened = try ManualEditor(root: root); defer { reopened.close() }
            XCTAssertEqual(try reopened.state().revisionCount, 3)
            status(13) { _ = try editor.executeAgentPlan(p, approval: a) }
        }
    }
    func testMalformedAndCrossContextFailClosed() throws {
        try fixture { editor, _ in
            for bytes in [Data(), Data([0xff]), Data("{}\n".utf8), Data(repeating: 120, count: 8193)] {
                XCTAssertThrowsError(try editor.validateAgentPlan(bytes))
                XCTAssertThrowsError(try editor.referenceAgentPlan(bytes))
            }
            let p = try plan(editor), a = try editor.approveAgentPlanFromHuman(p); defer { a.close() }
            let binding = String(decoding: try editor.agentBinding().dropLast(), as: UTF8.self)
            let prose = "{\"binding\":\(binding),\"goal\":\"make this photo brighter\",\"intent_version\":1,\"scope\":\"document\",\"type\":\"edit\"}\n"
            status(5) { _ = try editor.referenceAgentPlan(Data(prose.utf8)) }
            try fixture { other, _ in
                status(9) { _ = try other.validateAgentPlan(p.canonicalBytes) }
                status(3) { _ = try other.executeAgentPlan(plan(other), approval: a) }
            }
            XCTAssertEqual(try editor.state().revisionCount, 1)
        }
    }
    func testRealOwnerCancellationBusyManualUndoAndStopFences() throws {
        for mode in 0..<7 {
            try fixture { editor, _ in
                if mode == 2 { try editor.select(1); try editor.update([50]); try editor.action(1) }
                let before = try editor.state().revisionCount, p = try plan(editor)
                let a = try editor.approveAgentPlanFromHuman(p), b = try editor.approveAgentPlanFromHuman(p)
                defer { a.close(); b.close() }
                XCTAssertEqual(editor.agentTestControl(0, mode == 5 ? 3 : 1), 0)
                let run = AgentRun(editor, p, a), done = DispatchGroup(); done.enter()
                DispatchQueue.global().async { run.run(); done.leave() }
                let deadline = Date().addingTimeInterval(10)
                while editor.agentTestControl(1) != 1 && Date() < deadline { Thread.sleep(forTimeInterval: 0.001) }
                XCTAssertEqual(editor.agentTestControl(1), 1)
                status(8) { _ = try editor.executeAgentPlan(p, approval: b) }
                if mode == 0 || mode == 5 { try editor.cancelAgentExecution() }
                if mode == 1 { try editor.select(1); try editor.update([50]); try editor.action(1) }
                if mode == 2 { try editor.action(2); try editor.action(3) }
                if mode == 3 { a.close() }
                let closeDone = DispatchGroup()
                if mode == 4 {
                    // close closes admission and stops before waiting for this call.
                    closeDone.enter(); DispatchQueue.global().async { run.editor.close(); closeDone.leave() }
                    let until = Date().addingTimeInterval(10)
                    while Date() < until {
                        do { _ = try editor.state() } catch { break }
                        Thread.sleep(forTimeInterval: 0.001)
                    }
                }
                XCTAssertEqual(editor.agentTestControl(2), 0)
                XCTAssertEqual(done.wait(timeout: .now() + 10), .success)
                if mode == 4 { XCTAssertEqual(closeDone.wait(timeout: .now() + 10), .success) }
                else {
                    XCTAssertEqual(run.status, mode == 0 || mode == 5 ? 13 : mode == 3 || mode == 6 ? 0 : 9)
                    if mode == 6 { try editor.cancelAgentExecution() } // publication won; late cancel cannot undo.
                    XCTAssertEqual(try editor.state().revisionCount, before + (mode == 1 || mode == 3 || mode == 6 ? 1 : 0))
                    if mode == 0 {
                        status(15) { _ = try editor.executeAgentPlan(p, approval: a) }
                        _ = try editor.executeAgentPlan(p, approval: b) // BUSY did not consume b; cancellation is nonsticky.
                    }
                }
            }
        }
    }
}
