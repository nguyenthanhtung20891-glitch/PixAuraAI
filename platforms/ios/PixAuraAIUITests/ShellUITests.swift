import XCTest

final class ShellUITests: XCTestCase {
    func testManualEditorGestureHistoryCancelBackgroundAndReopen() {
        let app = XCUIApplication()
        app.launchArguments = ["--reset-shell-navigation", "--editor-test-image"]
        app.launch()
        func tap(_ label: String) {
            let button = app.buttons[label].firstMatch
            XCTAssertTrue(button.waitForExistence(timeout: 20), label)
            for _ in 0..<3 { if button.isHittable { break }; app.swipeUp() }
            for _ in 0..<5 { if button.isHittable { break }; app.swipeDown() }
            XCTAssertTrue(button.isHittable, label); button.tap()
        }
        func revisions(_ n: Int) {
            XCTAssertTrue(app.staticTexts["Committed revisions: \(n)"].waitForExistence(timeout: 20))
        }
        tap("Open Manual editor"); revisions(1)
        tap("Tone/Color"); tap("Exposure")
        let slider = app.sliders["manual-slider"]
        XCTAssertTrue(slider.waitForExistence(timeout: 10))
        slider.adjust(toNormalizedSliderPosition: 0.6)
        tap("Apply"); revisions(2)
        tap("Undo"); XCTAssertTrue(app.buttons["Redo"].isEnabled)
        tap("Redo"); revisions(2)
        tap("Exposure"); slider.adjust(toNormalizedSliderPosition: 0.4)
        tap("Cancel"); revisions(2)
        tap("Geometry"); tap("Rotate"); tap("90°")
        XCUIDevice.shared.press(.home); app.activate(); revisions(2)
        XCTAssertFalse(app.buttons["Apply"].isEnabled)
        tap("Crop")
        func field(_ id: String, _ value: String) {
            let field = app.textFields[id]; XCTAssertTrue(field.waitForExistence(timeout: 10)); field.tap()
            let old = field.value as? String ?? ""
            field.typeText(String(repeating: XCUIKeyboardKey.delete.rawValue, count: old.count) + value)
        }
        field("width_ppm", "500000"); field("x_ppm", "600000")
        tap("Preview crop")
        XCTAssertTrue(app.staticTexts["editor-error"].waitForExistence(timeout: 10))
        tap("Cancel"); revisions(2)
        tap("Detail"); tap("Blur"); tap("Reset to default"); tap("Apply"); revisions(2)
        tap("Filters/Presets")
        XCTAssertTrue(app.staticTexts["No presets available. The shipping catalog is empty."].exists)
        XCUIDevice.shared.orientation = .landscapeLeft
        tap("Back to Home"); tap("Open Manual editor"); revisions(2)
        XCUIDevice.shared.orientation = .portrait
        app.terminate(); app.launchArguments = ["--reset-shell-navigation"]; app.launch()
        tap("Open Manual editor"); revisions(2)
    }
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
        XCTAssertTrue(app.buttons["Open JPEG or PNG"].exists)
    }
}
