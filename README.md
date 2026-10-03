# PixAuraAI

Local-first mobile photo editor. Phase 1 includes minimal Android Compose and iOS SwiftUI shells with a shared C++17 ABI probe. No photo-engine feature is implemented. Start with [PRODUCT_SPEC.md](PRODUCT_SPEC.md), [ARCHITECTURE.md](ARCHITECTURE.md), [QUALITY_GATES.md](QUALITY_GATES.md) and [ROADMAP.md](ROADMAP.md).

## Foundation checks
Node 22+ with built-in test runner:
```
node --test tests/foundation.test.mjs tests/ci-tools.test.mjs tests/shells.test.mjs tests/document-contract.test.mjs tests/native-document.test.mjs
```
Windows with Visual Studio C++ workload:
```
powershell -NoProfile -File scripts/check-native.ps1
```
The script discovers MSVC using vswhere, builds a shared library and independent C/C++ consumers with warnings as errors, and runs them. It writes only under build/.

If MSVC is incomplete, an optional repository-local compiler fallback is documented in [TOOLCHAIN.md](docs/TOOLCHAIN.md); `scripts/check-native-zig.ps1` builds and runs the same shared-library consumers.

CMake 3.22+ and C++17 compiler:
```
cmake -S . -B build/host -DCMAKE_BUILD_TYPE=Debug
cmake --build build/host --config Debug
ctest --test-dir build/host -C Debug --output-on-failure
```
Clang/GCC sanitizer build: add `-DPIXAURA_SANITIZERS=ON` to configure.

Actual ASan/UBSan validation with positive tests and negative instrumentation probes:
```
bash scripts/check-sanitizers.sh
```
Run on Linux (including Ubuntu WSL on this Windows host) with Clang, its sanitizer runtimes, llvm-symbolizer, CMake and Ninja. CTest passes only when valid calls are clean and the deliberately invalid test-only probes produce the required diagnostics. Logs are under build/sanitize/test-logs.

Apple boundary on macOS with Xcode/Swift 5.9+:
```
swift test --package-path packages/core
```
Complete Apple Phase 0 CI gate on macOS/Xcode:
```
bash scripts/check-apple.sh
```
This executes macOS C/C++ and Swift/C boundary tests, builds the package for iOS without signing, and executes its XCTest boundary probes on an available iPhone simulator. The workflow uses macos-26 with Xcode 26.6, verifies script/workflow references, and preserves test evidence. No application UI is created.
Android NDK boundary (replace placeholder with installed NDK):
```
cmake -S platforms/android/native -B build/android -DCMAKE_TOOLCHAIN_FILE=<NDK>/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-26
cmake --build build/android
```
## Native application shells

Open platforms/android in Cursor/Android Studio, or set ANDROID_HOME to the installed SDK and run:
```
cd platforms/android
./gradlew assembleDebug assembleRelease testDebugUnitTest lintDebug lintRelease assembleDebugAndroidTest
./gradlew connectedDebugAndroidTest
```
On Windows use gradlew.bat. JDK 21.0.10, SDK 36, build tools 35.0.0, NDK 28.2.13676358 and CMake 3.22.1 are the pinned baseline. The connected test requires a booted device/emulator. Dependency locks and SHA-256 verification are enforced by default; do not regenerate them during ordinary verification.

On macOS open platforms/ios/PixAuraAI.xcodeproj in Xcode. The shared PixAuraAI scheme builds a real application and its Swift/unit/UI test targets. The package reference resolves directly to packages/core; no external Apple dependency. Run from repository root:
```
bash scripts/check-apple.sh
bash scripts/check-ios-shell.sh
```
The latter builds Debug/Release unsigned iOS device apps and executes simulator app/C ABI/UI tests. The committed Xcode project is generated with `node scripts/generate-ios-project.mjs`; source tests require deterministic output. Shell checks do not certify photo editing, GPU performance, physical accessibility or signing.

CI workflows: **Foundation boundaries** and **Native application shells**. The user attests final Phase 0/1 CI completion, including both mobile execution gates; historical [Phase 0](docs/reports/phase-0.md) and [Phase 1](docs/reports/phase-1.md) reports preserve earlier evidence snapshots. No unobserved build for a new revision is PASS.

Phase 2 Step 1 freezes the [document contract](docs/contracts/image-document-v1.md) and ADR [0008](docs/adr/0008-document-stacks-and-manifest.md), with test-only synthetic metadata fixtures. Step 2 implements the native metadata model, bounded parser/serializer, immutable revision transitions and separate document C API with caller-owned contexts under ADR [0009](docs/adr/0009-explicit-document-context.md). ABI 1 remains unchanged. See the [Step 1 report](docs/reports/phase-2-step-1.md) and [Step 2 report](docs/reports/phase-2-step-2.md) for acceptance and validation. No decoding, rendering, persistence adapter or editing UI is added. Stop before Step 3; do not commit or push.
