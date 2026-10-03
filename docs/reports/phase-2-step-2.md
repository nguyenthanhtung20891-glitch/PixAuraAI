# Phase 2 Step 2: native document foundation

Status: APPROVED, USER-ATTESTED FULL STEP 2 CI PASS on `f78ba096872249bf80ff1e4ee9e51a0a4ecfb6ef`, 2026-10-04. The user reports both Foundation boundaries and Native application shells green and explicitly authorizes Step 3. No new passing-run URL was supplied; this is user-attested completion, not locally observed Apple/MSVC runtime. Historical sections 13-16 retain the earlier failures and evidence limits. This does not certify full Phase 2 or the subsequent Step 3 tree.

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

## 14. Final Windows C consumer portability remediation

The section 13 correction was reviewed, committed as `871ca77a362b7e91f9c776f11eea43072ff819b2` (`fix: stabilize phase 2 foundation CI`) and pushed with explicit approval. The user now reports Native application shells, Foundation sanitizer, Ubuntu portable, Apple boundary and Android boundaries PASS; only Windows portable remains FAIL. This rerun status is user-attested here, not independently retrieved evidence. The confirmed remaining diagnostic is MSVC C4996 for `fopen()` in `packages/core/tests/document_c_consumer.c`, promoted to build failure C2220 by warnings-as-errors.

The only source change is a private test-only `open_file(path, mode)` helper in that C consumer. Under `_MSC_VER` it initializes a `FILE*` to NULL, calls `fopen_s`, and returns NULL on nonzero status. Elsewhere it returns standard `fopen`. All three call sites use the helper: original fixture read/re-read retain `rb`; canonical artifact write retains `wb`. Existing NULL checks, exit 1 on failed opens, reads/writes/close checks and byte assertions remain unchanged. No production behavior, schema, context lifecycle or document architecture changed.

Validation on this unpublished correction:

| Check | Observed result |
| --- | --- |
| Windows-compatible source checks | PASS 29/29, no skips; command below. |
| Standard Windows MSVC script | BLOCKED: executed `scripts/check-native.ps1` prerequisite check still reports missing `vcvarsall`; no local `/W4 /WX` execution claimed. Owner host maintainer; repair Visual Studio desktop C++ setup. |
| Windows Zig 0.14.1 fallback | PASS full warning-clean DLL/C/C++/document/allocation tests with `-Wall -Wextra -Wpedantic -Werror`. |
| Explicit secure-open branch exercise | PASS: test-only Zig C compile with `_MSC_VER=1951` selected `fopen_s`; linked actual Windows CRT, executed fixture/canonical assertions successfully. This is branch/runtime coverage, not an MSVC compiler or C4996-policy validation. |
| Failed-open behavior | PASS both ordinary and forced-secure-branch consumers return 1 for missing input and output in a missing directory; existing error semantics preserved. |
| Linux native CTest | PASS 5/5, includes independent C consumers and native document/history/allocation tests. |
| Linux ASan/UBSan | PASS 7/7, Clang 21.1.8; includes actual negative instrumentation probes (86/87). |
| Canonical byte parity | PASS secure-open Windows/Linux/sanitizer output hashes match approved fixture SHA-256 `a18deede5475c2642501a8911212f76bd63f8ed8738dd3f7d5f3fbfff72216e7`. |
| Warning policy and scope | PASS build/scripts unchanged: CMake retains `/W4 /WX` and `-Wall -Wextra -Wpedantic -Werror`; no `_CRT_SECURE_NO_WARNINGS` or warning suppression introduced. Only this report and the test consumer changed. |

Exact primary commands:

```text
node --test tests/foundation.test.mjs tests/ci-tools.test.mjs tests/shells.test.mjs tests/document-contract.test.mjs tests/native-document.test.mjs
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check-native.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check-native-zig.ps1 -ZigPath G:/PixAuraAI/build/tools/zig-x86_64-windows-0.14.1/zig.exe
wsl --distribution Ubuntu --exec bash -lc 'cd /mnt/g/PixAuraAI && cmake --build build/linux-host -j2 && ctest --test-dir build/linux-host --output-on-failure && bash scripts/check-sanitizers.sh'
```

Additional secure-branch command: `build/tools/zig-x86_64-windows-0.14.1/zig.exe cc -target x86_64-windows-gnu -std=c11 -Wall -Wextra -Wpedantic -Werror -D_MSC_VER=1951 -DPIXAURA_SHARED -I packages/core/include packages/core/tests/document_c_consumer.c build/zig-host/pixaura_core.lib -o build/zig-host/document_c_consumer-msvc-branch.exe`; executed with approved fixture and canonical output path. Zig caches were explicitly repository-local. All logs/generated outputs are ignored under `build/phase-2-fopen-fix` or existing ignored host build directories; none is staged.

Remaining confirmed CI-only gate: Foundation Windows portable must build/run the corrected C consumer with real hosted MSVC and unchanged `/W4 /WX` on the newly published SHA (G1/G3 and native G2). Previously passing workflows must remain green for that SHA; their older passing results do not certify this unpublished tree. No local macOS runtime PASS is claimed. Next human action: review this two-file fix and explicitly authorize commit/push, then obtain corrected-SHA workflow evidence. No commit/push occurred during this remediation. Stop before Step 3; it remains separately unauthorized.

## 15. Windows allocation-test cancellation and bounded diagnostics

The section 14 fix was reviewed and published with explicit approval as `780c622a46424d61cff09a6659876cb75d39e333` (`fix: make document C consumer MSVC portable`). This investigation retrieved the exact [Foundation run 37137067831](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37137067831), [Windows job 111243647415](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37137067831/job/111243647415), check-run annotations, and retained `document-foundation-windows-2025` artifact. The jobs API confirms [Native application shells run 37137067706](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37137067706) succeeded; all other Foundation jobs succeeded for that SHA. This is independently observed hosted evidence, not local Apple execution or evidence for the unpublished diagnostic changes below.

### Confirmed cancellation versus unknown internal subcase

The check annotation states exactly `The job has exceeded the maximum execution time of 15m0s`. Windows started at `2026-10-03T16:30:00Z`, finished at `16:45:08Z`, and has conclusion `cancelled`. The log confirms Visual Studio 18 2026/MSVC 19.51.36260.0 built every Debug target warning-clean. CTest then passed core_test (0.29 s), c_consumer (0.01 s), document_test (2.14 s), document_c_consumer (0.01 s), and started document_allocation_test at `16:30:56.6873562Z`. The next test-step event at `16:45:01.2484393Z` is `The operation was canceled.`

Thus the exact cancellation cause is the GitHub **job-level 15-minute timeout**, not an observed step timeout, CTest timeout, manual runner cancellation or compiler failure. The native-test phase occupied approximately 14 minutes after allocation-test start. There was no explicit CTest timeout on this test; no test-specific failure message or internal stack was emitted.

The retained artifact contains `CTestCheckpoint.txt` with only tests 1-4 and an unfinished `LastTest.log.tmp576e7` ending after document_c_consumer. It contains no allocation-test output. The old test printed sweep summaries to buffered stdout only after completion. These sources cannot determine which allocation index, concurrency stage or teardown operation stopped. No exact internal MSVC subcase or deadlock is claimed as confirmed.

Source inspection separates the possible sites: main-thread context initialization/warm open; exhaustive open/serialize/apply failure sweeps; a serializer worker paused at its first post-acquire allocation while main releases the handle; worker completion/join; stale-handle checks and context teardown. Synchronization uses atomic flags and spin/yield waits, plus the production registry mutex; the test has no condition variable. Pin/resume waits already had five-second bounds, but API calls, successful-call teardown and thread join had no independent supervisory bound. A destructor or API mutex stall could therefore outlive those waits. Source alone does not prove one occurred.

MSVC Debug behavior also differs from the local Clang/GCC-style fallback: [Microsoft STL's string move constructor](https://github.com/microsoft/STL/blob/main/stl/inc/xstring) is `noexcept` while allocating an iterator-debug proxy through [xmemory](https://github.com/microsoft/STL/blob/main/stl/inc/xmemory). Injecting failure into such debug-only allocation can terminate rather than reach the C API's bad_alloc handler. Microsoft documents that [abort in Debug can display an interactive dialog](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/abort?view=msvc-170). Those are supported source-level risks, not proof of this run's exact path; current upstream STL source is not asserted to be a byte-identical copy of the runner toolset. No allocator-mode/iterator-debug suppression, one-shot injection, skipped failure index or production exception redesign was introduced to bypass a suspected cause.

### Narrow remediation implemented

- `packages/core/tests/document_allocation_test.cpp`: flushed C-stdio progress identifies phase, open/serialize/apply path (0/1/2), failure index, requested allocation bytes and returned status. It brackets every injected call, sentinel/canonical/prior-snapshot check, release and teardown, and concurrency start/pin/release/resume/completion/join. Logging does not allocate through the replacement C++ allocator.
- The same test retains persistent fail-after injection and all 10000 candidate indices per sweep until first success. A separate noninjected watchdog fails with the current phase/path/index after 15 seconds without progress or a 90-second total execution limit. It is created before injection; its thread-local fail_after remains disabled. Serializer completion now has an explicit five-second wait before joining; the watchdog also covers join and API/teardown stalls. No worker is detached and no failure becomes a success.
- Test assertions and `std::terminate` now print the active subcase and exit 1 without unwinding or calling abort. MSVC CRT reports are directed to stderr and interactive abort/error-report behavior is disabled **only in this test executable**. CRT reports, warning checks and actual failures remain observable. Additional post-destroy stale-release/repeated-destroy assertions retain and extend ownership checks.
- `CMakeLists.txt`: `document_allocation_test` has a 120-second CTest TIMEOUT as an outer process guard, including failures before watchdog startup. `/W4 /WX`, `-Werror`, ASan/UBSan flags and C/C++ ABI consumers are unchanged.
- `.github/workflows/foundation.yml`: portable CTest uses `--verbose` so flushed diagnostics appear as the subcase runs. The job timeout remains 15 minutes; no timeout increase was made. Existing CTest artifacts are retained.
- This report records observed cancellation and explicitly leaves the unknown internal MSVC subcase unresolved.

### Validation and evidence limits

| Check | Observed result |
| --- | --- |
| Windows-compatible Node source checks | PASS 29/29, zero skips. |
| Windows Zig fallback native suite | PASS, including complete allocation sweeps, concurrent pinned serialization/release, stale/destroyed handles and teardown. Counts remain exactly 142/190/229 failures before success, matching prediagnostic evidence. |
| WSL Linux native CTest | PASS 5/5 with verbose allocation diagnostics and computed 120-second test timeout. Failure counts remain 192/227/278. |
| Actual ASan/UBSan | PASS 7/7, including allocation tests and negative runtime probes (86/87); same 192/227/278 failures. Local compiler is Clang 21.1.8. |
| Diagnostic failure probes | PASS on Windows fallback: an ignored copy with a deliberate blocked subcase exits 1 in 15.50 seconds, names `probe.watchdog`, path 1/index 7 and the 15-second no-progress bound. A separate ignored copy calls terminate with injection active and exits 1 naming `probe.terminate`, path 2/index 9 and exception/noexcept termination. Neither probe changes shipping/test-target sources or replaces the ordinary full-coverage runs. |
| Real local MSVC | BLOCKED: standard script rechecked, reports `vcvarsall missing`; incomplete installation also lacks STL headers. No MSVC reproduction, debugger stack or exact internal subcase is claimed. Owner host maintainer, remediation repair complete desktop C++/SDK setup; hosted MSVC remains CI-only on this host. |
| Actionlint/source hygiene/whitespace | PASS; no compiler warning policy or sanitizer suppression changed. |

Primary commands:

```text
node --test tests/foundation.test.mjs tests/ci-tools.test.mjs tests/shells.test.mjs tests/document-contract.test.mjs tests/native-document.test.mjs
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check-native.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check-native-zig.ps1 -ZigPath G:/PixAuraAI/build/tools/zig-x86_64-windows-0.14.1/zig.exe
wsl --distribution Ubuntu --exec bash -lc 'cd /mnt/g/PixAuraAI && cmake -S . -B build/linux-host -DCMAKE_BUILD_TYPE=Debug && cmake --build build/linux-host -j2 && ctest --test-dir build/linux-host --output-on-failure --verbose && bash scripts/check-sanitizers.sh'
build/tools/actionlint/actionlint.exe -shellcheck= -pyflakes= .github/workflows/foundation.yml .github/workflows/native-shells.yml
git diff --check
```

GitHub logs, check annotations, artifact contents, fresh local logs and deliberately failing probe sources/binaries are ignored under `build/phase-2-allocation-ci`. Initial probe orchestration failed due the host PowerShell Path/PATH environment collision; direct executable invocation succeeded. A last-line-only probe assertion also initially selected buffered stdout instead of the stderr diagnostic; corrected validation checks the full log and actual exit code. Neither unsuccessful harness attempt is counted PASS.

Only the four files listed above changed; nothing was staged, committed or pushed. No shipping source or document architecture changed; Step 3 was not started. This correction makes indefinite waits bounded and observable; it does **not** claim the unknown MSVC allocation subcase has been fixed. Remaining gate: execute Foundation Windows portable on the instrumented tree using actual MSVC Debug and inspect its phase/path/index plus returned status or termination/bound diagnostic. If it fails, correct that confirmed subcase while preserving full injection/concurrency/ownership coverage. Other workflows must remain green on the corrected SHA.

Exact next human action: review and explicitly authorize publishing these four diagnostic/bound changes for CI. A rerun of the old SHA cannot reveal new diagnostics. Repository maintainer/hosted runner owns the remaining MSVC G2/G3 evidence. Step 2 is not fully passed until that gate completes; Step 3 still requires separate authorization.

## 16. Confirmed MSVC allocation exception and catchable construction

The section 15 instrumentation was reviewed and published as `f68168207f0c130463ed9df4ef06c3b7cf281ed2`. Retrieved [Foundation run 37139721362](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37139721362) and [Windows job 111251458169](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37139721362/job/111251458169) expose the exact previously unknown subcase: `sweep.call`, path 0 (open), fail_after 2, allocation 16 bytes, then `std::terminate: unhandled exception or exception escaped noexcept`. The instrumented test exits as a failure in 0.04 seconds. This is exception termination, not a thread/join/condition-variable deadlock or another job timeout. [Native application shells run 37139721358](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37139721358) succeeded. These hosted results apply to the published diagnostic SHA, not the unpublished correction below.

### Call chain and allocation site

The production source and logged allocation order reconstruct this call chain:

```text
pixaura_document_open
  open_impl (noexcept)
    boundary (noexcept; catches bad_alloc)
      deserialize -> attempt<Snapshot>
        Parser::parse -> Parser::value (root JSON object)
          Json::Object result                 [map proxy 16; head/sentinel 120]
          Parser::string -> std::string result
            std::basic_string<char>::basic_string() noexcept
              _Construct_empty -> _Container_base12::_Alloc_proxy
                allocator<_Container_proxy>::allocate(1)
                  operator new(16) -> injected std::bad_alloc -> terminate
```

The offending noexcept function is the **MSVC STL default string constructor**, reached at the first object-key buffer in `Parser::string`, before `allowed(shape)` is called. Its noexcept condition is true for the default allocator. The 16 bytes hold an iterator-debug `_Container_proxy`, comprising two x64 pointers (container and first iterator), rather than JSON character storage. Earlier failures at index 0 (map proxy, 16 bytes) and index 1 (map sentinel, 120 bytes) return status 8; index 2 enters the string's noexcept constructor, so outer catches cannot run. This trace is reconstructed from control flow and the hosted allocation sequence; no local MSVC debugger stack was captured. Primary references: [Microsoft string implementation](https://github.com/microsoft/STL/blob/main/stl/inc/xstring), [proxy implementation](https://github.com/microsoft/STL/blob/main/stl/inc/xmemory), and [tree construction](https://github.com/microsoft/STL/blob/main/stl/inc/xtree). Current upstream headers are not claimed to be byte-identical to the hosted toolset.

The audit also found allocation-bearing noexcept string/vector **move constructors** and the vector default constructor in MSVC iterator-debug mode. Fixing only the first local string would leave equivalent termination paths in parser variants, identity/domain values, history stacks and serializer results. [Microsoft vector implementation](https://github.com/microsoft/STL/blob/main/stl/inc/vector) documents those proxy allocations.

### Exact correction and ABI audit

`document_containers.hpp` adds private `String`/`Vector<T>` adapters only when MSVC iterator debugging is enabled. Empty strings use the throwing pointer/count constructor; empty vectors use the throwing empty initializer-list constructor. Adapter move construction/assignment uses throwing copy paths, retaining source validity and allowing failures to propagate to the existing guard. Parser/domain/serializer owning values use these private types. Iterator debugging and its proxy allocations remain enabled; the same first string proxy remains allocation index 2 under MSVC. Other builds use aliases to the existing standard types. The tradeoff is extra bounded copying in MSVC Debug; ordinary release/platform container behavior is unchanged. No C header, ABI version, manifest, operation graph or context ownership contract changes.

All seven exported document C ABI functions were reviewed:

| Export | Exception/publication behavior |
| --- | --- |
| context_init | Registry/map construction is inside `boundary`; context storage is published only after construction succeeds. |
| context_destroy | Guarded destruction/deallocation and fixed-width context update; no allocating cleanup path. |
| open | `open_impl` retains noexcept and invokes `boundary`; parser/domain construction can now throw normally. Registry insertion precedes caller output publication. |
| create | Same guarded construction path, with independent root-only admission/publication coverage. |
| apply | Acquisition, transition and insertion stay guarded; prior immutable snapshot and caller output survive construction failure. |
| serialize | Result construction stays guarded; required size/caller bytes are written only after successful serialization (or the documented capacity result). |
| release | Guarded ownership check/erase; no allocating cleanup path. |

Reference-only lambda captures do not allocate before entering the guard. Map/set constructors are throwing; shared-pointer copies/moves and fixed-width handles do not allocate. Remaining numeric `std::to_string` uses a throwing sized-string construction and C++17 return elision. `valid_error` and `report` are explicitly noexcept: validation and error translation use fixed storage, literal status names and byte copies. `boundary` still catches `Failure`, `std::bad_alloc` and all other exceptions, returning `PIXAURA_DOCUMENT_RESOURCE_LIMIT` (8) for allocation failure. If construction of an internal error-result value also fails under persistent injection, that exception reaches the outer guard through a throwing adapter rather than terminating. No C++ exception is allowed across the C ABI; its existing noexcept guards remain.

### Regression coverage and observed validation

The allocation executable explicitly exercises path 0/fail_after 2, checks status/error code 8 and `RESOURCE_LIMIT`, preserves output sentinel and caller context bytes, compares the prior serialized snapshot byte-for-byte with the golden manifest, then successfully opens/releases another document. Valid/stale release and context destruction are checked with allocation failure active. Exhaustive persistent-injection sweeps retain all indices until first success for open/serialize/apply, add create and context initialization, and validate the full prior snapshot after each failed operation. Existing stale/destroyed ownership, pinned serialization/release concurrency, watchdog and 120-second CTest limit remain. Compile-time assertions require adapter default/move construction to be potentially throwing.

CMake and both Windows native scripts define `PIXAURA_TEST_FALLIBLE_STL` **only for the independent allocation executable**, so the adapters are exercised on fallback/Linux as well as MSVC. The macro does not enter shipping targets. A separate ignored validation build forces adapters across the entire C++ document suite to test history, validation and serialization behavior. Neither fallback nor Linux emulates MSVC debug proxy allocation: their explicit index-2 allocation sizes are 88 and 61 bytes respectively; only hosted MSVC can confirm the corrected 16-byte proxy case.

| Check | Observed result on final correction |
| --- | --- |
| Windows-compatible Node source tests | PASS 29/29, zero failures/skips. |
| Windows Zig fallback native suite | PASS core/document/C ABI consumers and allocation/concurrency/ownership suite. Open/serialize/apply/create failure counts: 416/389/441/89; context init fails at index 0 then succeeds at 1. |
| WSL Linux native CTest | PASS 5/5, including both C consumers. Allocation failure counts: 497/448/515/98; context init fails at 0 then succeeds at 1. |
| ASan/UBSan | PASS 7/7, including full allocation suite and actual negative runtime probes (86/87); same 497/448/515/98 counts. No sanitizer flags or vptr coverage removed. |
| Entire native suite with adapters forced | PASS 5/5 in separate Clang Debug CMake build; includes document validation/history/canonical tests as well as the allocation executable. |
| Real local MSVC | BLOCKED: `check-native.ps1` reports `vcvarsall missing`; local installation also lacks STL headers. Hosted MSVC Debug execution remains CI-only; host maintainer can repair desktop C++/SDK installation. |
| Actionlint and whitespace | PASS; `/W4 /WX`, `-Wall -Wextra -Wpedantic -Werror`, watchdog/CTest bounds and workflow job timeout remain unchanged. |

Primary commands:

```text
node --test tests/foundation.test.mjs tests/ci-tools.test.mjs tests/shells.test.mjs tests/document-contract.test.mjs tests/native-document.test.mjs
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check-native.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check-native-zig.ps1 -ZigPath G:/PixAuraAI/build/tools/zig-x86_64-windows-0.14.1/zig.exe
wsl --distribution Ubuntu --exec bash -lc 'cd /mnt/g/PixAuraAI && cmake --build build/linux-host -j2 && ctest --test-dir build/linux-host --output-on-failure --verbose && bash scripts/check-sanitizers.sh'
cmake -S . -B build/phase-2-exception-ci/fallible-host -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS=-DPIXAURA_TEST_FALLIBLE_STL
cmake --build build/phase-2-exception-ci/fallible-host -j2
ctest --test-dir build/phase-2-exception-ci/fallible-host --output-on-failure --verbose
build/tools/actionlint/actionlint.exe -shellcheck= -pyflakes= .github/workflows/foundation.yml .github/workflows/native-shells.yml
git diff --check
```

The final three CMake commands ran inside WSL. Retrieved job metadata/logs and local validation outputs are ignored under `build/phase-2-exception-ci`; no investigation/build output is staged. Initial adapter compilation exposed inherited-constructor syntax and zero-count vector element constraints; corrected code uses `using Base::Base` and empty initializer-list construction. Those initial failed builds are not counted as validation passes.

Exact changed files: `CMakeLists.txt`, `packages/core/src/document_containers.hpp` (new), `packages/core/src/document.hpp`, `packages/core/src/document.cpp`, `packages/core/src/document_api.cpp`, `packages/core/tests/document_allocation_test.cpp`, `scripts/check-native.ps1`, `scripts/check-native-zig.ps1`, and this report. No workflow, public C header, dependency or architecture decision changed. Nothing was staged, committed or pushed; Step 3 was not started.

Remaining gate: Foundation Windows portable must run actual MSVC Debug on the corrected published tree, report resource status for path 0/index 2, and complete every sweep plus ordinary document/C ABI tests within the existing bounds. Other Foundation/native-shell workflow gates must remain green for that same SHA; historical CI passes do not certify this unpublished tree. No local macOS runtime PASS is claimed.

Exact next human action: review this nine-file exception-safety correction and regression evidence, then explicitly authorize commit/push for CI. After publication, inspect the hosted MSVC regression/sweep output and require both workflows green before accepting Step 2. Repository maintainer/hosted runner owns that CI gate; Step 3 remains separately unauthorized.
