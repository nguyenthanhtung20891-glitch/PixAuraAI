# Phase 10.1 Step 3 — Parallel Android and iOS workflow

Status: LOCAL ANDROID OBSERVED / PHASE NOT CLOSED.

The ownership rules are in `docs/plans/phase-10.1-roadmap.md`. Windows and
Mac Studio keep separate clones. One owner integrates to `main`. Shared C++
has one owner. Android presentation is Windows work. Apple presentation,
Swift/C ABI, Xcode, and Simulator work belong on the Mac clone.

The common scenario list is open/import, manual edit, Assist navigation,
Auto proposal, plan inspection, preview, edit review, explicit HUMAN Apply,
ordinary history, undo/redo, JPEG/PNG export, save/share, English and
Vietnamese presentation, cancellation/failure, and lifecycle or stale-state
rejection. Existing source gates already lock the shared sentences. A
side-by-side run on the Windows clone and the Mac clone has not happened.
Hosted CI success is not that local parity result.

## Mac Studio Android observation

The Product Owner authorized the existing Android workflow on this Mac Studio
clone. The ownership note above stays the recorded policy. This section is
local Android evidence. Phase 10.1 is not closed. Phase 11 is not started.
Phase 12 is not started.

One toolchain installation:

- Eclipse Temurin 21.0.10+7 at
  `~/Library/Java/jdk-21.0.10+7/Contents/Home`
- Android SDK at `~/Library/Android/sdk`
- Command-line tools 21.0 (package `commandlinetools-mac-15641748`),
  SHA-1 `b62a5d8cf63ded173b47be867be4ee058ceda6df`
- Platform 36, build-tools 35.0.0, NDK 28.2.13676358, SDK CMake 3.22.1
- Emulator 37.2.13
- `system-images;android-35;google_apis;arm64-v8a` revision 9
- `system-images;android-35;google_apis;x86_64` revision 9
- Gradle 8.13, AGP 8.13.2, Kotlin 2.2.21

`~/.zshenv` exports `JAVA_HOME`, `ANDROID_HOME`, `ANDROID_SDK_ROOT`, and the
SDK tool directories. No second JDK or SDK was installed.

Strict Gradle verification rejected the official macOS `aapt2` 8.13.2-14304508
artifact because the metadata pinned only the Linux and Windows classifiers.
The macOS jar was downloaded from Google Maven. Its SHA-256 is
`0d47f17c3924e5472b6125aa608d949dd7f46510889729671f31f2f4d801e8e7`, matching
the published `.sha256`. The binary is a universal Mach-O. Linux and Windows
checksums are unchanged. Shared C++, Kotlin, and Swift sources are unchanged.

Local results on this host:

- `assembleDebug`, `assembleRelease`, `testDebugUnitTest` 7/7,
  `testReleaseUnitTest` 7/7, `lintDebug`, `lintRelease`, and
  `assembleDebugAndroidTest` succeeded with `--dependency-verification strict`.
- `tests/android-release-scanner.test.py` passed. Release isolation passed:
  12 native libraries, 2 dex files.
- NDK CMake `android-26` produced `libpixaura_core.so` and
  `libpixaura_bridge.so` for `arm64-v8a` (ELF AArch64) and `armeabi-v7a`
  (ELF 32-bit ARM EABI5).
- Debug and unsigned release APKs package those libraries for `arm64-v8a`,
  `armeabi-v7a`, and `x86_64`.
- ARM64 emulator: Pixel 6 profile, model `sdk_gphone64_arm64`, device
  `emu64a`, API 35, ABI `arm64-v8a`, SELinux Enforcing, Hypervisor.Framework.
  `connectedDebugAndroidTest` finished 58 tests, 0 failures, 0 skipped.
- The same emulator refuses the pinned x86_64 image:
  `Avd's CPU Architecture 'x86_64' is not supported by the QEMU2 emulator on aarch64 host.`
  That image remains installed for the hosted workflow. An ARM64 guest does
  not certify `armeabi-v7a`. The 32-bit evidence here is the NDK build and
  the packaged ELF, plus hosted CI on the predecessor SHA.

`cpu.reference.v1` stays VALIDATED. `android.vulkan.v1` stays UNKNOWN.
`apple.metal.v1` stays UNKNOWN. DH-APPLE-METAL-01 stays DEFERRED / NOT WAIVED
/ NOT SATISFIED. No physical device was used.

Apple and shared sources are unchanged, so the accepted local Apple results
on `dcef1ac59e718ac17425c1b6086dd6f9840ac311` remain the Apple evidence for
those sources. This metadata commit still needs Foundation and Native shells
on its own SHA. Hosted CI for that predecessor stays distinct from this local
Android run.

`node --test tests/*.test.mjs` on this host passed 224 tests. The 19 failures
are all in `tests/android-emulator.test.mjs`. macOS `realpath` rejects GNU
`-m`, which `scripts/check-android-emulator.sh` uses. That script remains the
Linux CI orchestrator and was not changed. Hosted Ubuntu CI remains its
execution evidence.
