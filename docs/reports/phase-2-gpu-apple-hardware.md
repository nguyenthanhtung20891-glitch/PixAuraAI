# Phase 2 Apple Metal hardware evidence

Status: PENDING REAL HARDWARE EXECUTION.

No physical-device Metal dispatch/parity is claimed. Hosted Apple green tests and shader compilation do NOT by themselves prove Metal execution. Simulator is UNSUPPORTED for certification. Phase 2 remains BLOCKED / NOT CLOSED until both Android and Apple physical-device evidence exists.

Command: `bash scripts/check-gpu-apple-hardware.sh 'platform=iOS,name=YOUR_DEVICE_NAME' EXISTING_TEAM_ID` with connected physical iPhone/iPad and existing owner-authorized signing/provisioning. Xcode equivalent: PixAuraAI scheme, physical destination, PixAuraAITests/GpuHardwareTests/testPhysicalMetalCertification.

Required record after execution: exact tested commit SHA; model (no serial), OS; MTLDevice name, families/unified-memory and Metal language/runtime metadata; physical hardware classification; shader/pipeline result; completed command dispatch/readback; all 14/14 parity cases with fallback disabled; PASS/UNSUPPORTED/FAIL; reviewed XCTest JSON attachment/xcresult/log location and execution date. Signing/credentials are not provisioned autonomously. A harness process/test success without its hardware PASS evidence is insufficient.

Contract: [GPU tile v1](../contracts/gpu-tile-v1.md). Android gate: [pending evidence](phase-2-gpu-android-hardware.md).
