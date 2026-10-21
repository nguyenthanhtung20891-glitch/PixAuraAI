# Phase 6 Step 7 - cross-platform Agent integration and security certification

## Phase 6 Step 7 - current authority

Step 6 COMPLETE / FULL PASS at closure SHA
`0251b485f844d70b496a525eb6541aea384ce51f`:
Foundation 39283971353 SUCCESS 7/7; Native application shells 39283971479 SUCCESS 3/3.
Step 7 COMPLETE / FULL PASS.
Implementation certification SHA: `31d7b574c9e68335cbac7bd8f1472f07810b7a57`.
Foundation 39609614182 SUCCESS 7/7; Native application shells 39609614238 SUCCESS 3/3.
This closure-only documentation commit requires independent complete closure-SHA
Foundation and Native shells workflows; the observed final engineering receipt
and final delivery record that distinct evidence. No old-SHA substitution.
Agent platform integration/security v1 governs thin Android JNI and Swift/C
adapters over existing native Agent/editor authority. Explicit human approval,
exact plan binding, single-use consumption, private candidates, source serial/
epoch CAS, cancellation/publication fencing and stop -> join -> destroy retained.
Full retained Phase 0-5 and Steps 1-6, manual/platform/security/race gates,
Windows/Linux/Apple/Android, active ASan/UBSan, release isolation, workflow lint
and source/diff checks PASS on the implementation SHA above.
Intent v1, Structured Intent v1, Plan v1, Registry v1, Schema 3 and Phase 5 unchanged.
Production model UNRESOLVED. CPU release VALIDATED; Vulkan/Metal UNKNOWN.
No NLP/model, external runtime, weights, network or AI UX.
DH-APPLE-METAL-01 DEFERRED, NOT WAIVED, NOT SATISFIED.
Step 8 NOT STARTED. Phase 7 NOT STARTED. No Step 9+. STOP after Step 7.
Historical authority follows.

## Baseline and audit

Clean main and origin/main verified at the Step 6 closure SHA above before edits.
Audited Steps 1-6, native document/manual editing, candidate/publication locks,
canonical parsing, Phase 4 policy, Phase 5 separation and ADR 0009 ownership.
Existing approval/execution C ABI is reused. One additive planner delegate is
needed because platform owners correctly hide the internal document/live handle.
It delegates to existing reference planner with trusted service context under its
owner lock; no second serializer, validator, planner or editing implementation.

## Integration and security

Android owns fixed direct transport storage, copied immutable plan bytes and
typed approval/receipt wrappers. JNI checks exact capacities/alignment, bounded
array lengths and enums before calling native authority. All JNI C++ exceptions
are translated. No Java allocation/callback is required after native publication.
Swift owns native context storage and typed approval objects; Data is value-owned.
Both keep execution outside lifecycle locks, count in-flight Agent calls and join
them alongside preview calls before destroy. Concurrent close callers all wait.
There are no native callbacks retaining platform state. Explicit human approval
is a trusted host capability, not a sandbox against malicious native application
code; never hand an editor/approval issuer to a planner/model.

Native status values remain authoritative, including 15 NOT_APPROVED, 8 RESOURCE,
9 STALE and 13 CANCELLED/stopped. Unknown nonzero statuses remain failures.
Platform adapters do not retry, reapprove, replan or publish history independently.
One core candidate, no queue; transport admission bounded at eight calls per owner
and native approval limit eight within existing combined handle ceilings.

Debug-only fixed checkpoint rendezvous exercises real owner/JNI/C execution races
without timing-dependent pixel workloads or retained platform callbacks. Native
test entrypoints and Swift debug wrappers are absent from production; Android
control owner exists only in instrumentation sources. Existing production object
scans are extended for this surface. Existing native failure/rollback, source ABA,
repository CAS, publication and handle corpora remain retained.

## Certification evidence

Observed local gates: Windows native 46/46; focused Agent/C consumer/release scan
3/3 after adding the fixed checkpoint corpus; Windows retained source 117/117;
Linux full source 159/159 and native 47/47. Android arm64-v8a and armeabi-v7a
native builds PASS. Actual production core/storage/JNI libraries for both ARM
ABIs PASS the expanded binary test-surface scan. Gradle debug/release application,
both JVM suites, debug/release lint and instrumentation APK build PASS (151 tasks).
No local Android device is attached; runtime instrumentation requires CI.

The first active ASan/UBSan full run passed 48/49 but the exhaustive editor
allocation-failure sweep hit its unchanged 600-second timeout while other builds
were running on Windows-mounted storage. Logs show progress through boundary 2,
allocation index 2560; no sanitizer report or deadlock evidence. The controlled
focused rerun passed 6/6: 1548 Agent assertions, 3740 recovered allocation failures,
50042 editor checks and both active sanitizer negative probes. The unchanged
editor test completed in 260.25 seconds. Implementation Foundation 39607400257
also independently passed the full 49/49 sanitizer suite. No timeout, sanitizer,
test or production semantics have been weakened to conceal the failure.

This report does not count unexecuted checks as PASS. Implementation and closure commits each
require independent complete Foundation (7 jobs) and Native shells (3 jobs) runs.
Apple hosted macOS/iOS Simulator execution is software evidence only, never
physical Apple GPU certification. Historical Android physical Vulkan evidence is
scope-specific supporting evidence, not new Agent/inference physical certification.

## Findings and scope

The initial implementation `986cbed8af23edb5e505421e192fa2a24f9a9930`
passed Foundation 39607400257 (7/7). Native shells 39607400052 exposed one
instrumentation-test defect: its shutdown poll called the retained manual
`state()` API, whose closed-owner contract uses IllegalStateException, while
catching Agent EditorFailure. The exact stack identifies ManualEditor.kt:59.
The test now polls `agentBinding()` and asserts its explicit stopped status 13.
Manual/API/runtime semantics are unchanged. The old failed SHA is not reused
as certification; the remediation SHA requires both complete workflows again.

A: expose existing authority safely, close/join Agent calls, deterministic platform
race/authority tests, status parity, release isolation and exact-SHA certification.
B: Phase 6 Step 8 closure and later Phase 7 product integration remain deferred.
C: optional UX/model research and extra physical measurements remain deferred.
No new tool, schema, registry, approval/candidate semantics or provider trust.
U2NETP remains unresolved/unapproved/unincorporated. No model/runtime/weights/UI.

## Observed implementation workflows

Implementation SHA: `31d7b574c9e68335cbac7bd8f1472f07810b7a57`.
[Foundation 39609614182](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/39609614182) SUCCESS 7/7.
[Native application shells 39609614238](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/39609614238) SUCCESS 3/3.

| Workflow | Required job | Result | Job ID |
| --- | --- | --- | --- |
| Foundation | android-boundary (arm64-v8a) | SUCCESS | 119254718792 |
| Foundation | apple-boundary | SUCCESS | 119254719120 |
| Foundation | workflow-lint | SUCCESS | 119254719138 |
| Foundation | android-boundary (armeabi-v7a) | SUCCESS | 119254719216 |
| Foundation | portable (windows-2025) | SUCCESS | 119254719231 |
| Foundation | sanitizer | SUCCESS | 119254719276 |
| Foundation | portable (ubuntu-24.04) | SUCCESS | 119254719429 |
| Native shells | source | SUCCESS | 119254718840 |
| Native shells | android-app | SUCCESS | 119254719320 |
| Native shells | ios-app | SUCCESS | 119254719321 |

Observed retained/platform results: Windows native 46/46; Linux and Apple native
47/47 each; active ASan/UBSan 49/49; Agent native corpus 1548 assertions; Swift
macOS 35/35 and iOS Simulator 35/35; Android emulator instrumentation 35/35 with
zero failures/errors/skips. Both Android ARM ABIs, debug/release apps, JVM suites,
lint, iOS app/UI and production release scans PASS. Source 117 Windows/159 Linux;
workflow lint and diff checks PASS. Hosted/simulator Apple results are software
evidence only; no physical Apple certification.

The only public ABI addition is pixaura_editor_agent_plan: delegate existing
planner through the trusted editor-owned document/live context without exposing
internal pointers. Existing ABI structures and Steps 1-6 semantics are unchanged.
Changed files cover the C header/delegate and debug checkpoints; Kotlin/JNI and
Swift owners; native C/C++/Swift/instrumentation/security corpora; binary scanner;
the Step 7 contract/report and seven current-authority documents. No app UI,
provider, registry, operation, Schema 3 or dependency change.

A findings resolved: minimal existing-authority delegate, typed platform
transport, fail-closed status handling, quiescent close, deterministic native
platform checkpoints, security/lifecycle/ABA/approval replay tests and release
object isolation. No architecture blocker. B/C items remain deferred as above.
This documentation closure has its own final SHA and must independently pass both
complete workflows before final delivery can claim final closure certification.
