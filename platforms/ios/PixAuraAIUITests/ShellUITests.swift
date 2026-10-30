import XCTest

final class ShellUITests: XCTestCase {
    func testPlanInspectionAllElevenExactValuesAndReadOnlyAccessibility() {
        let app = XCUIApplication(); app.launchArguments = ["--reset-shell-navigation", "--editor-test-image"]; app.launch()
        func tap(_ id: String) {
            let query = app.buttons.matching(NSPredicate(format: "(identifier == %@ OR label == %@) AND enabled == true", id, id))
            let ready = XCTNSPredicateExpectation(predicate: NSPredicate { _, _ in query.count == 1 }, object: nil)
            guard XCTWaiter.wait(for: [ready], timeout: 20) == .completed else { XCTFail("Not actionable: \(id)"); return }; query.firstMatch.tap()
        }
        tap("Open Manual editor"); tap("mode-auto")
        let cases = [("brightness","milli_linear = 100 (thousandths_linear_white)"), ("contrast","milli_stops = 250 (thousandths_log2_contrast_slope)"), ("exposure","milli_ev = 500 (thousandths_exposure_stop)"), ("highlights","milli_ev = -500 (thousandths_highlight_exposure_stop)"), ("saturation","milli_ratio = 1200 (thousandths_chroma_multiplier)"), ("shadows","milli_ev = 500 (thousandths_shadow_exposure_stop)"), ("temperature","kelvin = 7000 (daylight_white_kelvin)"), ("blur","milli_strength = 250 (thousandths_fixed_kernel_blend)"), ("sharpen","milli_amount = 250 (thousandths_fixed_kernel_unsharp)"), ("crop","x_ppm = 100000 (millionths_current_extent)"), ("rotate","quarter_turns = 1 (clockwise_quarter_turns)")]
        for (tool, exact) in cases {
            tap("auto-goal"); tap("auto-goal-pixaura.\(tool)"); tap("auto-request")
            let row = app.descendants(matching: .any).matching(identifier: "review-action-0").firstMatch
            XCTAssertTrue(row.waitForExistence(timeout: 20)); XCTAssertTrue(row.label.contains("Action 1 of 1:")); XCTAssertTrue(row.label.contains("pixaura.\(tool)")); XCTAssertTrue(row.label.contains(exact), row.label)
            XCTAssertTrue(app.staticTexts["review-action-detail-0"].exists)
            XCTAssertEqual(row.descendants(matching: .button).count, 0); XCTAssertEqual(row.descendants(matching: .slider).count, 0)
            if tool == "crop" || tool == "rotate" { XCTAssertTrue(row.label.contains("Geometry change")) }
            XCTAssertTrue(app.staticTexts["review-plan-count"].label.contains("1 planned action(s)."))
            XCTAssertTrue(app.buttons["review-preview-request"].exists); XCTAssertTrue(app.buttons["auto-apply"].exists)
            XCTAssertTrue(app.staticTexts["Committed revisions: 1"].exists)
        }
        tap("mode-manual"); XCTAssertFalse(app.buttons["auto-apply"].exists)
    }
    func testPlanInspectionMultiActionOrderUsesExactRawValuesAndSamePreview() {
        let app = XCUIApplication(); app.launchArguments = ["--reset-shell-navigation", "--editor-test-image", "--inspection-test-multi"]; app.launch()
        func tap(_ id: String) {
            let query = app.buttons.matching(NSPredicate(format: "(identifier == %@ OR label == %@) AND enabled == true", id, id))
            let ready = XCTNSPredicateExpectation(predicate: NSPredicate { _, _ in query.count == 1 }, object: nil)
            guard XCTWaiter.wait(for: [ready], timeout: 20) == .completed else { XCTFail("Not actionable: \(id)"); return }; query.firstMatch.tap()
        }
        tap("Open Manual editor"); tap("mode-auto"); tap("auto-goal"); tap("auto-goal-pixaura.exposure"); tap("auto-request")
        let count = app.staticTexts["review-plan-count"]; XCTAssertTrue(count.waitForExistence(timeout: 20)); XCTAssertTrue(count.label.contains("3 planned action(s)."))
        for (i, exact) in ["milli_ev = 101 (thousandths_exposure_stop)","milli_linear = -123 (thousandths_linear_white)","quarter_turns = 1 (clockwise_quarter_turns)"].enumerated() {
            let row = app.descendants(matching: .any).matching(identifier: "review-action-\(i)").firstMatch
            XCTAssertTrue(row.exists); XCTAssertTrue(row.label.contains("Action \(i+1) of 3:")); XCTAssertTrue(row.label.contains(exact), row.label)
        }
        XCTAssertFalse(app.descendants(matching: .any).matching(identifier: "review-action-3").firstMatch.exists)
        XCTAssertTrue(app.staticTexts["review-order-warning"].exists)
        tap("review-preview-request"); XCTAssertTrue(app.staticTexts["review-before"].waitForExistence(timeout: 20)); XCTAssertFalse(app.buttons["review-split"].isEnabled)
        XCTAssertTrue(app.staticTexts["Committed revisions: 1"].exists); XCTAssertTrue(app.buttons["auto-apply"].exists)
        tap("auto-dismiss"); XCTAssertFalse(app.buttons["auto-apply"].exists); XCTAssertTrue(app.staticTexts["Committed revisions: 1"].exists)
    }
    func testReviewPreviewToggleIsNotApplyAndGeometryUsesFit() {
        let app = XCUIApplication(); app.launchArguments = ["--reset-shell-navigation", "--editor-test-image"]; app.launch()
        func tap(_ id: String) {
            let query = app.buttons.matching(NSPredicate(format: "(identifier == %@ OR label == %@) AND enabled == true", id, id))
            let ready = XCTNSPredicateExpectation(predicate: NSPredicate { _, _ in query.count == 1 }, object: nil)
            guard XCTWaiter.wait(for: [ready], timeout: 20) == .completed else { XCTFail("Not actionable: \(id)"); return }; query.firstMatch.tap()
        }
        tap("Open Manual editor"); XCTAssertTrue(app.staticTexts["Committed revisions: 1"].waitForExistence(timeout: 20))
        tap("mode-auto"); tap("auto-goal"); tap("auto-goal-pixaura.exposure"); tap("auto-request")
        tap("review-preview-request"); XCTAssertTrue(app.staticTexts["review-before"].waitForExistence(timeout: 20))
        XCTAssertTrue(app.staticTexts["Committed revisions: 1"].exists)
        tap("review-toggle"); XCTAssertTrue(app.staticTexts["review-after"].exists)
        tap("review-split"); XCTAssertTrue(app.sliders["review-divider"].exists)
        tap("review-reset"); XCTAssertTrue(app.staticTexts["review-before"].exists)
        tap("auto-apply"); XCTAssertTrue(app.staticTexts["auto-completed"].waitForExistence(timeout: 20))
        XCTAssertTrue(app.staticTexts["Committed revisions: 2"].exists)
        tap("mode-manual"); tap("Undo"); tap("Redo")
        tap("mode-auto"); tap("auto-goal"); tap("auto-goal-pixaura.rotate"); tap("auto-request")
        tap("review-preview-request"); XCTAssertTrue(app.staticTexts["review-before"].waitForExistence(timeout: 20))
        XCTAssertFalse(app.buttons["review-split"].isEnabled)
        tap("review-toggle"); XCTAssertTrue(app.staticTexts["review-after"].exists)
        XCTAssertTrue(app.staticTexts["Committed revisions: 2"].exists)
    }
    func testReviewPreviewCancellationFailureAndDepartureKeepManualAvailable() {
        for flag in ["--review-preview-test-pause", "--review-preview-test-failure"] {
            let app = XCUIApplication(); app.launchArguments = ["--reset-shell-navigation", "--editor-test-image", flag]; app.launch()
            func tap(_ id: String) {
                let query = app.buttons.matching(NSPredicate(format: "(identifier == %@ OR label == %@) AND enabled == true", id, id))
                let ready = XCTNSPredicateExpectation(predicate: NSPredicate { _, _ in query.count == 1 }, object: nil)
                guard XCTWaiter.wait(for: [ready], timeout: 20) == .completed else { XCTFail("Not actionable: \(id)"); return }; query.firstMatch.tap()
            }
            tap("Open Manual editor"); XCTAssertTrue(app.staticTexts["Committed revisions: 1"].waitForExistence(timeout: 20))
            tap("mode-auto"); tap("auto-goal"); tap("auto-goal-pixaura.exposure"); tap("auto-request"); tap("review-preview-request")
            if flag.contains("pause") {
                let cancel = app.buttons["review-preview-cancel"]
                XCTAssertTrue(cancel.waitForExistence(timeout: 20)); XCTAssertFalse(app.buttons["auto-apply"].isEnabled)
                // Act on the already resolved semantic element. Re-running the
                // generic enabled/count query let the bounded native rendezvous
                // expire and remove this transient control before tap dispatch.
                cancel.tap(); XCTAssertTrue(app.staticTexts["review-preview-cancelled"].waitForExistence(timeout: 20))
            } else { XCTAssertTrue(app.staticTexts["review-preview-failed"].waitForExistence(timeout: 20)) }
            XCTAssertTrue(app.staticTexts["Committed revisions: 1"].exists)
            tap("mode-manual"); XCTAssertFalse(app.staticTexts["review-after"].exists)
            tap("Back to Home"); tap("Open Manual editor")
            XCTAssertTrue(app.staticTexts["Committed revisions: 1"].waitForExistence(timeout: 20)); XCTAssertFalse(app.buttons["review-toggle"].exists)
            app.terminate()
        }
    }
    func testIntegratedModeBackgroundNavigationAndRestartNeverRestoreReviewAuthority() {
        let app = XCUIApplication(); app.launchArguments = ["--reset-shell-navigation", "--editor-test-image"]; app.launch()
        func tap(_ id: String) {
            let query = app.buttons.matching(NSPredicate(format: "(identifier == %@ OR label == %@) AND enabled == true", id, id))
            let ready = XCTNSPredicateExpectation(predicate: NSPredicate { _, _ in query.count == 1 }, object: nil)
            guard XCTWaiter.wait(for: [ready], timeout: 20) == .completed else { XCTFail("Not actionable: \(id)"); return }; query.firstMatch.tap()
        }
        tap("Open Manual editor")
        XCTAssertTrue(app.staticTexts["Committed revisions: 1"].waitForExistence(timeout: 20))
        for (from,to) in [("manual","assist"),("assist","manual"),("manual","auto"),("auto","manual"),("assist","auto"),("auto","assist")] {
            tap("mode-\(from)"); tap("mode-\(to)")
            XCTAssertTrue(app.staticTexts["Committed revisions: 1"].exists)
            XCTAssertFalse(app.buttons["auto-apply"].exists)
        }
        tap("mode-auto"); tap("auto-goal"); tap("auto-goal-pixaura.exposure"); tap("auto-request")
        XCTAssertTrue(app.buttons["auto-apply"].waitForExistence(timeout: 20))
        XCUIDevice.shared.press(.home); app.activate()
        XCTAssertTrue(app.wait(for: .runningForeground, timeout: 20))
        XCTAssertFalse(app.buttons["auto-apply"].exists)
        XCTAssertTrue(app.staticTexts["Committed revisions: 1"].exists)
        tap("mode-manual"); tap("Back to Home"); tap("Open Manual editor")
        XCTAssertTrue(app.staticTexts["Committed revisions: 1"].waitForExistence(timeout: 20))
        tap("mode-auto"); tap("auto-goal"); tap("auto-goal-pixaura.exposure"); tap("auto-request")
        tap("auto-apply"); XCTAssertTrue(app.staticTexts["auto-completed"].waitForExistence(timeout: 20))
        XCTAssertTrue(app.staticTexts["Committed revisions: 2"].exists)
        app.terminate(); app.launchArguments = ["--reset-shell-navigation"]; app.launch()
        tap("Open Manual editor")
        XCTAssertTrue(app.staticTexts["Committed revisions: 2"].waitForExistence(timeout: 20))
        XCTAssertFalse(app.buttons["auto-apply"].exists); XCTAssertFalse(app.staticTexts["auto-completed"].exists)
        tap("Undo"); tap("Redo"); XCTAssertTrue(app.staticTexts["Committed revisions: 2"].exists)
    }

    func testReviewHumanApplyOneOrdinaryRevisionAndManualUndoRedo() {
        let app = XCUIApplication(); app.launchArguments = ["--reset-shell-navigation", "--editor-test-image"]; app.launch()
        func tap(_ id: String) {
            let query = app.buttons.matching(NSPredicate(format: "(identifier == %@ OR label == %@) AND enabled == true", id, id))
            let ready = XCTNSPredicateExpectation(predicate: NSPredicate { _, _ in query.count == 1 }, object: nil)
            guard XCTWaiter.wait(for: [ready], timeout: 20) == .completed else { XCTFail("Not actionable: \(id)"); return }; query.firstMatch.tap()
        }
        func revisions(_ count: Int) { XCTAssertTrue(app.staticTexts["Committed revisions: \(count)"].waitForExistence(timeout: 20)) }
        func changeExposure() -> Bool {
            tap("pixaura.exposure")
            let selected = app.buttons.matching(NSPredicate(format: "identifier == %@ AND selected == true AND enabled == true", "pixaura.exposure"))
            let slider = app.sliders["manual-slider"], value = app.staticTexts["parameter-value"]
            let ready = XCTNSPredicateExpectation(predicate: NSPredicate { _, _ in
                selected.count == 1 && slider.exists && slider.isEnabled && app.buttons["Apply"].isEnabled
            }, object: nil)
            guard XCTWaiter.wait(for: [ready], timeout: 20) == .completed else { XCTFail("Exposure gesture not ready"); return false }
            // One deliberate gesture, never a retry. XCTest's small 0.55 move
            // previously left the native-bound control at zero on the simulator.
            slider.adjust(toNormalizedSliderPosition: 0.75)
            let received = XCTNSPredicateExpectation(predicate: NSPredicate { _, _ in
                value.exists && value.label.hasPrefix("Exposure: ") && value.label != "Exposure: 0.0 EV" && app.buttons["Apply"].isEnabled
            }, object: nil)
            guard XCTWaiter.wait(for: [received], timeout: 20) == .completed else {
                XCTFail("Slider input did not reach native-bound controls: \(value.exists ? value.label : "missing")"); return false
            }
            return true
        }
        tap("Open Manual editor"); revisions(1)
        tap("Tone/Color"); guard changeExposure() else { return }; tap("Apply"); revisions(2)
        tap("mode-auto"); tap("auto-goal"); tap("auto-goal-pixaura.exposure"); tap("auto-request")
        tap("auto-dismiss"); revisions(2); tap("auto-request")
        XCTAssertTrue(app.staticTexts["auto-exact-values-0"].waitForExistence(timeout: 20))
        XCTAssertTrue(app.staticTexts["auto-exact-values-0"].label.contains("milli_ev = 500"))
        XCTAssertTrue(app.staticTexts["auto-source"].exists)
        tap("auto-apply"); XCTAssertTrue(app.staticTexts["auto-completed"].waitForExistence(timeout: 20)); revisions(3)
        XCTAssertFalse(app.buttons["auto-apply"].exists)
        tap("mode-manual"); tap("Undo"); tap("Redo"); revisions(3)
        guard changeExposure() else { return }; tap("Apply"); revisions(4)
    }
    func testReviewCancelModeDepartureAndFailurePreserveManualEditing() {
        for flag in ["--review-test-pause", "--review-test-failure"] {
            let app = XCUIApplication(); app.launchArguments = ["--reset-shell-navigation", "--editor-test-image", flag]; app.launch()
            func tap(_ id: String) {
                let query = app.buttons.matching(NSPredicate(format: "(identifier == %@ OR label == %@) AND enabled == true", id, id))
                let ready = XCTNSPredicateExpectation(predicate: NSPredicate { _, _ in query.count == 1 }, object: nil)
                guard XCTWaiter.wait(for: [ready], timeout: 20) == .completed else { XCTFail("Not actionable: \(id)"); return }; query.firstMatch.tap()
            }
            tap("Open Manual editor"); XCTAssertTrue(app.staticTexts["Committed revisions: 1"].waitForExistence(timeout: 20))
            tap("mode-auto"); tap("auto-goal"); tap("auto-goal-pixaura.exposure"); tap("auto-request"); tap("auto-apply")
            if flag == "--review-test-pause" {
                tap("auto-execution-cancel"); XCTAssertTrue(app.staticTexts["auto-cancelled"].waitForExistence(timeout: 20))
                tap("auto-request"); tap("auto-apply"); tap("mode-manual"); tap("mode-auto")
                XCTAssertFalse(app.staticTexts["auto-completed"].exists); XCTAssertFalse(app.buttons["auto-apply"].exists)
            } else { XCTAssertTrue(app.staticTexts["auto-error"].waitForExistence(timeout: 20)); XCTAssertFalse(app.staticTexts["auto-completed"].exists) }
            XCTAssertTrue(app.staticTexts["Committed revisions: 1"].exists)
            tap("mode-manual"); tap("Tone/Color"); tap("pixaura.exposure"); app.sliders["manual-slider"].adjust(toNormalizedSliderPosition: 0.6); tap("Apply")
            XCTAssertTrue(app.staticTexts["Committed revisions: 2"].waitForExistence(timeout: 20)); app.terminate()
        }
    }
    func testAutoProposalIsReadOnlyAndModeAbandonmentDoesNotRestoreIt() {
        let app = XCUIApplication(); app.launchArguments = ["--reset-shell-navigation", "--editor-test-image"]; app.launch()
        func tap(_ id: String) {
            let query = app.buttons.matching(NSPredicate(format: "(identifier == %@ OR label == %@) AND enabled == true", id, id))
            let ready = XCTNSPredicateExpectation(predicate: NSPredicate { _, _ in query.count == 1 }, object: nil)
            guard XCTWaiter.wait(for: [ready], timeout: 20) == .completed else { XCTFail("Not actionable: \(id)"); return }; query.firstMatch.tap()
        }
        tap("Open Manual editor"); XCTAssertTrue(app.staticTexts["Committed revisions: 1"].waitForExistence(timeout: 20))
        tap("mode-auto"); tap("auto-goal"); tap("auto-goal-pixaura.exposure"); tap("auto-request")
        tap("auto-dismiss"); XCTAssertTrue(app.staticTexts["auto-rejected"].waitForExistence(timeout: 20))
        XCTAssertTrue(app.staticTexts["Committed revisions: 1"].exists)
        tap("auto-request"); XCTAssertTrue(app.buttons["auto-dismiss"].waitForExistence(timeout: 20))
        for id in ["Apply", "Approve", "Execute", "Run Plan", "Commit"] { XCTAssertFalse(app.buttons[id].exists, id) }
        tap("mode-manual"); tap("mode-auto"); XCTAssertFalse(app.buttons["auto-dismiss"].exists)
        tap("mode-manual"); tap("Tone/Color"); tap("pixaura.exposure")
        app.sliders["manual-slider"].adjust(toNormalizedSliderPosition: 0.6); tap("Apply")
        XCTAssertTrue(app.staticTexts["Committed revisions: 2"].waitForExistence(timeout: 20))
        tap("mode-auto"); tap("auto-request"); tap("auto-dismiss")
        XCTAssertTrue(app.staticTexts["Committed revisions: 2"].exists)
        tap("mode-assist"); XCTAssertTrue(app.buttons["assist-request"].waitForExistence(timeout: 20))
    }
    func testAutoCancellationAndProviderFailureLeaveManualAvailable() {
        for flag in ["--auto-test-pause", "--auto-test-failure"] {
            let app = XCUIApplication(); app.launchArguments = ["--reset-shell-navigation", "--editor-test-image", flag]; app.launch()
            func tap(_ id: String) {
                let query = app.buttons.matching(NSPredicate(format: "(identifier == %@ OR label == %@) AND enabled == true", id, id))
                let ready = XCTNSPredicateExpectation(predicate: NSPredicate { _, _ in query.count == 1 }, object: nil)
                guard XCTWaiter.wait(for: [ready], timeout: 20) == .completed else { XCTFail("Not actionable: \(id)"); return }; query.firstMatch.tap()
            }
            tap("Open Manual editor"); XCTAssertTrue(app.staticTexts["Committed revisions: 1"].waitForExistence(timeout: 20))
            tap("mode-auto"); tap("auto-goal"); tap("auto-goal-pixaura.exposure"); tap("auto-request")
            if flag == "--auto-test-pause" {
                tap("auto-cancel"); XCTAssertTrue(app.staticTexts["auto-cancelled"].waitForExistence(timeout: 20))
                tap("auto-request"); tap("mode-manual"); tap("mode-auto"); XCTAssertFalse(app.buttons["auto-dismiss"].exists)
            } else { XCTAssertTrue(app.staticTexts["auto-error"].waitForExistence(timeout: 20)) }
            tap("mode-manual"); tap("Tone/Color"); tap("pixaura.exposure")
            app.sliders["manual-slider"].adjust(toNormalizedSliderPosition: 0.6); tap("Apply")
            XCTAssertTrue(app.staticTexts["Committed revisions: 2"].waitForExistence(timeout: 20)); app.terminate()
        }
    }
    func testAssistCancellationAndModeDepartureSuppressLateAdvice() {
        let app = XCUIApplication()
        app.launchArguments = ["--reset-shell-navigation", "--editor-test-image", "--assist-test-pause"]
        app.launch()
        func tap(_ id: String) {
            let query = app.buttons.matching(NSPredicate(format: "(identifier == %@ OR label == %@) AND enabled == true", id, id))
            let ready = XCTNSPredicateExpectation(predicate: NSPredicate { _, _ in query.count == 1 }, object: nil)
            guard XCTWaiter.wait(for: [ready], timeout: 20) == .completed else { XCTFail("Not actionable: \(id)"); return }
            query.firstMatch.tap()
        }
        tap("Open Manual editor")
        XCTAssertTrue(app.staticTexts["Committed revisions: 1"].waitForExistence(timeout: 20))
        tap("mode-assist"); tap("assist-goal"); tap("assist-goal-pixaura.exposure")
        tap("assist-request"); tap("assist-cancel")
        XCTAssertTrue(app.staticTexts["assist-cancelled"].waitForExistence(timeout: 20))
        XCTAssertFalse(app.buttons["assist-use"].exists)
        tap("assist-request"); tap("mode-manual")
        XCTAssertTrue(app.staticTexts["Committed revisions: 1"].exists)
        XCTAssertFalse(app.buttons["assist-use"].exists)
        tap("mode-assist")
        XCTAssertFalse(app.buttons["assist-use"].exists)
        tap("mode-manual"); tap("Tone/Color"); tap("pixaura.exposure")
        app.sliders["manual-slider"].adjust(toNormalizedSliderPosition: 0.6)
        tap("Apply")
        XCTAssertTrue(app.staticTexts["Committed revisions: 2"].waitForExistence(timeout: 20))
    }
    func testAssistAdviceDismissNavigationAndExplicitManualApply() {
        let app = XCUIApplication()
        app.launchArguments = ["--reset-shell-navigation", "--editor-test-image"]
        app.launch()
        func tap(_ id: String) {
            let query = app.buttons.matching(NSPredicate(format: "(identifier == %@ OR label == %@) AND enabled == true", id, id))
            let ready = XCTNSPredicateExpectation(predicate: NSPredicate { _, _ in query.count == 1 }, object: nil)
            guard XCTWaiter.wait(for: [ready], timeout: 20) == .completed else { XCTFail("Not actionable: \(id)"); return }
            query.firstMatch.tap()
        }
        func revisions(_ n: Int) {
            XCTAssertTrue(app.staticTexts["Committed revisions: \(n)"].waitForExistence(timeout: 20))
        }
        tap("Open Manual editor"); revisions(1)
        XCTAssertTrue(app.buttons["mode-auto"].isEnabled) // Step 4 proposal-only entry, not approval.
        tap("mode-assist"); tap("assist-goal"); tap("assist-goal-pixaura.exposure")
        tap("assist-request"); tap("assist-dismiss"); revisions(1)
        XCTAssertTrue(app.staticTexts["assist-dismissed"].exists)
        tap("assist-request"); tap("assist-use"); revisions(1)
        XCTAssertTrue(app.staticTexts["assist-notice"].waitForExistence(timeout: 20))
        XCTAssertTrue(app.staticTexts["Exposure: 0.0 EV"].waitForExistence(timeout: 20))
        let slider = app.sliders["manual-slider"]
        XCTAssertTrue(slider.waitForExistence(timeout: 20))
        slider.adjust(toNormalizedSliderPosition: 0.6)
        tap("Apply"); revisions(2)
        tap("Undo"); revisions(2); tap("Redo"); revisions(2)
        tap("mode-assist"); tap("assist-request"); tap("assist-dismiss"); revisions(2)
        tap("mode-manual")
        XCTAssertFalse(app.buttons["assist-use"].exists)
        tap("pixaura.exposure"); tap("Cancel"); revisions(2)
    }
    func testAssistProviderFailureStillAllowsManualEditing() {
        let app = XCUIApplication()
        app.launchArguments = ["--reset-shell-navigation", "--editor-test-image", "--assist-test-failure"]
        app.launch()
        func tap(_ id: String) {
            let query = app.buttons.matching(NSPredicate(format: "(identifier == %@ OR label == %@) AND enabled == true", id, id))
            let ready = XCTNSPredicateExpectation(predicate: NSPredicate { _, _ in query.count == 1 }, object: nil)
            guard XCTWaiter.wait(for: [ready], timeout: 20) == .completed else { XCTFail("Not actionable: \(id)"); return }
            query.firstMatch.tap()
        }
        tap("Open Manual editor")
        XCTAssertTrue(app.staticTexts["Committed revisions: 1"].waitForExistence(timeout: 20))
        tap("mode-assist"); tap("assist-goal"); tap("assist-goal-pixaura.brightness"); tap("assist-request")
        XCTAssertTrue(app.staticTexts["assist-error"].waitForExistence(timeout: 20))
        XCTAssertTrue(app.staticTexts["Committed revisions: 1"].exists)
        tap("mode-manual"); tap("Tone/Color"); tap("pixaura.exposure")
        app.sliders["manual-slider"].adjust(toNormalizedSliderPosition: 0.6)
        tap("Apply")
        XCTAssertTrue(app.staticTexts["Committed revisions: 2"].waitForExistence(timeout: 20))
    }
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
            print("PXUI action=\(sequence) id=\(id) stage=tap start_s=\(ProcessInfo.processInfo.systemUptime)")
            // Let XCTest resolve a hittable point and scroll the specific button
            // into view, as tap(_:) already does. A separate frame/window query
            // after the readiness predicate triggered AX snapshot retries after
            // crop cancellation (39621695967), before any tap was attempted.
            // The postcondition below still proves actual semantic activation;
            // existence or a coordinate event alone cannot satisfy this helper.
            button.tap()
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
