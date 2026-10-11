# Phase 3 Step 6: Gesture Editing Lifecycle Integration

Status: AUTHORIZED / IN PROGRESS. Baseline `311392faf9d5600598a5e541ba531dd5f722e996`.
Steps 1–5 accepted COMPLETE / FULL PASS. Steps 7/8 NOT STARTED.

## Implementation and acceptance

[Lifecycle contract](../contracts/gesture-lifecycle-integration-v1.md) refines the
existing Step 1 controller and Step 2–5 generic adapters. Integrated begin binds
the normalized verified original, one active gesture per context and PRV1 owner.
Accepted updates atomically replace pending state and request a generation;
invalid/allocation/admission failure retains previous state. Terminal operations
revoke publication. Commit remains one detached immutable proposal or no-op;
ordinary approval and SQLite epoch/session/history authority remain unchanged.
Undo/redo/checkout/document transition and preset propose reject while active.
Interruption cancels/revokes/releases with no history. No second engine/scheduler.

## Validation evidence

Local validation: Windows native 27/27 PASS, zero failed/skipped (Zig 0.14.1 /
Clang 19.1.7 fallback, unchanged ceilings). Final focused lifecycle/C gate 2/2
PASS: 196,650 native assertions, 10,000 updates per representative geometry,
tone and detail tool per run; two complete runs retain identical live allocation
payload after warmup. 467 injected allocation failures recovered per run (934
total), plus commit-handle admission recovery, PRV1 stop/overflow and allocation-free
cancel. Checkpoint-paused previews exercise update, cancel, background/disposal
and commit while rendering; 256 update-versus-commit/cancel races across two runs
accept only the serialized outcomes. Tool switch rejects old previews; stale
session/revision and exact base release invalidate/revoke. Contexts remain isolated.

Changed gestures produce one revision, neutral/return-to-neutral/cancel produce
none; repeated commit rejects. Actual SQLite schema-3 commit/reopen canonical
equality, ordered replay, original digest/pixels, undo/redo and preset-to-manual
stack boundaries pass. Existing crash/migration/resource/numerical regressions
remain in the full 27-test suite.

Portable source/documentation suite 75/75 PASS, zero skips. Full Windows Node
suite attempted 110 tests: 75 PASS / 35 FAIL because Bash/WSL orchestration was
unavailable; those 35 Linux-only emulator/provisioning checks remain required on
hosted Linux, not waived. Local WSL invocation failed with service
`Wsl/Service/RPC_S_CALL_FAILED` after sandbox escalation; no Linux/sanitizer PASS
is claimed locally. Strict actionlint and git diff --check PASS. Android final
debug/release assembly, strict lint, debug JVM 2/2 and release JVM 2/2 tests, zero
failures/skips, and instrumentation APK assembly PASS. All caches/temp/builds
are repository-local. Apple, Android instrumentation and exact-SHA hosted runs
remain pending. No unobserved gate is PASS.

Commands: CMake/Ninja Debug configure/build and ctest --output-on-failure --verbose;
node --test on all portable affected suites; Gradle --no-daemon
--dependency-verification strict assembleDebug assembleRelease testDebugUnitTest
testReleaseUnitTest lintDebug lintRelease assembleDebugAndroidTest; actionlint
1.7.12; git diff --check. Ignored local evidence: build/step6-windows-tests.log,
build/step6-gesture-final.log, build/step6-source-portable.log and
build/step6-android-final.log.

## Scope

A: generic lifecycle/state/PRV1 ownership, verified source admission, boundary/race/
fault/history/resource tests and documentation. B: final Android/iOS editor
controls and integration in frozen Step 7; Phase 3 certification/closure Step 8.
C: optional telemetry, new tools and catalogs. No architecture conflict/new ADR,
final editor UI, new numerical tool/kernel, AI/GPU expansion or storage schema.
DH-APPLE-METAL-01 remains unchanged/unexecuted. Phase 3 Step 7 has NOT started.

## Files changed

- `ARCHITECTURE.md`
- `.github/workflows/foundation.yml`
- `.github/workflows/native-shells.yml`
- `CMakeLists.txt`
- `DECISIONS.md`
- `QUALITY_GATES.md`
- `ROADMAP.md`
- `packages/core/include/pixaura/manual.h`
- `packages/core/include/pixaura/working.h`
- `packages/core/src/decode_api.cpp`
- `packages/core/src/document_api.cpp`
- `packages/core/src/manual_geometry_preview.cpp`
- `packages/core/src/manual_tools.cpp`
- `packages/core/src/manual_tools.hpp`
- `packages/core/src/manual_preview_binding.hpp`
- `packages/core/swift/Tests/GestureLifecycleTests.swift`
- `packages/core/tests/gesture_lifecycle_boundary.h`
- `packages/core/tests/gesture_lifecycle_c_consumer.c`
- `packages/core/tests/gesture_lifecycle_test.cpp`
- `platforms/android/app/src/androidTest/java/ai/pixaura/app/ManualBoundaryTest.kt`
- `platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/CoreProbe.kt`
- `platforms/android/native/bridge.cpp`
- `scripts/check-apple.sh`
- `tests/detail-tools.test.mjs`
- `tests/manual-geometry.test.mjs`
- `tests/presets.test.mjs`
- `tests/tone-color.test.mjs`
- `tests/gesture-lifecycle.test.mjs`
- `docs/contracts/gesture-lifecycle-integration-v1.md`
- `docs/reports/phase-3-step-6.md`
