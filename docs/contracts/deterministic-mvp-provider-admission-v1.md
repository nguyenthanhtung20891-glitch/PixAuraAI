# Deterministic MVP Assistance Provider admission v1

Phase 7 Step 7 Round 5. Admission evidence/selection, not a new Provider version.
State: IMPLEMENTATION_CERTIFIED / CLOSURE_CERTIFICATION_PENDING. Only observed full implementation and closure certification
can activate final admission/closure. Step 8 and Phase 8 remain NOT STARTED.

## Identity and product claim

Production MVP Assistance Provider: **reference.assistance.v1**, Provider contract 1,
capabilities 7. This is the existing ReferenceProvider, not a new implementation.
**reference.deterministic.v1** identifies the separate frozen reference planner.
The Product Owner/Principal Architect selection supersedes Round 4's search
recommendation; learned model search stops for this MVP.

PRODUCTION LEARNED MODEL: **NONE REQUIRED / NONE ADMITTED FOR PHASE 7 MVP**.
A provider is not a model. Future learned providers remain DEFERRED / REPLACEABLE.
No arbitrary-photo diagnosis, scene aesthetics, white-balance understanding,
beautification, natural-language interpretation or photographic quality is claimed.
Explicit UI goals/values and canonical machine-semantic requests are the input.
Intent v1 prose remains non-interpreted. Round 3/4 baselines remain OFFLINE ONLY.
EmbeddingGemma remains disqualified; NIMA/FFCC/MobileIQA are not selected/integrated.

## Unchanged authority chain

Explicit request -> provider -> UNTRUSTED Envelope v1 -> native validation ->
Structured Intent/planner or Plan v1 -> Registry v1 -> review -> explicit HUMAN Apply
-> existing bound single-use approval -> private candidate/execution -> at most
one ordinary revision -> shared history/undo/redo. Advice returns no Plan and never
mutates. Use suggestion remains navigation-only at the existing manual default.
No hidden prefill, Apply, retry, replan, reapproval or alternate-provider execution.

The provider accepts Request data only. Its header/interface exposes no document,
editor, approval issuer, candidate, callback, history, filesystem, environment,
shell, plugin loader, network or model runtime capability. Trusted validation is
separate: successful envelope encoding alone does not certify semantic validity.
Invalid untrusted candidates may be returned by the wrapper and must fail validation.

## Resource and lifecycle admission

Existing editor Service lazily owns one small ReferenceProvider shared by that
editor's Assist/Auto paths. Separate editors may own separate small providers.
This is intentional bounded lightweight ownership, not duplication of a model.
No process-global heavyweight runtime Owner is required or instantiated.
Intelligence Runtime Resource Admission v1 remains unchanged for future learned work.

Bounds: request structured/advice payload 2048 bytes, Plan payload 8192 bytes / 8
actions, binding 512 bytes, IDs 64 ASCII bytes, Envelope 12288 bytes. Reference
planner emits exactly one action; valid Plan candidates preserve up to eight in
exact order. No semantic expansion, repair or clamping. Native parser finite shape,
field/string/array limits remain final; JSON trees/output have size bounded by those
inputs. The caller owns input/output bytes; the provider retains only two mutexes,
nonwrapping epoch and stopped flag. No dynamic cache/container/handle, worker, queue,
model mapping, tokenizer, runtime session or GPU/device allocation exists.

One synchronous request per instance via try-lock; contention returns RESOURCE/BUSY.
Per-editor integration gates also prevent competing Assist/Auto/review work. No
unbounded global work scheduler is introduced. Cancel fences an in-flight result,
is nonsticky; stop is permanent. Owners obey stop -> join ALL calls -> destroy.
No callback can outlive ownership. Manual/source/history changes invalidate results
through existing source serial/generation/context and repository publication fencing.

Allocation tests count requested C++ allocation bytes/calls for request and trusted
validation separately over all 117 positive envelope cases. These cumulative totals
upper-bound simultaneous requested C++ payload bytes for those calls, but exclude
allocator metadata/stack/OS memory and are NOT RSS, a production lease, or a proof
that every hostile payload uses that measured maximum. Input/parser bounds provide
the structural bound; existing allocation-failure sweeps prove safe RESOURCE exits.
The allocation executable measures the shipping ReferenceProvider layout without
PIXAURA_ASSISTANCE_TESTING. The assistance executable's test_owner_bytes includes
its two checkpoint pointers and must NEVER be reported as production object size.
Retained provider state is shipping sizeof(ReferenceProvider), ABI-dependent, plus normal
mutex implementation internals; it owns no persistent heap member. Caller-retained
requests/results and editor snapshots are not mislabeled as provider caches.

## Admission criteria and evidence

| Criterion | Required evidence / interpretation |
|---|---|
| Quality | 39 valid and 42 malformed frozen Structured Intent cases; all eleven tool mappings/min/max/neutral where valid; 117 kind combinations; 22 existing UI typical cases; retained Plan/action/ordering/version/oversize/binding corpus |
| Determinism | Ten exact repeats per golden; independent JSON stream FNV-64 162cbcfb525cd535 (regression fingerprint, NOT a security digest); SHA-256 in manifest; all native hosts assert it; Android/Swift repeat all eleven Plans with fixed live context |
| Latency | Native provider request and trusted validation timed separately; mean/max report includes no image editing; no platform timing estimate is called a measurement |
| Memory | Allocation totals/failure sweep, object size and bounded parser shape; no cache/session/worker; no guessed memory cap or Phase 4/5 change |
| Package | Existing production source identity unchanged; no added runtime dependency/weights. Actual archive/object/package bytes remain measured in retained footprint/package scans; shared-object size is NOT an isolated linked provider delta |
| Battery/thermal | NO MODEL WORKLOAD: bounded synchronous JSON work only, no image/raster/GPU/background processing. Hosted timings plus code review characterize this provider. Whole-app battery and physical thermal behavior are NOT MEASURED and not inferred |
| Android | Same native provider through JNI; all eleven repeated canonical Plans, NOT_APPROVED without HUMAN Apply; retained Compose/lifecycle/manual/failure/instrumentation suite and both ABIs |
| Apple | Same C/native provider through Swift; all eleven repeated canonical Plans; retained macOS/Simulator/iOS app/UI and lifecycle/manual/failure suites; hosted software evidence only |
| Trust | Assistance Provider admission is distinct from Phase 5 backend trust; cpu.reference.v1 VALIDATED, Vulkan/Metal UNKNOWN |
| License/provenance | First-party provider/codec/planner/catalog source hashes and certified source SHA; existing shared native/SQLite/codec/crypto dependencies governed by ThirdPartyNotices and pinned project records; no third-party learned artifact, conversion or redistribution obligation introduced |
| Privacy/offline | Data-only synchronous interface; no network, upload, API key, telemetry, process environment, shell/filesystem scan or dynamic plugin path |
| Failure/fallback | Invalid/version/stale/resource/cancel/stop outcomes fail closed; Manual remains independent; no approval/candidate/document/history effect; no retry or stale resurrection |
| Release isolation | Native production-object scans, final Android APK native/DEX/assets scan and final Apple release app scan, with rejecting negative fixtures; research/model/test authority markers absent |
| Authority | Existing Phase 6 exact Plan/HUMAN approval/single-use/candidate/publication remains unchanged; retained no-approval/replay/race/atomicity/history tests |
| Final certification | Both full workflows independently completed/SUCCESS, each required job SUCCESS, exact implementation SHA and separate exact closure SHA; no old-SHA substitution |

No criterion is silently omitted for being model-free. Model-specific artifact/
inference metrics are explicitly not applicable, and unsupported whole-device
measurements remain NOT MEASURED. This does not waive any future learned-provider
or physical GPU admission requirement. DH-APPLE-METAL-01 remains DEFERRED /
NOT WAIVED / NOT SATISFIED. Existing Phase 4/5 256 MiB policies and segmentation
inference semantics are untouched.

## Certification and persistence

docs/admission/deterministic-mvp-provider-v1.json binds exact source identities,
canonical corpus and provider/planner roles. It is governance evidence, not runtime
approval or document persistence. Schema 3 is unchanged; no provider/proposal/review/
approval/candidate authority is persisted. Existing release dependency manifests
and notices continue to govern the implementation's shared dependencies.

Implementation authority remains pending until observed certification. A subsequent
documentation-only closure commit records that certified implementation and requires
both full workflows again. Its observed engineering receipt activates admission and
Step 7 closure only after its exact SHA succeeds. No self-referential SHA or additional
uncertified record commit is needed. Step 8 remains NOT STARTED throughout Round 5.
