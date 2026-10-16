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

A: evaluation implementation, negative/golden/corpus tests and exact-SHA closure.
Production model/runtime/operator expansion is a separate architecture decision,
not a defect to hide inside A. B: broader model/operator/runtime architecture
for real production segmentation, separately reviewed; not assigned to Step 6.
C: optional further candidate research/physical measurements.

Schema 3, prior providers, package signature/normalization/trust and
DH-APPLE-METAL-01 unchanged. No AI UX, training, cloud inference, model download,
Step 6 implementation or Phase 6 work. No physical Apple claim.
