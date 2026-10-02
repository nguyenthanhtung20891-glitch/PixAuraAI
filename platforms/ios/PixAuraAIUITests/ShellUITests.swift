import XCTest

final class ShellUITests: XCTestCase {
    func testLaunchCoreAndDestinations() {
        let app = XCUIApplication()
        app.launchArguments = ["--reset-shell-navigation"]
        app.launch()
        XCTAssertTrue(app.staticTexts["Shared core ready · ABI 1"].waitForExistence(timeout: 10))
        for destination in ["Projects", "Settings"] {
            app.buttons["Open \(destination)"].tap()
            XCTAssertTrue(app.navigationBars[destination].waitForExistence(timeout: 5))
            app.buttons["Back to Home"].tap()
        }
        app.buttons["Open Manual editor"].tap()
        XCTAssertTrue(app.navigationBars["Editor"].waitForExistence(timeout: 5))
        XCTAssertTrue(app.staticTexts["Manual session · Editing tools arrive in a later phase."].exists)
    }
}
