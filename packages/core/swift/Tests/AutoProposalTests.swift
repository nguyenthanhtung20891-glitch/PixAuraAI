import XCTest
import Foundation
import Dispatch
@testable import PixAuraCore

private final class AutoRun: @unchecked Sendable {
    let editor: ManualEditor
    private let lock = NSLock()
    private var result: Int32 = -1
    init(_ editor: ManualEditor) { self.editor = editor }
    func run() {
        var code: Int32 = 0
        do { _ = try editor.requestAuto(0) } catch { code = (error as? ManualEditorFailure)?.status ?? -1 }
        lock.lock(); result = code; lock.unlock()
    }
    var status: Int32 { lock.lock(); defer { lock.unlock() }; return result }
}
final class AutoProposalTests: XCTestCase {
    private func fixture(_ body: (ManualEditor) throws -> Void) throws {
        let root = FileManager.default.temporaryDirectory.appendingPathComponent("auto-platform-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: root, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: root) }
        let fixture = try XCTUnwrap(Bundle.module.url(forResource: "decode", withExtension: "json", subdirectory: "Fixtures"))
        let json = try XCTUnwrap(JSONSerialization.jsonObject(with: Data(contentsOf: fixture)) as? [String: [String: Any]])
        let bytes = try XCTUnwrap(json["png"]?["bytes"] as? [UInt8])
        let input = root.appendingPathComponent("input.png"); try Data(bytes).write(to: input)
        // The assertions below also exercise Phase 6 approval-eligibility
        // validation, which requires the established capability/policy owner.
        let capabilities = AppleCapabilityOwner(signals: TestAppleSignals()); defer { capabilities.close() }
        let editor = try ManualEditor(root: root, importing: input, capability: capabilities.snapshot())
        defer { editor.close() }; try body(editor)
    }
    private func status(_ code: Int32, _ body: () throws -> Void) {
        XCTAssertThrowsError(try body()) { XCTAssertEqual(($0 as? ManualEditorFailure)?.status, code) }
    }
    func testAllGoalsCanonicalPlansAreInertAndBoundToTheirOwner() throws {
        try fixture { editor in
            let revision = try editor.state().revisionID
            for choice in try editor.adviceChoices() {
                let proposal = try editor.requestAuto(choice.index)
                let bytes = try editor.inspectAuto(proposal)
                XCTAssertEqual(proposal.providerID, "reference.assistance.v1")
                XCTAssertTrue(proposal.proposalID.hasPrefix("auto."))
                XCTAssertEqual(proposal.actions.count, 1)
                XCTAssertEqual(proposal.actions[0].values, choice.values)
                XCTAssertEqual(try editor.validateAgentPlan(bytes).canonicalBytes, bytes)
                status(15) { _ = try editor.executeAgentPlan(editor.validateAgentPlan(bytes)) }
                XCTAssertEqual(try editor.state().revisionID, revision)
                try fixture { other in self.status(9) { _ = try other.inspectAuto(proposal) } }
                try editor.cancelAuto(); status(9) { _ = try editor.inspectAuto(proposal) }
                let repeated = try editor.requestAuto(choice.index)
                XCTAssertEqual(repeated.providerID, "reference.assistance.v1")
                XCTAssertEqual(try editor.inspectAuto(repeated), bytes)
                try editor.cancelAuto()
            }
            let p = try editor.requestAuto(0)
            try editor.select(4); try editor.update([250]); try editor.action(1)
            status(9) { _ = try editor.inspectAuto(p) }
            let undo = try editor.requestAuto(0); try editor.action(2); status(9) { _ = try editor.inspectAuto(undo) }
            let redo = try editor.requestAuto(0); try editor.action(3); status(9) { _ = try editor.inspectAuto(redo) }
            XCTAssertEqual(try editor.state().revisionCount, 2)
        }
    }
    func testCancelSourceBusyStopAndFailureNeverAcquireEditingAuthority() throws {
        for mode in 0..<5 {
            try fixture { editor in
                XCTAssertEqual(editor.autoTestControl(0, UInt32(mode % 3)), 0)
                let run = AutoRun(editor), done = DispatchGroup(); done.enter()
                DispatchQueue.global().async { run.run(); done.leave() }
                let deadline = Date().addingTimeInterval(5)
                while editor.autoTestControl(1) != 1 && Date() < deadline { Thread.sleep(forTimeInterval: 0.001) }
                XCTAssertEqual(editor.autoTestControl(1), 1)
                status(8) { _ = try editor.requestAuto(0) }; status(8) { _ = try editor.requestAdvice(0) }
                if mode == 1 { try editor.select(4); try editor.update([250]); try editor.action(1) }
                else { try editor.cancelAuto() }
                let closed = DispatchGroup()
                if mode == 4 { closed.enter(); DispatchQueue.global().async { run.editor.close(); closed.leave() } }
                XCTAssertEqual(editor.autoTestControl(2), 0)
                XCTAssertEqual(done.wait(timeout: .now() + 5), .success)
                XCTAssertEqual(run.status, mode == 1 ? 9 : 13)
                if mode == 4 { XCTAssertEqual(closed.wait(timeout: .now() + 5), .success) }
                else { XCTAssertEqual(try editor.state().revisionCount, mode == 1 ? 2 : 1) }
            }
        }
        for fault in UInt32(1)...UInt32(12) {
            try fixture { editor in
                XCTAssertEqual(editor.autoTestControl(3, fault), 0)
                XCTAssertThrowsError(try editor.requestAuto(0))
                XCTAssertEqual(try editor.state().revisionCount, 1)
                try editor.select(4); try editor.update([250]); try editor.action(1)
                XCTAssertEqual(try editor.state().revisionCount, 2)
            }
        }
    }
}
