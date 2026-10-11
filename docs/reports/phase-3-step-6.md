# Phase 3 Step 6: Gesture Editing Lifecycle Integration

Status: COMPLETE / FULL PASS (technical gates), ready for Product Owner acceptance. Baseline `311392faf9d5600598a5e541ba531dd5f722e996`.
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
PASS: 196,726 native assertions, 10,000 varying updates per representative geometry,
tone and detail tool per run, with the exact final accepted value in the proposal;
two complete runs retain identical live allocation
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
unavailable; all 35 Linux-only emulator/provisioning checks subsequently passed on
hosted Linux, without waiver. Local WSL invocation failed with service
`Wsl/Service/RPC_S_CALL_FAILED` after sandbox escalation; no Linux/sanitizer PASS
is claimed locally. Strict actionlint and git diff --check PASS. Android final
debug/release assembly, strict lint, debug JVM 2/2 and release JVM 2/2 tests, zero
failures/skips, and instrumentation APK assembly PASS. Successful reruns explicitly
routed caches/temp/builds into the repository; the initial full Windows Node attempt
used framework OS-temp fixtures. Hosted evidence below closes the remaining gates.

## Hosted remediation

Exact SHA `f3fe43c752034c77e7d1a6bdc29a8e8f7e2c1b92` passed hosted Linux,
MSVC, ASan/UBSan and both Android ABI jobs. Apple failed in the new Swift fixture's
decode-open call with status 6: Foundation's temporary-directory alias was rejected
by the unchanged O_NOFOLLOW_ANY admission. The fixture now uses Darwin realpath,
matching the accepted decode fixture, and stable heap-owned raster context storage.
No security check, warning or test is disabled. Corrected exact-SHA reruns passed.

Review also preserved Step 1's nonzero-update preview eligibility and PRV1 malformed
ticket statuses (2/3); added original-handle release invalidation using the existing
publication mutex without waiting for pixel work. Final local native 27/27 and
focused 2/2 gates plus Android regression and portable source checks are retained.

Commands: CMake/Ninja Debug configure/build and ctest --output-on-failure --verbose;
node --test on all portable affected suites; Gradle --no-daemon
--dependency-verification strict assembleDebug assembleRelease testDebugUnitTest
testReleaseUnitTest lintDebug lintRelease assembleDebugAndroidTest; actionlint
1.7.12; git diff --check. Ignored local evidence: build/step6-windows-tests.log,
build/step6-gesture-final.log, build/step6-source-portable.log and
build/step6-android-final.log.

## Exact-SHA hosted acceptance

Implementation SHA `cba85dc7da1273ba6c236e985a637c2a47deb22b`:
[Foundation boundaries](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/38102767486)
PASS 7/7 jobs;
[native application shells](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/38102767566)
PASS 3/3 jobs. All required Step 6 gates were observed, with zero test failures.

| Gate | Executed result |
| --- | --- |
| Source/documentation/workflow | 110/110 Node tests; strict actionlint PASS |
| Linux / Windows MSVC / Apple native | 27/27 tests on each platform |
| ASan / UBSan | 29/29, including active negative instrumentation probes |
| Android ABI | arm64-v8a and armeabi-v7a PASS |
| Android application | debug/release builds and lint; JVM 2/2 debug and 2/2 release; emulator instrumentation 11/11 |
| Swift boundaries | macOS 12/12; iOS simulator 12/12 |
| iOS application | debug/release builds; app 6/6 and UI 1/1 |

Linux lifecycle validation: 200,506 assertions and 602 injected failures per run
(1,204 recovered total). MSVC: 229,502 assertions and 1,644 injected failures per
run (3,288 recovered total). The same deterministic checkpoint/race/history and
bounded-allocation corpus runs in the sanitizer and Apple native gates.
No Step 6 CI-only gate remains. Local WSL remains unavailable; hosted Linux and
sanitizers supply the required evidence. Simulator Metal is UNSUPPORTED, not
physical certification; DH-APPLE-METAL-01 is unchanged and unexecuted.
The documentation closure commit's exact SHA and hosted reruns are recorded in
final delivery, because a commit cannot embed its own SHA.

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
