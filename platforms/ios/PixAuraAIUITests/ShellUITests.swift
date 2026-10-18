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
        func terminalGesture() {
            // The revision label already has the expected value on cancel and
            // neutral apply. Observe completion rather than racing the worker's
            // refresh with the next navigation query.
            // Resolve existence and enabled state together. Reading isEnabled
            // after exists can trigger XCTest's element-resolution retries on
            // a replaced accessibility snapshot, exhausting the outer waiter.
            let idle = app.staticTexts.matching(NSPredicate(format: "label == %@", "Editor busy"))
            let inactive = app.buttons.matching(NSPredicate(format: "label == %@ AND enabled == false", "Cancel"))
            let terminal = XCTNSPredicateExpectation(predicate: NSPredicate { _, _ in
                return idle.count == 0 && inactive.count == 1
            }, object: nil)
            let result = XCTWaiter.wait(for: [terminal], timeout: 20)
            XCTAssertEqual(result, .completed)
            if result != .completed {
                XCTFail("Gesture completion timeout\n\(String(app.debugDescription.prefix(16384)))")
            }
        }
        var actionSequence = 0
        func selectControl(_ id: String) -> Bool {
            actionSequence += 1
            let sequence = actionSequence
            // Match attributes in one query result, rather than chaining remote
            // exists/enabled/selected snapshots inside the waiter's predicate.
            func buttonPredicate(selected: Bool) -> NSPredicate {
                NSPredicate(format: "elementType == %d AND (identifier == %@ OR label == %@) AND enabled == true" +
                    (selected ? " AND selected == true" : ""),
                    XCUIElement.ElementType.button.rawValue, id, id)
            }
            func waitForMatches(_ predicate: NSPredicate, count: Int, stage: String) -> Bool {
                let query = app.descendants(matching: .any).matching(predicate)
                let start = ProcessInfo.processInfo.systemUptime
                var polls = 0
                var lastCount = -1
                var longestQuery: TimeInterval = 0
                let expected = XCTNSPredicateExpectation(predicate: NSPredicate { _, _ in
                    let queryStart = ProcessInfo.processInfo.systemUptime
                    // count re-evaluates the query. No element or snapshot is
                    // retained across polling or a category/lifecycle transition.
                    lastCount = query.count
                    longestQuery = max(longestQuery, ProcessInfo.processInfo.systemUptime - queryStart)
                    polls += 1
                    return lastCount == count
                }, object: nil)
                let result = XCTWaiter.wait(for: [expected], timeout: 20)
                let elapsed = ProcessInfo.processInfo.systemUptime - start
                print("PXUI action=\(sequence) id=\(id) stage=\(stage) result=\(result.rawValue) polls=\(polls) matches=\(lastCount)/\(count) elapsed_s=\(elapsed) max_query_s=\(longestQuery)")
                guard result == .completed else {
                    // A later snapshot explains failure; it cannot turn a timed
                    // out wait into success. Bound hierarchy output to 16 KiB.
                    XCTFail("Tool readiness timeout: \(id) stage=\(stage) app_state=\(app.state.rawValue)\n\(String(app.debugDescription.prefix(16384)))")
                    return false
                }
                return true
            }
            guard waitForMatches(buttonPredicate(selected: false), count: 1, stage: "actionable") else { return false }
            let button = app.buttons[id].firstMatch
            // Keep the reviewed upper-quarter target above scrollbars. Existence,
            // a visible event target and semantic readiness are distinct checks.
            let frame = button.frame
            let target = CGPoint(x: frame.minX + frame.width * 0.25, y: frame.minY + frame.height * 0.25)
            guard app.windows.firstMatch.frame.contains(target) else {
                XCTFail("Tool target is off screen: \(id)\n\(String(app.debugDescription.prefix(16384)))"); return false
            }
            print("PXUI action=\(sequence) id=\(id) stage=tap start_s=\(ProcessInfo.processInfo.systemUptime)")
            button.coordinate(withNormalizedOffset: CGVector(dx: 0.25, dy: 0.25)).tap()
            print("PXUI action=\(sequence) id=\(id) stage=tap-complete end_s=\(ProcessInfo.processInfo.systemUptime)")
            let control: NSPredicate
            switch id {
            case "Geometry", "Tone/Color", "Detail":
                let firstTool = ["Geometry": "pixaura.crop", "Tone/Color": "pixaura.exposure", "Detail": "pixaura.blur"][id]!
                control = NSPredicate(format: "elementType == %d AND identifier == %@ AND enabled == true",
                                      XCUIElement.ElementType.button.rawValue, firstTool)
            case "Filters/Presets":
                control = NSPredicate(format: "elementType == %d AND label == %@",
                                      XCUIElement.ElementType.staticText.rawValue,
                                      "No presets available. The shipping catalog is empty.")
            case "pixaura.crop":
                control = NSPredicate(format: "elementType == %d AND identifier == %@ AND enabled == true",
                                      XCUIElement.ElementType.textField.rawValue, "width_ppm")
            case "pixaura.rotate":
                control = NSPredicate(format: "elementType == %d AND label == %@ AND enabled == true",
                                      XCUIElement.ElementType.button.rawValue, "90°")
            default:
                control = NSPredicate(format: "elementType == %d AND identifier == %@ AND enabled == true",
                                      XCUIElement.ElementType.slider.rawValue, "manual-slider")
            }
            // Selected/enabled button AND associated control must coexist in the
            // same query result. No second tap, late-success override or sleep.
            var required = [buttonPredicate(selected: true), control]
            if id.hasPrefix("pixaura.") {
                // Re-selecting the same tool can leave its old selected trait
                // and slider visible while no gesture is active. Require the
                // real core-backed action state as well as those UI controls.
                required.append(NSPredicate(format: "elementType == %d AND (label == %@ OR label == %@) AND enabled == true",
                                            XCUIElement.ElementType.button.rawValue, "Apply", "Cancel"))
            }
            return waitForMatches(NSCompoundPredicate(orPredicateWithSubpredicates: required),
                                  count: id.hasPrefix("pixaura.") ? 4 : 2, stage: "selected-and-control")
        }
        tap("Open Manual editor"); revisions(1)
        guard selectControl("Tone/Color"), selectControl("pixaura.exposure") else { return }
        XCTAssertTrue(app.sliders["manual-slider"].waitForExistence(timeout: 10))
        app.sliders["manual-slider"].adjust(toNormalizedSliderPosition: 0.6)
        tap("Apply"); terminalGesture(); revisions(2)
        tap("Undo"); XCTAssertTrue(app.buttons["Redo"].isEnabled)
        tap("Redo"); revisions(2)
        guard selectControl("pixaura.exposure") else { return }
        app.sliders["manual-slider"].adjust(toNormalizedSliderPosition: 0.4)
        tap("Cancel"); terminalGesture(); revisions(2)
        guard selectControl("Geometry"), selectControl("pixaura.rotate") else { return }
        tap("90°")
        XCUIDevice.shared.press(.home); app.activate()
        XCTAssertTrue(app.wait(for: .runningForeground, timeout: 20))
        revisions(2)
        XCTAssertFalse(app.buttons["Apply"].isEnabled)
        guard selectControl("pixaura.crop") else { return }
        func field(_ id: String, _ value: String) {
            let field = app.textFields[id]; XCTAssertTrue(field.waitForExistence(timeout: 10)); field.tap()
            let old = field.value as? String ?? ""
            field.typeText(String(repeating: XCUIKeyboardKey.delete.rawValue, count: old.count) + value)
        }
        field("width_ppm", "500000"); field("x_ppm", "600000")
        tap("Preview crop")
        XCTAssertTrue(app.staticTexts["editor-error"].waitForExistence(timeout: 10))
        tap("Cancel"); terminalGesture(); revisions(2)
        guard selectControl("Detail"), selectControl("pixaura.blur") else { return }
        tap("Reset to default"); tap("Apply"); terminalGesture(); revisions(2)
        guard selectControl("Filters/Presets") else { return }
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
