import XCTest
import Foundation
import Dispatch
@testable import PixAuraCore

private final class AdviceRun: @unchecked Sendable {
    let editor: ManualEditor
    private let lock = NSLock()
    private var result: Int32 = -1
    init(_ editor: ManualEditor) { self.editor = editor }
    func run() {
        var code: Int32 = 0
        do { _ = try editor.requestAdvice(0) } catch { code = (error as? ManualEditorFailure)?.status ?? -1 }
        lock.lock(); result = code; lock.unlock()
    }
    var status: Int32 { lock.lock(); defer { lock.unlock() }; return result }
}

final class AssistExperienceTests: XCTestCase {
    private func fixture(_ body: (ManualEditor, URL) throws -> Void) throws {
        let root = FileManager.default.temporaryDirectory.appendingPathComponent("assist-platform-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: root, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: root) }
        let fixture = try XCTUnwrap(Bundle.module.url(forResource: "decode", withExtension: "json", subdirectory: "Fixtures"))
        let json = try XCTUnwrap(JSONSerialization.jsonObject(with: Data(contentsOf: fixture)) as? [String: [String: Any]])
        let bytes = try XCTUnwrap(json["png"]?["bytes"] as? [UInt8])
        let input = root.appendingPathComponent("input.png"); try Data(bytes).write(to: input)
        let editor = try ManualEditor(root: root, importing: input); defer { editor.close() }
        try body(editor, root)
    }
    private func status(_ code: Int32, _ body: () throws -> Void) {
        XCTAssertThrowsError(try body()) { XCTAssertEqual(($0 as? ManualEditorFailure)?.status, code) }
    }
    func testAllAdviceIsReadOnlyAndNavigationRequiresSeparateManualApply() throws {
        try fixture { editor, root in
            let original = try editor.state().revisionID
            let choices = try editor.adviceChoices(); XCTAssertEqual(choices.count, 11)
            for choice in choices {
                let advice = try editor.requestAdvice(choice.index)
                XCTAssertEqual(try editor.inspectAdvice(advice).values, choice.values)
                XCTAssertEqual(try editor.state().revisionID, original)
                try editor.cancelAdvice(); status(9) { _ = try editor.inspectAdvice(advice) }
            }
            let exposure = try XCTUnwrap(choices.first { editor.tools[$0.toolIndex].toolID == "pixaura.exposure" })
            let advice = try editor.requestAdvice(exposure.index)
            try editor.select(editor.inspectAdvice(advice).toolIndex)
            XCTAssertEqual(try editor.state().values, [0])
            XCTAssertEqual(try editor.state().revisionID, original)
            status(9) { _ = try editor.inspectAdvice(advice) }
            try editor.action(1); XCTAssertEqual(try editor.state().revisionCount, 1)
            try editor.select(exposure.toolIndex); try editor.update([250]); try editor.action(1)
            XCTAssertEqual(try editor.state().revisionCount, 2)
            let beforeUndo = try editor.requestAdvice(exposure.index)
            try editor.action(2); status(9) { _ = try editor.inspectAdvice(beforeUndo) }
            let beforeRedo = try editor.requestAdvice(exposure.index)
            try editor.action(3); status(9) { _ = try editor.inspectAdvice(beforeRedo) }
            editor.close(); editor.close(); status(13) { _ = try editor.requestAdvice(0) }
            let reopened = try ManualEditor(root: root); defer { reopened.close() }
            status(9) { _ = try reopened.inspectAdvice(beforeRedo) }
        }
    }
    func testCancellationBusyAndCloseJoinInFlightRequest() throws {
        try fixture { editor, _ in
            XCTAssertEqual(editor.adviceTestControl(0, 1), 0)
            let finished = expectation(description: "request joined")
            let first = AdviceRun(editor)
            DispatchQueue.global().async {
                first.run(); finished.fulfill()
            }
            let deadline = Date().addingTimeInterval(5)
            while editor.adviceTestControl(1) != 1 && Date() < deadline { Thread.sleep(forTimeInterval: 0.001) }
            XCTAssertEqual(editor.adviceTestControl(1), 1)
            status(8) { _ = try editor.requestAdvice(0) }
            try editor.cancelAdvice()
            XCTAssertEqual(editor.adviceTestControl(2), 0)
            wait(for: [finished], timeout: 5)
            XCTAssertEqual(first.status, 13)
            XCTAssertEqual(try editor.state().revisionCount, 1)
            XCTAssertEqual(editor.adviceTestControl(0, 1), 0)
            let closing = expectation(description: "stop join destroy")
            let second = AdviceRun(editor)
            DispatchQueue.global().async { second.run(); closing.fulfill() }
            let stopDeadline = Date().addingTimeInterval(5)
            while editor.adviceTestControl(1) != 1 && Date() < stopDeadline { Thread.sleep(forTimeInterval: 0.001) }
            XCTAssertEqual(editor.adviceTestControl(1), 1)
            try editor.cancelAdvice()
            let closed = expectation(description: "closed")
            DispatchQueue.global().async { second.editor.close(); closed.fulfill() }
            XCTAssertEqual(editor.adviceTestControl(2), 0)
            wait(for: [closing, closed], timeout: 5)
            XCTAssertEqual(second.status, 13)
        }
    }
    func testMalformedProviderFailureAndResourceFailurePreserveManualEditing() throws {
        let failures: [(UInt32, Int32)] = [(2, 6), (3, 5), (4, 8), (5, 14), (1, 13)]
        for (fault, code) in failures {
            try fixture { editor, _ in
                let revision = try editor.state().revisionID
                XCTAssertEqual(editor.adviceTestControl(3, fault), 0)
                status(code) { _ = try editor.requestAdvice(0) }
                XCTAssertEqual(try editor.state().revisionID, revision)
                let exposure = try XCTUnwrap(editor.tools.firstIndex { $0.toolID == "pixaura.exposure" })
                try editor.select(exposure); try editor.update([250]); try editor.action(1)
                XCTAssertEqual(try editor.state().revisionCount, 2)
            }
        }
    }
}
