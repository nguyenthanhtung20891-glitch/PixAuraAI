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

## Addendum — conditional acceptance disposition (2026-11-26)

This addendum does not declare Phase 11 closed and does not authorize Phases 12–14.
The conditionally accepted source remains `a3551b2e0fb21896ae80fb6ae8fb7fa195cc2f14`.
A later documentation commit that contains this addendum is not a new certification SHA.

### 1. Exact SHA and CI disposition

| Workflow | `phase-11-integration` | `main` |
| --- | --- | --- |
| Foundation boundaries | run 45719539195, attempt 1, 7/7 success | run 45719547963, attempt 1, 7/7 success |
| Native application shells | run 45719539432, attempt 1, 3/3 success | run 45719547130, attempt 1, 2/3; attempt 2, 3/3 success |

Attempt 2 is not attempt 1. It re-ran only the failed `android-app` job. `source` and `ios-app` stayed the original attempt-1 successes.

### 2. Dependency verification failure and retry

Attempt 1 `android-app` on `main` failed in 23 seconds, before any app test, on strict Gradle verification of `org.jetbrains.kotlin.android:org.jetbrains.kotlin.android.gradle.plugin:2.2.21` POM, reported from the Gradle Plugin Portal. Trusted SHA-256 in `platforms/android/gradle/verification-metadata.xml` is `8b4103cc1412eb838a33c36b0e5c6e63bd0cace4c30ab9658f3861e41fc08389`. Verification was not disabled and the trusted hash was not regenerated.

Independent checks after the failure:

- Maven Central serves that exact POM, SHA-256 `8b4103cc…08389`, matching the trusted metadata and the copy Gradle cached locally on 2026-11-16.
- The implementation artifact `kotlin-gradle-plugin-2.2.21-gradle813.jar` from Maven Central is SHA-256 `b7a52865c6a5e160d2cf61e4d423ce44a6a20de0a6b23d03404b6d01a2811737`, matching the trusted metadata. That jar was not the rejected artifact.
- The Plugin Portal stably serves a shorter marker POM, SHA-256 `45c2d0d6f1f62b1f724900693d48a68f58be3d73602e29f9167489d2332a5fb6`. It has the same single dependency, `org.jetbrains.kotlin:kotlin-gradle-plugin:2.2.21`. It adds no repository, plugin, or version.
- `pluginManagement` searches `google()`, then `mavenCentral()`, then `gradlePluginPortal()`. The concurrent integration run on this same SHA passed strict verification, which is what resolving the Maven Central body does. The failed job named the portal, which is the fallback that serves the non-matching marker.

No dependency compromise is indicated. One controlled retry was then run.

Attempt 2 `android-app` (job 139709527743, 2026-11-26 05:17:41Z–05:30:33Z) succeeded: release/unit/lint build, then `Finished 90 tests` on `emulator-5554 - 15`. Original attempt-1 failure evidence remains on run 45719547130.

### 3. Small-screen AVD

`ShellTest` failing to display `Settings` was reproduced only on the local hand-made arm64 `pixaura-ci-like` AVD. That AVD is not the hosted image (`google_apis` x86_64, 320×640 at 160 dpi). Its content origin was y=152 on a 640-px screen, and Pixel Launcher posted an ANR over the app. The hosted 320×640 job on this SHA executes the same assertion and passed all 90 instrumentation tests. The assertion was not changed. This is a malfunctioning local emulator configuration, not a layout defect on the qualified 320×640 configuration.

### 4. Product performance targets

`QUALITY_GATES.md` is unchanged from `2f823a5` to `a3551b2`. G8 remains physical low/mid/high cohort evidence: p50/p95, peak RSS, and thermal data. The sanitizer job budget (15 to 45 minutes), the iOS job budget (60 to 120 minutes), and the sanitized 12 MP ctest timeouts (120 to 600 seconds) are harness limits so those runs can finish. They are not product latency or resource acceptance criteria, and no resource-admission cap was raised to make them pass.

### 5. Remaining engineering blockers

None in the executable software scope. Checkout stays off. `cpu.reference.v1` stays VALIDATED; Metal and Vulkan stay UNKNOWN. The owner signing project remains local and unstaged.

### 6. External operator tasks

- **G7.** On physical devices: permission grant and denial, airplane-mode and traffic observation, backup exclusion, private storage, and recovery of the last approved revision.
- **G8.** On real LOW, MID, and HIGH Apple and Android devices: latency distribution, peak memory, memory-pressure, and thermal samples for the final 12 MP workflows. Host and emulator timings are not this gate.
- **G10.** Real App Store Connect and Play Console products, tax and contract setup, authorized signing, authentic sandbox purchase, restore, refund, and revocation. Android Billing dependency privacy admission. No live checkout until that exists.
- **G11.** Human VoiceOver, TalkBack, contrast, focus order, large text, and keyboard or switch-input walkthroughs on devices. Automated checks are not this gate.

### 7. Disposition

ENGINEERING COMPLETE — EXTERNAL ACCEPTANCE PENDING.

Phase 11 is not closed. Phases 12, 13, and 14 are not started.
