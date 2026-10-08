# Phase 2 Android Vulkan hardware evidence

Status: PENDING REAL HARDWARE EXECUTION.

No physical-device Vulkan dispatch/parity is claimed. Hosted SwiftShader is software and does NOT satisfy this gate. Phase 2 remains BLOCKED / NOT CLOSED until both Android and Apple physical-device evidence exists.

Command: `powershell -NoProfile -File scripts/check-gpu-android-hardware.ps1 -SdkRoot I:\AndroidStudioSDKdata`.

Required record after execution: exact tested commit SHA; device model (no serial), OS; GPU name/vendor/type/API/driver; hardware classification and controlled physical provenance; shader/pipeline result; completed dispatch/readback; all 14/14 parity cases with fallback disabled; PASS/UNSUPPORTED/FAIL; reviewed JSON/log location and execution date. Keep raw builds outside Git and attach only reviewed synthetic evidence. A harness process/test success without its hardware PASS evidence is insufficient.

Contract: [GPU tile v1](../contracts/gpu-tile-v1.md). Apple gate: [pending evidence](phase-2-gpu-apple-hardware.md).

The wrapper must match JSON validation_run/source_sha to its fresh invocation/SHA sidecar and require a successful instrumentation process. Stale saved device evidence is a FAIL; the run identifier contains no device identity or secret.

## Evidence readback remediation (2026-10-08)

Baseline SHA: `4f628e0737b6895194e3494f111b3fb01317b4ca`. The owner observed successful `GpuHardwareTest` execution / Gradle BUILD SUCCESSFUL on SM-F966B, followed by `run-as: unknown package: ai.pixaura.app`. This is a harness readback failure, not a Vulkan/GPU failure, and does not establish certification PASS.

Audit: `app/build.gradle.kts` and generated debug APK metadata identify target `ai.pixaura.app`, with no debug application-ID suffix; generated instrumentation APK metadata identifies `ai.pixaura.app.test`. `GpuHardwareTest` writes its private JSON using `targetContext`, so the owner is the target app, not the test package. The observed local UTP log records `uninstall_after_test: true` for both packages and explicitly uninstalls both after the test. The old package ID was correct; post-test app-private readback was invalid for this lifecycle. The observed JUnit XML contains test outcome but no diagnostic stdout payload.

Transport: preserve `connectedDebugAndroidTest`. The host creates an exclusive random `/data/local/tmp/pixaura-gpu-<validation_run>` directory and pushes the reviewed publisher script there. Instrumentation uses UiAutomation shell execution with a bounded base64 argument to publish only synthetic GPU diagnostic JSON, at most 16 KiB, with private shell permissions and no overwrite. The wrapper reads only this exact current artifact after UTP completion, validates its current run and source SHA, successful Gradle exit, hardware/controlled provenance, pipeline/dispatch and exactly 14/14 parity, then removes the artifact and publisher. No package ID or surviving installation is required; no private phone files, general logcat or device identifiers are collected. Missing, malformed, oversized, stale, mismatched or incomplete evidence fails. A prior installed test/private JSON cannot be selected. Clean committed checkout and emulator/software rejection remain mandatory. Node is the existing engineering tool used for bounded host evidence validation, not a production dependency.

Regression coverage: executable host validator cases A-F plus incomplete parity/software/fallback/failed-process/oversize rejection; Android synthetic transport test publishes through the same function, deletes its private fixture, reads shell evidence, rejects overwrite, and preserves evidence for host readback after UTP uninstall. Hosted emulator harness supplies the fresh transport directory and checks that exact synthetic artifact after Gradle completes. `TRANSPORT_TEST_ONLY` never certifies GPU hardware.

Local observed results: Windows source gates 51/51; focused Linux evidence regressions 9/9; Android Debug/Release and instrumentation APK builds, Debug/Release lint (zero issues), JVM 4/4; physical SM-F966B synthetic transport instrumentation 1/1 followed by successful exact-artifact host readback after UTP uninstalled both packages (also verified absent for Android user 0). PowerShell parse, Bash syntax and actionlint passed. Initial synthetic execution exposed UiAutomation command tokenization/quoting; invoking the pushed publisher with a simple argument list fixed it, and the final physical regression passed. No GPU certification was executed by this remediation. Logs and synthetic evidence remain ignored under `build/gpu-local/readback-*`.

Final exact-SHA hosted workflow closure is recorded in the delivery. Android hardware certification remains PENDING REAL HARDWARE EXECUTION until the Product Owner reruns the fixed command on the physical Z Fold7. Apple status is unchanged; Phase 2 remains NOT CLOSED, with no Phase 3 or additional Phase 2 step. No architecture conflict or numerical/production GPU change.

First publication `8f76e03ee9d1a9aa3c4eca93615577214ce6c222`, Native shells run `37793843765`, source job `113367994370`: 82/88 source tests passed; six executable emulator mock scenarios failed because their fake ADB did not recognize the new artifact commands (exit 91). Remediation updates those fixtures to execute the actual publisher, bind its invocation argument to the fresh host directory, simulate target/test uninstall before readback, and assert missing/stale artifact and setup failure rejection. Existing timeout, readiness, cleanup and disk assertions remain enforced. This CI failure is an orchestration fixture failure, not GPU certification failure.

Follow-up local Linux execution: emulator orchestration 34/34 and remaining source/native-shell contracts 57/57 (91/91 combined, zero skipped), using repository-local temporary fixtures. Only test fixtures and reporting change in this follow-up; the successful physical synthetic transport execution remains applicable. Final exact-SHA hosted closure remains mandatory and is supplied in delivery.
