# Phase 5 Step 4 - Apple local inference provider

Status: IMPLEMENTED; VALIDATION IN PROGRESS; complete exact-final-SHA workflows govern closure.
Baseline: `bf31eb33d5b5027236d71cf56da4c648d6b56255`.
Steps 1-3 are accepted COMPLETE / FULL PASS. Step 3 exact-SHA evidence:
Foundation 38685139496 (7/7), Native application shells 38685139454 (3/3).
These are historical Step 3 results, not Step 4 implementation/certification.
Only Step 4 is authorized. Steps 5+ and Phase 6 are NOT STARTED; eight steps only.

## Audit findings

1. `packages/core/swift/Sources/MetalValidation.swift` contains the existing
   private Metal hardware worker: device, command queue, compute pipeline and
   two storageModeShared buffers. It uploads, encodes, commits, waits for command
   completion, checks status/error, then reads back. ARC retains submitted
   resources. Calls are worker-only and synchronous; no global mutable GPU owner.
   The exposure diagnostic currently creates a private worker per evidence call.
2. `docs/contracts/gpu-tile-v1.md` bounds existing tiles at 16384 pixels and two
   <=256 KiB shared buffers (<=512 KiB total). This is a suitable ownership and
   tiling basis for a fixed F32 reference Conv, without a general inference engine.
   Extract/reuse hardware ownership rather than duplicate it. New inference
   allocations must be reserved under the existing inference context budget;
   the old standalone diagnostic does not perform that admission automatically.
3. `packages/core/swift/Sources/Shaders/exposure.metal` is fixed repository code.
   The existing worker loads that bundled source with Metal language 2.4 and safe
   math / fastMathEnabled=false, depending on OS. `scripts/check-metal-shaders.sh`
   already compiles AIR/metallib for macOS, iPhoneOS and iPhoneSimulator with
   -fno-fast-math and warnings as errors. A new fixed Conv must not accept source,
   functions or executable bytes from model packages. No general shader compiler,
   graph compiler or runtime code generation is proposed.
4. The current physical exposure diagnostic deliberately rejects Simulator.
   Preserve that physical-certification path unchanged. Any Step 4 hosted or
   Simulator provider invocation must be separate integration evidence, not a
   way to promote the exposure diagnostic or discharge DH-APPLE-METAL-01.
5. `AppleCapability.swift` probes MTLCreateSystemDefaultDevice availability and
   explicitly supplies gpu_validation=0. Simulator provenance is virtual; other
   API observations remain unobserved for certification. Shared Phase 4 policy
   retains tier, pressure, budget and publication authority. No inference
   selection/fallback policy belongs in this Swift service during Step 4.
6. `packages/core/src/inference.cpp` has the shared prepared-backend interface,
   context resource leases, checked handles, input normalization/validation,
   cancellation and complete-output publication. Its static registry currently
   exposes CPU and, on Android, Vulkan. There is no Apple inference backend.
   Reuse these semantic owners; add only a hardware adapter and minimum checked
   boundary after architecture approval. Swift must not parse ONNX or redefine
   normalization, parity, probabilities or statuses.
7. Existing Swift/C inference tests call the shared CPU certification entry point
   on a detached task. `packages/core/Package.swift` separates the C++ authority
   from the Swift Metal hardware layer, links Apple's Metal framework, bundles
   repository shaders, and targets iOS 16 / macOS 13. No OS minimum increase is
   justified by this audit. Shared C++ and Swift package build wiring both need
   consideration for the later adapter; no bridge has been implemented yet.
8. The shared CPU ONNX parser already enforces IR 9, default-domain opset 13,
   one attribute-free 1x1 Conv, no bias, constrained quarter weights and static
   tensor semantics. Step 3's additional fixtures cover all 15 weight triples;
   they and the four independent CPU goldens can be retained for Apple tests.
   No Apple parser, new model payload or operator coverage is needed for Metal.
9. Repository search found no Core ML execution implementation, MLModel runner
   or CoreML import in the native/Swift/iOS execution sources. Core ML exists as
   accepted package-format declarations and future architectural intent. Using
   that record would require an actual separately authenticated converted model,
   retained binding and runtime adapter; it is not an existing reusable shortcut.
10. The repository's Apple gates already cover native C++, Swift macOS, unsigned
    iOS package build, Simulator, application debug/release and ABI/UI tests.
    This Windows audit executes none of those Apple gates and makes no new
    execution, timing, numerical-parity or physical-hardware claim.

## Accepted Apple-specific Option A

Product Owner explicitly approves `apple.metal.v1` consuming the authenticated
CPU ONNX record ONLY for `pixaura.onnx.reference-segmentation.v1`. Mandatory
CPU compatibility is retained, not physical CPU placement. Metal compatibility,
availability, validation and certification remain separate. No new package field,
provider enum, alternate payload, canonical/signature or schema extension. Core ML
and LiteRT records unchanged. No automatic provider selection/fallback.

## Implemented contract and evidence

See [frozen Apple contract](../contracts/apple-inference-provider-v1.md).
One native ARC Metal resource owner now serves the retained exposure diagnostic
and shared inference provider; Swift performs no model/tensor/parity semantics.
Fixed repository shader embedding is reproducible; all three SDK shader gates
compile exposure and segmentation. No model-supplied source/code, general runtime
or new third-party dependency. Internal SwiftPM ARC target preserves private
linkage through existing Xcode partial-link gates. Platform minimums unchanged.

Apple's safe F32 explicit rounded fma product/add chain is checked against CPU
under abs(error)<=4e-38+3e-7*abs(CPU), independently derived for Metal's allowed
rounding/subnormal behavior, not inherited from Vulkan. GPU evidence is pending;
the contract is frozen before observation. All 15 weight triples and rounding/
subnormal cases are included. CPU arithmetic and Android provider unchanged.

Bounded backend payload/control reservation 1114112 bytes, admitted within the
same context ceiling. Commands complete before readback/release; common generation,
cancellation and retirement fences prevent stale publication. No persistent cache,
global mutable GPU state or policy duplication. Backend errors map to shared
unavailable/resource/internal/output-invalid statuses and leave verified data intact.

Local Windows/Linux/ASan-UBSan affected package/provider/C/release checks 6/6 each
PASS. Complete Linux source suite 132/132 PASS, zero skips; focused source/security
and independent Metal rounding/FTZ checks 15/15 PASS; actionlint PASS. Retained
Android debug/release, all three packaged ABIs, JVM/lint and instrumentation APK
build PASS. Android instrumentation and hosted Apple native/Swift/Simulator/
application gates must still be observed on the final workflows.
No unexecuted gate is PASS. Both complete workflows must succeed on the same final
Step 4 SHA; delivery records final SHA/IDs/totals without a later uncertified
status-only commit. Timing/memory observations are engineering integration evidence,
never real-device speedups or physical certification.

## Classification and scope

A: implementation review and complete platform/security/sanitizer/exact-SHA gates.
B: Step 5 compact model evaluation, Step 6 admission/fallback, Step 7 broader
certification, Step 8 closure. C: optional physical measurements; none observed.
Initial package interpretation decision is resolved; no silent scope extension.

Schema 3 unchanged; no AI UX, Core ML, Step 5+ or Phase 6. DH-APPLE-METAL-01
remains DEFERRED and mandatory before Phase 11 hardening, Phase 12 Beta readiness,
production release or any physical Apple GPU certification claim. Hosted macOS
and iOS Simulator evidence cannot discharge, waive or reinterpret that gate.
