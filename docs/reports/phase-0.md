# Phase 0 foundation report

Updated: 2026-10-02. Initial foundation run: 2026-10-01.

Historical evidence snapshot: this report predates remote publication and subsequent CI. The Phase 1 user instruction states all Phase 0 GitHub Actions boundaries passed and authorizes advancement. Current repository now has an origin remote and commit 5462e36. No authenticated run evidence was available during Phase 1; see [Phase 1 report](phase-1.md) for current status. The original observed evidence below is retained unchanged.

Status: **CONDITIONAL PASS / READY FOR CI**, not FULLY PASSED. All mandatory locally executable Phase 0 gates have passed, including actual AddressSanitizer execution and a negative probe inside the shared core. macOS/iOS execution is the only remaining mandatory platform gap. No CI job has executed; no remote is configured. Phase 1 is explicitly prohibited and has not started.

## Repository inspection
Repository initially contained only an existing initialized .git directory, no commits, no application files and no AGENTS.md. Existing Git state was preserved; no reinitialization, remote publication or commit was needed. No remote is configured, so the new GitHub workflow has not run. Windows host with PowerShell; tools were discovered rather than assumed available on PATH.

## Files created
The initial run created forty reviewable project files (generated outputs/compiler caches are excluded from Git). Validation closure adds seven files, bringing the current reviewable inventory to 47:

Root authoritative documents: AGENTS.md, PRODUCT_SPEC.md, ARCHITECTURE.md, AI_ARCHITECTURE.md, PHOTO_ENGINE.md, DEVICE_CAPABILITY_STRATEGY.md, PRIVACY_SECURITY.md, UX_PRINCIPLES.md, MONETIZATION.md, TESTING_STRATEGY.md, QUALITY_GATES.md, ROADMAP.md, DECISIONS.md.

Root tooling: README.md, CMakeLists.txt, .editorconfig, .gitattributes, .gitignore.

Documents: docs/TOOLCHAIN.md, docs/reports/phase-0.md, docs/adr/0001-native-shells-shared-core.md, docs/adr/0002-local-project-history.md, docs/adr/0003-deterministic-photo-pipeline.md, docs/adr/0004-ai-execution-boundary.md, docs/adr/0005-local-inference-and-tiers.md, docs/adr/0006-build-and-security-policy.md.

Shared boundary: packages/core/include/pixaura/core.h, packages/core/src/core.cpp, packages/core/tests/core_test.cpp, packages/core/tests/c_consumer.c, packages/core/Package.swift, packages/core/swift/Sources/CoreProbe.swift, packages/core/swift/Tests/CoreProbeTests.swift.

Platform/check automation: platforms/android/CoreProbe.kt, platforms/android/native/bridge.cpp, platforms/android/native/CMakeLists.txt, scripts/check-native.ps1, scripts/check-native-zig.ps1, tests/foundation.test.mjs, .github/workflows/foundation.yml.

Added for validation closure: packages/core/tests/asan_negative.cpp, packages/core/tests/ubsan_negative.cpp, scripts/verify-sanitizer.cmake, scripts/check-sanitizers.sh, scripts/check-apple.sh, scripts/select-ios-simulator.mjs, tests/ci-tools.test.mjs. Updated CMakeLists.txt, Swift package/test contracts, workflow, line-ending/generated-file rules and relevant authoritative documents. No shipping architecture change or new application feature.

Re-read AGENTS.md, QUALITY_GATES.md, TESTING_STRATEGY.md, ARCHITECTURE.md, all six accepted Phase 0 ADRs and the original Phase 0 report before closing validation. Acceptance criteria for this closure: positive shared-core tests clean under ASan/UBSan; deliberate invalid probes diagnosed at runtime; verifier rejects a clean binary as instrumentation proof; repeat all executable local gates; runnable Apple host/iOS CI design; validated workflow/script paths; no unobserved gate counted as passed.

## Architecture decisions
Six accepted ADRs choose native Kotlin/Compose and Swift/SwiftUI shells with shared C++17 C ABI; local SQLite and immutable assets/revision graph; CPU reference with Metal/Vulkan adapters and SDR first; local typed AI planning separated from bounded execution and approval; provider-neutral inference with ONNX interchange, CPU fallback and measured tiers; multi-host quality gates and privacy/dependency policy. GPU backends, models, persistence and actual mobile UIs are specified future work, not implemented claims.

No third-party shipping runtime dependency or model weight was added. Optional Zig 0.14.1 compiler was downloaded into ignored build/tools after verifying official SHA-256; it validates host C/C++ behavior only. Source/model licenses and toolchain locks remain explicit future gates. Trial/subscription/lifetime prices and no-credit model are documented without a billing implementation.

## Final Phase 0 gate matrix
| Phase 0 subgate | Result | Evidence classification | Observed evidence / remaining execution |
| --- | --- | --- | --- |
| G0 documents, ADRs, links and acceptance | PASS | locally validated | Seven Node tests pass; all requested documents and six ADRs present; complete gate/report contracts |
| G1 source hygiene, warnings and CI consistency | PASS for implemented scope | locally validated | Strict Windows/Linux/Android compilation; Node hygiene/path checks; Bash syntax; actionlint 1.7.12 exits 0 without workflow diagnostics |
| G2 independent C/C++ ABI behavior | PASS | locally validated | Windows DLL consumers pass; WSL normal CMake/CTest passes 2/2; ASan-instrumented shared-core valid calls clean |
| G3 Windows/Linux core build | PASS | locally validated | Windows Zig C++17 DLL and independent consumers; Linux Clang shared library/consumers |
| G3 Android JNI boundary compile/link | PASS | locally validated | Clean ARM64 and ARMv7 API 26 rebuilds; exported JNI entry point previously inspected |
| G3 macOS C/C++ core and Swift/C ABI tests | PENDING | pending CI execution | apple-boundary job invokes CMake/CTest and Swift Package XCTest with layout/version/invalid-argument checks |
| G3 unsigned iOS package build and simulator Swift/C tests | PENDING | pending CI execution | Same Apple job builds library for generic iOS and runs boundary XCTest on an available iPhone simulator |
| G3 Apple execution on Windows | UNAVAILABLE, not a waiver | legitimately unavailable on the current host | Windows/WSL do not provide Apple SDKs/Xcode/CoreSimulator; macOS job supplies the required execution path |
| G4 ASan/UBSan behavior and active-instrumentation proof | PASS | locally validated | Clang 21.1.8 on local WSL: 4/4 CTest tests pass, including ASan exit 86 and UBSan exit 87 negative diagnostics; clean-binary control rejected |

**CI validated: none.** Every hosted workflow job remains pending CI execution. Portable/sanitizer/Android jobs repeat checks already validated locally; Apple execution is the remaining mandatory evidence. G5-G12 are NOT_APPLICABLE to Phase 0 code (no persistence/render/model/billing/app UI implementation); their future acceptance remains unchanged.

Kotlin application compilation, Android library loading on-device, signing, GPU behavior and full iOS application builds are not claimed. The iOS validation target is only a library/package and XCTest boundary probe. Current lint/type gates use strict native compilers, source hygiene, workflow lint, and shell syntax checks; Swift compilation/XCTest executes in Apple CI.

## Exact AddressSanitizer result
Installed engineering tools in the existing Ubuntu WSL distribution using Ubuntu's official apt repositories: clang, cmake, ninja-build, libclang-rt-21-dev, llvm-21. No production source/runtime/language was replaced. Observed compiler Clang 21.1.8 (6ubuntu1), CMake 4.2.3, Ninja 1.13.2; target x86_64-pc-linux-gnu. Windows host executes these Linux processes locally, not through remote CI.

`bash scripts/check-sanitizers.sh` builds libpixaura_core.so and independent C/C++ consumers with ASan/UBSan and debug symbols, then executes four CTest tests. Valid calls pass without sanitizer errors. The test-only ASan caller allocates 8 bytes but declares a 16-byte output capacity; the write occurs inside the instrumented shared core. Runtime evidence:
```
exit=86
ERROR: AddressSanitizer: heap-buffer-overflow
WRITE of size 16
pixaura_get_core_info .../packages/core/src/core.cpp:14:13
main .../packages/core/tests/asan_negative.cpp:11:25
```
The independent UBSan probe performs volatile signed overflow:
```
exit=87
runtime error: signed integer overflow: 2147483647 + 1 cannot be represented in type 'int'
.../packages/core/tests/ubsan_negative.cpp:7:34
```
Final result: **100% tests passed, 0 tests failed out of 4**. Expected sanitizer termination is verified by a parent CMake script checking both diagnostic and exit code; it is not treated as a clean production operation. Full diagnostics: build/sanitize/test-logs/asan-negative.log and ubsan-negative.log; CTest transcript: build/sanitize/Testing/Temporary/LastTest.log. These generated artifacts remain local/ignored; CI uploads equivalents.

Verifier control: running the verifier against the ordinary Windows core_test.exe produced exit=0 and the ordinary ABI-success message; the verifier correctly rejected that binary as ASan evidence. This confirms success/accepted flags cannot masquerade as active instrumentation. Its separate log is build/sanitizer-verifier-control/asan-negative.log.

The first actual ASan run caught the intended overflow, but the parent verifier expected exit 86 while combined ASan/UBSan common options selected exit 87. Fixed by setting both runtimes' common exit code to the selected probe's expected value; exact diagnostic checks remain mandatory. The corrected complete suite passes. This was a test-harness defect, not a production memory defect.

## macOS/iOS CI validation design
The apple-boundary job in .github/workflows/foundation.yml uses **macos-26** ARM64, explicit `DEVELOPER_DIR=/Applications/Xcode_26.6.app/Contents/Developer`, and a 30-minute timeout. Runner/Xcode availability was verified against the [official runner inventory](https://github.com/actions/runner-images/blob/main/images/macos/macos-26-arm64-Readme.md). Actual job/tool versions are logged; inventory checks are not execution evidence.

`bash scripts/check-apple.sh` performs these required checks in order:
1. Log Xcode/Swift/CMake versions and run Node foundation/CI-tool tests.
2. Configure/build the identical C++ core with Apple Clang and execute independent C/C++ CTest consumers on macOS.
3. Execute the Swift Package tests on macOS with warnings as errors; verify C layout (16 bytes, alignment 4), version/feature values, null/short-buffer rejection and unchanged caller output on errors.
4. Select a real available iPhone from CoreSimulator JSON; unit fixtures verify newest-runtime selection and failure when none is available.
5. Build the PixAuraCore package scheme for generic iOS with `CODE_SIGNING_ALLOWED=NO`.
6. Execute the same Swift/C XCTest boundary tests on the selected iOS simulator, retaining a fresh .xcresult bundle. There is no Phase 1 app target or UI.

The job uploads macOS CTest logs, simulator inventory and XCTest results using a pinned upload-artifact action, even on failure, with 14-day artifact retention. The sanitizer job runs the same locally verified shell script and uploads diagnostics/CTest logs. Hosted Linux/Windows portable and ARM64/ARMv7 Android jobs remain present. A workflow-lint job downloads checksum-pinned actionlint 1.7.12, then checks YAML/actions/expressions and hosted shellcheck integration. All action references are pinned by commit SHA. Scripts are LF-controlled and package-generated .swiftpm state is excluded.

Local consistency evidence: actionlint 1.7.12 successfully parses the current workflow; Bash `-n` accepts both check scripts; Node tests resolve referenced script/test files and match Apple scheme names to the Swift library product. These are static checks only; Apple compilation/runtime results remain PENDING until the macOS job executes.

## Initial-run commands and context
Repository/tool inspection: `Get-Location`, `Get-ChildItem`, `rg --files`, `Get-Command`, `git --version`, `git status --short --branch`, `git remote -v`, `git ls-files --others --exclude-standard`, `git diff --check`, `node --version`, `dotnet --version`, `java -version`, and vswhere discovery. Initial Flutter version probing hung and was cancelled; Python PATH points to an unusable WindowsApps alias. Neither is required by the chosen stack.

Foundation checks:
```
node --test tests/foundation.test.mjs
powershell -NoProfile -File scripts/check-native.ps1
powershell -NoProfile -File scripts/check-native-zig.ps1 -ZigPath G:/PixAuraAI/build/tools/zig-x86_64-windows-0.14.1/zig.exe
```
MSVC check initially failed: installation has compiler executables but lacks vcvarsall and headers. The check script now detects this prerequisite correctly. The Zig fallback compiles shared DLL plus C/C++ consumers with strict warnings, executes both, and executes a separate undefined-behavior trap build. All fallback checks pass. Compiler download required a network escalation after sandbox connection failure; archive digest matched the official index. Slow Expand-Archive extraction was cancelled and completed with `tar -xf build/tools/zig-0.14.1.zip -C build/tools`.

Android ARM64 (using discovered absolute tool paths):
```
I:/AndroidStudioSDKdata/cmake/3.22.1/bin/cmake.exe -S platforms/android/native -B build/android -G Ninja -DCMAKE_MAKE_PROGRAM=I:/AndroidStudioSDKdata/cmake/3.22.1/bin/ninja.exe -DCMAKE_TOOLCHAIN_FILE=I:/AndroidStudioSDKdata/ndk/28.2.13676358/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-26
I:/AndroidStudioSDKdata/cmake/3.22.1/bin/cmake.exe --build build/android
```
Repeated with `-B build/android-armv7 -DANDROID_ABI=armeabi-v7a`, then built that directory. Initial configure could not find Ninja; explicit SDK Ninja path fixed it. Both builds complete four compile/link steps successfully. `llvm-readelf --dyn-syms build/android/libpixaura_bridge.so` confirms `Java_ai_pixaura_bridge_CoreProbe_nativeAbiVersion` and the shared core reference.

Sanitizer investigation: direct Zig builds with `-fsanitize=undefined` and `-fsanitize=address,undefined` executed successfully, but inspecting the lowered compiler invocation showed only undefined-behavior trap instrumentation. These runs are not credited as ASan. A PowerShell comma-argument parse error was corrected by quoting the combined flag. CI has a real Linux Clang ASan/UBSan job. A diagnostic `adb devices` attempt failed before enumeration due to its default configuration-directory permissions; device execution was not part of the Phase 0 compile criterion and is not claimed.

## Validation-closure commands and results
Executed on current Phase 0 sources:
```
node --test tests/foundation.test.mjs tests/ci-tools.test.mjs
powershell -NoProfile -File scripts/check-native-zig.ps1 -ZigPath G:/PixAuraAI/build/tools/zig-x86_64-windows-0.14.1/zig.exe
build/tools/actionlint/actionlint.exe -shellcheck= -pyflakes= .github/workflows/foundation.yml
git diff --check
git remote -v
```
Results: 7/7 Node tests, Windows C/C++ and undefined-behavior trap tests pass; actionlint exits 0; whitespace checks pass; remote output is empty. Windows actionlint was downloaded from the official release, checksum-verified before execution; shellcheck/pyflakes integrations are not installed locally, so Bash syntax is checked independently. Corrected the initial download filename from tar.gz to the official Windows ZIP; retained license/tool provenance under build/tools.

WSL tool installation (approved elevated execution needed to access the existing WSL service/install tool packages):
```
wsl --distribution Ubuntu --user root --exec sh -lc 'apt-get update && apt-get install -y --no-install-recommends clang cmake ninja-build'
wsl --distribution Ubuntu --user root --exec sh -lc 'apt-get install -y --no-install-recommends libclang-rt-21-dev llvm-21 && cd /mnt/g/PixAuraAI && bash scripts/check-sanitizers.sh'
```
After diagnosing/fixing the initial verifier failure, executed:
```
wsl --distribution Ubuntu --exec bash -lc 'cd /mnt/g/PixAuraAI && bash -n scripts/check-apple.sh scripts/check-sanitizers.sh && cmake -S . -B build/linux-host -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ && cmake --build build/linux-host && ctest --test-dir build/linux-host --output-on-failure && bash scripts/check-sanitizers.sh'
```
Results: shell syntax PASS; strict ordinary Linux host build PASS, 2/2 tests; ASan/UBSan PASS, 4/4 tests with active-instrumentation proof. The verifier control additionally invokes scripts/verify-sanitizer.cmake with the clean Windows core_test.exe and verifies rejection rather than a false pass.

Repeated both Android configure commands from the initial-run section and ran `cmake --build build/android --clean-first` and `cmake --build build/android-armv7 --clean-first` using the SDK CMake executable. Both native/JNI builds pass with warnings as errors.

Retried `powershell -NoProfile -File scripts/check-native.ps1`: it still correctly reports the incomplete optional Visual Studio installation (vcvarsall missing). This is not an unmet mandatory host gate: the identical Windows shared core/independent consumers pass via the existing Zig fallback, and Linux Clang supplies actual sanitizer evidence. Repairing MSVC is optional environment maintenance and is not required to close Phase 0.

## Remaining external requirement and human action
**Only mandatory gap: observed successful execution of the Apple boundary job.** No remote repository is configured and no accessible macOS runner is attached. Nothing in the current Windows/WSL host can execute Xcode/CoreSimulator.

The repository owner must commit the current 47 reviewable project files, publish/push them to an authenticated GitHub repository with Actions enabled, and run **Foundation boundaries** (push triggers it, or use Actions -> Foundation boundaries -> Run workflow). The account must have access to the macos-26 hosted runner. No Apple signing key or store credential is needed. Supply the workflow run URL/results; inspect the apple-boundary job and retained XCTest/CTest artifacts. A successful complete workflow provides CI validated evidence; failures must be diagnosed before promotion. A supplied compatible macOS runner executing `bash scripts/check-apple.sh` is an equivalent alternative, with retained logs/results.

## Next milestone
Stop at READY FOR CI / CONDITIONAL PASS. Obtain successful Apple execution evidence before classifying Phase 0 as FULLY PASSED. Do not start Phase 1; the user's explicit prohibition remains in effect even after the validation job runs, until they authorize advancement.
