# Phase 5 Step 5 - Compact segmentation evaluation

Status: IMPLEMENTED under approved Option A; exact-final-SHA certification pending.
Final delivery records observed SHA/workflows/totals; no later uncertified status commit.
Baseline: `a3ad7e7a61030992d933b3ca68572f83604dd5a9`.
Step 4 accepted FULL PASS: Foundation 38730904780 (7/7), Native application
shells 38730904823 (3/3). These are historical results, not Step 5 certification.
Only Step 5 is authorized; Step 6+ and Phase 6 NOT STARTED.

## Audit

Read current authoritative product/architecture/quality/roadmap/decision documents,
ADRs 0005/0012 and package/provider/evaluation/capability contracts. Existing
evaluation.h/evaluation.cpp evaluate manual operation stacks on working rasters;
they do not compute segmentation quality and must retain their semantics.
Existing inference C API already owns verification, immutable prepared packages,
output metadata/copy, generation fencing, cancellation and resource accounting.
An evaluation runner should consume those checked handles rather than add raw
ONNX/file execution or duplicate model parsing in scripts/Kotlin/Swift.

Existing deterministic decode/cross-tier image generators can supply patterns,
but their goldens are rendering/policy evidence, not independently defined
foreground ground truth. No approved realistic foreground-mask dataset was found.
New synthetic scene/mask definitions are needed; no private/random photos needed.

Existing package ceilings remain 64 MiB per payload, 128 MiB total payload,
65536-byte manifest, rank four, dimensions <=1024, tensor <=4194304 elements.
Inference context ceiling remains 256 MiB, <=32 handles. Reference graph payload
<=4096 bytes. Vulkan/Metal transient reservations remain 1638400/1114112 bytes.
No Phase 4 tier/pressure/admission logic belongs in the evaluation harness.
Production trust currently has zero anchors; any fixture execution must use the
existing structurally isolated test path, never enable test trust in release.

## Resolved architecture gate

[Candidate audit and concrete options](../reviews/phase-5-segmentation-candidates.md).
U2NETP is a plausible compact salient-foreground candidate, but its graph,
preprocessing and output topology exceed the frozen contract. It is not executed
or approved for redistribution. The Product Owner explicitly approved Option A on 2026-10-16, resolving the
initial stop gate. Production selection remains UNRESOLVED. Options B/C are
future architecture work, not Step 6 admission/fallback responsibilities.

## Implemented evaluation

[Frozen contract](../contracts/segmentation-evaluation-v1.md). Engineering-only
C++ helpers are compiled inside existing certification guards, with no new
production ABI, model parser or provider semantics. The same checked verification/
prepare/execute/query/copy path runs CPU on all native hosts, Vulkan through
Android instrumentation and Metal through hosted native/Swift/Simulator tests.
Release-object scans reject PXSEG/corpus/test-trust markers.

Five exact rational metrics and checked confusion counters preserve >=0.5.
Both masks empty => all scores 1; exactly one empty => IoU/Dice/precision/recall
0, accuracy retains TN/total. Six independent 2x2 goldens cover perfect/miss/
partial/FP/FN/empty; malformed dimensions/counts/probabilities/NaN/Inf/mask/version/
threshold/overflow reject. Output metadata/generation/model digest and released
model/output cases are checked before scoring.

24 cases: 12 geometric scenes at 8x8 and 256x256, independently generated exact
mask hashes and reference confusion counts. CPU also scores all-background and
all-foreground baselines. 72 CPU rows /24 accelerator rows per run, one case
at a time, <3500 bytes/row; bounded extraction validates exact fractions and
streams JSONL under build evidence. No realistic/photo quality claim.

### Synthetic quality (independent golden expectations, checked against execution)

| Case group | Reference IoU / Dice / accuracy | Limitation |
| --- | --- | --- |
| Nine color-separable scenes | 1 / 1 / 1 at both sizes | Colors explicitly expose truth; no learned recognition |
| Low contrast | 0 / 0 / 0.75 | Foreground lies below frozen threshold |
| Gradient background | 0.4 / 4/7 / 0.625 | Bright background produces false positives |
| Small invisible subject | 0 / 0 / (N-1)/N | Accuracy nearly 1 hides complete subject miss |
| All-background baseline | IoU/Dice/precision/recall 0; accuracy background fraction | Every case has nonempty truth |
| All-foreground baseline | IoU/accuracy/precision foreground fraction; recall 1; Dice=2F/(N+F) | Foreground area dominates score |

### Resource and timing observations

Signed packages 713/717 bytes; ONNX payloads 171/175 bytes at 8/256 respectively.
Measured prepared reservation delta 4096 bytes; prepared CPU context reservation
201417/201421 bytes. At 256: input 786432, output 262144, CPU scratch 0, backend
transient reservation CPU 0 / Vulkan 1638400 / Metal 1114112 bytes. Accelerated
values are existing bounded accounting estimates, not measured GPU/RSS peaks.
Evaluation buffers 1376256 bytes maximum plus bounded metadata/hash/temp copies.
No raised ceiling or persistent cache.
Contract-derived execution reservation at 256 is prepared-context reservation
plus 1048576 input/output bytes plus backend transient: CPU 1249997 bytes,
Vulkan 2888397, Metal 2364109. These are accounting estimates, not sampled peak
RSS or driver memory measurements; caller/evaluation buffers are separate.

Cold prepare once per model/resolution separately, first execution per case and
three warm repetitions (all retained, median/max). The first scene includes the
first execution of the prepared model; GPU pipeline setup occurs on execution.
Synthetic generation and metric scoring timed separately; total case spans
generation/hash/four executions/copies/metrics, excludes preparation/JSON/baselines.
No fastest-only selection or physical speedup claim. Final delivery/log JSONL
records actual host/provider timing observations; timings are not goldens.

CPU correctness evidence is distinct from Android emulator, hosted macOS and
iOS Simulator integration. No fresh physical measurements; DH remains deferred.
U2NETP quality/latency/runtime memory NOT MEASURED / UNSUPPORTED. Its approximate
4.7 MB checkpoint size is author-reported, not measured. No approved checkpoint
provenance/redistribution, weights, conversion, runtime or production selection.

## Validation and findings

Initial repository status clean; HEAD equals cached origin/main at the baseline;
initial git diff --check PASS. Documentation validation results are recorded in
the task delivery. Local focused Windows/Linux/ASan-UBSan package/provider/security checks 6/6
each PASS on the final native harness; Windows documentation/source 10/10 PASS.
Android debug/release, arm64/ARMv7/x86_64 native, JVM, lint and instrumentation
APK build PASS after stale generated asset intermediates were rebuilt. Full
Linux source: 132/134 passed under concurrent build load; two unchanged
readiness deadlines timed out, then passed 2/2 in the focused rerun with zero
skips after build completion. All 134 source cases observed passing across
those runs; complete hosted gates still require final-SHA observation.
No unobserved gate counts as PASS. Final delivery supplies exact-SHA job totals.

First exact-SHA MSVC build diagnosed C4244 in std::fill baseline constants: the
template deduced int for a float vector. Both constants are now explicit 0.0f/
1.0f; values and metric semantics are unchanged. Warning enforcement remains
enabled. Affected local Windows checks 6/6 PASS after the fix; the corrected
SHA requires both fresh complete workflows, never inherited prior-SHA results.

Android instrumentation subsequently passed 30/30, but its liblog printf buffer
truncated a >1 KiB JSON row and extraction correctly failed closed. Engineering
transport now emits <=700-character identified fragments with bounded, ordered,
complete reassembly. No field/assertion removed and no provider/model semantics
changed. The corrected SHA again requires both complete workflows.
Local documentation/source/fragment checks 11/11 PASS, including missing,
duplicate and out-of-order fragment rejection; all three Android debug ABIs and
instrumentation APK compile PASS. Unfragmented native rows remain compatible.

Independent extraction also caught XCTest output interleaved inside buffered
Apple JSON despite successful native/Swift tests. Apple now uses <=400-character
fragments with direct complete <512-byte writes, and both macOS/Simulator scripts
gate on structured extraction and retain JSONL/summary artifacts. Fragment count
is bounded at nine and reconstruction remains <3500 bytes. No row dropped and
no metric/model/provider execution semantics changed. Fresh exact-SHA workflows
must certify this final evidence correction.
The same POSIX implementation is exercised on Linux: affected native and
ASan/UBSan checks 6/6 each PASS; 72 rows reconstruct with a competing notification
writer. Source/codec checks 11/11, shell/build-source checks 12/12 and workflow
lint PASS. These local observations do not substitute for hosted Apple gates.

A: evaluation implementation, negative/golden/corpus tests and exact-SHA closure.
Production model/runtime/operator expansion is a separate architecture decision,
not a defect to hide inside A. B: broader model/operator/runtime architecture
for real production segmentation, separately reviewed; not assigned to Step 6.
C: optional further candidate research/physical measurements.

## Retained iOS UI gate investigation

Candidate `467ce21d537c121a29c10796af4fd30c6683b2b8` passed Foundation
38915254024 (7/7); Native shells 38915254130 preserved source/Android success.
Original iOS job 117020944573 failed waiting for actionable pixaura.crop after
background/foreground. The failure hierarchy nevertheless contained Crop and
Rotate, neither disabled, with revisions still 2. No missing project or crash.
The ONE authorized unchanged rerun, job 117280842933, passed the same crop
selection/fields/error/cancel sequence, then failed waiting for pixaura.blur
selection. Its failure hierarchy showed Blur Selected, its parameter value and
manual-slider, enabled Apply, and revisions 2. Swift macOS 31/31, Simulator
31/31 and app tests 7/7 passed; UI tests 2 executed, 1 failed. These observations
disprove a deterministic crop/reopen failure for this sequence; neither run
alone proves all application responsiveness behavior.

Inspection found an observer defect: the polling closure chained separate remote
exists/enabled or exists/selected queries under one 20-second XCTest waiter.
Logs show accessibility queries stalling/retrying across the deadline. Later
selection checks and the diagnostic hierarchy observed the valid state, but the
stored timeout was still asserted. The closure already built a fresh element
query, so stale-snapshot caching is NOT an established cause. The fallback also
retained a button across the transition, and the slider was retained across
Apply/undo/redo. No evidence establishes a production lifecycle/selection defect;
production Swift/native/shaders remain unchanged.

The targeted harness correction evaluates attribute matching in ONE query per
poll. A selected/enabled button AND its expected category content or associated
tool control must coexist in the same result. Crop requires width_ppm, rotate
requires 90-degree control, exposure/blur require manual-slider. Tool readiness
also requires enabled Apply/Cancel, proving a new active gesture rather than a
previously selected but inactive tool whose slider remains visible. Existing IDs,
coordinate visibility guard, real upper-quarter event and 20-second bounds
remain. The former second tap is removed. Failure records XCTFail and aborts
the scenario; a later valid snapshot never overrides timeout. Sliders are
re-resolved for each adjustment, and foreground activation is explicitly checked.
No fixed sleep, timeout increase, repeated action, assertion removal, test skip
or late-success exception. The postcondition is stronger than selection alone.

Bounded test-process PXUI records identify action sequence, stage, monotonic
tap start/end, wait duration, poll count, final match count and longest query.
Timeout includes activation state and at most 16384 hierarchy characters. No
production logging/telemetry or application state modification. Hosted evidence
must demonstrate these predicates and timing observations before closure; local
source checks cannot establish XCUITest runtime behavior. A new commit requires
both complete fresh exact-SHA workflows; prior candidate passes are diagnostic
history only. Final delivery records the new SHA/workflow IDs and whether the
A-class retained gate is resolved. B/C remain unchanged.
Local correction checks: targeted shell/source 6/6 PASS; full WSL source 135/135
PASS, zero skips; shell syntax, workflow lint and git diff --check PASS. Apple
UI execution is unavailable on this Windows host and is explicitly pending
the fresh exact-SHA hosted application workflow. No native production source
changed; all retained native/sanitizer/platform gates must run on that new SHA.

Schema 3, prior providers, package signature/normalization/trust and
DH-APPLE-METAL-01 unchanged. No AI UX, training, cloud inference, model download,
Step 6 implementation or Phase 6 work. No physical Apple claim.
