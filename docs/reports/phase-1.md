# Phase 1 native application shells report

Updated: 2026-10-02. Scope: native shells only; Phase 2 not started. Status: **local implementation complete, all automated local gates PASS; Apple execution BLOCKED on this Windows host; assistive walkthrough unrun; not a full Phase 1 PASS**.

## Inspection and precondition
Resumed from clean commit 5462e36. No modified/untracked application source, Phase 1 report or shell project existed at the interruption. No prior shell build/test was pending. Read AGENTS, architecture/product/gates/testing/roadmap, all six accepted ADRs, Phase 0 report, privacy/UX and existing boundary scripts/tests. Preserve the C ABI and C++ implementation unchanged.

The user states Phase 0 GitHub Actions across Windows/Linux/Android/sanitizers/Apple passed. This is user-attested; the checked Phase 0 report is an older pending-CI snapshot. Origin is https://github.com/nguyenthanhtung20891-glitch/PixAuraAI.git. GitHub CLI is absent; unauthenticated Actions API returned 404, which cannot establish success or failure for a private repository. No credentials were sought. No push, commit, signing or external deployment was performed.

## Acceptance and architecture
Four minimal native destinations, three visible mode entry points, a placeholder session that preserves mode, real shared ABI 1 status, background probe execution, no engine/import/export features. Android uses Kotlin/Compose + ViewModel/StateFlow/SavedStateHandle and JNI-packaged CMake libraries for ARM64/ARMv7/x86_64. iOS uses SwiftUI NavigationStack, MainActor observable state, scene restoration and local PixAuraCore Swift package. The package compiles the same C++ source, through the existing C module. Native adapters retain future GPU/inference ownership; no pixel semantics are duplicated.

ADR 0007 records the coordinator/build-input decision; ADRs 0001-0006 remain unchanged. Future Home -> Import -> Editor -> Compare -> Export extends typed coordinator paths after domain identity exists. Current shell state is not a persisted project or editing history. UI uses system typography/colors, scrollable content and labeled native controls; it is not the final visual identity.

## Files created or changed
Android: settings.gradle.kts, root/app build.gradle.kts, gradle.properties, Gradle wrapper scripts/JAR/properties, app/gradle.lockfile and verification-metadata.xml; manifest, theme/strings, neutral vector icon and backup/data-extraction rules; MainActivity, ShellViewModel, PixAuraTheme and navigation/ShellState; JVM ShellStateTest and instrumented ShellTest. CoreProbe.kt moved into bridge/src/main/kotlin/ai/pixaura/bridge with its package/JNI name unchanged. Native bridge and shared C ABI/source are unchanged.

iOS: PixAuraApp, ShellView, ShellModel, ShellState, PrivacyInfo.xcprivacy, ShellTests and ShellUITests; committed PixAuraAI.xcodeproj/project.pbxproj and shared scheme. Tooling: generate-ios-project.mjs, check-ios-shell.sh, isolated local AVD preparation script, tests/shells.test.mjs, updated foundation tests, native-shells.yml and upgraded foundation.yml actions/lint scope. Documentation: this report, ADR 0007, DEPENDENCIES, DECISIONS, AGENTS, README, ARCHITECTURE, QUALITY_GATES, ROADMAP, TESTING_STRATEGY, TOOLCHAIN and historical Phase 0 annotation. Generated binaries/logs/AVD/cache remain ignored.

## Local commands and exact observed results
Host tools: Windows PowerShell, Node 24.14.0, Gradle 8.13, local JetBrains JDK 21.0.10, AGP 8.13.2, Kotlin 2.2.21, SDK 36/build tools 35.0.0, NDK 28.2.13676358, CMake 3.22.1. Linux WSL: Clang 21.1.8, CMake 4.2.3, Ninja 1.13.2.

```
node --test tests/foundation.test.mjs tests/ci-tools.test.mjs tests/shells.test.mjs
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check-native-zig.ps1 -ZigPath G:/PixAuraAI/build/tools/zig-x86_64-windows-0.14.1/zig.exe
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check-native.ps1
build/tools/actionlint/actionlint.exe -shellcheck= -pyflakes= .github/workflows/foundation.yml .github/workflows/native-shells.yml
```
Node: 11/11 pass. Zig fallback: independent C/C++ DLL consumers and UB trap run pass. MSVC script: blocked, existing installation missing vcvarsall; unchanged optional environment issue. actionlint: exit 0. Windows does not have shellcheck/pyflakes; Bash syntax checked through WSL.

```
wsl --distribution Ubuntu --exec bash -lc 'cd /mnt/g/PixAuraAI && bash -n scripts/check-apple.sh scripts/check-ios-shell.sh scripts/check-sanitizers.sh && cmake -S . -B build/linux-host -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ && cmake --build build/linux-host && ctest --test-dir build/linux-host --output-on-failure && bash scripts/check-sanitizers.sh'
```
Exit 0: ordinary CTest 2/2; sanitizer CTest 4/4, including ASan diagnostic exit 86 and UBSan diagnostic exit 87. Native safety probes remain excluded from app targets.

Android build command (ANDROID_HOME=I:/AndroidStudioSDKdata, GRADLE_USER_HOME=G:/PixAuraAI/build/gradle-home):
```
gradle -p platforms/android --no-daemon assembleDebug assembleRelease testDebugUnitTest lintDebug lintRelease assembleDebugAndroidTest --write-locks --write-verification-metadata sha256
```
Exit 0, BUILD SUCCESSFUL in 39s, 146 tasks. Debug/release Kotlin and three-ABI JNI/C++ builds pass. JVM 2/2, zero skips/errors. Debug/release lint: no issues. Instrumentation APK builds. The initial failure from treating CoreProbe.kt as a directory was fixed by moving it into a proper source tree. Deprecated compiler DSL was replaced. Missing icon was fixed. Lint notices about newer versions were excluded specifically, preserving deliberate reviewed pins and all correctness checks. SDK XML metadata-version warning remains environmental; it did not prevent compilation.

Actual wrapper validation caught an extra character accidentally copied with the Gradle ZIP checksum. Corrected to the independently fetched official 64-character digest; strengthened the test to require an exact anchored value. Official wrapper JAR SHA-256 matches 81a82aaea5abcc8ff68b3dfcb58b3c3c429378efd98e7433460610fecd7ae45f. Linux AAPT2 checksum was independently added for compatible CI verification.

An isolated headless emulator uses official installed Android 36.1 x86_64 Google Play image revision 4, WHPX, software GPU, and repository-local AVD disks/config. `connectedDebugAndroidTest` initially failed before navigation because Compose's transitive Espresso 3.5.0 called an Android 16-removed InputManager method. Explicit official Espresso 3.7.0 test-only pin fixes that failure. Actual Gradle wrapper run with `connectedDebugAndroidTest assembleDebug assembleRelease testDebugUnitTest lintDebug lintRelease assembleDebugAndroidTest --write-verification-metadata sha256 --write-locks`: exit 0, BUILD SUCCESSFUL in 47s, 147 tasks; emulator 1/1, zero skips/failures. Test verifies actual JNI, visible ABI status, Home/Editor/Projects/Settings navigation and activity recreation/mode preservation.

Final strict run (no lock/checksum regeneration):
```
platforms/android/gradlew.bat -p platforms/android --no-daemon --offline connectedDebugAndroidTest assembleDebug assembleRelease testDebugUnitTest lintDebug lintRelease assembleDebugAndroidTest
```
Exit 0, BUILD SUCCESSFUL in 49s, 147 tasks (42 executed, 105 up-to-date). JVM tests re-executed: 2/2; actual emulator test re-executed: 1/1. Debug/release full lint pass. Offline dependency resolution and strict checksum/lock enforcement pass. Task-level SKIPPED entries for inapplicable Kotlin plugin validation, empty test prebuild, and redundant lint-vital reports are not counted as tests; both requested full lint variants executed. Final Node/source/doc/workflow/whitespace verification also passes. The isolated emulator was stopped after validation; no task build/test remains pending.

## Security/privacy and dependency evidence
No dangerous/media/Internet permissions, account, authentication or cloud/analytics SDK. Source Android manifest has no permission requests; inspection of the compiled release manifest confirms only AndroidX's app-internal signature permission for non-exported dynamic receivers. Its profile-installer service is guarded by system DUMP permission, not a requested app permission. Android backup and device transfer exclude all app data domains. iOS creates no media/project files; SceneStorage saves only shell route/mode. Its privacy manifest declares no tracking/collection, and the app-local UserDefaults reason CA92.1. Future managed project files must receive backup-exclusion/data-protection attributes at creation; that gate belongs to the engine phase, not a pretend storage feature here.

Dependency versions/licenses/provenance and measured APK sizes are recorded in [DEPENDENCIES](../DEPENDENCIES.md). OSV batch audit first executed for 293 locked Maven package/version entries, then re-executed for the final 289 entries after the Espresso test pin. Final classification: 107 release-runtime entries, zero OSV findings; 18 flagged tooling-only coordinates (AGP unified test platform: protobuf/Netty; Kotlin auxiliary configuration: BouncyCastle/OpenTelemetry). None of the flagged packages are in shipping runtime classpaths; their presence does not add app telemetry. Raw request/results are retained under ignored build/dependency-audit-*.json. Build-tool advisories require upstream upgrade review, not blind forced overrides of AGP's internal harness. Database coverage is limited; zero findings is not a claim of zero vulnerabilities.

## CI changes
Foundation boundaries remains Windows/Linux portable, Android ARM64/ARMv7 native, Apple package/C ABI and active sanitizer validation. Both workflows are actionlint-checked. Official actions upgraded to pinned checkout v5.0.0 and upload-artifact v6.0.0; new Android job uses pinned setup-java v5.0.0. All use Node 24. No third-party emulator action.

Native application shells adds source consistency; official SDK tools/JDK, Gradle debug/release/lint/unit and emulator JNI/Compose execution on ubuntu-24.04; macos-26/Xcode 26.6 runs Phase 0 package tests then iOS debug/release unsigned builds, app-hosted Swift/C ABI and UI tests on an available iPhone simulator. Artifacts retain result bundles/reports for 14 days. CI is prepared, not observed executing for this change.

## Final gate matrix
| Gate | Status | Evidence / limitation |
| --- | --- | --- |
| G0 source/docs/contracts | PASS locally | Authoritative docs, ADR 0007, links and source consistency |
| G1 hygiene/workflows | PASS locally | Node 11/11, actionlint, WSL Bash syntax; strict Android lint |
| G2/G3 Windows core | PASS via fallback | Zig C/C++ DLL and trap tests; optional MSVC unavailable |
| G2/G3 Linux core | PASS locally | Clang CMake/CTest 2/2 |
| G4 ASan/UBSan | PASS locally | 4/4, actual negative-probe diagnostic proof |
| G3 Android debug/release/JNI builds | PASS locally | Three ABIs; packaged core and bridge |
| G2 Android JVM reducer tests | PASS locally | 2/2, zero failures/skips |
| G2 Android emulator JNI/navigation/recreation | PASS locally | Android 16.1 x86_64 WHPX, 1/1; actual native load/status, destination navigation, activity recreation |
| G3 Apple package/core/Swift C ABI | BLOCKED locally | macOS/Xcode mandatory; CI configured, unobserved |
| G2/G3 iOS debug/release/app Swift/UI tests | BLOCKED locally | Real Xcode project/tests ready; Windows cannot execute them |
| G7 shell privacy policy | PASS source checks | Permissions/backup/manifest/dependencies; no media flows exist |
| G11 accessible shell source baseline | PASS locally | Native labeled controls, dynamic system text/colors, scrolling, large iOS controls, Android semantics smoke |
| G11 assistive-technology walkthrough | BLOCKED / unrun | Physical TalkBack/VoiceOver walkthrough not performed; manual/device evidence remains required where applicable |
| G5/G6/G8/G9/G10/G12 | NOT_APPLICABLE | No engine/persistence/render/AI/commerce/release work in Phase 1 |

## Blockers, warnings and human action
Apple execution: owner repository maintainer; affected G2/G3/G11; remediation run the prepared macOS job and retain actual build/XCTest evidence. Current host has no Xcode/CoreSimulator; no iOS build PASS is claimed. Missing authenticated Phase 0 run URL: owner repository maintainer; supply it to reconcile user-attested completion with historical report. Optional MSVC repair is host maintenance, not a mobile architecture change.

Build-tool advisory debt: owner build maintainer, review by 2026-11-02 or before release, whichever comes first; upgrade official AGP/Kotlin in a separate regression-tested change. Exposure limited to build/test tools and trusted synthetic tests in disposable CI; no flagged package ships in app runtime. SDK XML warning and unshrunk APK size are recorded; neither is hidden by broad lint suppression. Physical assistive-technology review remains unrun. No credentials/signing blocker for unsigned CI.

Exact next action: review the changes, commit them locally, and explicitly approve/push the commit to origin. In GitHub Actions run **Foundation boundaries** and **Native application shells** (push triggers both, or use Run workflow). Supply the run URLs and Android/Apple evidence artifacts, and record the native assistive-technology walkthrough before promoting the applicable G11 gate. No signing credentials are needed for the CI builds/tests. Diagnose any macOS-only failures before declaring Phase 1 fully passed. Do not advance to Phase 2.
