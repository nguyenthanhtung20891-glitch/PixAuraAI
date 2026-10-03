# Phase 2 Step 2: native document foundation

Status: IMPLEMENTED, READY FOR CI; mandatory Apple execution remains unobserved. 2026-10-03. This is not FULLY PASSED or full Phase 2 completion.

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
