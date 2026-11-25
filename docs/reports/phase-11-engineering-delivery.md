# Phase 11 consolidated engineering delivery and blocker matrix

Status: ENGINEERING COMPLETE FOR ALL EXECUTABLE SOFTWARE SCOPE / SUBMITTED FOR
INDEPENDENT PRINCIPAL ARCHITECT REVIEW. Phase 11 is NOT closed and is not
declared closed by engineering. Phase 12–14 are not started or authorized.
`cpu.reference.v1` VALIDATED; `apple.metal.v1` and `android.vulkan.v1` UNKNOWN;
DH-APPLE-METAL-01 unchanged (PASS/CLOSED for its tested iPhone XR 14-case scope only).

Predecessor certified at `2f823a55c36271b19847fade7134aa9b460b715d`
(Foundation 43809264602 7/7, Native shells 43809264489 3/3). Everything after
it was developed uncommitted by a prior agent and recovered by this takeover on
branch `phase-11-integration`. Exact-final-SHA hosted results are recorded in the
delivery message, not in this file (a file cannot name its own SHA).

## Takeover actions

1. Read-only audit; protected owner file `platforms/ios/PixAuraAI.xcodeproj/project.pbxproj`
   (SHA-256 `36be644a…174c`) kept modified, local and unstaged. No reset, clean or re-clone.
2. Recovery commit with explicit staging (151 files) excluding the owner project,
   `CLAUDE.md`, root-level `editor-v2-*` synthetic test directories and `build/`.
3. `store_ingress_v3` truncated-structure defect: confirmed real by ASan
   (heap over-read from copying the caller structure before validating its declared
   size), fix present and verified (only the version/size prefix is read until
   `struct_size` is validated). Native + sanitizer runs pass.
4. Defects found and fixed during takeover:
   - iOS `ManualEditorModel.swift` used a `@unchecked Sendable` store-observation
     class, which `tests/agent-execution.test.mjs` forbids. Replaced with a
     `DispatchSourceUserDataAdd` signal (weak self; native event is read on the
     existing command queue). No test change.
   - Provenance manifest `phase-11-source-v20.json` was stale; regenerated against
     the certified predecessor (67→68 changes, 83 additions, 113 protected native
     fingerprints preserved, CR18-02 fragments recorded).
   - Android `:store-integration` Billing-proof module did not compile
     (`Unresolved reference 'bridge'` …) because the commerce sources now import the
     checked Kotlin bridge and `NativeEditorRuntimePoolV2`. Module now also compiles
     `../bridge/src/main/kotlin` and stages exactly one `:app` file. No new dependency,
     no lock change, no JNI linking. This would have failed any root Gradle build.

5. Portability and CI-budget defects that macOS/clang hid and hosted CI exposed
   (all fixed without weakening any test, warning, sanitizer or assertion):
   - GCC 13 `-Werror`: `-Wmisleading-indentation` in several v2 sources and a
     dangling `std::initializer_list` in `quota.cpp` `recognized_schema` that made
     the Linux Debug build segfault (now a `std::vector`).
   - MSVC `/W4 /WX`: C4456/C4457 shadowing, C4244 fill narrowing, C3538 mixed
     `auto` declarators, C4996 `_open` (now `_sopen_s` wrappers), `_Static_assert` in C
     (portable typedef check), and hidden-DLL-symbol link errors in four v2 test
     targets (tests now compile the internal sources they exercise; `manual_tools.cpp`
     is isolated in a static proof library with consistent export linkage).
   - Linux UBSan: C consumer linked as C++ (`LINKER_LANGUAGE CXX`).
   - Swift 6.2 / Xcode 26.6: `asyncAfter` capture replaced by a one-shot timer.
   - Android hosted emulator (slow, 2 cores): UI tests clicked busy-disabled chips and
     buttons; tests now wait until the control is enabled before clicking. Assertions
     are unchanged.
   - Job budgets that the grown suites exceed on hosted runners: sanitizer job 15 -> 45 min,
     iOS job 60 -> 120 min (the 20 UI tests alone take about 35 min), per-test timeouts
     of the sanitized 12 MP tests 120 -> 600 s. No check was removed or skipped.

## Local evidence (this Mac Studio, commit `78cddf6` unless noted)

| Check | Result |
| --- | --- |
| Native host Debug ctest | 93/93 PASS |
| Native ASan/UBSan | 95/95 PASS |
| `scripts/check-apple.sh` (incl. Swift bridge 7/7) | PASS |
| iOS app unit tests | 35/35 PASS |
| iOS UI tests (pristine Duo simulator, EN/VI/large text) | 20/20 PASS |
| Apple Release isolation scan, model/privacy scan | PASS |
| Android Gradle `assembleDebug assembleRelease testDebugUnitTest testReleaseUnitTest lintDebug lintRelease assembleDebugAndroidTest` strict dependency verification | PASS (14 unit tests, 0 failures) |
| Android instrumentation, arm64 emulator, SELinux Enforcing | 90/90 PASS, 0 skipped; transport status `TRANSPORT_TEST_ONLY` |
| Android release APK isolation scan | PASS |
| NDK boundary builds `arm64-v8a` and `armeabi-v7a`, API 26 | PASS (both) |
| Node source suite in a clean clone | 254/263; the 9 failures are all `android-emulator.test.mjs` orchestrator tests that require GNU `realpath -m`/`timeout` and are covered by Linux CI |

Re-verification after the portability fixes: macOS host 93/93; Linux GCC 13 Debug
(Docker, `-Werror`) 93/93; Linux clang ASan/UBSan 95/95; Swift/iOS 35 unit + 20 UI on a
pristine simulator; Gradle strict chain PASS; node 254/263 (same 9 Linux-only).
Known local-only observation: `ShellTest` fails on my hand-made arm64 `pixaura-ci-like`
AVD (that AVD shows a 128 dp top inset and a Pixel Launcher ANR) but passes on the
`pixel_6` arm64 AVD (90/90) and on the hosted x86_64 CI emulator; the other 89 tests
pass on the small-screen AVD. Treated as an AVD artefact, not claimed as a product fix.

Notes on test integrity: iOS UI tests persist the Free quota ledger in the app
container, so they are only meaningful on a pristine simulator; a reused simulator
produced false "daily limit reached" failures that are not product defects. No
test, assertion, warning or sanitizer was weakened or skipped.

## Gate matrix

| Gate | Engineering status | Remaining |
| --- | --- | --- |
| G0 spec / G1 source / G3 build | Done locally, provenance v20 refreshed | Exact-SHA hosted CI |
| G2 behavior / G4 native safety / G5 persistence / G6 parity / G9 AI control | Done locally (counts above) | Exact-SHA hosted CI |
| G7 offline/privacy | Software tests done; no network/checkout/upload expansion | EXTERNAL: physical permission, traffic, backup-exclusion, storage observations |
| G8 device/performance | Real 12 MP host/app workflows, bounded LOW-tier tests | EXTERNAL: physical LOW/MID/HIGH Apple and Android cohorts, thermal/memory pressure |
| G10 commerce | Policy, trial, quotas, protected cache, verified-store ingress implemented; synthetic signed records only; **checkout OFF** | EXTERNAL: App Store Connect/Play Console, product IDs, signing, authentic sandbox purchases/restore/refund, Billing dependency privacy admission |
| G11 accessibility | EN/VI, large text, dark/light, automated checks | EXTERNAL: human VoiceOver/TalkBack/keyboard/switch walkthroughs |
| Hardware providers | CPU reference only | Metal/Vulkan admission not claimed |

## Not done on purpose

No physical-device, store-account, sandbox-transaction or human-accessibility
evidence was fabricated. No purchase flow or GPU provider was enabled. The owner
signing project was not committed. No Phase 12 work was started.

Next action: independent Principal Architect review.
