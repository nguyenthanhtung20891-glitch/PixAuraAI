# Phase 3 Step 5: Filters and Presets Foundation

Status: IN PROGRESS, not FULL PASS. Authorized baseline `2f82083f8a350e6a1c588d3dbd8e5d48e503a9b1`. Step 6 not started.

## Audit and frozen contract

P04 requires basic versioned recipes but approves no artistic names. [Preset v1](../contracts/presets-v1.md) freezes explicit ordered accepted tone/detail operation bundles, 16 KiB/16-operation bounds, exact integer canonical parameters, stable IDs/versions and metadata policy. Geometry rejects. Built-in shipping catalog is empty; synthetic reference recipes are test-only. Contract/source acceptance test observed red before implementation.

Existing detached batch revision/history and Schema 3 preserve all required semantics. No architectural conflict, new processing engine, Schema 4, migration change or durable opaque preset block. Caller supplies fresh operation/revision IDs; all-neutral produces no history; changed recipe retains every operation in one proposal. Existing manual replacement/preview/evaluation/cancellation/approval/persistence remain authoritative.

## Validation

Observed source suite: 107/107 PASS, zero skipped. Final focused contract/documentation checks: 13/13 PASS. Strict Clang shadow diagnostics, actionlint and git diff --check PASS. Android debug/release assembly, lint, JVM tests and instrumentation APK build PASS. Final Windows native 25/25 PASS (3,938 preset checks; 1,443 injected allocation failures recovered). Final Linux native 25/25 PASS (4,888 preset checks; 1,918 injected allocation failures recovered). Android JVM: 2 debug and 2 release tests, zero failures/skips. Final ASan/UBSan suite: 27/27 PASS, including active instrumentation probes and the complete allocation/storage/migration/crash regression corpus. Hosted Android instrumentation/Apple/Swift/iOS/exact-SHA gates remain pending, not PASS.

## Scope

A: bounded recipe/canonical/proposal boundaries and complete acceptance evidence. B: real gesture/editor flows remain frozen Steps 6/7 and phase certification Step 8. C: artistic catalogs, public sharing/import/marketplace/cloud/AI/GPU additions require separate decisions. No AI/network/plugins/remote config, new numerical kernels or final UI. DH-APPLE-METAL-01 remains unchanged/unexecuted. Phase 3 Step 6 has NOT started.

## Acceptance evidence

Native fixtures cover all nine accepted appearance descriptors at minimum/neutral/maximum and adjacent invalid values, recipe/version/category rejection, duplicates/escaped keys, every truncated prefix, exact canonical LF bytes, 16-operation admission, catalog ordering/duplicate bounds, 256-stack and 64-handle admission. Allocation sweeps preserve outputs and state and permit retry. Constituent numerical failure preserves the original source.

Preset proposal serialization equals the same ordinary manual ordered batch; CPU replay pixels equal sequential manual evaluation. Tests exercise A->B, repeated A, manual->preset and preset->manual, normal constituent replacement, neutral suppression, stale source/session/generation and ABA rejection. Existing preview cancellation fences reject cancelled publication without replacing the valid image. SQLite commit/reopen, source hash, ordered replay and durable undo/redo are tested through the existing repository API. Existing migration fault/crash regressions remain required; preset storage introduces no schema or transaction implementation.

## Files changed

- `.gitattributes`
- `.github/workflows/foundation.yml`
- `.github/workflows/native-shells.yml`
- `CMakeLists.txt`
- `DECISIONS.md`
- `QUALITY_GATES.md`
- `ROADMAP.md`
- `packages/core/src/document.cpp`
- `packages/core/src/document.hpp`
- `packages/core/src/document_api.cpp`
- `packages/core/src/manual_tools.cpp`
- `packages/core/src/manual_tools.hpp`
- `packages/core/src/storage.hpp`
- `packages/core/swift/Tests/ManualBoundaryTests.swift`
- `packages/core/tests/manual_geometry_test.cpp`
- `platforms/android/app/src/androidTest/java/ai/pixaura/app/ManualBoundaryTest.kt`
- `platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/CoreProbe.kt`
- `platforms/android/native/bridge.cpp`
- `scripts/check-apple.sh`
- `tests/detail-tools.test.mjs`
- `tests/manual-geometry.test.mjs`
- `tests/tone-color.test.mjs`
- `docs/contracts/presets-v1.md`
- `docs/reports/phase-3-step-5.md`
- `packages/core/include/pixaura/preset.h`
- `packages/core/src/preset.hpp`
- `packages/core/swift/Tests/Fixtures/preset-reference-v1.json`
- `packages/core/tests/preset_boundary.h`
- `packages/core/tests/preset_c_consumer.c`
- `packages/core/tests/preset_test.cpp`
- `tests/fixtures/preset-reference-v1.json`
- `tests/presets.test.mjs`
