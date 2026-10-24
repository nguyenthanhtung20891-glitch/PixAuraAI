# Phase 7 Step 3 — Manual + AI Assist Experience Foundation

<!-- phase7-step3-current-authority -->
## Phase 7 Step 3 - current authority

Phase 6 CLOSED / COMPLETE / FULL PASS. Phase 7 AUTHORIZED / IN PROGRESS.
Step 1 COMPLETE / FULL PASS. Step 2 COMPLETE / FULL PASS.
Certified Step 2 closure: `0d73ffc3ec1fa1ac5e1032a4f8f47a2c69e1b990`.
Foundation 39956128221 SUCCESS 7/7; Native application shells 39956128320 SUCCESS
3/3, completed exact SHA, attempt 1, every required job individually SUCCESS.
Step 3 COMPLETE / FULL PASS.
Implementation certification SHA: `71bb02e003a92abdd3816abb378c2d3e9dc0503f`.
Foundation 40054802528 SUCCESS 7/7.
Native application shells 40054802514 SUCCESS 3/3.
Both completed exact SHA, attempt 1; every required job individually SUCCESS.
This separate documentation-only closure commit requires independent full
closure-SHA certification by both workflows. Until that observed completion:
FINAL STEP 3 CLOSURE CERTIFICATION PENDING. The final engineering receipt/delivery
records its distinct SHA and workflow evidence without self-reference or old-SHA
substitution. No unobserved job is PASS.
Manual + AI Assist Experience v1: one manual editor/history, validated ADVICE only.
Use suggestion navigates to the existing manual tool at its ordinary default;
it does NOT Apply, prefill, create approval/candidate, execute or commit.
Explicit user manual controls and Apply remain authoritative. AI AUTO-EDIT disabled.
Narrow checked advice-only C/JNI/Swift adapters; no new execution authority.
Frozen Intent/Structured Intent/Plan/Registry/Envelope/Provider/reference planner,
approval/candidate, three-mode contract, Schema 3 and Phase 5 unchanged.
Production model UNRESOLVED. CPU VALIDATED; Vulkan/Metal UNKNOWN.
No model/runtime/weights/NLP/cloud/network AI. Local-first, ephemeral advice.
DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED.
Phase 7 Steps 4-8 NOT STARTED. Phase 8 NOT STARTED. No Step 9.
Full retained source/native/platform, active ASan/UBSan, release isolation,
workflow lint and diff gates required. Both complete exact implementation-SHA
and separate closure-SHA workflows required; every job completed/SUCCESS.
STOP after Step 3. Do not implement Step 4.
See docs/contracts/ai-assist-experience-v1.md and docs/reports/phase-7-step-3.md.
<!-- phase7-step3-history -->

Implementation and observed evidence follow. Earlier failed certification attempts are historical.


## Purpose and baseline

Certified Step 2 closure `0d73ffc3ec1fa1ac5e1032a4f8f47a2c69e1b990` is the
starting baseline (main/origin/main, clean, verified before work). Foundation
39956128221 SUCCESS 7/7 and Native shells 39956128320 SUCCESS 3/3, exact SHA,
attempt 1. Phase 6 remains CLOSED / COMPLETE / FULL PASS.

## UX architecture and implementation

Manual and AI Assist use the same native editor and existing manual controls.
Android Compose and SwiftUI expose an accessible mode selector, disabled Auto-Edit,
explicit adjustment choices with visible values, Request, loading/cancel,
recommendation, Use suggestion, Dismiss and safe failure/stale states. Honest
copy identifies deterministic local reference guidance, not image analysis.

Use suggestion only inspects the current native record and selects the existing
manual tool at its ordinary default. It does NOT prefill/evaluate/apply the
recommended value. The human chooses a value and separately presses existing
Apply. No new approval/candidate/execution path or history class is introduced.

The Step 2 internal provider has no platform transport. Four narrow checked
advice-only C functions in `pixaura/assist.h` are necessary to request/cancel,
inspect and enumerate typed data without platform envelope parsing. They delegate
frozen native Envelope/Structured Intent/Registry validation and expose no generic
provider or editing authority. Request 16 bytes; presentation 88 bytes; eleven
choices, four parameter slots, one active request/cache, no handles or queue.

## State ownership, cancellation and freshness

Each platform owner has a single advice lane separate from existing command and
preview lanes. Interaction is ephemeral and source-bound. Local nonwrapping UI
epoch plus native source serial/repository epoch/context prevent late-result,
manual/history/source-change and ABA reuse. Manual select/update/actions invalidate
advice. Mode departure, cancel, interruption and close abandon the interaction;
return does not re-request or resurrect. Native stop, join ALL wrapper calls,
destroy remains ADR 0009 ownership. Provider error does not enter manual authority.

States: Manual/Idle; Assist/Idle, Requesting, Proposal Available, Failed, Stale,
Cancelled, Rejected. State, document and execution authority remain separate.

## Security findings and fixes

A: initial new request-choice table had crop/rotation amounts in reverse identity
order. Native existing parameter validation rejected choice 9 (status 7), with no
document mutation. Corrected table order and retained all-eleven validation test.
A: Kotlin warnings-as-errors identified redundant null check and restricted data
class copy visibility. Removed redundant condition and made the immutable typed
choice an ordinary class; no compiler-warning suppression. No architecture change.
Further A findings, if discovered during certification, must be fixed and recorded.
No unresolved A finding may remain at closure.

B: Auto-Edit, review/approval UX, full cross-platform interaction orchestration,
production intelligence admission and model selection remain later authorized
steps; physical Apple Metal certification remains deferred. C: optional copy and
visual refinement, later performance measurements. No such work implemented.

## Tests and evidence

Native certification covers all eleven requests, read-only inspection, exact
values, foreign/forged/stale records, manual navigation without Apply, explicit
manual Apply, undo/redo, twelve deterministic checkpoint race orderings, duplicate
request BUSY, cancel/stop, malformed/provider/resource/internal failure, sequence
exhaustion and malformed/aligned/nonoverlap ABI. C consumer checks stable layout.
Swift wrapper tests cover typed transport, shared ordinary history, cancellation,
BUSY, close/join and provider failures. Android instrumentation exercises actual
Compose navigation/request/dismiss/manual Apply/undo/redo, recreation, cancel,
mode switching, duplicate request and provider failures. XCTest UI coverage uses
specific-element accessibility taps, not coordinate or frame/window snapshots.

The full retained native/source/platform and active ASan/UBSan gates must pass;
Apple/emulator execution is CI evidence only until actually observed. No unrun
check is reported PASS. Implementation and separate closure SHA each require
Foundation 7/7 and Native application shells 3/3, all jobs completed/SUCCESS.
Exact observed evidence is appended after certification without self-reference.

Release scans reject new certification hooks and debug-only failure launch flag
alongside retained approval/race/storage/test-provider markers. Hooks are guarded
by existing certification/debug compilation, absent from release objects.
The debug-only `--assist-test-pause` harness holds a native request checkpoint;
Cancel/mode departure releases it after native cancellation. XCTest can thus
observe real requesting/cancel/mode-switch behavior without timing-based taps.
That launch flag is also forbidden by production release scans.

## Frozen constraints

Intent v1, Structured Intent v1, Plan v1, Registry v1, Proposal Envelope v1,
Assistance Provider v1, deterministic reference planner, approval/candidate,
three-mode contract, Schema 3, shared revision/history/manual tools, ADR 0009 and
Phase 5 trust remain unchanged. No production model/runtime/weights/NLP/cloud/
network AI. Production model UNRESOLVED. CPU VALIDATED; Vulkan/Metal UNKNOWN.
U2NETP audited/unsupported/unapproved/unincorporated, checkpoint rights unresolved.
DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED. Hosted Apple is software
only. Phase 7 Steps 4–8 NOT STARTED. Phase 8 NOT STARTED.

## Changed files and certification record

The reviewed file inventory follows. Implementation/closure run evidence is
recorded only after exact-SHA workflow observation.
No approval, candidate, Plan execution or document persistence authority added.

## Reviewed implementation file inventory

- `AGENTS.md`
- `ARCHITECTURE.md`
- `CMakeLists.txt`
- `DECISIONS.md`
- `docs/contracts/ai-assist-experience-v1.md`
- `docs/plans/phase-7-roadmap.md`
- `docs/reports/phase-7-step-3.md`
- `packages/core/include/pixaura/assist.h`
- `packages/core/src/assist_experience.hpp`
- `packages/core/src/assist_experience_boundary.hpp`
- `packages/core/src/assist_request_codec.hpp`
- `packages/core/src/document.cpp`
- `packages/core/src/editor_api.cpp`
- `packages/core/swift/Sources/ManualEditor.swift`
- `packages/core/swift/Tests/AssistExperienceTests.swift`
- `packages/core/tests/assist_c_consumer.c`
- `packages/core/tests/assist_experience_test.cpp`
- `platforms/android/app/src/androidTest/java/ai/pixaura/app/AssistExperienceTest.kt`
- `platforms/android/app/src/main/java/ai/pixaura/app/AssistPanel.kt`
- `platforms/android/app/src/main/java/ai/pixaura/app/MainActivity.kt`
- `platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorModel.kt`
- `platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorScreen.kt`
- `platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/ManualEditor.kt`
- `platforms/android/native/editor_bridge.cpp`
- `platforms/ios/PixAuraAI/ManualEditorModel.swift`
- `platforms/ios/PixAuraAI/ManualEditorScreen.swift`
- `platforms/ios/PixAuraAI/ShellView.swift`
- `platforms/ios/PixAuraAIUITests/ShellUITests.swift`
- `QUALITY_GATES.md`
- `ROADMAP.md`
- `scripts/check-model-release.mjs`
- `tests/agent-execution.test.mjs`
- `tests/manual-editor.test.mjs`

## Local observed validation

Windows native (documented Zig compiler fallback): 49/49 PASS. Retained Windows
source: 124/124 PASS. Complete Linux source: 166/166 PASS. Android debug/release
build, both native ARM ABIs (plus x86_64), debug/release JVM (7+7 tests), lint
and instrumentation APK compilation PASS; no emulator execution claimed locally.
Release scans passed all 13 stripped native libraries; workflow lint and diff
check PASS. Completed Linux/native and active sanitizer results follow below.
Completed Apple and Android execution is recorded in the exact-SHA evidence below.

Linux native: 50/50 PASS. Full active ASan/UBSan: 52/52 PASS, 551.09 seconds;
negative probes confirmed instrumentation with required exits 86 (ASan) and 87
(UBSan). Final independent advice request goldens: 535 checks, eleven tools,
twelve deterministic race orderings. No unresolved A finding in local review.
No tests weakened, skipped or made optional; no rerun-until-green.

## Implementation certification remediation

Initial implementation `31a33e739476e0d53f54199fcbbf1d7ca6333fdf` is NOT
certified. Foundation 40030343544 found two deterministic test-build defects:
GCC's warnings-as-errors rejected adjacent same-line `if` statements in the new
race test; separated statements preserve every assertion. Clang's C linker did
not supply C++ UBSan vptr runtime symbols for the C consumer linked to the C++
core (`__ubsan_vptr_type_cache`, `__ubsan_handle_dynamic_type_cache_miss`).
The consumer still compiles as C, now links with CXX like retained C consumers.
The Windows MSVC job also required explicit C11 for its `_Static_assert` layout
checks; the target now declares `c_std_11`, retaining every size/offset assertion.
No production semantics, sanitizer settings, tests or warnings are weakened.
Native shells 40030343582 belongs only to that initial SHA; any results cannot
certify the remediation SHA. No rerun of either failed run is used.

Initial Android instrumentation completed 38 tests, 37 PASS / 1 FAIL / 0 skipped.
Its XML stack identifies `AssistExperienceTest.kt:75`, returning from Undo/Redo:
the helper tapped the off-screen AI Assist mode chip without scrolling, so the
advice panel did not exist. The helper now uses semantic `performScrollTo()`
before the tap and observes actual Assist mode, preserving every downstream
history/navigation/recreation assertion. This is a test harness correction,
not a product semantic change or retry. The other two new Assist tests passed.

Portability remediation SHA e530477ea33a1f2026ee77734b9f0b1a84013e04:
Foundation 40031104087 completed SUCCESS, all 7 jobs individually SUCCESS.
Native shells 40031104157 retained the same Android off-screen tap failure;
this SHA is NOT fully certified. Initial-SHA iOS app/UI job 120676763706
(run 40030343582) completed SUCCESS. These are historical diagnostic evidence,
not certification for the subsequent semantic-scroll correction.

The first scroll correction `92abd3c1665d748fa9ce8cbe63348693cf08cfda` exposed
the second ancestor: Compose `ActionsKt.scrollToNode` resolves only its closest
scrollable parent. The chip's horizontal mode row did not scroll the outer
vertical editor viewport. Native shells 40033377348 / Android job 120686837871
therefore failed the strengthened Assist-enabled postcondition (37/38 PASS,
0 skipped). The installed Compose 1.10.0 bytecode and XML stack confirm the
cause. The harness now scrolls the mode-selector row into the outer viewport
before scrolling/tapping its chip. No runtime change, retry, timeout increase
or assertion removal. A new SHA must independently pass both full workflows.

Local Android verification of the completed nested-scroll fix: existing owned
Android 16 / API 36.1 x86_64 AVD, WHPX, headless SwiftShader. All 38 retained
instrumentation tests PASS (0 failures/errors/skips), 2m11s, including all three
Assist cases and existing JNI/manual/Agent/history/lifecycle coverage. The first
local invocation passed all Assist cases but omitted the retained GPU transport
fixture argument (37/38); supplying its required fresh host-prepared directory
resolved that invocation defect without changing tests. Transport evidence was
verified exactly, archived under build, and the owned emulator shut down.
Foundation 40033377233 on the prior scroll SHA completed SUCCESS 7/7; the
subsequent correction still requires its own full exact-SHA certification.

## Observed final implementation certification

Certified implementation: `71bb02e003a92abdd3816abb378c2d3e9dc0503f`.
Both workflows completed SUCCESS on that exact SHA, attempt 1. Every required job
was individually observed completed/SUCCESS; no skipped, pending or missing job.

| Workflow | Run | Result |
| --- | --- | --- |
| Foundation boundaries | [40054802528](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/40054802528) | completed / SUCCESS, 7/7 |
| Native application shells | [40054802514](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/40054802514) | completed / SUCCESS, 3/3 |

| Retained evidence | Observed result |
| --- | --- |
| Windows MSVC native | 49/49 PASS; release-object isolation PASS |
| Linux native | 50/50 PASS; complete source/native/document/manual/Agent corpus retained |
| Active ASan + UBSan | 52/52 PASS; actual-instrumentation probes exited 86/87; no suppressions |
| Advice boundary | 535 checks, 11 independent request goldens, 12 deterministic race orderings, zero failures |
| Apple hosted native | 50/50 PASS |
| Swift macOS and iOS Simulator | 38/38 PASS on each |
| Android ARM64 + ARMv7 boundaries | Both required jobs PASS |
| Android application | Debug/release, JVM and lint PASS; emulator instrumentation 38/38, 0 failures/errors/skips |
| iOS application | Debug/release builds PASS; 7 app tests and 5 UI tests PASS |
| Release/test isolation | Production native-object scans, 13 local stripped Android libraries, iOS release binary PASS |
| Workflow lint / source consistency / diff | PASS; full retained gates, no weakening or optionalization |

The Android UI fix now passes on CI Android 15 and the local Android 16 emulator.
Use suggestion remains navigation-only: validated advice opens the same manual
controls at their normal defaults, with no document/history mutation. Only an
explicit human value change and existing manual Apply commit a normal revision.
Manual editing, undo/redo, failure fallback, stale/cancel/context/lifecycle fencing
and no-approval/no-candidate guarantees passed. No unresolved A finding remains.
No product runtime repair was required after the initial implementation; the
certification remediations changed tests/configuration/documentation only.

B: Steps 4–8, later production intelligence/model admission, and physical provider
certification remain deferred and NOT STARTED where not previously authorized.
C: optional copy/visual refinement and performance measurement remain future work.
All frozen contracts, Schema 3, shared history/manual semantics and Phase 5 trust
remain unchanged. No model/runtime/weights/NLP/cloud/network AI was added.
Production model UNRESOLVED; CPU VALIDATED; Vulkan/Metal UNKNOWN.
DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED. Hosted evidence is software
only. Phase 7 Steps 4–8 NOT STARTED. Phase 8 NOT STARTED. No Step 9.

## Separate closure certification

This closure changes only the seven current-authority documents. It adds no
production code, ABI, test behavior or workflow change. The new closure SHA must
independently complete both full workflows again; the implementation runs above
cannot certify it. Until observed completion: FINAL STEP 3 CLOSURE CERTIFICATION
PENDING. The observed final engineering receipt and final delivery record the
actual distinct closure SHA, run IDs, attempts and every job conclusion without
self-reference. They are engineering evidence only, never runtime authority.
