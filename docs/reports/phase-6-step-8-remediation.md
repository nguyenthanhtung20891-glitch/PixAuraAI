# Phase 6 Step 8 — certification remediation

Status: REMEDIATION CERTIFIED. Phase 7 NOT STARTED.
Certified SHA: `6c8a493961e98636732c7bc8b7186cf4d8cb952a`.
Foundation 39700953972 completed/SUCCESS 7/7; Native application shells
39700953955 completed/SUCCESS 3/3, both exact SHA, attempt 1. Final Phase 6
closure is separately governed by [the closure report](phase-6-closure.md) and
requires independent final-closure-SHA certification. Historical investigation follows.
Starting main/origin/main were clean and exactly
`8ad0da87420e54d8a9e896aa6d3c536bc88db51c`, before and after fetch.
No old run certifies the remediation commit.

## iOS failure — test harness category D

Native shells 39621695967, iOS job 119296553672, failed with xcodebuild exit 65
in “Build debug/release iOS app and execute Swift/C ABI and UI tests”. Builds,
release scan, Swift macOS/Simulator (35 each) and app tests (7) had succeeded.
UI tests executed two cases, one failed. The downloaded ios-shell-evidence
contains tests.xcresult; Windows cannot run Apple's xcresulttool. Complete job
output identifies ShellUITests.swift:79, not an Agent or native entry point.

After invalid crop rejection and cancellation, terminalGesture completed and
the committed revision count remained 2. At action 7 (Detail), the actionable
predicate found exactly one enabled button. The next independent `button.frame`
lookup performed three resolution attempts, then failed “Timed out while
evaluating UI query”. The tap-start diagnostic never occurred. Thus the failed
operation was the harness's redundant accessibility frame snapshot, before the
requested category tap or associated editing operation. No log establishes an
Agent ownership/handle/status/cancellation/stop/join defect or an app crash as
the cause. XCTest's subsequent crash-report search is teardown, not a crash finding.
The internal reason the Apple accessibility query stalled is not established.

The smallest correction uses `button.tap()` for all category/tool selections,
already used by the same test's ordinary controls and Geometry. XCTest resolves
the actual hittable point and scrolls that specific element into view. Remove
the separate button/window frame reads and hand-computed coordinate event.
Readiness and exact selected-button/control/active Apply+Cancel postconditions
remain required, as do revision, invalid-crop, cancellation, background/reopen,
landscape, undo/redo and persistence coverage. No timeout/retry increase, sleep,
second tap, late-success override or removed scenario. A source regression check
guards this event path and the retained semantic assertions. Runtime confirmation
was supplied by the certified exact-SHA hosted UI gate below; source checks alone
were not used to certify it.

## Sanitizer discrepancy — external reporting category J

Foundation 39621696053 / job 119296554878 now independently reports completed /
SUCCESS, as do all seven jobs and the workflow roll-up. Previous captured job,
check-run and attempt API responses reported queued/null after logs completed.
The full log proves 49/49 tests PASS, ASan negative probe exit 86, UBSan exit 87,
normal ctest return, successful sanitizer-evidence artifact finalization/upload
(artifact 12355616591), checkout post-job cleanup and orphan cleanup ending at
2026-10-21T15:03:35Z. No shell/pipe/background-process hang, job timeout,
dependency/condition/cancellation error or incomplete upload is shown.

This is an observed GitHub Actions reporting inconsistency that subsequently
cleared, not a repository sanitizer failure. GitHub's internal reporting cause
cannot be established from repository-accessible evidence. No runner retry,
sanitizer suppression, continue-on-error, coverage reduction or workflow change
is justified. New certification must observe every required job completed /
SUCCESS; successful logs or a successful roll-up alone remain insufficient.

## Scope, validation and disposition

Only the UI test harness, its source guard and engineering documentation change.
Production code, public ABI, Intent v1, Structured Intent v1, Plan v1, Registry v1,
approval authority, candidate lifecycle, platform security, Schema 3, manual tool
semantics/history and Phase 5 trust remain unchanged. No architecture blocker.
A: retained UI observer failure corrected and independently certified; none unresolved.
B: final Phase 6 closure is separately authorized and requires its own certification;
Phase 7/model/UX/physical certification remain future scope. C: optional research.
Production model UNRESOLVED; U2NETP unapproved/unincorporated/rights unresolved.
No NLP/model/runtime/weights/network/cloud AI/AI UX. CPU VALIDATED, Vulkan/Metal
UNKNOWN. DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED.

Local and exact-SHA execution evidence is recorded after observation in the
engineering receipt and final delivery; no pending gate is PASS.

Pre-push observations: Windows-applicable source 119/119 PASS, focused native
manual/geometry/Agent execution/release-surface gates 7/7 PASS, workflow lint and
git diff --check PASS. Hosted execution observed on the certified SHA: iOS app
7/7 and UI 2/2; Detail action tap and selected/control checks passed. Swift macOS
and Simulator 35/35 each; Android instrumentation 35/35 and both ARM ABIs/app/JVM/
lint PASS. Windows native 46/46; Linux/Apple native 47/47; active ASan/UBSan 49/49,
probes 86/87 and Agent 1548 assertions PASS. Source 119/119 Windows, 161/161 Linux;
release isolation and full retained regression gates PASS. No local Apple execution
is claimed on Windows. No test weakening, skip, optional gate or rerun-until-green.
