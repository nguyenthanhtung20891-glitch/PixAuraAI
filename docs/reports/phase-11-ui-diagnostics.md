# Phase 11 UI diagnostic lifecycle correction

Predecessor 0ad9d75224973bda08d6eb59e1d2bd37fd77ed74 is not fully certified.
Foundation 43441843381 completed SUCCESS 7/7, attempt 1. Native 43441843496
completed with source/Android successful and iOS failed, attempt 1.

The hosted log contains exactly four XCTest error records, all at the new
teardown screenshot line. Each affected test intentionally terminates its app.
Using testRun.hasSucceeded during teardown incorrectly entered diagnostic capture
before the test result was final. Screenshot capture on the terminated application
then introduced a failure. The four cases are Auto cancellation/provider failure,
bilingual export, review cancellation/mode departure, and preview cancellation.
The hosted app unit suite passed 16/16; UI suite reported 19 tests/4 failures.
No behavioral assertion error is present in that hosted run. The earlier v7
initial-proposal failure remains a separate historical observation whose root
cause was not reproduced; it is not reclassified as a screenshot failure.

The correction uses actual failureCount and checks the app state before attempting
a screenshot. Existing assertions, waits, lifecycle termination and all required
test selections remain intact. First-failure accessibility diagnostics remain.
This changes test infrastructure only; product source, ABI, Schema 3, numerical
behavior, native ownership and GPU admission remain unchanged.

The latest preserved focused log teardown-v9.log ends TEST SUCCEEDED, one affected
lifecycle test with zero failures, 58.825 seconds. The reported transport Bad Request
and exit 1 cannot be attributed to that successful test from the preserved evidence.
The preceding physical log instead failed at app launch because iOS did not trust
the developer certificate. No physical Metal test executed in that attempt. Owner
subsequently confirmed opening the installed app after the online trust request.
No account, certificate, profile or local signing overlay was modified.

Mac-supported source checks pass 205/205. Full affected UI regression and new
exact-SHA hosted certification are required; subsequent observed receipts record
their actual results. Physical XR is wired/paired with Developer Mode enabled,
iOS 18.7.9. Physical Metal, unit and Release benchmark evidence will retain its
own exact source and raw results; no host/Simulator substitution is permitted.

Phase 11 remains IN PROGRESS, not independently accepted or closed. Phase 12
NOT STARTED. Metal/Vulkan trust remains UNKNOWN; DH-APPLE-METAL-01 remains
DEFERRED / NOT WAIVED / NOT SATISFIED pending required independent review.
G8 cohort/thermal/UI targets and 12MP admission conflict, G10 store prerequisites,
and G11 human assistive-technology observations remain acceptance blockers.

## Additional local runtime findings (not reclassified as PASS)

The complete corrected UI suite on iPhone Duo/iOS 27.2 ran 19 tests: 15 passed,
4 failed behavioral assertions, and no diagnostic-capture error occurred.
Two Assist journeys failed their post-Apply revision assertion; editable review
failed field.isHittable before text input; result review failed finding the
manual-mode toolbar after Back to editing. Failure trees/screenshots are retained
in all-ui-v9.xcresult and its exported attachments. The review field was at y1540
inside a nested scroll whose frame extended beyond the 678-point app viewport.
One Assist snapshot retained Exposure 0.0 EV and an offscreen Apply; another had
no active adjustment and revision 1. These observations do not prove native edit
publication failure, and no native or UI fix is invented from them. A conventional
iPhone comparison is required before attributing the failures to the Duo runtime.
They remain open local UI findings, distinct from the four hosted teardown errors.

Conventional iPhone 18 Pro/iOS 27.2 comparison: all four affected behavioral
tests PASS, zero failures, 159.847 seconds of test execution (187.268 seconds
overall). Raw conventional-ui-v9.xcresult/log retained. The existing focused
lifecycle case also passed before this comparison. No source/assertion change
was made between the Duo and conventional runs. This establishes target-specific
behavior, not a universal UI PASS. Duo reachability/toolbar findings remain open
under Phase 11 accessibility/layout investigation; no release qualification is
claimed for that target. Complete hosted workflows still run their unchanged
required source/platform/unit/UI selections on the new implementation SHA.
