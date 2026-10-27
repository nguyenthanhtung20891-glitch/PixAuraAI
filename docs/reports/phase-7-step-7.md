# Phase 7 Step 7 — admission evidence

> Current integrated status: Steps 1-7 COMPLETE / FULL PASS. The observed exact
> closure-SHA receipts are in [Phase 7 closure](phase-7-closure.md). Earlier
> prepared/pending and future-step statements below retain their historical
> freeze-time meaning. Step 8 is separately authorized; Phase 8 NOT STARTED.

## Round 5 — Deterministic MVP Provider Admission and Step 7 Closure

Status: implementation certified; Step 7 CLOSURE CERTIFICATION PENDING.
This separate documentation-only closure requires independent full exact-SHA
certification before the observed final receipt activates admission/Step 7 closure. Step 8 NOT
STARTED. Phase 8 NOT STARTED. Starting main == HEAD == origin/main
`91a2bfc1094b212a99dede92ac470cd047e268d3`, verified after fetch, clean tracked/staged
tree. Accepted Round 4 Foundation 40365322260 SUCCESS 7/7 and Native shells
40365322294 SUCCESS 3/3, exact SHA, attempt 1, every required job SUCCESS.
Ignored Round 1–4 evidence is preserved.
The [implementation inventory](evidence/phase-7-step-7-round5-files.json) lists all
changed files; [executed local evidence](evidence/phase-7-step-7-round5-local.json)
retains measurements and the explicit local filesystem limitation.

### Certified implementation and prepared closure

Corrected implementation `4a795a81aa4b3b14f8c923bd8f9492859d877253` independently
passed Foundation 40459163663 (7/7) and Native shells 40459163654 (3/3), both
completed/SUCCESS, attempt 1, every required job individually SUCCESS. The
[job-level implementation receipt](evidence/phase-7-step-7-round5-implementation-certification.json)
records exact heads, jobs and conclusions. No initial-SHA or roll-up substitution.
All eleven explicit goals, 117 golden envelopes, 22 typical cases, fingerprint,
resource/failure paths and full retained native/platform gates passed. The corrected
measurement labels distinguish checkpoint/test layout from shipping layout.

Observed implementation evidence: Windows 52/52, Linux 53/53, Apple native 53/53,
active ASan/UBSan 55/55 (both instrumentation probes), source 177/177, Android both
native ABIs and 47/47 emulator instrumentation tests, Swift macOS/Simulator 44/44
each, iOS app 9/9 and UI 10/10. Final APK scan: 12 native libraries, 2 DEX files,
31,870,648 uncompressed bytes. Final unsigned Apple release APP scan: 8 files,
2,940,223 bytes, zero model assets. Release-negative fixtures and workflow lint pass.
These are software/package observations, not physical GPU or thermal certification.
Linux Debug provider request mean/max 146,181/226,624 ns, validation 354,518/440,663 ns;
shipping-layout object 104 bytes. Windows MSVC Debug request 249,067/318,100 ns,
validation 650,589/768,900 ns; shipping-layout object 184 bytes. Corresponding
instrumented test layouts are 120 and 200 bytes. Cumulative requested allocation
maxima: Linux 29,534 request / 65,553 validation bytes; Windows 42,367 / 99,173.
No such diagnostic value is promoted to a production allocation lease.

The final closure SHA is intentionally not written before it exists. This prepared
closure is documentation/authority only; its own two full workflows must complete
successfully. The observed final engineering receipt then activates:
PRODUCTION MVP ASSISTANCE PROVIDER reference.assistance.v1 — ADMITTED;
PRODUCTION LEARNED MODEL NONE REQUIRED / NONE ADMITTED FOR PHASE 7 MVP;
and Step 7 closure. Until that observed receipt: CLOSURE CERTIFICATION PENDING.
Step 8 and Phase 8 remain NOT STARTED. No runtime, contract semantic, dependency,
test or workflow behavior changes in this closure. The
[closure inventory](evidence/phase-7-step-7-round5-closure-files.json) records its scope.
No unresolved A-class finding remains for the bounded model-free provider; deferred
absolute-photo/learned research is not claimed solved or admitted. B/C future research
and optimization never authorize work beyond Step 7. CPU/Vulkan/Metal/DH trust unchanged.

### Principal Architect selection and exact identity

The intended production MVP Assistance Provider is the ALREADY IMPLEMENTED
**reference.assistance.v1**, contract/version 1, capabilities 7. The separate frozen
planner identity is **reference.deterministic.v1**. Neither is renamed or duplicated.
Existing shared-native Assist/Auto boundaries already construct ReferenceProvider;
no new production registration path, runtime source change or platform feature is
needed. [Admission contract](../contracts/deterministic-mvp-provider-admission-v1.md)
and [source/corpus manifest](../admission/deterministic-mvp-provider-v1.json) govern
this admission. A provider is not a learned model.

PRODUCTION LEARNED MODEL: **NONE REQUIRED / NONE ADMITTED FOR PHASE 7 MVP**.
Future learned provider DEFERRED / REPLACEABLE. Learned model search stops for MVP.
EmbeddingGemma stays DISQUALIFIED FOR THE CURRENT PIXAURAAI MVP; NIMA/FFCC/MobileIQA
are not selected or integrated. Round 3/4 research remains unchanged OFFLINE evidence,
not production-qualified. Learned-artifact/contextual-image gaps remain deferred
research requirements, not requirements silently waived for a shipped learned model.

### Product quality and authority

This provider wraps exact bounded machine-semantic requests. It does not inspect
photos, diagnose exposure/white balance/style, parse free-form Intent, translate,
perform NLP or choose artistic outcomes. Existing UI explicitly selects one of eleven
existing goals and values. Quality means exact request semantics, canonical envelopes,
existing validation/planner/Registry correctness and failure safety—not photo aesthetics.

Explicit request -> provider -> UNTRUSTED Envelope -> trusted native validation ->
existing Structured Intent/planner or Plan/Registry -> review -> HUMAN Apply ->
unchanged approval binding/single-use consumption -> private candidate/execution ->
at most one ordinary revision -> shared history/undo/redo. Provider has no document,
history, approval, candidate, editor callback, shell, filesystem or network capability.
Advice returns no Plan; Use suggestion remains navigation-only, no hidden prefill.
Manual remains independent. Auto means proposal/planning, never approval. No retry,
alternate-provider execution, replan or reapproval is introduced.

Retained native corpus: 39 valid Structured Intent goldens / 42 negative cases,
all eleven exact goal/tool mappings, minimum/maximum/neutral where permitted,
parameter/geometry boundaries, unknown fields/tools/versions, numeric and byte bounds.
Every valid case traverses all three envelope kinds: **117 canonical envelopes**,
ten exact repetitions each. Native test now asserts independent whole-stream FNV-64
**162cbcfb525cd535**, a non-security regression fingerprint; SHA-256 also binds the
stream in the manifest. Existing Plan corpus retains multi-action order and valid
eight-action/nine-action rejection coverage. No new multi-tool recipe is invented.
The eleven actual application catalog choices add **22 typical Advice/Structured
Intent cases**. Existing malformed/forgery/wrong-owner/context/ABA, cancel/stop/BUSY,
600 concurrent outcomes and allocation-failure tests remain active.

Android instrumentation now repeats all eleven native Auto Plans, checks exact
canonical bytes/values and provider identity, rejects execution as NOT_APPROVED,
and verifies unchanged ordinary history. Swift all-goal tests add the same repeated
Plan-byte check. Correlation IDs/request serials can differ between calls; Plan bytes
for unchanged live context do not. Platforms validate neither tool semantics nor
Plans independently; they use the existing shared-native authority.

### Ownership, bounds and measured cost

Each existing editor Service lazily owns one small synchronous provider reused by
Assist/Auto. There is no worker, queue, cache, model mapping, session or new handle.
Try-lock permits one request per provider, RESOURCE/BUSY otherwise. Cancellation
is nonsticky; stop is permanent; owner stop -> join ALL calls -> destroy remains
mandatory. No heavyweight Round 1 owner is instantiated for this model-free path.
Existing source/context/generation/serial/epoch fencing wins over late interaction.

Bounds remain: Structured/advice 2048 bytes, Plan 8192 bytes / eight actions,
binding 512 bytes, IDs 64 ASCII bytes, envelope 12288 bytes. Existing finite parser
shapes and integer/tool validators reject, never clamp or repair. Reference planner
emits one action; provider may wrap already explicit bounded multi-action Plan data.
Returned bytes belong to the caller; provider state retains no dynamic container.

Local Windows Zig Debug corpus: shipping-layout provider object **40 bytes** on
that ABI (the standalone assistance test object is **56 bytes**, including two
checkpoint pointers); request mean/max **242,156 / 496,100 ns**, trusted
validation/planning mean/max **621,611 / 1,363,200 ns**, over the 117 envelopes,
excluding image execution. These are build-specific diagnostics, not a universal
Release/device object size or latency claim.
Diagnostic requested C++ allocation totals max **23,544 bytes request** and
**54,564 bytes validation**, max **210 request allocations** across that corpus.
These are cumulative requested-byte observations, not live peaks, RSS, production
leases or every-malformed-input maxima; stack/allocator/OS and caller-owned inputs
are not included. Source bounds and retained failure sweeps supply structural/failure
evidence. Provider retains only mutexes, epoch and stopped flag. Object sizes/timings
are ABI/build-dependent; sanitizer instrumentation is separately labeled in logs.

Existing footprint tooling now records actual document/manual/editor object bytes
and Linux namespace-attributed provider text symbols as well as archives. These
shared-object/symbol measurements are NOT an isolated linked/package provider delta;
common parser code and linker coalescing prevent a false exact attribution. Actual
APK/app bytes are recorded by final-package scans on the certification SHA. Round 5
adds no production source, runtime dependency or model asset; the existing provider
has nonzero code/data footprint and is not mislabeled zero-cost.

Battery/thermal criterion: **NO MODEL WORKLOAD**—bounded synchronous JSON work only,
no raster/image/GPU/inference/background loop. Hosted/platform execution and bounded
code review characterize this provider's negligible model-specific workload. Whole-app
battery/physical thermal behavior is NOT MEASURED, not a hardware certification claim.
No Phase 4/5 cap, segmentation admission or intelligence resource contract is changed.

### Provenance, privacy, failure and release isolation

Source identity is bound by normalized UTF-8/LF SHA-256 and exact certified Git SHA:
provider header/codec, catalog/adapters, planner/document/manual sources and immutable
fixture metadata. Provider is first-party source; shared native/SQLite/codec/crypto
dependencies remain under existing pinned dependency/license/notices records.
No learned checkpoint, tokenizer, conversion artifact, redistribution obligation or
new dependency is introduced. The learned-model provenance contract is unchanged;
this model-free source admission does not fabricate a model manifest or bypass a
future learned artifact gate.

Provider input/output is local in-memory data. No upload, cloud, API key, telemetry,
environment read, filesystem scan, shell or dynamic loader exists in the interface.
Invalid requests/proposals, allocation/resource failure, stop/cancel/stale outcomes
leave document/history unchanged and create no approval/candidate/execution. Retained
platform failure, lifecycle, Manual-before/after, undo/redo, duplicate Apply and
publication atomicity suites remain mandatory; no pending authority enters Schema 3.

Native release-object scans now reject deferred research/admission-test markers.
Final Android APK scanning also rejects HDF5/NPZ/MAT/PB and known research-model markers,
with negative fixtures. A final Apple release APP scanner reuses the same forbidden
marker/model list across actual executable/framework/resources, rejects symlinks and
oversized members, and runs after the unsigned Release build with negative fixtures.
Existing native symbol/test-key, release/test-hook and Android asset allowlist gates
are retained. Research docs/data stay in the repository, not runtime app resources.

### Admission matrix and certification procedure

The contract explicitly covers quality, determinism, latency, memory/resource cost,
package, battery/thermal workload, Android, Apple, trust, provenance, privacy,
offline operation, fallback, release isolation and authority. No criterion is silently
omitted because the provider is deterministic. Learned performance/weights are not
applicable; physical GPU trust remains separate. No unresolved A-class authority
finding is known; final admission remains pending exact-SHA implementation and closure
certification. B: arbitrary-photo intelligence and physical future learned/device
qualification remain deferred, with no MVP diagnosis promise. C: optional allocation/
parser optimization must preserve exact bytes and contracts.

Local retained Windows native suite **53/53 PASS**; admission source checks **3/3 PASS**.
Full retained Linux source suite **177/177 PASS**, scanner negative fixtures and
workflow lint PASS. Local sanitizer run is explicitly **53/55, NOT FULL PASS**:
storage_test and storage_android_test require POSIX permissions and atomic
RENAME_NOREPLACE semantics unavailable on this G: v9fs mount. An isolated ignored
probe recorded chmod(0555) still reporting 0777, unprivileged write succeeding,
and renameat2(RENAME_NOREPLACE) returning EINVAL (22). Provider/admission sanitizer
tests passed, including fingerprint and allocation-failure paths. No unchanged
rerun, suppression, storage code/contract change or timeout increase was used.
The complete Linux CI filesystem must independently pass all 55 tests; this local
limitation is not substituted for that gate. Windows retains its native filesystem
suite. Initial offline manifest preparation used the wrong fixture field name
(plan rather than expected_plan); inspection corrected the engineering generator
before the manifest was created. No fixture/expected semantics were changed.
Actual final-SHA sanitizer/native/platform, full retained source, release scans and
workflow lint evidence must independently complete. Implementation and subsequent
documentation-only closure SHAs each require both full workflows, every required job
completed/SUCCESS. The observed final receipt activates Step 7 closure only after the
closure SHA succeeds; no unobserved COMPLETE claim or circular SHA is written here.

#### Measurement correction before closure

Initial implementation `9e5aaa5341188da211fa2e8fc9b6ef6f5e290623` had a measurement
label defect: assistance_test compiles PIXAURA_ASSISTANCE_TESTING, so its reported
object sizes include two checkpoint pointers. They were incorrectly described as
shipping-layout sizes. Request/Plan bytes, authority and runtime code are unaffected.
The correction labels that value test_owner_bytes, measures production_owner_bytes
inside the allocation executable WITHOUT that define, and adds a compile-time guard
and source regression. Windows Zig reports 56 test / 40 shipping-layout bytes;
Linux sanitizer reports 120 test / 104 shipping-layout bytes. Debug/ABI and mutex
internals remain explicitly distinguished from actual Release/device memory.
Corrected focused sanitizer assistance/allocation tests pass 2/2; the full local
filesystem limitation remains 53/55, never relabeled PASS. Initial Foundation run
40456988343 succeeded 7/7; Native run 40456988072 was still running when the defect
was found. Neither certifies the new correction SHA. No old workflow was rerun;
the correction requires fresh complete exact-SHA certification before closure.

Intent v1, Structured Intent v1, Plan v1, Registry v1, Proposal Envelope v1, Assistance
Provider v1, reference planner, HUMAN approval, candidate lifecycle, source/publication
fencing, Schema 3, Manual/shared history/platform authority, Phase 4/5 and Intelligence
Runtime Resource Admission v1 remain unchanged. cpu.reference.v1 VALIDATED;
android.vulkan.v1 UNKNOWN; apple.metal.v1 UNKNOWN. DH-APPLE-METAL-01 DEFERRED /
NOT WAIVED / NOT SATISFIED. Hosted Apple evidence is software only.

## Round 4 — Production Candidate Discovery & Absolute-Image Intelligence Feasibility

Round 4 engineering evaluation complete; exact-SHA certification PENDING until the
observed final delivery records both full workflows. Step 7 AUTHORIZED / IN PROGRESS /
ADMISSION BLOCKED. Production model UNRESOLVED. Step 8 NOT STARTED. Phase 8 NOT STARTED.
Starting main == HEAD == origin/main `e02750be62cddf284442a32598fdd12986d6659f`
verified after fetch, clean tracked/staged tree. Ignored historical evidence preserved.
Accepted Round 3: Foundation 40326439382 SUCCESS 7/7; Native shells 40326439363 SUCCESS
3/3, both exact SHA, attempt 1, every required job completed/SUCCESS.

EmbeddingGemma remains **DISQUALIFIED FOR THE CURRENT PIXAURAAI MVP**. No further
probe, tuning, rebuilding, provenance repair or mobile integration was performed.
Its current artifact remains CURRENT_ARTIFACT_PROVENANCE_FAILED. Round 3 is historical
relative-signal evidence, not an absolute correction qualification or replacement provider.

### Task decomposition and deterministic core

The [per-tool decomposition](../benchmarks/absolute-images-v1/task-decomposition.md)
distinguishes measurable conditions from desired corrections. Exposure, highlights
and shadows have deterministically observable extreme-condition evidence; safe
correction remains context-dependent. Brightness/contrast/saturation/temperature/
sharpness benefit from subject/intent/illuminant/noise context. Blur, crop and rotation
remain excluded from automatic inference, with existing explicit/manual tools intact.
No dark low-key photograph, warm sunset, foggy scene or selective-focus portrait is
automatically defective. A global quality score is not an editing direction.

The new OFFLINE one-image baseline measures linear luminance percentiles/spread,
clipping occupancy, saturation, channel balance and edge energy at fixed 224 RGB8.
It receives no paired reference. Fixed [policy v1](../benchmarks/absolute-images-v1/policy.json)
emits at most one bounded exposure/highlight/shadow suggestion, otherwise abstains.
Five ambiguous tool decisions always abstain. HEURISTIC_LOW is an evidence tier,
not calibrated probability; numeric suggestions are fixed feasibility values checked
against the existing Registry golden, not measured optimal corrections. No Plan,
Envelope, approval, candidate, edit/history operation or production provider is created.

### Absolute review and frozen evidence

[Review protocol](../benchmarks/absolute-images-v1/README.md),
[labels](../benchmarks/absolute-images-v1/labels.json),
[recipes](../benchmarks/absolute-images-v1/recipes.json) and
[pre-score bindings](../benchmarks/absolute-images-v1/review-freeze.json) document 32
CC0 natural bases and 16 visually reviewed degraded derivatives, eight scene strata,
48 images / 384 image-tool labels. No external photos were newly acquired; Round 3
individual source/license and byte identities are reused. Derivatives are not new
independent natural scenes. The policy and review were SHA-bound before first scoring;
the exclusive [start marker](evidence/phase-7-step-7-round4-start.json) records that
freeze. Freeze SHA-256 `f10eb08ad4040393c714a0466ee72960cb84ed0cbff2b960b61b90241e8dd929`.
No thresholds, images or labels were tuned after seeing results.

Labels are SINGLE AI VISUAL ENGINEERING REVIEW, not independent human consensus or
objective ground truth. They were made from images before metric outputs, not generated
by the tested metrics, and are explicitly not blind to derivative recipes. Confidence
is medium/low, with per-image/tool rationale and ambiguous exclusion. Sunset/white-balance
and softness variants can remain ambiguous despite known transformations. Technical
correction is separate from artistic taste. No parameter goldens or universal accuracy
thresholds exist. Future admission needs independent human review/adjudication and
fresh locked realistic evidence; this small feasibility set does not satisfy that gate.

### Absolute baseline results

| Tool | Positive / negative labels | Correct direction | Missed | False positive / acceptable labels | Abstained on scored labels | Ambiguous excluded |
|---|---:|---:|---:|---:|---:|---:|
| Exposure | 2 / 2 | 0 | 4 | 1 / 41 | 44 | 3 |
| Brightness | 1 / 1 | 0 | 2 | 0 / 46 | 48 | 0 |
| Contrast | 1 / 1 | 0 | 2 | 0 / 46 | 48 | 0 |
| Highlights | 0 / 1 | 1 | 0 | 1 / 46 | 45 | 1 |
| Shadows | 1 / 0 | 0 | 1 | 0 / 42 | 43 | 5 |
| Saturation | 1 / 1 | 0 | 2 | 0 / 45 | 47 | 1 |
| Temperature | 0 / 0 | 0 | 0 | 0 / 46 | 46 | 2 |
| Sharpen | 1 / 0 | 0 | 1 | 0 / 46 | 47 | 1 |

Only 3/48 images receive a suggestion; all 32 natural bases abstain. One broadly
white rose variant receives the reviewed highlight direction. Two nominal false
positives are label disagreements: v02 gets highlight reduction rather than its
labeled exposure reduction (a plausible alternative-tool correction, not proven harm);
v06 gets an exposure lift for a harsh contrast variant (concrete failure to distinguish
histogram darkness from the actual condition). Do not inflate alternative-tool
penalties into certainty. The 12 missed tool labels include alternatives for two tonal
variants, so they are coverage diagnostics, not 12 independently failed photographs.

No wrong-direction emissions on labeled correction slots, no nonfinite/invalid/out-of-
Registry values, no mutation. All 48 outputs reproduce exactly. The natural cohort
has no confidently labeled required correction; 32/32 abstention is NOT proof of useful
natural-photo correction coverage. Temperature has no confident directional label;
color accuracy is NOT VERIFIED. Native-unit calibration error is NOT SCORED.
No mixed-unit aggregate or statistical significance is claimed.

Per-domain and natural/derived strata are retained in the
[raw result](evidence/phase-7-step-7-round4-absolute.json). All three recommendations
are in derived urban/nature/flower cases; the nature contrast/exposure confusion
is a useful residual context case. Small strata do not establish scene generalization.
The baseline is not promoted to production and has not been optimized to this corpus.

### Candidate discovery and provenance

The complete [landscape/source table](../benchmarks/absolute-images-v1/candidate-landscape.md)
and [pinned source/provenance matrix](../benchmarks/absolute-images-v1/landscape.json)
cover NIMA, FFCC, MobileIQA, PaQ-2-PiQ, Deep White-Balance Editing, exposure correction,
HyperIQA and MUSIQ, plus the current IQA toolbox license boundary. Unknown parameters,
artifact sizes or conversion facts stay UNKNOWN. Monorepo dates are not model releases.

Shortlist, exactly three engineering hypotheses:

- NIMA MobileNet technical: compact rating-distribution veto/abstention evidence.
  Original repo HDF5 measured 13,174,216 bytes; SHA-256
  `78c8bb95098fd27695de4147465f4fd504d0ca5e11699a487534854b5aeb3036`.
  Apache-2.0 repository declaration; no full training/data-rights custody clearance.
  MOS distribution is not a defect classifier or direction/parameter source.
- FFCC: explicit illuminant uncertainty. Apache-2.0 source, archived; camera-linear
  training domain is not arbitrary already-white-balanced sRGB. No appropriate
  pinned production artifact or Kelvin calibration is established.
- MobileIQA: mobile-resolution contextual quality assessment. No LICENSE/checkpoint
  found in inspected official tree; README checkpoint/testing TBU. Published Android
  TFLite measurements are upstream only, not an audited artifact or Apple evidence.

PaQ-2-PiQ, Deep White-Balance and exposure-correction official sources declare
noncommercial terms and are excluded from the commercial production shortlist.
Direct RGB correction outputs also cannot replace existing tool execution authority.
HyperIQA and MUSIQ are deprioritized for heavier/context-score pipelines with no
demonstrated tool-specific gain or complete artifact lineage. The current IQA-PyTorch
toolbox declares PolyForm Noncommercial; it was not installed or treated as blanket
permission for all checkpoints. No weights from an unofficial mirror were substituted.

No learned inference executed in Round 4. NIMA was downloaded to ignored build/
only for official artifact identity/hash; it was NOT loaded or benchmarked. No new
framework installed. Learned PixAuraAI quality, incremental gain, load/cold/warm
latency, RAM and mobile compatibility all remain NOT VERIFIED / NOT MEASURED.
No host probe result is fabricated from upstream benchmark numbers or file size.

### Cost, decision and remaining gates

[Cost evidence](evidence/phase-7-step-7-round4-cost.json): WSL Linux x86_64 Python
3.14.4, absolute baseline median 0.07197 s / p95 0.07678 s / maximum 0.08213 s.
One-input traced Python allocation peak 11,224,124 bytes; 150,528 RGB input bytes
allocated before tracing. Diagnostic process peak RSS 68,591,616 includes retained
records, imports, repeated corpus generation and memory probe; not an allocation
lease or mobile budget. Low implementation complexity for measurements; candidate
integration complexity MEDIUM for NIMA, HIGH for FFCC's domain adaptation and MobileIQA's
missing rights/artifact plus attention/high-resolution transport. No production
package size, thermal, battery, Android/Apple memory-pressure evidence is established.

Round 4 recommendation, exactly one: **NEW_CANDIDATE_SEARCH_REQUIRED**.
The inspected deterministic policy has low correction coverage and context confusion;
these data do not prove every possible deterministic method insufficient, but do not
support DETERMINISTIC_ONLY_MVP admission. No inspected learned component establishes
an acceptable production-ready task/provenance/value combination. In particular, lack
of NIMA execution is NOT a negative measured quality result. HYBRID_WITH_CANDIDATE or
HYBRID_MULTI_COMPONENT would overclaim incremental value. None earns integration now.

Recommended future architecture (NOT IMPLEMENTED): one bounded deterministic
measurement core, optionally ONE small, licensed, provenance-complete contextual
technical-defect/uncertainty component for the specific residual cases; shared
evidence fusion with explicit uncertainty/conflict/abstention; independent bounded
deterministic parameter calibration; ordinary UNTRUSTED Envelope; existing native
validation/Plan/Registry/HUMAN approval/execution/history. No model zoo, generic MOS
to-edit shortcut or dedicated execution path. Candidate should first prove reduced
false correction on low-key/sunset/fog/selective-focus/noisy cases against a stronger
frozen absolute baseline. If it cannot, retain safe abstention and Manual control.

Before production: independently human-reviewed full-resolution corpus and fresh
holdout; rights/training/conversion provenance; actual candidate-vs-baseline measurement;
per-tool calibration/uncertainty and negative cases; runtime integrity/parity; physical
device headroom/lifecycle/package/thermal measurements under existing resource owner.
No new runtime, production registration, model loader, weights packaging or app changes.

A: admission evidence remains incomplete (human quality labels, contextual value,
candidate provenance/runtime/device qualification). No new editing-authority defect
or frozen-contract change is introduced. B: improve absolute baseline and independent
review diversity before any integration; no inferred correction for ambiguous intent.
C: optimize metric loops and evaluate bounded spatial/noise features only if useful.

All frozen Intent/Structured Intent/Plan/Registry/Envelope/Provider/planner, HUMAN
approval/candidate/publication, Schema 3, shared history/Manual/platform authority,
Phase 4 resource, Phase 5 segmentation/trust and Intelligence Runtime Resource Admission
v1 semantics are unchanged. CPU VALIDATED; Vulkan/Metal UNKNOWN. DH-APPLE-METAL-01
DEFERRED / NOT WAIVED / NOT SATISFIED. No production model/runtime/cloud inference,
free-form NLP or network editing authority. Hosted evidence remains software only.

The [Round 4 file inventory](evidence/phase-7-step-7-round4-files.json) records exact
paths. Both full workflows must certify the final evidence SHA, attempt/job level;
the final engineering receipt resolves SHA/run IDs without circular self-reference.
No Step 7 closure, Step 8 or Phase 8 authorization is created.

Pre-commit local validation: retained complete Linux source suite 174/174 PASS,
Round 2 evidence 4/4, Round 3 evidence 6/6, Round 4 evidence 8/8; actionlint PASS.
Initial local environment issues were diagnosed without repository behavior changes:
Windows sandbox Node spawn EPERM, then POSIX emulator fixtures' incompatible Windows
temporary paths; the full suite ran under the existing repository Linux Node toolchain
with TMPDIR inside build/. A non-login WSL PATH initially omitted that toolchain;
the explicit existing tool path resolved it. actionlint was an existing Windows
executable within a directory, not the directory itself. The ignored derivative
preparation initially called the PNG helper with an incorrect argument signature;
inspection corrected that helper call before review/freeze/scoring. No thresholds,
labels, tests, timeouts or production code were altered to hide a failure.

## Round 3 — Realistic Image Evidence & Incremental Value Gate

Status: AUTHORIZED / IN PROGRESS / ADMISSION BLOCKED. Round 3 evaluation complete;
exact-SHA certification pending. Step 7 is not complete. Step 8 and Phase 8 remain
NOT STARTED. Starting SHA `f2a4ef05e0166a3abd23b9a64d7d91a4c07cc039` verified main
== origin/main after fetch, clean tracked/staged state. Round 2 accepted COMPLETE /
FULL PASS: Foundation 40279433883 (7/7), Native shells 40279433687 (3/3), both exact
SHA, attempt 1, completed/SUCCESS. Existing ignored candidate evidence preserved.

The [natural-image method](../benchmarks/natural-images-v1/README.md) and
[individual CC0 source manifest](../benchmarks/natural-images-v1/sources.json)
record 32 realistic base photographs, four each in eight scene strata. Original
asset/page/license evidence stays ignored; compact normalized derivatives are
committed, 3,729,759 bytes in total. A near-duplicate was replaced by a hardware
photo before the split and corpus pin.
Portrait-specific quality and absolute artistic correction labels are absent.
Split: 16 development / 16 locked holdout, stratified deterministic SHA ordering,
no base or derivative crosses sets. Corpus/method pin commit
`fa5fa39e2af3c7a385cbf02a77273afd63c0d37c` preceded development fitting.

Seven signed controlled perturbation levels exercise the eight authorized tools.
Two bases per split also exercise opposed-half conflicts and two-condition
combinations. Both cheap metrics and semantic-axis evidence compare against the
same unmodified base. This tests relative recognition only:
**ABSOLUTE AUTO-CORRECTION STILL UNQUALIFIED**. Unchanged paired inputs have zero
evidence by construction; their false positives are not absolute neutral-photo
false positives. No exact inverse/perceptually optimal correction claim is made.

One predeclared candidate policy reuses the frozen Round 2 prototype directions;
no prototype search. Each method fits a single zero-intercept scalar calibration
per tool using development regular cases only. Fixed abstention/bins, golden
Registry bounds and a frozen pre-holdout policy bind all subsequent decisions.
The explicit reference provider is context; B (cheap metrics) versus C (embedding)
is the decisive comparison. Paired bootstrap intervals cluster by base photo,
not correlated derivatives. No aggregate mixed-unit calibration score is used.

Current artifact remains CURRENT_ARTIFACT_PROVENANCE_FAILED. No conversion,
new weights, production dependency, app wiring, runtime owner change, approval or
editing authority is authorized or introduced. Production model UNRESOLVED;
EmbeddingGemma unadmitted/unintegrated; CPU VALIDATED, Vulkan/Metal UNKNOWN.
DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED. All frozen contracts,
Schema 3, existing resource caps and Manual/history semantics remain unchanged.

### Frozen experiment and development

Pre-holdout calibration commit `d110b2f636351c6abeee96c590b6f107689c80a0` records
the development results and [frozen policy](../benchmarks/natural-images-v1/frozen-policy.json).
Policy SHA-256 `dd4610dc97d6edd66a5593e8f2e422ecf1b32e1735f046bd191ae5c5c782bd2b`;
configuration `ff2a6cf6bad86fef8cf818033c070c9ef55d9eca520ab5b50362b9e9ce0bc7b8`;
source/split manifest `7ff9aef7912046ae8dc1461f45b63afda9b2774bb42e43c5eb26255d1c827142`;
runner `1ee0006a6447e4bae861f13d567cb646b97bb4e90b7c13a9aa79f1e447eeb746`.
Bindings include all 32 derivative PNGs, native golden Registry, existing reference
baseline, runner dependencies, method and exact development result. Exactly one
development policy was executed; no prototype search, variant selection or tuning
after holdout. The exclusive holdout-start marker is retained as evidence.

Each split has 928 cases: 896 regular levels plus 16 combinations and 16 conflicts.
Development regular direction counts (B metrics / C candidate, each out of 96):
exposure 87/53, brightness 96/31, contrast 94/29, highlights 47/2, shadows 90/35,
saturation 81/49, temperature 86/61, sharpen 83/54. These are in-sample calibration
results, not independent qualification. Exact outputs/slopes/cost are in
[development evidence](evidence/phase-7-step-7-round3-development.json).

### Locked holdout result

Engineering MODEL FAMILY recommendation: **DISQUALIFY_CANDIDATE for the current
PixAuraAI MVP**. This is a recommendation to the Principal Architect, not a claim
about every possible use of this model family. Meaningful incremental image-aware
value over cheap metrics was **NOT DEMONSTRATED**.

| Tool | B direction /96 | C direction /96 | C minus B, percentage points | Paired bootstrap 95% interval, points | B/C weak /32 | B/C strong /32 |
|---|---:|---:|---:|---|---|---|
| Exposure | 86 | 48 | -39.58 | [-50.00, -29.17] | 24/14 | 32/19 |
| Brightness | 96 | 39 | -59.38 | [-67.71, -52.08] | 32/7 | 32/17 |
| Contrast | 95 | 30 | -67.71 | [-77.08, -57.29] | 31/10 | 32/9 |
| Highlights | 64 | 6 | -60.42 | [-79.17, -39.58] | 14/0 | 26/4 |
| Shadows | 94 | 41 | -55.21 | [-64.58, -44.79] | 30/14 | 32/15 |
| Saturation | 78 | 43 | -36.46 | [-50.00, -25.00] | 16/12 | 32/16 |
| Temperature | 86 | 62 | -25.00 | [-36.46, -13.54] | 24/15 | 32/26 |
| Sharpen | 83 | 61 | -22.92 | [-37.50, -8.33] | 22/11 | 31/27 |

Each interval uses 2,000 paired bootstrap samples of the 16 held-out BASE photos,
seed 73003. Intervals are descriptive, without a multiplicity-adjusted significance
claim. Small scene/stress strata and one source directory limit generalization.
The exact-RGB audit found all 768 requested nonzero regular perturbations were
byte-observable. No cases or labels were removed or changed after scoring.

| Tool | B/C abstentions /112 regular | B/C missing actionable recommendations | B/C native-unit MAE on emitted applicable samples |
|---|---|---|---|
| Exposure | 26/45 | 10/29 | 127.326 / 240.672 milli-EV |
| Brightness | 16/57 | 0/41 | 16.146 / 53.727 milli-linear |
| Contrast | 17/62 | 1/46 | 44.989 / 150.440 milli-stops |
| Highlights | 48/102 | 32/86 | 178.125 / 415.000 milli-EV |
| Shadows | 18/50 | 2/34 | 88.298 / 283.871 milli-EV |
| Saturation | 34/51 | 18/35 | 63.846 / 103.115 milli-ratio |
| Temperature | 26/37 | 10/21 | 127.326 / 197.632 kelvin |
| Sharpen | 71/71 | 7/14 | 75.805 / 71.353 milli-amount |

Missing recommendation counts exclude intentionally non-actionable sharpen
opposites; wrong-direction emissions are separately reflected in direction scores.
MAE is conditional on emitted applicable actions, with different sample coverage;
sharpen's small conditional MAE difference is not an overall quality advantage.
The targets are fixed perturbation-scaled native bins, not perceptually optimal
inverse edits. No mixed-unit parameter aggregate is used. Both B/C have zero
invalid/nonfinite/unbounded outputs and zero false positives on the 16 identical
reference pairs per tool. That paired false-positive result does not qualify
absolute neutral-photo behavior. All 928 candidate recommendations repeat exactly;
maximum repeated embedding-component difference is zero. Actual image embedding
calls: 1,632 (two evaluations per distinct image, reusing byte-identical references).
The metric implementation's repeated evaluations are deterministic in preflight
and a focused real-photo regression; this is not a second complete holdout run.

Sharpen's opposite/no-action cases require an additional false-positive check:
including unchanged and opposite cases, B emits 0/64 unwanted recommendations,
C emits 7/64, and the explicit reference emits 64/64. Other tools have 0/16 B/C
false positives on their non-actionable regular cases. These counts are derived
from the same retained records, without changing scoring inputs or policy.

| Scene stratum (2 held-out bases each) | B correct /96 | C correct /96 |
|---|---:|---:|
| Flowers | 90 | 50 |
| Indoor | 87 | 37 |
| Landscape | 81 | 44 |
| Low light | 65 | 38 |
| Nature | 84 | 45 |
| Objects/food | 89 | 37 |
| Urban | 92 | 44 |
| Water | 94 | 35 |

Conflict abstentions: B 8/16, C 7/16. Combination primary directions: B 13/16,
C 8/16. Candidate wins in isolated stress cells do not establish robust context
or complementary signal; only two stress bases were evaluated per split. No
combined/ensemble policy was trained or evaluated, and none is claimed qualified.

A explicit reference baseline was executed through the existing JS provider on
the same requests. It emits every explicit request and has 48/96 regular directions
per tool in this balanced experiment, with 16/16 paired-reference false positives.
It does not claim image understanding. A reporting defect in the frozen raw runner
assumed positive direction for all reference values: 7000 K is actually opposite
to the temperature rule's 6000 K positive correction. The separate reporting tool
corrects temperature's combination count from 2/2 to 0/2. Balanced regular counts
are unchanged. Raw records remain intact; B/C evidence, policies, slopes, labels,
thresholds and their comparison are unaffected. No inference was rerun. A focused
regression checks the corrected native-value interpretation.

### Cost and disposition

Host: WSL x86_64, Intel Core i5-12600K, Python 3.14.4, existing LiteRT-LM API 0.18.0,
CPU text/vision one thread, audio disabled. Candidate load 2.9880 s, first image
0.9126 s after static-text setup, warm image median 0.4735 s / p95 0.4991 s. No
cold OS-cache guarantee. Metric paired evaluation median 0.0573 s, maximum 0.0960 s.
Metric memory probe: 9,181,175 peak traced Python-allocation bytes for one 224x224
call (input allocated before tracing); at most 50,176 pixel records. This is not
native/device accounting or a production lease. Reference caching could reduce
baseline cost further; no such optimization was used to win this comparison.

Candidate artifact 387,710,976 bytes; process peak RSS 799,682,560 bytes, active RSS
788,369,408 (anonymous 405,835,776 / file-backed 382,533,632), post-close RSS
276,512,768. Process evidence includes Python, retained embedding-cache and corpus
overhead; post-close is not proof of a runtime leak, and RSS is never an exact lease.
No mobile/package/thermal/memory-pressure qualification or accelerator trust claim.

Metrics outperform the candidate on every tool and scene stratum; candidate
direction gaps are large, with every paired interval below zero. Conflict and
combination behavior do not compensate, and conditional sharpen calibration is
not better coverage. This result does not justify a roughly 388 MB artifact and
heavy runtime versus inexpensive read-only metrics for these current MVP tasks.
Do not invest in rebuilding or mobile integration on the evidence of this round.
Metrics themselves remain offline, non-authoritative and unqualified for absolute
auto-correction. No replacement production provider/model is selected here.

CURRENT ARTIFACT status remains **CURRENT_ARTIFACT_PROVENANCE_FAILED**, unadmitted
and unintegrated. Quality disqualification and artifact provenance failure are
independent. No new conversion-lineage evidence was obtained or inferred.

Raw [holdout](evidence/phase-7-step-7-round3-holdout.json),
[start marker](evidence/phase-7-step-7-round3-holdout-start.json) and
[derived analysis](evidence/phase-7-step-7-round3-analysis.json) retain exact counts,
per-tool domains/calibration coverage, raw evidence, host cost and reporting erratum.
The [changed-file inventory](evidence/phase-7-step-7-round3-files.json) lists every
changed path. No production source/dependency/ABI/app behavior changed.

A: production admission remains blocked by failed provenance and no demonstrated
incremental quality; no new approval/execution or persistence bypass exists in this
offline path. B: absolute auto-correction and physical-device quality/resources
remain unqualified; no mobile work is authorized. C: faster metric implementations
and broader task research are optional future work, not implementation authority.

Final exact evidence-SHA certification is pending until both full workflows and
every required job complete/SUCCESS. The final engineering receipt records actual
SHA/run/attempt/job evidence without circular self-reference. No earlier SHA can
certify a later commit. No unexecuted test is PASS; no tests were weakened, skipped
or optionalized, and no rerun-until-green was used. This is Round 3 evidence only,
not Step 7 closure or model admission.

Local validation corrections: the existing text-hygiene gate initially decoded
the new PNG fixtures as UTF-8. It now admits only manifest-listed CC0 PNG paths
with exact SHA-256, bounded size, PNG signature and 224x224 RGB8 headers; altered
bytes and unlisted images have negative tests. Text checks remain intact. The
copied start-marker evidence also needed a terminal newline, and the report's
changed-file inventory had to be generated before link validation. These were
tooling/evidence defects, corrected without changing any frozen experiment input
or rerunning inference. Original ignored run logs/start marker remain preserved.

## Round 2 — Candidate Qualification

Status: AUTHORIZED / IN PROGRESS / ADMISSION BLOCKED. Round 2 engineering
recommendation: **HOLD_FOR_MORE_QUALITY_EVIDENCE**. This does not admit a model or
close Step 7. Step 8 and Phase 8 remain NOT STARTED.

Certified starting SHA `001967ce53276a7eba4166f57f339fcbbec666d6`, main ==
origin/main, clean tracked/staged tree after fetch. Round 1 accepted COMPLETE /
FULL PASS: Foundation 40264527997, Native shells 40264528001, attempt 1,
completed/SUCCESS, respectively 7/7 and 3/3 required jobs.
Ignored prior audit/probe artifacts were preserved.

### Provenance decision

**CURRENT_ARTIFACT_PROVENANCE_FAILED**. Exact twenty-item matrix and primary-source
links: [provenance evidence](evidence/phase-7-step-7-round2-provenance.json).
Official upstream `google/embeddinggemma-2` revision
`914f7f89142e33e77833254d9c9b90c3cef7303b`; converted repository
`litert-community/embeddinggemma-2-text-vision-440m-litert-lm` revision
`e301f74d5551b0c2641bd5cb4652a76239d5c5f8`.
Current artifact 387,710,976 bytes, SHA-256
`92dcbea108899e5d6e30d919b0744f90d9967e80c67a4ab5503ac16d54f62eb0`.
Locally hashed embedded tokenizer matches the official published tokenizer hash.
Official original checkpoint hash is published, not locally verified; original
weights were not downloaded. Apache-2.0 is declared upstream; canonical license
text identity is recorded. Conditional upstream redistribution permission does not
establish the converted artifact's missing derivation/attribution chain.

Converter identity/revision/argv, optimization, environment, intermediates and
parity are unproven. Quantization descriptions alone do not establish conversion
lineage. Clean-source conversion is **CONDITIONALLY_REPAIRABLE_NOT_DEMONSTRATED**:
official pinned source/config and generic LiteRT Torch module export provide an
engineering route, but inspected converter revision
`c677a8069d63fc86a6d14e11168dfc387fd9357c` has an older 300M text-only example;
its HF task enumeration has no embedding task for this text+vision candidate.
No ready-made pinned working 440M pipeline or QAT reproduction is claimed.
No full conversion or new weight acquisition occurred. A bounded wrapper/export/
parity experiment would be needed before claiming a reproducible rebuild path.

### Semantic experiment and results

[Frozen method](../benchmarks/intelligence-qualification-v1.md),
[raw scores/results](evidence/phase-7-step-7-round2-results.json) and
[environment](evidence/phase-7-step-7-round2-environment.json) define the evidence.
96 procedural cases, eight explicit goals, positive/opposite/neutral/boundary/
combination/conflict variants, two repeats; labels and policy frozen before model
scoring. No realistic reviewed photos: **INSUFFICIENT REALISTIC QUALITY EVIDENCE**.

Candidate policy: each tool 0 selected positives, 0 correct directions, 100% false
negative rate. True abstention is 6/6 for exposure, brightness, contrast, shadows,
saturation and temperature; 5/6 highlights; 8/8 sharpen. One highlights conflict
false positive (+500 milli-EV); zero invalid/nonfinite/unbounded outputs. All 96
outputs repeat exactly, maximum embedding component difference zero. No emitted
positive calibration samples: all candidate parameter errors are null/unmeasured.
Combination recovery 0/16; conflict abstention 15/16. The frozen policy is
UNQUALIFIED for every tested category; no category is QUALIFIED.

| Tool | Baseline selection / positives | Baseline correct direction | Baseline FP / negatives | Baseline mean absolute native-unit error | Metric-only correct direction / positives; FP |
|---|---:|---:|---:|---:|---:|
| Exposure | 6/6 | 4/6 | 6/6 | 333.333 milli-EV | 5/6; 0 |
| Brightness | 6/6 | 4/6 | 6/6 | 66.667 milli-linear | 4/6; 0 |
| Contrast | 6/6 | 4/6 | 6/6 | 166.667 milli-stops | 4/6; 6 |
| Highlights | 6/6 | 4/6 | 6/6 | 333.333 milli-EV | 2/6; 5 |
| Shadows | 6/6 | 4/6 | 6/6 | 333.333 milli-EV | 2/6; 5 |
| Saturation | 6/6 | 4/6 | 6/6 | 133.333 milli-ratio | 1/6; 3 |
| Temperature | 6/6 | 2/6 | 6/6 | 666.667 kelvin | 2/6; 3 |
| Sharpen | 4/4 | 4/4 | 8/8 | 0 milli-amount | 4/4; 4 |

The baseline simply honors the explicit request and has no automatic abstention.
Sensitivity margins 0/0.01/0.02 did not establish a useful bounded policy. A clearly
post-hoc diagnostic ignoring the neutral prototype ranks temperature direction
correctly 5/6 and softness 4/4; this was not used to alter goldens or policy and is
not qualification. Those two signals are PROMISING / MORE EVIDENCE REQUIRED at
the evidence level only. Remaining categories have no demonstrated policy value.
Blur, rotation and crop were not inferred or tested and are NOT SUITABLE FOR THIS
PROVIDER FAMILY under the authorized eight-tool intelligence subset.

Meaningful product improvement over the reference baseline is **not demonstrated**.
The candidate abstains more but misses every positive. The small raw ranking
signals and synthetic-only corpus do not establish family-wide unsuitability.
Hold pending realistic reviewed evidence and proof of a repairable conversion
pipeline; do not spend on mobile production integration or call this a rebuild
approval. Final admission thresholds/decision remain with Principal Architect.

### Host cost, security and boundaries

Load 2.7447 s; 24 text prototypes 3.2175 s; first image 0.8943 s; 191 warm images
median 0.4375 s, p95 0.4660 s, range 0.4159–0.4941 s. Read-only calibration median
0.0540 s. Process peak RSS 748,568,576 bytes; after inference RSS 714,088,448
(anonymous 367,005,696; file-backed 347,082,752); after close RSS 230,432,768.
These include Python/corpus/runtime overhead, not exact allocation leases, mobile
budgets or physical-device certification. No cold OS-cache guarantee. CPU only;
Android/Apple physical memory, thermal, pressure and package impact NOT VERIFIED.

A: provenance gate remains blocking; useful semantic qualification and realistic
quality evidence remain absent. No new authority bypass found in offline tooling.
B: clean converter proof/parity, reviewed natural-photo corpus and later physical
device evidence required. C: prototype/neutral calibration research may be useful
only under a new reviewed evaluation version; no tuning of this frozen run.

No production code, runtime owner, dependencies, app adapters or frozen contracts
changed. No document/history mutation or approval/candidate capability exists in
the offline runner. No network inference, free-form NLP, weights, production model
registration or model bundle was added. Schema 3 and all existing Phase 4/5 caps
and segmentation semantics unchanged. Production model UNRESOLVED; EmbeddingGemma
candidate/unadmitted/unintegrated; CPU VALIDATED, Vulkan/Metal UNKNOWN.
DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED.

Changed-file inventory: `.gitattributes` (stable evidence line endings), two
workflow files (add offline regression only),
`scripts/qualify-intelligence.py`, `tests/intelligence-qualification.test.py`, this
report, four `docs/benchmarks/intelligence-qualification-v1*` files and three
`docs/reports/evidence/phase-7-step-7-round2-*` JSON files. No tests removed,
weakened, skipped or optionalized. Exact evidence-SHA certification remains pending
until both full workflows complete and every required job succeeds. The final
engineering receipt records the actual SHA/run IDs without a circular self-hash.

## Historical Round 1 implementation record

Status: AUTHORIZED / IN PROGRESS / ADMISSION BLOCKED.
Remediation implementation certification pending; Step 7 is not complete.
Starting SHA f2dd3ceac7f9f18bf9033b9e398ac703d491e3c5 verified clean tracked/staged
main == origin/main after fetch. Existing untracked probe report is retained below
as historical evidence. Ignored probe/audit artifacts are preserved.

## Authorized foundation

The Principal Architect approved a separate application/process intelligence owner,
not a change to Phase 4/5 caps or segmentation. The internal resource mechanism has
one lazy runtime/session, one global active request, fail-closed checked accounting,
protected Manual/platform reserves, cancellation, client abandonment suppression,
stop/join/unload and repeated lazy initialization. It is not wired to any app,
production adapter, provider, ABI or document. No production numeric policy exists.
Synthetic adapters exist only in the certification executable, never application
release targets. Clients do not own a heavyweight model. No editor/history API is
included by the owner. Physical device admission remains outstanding.

Offline provenance tooling rejects missing/unknown/noncanonical fields and checks
exact local final artifact names, sizes and hashes. Structural completeness never
certifies evidence truth, licensing or production admission. No weights are fetched.
Candidate-neutral paired quality measurements use the same explicit bounded input
for baseline/candidate, without expected-label exposure. Tiny generated fixtures
prove scoring mechanics only; thresholds remain pending Principal Architect and
no image-quality/model qualification is claimed.

B-01 root cause: standalone Zig script maintained an obsolete explicit source list.
It now delegates to the existing CMake-based Windows GNU gate, preserving canonical
sources, consumers, warnings and retained tests instead of appending one symbol.
B-02: final Android release APK is now scanned after assembly, including packaged
native/DEX/resource bytes and UTF-16 markers. Unreviewed assets/model suffixes,
ambiguous ZIP paths, encryption and oversized content fail closed. Negative ZIP
fixtures prove rejection on each relevant packaged surface. Existing host/iOS
production scanning remains unchanged.

## Findings, admission and boundaries

A-01: additive architecture/mechanism established; no claim that EmbeddingGemma
fits a production resource policy. OS adapters, physical budgets/measurements and
actual runtime accounting remain mandatory before admission.
A-02: provenance gate established; candidate conversion evidence remains incomplete.
B-01/B-02 repairs require the exact-SHA evidence below before remediation is certified.
B future work: approved physical-device policy, complete candidate provenance,
independent quality corpus/thresholds and subsequent separately authorized model
integration. C optional: broader diagnostic/performance reports.
No editing/approval/publication authority defect was introduced or redefined.

EmbeddingGemma 2 remains CANDIDATE ONLY; production model UNRESOLVED. No production
provider, LiteRT dependency, model assets, free-form NLP or cloud inference added.
CPU VALIDATED; Vulkan/Metal UNKNOWN. U2NETP unapproved/unincorporated/rights unresolved.
DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED.
Intent/Structured Intent/Plan/Registry/Envelope/Provider, reference planner,
approval/candidate/stale/publication, Schema 3, shared manual/history, ADR 0009 and
Phase 4/5 trust/resource semantics unchanged. Step 8 and Phase 8 NOT STARTED.

## Certification

Implementation and any subsequent documentation SHA require their own full
Foundation 7/7 and Native shells 3/3 completed/SUCCESS evidence. No Step 7 closure
commit is created in this round. The final engineering receipt records the actual
SHA and observed run IDs without self-referential commit claims. Pending checks
are not PASS. This record will be reconciled with actual local results before push.

## Preserved historical preflight evidence

Qualification: Phase 5 DOES have separately admitted inference contexts. Their
frozen segmentation profile does not admit this candidate. The historical statement
about no inference class applies to Phase 4 raster work classes, not absence of
Phase 5 inference admission. RSS is not exact owned allocation accounting.

# Phase 7 Step 7 — production intelligence admission preflight

Status: **ADMISSION_BLOCKED**. Step 7 is not complete. Production model remains
**UNRESOLVED**. Step 8 and Phase 8 are NOT STARTED.

## Baseline and scope

Clean `main` and `origin/main` were verified at
`f2dd3ceac7f9f18bf9033b9e398ac703d491e3c5`, the certified Step 6 closure.
Its Foundation run 40198995481 completed SUCCESS 7/7 and Native application
shells run 40198995512 completed SUCCESS 3/3 (baseline evidence only).
This investigation acquired public upstream artifacts into ignored
`build/phase7-step7-audit`; nothing was incorporated into production assets.

## Candidate and provenance

Architect-selected candidate: EmbeddingGemma 2, text+vision 440M. No substitute.
Official model: https://huggingface.co/google/embeddinggemma-2,
revision `914f7f89142e33e77833254d9c9b90c3cef7303b`.
The upstream API declares Apache-2.0. Its raw `model.safetensors` is
1,488,915,288 bytes, published LFS SHA-256
`197a32965d4b1105faf060417baa899e193fb73cd401f42ec9295234d5553d79`.
The raw checkpoint was NOT downloaded or locally hash-verified.

The Google AI Edge documentation links the converted distribution:
https://huggingface.co/litert-community/embeddinggemma-2-text-vision-440m-litert-lm,
revision `e301f74d5551b0c2641bd5cb4652a76239d5c5f8`.
Downloaded `embeddinggemma-2-text-vision-440m.litertlm`: **387,710,976 bytes**,
locally verified SHA-256
`92dcbea108899e5d6e30d919b0744f90d9967e80c67a4ab5503ac16d54f62eb0`.
The model card declares Apache-2.0 and int4 QAT text/embedder, int8 QAT vision.
Exact conversion toolchain/reproduction provenance has not been completed;
this is not a completed production licensing/provenance admission.

Runtime probe: `litert-lm-api==0.18.0`, Linux x86_64 wheel
`litert_lm_api-0.18.0-py3-none-manylinux_2_27_x86_64.whl`, 47,051,760 bytes,
locally verified SHA-256
`b64e2cf6d7dcb90ff094b74af595cc5d53faa07e0889f967d15df8d3e696b53c`.
The wheel was extracted locally, not installed globally. Official runtime source
v0.18.0 resolves to `b2f686e2ed4718fb84ec398a61dd59ca0f0aff27`.
Runtime Apache-2.0 license was retrieved. No runtime entered application builds.

## Actual CPU evidence

Environment: Linux WSL2 x86_64, glibc 2.43; CPU backend, one thread,
vision CPU backend, audio backend disabled. The physical container contains text,
vision, embedder and associated weights, with **no audio model sections**.
The SDK nevertheless reports audio support from inherited audio-preprocessor
metadata (confirmed against its source); no model bytes were modified.

A project-generated 256x256 RGB PNG gradient was used solely as a resource probe,
not a quality benchmark. SHA-256:
`242a3632ec44b281443ff13c0642612102585b623101c74a09a61ca070b5a6cf`.
Actual inference returned 768 finite embedding components with L2 norm
1.0000004133093. No editor, approval issuer, candidate or history was accessible.

| Measurement | Observed |
| --- | ---: |
| Engine initialization in image probe | 0.302525 seconds |
| First image embedding in that process | 1.158499 seconds |
| Process peak RSS (`ru_maxrss`) | 657,223,680 bytes |
| `/proc` high-water RSS | 657,956,864 bytes |
| Anonymous resident memory after inference | 278,863,872 bytes |
| File-backed resident memory after inference | 379,092,992 bytes |
| RSS after engine close | 225,345,536 bytes |

A separate initialization-only process previously took 2.541648 seconds.
These are single-process observations, not a latency distribution, sustained-run
benchmark, cold filesystem-cache measurement, mobile footprint, or admission PASS.
Model asset bytes, mapped address space, process RSS and charged owned capacity
are distinct measurements and must not be equated.

## Blocking resource decision — A finding

[Budget admission v1](../contracts/budget-admission-v1.md) freezes a maximum
256 MiB native context capacity and requires truthful simultaneous allocation
accounting, including relevant platform/native work. It defines no inference
work class. The image probe's peak RSS is approximately 627.5 MiB; anonymous
residency alone is approximately 266 MiB, before reserving editor working space.
RSS includes the host process and is NOT a precise native-owned allocation charge.
Consequently this probe does not prove a specific budget-request rejection, but
there is no demonstrated bounded admission path for this runtime under the
existing authority. Shipping an unaccounted separate allocation owner or raising
limits would conceal rather than resolve that gap.

Principal Architect decision required: define/approve an explicit inference
resource ownership/admission strategy and acceptable measured mobile budgets,
while preserving manual editing and the existing editing ceilings. Alternatively,
require a demonstrably compliant artifact/configuration of the same authorized
candidate. This report does not authorize either change or a substitute model.

The existing Phase 5 package limits (64 MiB/file, 128 MiB aggregate) concern its
frozen segmentation package/profile. They have NOT been silently repurposed as
an embedding profile, nor treated as proof that every application asset must fit
that format. Similarly, 388 MB does not by itself establish a mobile store-size
violation. Actual APK/AAB/iOS impact remains unmeasured.

## Work deliberately not claimed

No production provider, admission manifest, prototype/calibration policy or new
normative admission contract was frozen while resource authority is unresolved.
No quality thresholds/results, false-positive/abstention rates, Android/iOS
runtime tests, accelerator tests, release package sizes, or complete retained
Step 7 certification are claimed. No Step 7 SHA/workflow certification exists.
The retained baseline results do not certify this report.

Local evidence is in ignored `build/phase7-step7-audit`: pinned metadata,
`container-sections.json`, `cpu-preflight.json`, `cpu-inference-preflight.json`,
probe scripts and logs. Large artifacts and raw runtime logs are not committed.

## Findings and unchanged boundaries

A: unresolved resource-admission ownership/budget decision above; admission stops.
B: after that decision, complete conversion provenance, actual mobile integration,
quality/calibration benchmarks, lifecycle/security tests, package measurements and
full exact-SHA certification. These are unfinished Step 7 gates, not waived work.
C: optional additional performance characterization after the blocking gate.

No production code, public ABI, test behavior or workflow changed. Model evidence
cannot approve or execute. Intent, Structured Intent, Plan, Registry, Envelope,
Provider authority, Assist/Auto/review/lifecycle contracts, Phase 6 approval and
candidate/publication semantics, Schema 3, shared manual/history, ADR 0009 and
Phase 5 trust remain unchanged. No cloud/network inference, silent runtime model
download, arbitrary model loading or free-form chat/NLP architecture was added.
Public artifact acquisition for this audit is not an application network feature.

Production model remains UNRESOLVED; U2NETP unapproved/unincorporated, rights
unresolved. CPU release trust VALIDATED; Vulkan/Metal UNKNOWN.
DH-APPLE-METAL-01 remains DEFERRED / NOT WAIVED / NOT SATISFIED.
Hosted evidence does not constitute physical certification.

Changed tracked file: `docs/reports/phase-7-step-7.md` only.
Final admission state: **ADMISSION_BLOCKED**. Step 8 and Phase 8 NOT STARTED.

## Round 1 changed-file inventory

- .github/workflows/foundation.yml
- .github/workflows/native-shells.yml
- AGENTS.md
- ARCHITECTURE.md
- CMakeLists.txt
- DECISIONS.md
- docs/contracts/intelligence-runtime-resource-admission-v1.md
- docs/contracts/production-intelligence-provenance-evaluation-v1.md
- docs/plans/phase-7-roadmap.md
- docs/reports/phase-7-step-7.md
- packages/core/Package.swift
- packages/core/src/intelligence_resources.cpp
- packages/core/src/intelligence_resources.hpp
- packages/core/tests/intelligence_resources_test.cpp
- QUALITY_GATES.md
- ROADMAP.md
- scripts/check-android-release.py
- scripts/check-native-zig.ps1
- scripts/evaluate-intelligence.mjs
- scripts/intelligence-admission.mjs
- tests/android-release-scanner.test.py
- tests/intelligence-admission.test.mjs
- tests/native-document.test.mjs
- tests/persistence.test.mjs

## Local validation before certification

Retained full Windows CMake/Zig suite: 53/53 PASS, including preserved core UBSan. Retained active ASan/UBSan
full suite: 55/55 PASS, including negative instrumentation probes; final resource
boundary changes additionally pass 49 assertions under the sanitizer build.
Full Linux source suite after delegation/header correction: 172/172 PASS; added
recorded-output CLI plus final tooling checks: 4/4 PASS. Final exact-SHA CI remains
authoritative for the complete final source set. Existing real release APK scan
passes (12 native libraries, 2 DEX files); negative packaged fixtures reject.
That local APK is prior build evidence, not certification of the new SHA. The new
CI step must scan the freshly assembled final release. actionlint and diff checks PASS.
Initial local failures were root-caused: missing initializer_list include; legitimate
Android baseline-profile assets; authority title placement; old source assertions
expecting handwritten Zig inventories. Fixes preserve canonical CMake/test authority,
allow only exact legitimate baseline-profile formats, and retain all prior gates.
No test skip/optionalization, sanitizer suppression or rerun-until-green.

No remediation closure commit or Step 7 admission is claimed here. Actual implementation
SHA and workflow observations are recorded by the final engineering delivery.

## Round 1 cleanup remediation

Initial implementation: 787cf45c0c8a2d66f3fab96ded011d05c7a02e26. Foundation run
40262829420 completed SUCCESS 7/7, attempt 1. Its Native shells run 40262829381
was still in progress when final review found a new foundation cleanup defect.
These results do not certify the correction below.

Cancellation or stop could overwrite RESOURCE/FAILED status before cleanup tested
that status, retaining a runtime whose run had failed. A deterministic rendezvous
now cancels an admitted request immediately before its injected allocation failure.
The test failed against the initial implementation (session destruction missing).
The correction retains execution_failed independently of final status precedence,
so cancellation may win the reported outcome while failed sessions are destroyed.
No frozen approval/candidate/edit semantics are involved or changed. The focused
ASan/UBSan resource corpus now passes 51 assertions; focused source checks pass 8/8.
The new correction SHA requires both complete workflows independently, with no
rerun of the old SHA and no old-SHA substitution. Actual SHA/run IDs are recorded
in final engineering delivery. Step 7 admission remains blocked.
