# Phase 8 Step 3 - Plan Inspection & Action Review

Status: AUTHORIZED / IN PROGRESS; implementation certification pending.
Steps 4-6 NOT STARTED. No editable review. This report does not certify its own SHA.

## Baseline and authority

After fetch, main == HEAD == origin/main ==
`b3d40ae0327d904c8f67995646b249fd95ecfa26`, clean tracked/staged tree. Historical
ignored evidence preserved. Phase 7 CLOSED / COMPLETE / FULL PASS. Phase 8 Steps
1-2 accepted COMPLETE / FULL PASS. Step 2 closure independently passed Foundation
40914672598 (7/7) and Native shells 40914672662 (3/3), attempt 1, every required
job completed/SUCCESS. Its [observed receipt](evidence/phase-8-step-2-closure-certification.json)
is retained without rewriting historical Step 1/2 contracts/reports/manifests.

## Architecture, records and native metadata

The additive [Plan Inspection & Action Review v1 contract](../contracts/plan-inspection-action-review-v1.md)
defines a caller-owned typed RIN1 projection. Parameters are 112 bytes, actions
544 bytes, and the complete inspection is 4808 bytes, with at most 8 actions and
4 parameters each. It binds exact native proposal/source/review identity,
repository epoch, canonical Plan SHA-256 and source revision. It is presentation,
not a Plan, approval, candidate, publisher or persistent object.

The existing canonical parser and Registry validator process exact cached Plan
bytes. `manual::registry()` supplies every stable tool ID, label, operation and
parameter version, parameter identity, unit, bound/reference and geometry category.
No Registry metadata/semantics or Plan format changes. Output is atomic, with
unknown/malformed/noncanonical/stale/forged/cross-owner data rejected as a whole.
Current checks reconstruct and compare the complete projection; edits to labels,
values, action count/order or unused bytes do not convey authority.
The frozen native action-projection helper also reprojects those validated Plan
bytes and compares the complete action count/array with the cached proposal,
rejecting a mismatch rather than trusting a potentially inconsistent summary.

## Presentation and association

Android and Apple display context (mode/status, informational provider, separate
planner), existing comparison, exact count and ordered actions, bounded feedback,
and distinct Preview/Reject/explicit Apply decisions. Every action shows position,
native label and stable ID/versions, exact signed raw values with Registry units,
and min/max/reference values. No lossy friendly conversion replaces raw values.
No platform Plan parser, parameter calibration or Registry rule is added.

Static multi-action/order and geometry warnings derive from native flags. Crop
shows all four exact millionth coordinates; rotation shows exact clockwise quarter
turns. Step 2 preview supplies resulting dimensions when available; geometry still
uses safe fit/toggle. No action sliders, editors, remove/reorder/replace/restore
controls. Existing comparison viewport controls remain presentation-only.

The immutable AutoReview owns its exact description alongside the same canonical
Plan. Native preview/inspection association is checked at actual installation under
the existing platform lifecycle gate. Preview A plus inspection B rejects. Existing
owner/request epochs fence late results. Manual/source/history/mode changes discard
or label stale inspection, and Apply is absent/unavailable. No silent refresh,
rebase, retry, repair or partial valid subset.

## Authority, lifecycle and resources

Inspection calls reuse existing native source/repository authority and serialize
through owner gates. No inspection worker, queue, image, cache or retained native
description is added. Stop -> join ALL calls -> destroy remains unchanged. The
existing review Apply path is untouched: explicit HUMAN event -> exact binding ->
single-use approval -> private candidate/execution -> at most one ordinary revision.
Inspection, scrolling and preview never approve, execute or mutate history.

Native extraction owns a fixed 4808-byte result/transport, retains zero native
inspection bytes and zero image bytes. Existing <=8192-byte Plan parsing/Registry
validation uses bounded temporary allocations. Platform transport is 4808 bytes
plus bounded value objects. Existing Phase 4/5 and additive intelligence resource
authority remain unchanged. No new ceiling or RSS-derived lease.

Initial Windows Zig Debug measurement on the existing 2x3 owned fixture, 100
repeated identical extractions: one action mean 3.025 ms; eight actions mean
3.762 ms. These include current repository/binding validation, not raster work.
Every repeated record was byte-identical. Native allocator sweep evidence and
final exact-SHA hosted results will be recorded only after execution; cumulative
requested bytes are not a peak-live/process/mobile memory lease. No GPU or
physical-device thermal/battery claim.

## Tests and platform evidence

Locally executed native inspection/C consumer and retained preview tests: 3/3
passed. Inspection initially passed 1422 checks across all eleven tools, exact
parameter identities/raw values/units/bounds, 1/8 actions, 9-action rejection,
forgery/ordering/owner/stale/Manual/undo/redo/repository/mode/stop and preview mismatch.
Preview retained 986 checks unchanged. Initial Android Kotlin/instrumentation
compilation passed with pinned offline dependencies. Subsequent added platform
tests require compilation/execution; no unexecuted Apple/Android result is PASS.

Native tests assert zero approval, candidate and history/document effects.
The independent shipping-source allocator clone adds exhaustive inspection OOM
testing while retaining its complete Begin/Render sweeps unchanged. Android
instrumentation exercises all eleven visible action descriptions, exact raw units,
stale/unsupported states, read-only accessibility and an eight-action native-backed
list/preview. Swift tests cover all eleven metadata projections and eight-action
order/precision/association. iOS semantic UI tests cover all eleven actions and a
mixed three-action sequence (101 exposure, -123 brightness, one quarter turn).

Certification fixtures route through existing kind-3 provider validation, never
approval/execution; JNI certification and Swift DEBUG gates exclude them from
release. Native/APK/Apple scanners additionally reject `InspectionTestHooks`,
`inspectionCandidateTest` and `inspection-test-multi`, with negative packaged
fixtures for native/DEX/resources. Final package scans remain required.

## Security and frozen boundaries

| Threat | Protection / test |
| --- | --- |
| UI label becomes execution identity | display-only native strings; forged label rejected |
| Forged record gains authority | no capability fields; complete native reconstruction |
| Stale/cross-document Apply | existing native exact binding; wrong-owner/stale tests |
| Reorder/hidden action/value mismatch | exact count/order/raw records; tamper and eight-action tests |
| Preview A / inspection B | native association at installation; mismatched review tests |
| Rounding changes raw value | exact integer + native unit; 101/-123 precision cases |
| Inspection mutates durable state | zero approval/candidate/revision/history assertions |
| Teardown/late result | existing owner/request fencing and stop/join/destroy |
| Release/privacy/resource leakage | bounded metadata, no cache/network/persistence; final scanners |

No A-class defect identified in local inspection/tests; final disposition remains
conditional on full exact-SHA certification. B/C: geometry spatial split remains
the explicitly authorized Step 2 limitation; friendly localization is optional,
with native exact identity/units always visible. Hosted evidence is not physical
accelerator or thermal certification.

Intent/Structured Intent/Plan/Registry/Envelope/Provider/reference planner,
Full Review Experience v1, Review Preview Foundation v1, HUMAN approval, candidate
lifecycle, publication fencing, Schema 3, Manual/shared history, platform authority,
Phase 4/5 and intelligence resource semantics unchanged. Source governance chains
only exact per-file Step 3 before/after hashes and explicit additions onto the
certified Step 2 inventory; no blanket exemption or historical rewrite.

Provider reference.assistance.v1 ADMITTED; planner reference.deterministic.v1.
Learned model NONE REQUIRED / NONE ADMITTED FOR PHASE 7 MVP. Free-form Intent v1
uninterpreted. No new learned AI/NLP, network/cloud, model/runtime/weights or research.
CPU VALIDATED; Vulkan/Metal UNKNOWN. DH-APPLE-METAL-01 DEFERRED / NOT WAIVED /
NOT SATISFIED. Privacy/local-first and no Schema 3 persistence remain intact.

## Certification and closure

Exact implementation SHA must independently pass all 7 Foundation and 3 Native
shell jobs, each inspected. Deterministic failures require root cause, correction,
new SHA and fresh full certification, never rerun-until-green. After implementation
certification, a separate documentation-only closure must independently pass both
complete workflows on its own exact SHA. Only the external observed receipt then
activates Step 3 COMPLETE / FULL PASS. Steps 4-6 remain NOT STARTED. Stop after Step 3.

## Development verification before exact-SHA certification

The exact [implementation file inventory](evidence/phase-8-step-3-files.json) and
[per-file source binding](phase-8-step-3-v1.json) record this step's scope.
Local checks are development evidence, not substitutes for exact-SHA workflows:

- Complete Windows Zig retained suite: 58/58 at the initial implementation stage;
  subsequent exact-Plan projection changes passed focused inspection/C ABI and
  the unchanged Begin/Render plus additive inspection allocator sweeps (3/3).
- Final focused active ASan/UBSan execution: inspection/C ABI 2/2, independent
  allocation clone 1/1, active instrumentation probes 2/2. Inspection: 1524 checks,
  including two simultaneously valid owners and extraction during a paused preview
  followed by Manual mutation. No timing sleeps used as race proof.
- Persistent allocation fault sweep: Windows Zig 343 failures recovered, maximum
  cumulative requested bytes 48,642; ASan/UBSan 426 failures recovered, cumulative
  requested bytes 54,540. These totals are not peak live allocations or RSS leases.
  Each failure preserves output/document state; native retained inspection bytes=0.
- Final native extraction benchmark: 100 repeats, one action mean 3.122 ms,
  eight actions mean 4.003 ms; complete record remains 4808 bytes, zero image bytes.
- Android pinned offline debug/release build, JVM tests, lint and instrumentation
  APK compilation passed; final native projection package rebuild also passed.
  Local packaged release isolation and negative fixtures passed: 12 native
  libraries, 2 DEX files. Emulator instrumentation remains a CI-only execution gate.
- Retained source command: 196/196 during development; focused exact-scope/hash
  checks rerun after source updates. Actionlint passed. Local invocation quoting/
  final-line CRLF errors in sanitizer tooling were corrected; no source/test
  assertion was altered to obtain a pass.
- Apple/macOS/Simulator/iOS execution is NOT locally executed on this Windows host.
  It remains NOT VERIFIED until observed on the exact implementation SHA in CI.

All existing frozen contracts/engines and Step 1/2 certified records remain intact.
Implementation and separate closure certification are still required. No Step 4.
