# Phase 2 Step 2: native document foundation

Status: IMPLEMENTED, CI REMEDIATION VALIDATED LOCALLY; corrected-tree hosted CI remains pending. 2026-10-03. This is not FULLY PASSED or full Phase 2 completion. The initial prepublication evidence below is historical; section 13 records the published CI failures and their narrow remediation.

The user approved Step 1, authorized Step 2, and approved caller-owned contexts after the initial attempt stopped on the process-global registry conflict. The contract, ADR 0009, ADR 0008 supersession annotation and decision index were updated before implementation. All twelve frozen document decisions remain intact. No commit or push occurred. HEAD remains `2525b52207ef1f6a3af945032d8190366b177f27`. Prior Phase 0/1 final CI completion is user-attested; no new CI run is claimed.

## 1. Files created and changed

New Step 2 files:

- `docs/adr/0009-explicit-document-context.md`
- `packages/core/include/pixaura/document.h`
- `packages/core/src/document.hpp`
- `packages/core/src/document.cpp`
- `packages/core/src/document_api.cpp`
- `packages/core/tests/document_test.cpp`
- `packages/core/tests/document_c_consumer.c`
- `packages/core/tests/document_allocation_test.cpp`
- `packages/core/swift/Tests/DocumentBoundaryTests.swift`
- `packages/core/swift/Tests/Fixtures/image-document-v1.json`
- `platforms/android/app/src/androidTest/java/ai/pixaura/app/DocumentBoundaryTest.kt`
- `tests/native-document.test.mjs`

Updated by Step 2:

- This report, `docs/contracts/image-document-v1.md`, ADR 0008 annotation and `DECISIONS.md`.
- `AGENTS.md`, `ARCHITECTURE.md`, `QUALITY_GATES.md`, `ROADMAP.md`, `README.md`, `TESTING_STRATEGY.md`: authorized scope, delivery, evidence and remaining gates.
- `CMakeLists.txt`, `packages/core/Package.swift`, `scripts/check-native.ps1`, `scripts/check-native-zig.ps1`, `scripts/check-apple.sh`: identical shipping sources and independent native/platform checks.
- `.github/workflows/foundation.yml`, `.github/workflows/native-shells.yml`: new source checks, retained execution gates and canonical native artifacts.
- `platforms/android/app/build.gradle.kts`, `platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/CoreProbe.kt`, `platforms/android/native/bridge.cpp`: metadata-only test asset/JNI adapter, no photo algorithm.
- `.gitignore`, `tests/foundation.test.mjs`: exclude generated `.kotlin` compiler sessions from source scanning/version control.

Preexisting uncommitted Step 1 changes were preserved. The approved fixture/oracle, core.h/core.cpp, dependency locks, verification metadata and shipping toolchain pins are unchanged. No runtime or fuzzing dependency was added. Generated binaries, logs and the local Android harness remain under ignored build directories.

## 2. Native types

Private C++ code implements validated `HexIdentity<32>` IDs, `HexIdentity<64>` digests, `DocumentIdentity`, `SourceMetadata`, `SourceDescriptor`, `EditOperation`, typed `Exposure`/`Crop`/`Rotate` parameters, `Revision`, `SessionIdentity`, `DetachedCandidate`, `ImageDocument`, immutable shared `Snapshot` ownership and `Result<T>` failures. The document stores full ordered stacks, current revision, redo and all retained single-parent branches. Session/generation is not serialized. ImageDocument construction is private; factories/transitions return validated immutable snapshots.

These are metadata types. No evaluator, pixel buffer, codec, GPU, inference, editing UI, database or import service exists. Metadata loading/creation does not issue a VerifiedSource capability; the later import adapter must verify copied original bytes/hash/metadata.

## 3. Parser and serializer

Native parsing enforces 8 MiB manifests, 64 KiB commands, depth 16, decoded strings 256 UTF-8 bytes, operations/revisions 4096, stack 256 and redo 4095. Schema-directed field/array admission precedes untrusted container growth. Recursion is explicitly bounded. Integer accumulation is checked before multiplication/addition; lengths are checked before size_t conversion. Manifest numeric values never become unchecked allocation sizes.

Parsing rejects duplicate/escape-equivalent keys, invalid UTF-8/BOM/control characters/surrogates, truncation/trailing input, wrong fields/types/containers, fractional/exponent/leading-zero/negative-zero tokens and out-of-range integers. Unknown schema/operation/version and invalid parameters fail explicitly. Validation checks identities, root/actor/plan rules, earlier parents, duplicate stacks, dangling references, all-operation reachability, current revision and contiguous redo. No coercion, implicit migration, partial opening or pruning occurs.

Serialization revalidates the immutable document, sorts ASCII keys, preserves arrays, emits shortest decimal integers/standard escapes and exactly one LF. It checks output size and cannot mutate state. Native and C ABI fixture round trips reproduce the approved bytes.

## 4. Revision transitions

Implemented root-only initial metadata creation, append/batch proposals, parameter replacement with fresh IDs/full stacks, explicit reorder/removal, undo, redo, checkout and branches after undo. Previous records remain unchanged; a new branch clears redo navigation and retains old branches. Replay reads only the chosen revision's complete stack, preserving order without accumulating ancestors.

Expected session, generation and base revision are checked; detached candidates also check project/document identity. Stale candidates, reopened sessions and revision ABA reject without input mutation. Successful transitions increment nonserialized generation, with exhaustion guards. Invalid commands, versions, operation freshness/reachability and resource limits reject.

The C API returns detached next snapshots. It cannot establish whether a retained input handle is the application's current active snapshot. Application services must order transitions, compare against the active session, obtain user approval, persist atomically in SQLite and publish only after success. Actor strings grant no approval authority. No catalog is written here.

## 5. C ABI additions

Separate document API family version 1 exports context init/destroy and document open/create/apply/serialize/release. Every handle operation receives the owning context. C declarations use fixed-width values and C-only structs; no C++ type or document address crosses the ABI. Existing ABI 1's probe, layout, statuses, version rejection, reserved field and zero feature bits are unchanged.

Layouts: context 64 bytes; handle 56 bytes/serial offset 48; error 184 bytes/alignment 4/message offset 24. Structs have version/size and zero reserved fields. Error layout is validated before output writes. Diagnostics are bounded ASCII identifiers with optional record index, never source contents. Exceptions including bad_alloc are contained and translated at C entry points.

## 6. Ownership rules

Caller owns accessible, noncopied context storage, inputs, output buffers and error/result structs. The library owns registry allocations/snapshots; no cross-module free is needed. The caller supplies a fresh context identity unique across lifetimes/reinitialization and a fresh session identity per open. This uniqueness is an explicit application identity-service obligation. Truthful accessible memory capacities remain a C caller obligation; arbitrary dangling pointers cannot be validated.

Tokens contain context identity and a nonzero monotonic serial, never an address. Serials cannot reuse/wrap. Cross-context, zero/forged/released tokens, destroyed contexts and old tokens after storage reuse with a fresh identity reject. Destroy invalidates all owned handles; repeated destroy rejects while caller storage stays valid. A context admits 64 simultaneously owned snapshots; exceeding that runtime budget rejects without eviction/history pruning. Release permits new allocations without serial reuse.

Init/destroy require exclusive access with no in-flight calls. Live registry access is mutex-protected; acquired shared snapshots survive concurrent release. A deterministic test pauses serialization after acquisition, releases its handle on another thread, then proves successful completion. Test synchronization is bounded to five seconds. Serialized caller bytes remain independent after input overwrite, transitions, release and destruction.

Two-call serialization includes LF and no NUL. Undersized fill writes only required length and optional error, leaving bytes untouched. Other failures preserve handles, required length and byte buffers; invalid error layouts preserve all outputs.

## 7. Adversarial and allocation coverage

Structured fuzz-style tests cover every incomplete fixture prefix, deterministic byte mutations with successful-input canonical reload, malformed nesting and 100,000-level input, oversized arrays/strings/bytes, exact-byte admission, unknown fields/schema/type/versions, NaN/Infinity/integer rejection, invalid UTF-8/surrogates, duplicate IDs/keys, dangling/future parents, invalid root/current/redo, repeated stacks and unreferenced operations. Valid fixtures prove admission at exactly 4096 revisions, 4096 operations and 256 operations per stack.

History tests cover initial creation, append/replacement, root NO_HISTORY, undo/redo/reload, checkout/retained branches, candidate/session/generation staleness and ABA, source descriptor immutability and AI actor/plan association through the same stack. No AI execution occurs.

C/C++ ownership tests cover isolation, handle versions/sizes/reserved/serials, fresh allocation/storage reuse, null pointers, length/API/error-layout rejection, destroyed/double-destroyed resources, buffer sentinels and live-handle admission. Allocation injection is confined to an independent test executable compiling the production sources; no failure hook/allocator ships. It sweeps each successive allocation until open/serialize/apply succeeds and checks prior snapshots and complete caller buffers after failures. Failed context initialization is also tested. Windows observed 142/190/229 injected failures before success; Linux/ASan 192/227/278. Library allocator patterns differ, canonical values do not.

Full-range serial/generation exhaustion is guarded in code but impractical to reach by repeated-transition execution; no such test execution is claimed. This is deterministic structured adversarial coverage, not long-running coverage-guided fuzzing.

## 8. Exact validation results

| Check | Observed result |
| --- | --- |
| Windows foundation/CI/shell/Step 1/new source checks | PASS 29/29, zero failures/skips |
| Linux same plus emulator orchestration | PASS 58/58, zero failures/skips |
| Linux warning-clean shared build/CTest | PASS 5/5, existing C/C++ ABI and new document/ownership/allocation consumers |
| Actual Linux ASan/UBSan | PASS 7/7, including negative runtime instrumentation probes with expected exits 86/87 |
| Windows standard MSVC script | BLOCKED; executed prerequisite check reports missing vcvarsall |
| Windows Zig fallback | PASS shared DLL, existing C/C++ ABI, new C/C++ document tests, allocation/pinning test and original undefined-behavior trap probe |
| Windows/Linux/sanitizer canonical artifacts | PASS exact fixture equality; SHA-256 a18deede5475c2642501a8911212f76bd63f8ed8738dd3f7d5f3fbfff72216e7 |
| Android final full regression | PASS exit 0, BUILD SUCCESSFUL in 1m 47s, 147/147 actionable tasks executed |
| Android native Debug/Release | PASS arm64-v8a, armeabi-v7a and x86_64, including final layouts/32-bit compile |
| Android JVM/emulator execution | PASS 2 JVM and 2 instrumentation tests; zero errors/failures/skips |
| Android full Debug/Release lint | PASS: both reports say No issues found |
| Bash syntax/actionlint 1.7.12 | PASS; unavailable local shellcheck/pyflakes disabled, Bash syntax separately executed |
| Source/whitespace | PASS Node hygiene covers untracked source too; git diff --check exits 0 with only LF/CRLF conversion warnings |
| Apple native/Swift/iOS package/app/UI execution | BLOCKED locally: no macOS/Xcode; new tests/resource and both CI paths wired, no execution PASS claimed |
| SQLite durability/source-byte verification/pixel parity | NOT_APPLICABLE to Step 2 scope; mandatory later implementation gates |

Windows tools: Node 24.14.0/Zig 0.14.1. WSL: Ubuntu Clang 21.1.8/CMake 4.2.3/Node 24.14.0. Mobile inputs retain pinned JDK 21.0.10, Gradle 8.13, AGP 8.13.2, Kotlin 2.2.21, NDK 28.2.13676358 and CMake 3.22.1. Local installed emulator 36.5.11.0 uses API 36.1 x86_64/WHPX/software GPU; it is not the hosted API 35/KVM image. Each owned emulator was stopped. SDK XML-version warnings remain nonfatal; verification/lint were not relaxed.

Commands executed, including the final rerun after deterministic pinning coverage:

```text
node --test tests/foundation.test.mjs tests/ci-tools.test.mjs tests/shells.test.mjs tests/document-contract.test.mjs tests/native-document.test.mjs
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check-native.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check-native-zig.ps1 -ZigPath G:/PixAuraAI/build/tools/zig-x86_64-windows-0.14.1/zig.exe
wsl --distribution Ubuntu --exec bash -lc 'cd /mnt/g/PixAuraAI && bash -n scripts/*.sh && build/tools/node-v24.14.0-linux-x64/bin/node --test tests/foundation.test.mjs tests/ci-tools.test.mjs tests/shells.test.mjs tests/android-emulator.test.mjs tests/document-contract.test.mjs tests/native-document.test.mjs && cmake -S . -B build/linux-host -DCMAKE_BUILD_TYPE=Debug && cmake --build build/linux-host -j2 && ctest --test-dir build/linux-host --output-on-failure && bash scripts/check-sanitizers.sh'
wsl --distribution Ubuntu --exec bash -lc 'cd /mnt/g/PixAuraAI && cmake --build build/linux-host -j2 && ctest --test-dir build/linux-host --output-on-failure && bash scripts/check-sanitizers.sh'
powershell -NoProfile -ExecutionPolicy Bypass -File build/phase-2-step-2/check-android.ps1
build/tools/actionlint/actionlint.exe -shellcheck= -pyflakes= .github/workflows/foundation.yml .github/workflows/native-shells.yml
git diff --check
```

The ignored Android harness uses private repository-local AVD/user/Gradle directories, bounded readiness, strict offline verification and fresh tasks, then stops only its owned emulator. Exact Gradle invocation:

```text
platforms/android/gradlew.bat -p platforms/android --no-daemon --offline --dependency-verification strict --no-build-cache --rerun-tasks --max-workers=1 "-Dorg.gradle.jvmargs=-Xmx768m -Xms128m -XX:+UseSerialGC -Dfile.encoding=UTF-8" "-Pkotlin.compiler.execution.strategy=in-process" assembleDebug assembleRelease testDebugUnitTest lintDebug lintRelease assembleDebugAndroidTest connectedDebugAndroidTest
```

Evidence: `build/linux-host/Testing`, `build/sanitize/Testing` and `test-logs`, each host's `document-canonical.json`, `build/phase-2-step-2` emulator/ADB logs and Android app build reports/test-results/outputs. Final XML timestamps: JVM 2026-10-03T14:54:59.226Z; instrumentation 2026-10-03T14:55:28. The latter executed canonicalNativeDocumentFixture and realJniBoundaryAndShellNavigation. CI retains canonical/CTest/sanitizer/mobile artifacts. Source tests verify the Swift bundled fixture matches the approved fixture byte-for-byte.

Unsuccessful initial attempts were diagnosed, fixed and not counted PASS: WSL sandbox service denial required escalation; Android Start-Process environment failure disappeared with approved service access; std::data collided with a C++ test helper, which was renamed; concurrent source scanning encountered generated Kotlin session files, now excluded. Apple/MSVC blockers were not relabeled as passed.

## 9. Contract clarifications

Accepted [ADR 0009](../adr/0009-explicit-document-context.md) and the [contract](../contracts/image-document-v1.md) replace only the context-free registry proposal and leave ABI 1 feature bits zero as the user requested. Layouts, caller identity/storage obligations, synchronized calls/exclusive teardown, the 64-live-handle runtime budget and nonserialized session envelope make the approved lifecycle concrete. No schema migration or history redesign occurred. ADR 0008 retains its accepted historical decision with an explicit supersession annotation.

## 10. Blockers and limits

- Apple execution: owner repository maintainer/Apple runner; remediation run Foundation boundaries and Native application shells on the reviewed published tree and retain CTest/Swift/XCTest/UI artifacts. Affects G3 and Apple G2/parity.
- Local MSVC: owner host maintainer; remediation repair Visual Studio desktop C++ workload and run the extended script. Affects local MSVC G1/G3; fallback execution is independent evidence.
- Current-tree hosted CI: unobserved because commit/push are forbidden; owner repository maintainer, remediation publish only after separate authorization and run both workflows.

No architecture conflict remains. Tests establish metadata/history/ownership behavior, not copied-source integrity, crash durability, decoder support, pixel determinism or device performance. Those remain later gates. Source descriptors hold no writable file capability.

## 11. Proposed Phase 2 Step 3

After Step 2 review and mandatory CI evidence, propose a separately scoped persistence/managed-asset foundation: platform SQLite authority, verified immutable private sources, atomic operation/revision/current/redo publication with expected epoch, interruption recovery and explicit versioned migration/backup fixtures. Specify durability and ownership first. Decoding/rendering remain separately authorized work. Step 3 was not started.

## 12. Exact human decisions next

Review Step 2, including explicit caller context identity/lifetime obligations. No new product/platform compromise is requested. Separately authorize commit/push if publication for CI is wanted; neither occurred. Obtain mandatory Apple/current-tree workflow evidence before declaring all gates passed. Then explicitly approve a Step 3 scope if desired. No automatic advancement, decoding, rendering, AI or editing UI work occurred.

## 13. Published CI evidence and minimal remediation

The user subsequently authorized publication of the reviewed Step 1/2 tree. Commit `744c8c5d711c1dba14514ae4f9078cc420694726` was pushed to main with message `feat: implement PixAuraAI phase 2 native document foundation`. On this follow-up, the GitHub Actions jobs API independently confirmed [Foundation boundaries run 37134260674](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37134260674) and [Native application shells run 37134260640](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37134260640) for that exact SHA. Shell source/android-app/ios-app succeeded. Foundation workflow-lint, portable Ubuntu, apple-boundary and both Android ABIs succeeded; portable Windows and sanitizer failed. Apple success is hosted CI evidence for the published SHA, never local macOS execution or evidence for these unpublished corrections.

Exact failing logs were retrieved before changing source, using the existing Git credential without printing it. Ignored evidence lives under `build/phase-2-ci-fix/`: Windows job `111235434462.log`, sanitizer job `111235434454.log`, and both workflows' job metadata. No investigation data or credentials enter version control.

### Windows: all four observed failures

The Windows Foundation source command executed 23 tests: 19 passed, four failed, all in `tests/document-contract.test.mjs`. The three equality failures report `ERR_ASSERTION`, `AssertionError`, operator `strictEqual`, message `Expected values to be strictly equal:`. The complete JSON payload is identical; actual ends `}}\n`, expected ends `}}\r\n`.

| Exact test name | Exact failing assertion/error | Classification and minimal remediation |
| --- | --- | --- |
| contract golden bytes are deterministic and survive key-order normalization/reload | Line 17: `assert.equal(serialize(fixture()), golden)`; strictEqual LF actual versus CRLF expected | Windows Git checkout line endings, no document contract divergence. Pin the approved golden JSON fixture to LF in `.gitattributes`. |
| replay preserves explicit ordering and full stacks do not duplicate ancestor operations | Line 30: `assert.equal(serialize(state.document), golden)`; same strictEqual LF/CRLF mismatch | Same checkout issue and same LF attribute fix. No history/replay implementation change. |
| invalid and stale commands leave inputs intact; generation rejects revision ABA | Line 64: `assert.equal(serialize(initial.document), golden)`; same strictEqual LF/CRLF mismatch | Same checkout issue and same LF attribute fix. No stale-generation implementation change. |
| bounded token walk rejects corruption, duplicates, invalid UTF-8 and noninteger JSON | Line 130: `assert.throws(() => parseManifest(text), /INVALID_PROJECT/)`; `Missing expected exception.`, `ERR_ASSERTION`, operator `throws` | Same checkout issue. With CRLF, the first adversary `golden.slice(0, -2)` removes only CRLF, leaving valid complete JSON. Pin LF so it removes the final closing brace and LF as intended. No parser relaxation or test expectation change. |

An isolated archived baseline with CRLF fixtures reproduced these exact four failures (nine document tests: five pass/four fail). `.gitattributes` now applies `text eol=lf` to exactly `tests/fixtures/image-document-v1.json` and its Swift bundled copy. JSON content and strict byte assertions are unchanged. `git -c core.autocrlf=true checkout-index` into an ignored directory confirmed both checkout copies have the original canonical SHA-256 `a18deede5475c2642501a8911212f76bd63f8ed8738dd3f7d5f3fbfff72216e7`. No blanket JSON normalization or checkout setting change was introduced.

Replacing only the isolated baseline's two CRLF fixture copies with those simulated Windows checkout outputs made the exact Foundation source selection pass 23/23: `node --test build/phase-2-ci-fix/baseline/tests/foundation.test.mjs build/phase-2-ci-fix/baseline/tests/ci-tools.test.mjs build/phase-2-ci-fix/baseline/tests/document-contract.test.mjs build/phase-2-ci-fix/baseline/tests/native-document.test.mjs`. This is observed checkout remediation evidence, not a hosted CI rerun.

### Sanitizer: exact root cause and fix

The retrieved CI log uses Ubuntu Clang 18.1.3. Ninja reports `Linking C executable c_consumer` and invokes `/usr/bin/clang -g -fsanitize=address,undefined ... c_consumer.c.o ... libpixaura_core.so`. Its linker errors are exactly `undefined reference to '__ubsan_vptr_type_cache'` and `undefined reference to '__ubsan_handle_dynamic_type_cache_miss'`. The first actual symbol differs from the screenshot transcription; this report uses the downloaded job log.

The consumer's C link driver does not select the C++ UBSan runtime providing dynamic-type/vptr support required by the sanitizer-instrumented C++ shared library. `CMakeLists.txt` now sets `LINKER_LANGUAGE CXX` for `c_consumer` and `document_c_consumer` only inside `if(PIXAURA_SANITIZERS)`. Both `.c` sources still compile with the C compiler. Generated Ninja rules confirm C compilation and CXX executable linking, retaining `-fsanitize=address,undefined` on compilation/linking. No vptr or other sanitizer suppression, source removal, architecture change or non-sanitizer link-language change occurred.

Local Clang 21.1.8 also links the archived baseline successfully, so the Clang 18 CI failure is not claimed as locally reproduced. The downloaded CI command/errors establish its cause; corrected builds and full runtime tests pass locally. The original CI compiler/runner must still validate the correction.

### Corrected-tree validation

| Check and exact command | Observed result |
| --- | --- |
| Windows: `node --test tests/foundation.test.mjs tests/ci-tools.test.mjs tests/shells.test.mjs tests/document-contract.test.mjs tests/native-document.test.mjs` | PASS 29/29, no skips; includes Apple package/fixture/CI wiring checks. |
| Windows: `node --test tests/*.test.mjs` | FAIL 29/58: the 29 Linux-only executable fake-SDK/emulator scenarios cannot execute correctly on Windows (including EPERM for the symlink scenario). All 29 Windows-compatible checks pass. No unrelated emulator test rewrite was authorized. |
| WSL Ubuntu: `export PATH="$PWD/build/tools/node-v24.14.0-linux-x64/bin:$PATH"; node --test tests/*.test.mjs` | PASS 58/58, no skips, including the complete emulator orchestration harness. |
| WSL: `cmake -S . -B build/linux-host -DCMAKE_BUILD_TYPE=Debug`; `cmake --build build/linux-host -j2`; `ctest --test-dir build/linux-host --output-on-failure` | PASS 5/5, warning-clean native C/C++ document, allocation and both C ABI consumers. |
| WSL: `bash scripts/check-sanitizers.sh` | PASS 7/7 with Clang 21.1.8, ASan/UBSan and both runtime negative probes (86/87); both C consumers use corrected CXX linking. |
| Windows: `powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check-native-zig.ps1 -ZigPath G:/PixAuraAI/build/tools/zig-x86_64-windows-0.14.1/zig.exe` | PASS shared DLL, C/C++ document/ABI consumers, allocation/concurrent-release tests and original UB trap probe. |
| Windows: `powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check-native.ps1` | BLOCKED: incomplete Visual Studio installation, `vcvarsall missing`. Owner host maintainer; repair desktop C++ workload. Hosted Windows MSVC build remains mandatory. |
| `build/tools/actionlint/actionlint.exe -shellcheck= -pyflakes= .github/workflows/foundation.yml .github/workflows/native-shells.yml`; WSL `bash -n scripts/*.sh` | PASS actionlint 1.7.12 and Bash syntax. Local shellcheck/pyflakes unavailable; not claimed executed. Workflows are unchanged. |
| Android NDK CMake configure/build, each ABI below | PASS fresh arm64-v8a and armeabi-v7a core/JNI shared library builds; pinned NDK 28.2.13676358, CMake 3.22.1, API 26. No platform source change. |
| `git diff --check` | PASS; no staged changes, commit or push during remediation. |

Android command, repeated with ABI/build directory `armeabi-v7a`/`android-armv7`:

```text
I:/AndroidStudioSDKdata/cmake/3.22.1/bin/cmake.exe -S platforms/android/native -B build/phase-2-ci-fix/android-arm64 -G Ninja -DCMAKE_MAKE_PROGRAM=I:/AndroidStudioSDKdata/cmake/3.22.1/bin/ninja.exe -DCMAKE_TOOLCHAIN_FILE=I:/AndroidStudioSDKdata/ndk/28.2.13676358/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-26
I:/AndroidStudioSDKdata/cmake/3.22.1/bin/cmake.exe --build build/phase-2-ci-fix/android-arm64
```

Only `.gitattributes`, `CMakeLists.txt` and this report changed. All fresh logs, isolated checkout/reproduction fixtures and generated binaries remain ignored under build. Shipping source, schema, caller contexts, platform bridges and document architecture are unchanged. Step 3 was not started.

Next human action: review these three-file corrections and explicitly authorize their commit/push. Then run both workflows on the resulting corrected SHA, requiring Windows portable source/MSVC execution and Ubuntu Clang 18 sanitizer success, plus preservation of all previously passing platform jobs. Owner repository maintainer/hosted runners; affects G1-G4. Apple runtime validation for the corrected tree is CI-only on this Windows host. No CI rerun of the old SHA can validate unpublished changes. Do not promote Step 2 as fully passed until corrected-tree mandatory CI succeeds; Step 3 still requires separate authorization.
