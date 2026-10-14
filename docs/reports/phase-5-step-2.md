# Phase 5 Step 2 - provider abstraction and CPU reference

Status: IMPLEMENTED; complete exact-final-SHA workflows govern acceptance.
Baseline: 8d5a30338c97a05e987ecf1324b86a2afb795356. Step 1 COMPLETE / FULL PASS:
Foundation 38491704459 (7/7), Native application shells 38491704500 (3/3).
Only Step 2 authorized; Steps 3+ and Phase 6 NOT STARTED. Eight-step roadmap retained.

[Provider contract](../contracts/inference-provider-v1.md),
[CPU/ONNX profile](../contracts/cpu-reference-provider-v1.md),
[frozen roadmap](../plans/phase-5-roadmap.md).

## Audit and accepted decision

ADR 0005 freezes ONNX interchange and eventual CPU/XNNPACK fallback, but the repo
incorporates no inference engine. Step 1's existing payloads are deliberately
non-executable signature fixtures. Reinterpreting them as executable custom data
would bypass the accepted package contract. Product Owner approved Option A:
dependency-free cpu.reference.v1 with genuine, bounded ONNX parsing/execution.
No general ONNX or protobuf runtime is incorporated, and no general ONNX support
is claimed. General runtime/model selection remains separately authorized work.

Reuse: Step 1 verifier/trust/digests/owned snapshots; ADR 0009 caller-owned contexts
and fresh identity/serial handles; existing evaluation cancellation/checkpoint
vocabulary; shared C/JNI/Swift certification boundaries. Inference does not reuse
preview requests or duplicate Phase 4 tier/thermal/backend/pressure policy.

## Frozen execution

Profile pixaura.onnx.reference-segmentation.v1: ONNX IR 9/default-domain opset 13,
one attribute-free 1x1 Conv, one F32 raw-data RGB weight initializer, one static
RGB input and one probability output. Parsed coefficients from {0,.25,.5,.75,1}
sum to 1; model B changes weights and output, proving interpretation rather than
hard-coded outputs. Full message/graph validation precedes preparation/execution.
Unknown fields/operators/attributes/storage/topology/dynamic/external/sparse/
training semantics reject. Genuine ONNX outside this profile is not executable.

Owned nonrecursive parser: <=4096 payload bytes, <=128 fields/message, fixed
nesting <=7, one node/input/output/initializer, zero attributes/temporary tensors.
Minimal checked varints, lengths, singular duplicates and tensor products reject.
Rank 4, dimensions 1..1024, <=4194304 elements; signed shapes must match graph.
No allocation is driven directly by a payload length.

Step 1 normalization remains signed positive millionth scales, zero bias and
range [0,1]. Binary32 decode/conversion uses explicit IEEE words, including
subnormals and ties-even; binary64 multiplication/division normalizes, then ordered
RGB weighted accumulation and binary32 conversion produce probability. No fast
math, FMA contraction, transcendentals, approximate SIMD or platform libm sigmoid.
FE_TONEAREST required. Invalid/nonfinite input and output reject without clamp.
Reference parity is bit-exact IEEE binary32 words; mask threshold >=0.5. This
synthetic graph proves semantics, not trained subject/background quality.

## ABI, ownership and cancellation

Static descriptor cpu.reference.v1/version 1, CPU/reference, binary segmentation,
F32/NCHW, available without GPU, reference validation (no hardware certification).
Provider-neutral prepared-backend interface and bounded static registry separate
the CPU Conv implementation from shared normalization, validation and publication.
Versioned/sized C request/tensor/result/resource structs; bounded caller buffers,
enum/reserved/alias checks and exception containment. Numeric statuses 0..18 are
frozen in the provider contract; no vendor/parser error codes become authority.

Context-owned verification delegates to Step 1, then returns a non-pointer verified
package token. Preparation accepts that token, retains immutable bytes and signed
metadata, and returns a prepared model token. No execution accepts raw paths,
unsigned bytes or caller trust. Production store stays empty pending approved key
provisioning. Signed test packages are admitted only inside certification code.
Legacy Step 1 opaque-pointer exclusive-release semantics remain unchanged; internal
metadata retention is an additive handoff, not a package/trust/format change.

Fresh caller identity per context lifetime and monotonic kind/serial tokens reject
cross-context, stale, forged-kind and double release. Acquired snapshots retain
lifetimes; package release after prepare is safe. Init/destroy require quiescent
exclusive caller access. One execution per context, independent contexts, no pool,
singleton provider, daemon or persistent cache. Context resource ceiling 128 KiB
through 256 MiB; 128 KiB control, <=32 handles, 4 KiB prepared metadata/model,
actual package data plus conservative 64 KiB package control, exact input/output
array capacities. Scalar execution scratch/temporary tensors zero. At maximal
profile dimensions input 12 MiB + output 4 MiB; retained outputs continue counting.
No Phase 4 ceiling raised; final capability-aware admission remains Step 6.

Increasing request generations, pre-cancel and generation-qualified active cancel
are frozen. Checks at admission, allocation, each 1024 pixels and publication.
Cancellation/release prevents output publication and never mutates model bytes.
Deterministic checkpoint tests cancel/release after 1024 real pixels, verify no
result, and prove later requests work. No artificial sleeps. Consumers must fence
their own current request before display; completed outputs are immutable proposals,
not project history or approval.

## Fixture and validation evidence

32 signed genuine/near-ONNX package rows, including 6 valid profiles and negative
wire/varint/length/count/shape/opset/operator/attribute/storage/external/sparse/
training/topology cases. Four compact zero/one/gradient/checker goldens encode
model digest, dimensions, quarter-unit inputs and expected IEEE words. Independent
integer oracle computes (R+2G+B)/16; no interpreter-generated golden authority.
Tests also exercise 1x1/1024x1024, signed normalization, subnormals/ties, null/alias/
rank/type/byte/range/NaN/Inf errors, output validation, repeat/stale/double release,
two-context concurrent interleaving, cancellation/release fences and resource cleanup.

Observed local Windows full retained suite 37/37 PASS, followed by current affected
inference/package/C/release checks 6/6 PASS after CPU-only refinements. Linux and
ASan/UBSan affected checks 6/6 PASS; a further local full WSL run was stopped in
unchanged editor fault tests, so no local complete retained Linux/sanitizer PASS
is claimed for that interrupted run. Full retained suites remain mandatory in
the exact-SHA hosted workflows. Windows source 94/94, Linux source 130/130 PASS,
zero failures/skips; focused docs/dependency/fixture checks 8/8 PASS. Android
debug/release, all three ABIs, JVM/lint and instrumentation APK build PASS, with
current final native corpus rebuilt before push. Native release objects and actual
release products exclude test anchors
and both certification entry points. Test seed is public RFC8032 engineering-only
material; no production private keys, signing or trust-injection surface.

Tiny-fixture timings and owned reservations are emitted by certification logs:
4 pixels, input 48 bytes, output 16 bytes, scratch 0, conservative retained/context
reservations reported separately. Debug/certification time is rough correctness
evidence, not production inference performance or model-quality claims.
Observed local debug 4-pixel execution was roughly 2..38 microseconds (cold calls
vary); sanitizer calls were roughly 12..496 microseconds. Retained reservation
201433 bytes includes conservative control/package/model ownership plus 16 output
bytes; private 48-byte input adds to the transient peak, then is released. These
are tiny synthetic measurements, not RSS, trained-model latency or optimization.

Required: complete Windows/Linux/Apple native, active ASan/UBSan, Android arm64/
ARMv7 builds and JNI instrumentation, Swift macOS/iOS simulator, retained Phase
0-4 and full Step 1 security regressions, source/workflow lint and diff hygiene.
Local Apple/instrumentation execution is not claimed. Both final exact-SHA hosted
workflows must finish SUCCESS (Foundation 7/7, shells 3/3) on the same final commit.
Delivery records SHA/run IDs/test totals after observing them; no later uncertified
status commit or prior-SHA promotion. Unobserved/skipped required gates never PASS.

## Classification and stop

A: implementation/current full platform validation and exact-SHA certification.
No unresolved architecture conflict; initial execution choice explicitly approved.
B: general production model/runtime coverage, accelerated providers (Steps 3/4),
segmentation quality (Step 5), capability admission (Step 6), provider parity
certification (Step 7). No Step 9 or automatic next-step authorization.
C: existing retained UI timing sensitivity; no unrelated UX changes.

Schema 3 unchanged; no project inference state, remote inference/download, telemetry,
general ONNX runtime, NNAPI/Core ML/Metal/GPU inference, AI UX or Editing Agent.
DH-APPLE-METAL-01 unchanged. Steps 3+ and Phase 6 NOT STARTED. Stop after Step 2.
