# Phase 2 Apple Metal hardware evidence

Status: DEFERRED / PENDING REAL HARDWARE EXECUTION — DH-APPLE-METAL-01.

No physical-device Metal dispatch/parity is claimed. Hosted Apple green tests and shader compilation do NOT by themselves prove Metal execution. Simulator is UNSUPPORTED for certification. The explicit [Product Owner/Architect decision](../../DECISIONS.md#phase-2-closure-decision-2026-10-08) closes Phase 2 and defers Apple execution because no real Apple hardware is currently available. This is not a waiver or removal of the gate.

[DH-APPLE-METAL-01](../../QUALITY_GATES.md#dh-apple-metal-01-deferred-physical-apple-metal-certification) remains mandatory before Phase 11 hardening acceptance, Phase 12 Beta readiness completion, production release or any claim of physical Apple GPU certification. Owner: Product Owner/Architect. Future acceptance/release reports must explicitly carry the gate's status and reviewed execution evidence; agents must not silently forget or auto-waive it. Apple hardware status remains unexecuted and is not PASS.

Command: `bash scripts/check-gpu-apple-hardware.sh 'platform=iOS,name=YOUR_DEVICE_NAME' EXISTING_TEAM_ID` with connected physical iPhone/iPad and existing owner-authorized signing/provisioning. Xcode equivalent: PixAuraAI scheme, physical destination, PixAuraAITests/GpuHardwareTests/testPhysicalMetalCertification.

Required record after execution: exact tested commit SHA; model (no serial), OS; MTLDevice name, families/unified-memory and Metal language/runtime metadata; physical hardware classification; shader/pipeline result; completed command dispatch/readback; all 14/14 parity cases with fallback disabled; PASS/UNSUPPORTED/FAIL; reviewed XCTest JSON attachment/xcresult/log location and execution date. Signing/credentials are not provisioned autonomously. A harness process/test success without its hardware PASS evidence is insufficient.

Contract: [GPU tile v1](../contracts/gpu-tile-v1.md). Android gate: [accepted PASS evidence](phase-2-gpu-android-hardware.md). Phase closure: [PHASE 2 CLOSED](phase-2-exit-closure.md).
