# Phase 3 Step 4: Detail Tools

Status: IN PROGRESS, not FULL PASS. Authorized baseline
`cc01732644fc97199529802a8cf2db11e23bfea5`. No Step 5 work.

## Contract freeze and audit

PRODUCT_SPEC P04 requires versioned sharpen/blur but freezes no extra controls.
The [detail contract](../contracts/manual-detail-v1.md) freezes one scalar each,
fixed radius-one 3x3 binomial kernel, clamp-to-edge borders, strict linear
premultiplied arithmetic, coverage blur and alpha-preserving normalized unsharp.
Independent hand-calculated impulse/border tests passed before implementation;
the descriptor-presence test failed as expected before the new code existed.
The exact-rational development oracle is separate from the engine and generates
pixel fixtures without invoking implementation code.

[ADR 0020](../adr/0020-bounded-detail-reference.md) preserves two request rasters
and one shared <=1 MiB scratch buffer. Three input rows use <=786432 bytes;
rotation and detail reuse the buffer, with maximum admission rather than sum.
No uncontrolled full-image intermediate or kernel-radius growth.

## Storage compatibility

Schema 2 cannot store detail without breaking its exact immutable fingerprint.
[Schema 3](../contracts/project-storage-v3.md) adds only two strict typed columns.
Historical schemas 1/2 are untouched. Owner-requested 2->3 uses a controlled
transaction, verifies exact source/result fingerprints and complete canonical
state before changing version and COMMIT. No open auto-migrates; direct 1->3
rejects, and owners request 1->2 then 2->3 separately. IDs/source/order/history/
redo/current/session/generation/epoch and old payload meanings remain.
Rollback preserves logical source state; physical WAL byte identity is not claimed.

## Validation

Observed local results on the Step 4 implementation:

- Windows Zig compiler fallback: 23/23 native tests PASS. Hosted MSVC remains pending.
- Linux native: 23/23 PASS; ASan/UBSan: 25/25 PASS including active instrumentation probes.
- Independent rational fixture regeneration: PASS, 280 cases; existing tone oracle unchanged and PASS.
- Detail reference: impulses, flat/edge/checkerboard/grayscale/saturated/HDR, opaque/partial/zero alpha, borders, narrow images, signed zero/subnormal, tile seams, repeats and mixed geometry/tone order. Neutral/source and sharpen-alpha bits preserved; overflow rejects before publication.
- Native gesture/PRV1/restart suite: 25,147 checks PASS, including 1001 updates per changed commit, neutral/cancel/stale/failure retry and exact same-platform replay after reopen.
- Manual allocation sweep: 3466 failures recovered; renderer/decode allocation sweep: 1311 failures recovered across 23 phases.
- Storage: old 1->2 coverage retained; empty/populated/mixed tone/branch/redo 2->3, exact schema fingerprints, complete canonical envelope equality, reopen, IDs/order/source/current/session/generation/epoch preservation, malformed/application/version rejection and repeated migration rejection PASS. Four controlled precommit faults, allocation and SQLite NOMEM/IOERR/FULL/INTERRUPT rollback PASS. Five new 2->3 process-crash boundaries PASS (19 crash children total including existing durability cases).
- Shell syntax, actionlint, frozen schema-1/2 source fingerprint assertions, deferred Apple gate equality and git diff --check: PASS.

Full Linux source checks: 103/103 PASS, zero skipped. Android debug/release assembly, debug/release lint, JVM tests (2 debug + 2 release) and instrumentation APK build: BUILD SUCCESSFUL. Local emulator execution is not claimed. Android emulator instrumentation, hosted MSVC, Apple native/Swift/iOS simulator/app tests and both exact-SHA workflows are pending. No pending or skipped gate is PASS. Initial hosted Windows found a source-checkout CRLF hashing defect in the new historical-file assertion; it is corrected by LF normalization, without changing either historical SQL or the native exact SQL fingerprint policy. Corrected focused source tests: 4/4 PASS.

## Scope and conflicts

A: detail numerical contracts, bounded CPU kernels, schema evolution and complete
integration/validation. B: real editor flows/UI remain frozen Steps 6/7 and phase
certification Step 8; Step 5 filters are not started. C: future detail GPU
acceleration and unapproved controls remain backlog. Denoise/clarity/texture/
dehaze/local/AI detail are unauthorized. No unresolved architecture conflict;
new detail scratch/storage capabilities are explicitly authorized and documented.
GPU exposure is unchanged. DH-APPLE-METAL-01 remains unchanged/unexecuted.
Phase 3 Step 5 has NOT started.

## Changed files

- `.github/workflows/foundation.yml`
- `.github/workflows/native-shells.yml`
- `CMakeLists.txt`
- `DECISIONS.md`
- `QUALITY_GATES.md`
- `ROADMAP.md`
- `docs/contracts/manual-tools-v1.md`
- `packages/core/include/pixaura/evaluation.h`
- `packages/core/include/pixaura/storage.h`
- `packages/core/src/document.cpp`
- `packages/core/src/document.hpp`
- `packages/core/src/evaluation.cpp`
- `packages/core/src/geometry.cpp`
- `packages/core/src/manual_tools.cpp`
- `packages/core/src/manual_tools.hpp`
- `packages/core/src/storage.cpp`
- `packages/core/src/storage_api.cpp`
- `packages/core/swift/Tests/ManualBoundaryTests.swift`
- `packages/core/swift/Tests/StorageBoundaryTests.swift`
- `packages/core/tests/decode_allocation_test.cpp`
- `packages/core/tests/document_test.cpp`
- `packages/core/tests/manual_geometry_test.cpp`
- `packages/core/tests/manual_test.cpp`
- `packages/core/tests/manual_tone_boundary.h`
- `packages/core/tests/storage_c_consumer.c`
- `packages/core/tests/storage_test.cpp`
- `platforms/android/app/src/androidTest/java/ai/pixaura/app/ManualBoundaryTest.kt`
- `platforms/android/app/src/androidTest/java/ai/pixaura/app/StorageBoundaryTest.kt`
- `scripts/check-apple.sh`
- `tests/geometry-execution.test.mjs`
- `tests/manual-geometry.test.mjs`
- `tests/preview.test.mjs`
- `tests/support/document-contract.mjs`
- `tests/tone-color.test.mjs`
- `docs/adr/0020-bounded-detail-reference.md`
- `docs/contracts/manual-detail-v1.md`
- `docs/contracts/project-storage-v3.md`
- `docs/reports/phase-3-step-4.md`
- `packages/core/src/detail.hpp`
- `packages/core/src/detail_math.hpp`
- `packages/core/src/storage_schema_v3.hpp`
- `packages/core/tests/detail_fixtures.hpp`
- `packages/core/tests/detail_test.cpp`
- `scripts/detail-reference.py`
- `tests/detail-tools.test.mjs`
