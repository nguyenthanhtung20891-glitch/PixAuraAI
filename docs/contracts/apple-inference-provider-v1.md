# Apple Metal inference provider v1

Status: FROZEN for authorized Step 4, Apple-specific Option A approved.
Provider `apple.metal.v1`, version 1, backend 3 (Metal), F32/NCHW binary segmentation.
This is NOT general ONNX or Core ML execution. No new third-party dependency.

## Compatibility and authority

Only `pixaura.onnx.reference-segmentation.v1`: ONNX IR 9, default opset 13,
one attribute-free 1x1 Conv, no bias; three quarter-step weights in [0,1] summing
to one; exact signed tensor metadata and version-1 runtime/API/operator floors.
Existing CPU preparation validates the entire graph before preparing this backend.
Mandatory CPU compatibility is retained. Shared verification, immutable snapshots,
normalization, input/output validation and generation/publication fences remain
authoritative. No raw model path or package-supplied executable code.

Approved Apple-specific clarification permits the authenticated CPU ONNX portable
semantic record to authorize this exact backend/profile. No Metal record/field,
alternate payload, canonical/signature change or package schema extension. Core ML
and LiteRT records remain unchanged. Package compatibility grants no Metal trust.

Selection is explicit: CPU or Metal. No automatic fallback/admission or duplicate
T0/T1/T2/pressure logic; Step 6 owns policy. Descriptor availability observes Metal
only, validation UNKNOWN, deterministic flag 0 (bounded equality); max dimension
1024 and context ceiling 256 MiB unchanged. Hosted/simulator is not certification.

## Kernel and Apple numerical authority

Fixed reviewed `segmentation.metal`, embedded reproducibly from repository source;
the same source compiles under all three Apple SDK gates. Metal language 2.4,
safe math and precise math functions (legacy fastMathEnabled=false). Runtime
library creation uses ONLY fixed repository code, as the retained exposure path
already did. No model-dependent source selection/generation or compiler service.

F32 storage only. Explicit fma(x,w,0) implements each rounded product; then
fma(r,1,g), fma(rg,1,b) implement the two ordered rounded adds. This fixes rounding
points with explicit built-in operations rather than relying on implicit compiler
contraction across expressions. No FP16, hidden clamp, sigmoid or normalization
inside the shader. Shared normalization runs once before dispatch.

Apple-specific acceptance, frozen before execution evidence:

`abs(Metal - CPU) <= 4e-38 + 3e-7 * abs(CPU)`.

Derivation is independent of Android despite the same resulting constants.
Safe F32 operations are compared to the unchanged ordered binary64/RN32 CPU
authority. Do not assume a selectable arithmetic RTE mode or denormal preservation:
allow adjacent F32 rounding (including RTZ) and subnormal flushing. With the exact
quarter-weight constraint there are at most two inexact arithmetic operations
along a term path, plus CPU's final half-ULP rounding; combined relative error is
below 3e-7. Three lost subnormal terms are bounded by 3*FLT_MIN <4e-38. All terms
are nonnegative, so cancellation cannot amplify relative error. Fixed zero/one
FMA operands add no new mathematical operation to the graph.

Apple references: [safe math](https://developer.apple.com/documentation/metal/mtlmathmode/safe),
[Metal language specification](https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf),
floating-point sections 8.1-8.5. Runtime parity tests cover all weight triples,
rounding/subnormal vectors and supported shapes on the actually observed host.
Every tile is checked in shared C++ before publication; invalid/nonfinite/out-of-
range/readback/parity output => OUTPUT_INVALID, never CPU substitution. Threshold
remains >=0.5; values within tolerance of the threshold may yield different masks.
CPU's exact goldens and independent oracle are unchanged.

## Resources and lifecycle

`src/apple/metal_worker.mm` is one ARC hardware owner reused by old exposure Swift
and the new provider. Swift no longer duplicates device/queue/buffer ownership.
The internal ARC target is not an external dependency. Its interface accepts no
model path/source; public hardware C functions expose exposure only, not raw Conv.

One private device/queue/pipeline per execution; two storageModeShared buffers
<=256 KiB each, <=512 KiB total. Fixed 16-byte count/weight push block. At most
16384 pixels/tile, two packing/readback arrays <=512 KiB total. Reserve additional
64 KiB adapter control: backend transient bound 1114112 bytes. No extra upload/
readback device allocation, worker pool, persistent pipeline/model cache or global
mutable owner. Driver-internal overhead is opaque, not an RSS guarantee.

Model metadata includes this bound in execution_bytes; shared checked reservation
precedes backend allocation under the unchanged context ceiling. At max dimensions,
private input 12 MiB + output 4 MiB, plus bounded backend and normal context/model
ownership. One execution/context, independent contexts retain independent workers.

Commands retain buffers/encoder state through completion before any readback or
release. Shared pre-cancel and checks before/after each tile save subsequent work;
submitted Metal work is not preempted. Final shared generation/cancellation/model
retirement fence prevents stale publication. No hard real-time completion promise.
Later requests and CPU state remain valid after request/backend failure.

Unavailable -> PROVIDER_UNAVAILABLE; allocation/admission -> RESOURCE;
pipeline/command/platform failure -> INTERNAL; bad readback/parity -> OUTPUT_INVALID.
No raw NSError or platform codes escape. Test-only injected failure after submitted
work waits for completion and proves cleanup, not actual physical device loss.
Package trust and immutable model bytes are never changed by backend failure.

## Evidence and boundaries

Native C++ and Swift detached tests call the same shared checked provider API.
Corpus retains 32 ONNX rows/four goldens and tests 21 valid models, all 15 weight
triples, 1x1/2x2/8x8/256x256/1024x1024, resource/handle/context/cancellation faults.
Release excludes test trust and corpus entry points. Shared ASan/UBSan remains
mandatory; no sanitizer exemption. Android provider and CPU semantics unchanged.

Hosted macOS / iOS Simulator execution is integration only. Metal absence is an
unavailable result, not a fabricated parity PASS. No physical evidence is claimed.
DH-APPLE-METAL-01 remains DEFERRED and required before Phase 11 hardening, Phase 12
Beta completion, production release or any physical Apple GPU certification claim.
Schema 3 unchanged. No AI UX, Step 5+ or Phase 6.
