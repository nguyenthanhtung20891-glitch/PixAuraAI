# Phase 2 Android Vulkan hardware evidence

Status: PENDING REAL HARDWARE EXECUTION.

No physical-device Vulkan dispatch/parity is claimed. Hosted SwiftShader is software and does NOT satisfy this gate. Phase 2 remains BLOCKED / NOT CLOSED until both Android and Apple physical-device evidence exists.

Command: `powershell -NoProfile -File scripts/check-gpu-android-hardware.ps1 -SdkRoot I:\AndroidStudioSDKdata`.

Required record after execution: exact tested commit SHA; device model (no serial), OS; GPU name/vendor/type/API/driver; hardware classification and controlled physical provenance; shader/pipeline result; completed dispatch/readback; all 14/14 parity cases with fallback disabled; PASS/UNSUPPORTED/FAIL; reviewed JSON/log location and execution date. Keep raw builds outside Git and attach only reviewed synthetic evidence. A harness process/test success without its hardware PASS evidence is insufficient.

Contract: [GPU tile v1](../contracts/gpu-tile-v1.md). Apple gate: [pending evidence](phase-2-gpu-apple-hardware.md).
