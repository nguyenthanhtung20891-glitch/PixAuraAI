# Autonomous engineering contract

PixAuraAI is an AI-native, local-first photo editor. Read PRODUCT_SPEC.md, ARCHITECTURE.md, QUALITY_GATES.md, ROADMAP.md and relevant subsystem documents before changing code. User instructions take precedence. Current authorized scope is Phase 1 native application shells only. Do not begin Phase 2 or implement photo-engine features. Do not push without explicit approval.

## Invariants
- Originals are immutable. Import into private managed storage; export creates a new destination.
- Manual, AI-assisted manual and AI edits use the same versioned operation graph and project history.
- Planning never mutates a project. Validated execution produces a candidate; only user approval commits an AI candidate.
- Core editing works offline. No photo, mask, prompt or embedding leaves the device by default.
- AI uses registered deterministic tools before considering synthesis. Generative editing is outside MVP.
- Resource failure reduces quality or disables AI; it must not disable manual editing.

## Workflow
Inspect repository and specifications, define measurable acceptance, implement narrowly, add meaningful tests, run applicable checks, diagnose and fix failures, update documents/ADRs, and report evidence. Never label unrun checks as passed. Record blockers with owner, remediation and affected gate. Continue between milestones when gates pass, subject to the requested scope. No routine approvals. Human gates: credentials/signing, irreversible external actions, conflicting requirements, outside-repository data loss, or product-level platform limitations.

Use Cursor as primary IDE. Windows validates the portable core and Android when SDKs are installed; macOS validates iOS. Prefer rg for searches. Do not commit credentials, photos, generated builds or model weights. Do not introduce dependencies without version, license, security and binary-size review. No floating production dependency versions. Update DECISIONS.md and add a numbered ADR for architectural changes; supersede rather than rewrite accepted historical decisions. Do not spawn other agents unless explicitly requested by the user or applicable instructions.

## Boundaries and verification
Native Kotlin/Compose and Swift/SwiftUI presentation; shared C++17 core exposed through a checked C ABI. Platform bridges own hardware and OS APIs. No UI-thread decode, render, inference or persistence work. Tests must prove behavior across boundaries, not just mirror implementation.

Foundation: `node --test tests/foundation.test.mjs`; Windows native: `powershell -NoProfile -File scripts/check-native.ps1`; CMake: configure/build/CTest per README.md. Phase 1 adds platform app lint/type/build/UI gates. QUALITY_GATES.md is authoritative for scope and evidence.

Preserve Phase 0 validation: run foundation/CI Node tests, the Windows compiler fallback when MSVC is incomplete, and `bash scripts/check-sanitizers.sh` through local WSL or Linux CI. `bash scripts/check-apple.sh` validates macOS and the iOS package/test boundary on Apple CI. Negative sanitizer probes are test-only and must never enter shipping targets. Phase 1 adds `tests/shells.test.mjs`, Android Gradle build/lint/unit/instrumentation gates and `bash scripts/check-ios-shell.sh`. Never call unobserved Apple execution PASS. Reports distinguish user-attested Phase 0 CI completion from locally observed evidence.
