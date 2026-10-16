# Binary segmentation evaluation v1

Status: FROZEN under Phase 5 Step 5 Option A approval. Engineering-only, offline,
binary foreground segmentation. Production model selection is **UNRESOLVED**.
No model/runtime/operator expansion, training, real-photo claim or product UX.

## Inputs and identities

Metric version 1 accepts aligned accessible binary32 probability storage and a
byte-per-pixel expected mask, values exactly 0 or 1. Row-major dimensions must
match exactly; width/height 1..256, total 1..65536. Truthful counts/pointers and
stable caller storage are required. The metric helper does not dereference model
paths or claim verification of arbitrary caller probabilities.

The execution runner accepts existing isolated signed fixture packages through
Step 1 verification, then explicitly prepares the selected provider. It obtains
authenticated tensor metadata through the checked shared API, executes, checks
output version/shape/model manifest digest/request generation and copies the
owned output. Released/stale tokens reject. No raw ONNX execution, new trust
anchor or production signing path. Release builds exclude the entire harness.
Future engineering adapters can use the metric helper with their own reviewed
input binding; this does not authorize new providers or model formats.

Case identity is (evaluation version, case ID, dimensions). Expected identity is
SHA-256 of exactly the row-major mask bytes; an independent generated golden
must match before scoring. An executable record also carries signed manifest
SHA-256, exact ONNX payload SHA-256 and explicit provider ID. Manifest identity
transitively authenticates the complete package payload set. The source recipes
freeze input pixel values for each case/version, not arbitrary image filenames.

## Metrics and arithmetic

Foreground is probability **>=0.5**, unchanged from inference. No threshold sweep,
clamp, morphology, matting or boundary metric. Finite probabilities in [0,1]
only; IEEE word checks reject NaN/Inf, negative nonzero and >1 before comparison.
Negative zero is background. Values below 0.5 are background.

Checked uint64 counters TP, FP, FN, TN in row-major traversal; checked additions
precede total, union, doubled TP and denominator formation. Zero/over-limit total
or overflow rejects. No unordered reduction or floating summation. Scores are
exact integer rational pairs [numerator, denominator], not rounded decimal or
locale-dependent strings; fractions need not be reduced. Decimal displays may
divide these pairs, but the pairs/counts remain the reproducible authority.

| Metric | Nonzero-denominator formula |
| --- | --- |
| IoU / Jaccard | TP / (TP+FP+FN) |
| Dice / mask F1 | 2TP / (2TP+FP+FN) |
| Pixel accuracy | (TP+TN) / total |
| Precision | TP / (TP+FP) |
| Recall | TP / (TP+FN) |

Both empty: all five scores 1. Expected empty/predicted nonempty: IoU, Dice,
precision and recall 0; accuracy=TN/total. Expected nonempty/predicted empty:
IoU, Dice, precision and recall 0; accuracy=TN/total. These explicit precision/
recall conventions intentionally penalize the one-empty cases; no NaN/undefined
scores. No production quality threshold is adopted.

Six hand-counted 2x2 goldens: perfect, complete miss, partial overlap, false
positive only, false negative only and empty/empty. Partial overlap TP=FP=FN=TN=1
gives IoU 1/3 and all other scores 1/2. Invalid version, threshold, dimensions,
buffers, mask values, nonfinite/range probabilities and arithmetic overflow reject
with bounded internal Status: version, threshold, shape, mask, probability,
overflow or identity; success is ok. Failed validation publishes no result row.
These engineering codes do not change inference's stable public statuses.

## Corpus and preprocessing

24 repository-owned cases: 12 scene types each at 8x8 and 256x256. Rectangle,
ellipse, one-pixel cross/thin structure, disconnected corner regions, border
subject, high/low contrast, gradient background, one-pixel small subject, large
subject, hole and repeating alias-sensitive diagonal edge. Integer geometric
ground truth is independent of color thresholding. The independent Node oracle
freezes mask SHA-256, foreground count and reference confusion counts; C++ builds
the actual RGB/mask arrays and must match that oracle before evidence passes.
The generated JSON and compact C++ table are reproducible with
`node scripts/segmentation-evaluation-fixtures.mjs --check`.

Default monochrome RGB is 0.75 foreground / 0.25 background. High contrast uses
1/0; low contrast uses 0.4375/0.375; gradient background uses x/width; small
subject uses indistinguishable 0.25 colors. Inputs are planar F32 RGB NCHW at
exact authenticated dimensions. No resize, crop, interpolation, letterbox or
original-size mask remapping; signed normalization runs exactly once in the
unchanged provider. A future fixed-size production candidate needs separate
preprocessing review. No photographic/realistic quality dataset incorporated.

Existing authenticated fixtures: valid-A at 8x8, weights-121 at 256x256, both
weights (0.25,0.5,0.25). They are EXECUTION_FIXTURE, never production candidates.
Trivial all-background/all-foreground baselines are BASELINE, with null model
and provider identity. Each baseline is scored by the same metric authority.
Synthetic color separability does not prove subject recognition on photographs.

## Results, timing and bounds

Each complete JSON object is emitted as a `PXSEG ` log row; field names/order are
stable for v1 engineering evidence, not a user/project file format. Required
fields: evaluation_version, metric_version, case_id, width, height, threshold
[1,2], expected_sha256, predictor, candidate_state, production_selection,
provider_id, manifest_sha256, payload_sha256, counts [TP,FP,FN,TN] and five ratios.
Baseline identity fields are null. Executable rows additionally contain timing
and resource fields described below. No paths/photos/secret material in rows.
Android liblog printf has a smaller buffer than logcat's entry ceiling. Its
transport uses `PXSEG_PART row_id zero_based_index total_parts fragment`, each
fragment <=700 ASCII characters, <=9 parts. Atomic process-local IDs distinguish
interleaving rows. Extraction permits <=8 pending rows, checks sequential indices,
constant part count, <3500 reconstructed bytes and complete termination; malformed,
duplicate/out-of-order or missing parts fail. The reconstructed JSON is unchanged.
POSIX hosts (Linux and Apple) use <=400-character fragments and direct complete writes <512 bytes to
avoid stdio-buffer interleaving with XCTest messages. Both hosted macOS and
Simulator streams must successfully reconstruct/validate before their gate passes;
JSONL and summaries are retained in Apple workflow artifacts. No row is dropped
to conceal malformed evidence. Windows retains its observed complete raw rows.

Cold preparation measured once per resolution/provider, repeated as shared
metadata across its cases. It excludes verification and includes the actual
prepare API (Metal/Vulkan pipeline initialization occurs on execution). Each
case records first_case_execute_ns plus exactly three subsequent executions,
warm_count=3, median and max of all three; no discarded warmups/fastest-only
selection. First case is the first execution of that prepared model; later
cases reuse it. Outputs must be repeatable and match independent confusion
goldens. Backend execution includes shared validation/normalization and internal
CPU parity where applicable; no inferred GPU-only speedup.

preprocess_ns measures synthetic image/truth generation, not a photo pipeline.
postprocess_ns measures metric computation only. case_total_ns includes generation,
identity hashing, four executions/copies/releases and metrics; excludes preparation,
baseline scoring and result serialization. No statistical confidence claim.
Windows/Linux, Android emulator, hosted macOS and iOS Simulator timing results
must stay separately labeled by their enclosing observed run evidence. Driver
upload/dispatch/readback details remain retained provider logs, not invented
per-case measurements. Physical observations are optional and separately labeled.

Record exact package/model bytes, measured prepared/context reservation queries,
input/output bytes, backend transient reservation and explicit evaluation buffers.
Backend reservation is a bounded accounting estimate, not measured driver/RSS.
CPU provider scratch is zero; accelerated pack/readback/control storage is included
in backend_transient_bytes, not an additional zero-cost assertion. Evaluation owns
truth N bytes, planar RGB 12N bytes and two output copies 8N bytes: 21N <=1376256
bytes, plus bounded metadata, temporary hash and corruption-test copies. All
cases are processed/released one at a time; no dataset/output tensor accumulation.
Model context remains <=64 MiB for this harness, below the existing 256 MiB hard
ceiling; no cache, tier/admission policy or global worker pool added.

Per-row text <3500 bytes, 72 rows per CPU run (24 execution +48 baseline), 24
per accelerator run. Offline extraction with scripts/segmentation-results.mjs
accepts logs <=64 MiB, rows <=8192 characters, <=512 results; validates counts/
fractions/identity and writes JSONL incrementally. Its log bound also bounds
line-reader allocation. Evidence output is ignored build data, not project state.

## Candidate conclusions and scope

Candidate states distinguish EXECUTION_FIXTURE, AUDITED_UNSUPPORTED, LICENSE_BLOCKED,
RUNTIME_BLOCKED, PROVIDER_PROFILE_BLOCKED, VIABLE_FOR_FURTHER_REVIEW and
PRODUCTION_SELECTED; blockers may coexist in audit records. U2NETP remains
AUDITED_UNSUPPORTED with provider-profile, preprocessing and unresolved checkpoint
license/provenance blockers. NOT MEASURED is required for unexecuted quality,
latency and memory; author-reported size is not PixAuraAI measurement.
See [candidate audit](../reviews/phase-5-segmentation-candidates.md).

Option B (broader owned profile) and Option C (approved runtime/model) remain
future architecture decisions, not Step 6 responsibilities. Step 6 admission
can reason only about presently executable synthetic semantics; it must not
promise real subject segmentation. Schema 3 unchanged. No AI UX, Step 6 work
or Phase 6. DH-APPLE-METAL-01 remains DEFERRED; hosted/simulator is not physical
Apple certification and cannot waive the existing hardening/Beta/release gate.
