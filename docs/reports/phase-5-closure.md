# Phase 5 closure — local inference foundation

Committed candidate status: **PENDING_EXACT_SHA**. Steps 1–7 are accepted COMPLETE /
FULL PASS. Step 8 is authorized at `b46b30c26af0790d0715bc48d24093b83290b8fe`.
Phase 6 NOT STARTED. Production model UNRESOLVED.

Phase 5 becomes **CLOSED / COMPLETE / FULL PASS** only after BOTH complete
workflows pass on the final Step 8 SHA. Until then Step 8/Phase 5 closure is
pending. `scripts/phase5-closure.mjs --certify` materializes the exact final SHA,
run IDs/results and all eight completed step states in an observed engineering
receipt under `build/`. Final delivery links that receipt after checking GitHub.
The committed [manifest v1](phase-5-closure-v1.json) is a candidate/inventory,
not evidence that pending execution happened. This avoids a self-referential SHA
or an additional status commit without its own certification. No receipt is
runtime trust authority, a signed attestation or document Schema 3 data.

## Frozen final scope

Signed local model-package verification; immutable verified package/model
ownership; provider-neutral checked inference; cpu.reference.v1,
android.vulkan.v1 and apple.metal.v1; binary segmentation reference profile;
deterministic CPU arithmetic; bounded accelerated execution; compact synthetic
segmentation evaluation; Phase 4 capability-aware admission and pre-execution
fallback; provider parity/security suites; explicit trust separation; exact-SHA
certification. No new product capability is added by Step 8.

| Step | Status before closure receipt | Accepted final SHA | Key contract | Key evidence | Deferred items |
| --- | --- | --- | --- | --- | --- |
| 1 Model Package & Manifest Contract | COMPLETE / FULL PASS | 8d5a30338c97a05e987ecf1324b86a2afb795356 | model-package-v1 | Foundation 38491704459; shells 38491704500 | Production key provisioning/artifact rights |
| 2 Provider Abstraction & CPU Reference Provider | COMPLETE / FULL PASS | 58202caf1fd86215abe4650b208b2134cdca9b58 | inference-provider-v1; cpu-reference-provider-v1 | Foundation 38605609607; shells 38605609516 | Broader profile/runtime only if approved |
| 3 Android Local Inference Provider | COMPLETE / FULL PASS | bf31eb33d5b5027236d71cf56da4c648d6b56255 | android-inference-provider-v1 | Foundation 38685139496; shells 38685139454 | Inference physical evidence/trust promotion |
| 4 Apple Local Inference Provider | COMPLETE / FULL PASS | a3ad7e7a61030992d933b3ca68572f83604dd5a9 | apple-inference-provider-v1 | Foundation 38730904780; shells 38730904823 | Physical Metal gate/trust promotion |
| 5 Compact Segmentation Evaluation | COMPLETE / FULL PASS | 59d846967fffef7e735597f6b65301841659336f | segmentation-evaluation-v1 | Foundation 38999701451; shells 38999701458 | Production model UNRESOLVED; checkpoint rights |
| 6 Capability-Aware Admission & Fallback | COMPLETE / FULL PASS | 5592cd839c2607379c6cb8c02a4b42a44e80311d | inference-admission-v1 | Foundation 39015611339; shells 39015611451 | No future profile/runtime promise |
| 7 Provider Parity & Security Certification | COMPLETE / FULL PASS | b46b30c26af0790d0715bc48d24093b83290b8fe | inference-provider-certification-v1 | Foundation 39038860025; shells 39038860056 | Accelerator trust/physical evidence remain separate |
| 8 Phase 5 Closure | PENDING_EXACT_SHA; COMPLETE / FULL PASS only after receipt | FINAL_WORKFLOW_HEAD_SHA, resolved in receipt | phase-5-closure-v1 engineering inventory | Fresh full Foundation 7/7 and shells 3/3 on that SHA | B/C items below; Phase 6 NOT STARTED |

Every historical row above has both complete workflows SUCCESS (7/7 and 3/3).
Those accepted histories are not Step 8 execution. Receipt step 8 resolves to the
actual closure commit, not the Step 7 baseline; no invented historical SHA.

## Final contract inventory and agreement audit

All eight version-1 contracts are retained byte-for-byte (LF-normalized digests in
the manifest): [package](../contracts/model-package-v1.md),
[provider-neutral](../contracts/inference-provider-v1.md),
[CPU reference](../contracts/cpu-reference-provider-v1.md),
[Android](../contracts/android-inference-provider-v1.md),
[Apple](../contracts/apple-inference-provider-v1.md),
[evaluation](../contracts/segmentation-evaluation-v1.md),
[admission](../contracts/inference-admission-v1.md), and
[software certification](../contracts/inference-provider-certification-v1.md).
No accepted ADR or contract is superseded by closure. ADR 0005's broad future
runtime directions remain distinct from the approved dependency-free narrow
Phase 5 implementation; ADR 0022 remains signature authority. ADR 0009 ownership
and the existing DH decision remain applicable.

Historical contract scope markers (such as Step 3/4 explicit execution and
"Step 6 owns policy") describe when features landed, not competing current
fallback engines. Step 1's operator/converted-format registry is declarative;
it is not executable coverage. Step 6 owns current automatic policy, and Step 7
Option A explicitly preserves software PASS versus release UNKNOWN. No semantic
conflict was found. No accepted contract needs rewriting.

| Provider / version | Accepted software suite | Release trust | Physical inference evidence |
| --- | --- | --- | --- |
| cpu.reference.v1 / 1 | PASS at Step 7; mandatory again on closure SHA | VALIDATED | No physical certification claim |
| android.vulkan.v1 / 1 | PASS at Step 7; mandatory again on closure SHA | UNKNOWN | NOT OBSERVED; emulator software/integration only |
| apple.metal.v1 / 1 | PASS at Step 7; mandatory again on closure SHA | UNKNOWN | NOT OBSERVED; hosted macOS/iOS Simulator software/integration only |

No provider is CERTIFIED. Availability, software-suite result, physical evidence
and release trust remain separate fields. No trust promotion policy, device-name
rule, benchmark authority or runtime self-certification is created.

## Package, semantics, evaluation and admission

Step 1 retains canonical bounded positional manifests, shared pure Ed25519
(reviewed libsodium 1.0.22), key/trust/purpose validation, payload digests/file sets,
no-follow path safety, license/provenance declarations and immutable snapshots.
Production store v1/generation 1 has ZERO anchors and fails closed. Test purpose-2
anchors/fixtures/adapters exist only in certification builds and cannot grant
production authority. Positive test-build execution is not positive release-key
execution. No raw model path, unsigned acceptance or trust shortcut.

Profile stays `pixaura.onnx.reference-segmentation.v1`: ONNX IR 9, default opset 13,
one attribute-free 1x1 Conv without bias, three nonnegative quarter-step RGB weights
summing to one, F32/NCHW, frozen signed normalization and foreground probabilities.
CPU remains ordered binary64 arithmetic with explicit binary32 conversion/RN-even,
exact goldens and independent oracle. Foreground remains >=0.5.

Vulkan bound: `abs(Vulkan - CPU) <= 4e-38 + 3e-7 * abs(CPU)`.
Metal bound: `abs(Metal - CPU) <= 4e-38 + 3e-7 * abs(CPU)`.
Independent derivations remain intact. Full vectors/shape/type/domain are checked.
Classification must agree outside the closed CPU tolerance band; inside, the
documented rounding disagreement is allowed. No snapping or CPU substitution.

Evaluation retains IoU, Dice/F1, pixel accuracy, precision and recall as checked
integer rational counts, explicit both-empty/one-empty behavior, 24 deterministic
synthetic cases and independent goldens. No production-quality threshold or claim.
Production model UNRESOLVED is an accepted Phase 5 outcome, not a closure failure.
U2NETP: AUDITED; UNSUPPORTED BY CURRENT PROFILE; UNAPPROVED; UNINCORPORATED;
CHECKPOINT RIGHTS UNRESOLVED. No weights, measured quality or fabricated timing.

Shared Phase 4 authority retains T0/T1/T2, pressure handling, material epochs and
exact three-observation recovery. Priority remains Android Vulkan -> CPU, Apple
Metal -> CPU, portable CPU. Automatic acceleration requires accepted provider trust
plus Phase 4 permission; UNKNOWN descriptors keep current automatic release
selection on CPU. PRE-EXECUTION FALLBACK ONLY. Started errors return shared failure,
never silent CPU replay. Security/invalid/stale/cancelled requests cannot use
fallback. BUSY rejects without queue. Stale epochs reject/release/re-admit; latest
admission generation and final publication fences remain unchanged.

## Audited resource and lifecycle bounds

| Bound | Unchanged value and meaning |
| --- | --- |
| Inference context | 131072..268435456 bytes; 131072-byte control reservation |
| Owned tokens / execution | <=32 typed handles; one pending OR running admission/context; nonwrapping serials |
| Explicit prepared model | 4096-byte conservative control lease, package ownership retained |
| Admission preparation | 8192-byte conservative model/plan/control lease; not an additional 4096 lease |
| Admission safety | 917504 bytes additional room + 131072 control = Phase 4 1048576-byte reserve |
| Tensors | Rank 4, dimensions <=1024; generic <=4194304 elements / 16777216 bytes |
| Reference max shape | 1024x1024 pixels; private input 12582912 bytes, output 4194304 bytes |
| CPU backend scratch/transient | 0; exact input/output are separate accounted leases |
| Vulkan transient | 1638400 bytes: <=1 MiB device allocation requirements +512 KiB packing/readback +64 KiB control |
| Metal transient | 1114112 bytes: <=512 KiB shared GPU buffers +512 KiB packing/readback +64 KiB control |
| GPU tile | <=16384 pixels, fixed private capacities and code; no model-supplied executables |
| Reference ONNX parser | <=4096 bytes, <=128 fields/message, fixed nesting <=7; no recursive graph execution |
| Package | Manifest <=65536, signature exactly80, <=8 payloads/10 entries; <=64 MiB/payload, <=128 MiB total payload |
| Package/control totals | Data <=134283344 bytes; 65536-byte verification/control floor; verification ceiling <=256 MiB |

Admission reserves max(native control + input + output + backend, signed declared
peak), plus already retained package/control/output occupancy and existing safety
room. Fixture 16 MiB declared memory is a floor, not measured RSS. Dynamic cap
publication and lease checks remain serialized. No cross-context global RAM
allocator, driver RSS guarantee or real-time GPU preemption claim. Borrowed caller
buffers are the caller's budget responsibility. Release accounting, stale/wrong-
kind/cross-context handles, double release and independent contexts remain tested;
actual destruction requires quiescence, fresh identities and accessible storage.

## Retained certification and physical limitations

Retain full package mutations, malformed signed ONNX, ABI/invalid tensor/input,
invalid output (including final element), stale/cross-context handles,
cancellation/publication fencing, integer overflow/resource/backend failure,
no-security-fallback and zero post-start replay. Step 7 per-provider suite:
105 complete vectors / 10159065 probabilities / 36727185 checks, zero failures;
45 package rows +96 deterministic mutations; 32+15 signed ONNX rows;
180-row admission/trust matrix with 3749 checks, including synthetic 0/1/2 trust.
Production object scans retain test-key/adapter/corpus exclusion.

DH-APPLE-METAL-01 = DEFERRED, NOT WAIVED, NOT SATISFIED. Mandatory before Phase 11
hardening acceptance, Phase 12 Beta readiness completion, production release or
any physical Apple GPU certification claim. Hosted/simulator results cannot
satisfy it. Historical Z Fold7 Vulkan exposure 14/14 at
`ade92bea17561a99d0fe3275cbfc75c21fd07dae` is supporting/scope-specific only,
insufficient for Phase 5 inference release certification. No fresh physical claim.

## Deferred work and Phase 6 boundary

A: closure consistency, regressions and complete fresh final-SHA evidence; fix A
only. B: production segmentation model selection; broader operator/runtime
architecture only if approved; production artifact provenance/license/checkpoint
rights; accelerator release-trust promotion policy; physical Apple Metal and any
later release-policy physical inference certification. C: optional measurements,
tuning/research. No B/C implementation or architecture change is included.

A later authorized Phase 6 may rely on verified packages, provider-neutral checked
inference, CPU semantic authority, Android/Apple local backends, evaluation harness,
shared capability/admission, pre-execution-only fallback, certification evidence
and trust-state separation. None grants model viability, AI candidate approval or
product feature authority. No planner, intent parser, recommendation system,
Editing Agent runtime, AI UI or background-removal product flow is implemented.

Schema 3 unchanged. No weights. No external runtime. No AI UX. No Editing Agent.
Phase 6 NOT STARTED. No Step 9+. STOP AFTER PHASE 5 CLOSURE.
