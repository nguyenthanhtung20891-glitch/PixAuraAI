# Inference provider software certification v1

Status: FROZEN for Phase 5 Step 7, under Product Owner Option A. This contract
certifies the frozen implementation/profile, not a production segmentation model.
Production model selection remains UNRESOLVED. Step 8 and Phase 6 NOT STARTED.

## Four independent evidence fields

Every record distinguishes software-suite result, hosted/simulator evidence,
physical evidence, and release trust. Software-suite PASS with release UNKNOWN is
valid. Neither this document nor a JSON record is runtime trust authority. No
runtime loader, trust file, new signing authority or dynamic self-certification.

| Provider/version | Platform | Release trust | Physical inference evidence |
| --- | --- | --- | --- |
| cpu.reference.v1 / 1 | Windows, Linux, Apple host, Android arm64-v8a/armeabi-v7a | VALIDATED (1), unchanged | No physical certification claim |
| android.vulkan.v1 / 1 | Android Vulkan compute/F32 | UNKNOWN (0), unchanged | Not observed by Step 7 unless separately recorded |
| apple.metal.v1 / 1 | Apple Metal/F32 | UNKNOWN (0), unchanged | NOT OBSERVED; CERTIFIED forbidden |

Availability remains observed and separate from trust. UNKNOWN=0, validation=1,
physical certification=2 retain existing semantics. Synthetic 0/1/2 adapters are
test-only. A future accelerator promotion needs separately approved evidence
policy or existing explicit authority; Step 7 creates neither. Device names,
vendors, OS versions and latency cannot confer trust. DH-APPLE-METAL-01 remains
DEFERRED and mandatory at its existing later boundaries.

## Package, model and arithmetic authority

All execution begins with Step 1 canonical file-set/manifest/trust/signature/
digest verification, then immutable ownership, semantic preparation and checked
execution. Signed purpose-2 engineering fixtures use the same verifier only in
certification builds. Production store remains empty and rejects test packages;
positive certification-build execution is not positive release-package execution.
No unsigned bypass, raw model path or release test key is allowed.

Only pixaura.onnx.reference-segmentation.v1: IR 9, default-domain opset 13, one
attribute-free 1x1 Conv, no bias, RGB quarter-step nonnegative weights summing to
one. Signed normalization, F32/NCHW shapes, finite [0,1] probabilities unchanged.
CPU ordered binary64 followed by explicit RN32 remains semantic authority.

CPU goldens are exact binary32 words, independently derived from dyadic integer
arithmetic. Every accelerated output element must satisfy its accepted bound:

* Vulkan: `abs(Vulkan - CPU) <= 4e-38 + 3e-7 * abs(CPU)`.
* Metal: `abs(Metal - CPU) <= 4e-38 + 3e-7 * abs(CPU)`.

The identical formula does not imply identical backend floating-point behavior.
Full output arrays and tensor metadata are compared; no sampling replaces them.
The shared Step 7 corpus covers all 15 weight triples at 256x256 plus retained
1x1, 2x2, 8x8, 1024x1024 and normalization fixtures. Each executes zero, one,
gradient, checker and IEEE boundary vectors (near 0, 0.5, 1; rounding; subnormal).
The first four additionally use an independent integer/dyadic oracle on every
pixel. Retain all previous four exact CPU goldens and backend boundary tests.

Threshold is independently `probability >= 0.5` on each provider's returned
probability. When `abs(CPU - 0.5) > tolerance(CPU)`, classifications must match.
Inside that closed band, parity must still hold but classifications may differ;
no bit-identical mask claim is made. The exact example CPU 0.5 versus accelerator
binary32 word 0x3effffff is accepted by both bounds with different classifications.
Tests assert this explicitly. No threshold shift, snapping or CPU substitution.

## Required security and ownership matrix

| Area | Mandatory retained/additive evidence |
| --- | --- |
| Package | Full Step 1 tamper/key/domain/file/path/canonical/size/license corpus; 96 seeded single-bit mutations across manifest/signature/payload, seed 0x705ec001 |
| Model | All 32 signed Step 2 graphs plus 15 signed Step 7 malformed fixtures: wire types, nesting, truncations, nonfinite/negative F32 weights, topology; retained opset/op/attribute/external/dynamic/overflow rejection |
| ABI/input | Checked size/version/reserved/enums, rank/dimensions/type/layout/bytes/normalization; NaN, both infinities, below/above range; null and invalid overlaps; uint32/uint64 boundaries; no clamp |
| Output | Full finite/range checks before registration; injected NaN, Inf, negative, >1, invalid final element; backend completion/readback failure; exact output-copy sizes and tensor metadata |
| Lifecycle | Zero/wrong-kind/cross-context/stale/released/malformed/exhausted serials, double release, context destroyed before access; no serial wrap/reuse; verified package survives release |
| Cancellation/races | Pre-cancel, bounded checkpoints, after completed GPU tile, concurrent cancellation/retirement at publication; no successful partial/stale publication; independent contexts/owners |
| Resource | CPU Step 2 limits, Vulkan 1638400-byte and Metal 1114112-byte transient reservations; tiles <=16384 pixels; exact/one-byte resource boundaries, checked addition overflow, dimension/product limits, recoverable failure and accounting restoration |
| Admission | Retained 180-row trust/tier/platform/thermal/memory matrix; UNKNOWN/NORMAL/ELEVATED/SEVERE/CRITICAL, capability epoch revocation, latest generation, exact three-observation recovery, BUSY without queue |
| Fallback | Pre-start unavailable/trust/capability/model/resource fallback only; security/invalid/stale requests reject; started unavailable/RESOURCE/CANCELLED/OUTPUT_INVALID/INTERNAL return without CPU replay |
| Isolation | No release adapters/keys/fault hooks; production-configuration object scans; C/JNI/Swift checked shared native ownership, no independent Kotlin/Swift domain semantics |

Direct GPU fault cases perform real submission/completion and cleanup, then an
injected failure; they do not constitute observation of physical device loss.
Step 7 output faults are inserted after backend completion, before full validation.
An invalid final element represents partial/invalid output, not an observed short
driver readback. Backend interfaces accept fixed private buffer capacities, not a
model-supplied output count; output sizes are derived from the validated graph.
Driver-internal allocations remain opaque, not a measured RSS guarantee. No hard
real-time cancellation guarantee. Actual context destruction requires quiescence;
tests do not race destruction against live C calls contrary to the owner contract.

Started synthetic adapter errors count accelerator invocations and CPU executions
independently: exactly one failed accelerator invocation and zero CPU executions.
Real release descriptors remain UNKNOWN: retained native-platform integration must
automatically select CPU even with otherwise eligible platform capability facts.
Dynamic capability changes do not mutate compiled provider descriptors.

## Fixed code authenticity

Model bytes provide only bounded parsed weights/metadata, never Vulkan shader
source, SPIR-V, Metal source, AIR, metallib or machine code. Vulkan uses repository
`platforms/android/native/segmentation.comp`, built by glslc from pinned NDK
28.2.13676358. `scripts/check-inference-spirv.cmake` requires NoContraction,
F32 and excludes RelaxedPrecision/FMA. No runtime compilation of model code.
Apple uses fixed `packages/core/swift/Sources/Shaders/segmentation.metal`, embedded
reproducibly by `scripts/metal-inference-shaders.mjs`; Metal 2.4 safe math, SDK
compile gates and unchanged private ARC worker. Runtime library creation consumes
only that fixed source. Tool/SDK versions observed in CI logs remain evidence,
not a newly invented pinned Xcode/compiler or shader signing architecture.

## Acceptance and artifacts

`tests/fixtures/inference-certification-v1.json` is a bounded engineering matrix,
never runtime/persisted authority. PXSEC v1 runtime lines identify provider,
complete vectors/elements/checks, threshold-band population, failures and unchanged
release trust. A suite pass requires all counters' failure totals zero AND all
retained gates; no unavailable backend is a parity pass. Implementation identity
is provider version 1 plus exact final source SHA, not the version string alone.

Require complete Foundation boundaries (all seven jobs) and Native application
shells (all three jobs) SUCCESS on the same final Step 7 SHA. Required observed
execution includes Windows/Linux native, ASan/UBSan with negative probes, Android
both supported ABIs/JVM/instrumentation, Apple native/Swift macOS/iOS Simulator,
iOS app/UI, retained Phase 0–4 and Steps 1–6, workflow lint and diff whitespace.
Hosted macOS and iOS Simulator remain software/integration evidence only.
Historical Z Fold7 exposure 14/14 at ade92bea17561a99d0fe3275cbfc75c21fd07dae is
supporting backend evidence only and insufficient for Phase 5 inference trust.
No fresh physical execution is implied by hosted success.

A: current certification/security correctness and required evidence. B: production
model/runtime/operator work and deferred physical certification. C: optional tuning
or extra hardware measurements. Fix A only. No Schema 3 persistence, production
model, weights, external runtime, AI UX, Step 8 or Phase 6 work.
