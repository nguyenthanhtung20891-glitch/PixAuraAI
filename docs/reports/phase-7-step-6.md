# Phase 7 Step 6 � Cross-Platform Lifecycle & Interaction Integration

<!-- phase7-step6-current-authority -->
## Phase 7 Step 6 - current authority

Phase 6 CLOSED / COMPLETE / FULL PASS. Phase 7 AUTHORIZED / IN PROGRESS.
Step 1 COMPLETE / FULL PASS. Step 2 COMPLETE / FULL PASS. Step 3 COMPLETE / FULL PASS.
Step 4 COMPLETE / FULL PASS. Step 5 COMPLETE / FULL PASS.
Certified Step 5 closure: `eef23391fb18c0c9b30cefc632c7a86126e09a60`.
Foundation 40152878375 SUCCESS 7/7; Native application shells 40152878474 SUCCESS
3/3, completed exact SHA, attempt 1, every required job individually SUCCESS.
Step 6 COMPLETE / FULL PASS.
Implementation certification SHA: `8955c81cbccce3f4e10b0fea4881425f1b2b607b`.
Foundation 40196770513 SUCCESS 7/7.
Native application shells 40196770526 SUCCESS 3/3.
Both completed exact SHA, attempt 1; every required job individually SUCCESS.
This separate documentation-only closure commit requires independent full
closure-SHA certification by both workflows. Until observed completion:
FINAL STEP 6 CLOSURE CERTIFICATION PENDING. The final engineering receipt/delivery
records its distinct SHA and workflow evidence without self-reference or old-SHA
substitution. No unobserved job is PASS.
MANUAL / AI ASSIST / AI AUTO-EDIT share native document/source/history authority.
All six mode transitions abandon ephemeral AI state without edits or approval.
Explicit HUMAN Apply, exact Plan binding, single-use approval, private candidate,
source serial/epoch CAS and cancellation/publication fencing remain unchanged.
Application admission includes queued validation/completion and pending cancellation.
Late callbacks cannot resurrect review or transfer authority to another owner.
Stop -> join ALL calls -> destroy (ADR 0009); no persistent AI authority.
Publication wins or cancellation wins under existing native rules; no automatic undo,
retry, replan or reapproval. Refresh ordinary history after every Apply outcome.
Manual first-class; Assist Use suggestion navigation-only. No new public ABI.
Intent/Structured Intent/Plan/Registry/Envelope/Provider/reference planner,
AI Assist/Auto Proposal/Review Approval v1, approval/candidate, Schema 3,
manual/history and Phase 5 trust unchanged. Production model UNRESOLVED.
U2NETP unapproved / unincorporated / rights unresolved.
CPU VALIDATED; Vulkan/Metal UNKNOWN. No model/runtime/weights/NLP/cloud/network AI.
DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED; hosted Apple software only.
Phase 7 Steps 7-8 NOT STARTED. Phase 8 NOT STARTED. No Step 9.
Both full implementation-SHA and separate closure-SHA workflows required,
every required job completed/SUCCESS. Full retained native/platform/source,
active ASan/UBSan, release isolation, workflow lint and diff checks required.
STOP after Step 6. Do not implement Step 7 or Step 8.
See docs/contracts/ai-interaction-lifecycle-integration-v1.md and docs/reports/phase-7-step-6.md.
<!-- phase7-step6-history -->

Earlier authority below is historical at its certified freeze time.

## Purpose, baseline and audit

Certified Step 5 closure eef23391fb18c0c9b30cefc632c7a86126e09a60 verified clean main,
HEAD == origin/main after fetch. Foundation 40152878375 SUCCESS 7/7; Native shells
40152878474 SUCCESS 3/3, exact SHA, attempt 1. Phase 6 and Steps 1-5 remain frozen.
Audited product/architecture/decisions/quality/roadmap, ADR 0009, Steps 1-5
contracts and reports, native approval/candidate/provider boundaries, both platform
wrappers, UI/session models, screen disposal/background hooks and persistence.

## Architecture and ownership

Existing fixed command, advice and preview lanes remain. UI/model owns ephemeral
mode/request/review/receipt presentation only. Shared native owns source serial,
ABA protection, context identity, approval, candidate and repository epoch CAS.
No C ABI, production C++, persistence, provider/model or editing tool changes.
All six directed mode edges preserve one document/history. Screen departure closes
native owner; background interrupts ephemeral work without automatic undo. Native
stop then wrapper join precedes destroy. Old results are epoch/owner filtered.

## A findings and narrow fixes

Android previously released providerActive on the advice lane before command-lane
validation/completion drained. A fresh interaction could overlap older cleanup and
have its loading state cleared. Keep provider admission occupied until queued
validation and presentation complete, releasing it last. A deterministic test holds
the existing command lane, resumes native work and proves admission stays occupied.

Both platforms must reject new provider/Apply work while a queued cancellation is
pending. Android gates on existing cancellation flags; Apple checks its existing
cancellation flags under the same worker lock as provider admission. This prevents
late cancellation from targeting a fresh interaction. No native cancellation or
approval-consumption rule changes; cancellation remains nonsticky.

Apple observable-model destruction additionally schedules close on its existing
command lane even if no SwiftUI onDisappear event is delivered. The closing worker
is captured independently, not the dying model, and native stop/join remains off
the UI lane. Weak model callbacks cannot regain ownership. Repeated close is safe.

## Integration and evidence strategy

Android semantic Compose tests cover all six mode edges, review abandonment,
close/reopen before and after publication, cross-owner review rejection and ordinary
undo/redo. Retained tests cover paused Advice/Auto/Apply teardown, recreation,
manual/undo/redo races, double Apply, provider failure, cancellation and no history.
Apple app-model tests cover all six edges, close/reopen, one-shot Apply, normal
history and no review/receipt restoration. Semantic XCTest covers background,
navigation departure, process restart, ordinary persisted revision and undo/redo.
Retained Swift native checkpoint tests certify cancel/publication orderings,
manual/undo/redo ABA and stop/join. No timing sleep is used as correctness evidence
in the new tests; existing rendezvous and queue barriers establish ordering.

## Security, persistence, failures and parity

No approval/candidate capability is exposed to provider. Forged/wrong owner/context
review, stale source and replay remain rejected by existing native authority.
Unknown status is failure; malformed receipt cannot create completion. Existing
Apply cleanup refreshes authoritative history even after UI abandonment or transport
failure; no automatic retry/reapproval/replan. Publication-winning result remains
ordinary history, never undone because UI departed. Pending AI authority never
enters Bundle/SavedStateHandle/UserDefaults/Schema 3/files. Only existing project
locator and ordinary durable history reopen. Release scans remain mandatory.

## A/B/C and boundaries

A: admission, cancellation isolation, observable-owner destruction and compilation
findings resolved. Certified implementation evidence follows; no unresolved A
finding or architecture blocker remains.
B: Step 7 production intelligence admission, Step 8 closure and physical hardware
certification. C: optional presentation polish. None implemented here.
Production model UNRESOLVED; U2NETP unapproved/unincorporated/rights unresolved.
CPU VALIDATED; Vulkan/Metal UNKNOWN. DH-APPLE-METAL-01 DEFERRED / NOT WAIVED /
NOT SATISFIED. No model/runtime/weights/NLP/cloud/network AI. Phase 8 NOT STARTED.

## Validation and exact-SHA certification

Implementation certification is recorded below; separate closure certification
requires both full workflows again. The retained requirements include source,
native Windows/Linux/Apple/Android, Android app/instrumentation, Swift/macOS and
Simulator/iOS app/UI, active ASan/UBSan, release isolation, workflow lint and diff
checks required. Exact changed files and observed evidence will be recorded below.

## Observed local validation (before CI)

Windows native 51/51 PASS (documented Zig fallback). Windows retained source
127/127 PASS; Linux full source 169/169 PASS, zero skipped. Android strict
debug/release builds, both ARM ABIs, JVM tests, lint and instrumentation compile
PASS. Complete emulator suite 46/46 PASS, zero failures/errors/skips; repeated
after strengthening the pending-cancellation barrier, again 46/46 PASS. Actual
production release library scans PASS. Emulator GPU evidence is TRANSPORT_TEST_ONLY.
Workflow lint and diff checks PASS. No tests weakened/skipped/optionalized; no
rerun-until-green, sanitizer suppression or timeout inflation.

Local WSL native was 50/52; active sanitizers 52/54, NOT full PASS. Both failed
storage_test/storage_android_test on Windows DrvFS (9p, uid/gid 0, no metadata),
with TMPDIR confined to repository build/tmp. The unwritable-directory test's
unprivileged child did not get the required DAC denial. An isolated probe proved
chmod(0500) still reports 0777 and uid 65534 can write. The probe cleaned its own
repository-only files. Both ASan/UBSan negative instrumentation probes passed
(exit 86/87), with no sanitizer defect reported. No native storage/test changes
were made to conceal the filesystem limitation. Complete CI Linux/sanitizer jobs
on their supported filesystem remain mandatory before certification.

Apple native/Swift/app/UI cannot execute on this Windows host and are pending CI.
The new Apple model teardown uses the existing worker lane and is covered by a
self-contained model-destruction test plus semantic background/navigation/restart UI.

## Exact changed-file inventory

- `AGENTS.md`
- `ARCHITECTURE.md`
- `DECISIONS.md`
- `QUALITY_GATES.md`
- `ROADMAP.md`
- `docs/contracts/ai-interaction-lifecycle-integration-v1.md`
- `docs/plans/phase-7-roadmap.md`
- `docs/reports/phase-7-step-6.md`
- `platforms/android/app/src/androidTest/java/ai/pixaura/app/AutoReviewTest.kt`
- `platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorModel.kt`
- `platforms/ios/PixAuraAI/ManualEditorModel.swift`
- `platforms/ios/PixAuraAITests/ShellTests.swift`
- `platforms/ios/PixAuraAIUITests/ShellUITests.swift`
- `tests/agent-execution.test.mjs`

## Initial certification failure and minimal remediation

Initial implementation d0dfc55785e2d61f699cf8ca823cab68b790891e is NOT certified.
Foundation 40187407865 and Native shells 40187407855 are attempt 1. Native shells
failed iOS job 121151020273 at app compilation: ManualEditorModel.swift:361,
capture of non-Sendable EditorWorker in the deinitializer's @Sendable queue closure.
This is an adapter concurrency compilation defect, not a native authority failure.
Move queue handoff into EditorWorker.closeAfterOwnerRelease, using its existing
nonisolated commands.async pattern. The deinitializer only invokes that method;
no new closure crosses its isolation context. No unchecked Sendable, suppression,
warning weakening, teardown omission or test weakening. The model-destruction test
is retained unchanged. New SHA requires both complete workflows afresh.

On the initial SHA CI Linux native passed 52/52 (both local DrvFS failures passed),
active sanitizers passed 54/54 with probes 86/87, Android instrumentation 46 tests
SUCCESS, Apple boundary SUCCESS. Those results do not certify newer code.

## Test-harness compilation remediation

725eebbffeca8b01ec5f4126caad04b906eed14c is NOT certified: Foundation 40188240245
completed SUCCESS 7/7, but Native shells 40188240258 failed iOS test compilation.
The production debug/release app build passed, confirming the worker queue handoff
fix. ShellTests.swift:72's local weak var triggered a never-mutated diagnostic,
promoted by unchanged warnings-as-errors. Replace the local weak variable with a
small test-only ModelLifetimeProbe holding a weak property. The probe remains
strongly owned while the observed model must deallocate; the same nil assertion
and native closed-state/barrier assertions remain. No fake mutation, suppression,
warning weakening or production-code change. Full new-SHA certification required.

## Historical implementation certification and first closure attempt

Implementation **1433a45b5b560c6834eed7328440c130686c0943** is certified:
Foundation boundaries **40189453335**, attempt 1, completed / SUCCESS, **7/7**
required jobs individually completed / SUCCESS. Native application shells
**40189453294**, attempt 1, completed / SUCCESS, **3/3** required jobs individually
completed / SUCCESS. Both have that exact head SHA. No older SHA substitutes.

Retained CI native host, both Android ARM ABIs, source, workflow lint, release
objects/binaries, Android app/JVM/lint/instrumentation, Swift/macOS/Simulator and
iOS app/UI gates PASS. Active ASan/UBSan **54/54**, actual probes 86/87 confirmed.
Swift/macOS and Simulator **44/44** each. iOS app tests **9/9**, UI tests **10/10**,
zero failures. Both new model tests passed, including observable destruction with
no view callback. The semantic background/navigation/restart UI test passed.
Android full instrumentation **46 tests**, SUCCESS. Local DrvFS permission failures
remain documented limitations; CI Linux storage/native and sanitizer gates pass
on their supported filesystem. They were not weakened or skipped.

No unresolved A finding. B: future Step 7 intelligence admission, Step 8 closure,
and physical provider certification; C: optional polish. None implemented here.
All frozen authority, Schema 3, shared Manual/history and trust remain unchanged.
No public ABI or native production editing implementation changed.

This separate closure commit changes only seven Markdown authority/report files.
It requires both complete exact-closure-SHA workflows independently again. The
final engineering receipt/delivery records that distinct SHA/run evidence without
self-reference or reuse of implementation certification. Until then final closure
certification is pending. Phase 7 Steps 7-8 and Phase 8 NOT STARTED.

## Closure certification failure: verified no-op UI input

Documentation-only closure candidate 9e86e29f568c4cfe23aad700af34061eb3bf7e1f
is NOT certified. Foundation 40191429450 completed SUCCESS 7/7; Native shells
40191429388 failed iOS UI job 121163057256, attempt 1. All nine app tests passed;
the failure was testReviewHumanApplyOneOrdinaryRevisionAndManualUndoRedo.
Its first manual revision-count assertion failed, followed by four cascading counts.
Auto completion itself was observed. Other nine UI tests passed.

The retained ios-shell-evidence artifact was downloaded and bounded zstd records
inspected without an external dependency. Accessibility snapshots at 12:31:57-
12:32:09 show manual-slider value 0, parameter-value Exposure: 0.0 EV and revision
count 1 after XCTest's normalized 0.55 gesture. The following Apply was a no-op;
manual editing correctly did not create a revision. This is a missing input
postcondition in the Step 5 UI harness, not a native commit/approval defect.

Require selected/enabled Exposure and live controls first, issue one deliberate
0.75 slider gesture, then require changed native-bound parameter text and enabled
Apply before tapping. Keep all original revision/history assertions. Stop on a
failed input precondition instead of cascading. No retry, coordinate action,
timeout increase, production change or assertion weakening. The corresponding
source guard retains these preconditions. Current Step 6 returns IN PROGRESS;
new implementation and separate closure SHAs require both full workflows afresh.

## Final certified implementation and replacement closure

Final implementation **8955c81cbccce3f4e10b0fea4881425f1b2b607b** is certified.
Foundation boundaries **40196770513**, attempt 1, completed / SUCCESS, **7/7**
required jobs individually completed / SUCCESS. Native application shells
**40196770526**, attempt 1, completed / SUCCESS, **3/3** required jobs individually
completed / SUCCESS. Both have that exact head SHA.

The strengthened review/manual UI test passed (51.009 seconds); iOS app tests
**9/9**, semantic UI tests **10/10**, zero failures. All original history counts
remain. Active ASan/UBSan **54/54** with probes 86/87, both Android ARM ABIs,
Android app/JVM/lint/46-test instrumentation, native hosts/Apple boundary, Swift
macOS/Simulator, retained source/corpora and release isolation all PASS. No native
storage or execution semantics were changed for the documented local DrvFS limit.
No tests were weakened/skipped/optionalized. No rerun-until-green was used.

No unresolved A finding remains. All earlier failed or superseded SHAs/run evidence
above are historical only; they do not certify this implementation or its newer
replacement closure. The replacement closure changes seven Markdown files only
and must independently pass both full workflows again. Final observed receipt/
delivery records that distinct SHA and exact runs without self-reference. Until
that observation: FINAL STEP 6 CLOSURE CERTIFICATION PENDING.
Phase 7 Steps 7-8 NOT STARTED. Phase 8 NOT STARTED. STOP after Step 6.
