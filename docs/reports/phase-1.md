# Phase 1 native application shells report

Updated: 2026-10-03. Scope: native shells only; Phase 2 not started. Status: **fifth CI failure independently inspected; emulator API 24+ first-time userdata clamp confirmed and reproduced with the exact hosted build/image; explicit-data remediation resolves to 2048 MiB locally; hosted JNI/Compose gate pending**.


## Fifth CI rerun: emulator first-time setup overrides userdata, 2026-10-03

Inspected the actual [Native application shells run 37112785427](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37112785427) at commit 4c5a7d63d542dfde8928695bc3a5092ed2a3ed4f, Android job 111173793207, GitHub job/step statuses, full Android log and android-shell-evidence artifact 11270281281. Used the existing GitHub credential for read-only downloads; credentials stayed in memory and were not displayed or written. Evidence is retained under ignored build/emulator-investigation. Source and ios-app jobs **PASS**; Android debug/release/JVM/lint/instrumentation APK step **PASS**; hosted emulator startup **FAIL**. These fifth-run statuses are independently observed, unlike the older user-attested entries below. iOS was not executed locally.

Hosted artifact: complete pre-launch avd-config.ini contains disk.dataPartition.size=2048M, disk.cachePartition.size=128M, PlayStore.enabled=no, image.sysdir.1=system-images/android-35/google_apis/x86_64/ and the generic avdmanager defaults, with no named hardware profile enforcing 6 GiB. Console logs show 6903 MiB free and 4224 MiB required. Emulator stdout identifies **37.2.12.0 / build 16428233**, the API 35 Google APIs x86_64 system directory, then fatal userdata creation at the repository AVD path: **6903.40 MB available; 7372.80 MB needed**. SDK installation logs identify **x86_64-35_r09.zip**. The old artifact did not retain post-resolution hardware-qemu.ini, post-failure config.ini, package metadata or userdata geometry; those historical omissions are explicit, not reconstructed hosted evidence.

Confirmed source: Android Emulator QEMU2 [android-qemu2-glue/main.cpp](https://android.googlesource.com/platform/external/qemu/+/emu-master-dev/android-qemu2-glue/main.cpp), firstTimeSetup and kMinPlaystoreImageSize. firstTimeSetup is true when wipe-data is requested **or the runtime userdata file is absent**. For API >=24 **or** PlayStoreImage, that path raises the parsed disk size to **6 * 1024^3 = 6442450944 bytes = 6144 MiB** and calls avdInfo_replaceDataPartitionSizeInConfigIni. It then applies a **1.2** free-space factor: **6144 * 1.2 = 7372.80 MiB**. This is an emulator creation-time minimum applied after config/command-line parsing; despite the constant's name it also applies to this Google APIs / PlayStore-disabled AVD. Changing the profile or disabling the Play Store flag would not remove the API-level condition. Neither writing 2048M nor passing -partition-size 2048 prevented this later clamp.

Reproduced with the **exact Linux emulator 37.2.12 / build 16428233** and **exact API 35 Google APIs x86_64 revision 9** downloaded from official SDK metadata, not the installed local Android 36 image. Original hosted config plus the original wipe-data/partition-size arguments produced post-resolution **config.ini disk.dataPartition.size=6442450944** and **hardware-qemu.ini disk.dataPartition.size=6g**. Original resolution log, rewritten config and generated hardware are retained in build/emulator-investigation/reproduction-avds/pixaura-original.avd and original-resolved.log. This directly confirms the effective-size source in the actual hosted binary.

The matching revision 9 package has **no userdata.img**; its data/empty_data_disk explicitly declares factory-empty userdata. Therefore the fix creates a fresh, private **2147483648-byte raw ext4 image** under the new CI AVD using mke2fs, then supplies it through documented [-data and -datadir](https://developer.android.com/studio/run/emulator-commandline). Each run uses a unique newly formatted file. The launch omits -wipe-data because it would reactivate firstTimeSetup and overwrite this fresh image with the forced 6 GiB partition; isolation and factory reset are provided by formatting a new file, not by retaining old test state. Images without the factory-empty marker fail explicitly rather than inventing initial data. Normal ext4 metadata checksums/features, SDK/disk safety checks, required actual emulator execution and strict dependency/lint gates remain enabled. API 35 x86_64, 2048 MiB bound, 128 MiB cache, SD disabled, KVM preference/software fallback and existing readiness/instrumentation deadlines remain.

Pre-launch proof is now mandatory: inspect real ext4 superblock/file size, invoke the SDK's -check-snapshot-loadable diagnostic with the actual launch arguments plus -verbose (120s + 5s bound; no guest CPU execution), copy the resulting hardware-qemu.ini/config.ini, inspect raw virtual size with bundled qemu-img, and reject missing/duplicate/oversized hardware sizes, wrong data paths, initPath recreation, corrupt image geometry or QEMU size mismatch. The intentional absent snapshot reports Not loadable while the diagnostic exits; that message is not boot/test evidence. The actual test VM launches only after size/path assertions succeed. After ADB visibility, assert the running hardware and QEMU raw/overlay virtual size/backing path again before boot/package readiness and instrumentation. Assertion or resolution failures retain the original failure status and evidence.

Exact fixed-resolution proof from the matching binary/image: **hardware-qemu.ini disk.dataPartition.size=2g**; explicit disk.dataPartition.path points to the newly formatted CI image; fileBytes, ext4VirtualBytes and resolvedBytes all **2147483648**, equal to the 2048 MiB bound. Retained proof: build/emulator-investigation/exact-fixed-size.json; fixed-resolved.log; reproduction-avds/pixaura-fixed.avd/hardware-qemu.ini. A bounded CPU-paused local VM (-qemu -S, 30s overlay-creation budget) then created the actual QCOW2 userdata overlay: qemu-img reports virtual-size **2147483648**, format qcow2, and full-backing-filename equal to the verified raw image. assert-android-userdata reports fileBytes/ext4VirtualBytes/resolvedBytes/qemuVirtualBytes/boundBytes all **2147483648**. Proof is retained in build/emulator-investigation/exact-qemu-overlay.json and exact-overlay-size.json. The owned VM was stopped; this paused inspection is not Android boot or instrumentation evidence. Configuration-only resolution does not count as a successful guest boot or JNI/Compose test.

Evidence additions before VM launch: created/full configured/full resolved AVD configs; hardware-qemu before-state (explicitly absent when not generated), resolved hardware, emulator version/package metadata, system-image source.properties/package.xml/advancedFeatures.ini where present, initial userdata.img virtual size or explicit absence plus data directory/empty marker, real prepared/resolved userdata reports, exact launcher and diagnostic argument lists, verbose resolution output. On exit, preserve latest config/hardware even on failure. The existing always-upload directory covers every new file. No larger disk admission budget or additional cleanup was added; safe workspace-only cleanup and disk-space preflight remain intact.

Changed files: scripts/check-android-emulator.sh; new scripts/assert-android-userdata.mjs; tests/android-emulator.test.mjs; tests/ci-tools.test.mjs; .github/workflows/native-shells.yml (comment only); docs/DEPENDENCIES.md (engineering tool review); this report. No application/native code, project/dependency locks, architecture decisions, Phase 2 work, commit or push.

Final local validation: Linux combined Node **41/41**, zero skips/failures (16 source/geometry checks + 25 orchestration scenarios); Windows Node **16/16**, zero skips/failures; actionlint and git diff --check exit 0; Zig C/C++ consumers/trap **PASS**; Linux core **2/2**, ASan/UBSan **4/4** (negative exits 86/87); Bash syntax and exact-build resolution **PASS**. MSVC check re-executed: **BLOCKED**, installed workload still lacks vcvarsall; fallback passed. Owner host maintainer; optional remediation repair desktop C++ workload. Shellcheck/pyflakes remain unavailable locally. Linux source/orchestration tests include real mke2fs-created ext4 images, all previous disk/readiness/cleanup cases, and effective-size/path/geometry/recreation/QEMU mismatch/diagnostic error/timeout cases. They never substitute for actual hosted instrumentation.

Commands used (from repository root):

```text
node --test tests/foundation.test.mjs tests/ci-tools.test.mjs tests/shells.test.mjs
build/tools/actionlint/actionlint.exe -shellcheck= -pyflakes= .github/workflows/foundation.yml .github/workflows/native-shells.yml
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check-native.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check-native-zig.ps1 -ZigPath G:/PixAuraAI/build/tools/zig-x86_64-windows-0.14.1/zig.exe
wsl --distribution Ubuntu --exec bash /mnt/g/PixAuraAI/build/emulator-investigation/reproduce.sh
wsl --distribution Ubuntu --exec bash /mnt/g/PixAuraAI/build/emulator-investigation/verify-overlay.sh
wsl --distribution Ubuntu --exec bash -lc 'cd /mnt/g/PixAuraAI && bash -n scripts/*.sh && build/tools/node-v24.14.0-linux-x64/bin/node --test tests/foundation.test.mjs tests/ci-tools.test.mjs tests/shells.test.mjs tests/android-emulator.test.mjs && cmake --build build/linux-host && ctest --test-dir build/linux-host --output-on-failure && bash scripts/check-sanitizers.sh'
git diff --check
```

Remaining CI-only gate: corrected hosted API 35 boot, on-runner effective-size assertion and actual JNI/Compose connectedDebugAndroidTest on the new revision. Owner repository maintainer; affected G2 emulator execution / Phase 1 promotion. Exact next human action: review the seven changed files, then commit/push through the maintainer's process or separately authorize it; run Native application shells on the new revision and retain the URL/android-shell-evidence artifact, especially userdata-resolved.json, hardware-qemu.resolved.ini, userdata-running-qemu-info.json and actual instrumentation results. No commit/push was authorized for this turn. Phase 0 CI completion remains user-attested; physical assistive walkthrough remains unrun. Do not begin Phase 2.

## Fourth CI rerun: confirmed userdata disk exhaustion, 2026-10-03

User-supplied hosted-runner diagnostic: `FATAL | Not enough space to create userdata partition. Available: 6903.38 MB need 7372.80 MB`. The emulator exited before device visibility; `device 'emulator-5554' not found` is a downstream symptom. This establishes the cause left unknown in the third-rerun history below. Source, ios-app, Android debug/release/JVM/lint/instrumentation APK gates PASS are user-attested; no run URL was supplied and no Apple execution is locally observed.

Acceptance: explicit smaller writable disks on the same repository-isolated API 35 Google APIs x86_64 AVD; bounded free-space preflight; narrowly scoped CI cleanup only if needed; diagnostics and required instrumentation retained. No architecture, dependencies or app behavior change; no Phase 2, commit or push.

Exact disk configuration: replace inherited disk keys with `disk.dataPartition.size=2048M`, `disk.cachePartition=yes`, `disk.cachePartition.size=128M`, `hw.sdCard=no`; remove inherited sdcard.size/path. Launch also explicitly passes `-partition-size 2048 -cache-size 128`, retaining wipe-data/no-snapshot, software GPU, 2048 MB RAM, two cores and KVM preference/software fallback. Android documents these [emulator disk options](https://developer.android.com/studio/run/emulator-commandline). The source image is unchanged; actual hosted boot remains the acceptance gate for its compatibility with the smaller writable partition.

Before launch, df checks the AVD filesystem (10s plus 5s termination grace), requiring 4224 MiB = 2048 userdata + 128 cache + 2048 reserve for writable overlays/test output. This is a conservative admission budget, not a guarantee against subsequent external disk consumption. If insufficient and CI=true/GITHUB_ACTIONS=true, one cleanup removes only repository platforms/android/app/build/tmp after realpath verifies the exact expected path; symlink redirects are rejected. Cleanup has a 30s plus 5s bound, followed by one disk recheck. Local execution does not delete temporary output. SDK components, APKs, reports, dependency caches and outside-workspace files are preserved. Low space or invalid measurement fails before emulator launch with the existing exit-status/evidence cleanup intact.

Free MiB, configured sizes, required reserve and AVD disk keys appear in console and disk-preflight.log; the complete config.ini is copied into the always-upload evidence. Existing readiness/deadline/process-cleanup/strict dependency verification/instrumentation behavior remains required.

Changed files: scripts/check-android-emulator.sh; .github/workflows/native-shells.yml (explanatory comment); tests/android-emulator.test.mjs; this report. Seven additional fake-SDK behaviors cover sufficient space/default-key replacement, exact 4224 MiB threshold, successful cleanup/recheck, persistent low space, local no-cleanup, symlink rejection and invalid df output. These supplement all eight existing readiness/exit/cleanup behaviors; fake instrumentation is never real JNI/Compose evidence.

Local observed validation: Windows source/foundation/workflow/shell Node **PASS 15/15**, zero skips; Linux combined suite **PASS 30/30**, zero skips (15 source checks + 15 orchestration scenarios); all scripts Bash syntax **PASS**; actionlint **PASS** (shellcheck/pyflakes unavailable); Windows Zig C/C++ boundary and trap checks **PASS**; Linux core CTest **PASS 2/2**; ASan/UBSan **PASS 4/4**, negative diagnostic exits 86/87. WSL sandbox E_ACCESSDENIED was resolved by an approved escalated run. The standard Windows check-native.ps1 was re-executed and is **BLOCKED** by the existing incomplete Visual Studio desktop C++ workload (vcvarsall missing); the required compiler fallback passed. Owner: host maintainer; remediation repair that workload; affected direct MSVC G2/G3 validation, already covered locally through the fallback. No real emulator, Android app rebuild or Apple execution is claimed for this disk-only change.

Commands:

```text
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check-native.ps1
node --test tests/foundation.test.mjs tests/ci-tools.test.mjs tests/shells.test.mjs
build/tools/actionlint/actionlint.exe -shellcheck= -pyflakes= .github/workflows/foundation.yml .github/workflows/native-shells.yml
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check-native-zig.ps1 -ZigPath G:/PixAuraAI/build/tools/zig-x86_64-windows-0.14.1/zig.exe
wsl --distribution Ubuntu --exec bash -lc 'cd /mnt/g/PixAuraAI && bash -n scripts/*.sh && build/tools/node-v24.14.0-linux-x64/bin/node --test tests/foundation.test.mjs tests/ci-tools.test.mjs tests/shells.test.mjs tests/android-emulator.test.mjs && cmake --build build/linux-host && ctest --test-dir build/linux-host --output-on-failure && bash scripts/check-sanitizers.sh'
git diff --check
```

Remaining CI-only gate: actual hosted Ubuntu API 35 emulator boot and JNI/Compose connectedDebugAndroidTest on this corrected revision. Owner: repository maintainer; affected G2 / Phase 1 promotion. Exact next human action: review these four files, commit/push through the maintainer's process, run Native application shells on the new revision, and retain the run URL plus android-shell-evidence artifact with disk-preflight.log, avd-config.ini and actual instrumentation results. Rerunning the old remote revision cannot validate this fix. Phase 0 CI completion remains user-attested; physical assistive walkthrough remains unrun. Do not begin Phase 2.

## Third CI rerun: Android emulator orchestration, 2026-10-03

The user supplied the third rerun result after commit 3c1467f322f0abfb85529fe9d58c1daad6ad0890: source **PASS**, ios-app **PASS**, Android debug/unsigned Release builds, JVM tests, full lint and instrumentation APK build **PASS**. Only Execute JNI and Compose smoke tests on Android emulator **FAIL**, exit 124. ADB server started successfully; cleanup printed `kill: (...) - No such process`. These CI outcomes remain user-attested; no run URL or full emulator logs were available for independent inspection. They supersede historical lint/dependency/Apple blockers below.

Exact timed-out command in the inspected workflow: `timeout 180 "$ANDROID_HOME/platform-tools/adb" wait-for-device`. It is the step's only explicit timeout; the later boot loop's failed `test` would return 1, not 124. [GNU timeout](https://www.gnu.org/software/coreutils/manual/html_node/timeout-invocation.html) documents 124 for expiry. Confirmed failure: no ADB-visible emulator within that 180-second device wait, before Gradle instrumentation was invoked. The missing process at cleanup indicates that launch PID was already gone by then; why it exited is **not established** by the supplied excerpt. The old `kill ... || true` already prevented cleanup's missing-process status from becoming the test result; the message was misleading noise, not the root failure.

Acceptance for this fix: deterministic owned serial/AVD, bounded device/boot/package readiness and test execution, prompt early-exit detection, idempotent cleanup preserving all failure statuses, and retained startup/test evidence. Added scripts/check-android-emulator.sh and wired the existing CI emulator step to it. The actual `connectedDebugAndroidTest` task and app/instrumentation source are unchanged; dependency verification stays strict, targetSdk stays 36, and no test is skipped, retried into PASS, or made continue-on-error.

The script starts ADB before launch, checks that emulator-5554 is unused, creates only a repository-isolated API 35 Google APIs x86_64 AVD, cold-boots/wipes that CI-owned AVD, and explicitly binds even port 5554 / ANDROID_SERIAL=emulator-5554. It retains the existing software GPU mode and uses 2 cores / 2048 MB RAM. It checks emulator liveness during every readiness stage; a dead process fails immediately and records its exit status instead of silently waiting for a device that cannot appear.

| Stage | Bound / readiness condition |
| --- | --- |
| ADB server / AVD creation | 30s / 60s, each with a 5s forced-termination grace |
| Device visibility | 180s; serial-specific get-state must return device |
| Android boot | 600s; sys.boot_completed must equal 1 |
| Package manager | 120s; pm path android must return a package path, not merely status 0 |
| Each ADB probe / diagnostics call | 10s plus 5s forced-termination grace; polling interval at most 2s |
| Gradle JNI/Compose instrumentation | Independent 900s (15 minutes), plus 10s forced-termination grace |
| CI emulator step | 35 minutes; existing job budget remains 45 minutes |

Hardware acceleration remains enabled when /dev/kvm is accessible and the emulator's bounded accel-check succeeds. Permission setup is bounded/noninteractive. Otherwise the script selects the documented [software acceleration fallback](https://developer.android.com/studio/run/emulator-commandline) `-accel off` and retains the same required test. A software boot exceeding the budget still fails; no instrumentation failure is hidden by fallback.

Cleanup captures the original status, bounded diagnostics and process state, then stops only the owned emulator PID if still alive, with bounded TERM/KILL escalation. It exits with the original status even when the emulator already died. The always-upload artifact includes separate emulator stdout/stderr; ADB server/create/acceleration/readiness logs; prelaunch/final adb devices output; full boot properties and package-manager state; logcat; final phase/exit code; Gradle output; and existing Android test result/report directories. Console output names the timed-out stage and exposes emulator log tails.

Changed files: .github/workflows/native-shells.yml; scripts/check-android-emulator.sh; tests/ci-tools.test.mjs; tests/shells.test.mjs; tests/android-emulator.test.mjs; this report. New source assertions require bounded staged readiness, explicit serial, separate test deadline, process-aware status-preserving cleanup, strict verification and always-upload evidence. The Linux source CI job also runs eight executable fake-SDK/Gradle behavior scenarios: success with delayed readiness, emulator early exit 23, invisible device, stalled ADB, boot timeout, package-manager timeout despite shell status 0, instrumentation exit 7 after emulator exit, and independent instrumentation timeout 124. Tests prove failure statuses survive cleanup, Gradle waits for readiness, evidence exists and mock emulator processes do not leak. They validate orchestration; they do **not** count as real JNI/Compose execution.

Local observed validation: Windows Node source/foundation/CI/shell checks **PASS 15/15**, zero skips; actionlint **PASS**, exit 0; Bash syntax **PASS**; Windows Zig fallback C/C++ consumers and UB trap **PASS**; Linux CMake/CTest **PASS 2/2**; ASan/UBSan **PASS 4/4**, including diagnostic exits 86/87. Linux combined Node suite **PASS 23/23** (15 source checks + 8 orchestration behaviors), zero skips. Shellcheck/pyflakes are unavailable locally and are not claimed PASS. WSL initially lacked Node; downloaded only the official Node 24.14.0 Linux x64 archive under ignored build/tools, verified SHA256 `41cd79bb7877c81605a9e68ec4c91547774f46a40c67a17e34d7179ef11729df` against the official release SHASUMS256.txt before extraction/execution. This is the existing Node baseline, MIT licensed with bundled notices; no shipping dependency, toolchain pin or production binary size changed, and no system package was installed.

```text
node --test tests/foundation.test.mjs tests/ci-tools.test.mjs tests/shells.test.mjs
build/tools/actionlint/actionlint.exe -shellcheck= -pyflakes= .github/workflows/foundation.yml .github/workflows/native-shells.yml
git diff --check
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check-native-zig.ps1 -ZigPath G:/PixAuraAI/build/tools/zig-x86_64-windows-0.14.1/zig.exe
wsl --distribution Ubuntu --exec bash -lc 'cd /mnt/g/PixAuraAI && bash -n scripts/check-apple.sh scripts/check-ios-shell.sh scripts/ios-simulator-architecture.sh scripts/check-sanitizers.sh scripts/check-android-emulator.sh && build/tools/node-v24.14.0-linux-x64/bin/node --test tests/foundation.test.mjs tests/ci-tools.test.mjs tests/shells.test.mjs tests/android-emulator.test.mjs && cmake --build build/linux-host && ctest --test-dir build/linux-host --output-on-failure && bash scripts/check-sanitizers.sh'
```

No real emulator/instrumentation, Android application rebuild or Xcode execution is claimed for this orchestration-only follow-up. Previous local memory-pressure and incomplete MSVC limitations remain historical environment evidence. Remaining mandatory CI gate: launch the real API 35 emulator on the hosted Ubuntu runner, validate actual KVM/fallback startup and execute JNI/Compose tests with the corrected script. Owner: repository maintainer; affected G2 emulator execution / Phase 1 promotion. If launch still fails, the new stdout/stderr and phase artifacts must identify its underlying cause before declaring PASS.

Exact next human action: review these six files, then commit/push through the maintainer's process or separately authorize the agent; run Native application shells on that new revision and retain its URL/android-shell-evidence artifact. Rerunning the existing remote revision cannot exercise these local changes. No commit, push, app behavior change, test removal, architecture redesign or Phase 2 work occurred. Physical assistive walkthrough remains unrun.

## Second CI rerun: Android target-SDK lint remediation, 2026-10-03

The user reports the rerun after commit 2557fd4bebf051916f40163167ad7a822b972415: source job **PASS**, ios-app **PASS**, and Android dependency verification **PASS**. The earlier missing metadata and iOS simulator architecture mismatch are resolved in that run. These CI outcomes are user-attested; no run URL/artifact was supplied or independently fetched in this session. iOS is not locally executed on Windows. Historical pending-Apple statements below describe earlier validation, not this newer CI result.

The supplied Android diagnostic confirms the remaining failure is `:app:lintDebug`, build.gradle.kts:15, `[OldTargetApi]`: targetSdk 36 is not the latest API. Strict `warningsAsErrors = true` promotes that warning to an error. The approved Phase 1 baseline intentionally remains SDK 36; API 37 / Android 17 migration requires an explicit platform upgrade with behavior-change testing.

Remediation: add only `disable += "OldTargetApi"` in the existing Android lint block, with a comment explaining the pinned target and explicit API 37 migration. `targetSdk = 36`, `warningsAsErrors = true`, and `abortOnError = true` remain unchanged. Existing AndroidGradlePluginVersion/GradleDependency exceptions remain unchanged; no unrelated issue was newly suppressed, and no lint baseline, warning downgrade or dependency/build-input change was introduced. The new source test requires the exact explicit target, strict lint settings and exact three-item exception set (the two existing version-review exceptions plus the sole target-SDK exception OldTargetApi), and rejects broad baseline/ignore/checkOnly settings.

Changed files for this follow-up only: platforms/android/app/build.gradle.kts, tests/shells.test.mjs, and this report. No architecture change, Phase 2 work, commit or push occurred.

Local validation commands:

```text
node --test tests/foundation.test.mjs tests/ci-tools.test.mjs tests/shells.test.mjs
build/tools/actionlint/actionlint.exe -shellcheck= -pyflakes= .github/workflows/foundation.yml .github/workflows/native-shells.yml
git diff --check
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check-native-zig.ps1 -ZigPath G:/PixAuraAI/build/tools/zig-x86_64-windows-0.14.1/zig.exe
wsl --distribution Ubuntu --exec bash -lc 'cd /mnt/g/PixAuraAI && bash -n scripts/check-apple.sh scripts/check-ios-shell.sh scripts/ios-simulator-architecture.sh scripts/check-sanitizers.sh && cmake --build build/linux-host && ctest --test-dir build/linux-host --output-on-failure && bash scripts/check-sanitizers.sh'
```

Observed **PASS**: Node **14/14**, zero skips/failures; actionlint exit 0 (unavailable shellcheck/pyflakes disabled); whitespace checks; Windows fallback C/C++ consumers and UB trap; Linux core **2/2**; ASan/UBSan **4/4** including negative diagnostic exits 86/87; Bash syntax. The existing MSVC installation limitation is unchanged and was not re-probed for this lint-only follow-up.

Android validation uses ANDROID_HOME=I:/AndroidStudioSDKdata and the previously populated isolated GRADLE_USER_HOME=G:/PixAuraAI/build/gradle-remediation-verify-final. No metadata or lock generation, SDK upgrade or network access is requested. Build-cache reuse is disabled and actionable tasks are forced to rerun:

```text
platforms/android/gradlew.bat -p platforms/android --no-daemon --offline --dependency-verification strict --no-build-cache --rerun-tasks assembleDebug assembleRelease testDebugUnitTest lintDebug lintRelease assembleDebugAndroidTest connectedDebugAndroidTest
```

The combined build/emulator attempt failed before Gradle execution because Windows paging-file capacity could not satisfy JVM startup (error 1455). A smaller-heap attempt also failed during JVM initialization. The isolated emulator was unresponsive to graceful adb shutdown and only its two identified processes (started by this task) were stopped. Retried the build/unit/lint/APK gates without the emulator using these local command-line resource overrides; no committed JVM setting was changed:

```text
JAVA_OPTS=-Xmx64m -Xms32m -XX:+UseSerialGC
platforms/android/gradlew.bat -p platforms/android --no-daemon --offline --dependency-verification strict --no-build-cache --rerun-tasks --max-workers=1 "-Dorg.gradle.jvmargs=-Xmx768m -Xms128m -XX:+UseSerialGC -Dfile.encoding=UTF-8" -Pkotlin.compiler.execution.strategy=in-process assembleDebug assembleRelease testDebugUnitTest lintDebug lintRelease assembleDebugAndroidTest
```

Reduced-memory Android run **PASS**, exit 0, BUILD SUCCESSFUL in **2m 42s**, **146/146 actionable tasks executed**. Debug/unsigned Release builds and three-ABI JNI compilation pass; JVM tests re-executed **2/2**, zero skips/failures/errors; full Debug/Release lint reports say no issues; instrumentation APK builds. Strict dependency verification and existing locks remain enforced offline. The local SDK inventory is not the hosted runner's latest SDK inventory; this does not claim reproduction of the hosted OldTargetApi diagnostic. The source check verifies the narrow exception directly, and executed lint verifies that remaining checks still run.

Emulator instrumentation for this follow-up is **BLOCKED / unrun** by local memory pressure; the earlier Phase 1 instrumentation PASS is historical evidence and is not counted as a new run. Owner: host maintainer; remediation provide sufficient free commit memory/paging-file capacity or execute the prepared Android emulator CI step; affected local G2 emulator evidence. The isolated emulator is stopped, and no build/test remains pending. One intermediate Node hygiene check while Gradle was active encountered its temporary `.kotlin/sessions/*.salive` file; rerun source checks after build completion rather than modifying source-hygiene rules for this lint-only change.

Remaining CI-only gate: rerun android-app on the corrected revision with the hosted SDK inventory, confirming full strict lint/build/unit/emulator completion. Owner: repository maintainer; affected G1/G2/G3 Android CI and Phase 1 promotion. Exact human action: review these three files, then commit/push through the maintainer's process (or separately authorize the agent) and run Native application shells on that new revision; retain the run URL and Android evidence artifact. Rerunning the unchanged remote commit cannot exercise this fix. iOS CI PASS is preserved as user-attested evidence; no local iOS PASS is claimed. Physical assistive walkthrough remains unrun. Do not begin Phase 2.

## Confirmed CI remediation, 2026-10-03

The user supplied actual failure diagnostics after the initial inspection below. Android's clean root-project `classpath` resolution rejected three unlisted parent/BOM metadata artifacts. iOS's Swift package emitted `arm64-apple-ios-simulator.swiftmodule` while the app/test compiler targeted x86_64, causing incompatible-target/module-resolution diagnostics and xcodebuild exit 65. These supersede the earlier missing-log blocker. No architecture redesign, Phase 2 work, dependency upgrade, commit or push occurred.

### Android checksum remediation and clean-cache evidence

Reproduced the exact three-artifact classpath failure with the wrapper and a previously empty GRADLE_USER_HOME=G:/PixAuraAI/build/gradle-remediation-generate:

```text
platforms/android/gradlew.bat -p platforms/android --no-daemon --dependency-verification strict help
```

Running checksum generation afterward in that now-populated cache succeeded but added none of the missing entries: cached parsed dependency metadata bypassed the parent/BOM downloads. A second empty-cache strict run confirmed the same failure persisted. This demonstrated why a successful cached build/generation command alone was inadequate evidence.

Generated the final metadata directly from another previously empty GRADLE_USER_HOME=G:/PixAuraAI/build/gradle-remediation-generate-clean, with ANDROID_HOME=I:/AndroidStudioSDKdata:

```text
platforms/android/gradlew.bat -p platforms/android --project-cache-dir G:/PixAuraAI/build/gradle-remediation-generation-project --no-daemon --write-verification-metadata sha256 assembleDebug assembleRelease testDebugUnitTest lintDebug lintRelease assembleDebugAndroidTest
```

Observed **PASS**, exit 0, 3m 20s, 146 tasks executed. Gradle's supported workflow downloaded the artifacts from the existing repositories and added exactly three SHA256 records (13 XML lines), with origin `Generated by Gradle`:

| Artifact | SHA256 |
| --- | --- |
| com.google.guava:guava-parent:33.3.1-jre / guava-parent-33.3.1-jre.pom | 55441db27e8869dfefe053059bdf478bdc7e95585642bf391f0023345fd56287 |
| org.junit:junit-bom:5.10.2 / junit-bom-5.10.2.module | de23b114b3e4119a8fe6eb17bed5a3852816698bace67071579d6d927ebb080a |
| org.jetbrains.kotlinx:kotlinx-coroutines-bom:1.8.0 / kotlinx-coroutines-bom-1.8.0.pom | 1239e9dbe1397cd5971342956b2511bc3ace7b641842e4372a088dcfa8b9ad55 |

Independent Get-FileHash inspection of those actual Gradle-downloaded files matched the generated entries. No checksum was invented or copied from an external checksum service. No existing checksum was replaced, metadata verification disabled, trust exception added, repository changed, or lockfile regenerated. CI's two Gradle commands now explicitly pass `--dependency-verification strict`. A source regression check requires those artifact-specific SHA256 entries, metadata verification enabled, no trusted-artifact exception, and strict CI commands.

Final validation uses a further previously empty GRADLE_USER_HOME=G:/PixAuraAI/build/gradle-remediation-verify-final, a separate project cache, build caching disabled and forced task execution; it does not write verification metadata or locks:

```text
platforms/android/gradlew.bat -p platforms/android --project-cache-dir G:/PixAuraAI/build/gradle-remediation-final-project --no-daemon --no-build-cache --dependency-verification strict --rerun-tasks assembleDebug assembleRelease testDebugUnitTest lintDebug lintRelease assembleDebugAndroidTest connectedDebugAndroidTest
```

Final strict fresh-cache run **PASS**, exit 0, BUILD SUCCESSFUL in 3m 27s, **147/147 actionable tasks executed**. Debug/unsigned Release builds and three-ABI JNI/C++ compilation passed; JVM tests actually re-executed **2/2**, zero skips/failures/errors; full Debug/Release lint reports contain no issues; instrumentation APK built and emulator test actually executed **1/1**, zero failures. No cached test or lint result is counted as new execution. SDK XML metadata warning remains nonfatal. The fresh Windows dependency resolution exercises the confirmed classpath issue; the corresponding clean Ubuntu runner, Linux AAPT2 and API 35 KVM instrumentation still require the next GitHub run. The existing Linux AAPT2 checksum remains intact. Local instrumentation uses the repository-isolated Android 16 x86_64 AVD, not a user's AVD; it was stopped after validation.

### iOS architecture remediation and verification limits

Inspected the deterministic project generator/generated project, local Swift package product wiring, shared scheme, both Apple scripts, simulator selection and macos-26 workflow. Existing project settings contained no explicit ARCHS, ONLY_ACTIVE_ARCH or EXCLUDED_ARCHS; the simulator commands specified only an id, leaving architecture selection implicit. The user-provided diagnostic establishes the resulting package/app mismatch; Windows cannot independently observe resolved Xcode settings.

The project generator now explicitly inherits `ARCHS = "$(ARCHS_STANDARD)"` for both configurations, and its committed project output was regenerated. It remains deterministic, without fixed arm64/x86_64 or excluded architectures in any target. The shared helper scripts/ios-simulator-architecture.sh obtains the executing host architecture from `uname -m`, accepts arm64 and x86_64, rejects unsupported hosts and logs the selected architecture. Both check-apple.sh and check-ios-shell.sh use that same value in simulator destination `arch`, command-line `ARCHS`, `ONLY_ACTIVE_ARCH=YES`, and empty `EXCLUDED_ARCHS`. Command-line settings apply to the package, application and test targets together. On the native ARM64 GitHub runner this selects arm64; native Intel development selects x86_64. Device build commands receive none of these simulator overrides and retain normal SDK-selected architectures.

New Node CI consistency assertions require native architecture detection, shared helper use by both Apple scripts, matching simulator destination/build settings, unmodified device architecture selection, standard generated-project architectures, and workflow invocation of the checked script. Existing deterministic generation/source assignment checks remain active. WSL shell behavior probes verified arm64 and x86_64 selection and explicit rejection of an unsupported host; an initial probe had a Windows/Bash printf quoting error and was corrected before the successful rerun. Bash syntax passes. These checks are **source/script evidence only**, not Xcode execution or an iOS PASS.

### Local gates and remaining action

Node foundation/CI/shell checks **PASS 13/13**, zero skips/failures; Windows actionlint **PASS** (unavailable shellcheck/pyflakes disabled); git diff --check **PASS**. Windows native script **BLOCKED** on missing vcvarsall, with the existing Zig C/C++ shared-library consumers and UB trap fallback **PASS**. WSL Linux CMake build/CTest **PASS 2/2**; ASan/UBSan **PASS 4/4**, with negative diagnostic exits 86/87 proving active instrumentation. All shell scripts, including the new architecture helper, passed Bash syntax checking. Sandbox network/WSL access failures were retried with approved escalation; they are host restrictions, not source failures.

Changed files: .github/workflows/native-shells.yml; platforms/android/gradle/verification-metadata.xml; scripts/check-apple.sh; scripts/check-ios-shell.sh; scripts/ios-simulator-architecture.sh; scripts/generate-ios-project.mjs; platforms/ios/PixAuraAI.xcodeproj/project.pbxproj; tests/ci-tools.test.mjs; tests/shells.test.mjs; this report. Shared C++/C ABI, JNI, UI/runtime source, dependencies, repository configuration and lockfiles are unchanged.

Remaining mandatory CI-only gates: corrected Ubuntu Android strict resolution/build/lint/unit/API 35 emulator execution, and macOS Apple package/C ABI plus unsigned iOS Debug/Release builds and simulator Swift/UI tests. Owner: repository maintainer; affected G2/G3 and CI promotion. Exact next human action: review this working-tree diff, then commit/push through the maintainer's normal process and run **Foundation boundaries** and **Native application shells** on the corrected revision; retain both run URLs and evidence artifacts and confirm native architecture in Apple logs. Rerunning the existing remote revision cannot validate these local changes. No commit or push is authorized for this agent. Physical assistive-technology walkthrough remains unrun, and optional MSVC repair remains host-maintainer work. Phase 0 CI completion stays user-attested. Do not begin Phase 2.

## Historical initial CI inspection, before actual diagnostics

Inspected clean commit 798c9be, authoritative specifications, testing strategy, both workflows, Android build/test configuration, iOS application/test source, deterministic project generator and Apple scripts. Acceptance for remediation: identify each failure from actual evidence, reproduce where possible, change only the implicated runtime/build/CI behavior, preserve the architecture and Phase 0 gates, and rerun available local gates. No Phase 2 work, dependency changes, commit or push occurred.

The supplied Android and iOS logs both contain only `[paste log]`. The user reports GitHub Actions failures, but no run URL, failed step, diagnostic or evidence artifact was supplied. GitHub CLI is unavailable on this host. Root causes of those reported failures remain **unconfirmed**; no source/build/workflow fix is justified by the available evidence. The earlier report's prepared/unobserved CI description is historical, not a claim that the reported run passed.

Observed local rerun commands and results:

```text
node --test tests/foundation.test.mjs tests/ci-tools.test.mjs tests/shells.test.mjs
build/tools/actionlint/actionlint.exe -shellcheck= -pyflakes= .github/workflows/foundation.yml .github/workflows/native-shells.yml
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check-native.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check-native-zig.ps1 -ZigPath G:/PixAuraAI/build/tools/zig-x86_64-windows-0.14.1/zig.exe
wsl --distribution Ubuntu --exec bash -lc 'cd /mnt/g/PixAuraAI && bash -n scripts/check-apple.sh scripts/check-ios-shell.sh scripts/check-sanitizers.sh && cmake -S . -B build/linux-host -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ && cmake --build build/linux-host && ctest --test-dir build/linux-host --output-on-failure && bash scripts/check-sanitizers.sh'
```

Node **PASS 11/11**; actionlint **PASS**, exit 0 (shellcheck/pyflakes disabled as unavailable locally); Windows Zig shared-library C/C++ consumers and UB trap **PASS**. MSVC **BLOCKED**: installed desktop C++ workload still lacks vcvarsall. The initial plain PowerShell invocation was rejected by execution policy; process-local ExecutionPolicy Bypass allowed the script to diagnose the missing compiler setup. WSL initially returned sandbox E_ACCESSDENIED; an approved escalated rerun **PASS**: Bash syntax, Linux CMake build/CTest **2/2**, ASan/UBSan CTest **4/4**, including actual diagnostic exits 86/87.

With ANDROID_HOME=I:/AndroidStudioSDKdata and GRADLE_USER_HOME=G:/PixAuraAI/build/gradle-home:

```text
platforms/android/gradlew.bat -p platforms/android --no-daemon --offline connectedDebugAndroidTest assembleDebug assembleRelease testDebugUnitTest lintDebug lintRelease assembleDebugAndroidTest
```

The first local run **FAIL** at connectedDebugAndroidTest because no device was running. Started only the repository-isolated pixaura-shell AVD via scripts/prepare-local-avd.mjs and a hidden headless emulator; boot readiness confirmed. Repeating the exact strict offline command **PASS**, exit 0, BUILD SUCCESSFUL in 1m 6s, 147 tasks (15 executed, 132 up-to-date). Instrumentation actually executed **1/1** on Android 16; debug/release three-ABI build tasks and both lint gates succeeded. JVM test and lint analysis/report tasks reused cached outputs: the retained JVM XML records **2/2**, zero skips/failures/errors, dated 2026-10-02; retained debug/release lint reports say no issues. These cached results are not newly executed tests. SDK XML metadata warning remains nonfatal. No locks/checksums were regenerated. This local missing-device issue is resolved and is **not evidence of the GitHub Android root cause**.

Apple package/core, unsigned app builds and simulator Swift/UI tests remain **BLOCKED locally**, not PASS: this Windows host has no Xcode/CoreSimulator. No Apple script execution is claimed. Existing architecture and CI configuration remain intact; only this report changed.

Outstanding blocker: owner repository maintainer; affected Phase 1 G2/G3 and CI promotion; remediation supply the failed Android/iOS step logs or accessible run artifacts, then reproduce and apply evidence-based fixes and rerun the affected CI jobs. Optional MSVC repair remains owned by host maintainer; Windows fallback passed. Phase 0 CI completion remains user-attested. Stop at Phase 1; no push is authorized.

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
