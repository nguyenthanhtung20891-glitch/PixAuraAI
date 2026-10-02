import XCTest
import PixAuraCore
@testable import PixAuraAI

final class ShellTests: XCTestCase {
    func testNavigationPreservesSession() {
        let original = ShellState()
        var state = original
        state.openEditor(.assisted)
        state.navigate(.settings)
        state.back()
        XCTAssertEqual(original, ShellState())
        XCTAssertEqual(state.destination, .home)
        XCTAssertEqual(state.mode, .assisted)
        state.openEditor(.ai)
        XCTAssertEqual(state.destination, .editor)
        XCTAssertEqual(state.mode, .ai)
    }

    func testRealCoreContract() {
        XCTAssertEqual(CoreProbe.abiVersion(), 1)
    }

    func testRestorationRejectsUnknownValues() {
        XCTAssertEqual(ShellState.restore(destination: "Import", mode: "unknown"), ShellState())
        XCTAssertEqual(ShellState.restore(destination: "Editor", mode: "AI"),
                       ShellState(destination: .editor, mode: .ai))
    }

    @MainActor func testProbePublishesSuccessWithoutChangingSession() async {
        let model = ShellModel()
        model.openEditor(.manual)
        await model.checkCore()
        XCTAssertEqual(model.probe, "Shared core ready · ABI 1")
        XCTAssertEqual(model.state.destination, .editor)
    }
}
