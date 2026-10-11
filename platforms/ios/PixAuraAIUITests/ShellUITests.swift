import XCTest

final class ShellUITests: XCTestCase {
    func testManualEditorGestureHistoryCancelBackgroundAndReopen() {
        let app = XCUIApplication()
        app.launchArguments = ["--reset-shell-navigation", "--editor-test-image"]
        app.launch()
        func tap(_ label: String) {
            let button = app.buttons[label].firstMatch
            XCTAssertTrue(button.waitForExistence(timeout: 20), label)
            XCTAssertTrue(button.isEnabled, label)
            // XCTest tap scrolls the specific element into view, including
            // landscape's nested scroll views. Whole-app swipes can target a
            // different scroll view and falsely reject an actionable button.
            button.tap()
        }
        func revisions(_ n: Int) {
            XCTAssertTrue(app.staticTexts["Committed revisions: \(n)"].waitForExistence(timeout: 20))
        }
        func selectControl(_ id: String) {
            let button = app.buttons[id].firstMatch
            let ready = XCTNSPredicateExpectation(
                predicate: NSPredicate(format: "exists == true AND enabled == true"), object: button)
            guard XCTWaiter.wait(for: [ready], timeout: 20) == .completed else {
                XCTFail("Tool is not actionable: \(id)\n\(app.debugDescription)"); return
            }
            // The captured iOS 26 hierarchy places the horizontal scrollbar's
            // accessibility region over the lower half of this button row. Target
            // the visible upper-left quarter, then observe selection (core-backed
            // for tools). The left quarter also avoids the vertical scrollbar;
            // a synthesized tap alone does not prove a gesture began.
            // isHittable uses XCTest's default hit point, which can be covered
            // by that scrollbar even while this upper target remains visible.
            // Check our actual coordinate is on screen, then require selection
            // after a real event; do not substitute a controller/test-only action.
            let frame = button.frame
            let target = CGPoint(x: frame.minX + frame.width * 0.25, y: frame.minY + frame.height * 0.25)
            guard app.windows.firstMatch.frame.contains(target) else {
                XCTFail("Tool target is off screen: \(id)\n\(app.debugDescription)"); return
            }
            button.coordinate(withNormalizedOffset: CGVector(dx: 0.25, dy: 0.25)).tap()
            let selected = XCTNSPredicateExpectation(predicate: NSPredicate(format: "selected == true"), object: button)
            var result = XCTWaiter.wait(for: [selected], timeout: 20)
            if result != .completed && !app.staticTexts["editor-error"].exists {
                // Category changes remove controls and move this row. On loaded
                // simulator hosts a coordinate event can miss that moving target.
                // Retry only an unselected, enabled tool, using a fresh snapshot
                // and a point above the scrollbar. Never retry a core error or
                // proceed without observing the same selection postcondition.
                if button.exists && button.isEnabled && !button.isSelected {
                    button.coordinate(withNormalizedOffset: CGVector(dx: 0.25, dy: 0.1)).tap()
                    let retried = XCTNSPredicateExpectation(predicate: NSPredicate(format: "selected == true"), object: button)
                    result = XCTWaiter.wait(for: [retried], timeout: 20)
                }
            }
            XCTAssertEqual(result, .completed,
                           "Tool did not become selected: \(id)\n\(app.debugDescription)")
        }
        tap("Open Manual editor"); revisions(1)
        selectControl("Tone/Color"); selectControl("pixaura.exposure")
        let slider = app.sliders["manual-slider"]
        XCTAssertTrue(slider.waitForExistence(timeout: 10))
        slider.adjust(toNormalizedSliderPosition: 0.6)
        tap("Apply"); revisions(2)
        tap("Undo"); XCTAssertTrue(app.buttons["Redo"].isEnabled)
        tap("Redo"); revisions(2)
        selectControl("pixaura.exposure"); slider.adjust(toNormalizedSliderPosition: 0.4)
        tap("Cancel"); revisions(2)
        selectControl("Geometry"); selectControl("pixaura.rotate"); tap("90°")
        XCUIDevice.shared.press(.home); app.activate(); revisions(2)
        XCTAssertFalse(app.buttons["Apply"].isEnabled)
        selectControl("pixaura.crop")
        func field(_ id: String, _ value: String) {
            let field = app.textFields[id]; XCTAssertTrue(field.waitForExistence(timeout: 10)); field.tap()
            let old = field.value as? String ?? ""
            field.typeText(String(repeating: XCUIKeyboardKey.delete.rawValue, count: old.count) + value)
        }
        field("width_ppm", "500000"); field("x_ppm", "600000")
        tap("Preview crop")
        XCTAssertTrue(app.staticTexts["editor-error"].waitForExistence(timeout: 10))
        tap("Cancel"); revisions(2)
        selectControl("Detail"); selectControl("pixaura.blur"); tap("Reset to default"); tap("Apply"); revisions(2)
        selectControl("Filters/Presets")
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
