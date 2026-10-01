# Foundation toolchain and provenance

Shipping mobile toolchain locks are a Phase 1 deliverable. Phase 0 runtime/core has no third-party library dependencies.

Observed on this Windows host: Node 24.14.0, Git 2.55.0.windows.2, Java 21.0.10, Android CMake 3.22.1 and NDK 28.2.13676358 (Clang 19.0.1). SDK tools are installed under I:/AndroidStudioSDKdata and are not on PATH. Android baseline compile target API 26. Existing Visual Studio 18.7 installation is incomplete (missing vcvarsall and C++ headers); host MSVC checks require workload repair.

Optional local test fallback: Zig 0.14.1 used solely as a bundled Clang/MinGW C/C++ host compiler, not an application language or shipping dependency. Download official Windows x86_64 archive, verify before extracting/executing:

```
URL: https://ziglang.org/download/0.14.1/zig-x86_64-windows-0.14.1.zip
SHA-256: 554f5378228923ffd558eac35e21af020c73789d87afeabf4bfd16f2e6feed2c
```

Digest provenance: [official download index](https://ziglang.org/download/index.json). Zig is MIT licensed; bundled LLVM/Clang/libc components have their own notices retained in the extracted tool distribution. This fallback is not used to ship binaries or establish the release MSVC toolchain. Download/extract/cache under ignored build/tools; do not commit the archive or compiler. Run `powershell -NoProfile -File scripts/check-native-zig.ps1 -ZigPath <absolute-zig.exe>` after integrity verification. Compiler caches remain under build/zig-host. Official toolchain validation and sanitizers still run separately in CI.

GitHub workflow uses checkout v4 pinned by commit SHA and fixed runner family labels. Hosted images/tool versions are mutable, so logs must retain versions. Android NDK is pinned to the version verified locally. Phase 1 must add exact Gradle/JDK/Kotlin/Compose/Xcode inputs, lockfiles and dependency verification before app releases.

The Zig fallback runs undefined-behavior checks in trap mode. Do not treat an accepted `-fsanitize=address` option as ASan evidence: inspection of this compiler's lowered invocation on Windows showed undefined-behavior instrumentation but no address instrumentation. Actual Linux Clang ASan/UBSan execution supplies mandatory independent evidence, locally through WSL or in CI.

## Validation-closure tools
Actual local sanitizer validation now executes on the same Windows machine through its installed Ubuntu WSL distribution: Clang 21.1.8 (6ubuntu1), CMake 4.2.3, Ninja 1.13.2 and LLVM 21 symbolizer/compiler-rt. Installed from Ubuntu's official apt repositories: clang, cmake, ninja-build, libclang-rt-21-dev and llvm-21. These are engineering tools, not shipping dependencies. Shared-core tests pass with ASan/UBSan; both negative probes prove runtime instrumentation. Full details/log locations are in the Phase 0 report.

Workflow syntax validator: actionlint 1.7.12 (MIT) from its [official release](https://github.com/rhysd/actionlint/releases/tag/v1.7.12), retained under ignored build/tools with license. Windows x86_64 ZIP SHA-256 is `6e7241b51e6817ea6a047693d8e6fed13b31819c9a0dd6c5a726e1592d22f6e9`; CI Linux x86_64 tarball SHA-256 is `8aca8db96f1b94770f1b0d72b6dddcb1ebb8123cb3712530b08cc387b349a3d8`. Both digests are from official GitHub release assets. Windows actionlint validates YAML/actions/expressions with unavailable shellcheck/pyflakes integrations disabled; Bash syntax is independently checked through WSL. CI uses actionlint with the hosted shellcheck integration.

Apple CI explicitly uses macos-26 and `/Applications/Xcode_26.6.app/Contents/Developer`. These inputs were checked against the [official ARM64 runner inventory](https://github.com/actions/runner-images/blob/main/images/macos/macos-26-arm64-Readme.md). The script logs actual Xcode/Swift/CMake versions and selects an available iPhone from installed iOS runtimes instead of assuming a fixed simulator name. CI action artifacts retain XCTest/CTest evidence for 14 days; workflow console logs retain build/toolchain output. Neither the runner inventory nor local syntax checks qualify as Apple execution evidence.
