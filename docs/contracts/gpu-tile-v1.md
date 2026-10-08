# First GPU tile pipeline, version 1

Scope: Phase 2 original exit closure only. Identity and exposure/1/1; CPU remains semantic authority. Physical Android Vulkan and physical Apple Metal certification are independent controlled gates. Phase 2 is NOT CLOSED until both gates execute successfully. No additional edit operation, production UI or persistent GPU cache.

## Numerical acceptance fixed before hardware execution

Parameters remain integer milli_ev [-5000,5000], thousandths of an exposure stop. Empty/identity and milli_ev=0 bypass arithmetic. The exact [CPU recipe](cpu-evaluation-v1.md) supplies frozen binary64 table gain G(e), binary64 multiplication and one binary32 rounding. GPU uses RN32(G(e)) and FP32 multiplication per operation, with fast math disabled. FP16 is forbidden. Premultiplied linear-sRGB RGBA32F; no gamma, division/unpremultiplication, pivot, saturation, hidden clamping or gain folding. Negative and greater-than-one finite RGB remain legal. Alpha is bit-exact, including signed zero. Identity is bit-exact for every channel, including subnormals. Raw uint4 shader loads/stores preserve unchanged bits.

For nonzero exposure RGB, acceptance is abs(GPU-CPU) <= 4e-37 + 2e-6*abs(CPU), independently per channel. The absolute allowance bounds FTZ behavior for subnormal inputs scaled by at most 32 (32*FLT_MIN < 4e-37); the relative allowance covers gain rounding and FP32 multiplication. NaN/infinity always fails. This is an acceptance threshold, not measured hardware evidence. GPU infinity near FLT_MAX must reject/fallback even if CPU reference is finite. CPU overflow, invalid alpha/nonfinite source or hidden zero-alpha RGB rejects before dispatch. Every single operation is checked separately; composition cannot silently fold gains or claim a fixed whole-stack tolerance. Version 1 is a single-operation tile primitive, not a new document operation or a full stack/preview GPU scheduler.

The shared certification corpus has 14 exposure parameters (including identity, limits, integer stops and fractional values) and 257 deterministic RGBA pixels per case, 3598 pixel results/14392 channel comparisons. Includes negative, >1, large finite, transparent/signed-zero, subnormal and tiny-alpha values. CPU/protocol tests additionally check all 10001 legal gains and failure recovery. Hardware PASS requires all 14 actual GPU dispatch/readback/parity cases, with fallback disabled.

## Ownership and memory

gpu.h is additive version 1, ABI 1/feature bits and document/storage formats unchanged. Synchronous worker-only call, no callback retention. Borrowed immutable source and separate caller-owned output each contain <=16384 pixels/256 KiB. Overlap rejects. Shared CPU reference and candidate arrays each <=256 KiB, released on return, including failures. Vulkan owns two <=256 KiB host-visible/coherent storage buffers; driver memory requirements each must be <=512 KiB, so admitted device memory <=1 MiB. There is no additional upload/readback allocation: mapped storage is staging and readback. Metal owns two <=256 KiB storageModeShared buffers, <=512 KiB total, retained until command completion and released with the synchronous backend owner. Shader, pipeline, descriptor and command object counts are constant; driver-internal allocations are additional opaque overhead, not an RSS guarantee.

Maximum explicit transient payload, excluding borrowed source/caller output: Vulkan <=1.5 MiB, Metal <=1 MiB. Full-sized raster allocation is never performed by a backend. Certification uses only 257-pixel borrowed arrays. These standalone diagnostics own no decode context/registered image handle and cannot allocate around a context budget. Any future context evaluator integration must reserve this transient payload before dispatch under its existing aggregate admission; this primitive does not implement that integration or claim a GPU preview path. Existing 128 MiB raster/8388608 pixels/256 MiB context/64 handles remain unchanged. No persistent GPU cache, automatic eviction or history mutation.

## Capabilities and synchronization

Android: Vulkan 1.0 instance creation, bounded <=32 device and <=64 queue-family enumeration, physical integrated/discrete type, nonzero vendor, software-name/vendor rejection, compute queue, 64-thread workgroup, >=256 dispatch groups, storage descriptor/range/push limits, host-visible coherent memory. Unknown/virtual/CPU, Google SwiftShader/vendor 0x1ae0, Mesa software vendor 0x10005, llvmpipe/lavapipe/software/virtual/emulator/Venus names reject as UNSUPPORTED. Physical Android test also rejects emulator provenance. Record selected or first rejected device name/type/vendor/API/driver; no UUID/serial. Limits unsupported are not PASS. Repository GLSL is compiled by pinned NDK glslc to Vulkan 1.0 SPIR-V and embedded from the generated build directory; no binary shader artifact committed.

Host write -> compute read/write barrier; compute write -> host read barrier; submit with fence and await completed work before coherent readback. Fence failure rejects; teardown waits for submitted work before freeing. The 30-second fence wait bounds validation wait, but terminal driver cleanup vkDeviceWaitIdle may block; no hard real-time guarantee. Vulkan source follows [Khronos synchronization rules](https://docs.vulkan.org/spec/latest/chapters/synchronization.html).

Apple: MTLCreateSystemDefaultDevice, Apple family 1 or Mac family 2, nonsoftware/nonvirtual name, buffer limit and >=64 threads/pipeline. Simulator explicitly UNSUPPORTED regardless of host Metal availability. Record device name, relevant families/unified memory, OS, model and Metal language version. Repository Metal 2.4 shader compiles for macOS/iPhoneOS/simulator in hosted CI; runtime creates its own library with fast math disabled and actual compute pipeline, queue, buffers, encoder/dispatch/command commit. [Command buffer](https://developer.apple.com/documentation/metal/mtlcommandbuffer) completion/error checked before readback. ARC keeps bounded buffers alive until completion. Hosted macOS compilation or preexisting Apple green tests never certify a physical mobile Metal dispatch.

## Failure and publication

Backend unavailable/software/API/queue/capability rejection=5; allocation failure=8; pipeline/submission/readback/parity failure=14. CPU fallback (explicit allow_fallback=1) uses already validated reference and reports used_gpu=0 plus original backend_status. Certification uses allow_fallback=0, so fallback can never certify hardware. Cancellation=13 never falls back. Source/numeric/admission failure rejects without dispatch. All errors preserve source/output/result and retain no candidate. Source/document/history/current valid display cannot be mutated through this diagnostic API.

Optional borrowed current callback must return nonzero only for the current uncancelled generation; checks at admission, bounded CPU/parity loops, before dispatch, after completion and before returning a private candidate. Native regression uses real Step 11 preview tickets and supersedes during the callback to prove stale rejection. Actual display installation must remain under the Step 11 platform owner lock and native current check; this tile service never grants publication authority. GPU dispatch is non-preemptible; cancellation discards completed work and never frees in-flight buffers.

Fault regression distinguishes injected adapter status/parity faults from real driver observations. Native tests cover unsupported/software classification, unavailable backend, allocation, pipeline/submit/readback protocol failures, parity corruption, cancellation/stale generation, overflow/invalid source, output preservation and CPU fallback. A separate fail-at-N test actually fails both private CPU allocations and proves recovery. Physical-driver loss and vendor-specific failures remain device-validation limitations, not inferred from injection.

## Controlled execution

Both controllers require a clean committed checkout so the recorded source SHA identifies the tested repository content.

Android evidence transport uses an exclusive wrapper-created run directory under `/data/local/tmp`, a repository-owned publisher invoked by instrumentation as shell, and at most 16 KiB of synthetic diagnostic JSON. It survives Gradle/UTP uninstall and never relies on app-private readback or a hardcoded application ID. The wrapper reads only the exact current artifact, requires current validation_run/source_sha, successful test exit, hardware/controlled provenance, completed pipeline/dispatch and exactly 14/14 parity, then removes its artifact and publisher. Existing Node engineering tooling validates the bounded host artifact. Hosted synthetic transport regression has no hardware certification authority.

Android, one previously authorized connected physical device, installed SDK/JDK and Gradle inputs:

```powershell
powershell -NoProfile -File scripts/check-gpu-android-hardware.ps1 -SdkRoot I:\AndroidStudioSDKdata
```

Apple, connected physical iPhone/iPad, Xcode and existing signing/provisioning authorized by the owner:

```bash
bash scripts/check-gpu-apple-hardware.sh 'platform=iOS,name=YOUR_DEVICE_NAME' EXISTING_TEAM_ID
```

Equivalent Xcode path: PixAuraAI scheme -> connected physical device -> PixAuraAITests/GpuHardwareTests/testPhysicalMetalCertification. Signing/credentials are not provisioned by scripts. Android instrumentation class is ai.pixaura.app.GpuHardwareTest with pixauraHardware=true and the wrapper-generated bounded pixauraRun/pixauraSha arguments. Evidence includes platform, model, OS, GPU/API metadata, hardware/software classification, pipeline and dispatch result, parity count, total tests, PASS/UNSUPPORTED/FAIL and exact source SHA sidecar. Android's fresh random validation_run and source_sha must match the current wrapper invocation; an older saved device JSON cannot certify a new run. The run identifier is diagnostic freshness metadata, not a device identifier or secret. Android saves JSON and human-readable logs under build/gpu-android-hardware; Apple keeps JSON XCTest attachment and a fresh xcresult/log/SHA under build/gpu-apple-hardware. Export reviewed evidence without unnecessary device identifiers into the pending [Android](../reports/phase-2-gpu-android-hardware.md) and [Apple](../reports/phase-2-gpu-apple-hardware.md) reports. Unsupported or failed execution leaves Phase 2 BLOCKED.
