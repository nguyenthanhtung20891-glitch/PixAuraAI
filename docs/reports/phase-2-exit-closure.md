# Phase 2 exit closure: first GPU pipeline

Status: BLOCKED pending real Android Vulkan and real Apple Metal execution evidence. Phase 2 is NOT CLOSED. This is the original exit criterion, not an additional step; Phase 3 is not started.

## Authority and baseline

Accepted HEAD: 47c97ea44df8cd146040055eaa05e6eada1ac35f. Product Owner/Architect selected PHYSICAL DEVICE validation as the authoritative hardware evidence path and authorized identity/exposure/1/1 only. Existing blocked-audit documentation was present on entry and is continued here. [ADR 0018](../adr/0018-physical-gpu-certification.md) and [GPU tile v1](../contracts/gpu-tile-v1.md) freeze this implementation/validation boundary.

## A. Implementation

Bounded synchronous shared C tile service, CPU reference/frozen gain, explicit parity and opt-in CPU fallback. Repository-owned GLSL/SPIR-V Vulkan compute Android and Metal 2.4 compute Apple; actual dispatch/completion/readback required. Software/unknown devices reject. No persistent GPU cache or display/document/history publication. Step 11 generation eligibility remains mandatory; native test supersedes a real ticket during tile work. All existing raster/context/handle ceilings remain unchanged. Hardware harnesses and pending evidence templates prepared.

Implementation and locally executable validation COMPLETE. Hosted Apple/real MSVC compilation and final exact-SHA hosted regression remain to be observed. Hardware certification remains independently pending.

## B. Hosted compile/regression

Pending new exact-SHA hosted execution after local checks. Current accepted baseline workflows are [Foundation 37656632193](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37656632193), 7/7 jobs, and [Native shells 37656632179](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37656632179), 3/3 jobs, exact accepted SHA. They certify the CPU/platform baseline and do not certify GPU execution.

Hosted CI must compile Vulkan/SPIR-V and Metal shader/backend, execute CPU/protocol/fallback/allocation/regression suites, Android SwiftShader rejection and Apple simulator rejection, preserving every Foundation/native shell gate. Hardware is a separate controlled gate. New run IDs/results and local executed counts will be recorded after observation.

## C. Physical hardware certification

PENDING REAL HARDWARE EXECUTION on both platforms. [Android evidence](phase-2-gpu-android-hardware.md) and [Apple evidence](phase-2-gpu-apple-hardware.md) remain pending templates, not PASS.

Android hosted emulator explicitly uses -gpu swiftshader_indirect; SwiftShader is software rendering and does NOT satisfy real GPU execution. The native classifier rejects it even when Vulkan API is advertised. Hosted Apple green tests and Metal shader/backend compilation do NOT by themselves prove Metal execution. Simulator cannot certify physical Apple hardware.

Commands still required on controlled hardware:

```powershell
powershell -NoProfile -File scripts/check-gpu-android-hardware.ps1 -SdkRoot I:\AndroidStudioSDKdata
```

```bash
bash scripts/check-gpu-apple-hardware.sh 'platform=iOS,name=YOUR_DEVICE_NAME' EXISTING_TEAM_ID
```

Both need actual physical-device hardware PASS with completed dispatch/readback and 14/14 shared parity cases, fallback disabled, exact source SHA and reviewed model/OS/GPU/API evidence. Apple needs existing owner-authorized signing/provisioning; scripts do not provision credentials. No device serial/UUID is included in diagnostic JSON.

## Limitations and acceptance

Only single-operation identity/exposure tiles and dedicated diagnostics. No whole-stack GPU scheduler, context/preview GPU integration, production editing UI or additional GPU tools. Driver internal memory/terminal waits are opaque and not a peak-RSS or hard-latency guarantee. Injected failure tests do not prove physical vendor-driver loss behavior. Final Phase 2 acceptance requires both physical Android Vulkan and physical Apple Metal evidence; until then Phase 2 remains BLOCKED / NOT CLOSED.

## Files, commits and delivery

Exact changed-file inventory: [phase-2-gpu-files.txt](phase-2-gpu-files.txt). No generated shader binaries, build artifacts, credentials or photos are committed. Local observed checks: Windows source 50/50; full Linux source 81/81; Windows Zig native fallback 17/17; Linux Clang native 17/17; actual ASan/UBSan 19/19 including active negative probes; Android Debug/Release ARM64/ARMv7/x86_64, lint zero issues, JVM 4/4, emulator instrumentation 5/5 with zero skipped tests. Native GPU protocol checks=24486, corpus=14, all 10001 gains; actual private-allocation failures recovered=2/2. actionlint, Bash syntax and git diff --check PASS.

Executed commands: node --test over applicable Windows tests and full Linux tests; scripts/check-decode-zig.ps1 with repository Zig 0.14.1; WSL CMake/CTest and scripts/check-sanitizers.sh; Android Gradle offline/strict verification assembleDebug/assembleRelease/lintDebug/lintRelease/testDebugUnitTest/testReleaseUnitTest/assembleDebugAndroidTest/connectedDebugAndroidTest; actionlint and Bash -n. Logs remain ignored under build/gpu-local. Android runtime JSON explicitly identifies Goldfish GFXStream (SwiftShader Device (LLVM 10.0.0)), vendor 6880, type 4, API 4202496, driver 100671488, hardware=false, pipeline=false, dispatch=false, test_count=0, expected_test_count=14, status=UNSUPPORTED. Its rejection regression passes; hardware certification does not.

Initial native stale-generation fixture used a malformed context identity; corrected to the existing 32-hex identity contract and all final native tests pass. Windows source tests were initially invoked with Unix-only tests and default temporary storage; final invocations use the Windows-applicable set and repository-local temporary storage, with the full suite executed under WSL. Existing permission-sensitive Linux storage tests fail on Windows-mounted temporary storage; rerun with repository-mounted tmpfs passes without weakening assertions. Initial sandbox child-process/WSL access failed; authorized process access resolved it. No architecture conflict, ceiling change, vendor modification, test disablement or security relaxation.

Implementation commit and new exact-SHA hosted results will be recorded in the delivery after observation. No hardware PASS is recorded by this preparation task. The current tree is READY FOR HOSTED VALIDATION; READY FOR HARDWARE CERTIFICATION requires those hosted gates to pass first.
