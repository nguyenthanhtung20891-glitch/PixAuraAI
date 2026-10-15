# Phase 5 Step 3 - Android local inference provider

Status: IMPLEMENTED; VALIDATION IN PROGRESS; exact-final-SHA workflows govern closure.
Baseline: `58202caf1fd86215abe4650b208b2134cdca9b58`.
Step 2 accepted: Foundation 38605609607 (7/7), application shells 38605609516
(3/3), complete SUCCESS on that baseline. Those are historical Step 2 results,
not Step 3 certification. Only Step 3 is authorized. Steps 4+ and Phase 6 are
NOT STARTED. The frozen eight-step roadmap remains unchanged.

## Repository audit

- `platforms/android/native/gpu_vulkan.cpp` owns the existing synchronous Vulkan
  instance/device/compute queue, two mapped coherent storage buffers, descriptor
  set, fixed shader/pipeline, command buffer and completion fence. It is currently
  a private exposure-validation worker, not a reusable inference service. Extract
  and reuse its ownership implementation rather than duplicate device/queue code.
- `platforms/android/native/CMakeLists.txt` compiles repository GLSL using the
  pinned NDK glslc, targeting Vulkan 1.0 at build time, then embeds generated
  SPIR-V. A second fixed reviewed convolution shader can use this same strategy;
  model data must never supply code. No new external runtime appears necessary.
- Existing GPU tiles have at most 16384 pixels; each mapped buffer is at most
  256 KiB, admitted device memory at most 1 MiB. Tiled inference can retain these
  bounds without increasing any Phase 4 ceiling. Extraction must preserve the
  existing exposure shader, physical-device filtering and historical tests.
- `packages/core/src/inference.cpp` has one static backend, `cpu.reference.v1`,
  and a private prepared-execution interface. Shared code already owns signed
  normalization, input validation, resource leases, handles, cancellation,
  generation fencing, output validation and publication. The Android adapter
  should join that registry/interface, retaining these shared authorities.
- `packages/core/src/onnx_profile.cpp` is the existing bounded graph authority:
  ONNX IR 9, opset 13, one attribute-free 1x1 Conv. Reuse this parser and its
  validated numeric plan. Do not implement an Android graph parser or expand ops.
- Step 1 retains immutable verified bytes and the authenticated CPU provider
  binding internally. Production trust is still empty and rejects test anchors;
  existing synthetic admission is certification-only. No new raw-path execution
  or caller-supplied trust API is appropriate.
- `platforms/android/native/capability_bridge.cpp` probes bounded Vulkan instance/
  device/queue availability only. The Phase 4 capability owner has no production
  certification setter. AVAILABLE must remain distinct from VALIDATED/CERTIFIED;
  Step 6 retains provider-selection, pressure and fallback policy.
- The existing JNI CoreProbe certification boundary exercises shared native
  semantics. Extend only the minimum debug/test invocation; keep model, tensor,
  status and publication semantics out of Kotlin. No AI UI is needed.
- Android policies are minSdk 26, targetSdk 36, NDK 28.2.13676358, arm64-v8a and
  armeabi-v7a retained, with x86_64 for emulator integration. No policy change
  is needed. At audit time adb reported one emulator and no physical device.
- Accepted Z Fold7 evidence is historical exposure certification (14/14) at
  `ade92bea17561a99d0fe3275cbfc75c21fd07dae`, not Step 3 inference evidence.
  Existing physical GPU certification rejects software Vulkan. Any new emulator
  inference invocation must be separately labelled integration-only and must
  not weaken or reuse that physical certification authority.

## Accepted signed-provider compatibility decision

The frozen [model package v1 contract](../contracts/model-package-v1.md) defines
signed provider IDs 1 CPU ONNX, 2 Core ML, 3 LiteRT. Payload format IDs are coupled
to those records; provider runtime/API version floors are provider-specific.
`model_package.cpp` rejects unknown IDs. No Vulkan compatibility record exists.
The current binding retains only the CPU record for execution preparation.

The Product Owner approved Option A after this audit: the signed CPU ONNX record
authenticates portable ONNX semantics, not physical CPU placement. Its allowance
to `android.vulkan.v1` is restricted to the exact frozen reference profile.
Mandatory CPU compatibility remains; Vulkan trust stays separate.

Accepted option A: explicitly approve the CPU ONNX record as an authenticated
ONNX compatibility source for both `cpu.reference.v1` and
`android.vulkan.v1`, restricted to the exact frozen reference profile and version-1
runtime/API/operator floors. CPU compatibility remains mandatory. No new payload,
manifest bytes, provider numeric ID, verification relaxation or automatic backend
selection. This clarification is explicitly authorized by the Product Owner.

Rejected alternative B: introduce an explicitly versioned signed Vulkan compatibility
record with independent runtime/API floors referencing the same ONNX payload.
This provides separate signer authorization but changes the frozen package schema,
provider/format coupling and fixtures. It requires explicit package-contract
architecture approval before implementation; do not add an ad hoc ID 4 to v1.

Neither option adds a general inference dependency or expands ONNX operators.
The decision is resolved; no package-schema extension is authorized or implemented.

## Implemented contract

See [frozen Android contract](../contracts/android-inference-provider-v1.md).
Stable ID `android.vulkan.v1`; shared parsing and normalization; one reviewed F32
Conv shader using the parsed three coefficients. Reuse bounded Vulkan tiles,
mapped coherent storage, barriers, fences and terminal-failure cleanup. Account
all adapter/native/device allocations within the inference context before
allocation. No persistent cache, background pool, automatic fallback or final
capability admission policy. Keep CPU explicit and independently available.

GPU error bound frozen before execution: abs(error) <=4e-38+3e-7*abs(CPU).
It accounts for constrained quarter-weight arithmetic, adjacent representable
rounding, CPU's final RN32 and subnormal flushing. CPU bit-exact goldens unchanged.
Every GPU tile is checked against that reference before shared publication.
Generated SPIR-V inspection observed five NoContraction decorations, F32 only,
three products and two ordered adds, no RelaxedPrecision/FMA. No hidden clamp.

Required evidence remains both Android ABIs, actual JNI instrumentation, CPU vs
Vulkan corpus/parity, bounded resources, cancellation, context isolation and
backend-failure cleanup, retained Step 1/2 and Phase 0-4 regressions, active
ASan/UBSan, Apple/Swift retained gates, lint and both complete same-final-SHA
workflows. Evidence below distinguishes local validation and final certification.

Local Android debug/release, all three packaged ABIs, JVM tests and lint PASS.
Windows/Linux/active ASan-UBSan affected package/provider/C/release-isolation tests
6/6 each PASS. Complete Linux source suite 131/131 PASS, no skips. The same source
suite attempted on Windows passed 95 and failed 36 POSIX harness cases due host
shell/path incompatibility; those unchanged cases passed on Linux, not disabled.
Focused source/security checks 14/14 PASS; actionlint PASS.
Android JVM tests 7/7 per build type; full instrumentation 30/30 PASS with the
existing required fresh transport setup. An earlier direct full-suite invocation
failed only its missing-transport precondition; rerunning with the prescribed
setup passed, with no product change. Expanded provider corpus: 32 ONNX rows,
21 valid model executions (all 15 quarter-weight triples plus retained profiles),
four independent goldens, four backend fault modes; zero failures on SwiftShader.
Test inputs include normals/subnormals/rounding boundaries; 1x1, 2x2, 8x8, 256x256
and 1024x1024. CPU authority and original 32 fixtures/four goldens unchanged.
All 15 additional signed models also run on Windows/Linux/ASan-UBSan CPU tests.
Release libraries for arm64/ARMv7/x86_64 passed actual binary test-anchor/corpus
exclusion scans. No signing/private-key/trust-injection runtime API added.
Full retained suites and Apple/Swift must be observed on the final
same-SHA workflows; delivery records their final SHA/IDs without a later
uncertified documentation-only status commit.

Observed emulator engineering timings, not physical certification or speedup:
256x256 CPU execute approximately 8-12 ms, Vulkan backend approximately 50-60 ms
including per-request device/pipeline setup and per-tile CPU validation. One sample
upload 1.08 ms, dispatch/wait 2.03 ms, readback 0.13 ms. 1024x1024 observations
varied roughly 0.24-0.86 seconds for Vulkan backend. Backend total excludes shared
input validation/normalization and final publication; CPU timing includes its
whole checked execute call, so these are not an apples-to-apples performance claim.
Bounded extra reservation 1638400 bytes = <=1 MiB device allocations +512 KiB
packing/readback arrays +64 KiB adapter control. Driver internals remain opaque.
No fresh physical Android device was observed; Z Fold7 evidence remains historical.

## Classification and scope

A: implementation/security verification and complete exact-SHA certification.
B: Apple accelerated provider, compact model evaluation, shared admission/fallback
and broader provider certification remain in frozen Steps 4-7.
C: optional additional physical-device measurements; no connected physical device
was observed and no fresh physical inference claim is made.

Accepted Step 1/2 fixtures, package schema/trust/signature and CPU arithmetic are
unchanged. Additional Step 3 synthetic fixtures are test-only. No AI UX or new
dependency. Vulkan and shared bounded resource handoff are Step 3 only.
Schema 3 unchanged;
DH-APPLE-METAL-01 unchanged; no Apple provider; Steps 4+ and Phase 6 NOT STARTED.
