# Android Vulkan inference provider v1

Status: FROZEN for authorized Phase 5 Step 3 under approved Option A.
Provider ID `android.vulkan.v1`, version 1, backend enum 2 (Vulkan compute).
No new dependency, model payload, package field or numeric manifest provider ID.
Core ML/LiteRT compatibility records are unchanged.

## Signed compatibility and semantic authority

The authenticated CPU ONNX record is the portable ONNX semantic compatibility
source for this approved alternate backend ONLY when the complete model matches
`pixaura.onnx.reference-segmentation.v1`. The shared CPU preparation/parser checks
all runtime/API/operator floors and the full graph first. CPU compatibility is
mandatory; Vulkan-only packages are not admitted. Signature, digest, license,
normalization, trust and immutable ownership remain Step 1 authority.

ONNX IR 9, default opset 13, exactly one attribute-free 1x1 Conv without bias;
weights are parsed quarter increments in [0,1], summing to one. No additional ops,
attributes, shapes, initializer storage or graph semantics. No raw-path execution.
Input/output remain the checked provider-neutral F32 NCHW RGB/probability contract.
Shared native normalization runs once before backend dispatch. Model bytes never
become shader source, SPIR-V or executable code.

Callers explicitly select CPU or Vulkan. CPU remains separately available.
No automatic selection, fallback, policy duplication or tier/pressure heuristic.
Step 6 retains capability-aware selection/admission. Descriptor availability probes
Vulkan instance/compute limits; it is not pipeline validation or certification.
Vulkan descriptor validation remains UNKNOWN (0), deterministic flag 0 (bounded
numerical parity). Backend 2 means Vulkan compute required; max dimension 1024,
max context ceiling 256 MiB. Availability is not trust, including on physical GPUs.

## Numerical rule, fixed before dispatch evidence

Repository-owned `segmentation.comp`, Vulkan 1.0 SPIR-V built by pinned NDK glslc,
local size 64. Normalized NCHW input is packed into bounded vec4 tile storage.
F32 products R*wR, G*wG, B*wB, then ordered (R+G)+B; GLSL `precise` forbids
contraction. No FP16, fast-math, nonlinear operation or clamp. Shader stores one
foreground probability; padding channels have no semantic meaning.

For every value, require finite probability in [0,1], exact shape/type/bytes and:

`abs(Vulkan - CPU) <= 4e-38 + 3e-7 * abs(CPU)`.

CPU means the unchanged ordered binary64 calculation followed by explicit binary32
ties-even rounding. Vulkan 1.0 does not promise round-to-nearest: allow either
adjacent representable result. Quarter weights summing to one imply either exact
power-of-two products with at most two inexact adds, or one 0.75 product and one
inexact add. Relative allowance bounds those two full-ULP errors and CPU's final
half-ULP rounding (less than 3e-7). The absolute allowance exceeds three FLT_MIN
terms lost to subnormal input/product/sum flushing. Inputs,
weights and partial sums are nonnegative, so there is no cancellation amplification.
This is bounded equality, not bit-exact GPU parity. Mask threshold remains >=0.5;
values within tolerance of that boundary can yield different threshold decisions.
No threshold shift is introduced. CPU bit-exact goldens remain authoritative.
The provider checks this bound against CPU authority before returning each tile;
failure is OUTPUT_INVALID and never substitutes CPU output or partially publishes.
Precision authority: [Khronos SPIR-V environment](https://docs.vulkan.org/spec/latest/appendices/spirvenv.html#spirvenv-precision-operation).
Generated shader inspection requires five NoContraction arithmetic decorations,
one 32-bit float type, and no RelaxedPrecision or FMA instructions.

## Ownership, resources and synchronization

Reuse `vulkan_worker.hpp`, extracted from the existing exposure worker, retaining
its synchronous private device/compute queue/pipeline/command/fence ownership.
Exposure certification retains physical-only filtering and its original shader.
Inference permits software Vulkan solely as honestly labelled integration evidence.
No device name grants validation or physical certification.

At most 16384 pixels/tile. Two mapped host-visible/coherent storage buffers each
256 KiB maximum; each Vulkan allocation requirement <=512 KiB, total <=1 MiB.
Two private packed input/readback arrays total 512 KiB. No additional staging or
device input copies. Three weights and count occupy a fixed 16-byte push block.
Descriptors 2, descriptor set/pipeline/command buffer/fence each 1. Reserve 64 KiB
for adapter control in addition to explicit payload, yielding 1638400 transient
bytes; opaque driver-internal allocations are not a measured RSS guarantee.
Reservation occurs within the existing context ceiling before backend allocation.
Model metadata execution_bytes includes private input/output plus this bound.
At max shape private input 12 MiB/output 4 MiB, plus bounded tiles. No cache.

Host-write/compute and compute/host-read barriers, successful fence completion
before readback, no in-flight buffer reuse. On failure teardown waits for submitted
work before releasing objects; device loss is terminal. Fence wait is 30 seconds,
but driver cleanup may block: no hard real-time cancellation guarantee.
One execution/context; independent contexts own independent queues/resources.
No singleton registry, worker pool, retained JNI reference or background daemon.

Shared pre-cancel, generation checks, model retirement and final publication lock
remain unchanged. Check cancellation before initialization and before/after each
tile. GPU dispatch cannot be preempted; cancelled work is completed/discarded
before resources are released. Later requests remain valid after request failure.

## Failures and evidence

Unavailable instance/device/compute/memory capability -> PROVIDER_UNAVAILABLE;
allocation/admission -> RESOURCE; backend pipeline/submit/fence failure -> INTERNAL;
invalid readback/parity -> OUTPUT_INVALID. No raw VkResult escapes. Unsupported
graph remains MODEL_UNSUPPORTED/MODEL_INVALID, not a package-trust failure.
No mutation of package/model/document semantics on failure; no fallback.

Debug-only argument-free JNI invokes verified signed synthetic fixtures through
the checked shared ABI. Production has no test key/trust injection/corpus API.
Tests cover 32 graphs, four independent goldens, all 15 accepted weight triples,
1x1/8x8/2x2/256x256/1024x1024, normalization, malformed requests, bounded-resource rejection, stale/
cross-context/double release, two-context execution, pre/mid-work cancellation,
model retirement and injected unavailable/allocation/submission/readback failures.
Injected submit failure occurs after real submission and proves teardown ownership;
it is not an observation of physical driver loss. Local debug timing logs separate
total, upload, dispatch/wait and readback; no product speedup claim.

Emulator dispatch/readback/parity is integration evidence only. Historical physical
exposure certification is not inference certification. Fresh physical evidence,
when available, must be separately identified. Schema 3 unchanged; no Apple
provider, AI UX, Step 4 or Phase 6. DH-APPLE-METAL-01 unchanged.
