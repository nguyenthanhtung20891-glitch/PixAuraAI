# PixAuraAI

Local-first mobile photo editor. Phase 0 foundation; no mobile application or editing feature is implemented yet. Start with [PRODUCT_SPEC.md](PRODUCT_SPEC.md), [ARCHITECTURE.md](ARCHITECTURE.md), [QUALITY_GATES.md](QUALITY_GATES.md) and [ROADMAP.md](ROADMAP.md).

## Foundation checks
Node 22+ with built-in test runner:
```
node --test tests/foundation.test.mjs tests/ci-tools.test.mjs
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
No Gradle/Xcode app exists until Phase 1. The native Android library is only a JNI version probe; the Swift module is only an ABI boundary probe. Phase 0 is CONDITIONAL PASS / READY FOR CI, not FULLY PASSED. CI config is not proof of execution. See [Phase 0 report](docs/reports/phase-0.md) for observed checks and the required human action to run Apple validation. Phase 1 remains prohibited.
