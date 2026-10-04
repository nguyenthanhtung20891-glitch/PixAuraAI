# Phase 2 Step 3: durable persistence and immutable asset storage

Status: IMPLEMENTED; autonomous CI closeout authorized on 2026-10-04; corrected-revision CI gates pending. Step 4 remains unauthorized. The user attests Foundation boundaries and Native application shells fully green for approved Step 2 revision `f78ba096872249bf80ff1e4ee9e51a0a4ecfb6ef`. That attestation authorizes this step; it is not an independently observed Apple/MSVC execution for this working tree. Historical remediation sections below retain their original authorization and evidence; section 12 records the current closeout.

## 1. Repository findings and scope

Audited PRODUCT_SPEC, ARCHITECTURE, QUALITY_GATES, ROADMAP, PRIVACY_SECURITY, TESTING_STRATEGY, dependency/toolchain review, Step 1 contract/report, Step 2 types/API/fixture/builds, and ADRs 0002/0008/0009. Steps 1/2 already freeze SQLite authority, SHA-256 immutable source identity, full ordered revision stacks, retained single parents, current/redo navigation, canonical schema 1, fresh sessions/generations, durable epochs, asset-before-reference, explicit migration and caller-owned document contexts. No dependency, live database, importer or filesystem implementation previously existed. The fixture's `aaaa...` source digest is synthetic metadata, not real verified bytes. Storage tests substitute the SHA-256 of synthetic streamed bytes; they do not modify the approved fixture or claim decoded metadata.

Open implementation decisions were SQL normalization/constraints, direct dependency/toolchain integration, journal/sync settings, filesystem primitives, service lifecycle, recovery/cleanup scope and checkpoint scheduling. [ADR 0010](../adr/0010-durable-native-project-storage.md) and [storage contract](../contracts/project-storage-v1.md) record them. No frozen semantics conflict was found. Shared direct SQLite services implement native authority; platform seams provide roots/lifecycle, never duplicated document semantics. No image decode/encode, pixels, rendering, GPU, adjustments, masks, AI, UI, cloud or Step 4 work.

## 2. Architecture, representation and transactions

Internal `AssetStore`/`Repository` services own private-root capabilities and SQLite connections. `storage_schema.hpp` defines STRICT normalized assets, documents, operations, revisions, full ordered stacks and redo. Identity/order uniqueness, asset/current/parent/stack FKs, parameter shape/ranges, bounded indices, actor/plan rules, parent-order and immutability triggers are explicit. Current FK is deferred so a single transaction can publish document and revision. Native validation additionally rejects unreferenced operations, invalid redo/root/history semantics and malformed canonical metadata. Reopen enforces a compiled expected schema fingerprint, application/storage version, quick_check/foreign_key_check, bounded row/position counts, existing C++ parsing/validation and source/profile hash verification. SQL uses fixed text plus bound values.

Create/import inserts only validated approved history and verified source association. Apply uses Step 2 transition then appends fresh tails and atomically updates current/redo/session generation/epoch. Parameter replacement creates new operation/revision rows; old meanings never change. Undo/redo/checkout do not rewrite history; branch-after-undo clears navigation redo only. Expected durable epoch plus native session/base/generation checks reject competing or stale writers. Fresh-session acquisition resets generation and increments durable epoch explicitly. SQLite schema version, manifest version, operation versions and ABI version remain independent. No timestamps or redundant serialized DB snapshots are introduced.

## 3. Durability, checkpoints and migration

WAL/FULL, foreign_keys ON, fullfsync/checkpoint_fullfsync, 250 ms busy timeout, BEGIN IMMEDIATE writes, 1000-page auto-checkpoint, 16 MiB journal recycle target, 4096-byte/65536-page DB budget, and 32 MiB pre-write WAL admission are deliberate. Above the WAL threshold a bounded truncate checkpoint must finish; pinned readers produce BUSY, never unlimited new writes. Single bounded transactions can temporarily exceed that admission threshold. FULL prioritizes committed WAL durability while allowing two readers during writes. Rollback/EXTRA was considered but would block that concurrency use case. File/directory sync errors propagate; hardware power-loss behavior is not inferred from subprocess tests.

After DB commit, checkpoint publication reserves the writer, reads a consistent immutable DB snapshot, canonicalizes and reparses/reproduces bytes, syncs a unique bounded temp and atomically replaces checkpoint.json. `Saved{epoch, checkpoint_published}` reports committed state even if checkpoint work fails. Reopen uses only SQLite, never a stale checkpoint. Atomic replacement can leave old/new checkpoint after interruption; either is harmless to live authority.

Ordinary open initializes only an empty unversioned store; unsupported existing versions reject. Explicit migrate(1,1) transactionally verifies a no-op and fault-tests rollback; all invented historical/future pairs reject. There is no historical migration to implement. Future actual transforms must use the already-frozen temporary-copy/backup/validation/publication protocol; no in-place or implicit upgrade is permitted by this infrastructure.

## 4. Asset store and security

Ingest uses one 64 KiB content buffer, expected exact positive size <=8 GiB, OS-random exclusive temp names, streaming FIPS 180-4 SHA-256, optional expected digest, sync/close, no-overwrite content-derived basename and destination hash/length verification. Source names are only bounded optional metadata and currently ignored rather than introducing a new manifest field. Identical content deduplicates independent of filenames; mismatch/truncation/read/write errors never replace assets. Source/profile references follow complete publication, never the reverse. DB failure may leave an immutable orphan.

POSIX opens components/capabilities with NOFOLLOW, uses read-only mode, linkat without replacement and directory fsync. Darwin requests full file barriers. Windows pins directory ancestors, checks reparse attributes/link counts, uses BCrypt OS randomness, FlushFileBuffers and same-volume WRITE_THROUGH MoveFileEx. Existing originals are never overwritten. Reopen rejects missing/truncated/tampered bytes. Temps and orphan assets are retained/ignored safely; no automatic promotion, checkpoint restore or history/asset deletion occurs. Normal error cleanup targets only that call's owned temp.

Reviewed filename/path traversal, NUL/length/identity bounds, Windows ADS, links/hard-link aliases/reparse points, hash/substitution, integer arithmetic, malformed schema/rows/JSON, SQL injection and allocation amplification. Root is an existing caller-owned local OS-private directory, stable for service lifetime. SQLite fixed-path/sidecar checks and NOFOLLOW rely on that namespace ownership; no protection against compromised same-UID private-root replacement is claimed. No content or filename logging ships. SQLite per-connection value/SQL/depth/column/variable/attachment/VM-step/cache budgets complement document limits; those are not a certified whole-process RSS limit. Source hashing validates bytes, not encoded metadata; decode/metadata extraction awaits separately authorized work.

## 5. API and platform integration

Private C++ services are authoritative. `restore_generation` restores only a validated storage session envelope; schema-1 serialization remains unchanged. Existing document ABI/contexts/handles and zero feature bits remain intact. A separate checked one-shot `pixaura_storage_check` creates/reopens the schema and returns fixed-width version info. It exposes no SQLite handles, C++ values, internal pointers or filesystem internals, contains exceptions, and preserves output on failure. No persistence handles/global application registry were added. JNI instrumentation and Swift package tests use the same native schema creation/reopen seam; production picker/import/edit UI was not introduced.

Repository calls serialize on its mutex; distinct connections have WAL reader snapshots and serialized writers, with epoch comparison. Asset ingestion is concurrent and duplicate publication uses OS exclusivity plus verification. Internal destruction requires exclusive caller access or shared ownership retained through work; the one-shot C function has no persistent context to destroy. Step 2 document-context destruction remains exclusive as frozen in ADR 0009. Concurrency tests retain service ownership while the caller releases its reference.

## 6. Tests and failure coverage

`storage_test`: fresh schema/reopen, exact canonical reproduction, full history/branch retention, current/redo and fresh-session restart/stale rejection, parameter replacement immutability, FKs/duplicates/dangling parent/bad rows/schema, future version rejection, explicit migration admission/failure, atomic append/navigation rollback, BUSY, WAL reader budget recovery, source-before-reference and stored tamper rejection. Asset tests: empty rejection, known empty/abc/million-a SHA-256 vectors, multi-chunk bounded buffer, filename-independent dedup/different content, truncation/excess bytes, read/write/digest failures, collision preservation, traversal, link/reparse checks where available and concurrent duplicate ingest. Independent C storage consumer checks layout/version/errors/sentinels plus creation/reopen. Existing C/C++ consumers, ownership/allocation sweeps and sanitizers remain.

Nine child-process crashes exit 73 without unwinding/destructors: before temp; during write; after file sync/before publish; after publish/before DB; inside DB transaction; after DB commit/before checkpoint; during checkpoint temp write; before checkpoint rename; immediately after checkpoint rename. Points 0-4 reopen prior empty DB with no broken reference; points 5-8 reopen the complete committed document regardless of checkpoint/temp state. Incomplete staged bytes never appear in trusted digest storage. Separate ordinary failures check status/rollback and committed-epoch/false-checkpoint reporting; pinned reader/writer barriers use five-second test bounds. CTest storage bound is 180 seconds; existing allocation bounds/15-minute workflow budget are unchanged. No hook/abrupt-exit behavior ships: fault state/hooks are compiled only in the independent test executable.

## 7. Validation evidence

Final local evidence (logs under ignored `build/phase-2-step-3`):

| Gate | Observed result / command |
| --- | --- |
| Windows compatible source | 32/32, no skips; `node --test` foundation, ci-tools, shells, document-contract, native-document, persistence suites; windows-source.log |
| Linux source | 61/61, no skips; Node 24.14.0 `--test tests/*.test.mjs`, including Linux-only emulator orchestration harness; linux-source.log |
| Windows native fallback | PASS; Zig 0.14.1 `scripts/check-native-zig.ps1`, existing native/allocation consumers plus storage/crash/concurrency and C storage consumer; windows-native.log |
| Linux native | CMake Debug/Clang 21.1.8/Ninja build and CTest 7/7; linux.log |
| ASan/UBSan | `bash scripts/check-sanitizers.sh`, CTest 9/9 including both deliberate runtime-negative probes, instrumented SQLite and new storage tests; sanitizers.log |
| Android | Offline strict dependency verification, Debug/Release builds, unit tests, both lints and instrumentation APK PASS; connected instrumentation 3/3, zero failures/skips; android-final.log and connected XML |
| Android native compatibility | Actual Gradle builds include arm64-v8a, armeabi-v7a and x86_64; Android 16/API 36.1 local x86_64 emulator ran JNI document/storage/shell tests |
| Workflow/wiring | actionlint 1.7.12 both workflows PASS; Node verifies Apple/Swift/CMake/Android integration; shell syntax checks PASS |
| Whitespace/index | `git diff --check` PASS; staged index empty, generated outputs ignored |
| Real MSVC | BLOCKED locally: missing vcvarsall/STL desktop installation; hosted Windows Debug build/runtime remains mandatory |
| Apple | No local macOS, Swift/Darwin or iOS runtime execution; existing Apple workflow wiring verified only |

Actual Android instrumentation initially exposed root-access failure: private app storage is accessible without permission to list `/data` ancestors. Linux/Android now retain search-only O_PATH intermediate capabilities with NOFOLLOW, then a readable owned leaf; canonical private roots are passed by boundary tests. A native mode-0111 ancestor regression proves this behavior. Darwin uses atomic O_NOFOLLOW_ANY root opening, subject to Apple CI execution. Rerun instrumentation passed all three tests. Windows exercised rejection through a real NTFS junction where symbolic-link creation was unavailable. Final hardening additionally proves exclusive temp-name collisions preserve another owner's staged file, for both assets and checkpoints; cleanup activates only after this call successfully creates its temp.

The last Linux rerun first used a nonexistent check-native.sh command (no test ran); the corrected documented configure/build/CTest command above passed. This attempt is not counted as a validation result. Android's initial SDK_HOME/USER_HOME environment conflict was corrected without changing product code.

Failed diagnostic attempts: callback needed an explicit int return; independent storage target needed the native include directory; unchanged upstream Windows SQLite generated unused VFS parameters/legacy OS variable warnings under Clang -Wextra, now exempted only on that vendor C compilation while -Werror remains. MSVC upstream already carries its own warning pragmas; no application warning flags were relaxed. A restricted Windows run could not open host temp ancestors (Access Denied); unchanged security checks passed with authorized escalation. An initial Windows invocation included the explicitly Linux-only bash emulator harness, which failed on the incompatible host; its complete cases are run under Linux, and Windows uses its compatible source suite. Missing report links during documentation construction were corrected. None of these failed attempts is counted PASS.

## 8. Dependency/license and build integration

SQLite 3.53.4 direct official amalgamation, public-domain dedication (SPDX blessing), archive SHA3-256 `628a44cfe82c66aed1ccbbe85a562d2e33ebe64b3288981ed76285612227934e` verified before extraction. Only unchanged sqlite3.c/h are vendored, with exact SHA-256 provenance and dedication. CMake config fails on changed hashes; source gates and Windows fallback verify them; Swift/Android build the same offline pin. No ORM/network/runtime extension loader or new Gradle artifact. Compile options disable load extensions/shared cache/DQS and bound metadata/SQL/attachments. Official release/CVE information was reviewed; no exhaustive vulnerability-free certification is claimed. Dependencies documentation records footprint/provenance/security scope. Windows bcrypt is an OS library, not a redistributed dependency. Own SHA-256 is content identity only, not custom encryption/authentication or a replacement for OS protection.

Source hygiene exempts only the two exact-hashed upstream files from local formatting rules, preserving provenance; all authored files retain hygiene checks. Git attributes preserve vendor bytes. SQLite symbols are hidden in CMake/Apple; checked public C APIs stay explicit. Foundation CI Windows/Ubuntu/macOS native CTest and sanitizer jobs automatically execute new tests. Android packages the same core; JNI/Swift tests are added to existing actual emulator/simulator gates. Both workflow source jobs verify the new dependency/wiring. Action commit/toolchain pins and supply-chain verification remain unchanged.

## 9. Changed files, risks and human gate

The exact working-tree inventory follows below. All downloads/builds/investigation outputs remain ignored under build; test-private directories are cleaned by the harness. No generated apps/libraries are staged. Nothing is committed/pushed.

Remaining mandatory CI-only gates: real MSVC Debug runtime, macOS native/Swift/Darwin durability primitives and iOS simulator/app checks on the eventual published revision. Android local evidence will be classified by actual execution. Native subprocess crashes prove process interruption, not physical power failure or filesystem firmware honesty. Apple production root backup/protection attributes remain platform integration obligations; schema probes use ephemeral test roots. Orphan GC, real historical migrations, standalone checkpoint recovery UI, metadata decoding and actual application approval wiring are explicitly later scope.

Proposed Step 4: separately review/freeze bounded decode and encoded metadata-verification contracts, reference formats/budgets and platform codec ownership before any implementation. This is a proposal only, with no decoder/rendering code started.

Exact next human actions: review the implementation/ADR/dependency/validation evidence; explicitly authorize commit/push if accepted; obtain all mandatory corrected-SHA workflow results before marking Step 3 fully passed; then issue separate Step 4 scope/authorization. Repair local MSVC desktop C++/SDK setup if local real-MSVC evidence is desired (host maintainer). No product decision requires weakening the frozen architecture.

## Exact file inventory (43 files)

- `.gitattributes`
- `.github/workflows/foundation.yml`
- `.github/workflows/native-shells.yml`
- `AGENTS.md`
- `ARCHITECTURE.md`
- `CMakeLists.txt`
- `DECISIONS.md`
- `QUALITY_GATES.md`
- `README.md`
- `ROADMAP.md`
- `TESTING_STRATEGY.md`
- `docs/DEPENDENCIES.md`
- `docs/reports/phase-2-step-2.md`
- `packages/core/Package.swift`
- `packages/core/src/document.cpp`
- `packages/core/src/document.hpp`
- `platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/CoreProbe.kt`
- `platforms/android/native/bridge.cpp`
- `scripts/check-apple.sh`
- `scripts/check-native-zig.ps1`
- `scripts/check-native.ps1`
- `tests/foundation.test.mjs`
- `docs/adr/0010-durable-native-project-storage.md`
- `docs/contracts/project-storage-v1.md`
- `docs/reports/phase-2-step-3.md`
- `packages/core/include/pixaura/storage.h`
- `packages/core/src/sha256.cpp`
- `packages/core/src/sha256.hpp`
- `packages/core/src/storage.cpp`
- `packages/core/src/storage.hpp`
- `packages/core/src/storage_api.cpp`
- `packages/core/src/storage_files.cpp`
- `packages/core/src/storage_files.hpp`
- `packages/core/src/storage_schema.hpp`
- `packages/core/swift/Tests/StorageBoundaryTests.swift`
- `packages/core/tests/storage_c_consumer.c`
- `packages/core/tests/storage_test.cpp`
- `packages/core/vendor/sqlite/LICENSE.md`
- `packages/core/vendor/sqlite/provenance.json`
- `packages/core/vendor/sqlite/sqlite3.c`
- `packages/core/vendor/sqlite/sqlite3.h`
- `platforms/android/app/src/androidTest/java/ai/pixaura/app/StorageBoundaryTest.kt`
- `tests/persistence.test.mjs`

## 10. Autonomous CI portability remediation (2026-10-04)

User-supplied Step 3 CI evidence identifies Foundation portable `windows-2025`, Foundation `apple-boundary`, and Native application shells Apple/package as FAIL; Ubuntu portable/sanitizer and Android Foundation were reported green. Apple CMake had already passed all seven native tests; its subsequent SwiftPM compilation failed. These supplied results are separate from execution observed below. No commit/push or Step 4 work was authorized or performed in this pass.

### Root causes and exact fixes

- MSVC C4244 under `/WX`: both SHA-256 padding `std::fill` calls deduced an `int` value from literal `0` while assigning to `uint8_t`. Both now pass `uint8_t{0}`. `/W4 /WX` and all owned warning policies remain intact. Empty, abc and million-a known vectors remain; an additional FIPS 56-byte vector verifies the two-block padding branch and expected digest `248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1`.
- MSVC storage-test C3530/C2513/C2062/C2144 and apparent zero-argument `verify`: the exact declaration was `auto small=store.ingest(...)`, followed by `small.digest` and `store.verify(small)`. Windows SDK RPC headers define `small` as `char`, producing `auto char`, `char.digest` and `verify(char)` after preprocessing. This is macro leakage, not nonstandard C++17 declaration syntax. Linux headers do not introduce it; the local Clang/MinGW RPC header confines that macro to `RC_INVOKED`, explaining why the fallback accepted the original. Rename to `abc_asset` throughout these scenarios; deduplication, hash mismatch, tamper/collision preservation and traversal assertions are unchanged. A separate strict Clang/MinGW object compilation explicitly enables `-Dsmall=char` and passes, verifying resilience to that SDK macro; `windows-macro.log`.
- AppleClang SwiftPM `-Werror,-Wambiguous-macro`: modular SDK `sys/param.h` and SQLite's internal amalgamation definitions both expose MIN/MAX. `check-apple.sh` passes `-Xcc -Werror` across package targets, unlike the already-green CMake path. The existing `CPixAuraSQLite` target now alone receives `-Wno-ambiguous-macro`, conditioned on macOS/iOS. Other diagnostics still obey `-Werror`; owned C/C++ and Swift warning policies remain strict. No amalgamation, version, module boundary or frozen architecture changed. Both workflows invoke `check-apple.sh`, and the iOS application also references the same local `packages/core` product. This one vendor-target fix therefore addresses both reported Apple/package failures, subject to actual Apple execution.
- Added a source regression verifying the exception remains in the SQLite target, is Apple-conditioned, preserves the strict Apple command and retains the application's local package reference. During local escalation the tool generated untracked `.codex/config.toml` without a final newline; the source walker initially rejected it. Ignore root `.codex/` in Git and exclude generated tool configuration alongside existing build/cache directories. Authored source hygiene assertions and every test remain enabled.

Reviewed Step 3 storage/SHA/API/schema/tests and platform build wiring for narrowing, same-declaration `auto` types, macro collisions, byte signedness, bounded size/ssize conversions, path capabilities and third-party flags. No further source change was justified. OS read/write conversions remain bounded by streaming/checkpoint limits, negative POSIX results are checked before unsigned conversion, and platform filesystem separation remains as frozen. This pass makes no architectural decision and requires no new ADR.

### Observed validation for this remediation

Logs are ignored under `build/phase-2-step-3/ci-remediation/`.

| Gate | Observed result |
| --- | --- |
| Windows source | PASS 33/33, zero skips; `node --test` foundation, ci-tools, shells, document-contract, native-document, persistence; `windows-source.log` |
| Windows native fallback | PASS, Zig 0.14.1 `check-native-zig.ps1`; core/document/C consumers, allocation failures, storage/crash/concurrency, SHA vectors and C storage reopen; `windows-native.log` |
| Linux source | PASS 62/62, zero skips; repository-local Node 24.14.0 `node --test tests/*.test.mjs`, including emulator orchestration; `linux-source.log` |
| Linux native | PASS Clang 21.1.8 CMake Debug/Ninja, CTest 7/7 including storage/document/C consumers; `linux-native.log` |
| ASan/UBSan | PASS `bash scripts/check-sanitizers.sh`, CTest 9/9, instrumented storage and both runtime-negative probes; `sanitizers.log` |
| Android builds/native/lint | PASS strict offline Gradle Debug/Release, unit gate, both lints, instrumentation APK; three native ABIs arm64-v8a/armeabi-v7a/x86_64; `android.log`. JVM two-test XML and unchanged lint tasks were reused/up-to-date, not newly executed assertions |
| Android instrumentation | PASS newly executed 3/3, failures/errors/skips 0; Android 16/API 36.1 x86_64 `pixaura-shell`, JNI document/storage and Compose shell tests; connected XML and `android.log` |
| Workflows / shell syntax | PASS actionlint 1.7.12 on both workflows; `bash -n` on every `scripts/*.sh`; Apple package/app/source wiring regression passes |
| Whitespace / vendor integrity | PASS `git diff --check` and source hygiene; SQLite pin checks pass in source/native configuration and fallback |
| Real MSVC | BLOCKED locally: attempted `check-native.ps1` with execution-policy bypass; missing `vcvarsall.bat`, desktop C++ installation incomplete; `msvc.log`. Owner host maintainer; repair workload or obtain hosted Windows CTest evidence; affects G1/G2/G3 |
| GCC | BLOCKED locally: attempted CMake GCC configure, neither `gcc` nor `g++` available in WSL PATH; `gcc.log`. Owner host/CI maintainer; execute existing Ubuntu portable job or install host compiler; no GCC PASS claimed |
| Apple runtime | CI-only: Windows/WSL cannot execute AppleClang SDK modules, Swift/Darwin/macOS tests or Xcode iOS package/app/simulator. Owner repository maintainer; rerun both workflows on corrected revision; affects G1/G2/G3/G5 |

Diagnostic attempts are not PASS: default PowerShell script policy blocked the first MSVC attempt before workload discovery; sandbox denied Windows temporary-ancestor access and ADB configuration; authorized retries ran successfully. Initial WSL shell quoting produced no logs, corrected literal script execution produced the recorded results. Initial Android retries had no device; launched only the existing repository-isolated hidden headless AVD, confirmed boot completion, then reran successfully. The initial generated-config hygiene failure was fixed as described above. GCC configuration unavailability is an environment limitation, not a compiler test result.

SQLite `sqlite3.c` SHA-256 remains exactly `b1dd5d74ec7f29055a6684fa06fb3c2f6821c87dd38f9a458dfd2e8a1db28189`; header remains `919e7f2e8ed1d8f56ac17b412b8971c76aa5d1a879752cc6058f75e7d5910e1d`. Both match reviewed provenance and neither has a Git diff.

This remediation changes seven files: `.gitignore`, `packages/core/Package.swift`, `packages/core/src/sha256.cpp`, `packages/core/tests/storage_test.cpp`, `tests/foundation.test.mjs`, `tests/persistence.test.mjs`, and this report. No commits, pushes, upstream-byte edits, test removals, global warning relaxations or Step 4 implementation occurred. Both red workflows are expected to be repaired for the supplied failures; that expectation is not a claim of CI success. Recommended next action: maintainer reviews this consolidated diff, then commits/pushes through an explicitly authorized process and reruns **Foundation boundaries** and **Native application shells** on the corrected SHA. Retain actual Windows/MSVC and Apple package/iOS run evidence before marking Step 3 fully passed; do not advance to Step 4.

## 11. CI remediation round 2: Darwin caller-path contract (2026-10-04)

This round starts from approved, published `b833eefafebc0b31a14cdf98ad494e741ef27c9c`. No commit/push is authorized for round 2 and none was performed. The initial working tree was clean. Production storage, SQLite bytes, warning policies, frozen architecture and Step 4 remain unchanged.

### Independently inspected CI provenance

Authenticated GitHub Actions REST metadata and complete downloaded job logs, retained only under ignored `build/phase-2-step-3/ci-round-2/`, establish these completed runs, both on exactly `b833eefafebc0b31a14cdf98ad494e741ef27c9c`:

| Workflow/run | Exact job inspected | Observed result |
| --- | --- | --- |
| [Foundation boundaries 37179388080](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37179388080) | Windows portable `111368670574` | PASS real MSVC build and CTest 7/7; checkout log confirms approved SHA |
| Same Foundation run | Apple boundary `111368670496` | FAIL Swift storage test after CMake native 7/7 and Swift compilation/CoreProbe/DocumentBoundary PASS |
| [Native application shells 37179388104](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37179388104) | iOS app `111368670672` | FAIL during prerequisite `check-apple.sh`, same macOS Swift storage test; later iOS device/simulator/app stages did not execute |
| Same two runs | Other jobs, metadata inspected | Foundation Ubuntu/sanitizer/workflow-lint/both Android boundaries, shells source/Android app PASS |

Both Apple logs identify Xcode 26.6 (17F113), Swift 6.3.3, arm64-apple-macosx26.0. Both failures are `StorageBoundaryTests.testNativeSchemaCreationAndReopen`, twice returning 6 and leaving zero-initialized info unchanged, with no catalog file. The earlier ambiguous MIN/MAX compilation failure is resolved. The supplied Windows log describing int-valued SHA padding and `small` corruption is **stale for the approved SHA**, contradicted by current Windows PASS and committed source: both fill calls use `uint8_t{0}` and the affected variable is `abc_asset`. No original raw stale-log attachment/run ID was provided, so its originating older run cannot be independently assigned. No Windows production-source remediation was made this round.

### Status, failure trace and root cause

`storage.h` explicitly uses document status codes; `document.h` defines integer 6 as **PIXAURA_DOCUMENT_INVALID_PROJECT**, not IO_ERROR (12) or SQLite result 6. `storage_api.cpp` constructs Repository from the exact bounded UTF-8 byte span and catches `document::Failure`, returning its code without touching caller output. Swift's buffer lives through the synchronous call, and native construction copies the root; no UTF-8 lifetime defect was found.

The incorrect caller expression was `Array(root.resolvingSymlinksInPath().path.utf8)`. Foundation's method is not equivalent to POSIX realpath on Darwin: [Apple documents](https://developer.apple.com/documentation/foundation/nsurl/resolvingsymlinksinpath) that an existing path beginning with `/private` can have that prefix stripped. Temporary `/private/var/...` roots are therefore presented again as `/var/...`, reintroducing the system symlink. The frozen storage contract requires existing canonical absolute private directories without symlink components. Native C++ tests already use `std::filesystem::canonical`, which explains why both Apple CMake suites passed while Swift failed.

The implicated native boundary is `Files::Impl` at `storage_files.cpp:148`: `open(root.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW_ANY | O_CLOEXEC)`, then `require(current.handle != invalid, 6)`. This runs while constructing Repository's AssetStore, before SQLite schema/open/WAL operations. Darwin's [XNU lookup implementation](https://raw.githubusercontent.com/apple-oss-distributions/xnu/main/bsd/vfs/vfs_lookup.c) rejects symlink traversal under NAMEI_NOFOLLOW_ANY with **ELOOP** (Darwin errno 62). This root-call attribution is a source/documentation diagnosis consistent with the CI observations; the original CI logs did **not** capture the pathname or underlying errno, so a measured original-run ELOOP cannot honestly be claimed. Retained Swift regressions now capture direct no-follow-open errno and immediate native-call errno, printing only numeric status/errno and no paths. The direct open regression requires ELOOP; C ABI errno is diagnostic observation, not a new output guarantee. Apple execution of these diagnostics still requires corrected-revision CI. SQLite result code is not applicable to this diagnosed failure because SQLite open has not been reached.

### Fix and focused regression coverage

The Swift test now calls POSIX `realpath` on its already-created caller-owned test directory in a detached task, copies the returned canonical bytes before freeing the allocated buffer, and sends those bytes directly to the C API. It does not pass the canonical result through Foundation path presentation again. Apple's [Libc realpath implementation](https://raw.githubusercontent.com/apple-oss-distributions/Libc/main/stdlib/FreeBSD/realpath.c) supports an allocated result when the output pointer is null. Failure reports only numeric errno. Native storage security and durability behavior are preserved; this corrects the test's caller contract rather than accepting symlink roots in production.

- Swift success probe still creates/reopens twice and checks schema 1/runtime 3053004, now also API/layout outputs, actual catalog existence and a root containing spaces, percent/hash and UTF-8 characters. Any differing original Foundation alias is checked for status 6 and preserved sentinels before creation; controlled alias regressions run regardless of temporary-root presentation.
- Swift negative tests verify all four caller sentinel fields survive relative/traversal/nonexistent/file-root/symlink/unwritable failures; controlled leaf and ancestor aliases are rejected, direct Darwin no-follow open captures errno, unwritable creation returns IO_ERROR and creates no catalog. Only fixed numeric diagnostics appear in test output; no production logging was added. Detached results contain explicit Sendable scalar metadata.
- Native storage test reproduces nested `private/var/folders/.../T/project #%` paths with spaces, verifies schema creation/reopen/catalog/runtime, rejects traversal/file roots and an intermediate `var` symlink or Windows NTFS junction. POSIX unwritable-directory regression executes with ordinary permissions; when WSL runs as root it uses an unprivileged child, rather than skipping the permission test. Observed Linux failure is mkdirat/EACCES (errno 13), IO_ERROR 12; intermediate alias rejection is INVALID_PROJECT 6 with ENOTDIR (errno 20), matching Linux's different openat flags.
- Independent C consumer verifies the actual SQLite file header, rejects a database file used as the root, rejects missing/traversal paths and preserves every caller output byte. Existing platform-specific status mapping remains: missing Windows root is IO_ERROR 12, missing POSIX root is INVALID_PROJECT 6.

Audited Darwin directory opens/sync, O_DIRECTORY/O_NOFOLLOW_ANY and child O_NOFOLLOW, exclusive temporary creation, read-only assets, F_FULLFSYNC file barriers, same-directory atomic rename and post-publication directory fsync, path validation, and fixed SQLite sidecar checks. Existing Apple CMake 7/7 executed these native durability/crash tests successfully on the approved SHA. No evidence justifies changing those primitives. SQLite still receives a directory-derived fixed catalog path, READWRITE/CREATE/FULLMUTEX/PRIVATECACHE/NOFOLLOW, without URI interpretation; WAL/journal work follows root admission. Canonicalization remains caller-owned; protection/backup policy remains platform-owned. No new architecture or dependency requires an ADR.

### Local validation of round 2

| Gate | Observed execution |
| --- | --- |
| Windows source | PASS 33/33, no skips; six compatible Node suites; `windows-source.log` |
| Windows native fallback | PASS Zig 0.14.1 `check-native-zig.ps1`, storage/path/crash/concurrency, document/allocation tests and independent C consumers; `windows-native.log` |
| Linux source | PASS 62/62, no skips; Node 24.14.0 `node --test tests/*.test.mjs`; `linux-source.log` |
| Linux native | PASS Clang 21.1.8 Debug CMake/Ninja and CTest 7/7; new storage/path/C consumers executed; `linux-native.log` |
| ASan/UBSan | PASS `bash scripts/check-sanitizers.sh`, CTest 9/9 including both actual runtime-negative probes and new path/permission tests; `sanitizers.log` |
| Android | PASS strict offline Gradle Debug/Release/build/unit/lint/instrumentation APK gates; native three-ABI tasks and unchanged JVM/lint evidence reused/up-to-date. Fresh connected instrumentation 3/3, zero failures/errors/skips on Android 16/API 36.1 x86_64; `android.log` and connected XML |
| Workflows / syntax / hygiene | PASS actionlint 1.7.12 both workflows; every `scripts/*.sh` passes `bash -n`; source/wiring checks and `git diff --check` pass |
| Real MSVC / Apple locally | No desktop MSVC installation repair or Apple runtime exists locally. Real MSVC PASS is independently observed CI evidence for baseline b833eef; the added test compilation on real MSVC and modified Swift/Darwin/iOS execution remain corrected-revision CI-only |

GitHub CLI was unavailable; unauthenticated Actions API returned 404 for the private repository. The existing Git credential was used only in memory for authenticated read-only API calls, never printed or stored in evidence. No failed access attempt is counted as validation. Build/cache/log artifacts remain ignored; staged index remains empty.

Round 2 changes exactly four files: this report, `packages/core/swift/Tests/StorageBoundaryTests.swift`, `packages/core/tests/storage_test.cpp`, and `packages/core/tests/storage_c_consumer.c`. `sqlite3.c` SHA-256 remains `b1dd5d74ec7f29055a6684fa06fb3c2f6821c87dd38f9a458dfd2e8a1db28189`, byte-identical to approved provenance and baseline with no vendor diff. No test was removed, no quality gate weakened, no commit/push performed and no Step 4 begun.

Remaining CI-only G1/G2/G3/G5 owner: repository maintainer. Recommended next action: review these four files, explicitly authorize a new commit/push if accepted, then run both workflows on that corrected SHA. Require actual Swift schema creation/reopen and numeric Darwin alias diagnostics, followed by the previously unreached iOS package/device/simulator/app gates, before claiming full Step 3 completion. The original-run errno was unavailable; this report does not substitute inference for measured Apple runtime evidence.

## 12. Authorized autonomous CI closeout (2026-10-04)

The Product Owner explicitly authorizes repository-local remediation, focused commits, push to the existing approved origin/main, and observation of both workflows until closure. Starting HEAD is `b833eefafebc0b31a14cdf98ad494e741ef27c9c`. Audit found exactly the four reviewed Round 2 files plus intentionally updated `AGENTS.md` and new `scripts/codex-git.ps1`; no unrelated untracked artifacts or staged files. Generated builds, caches, databases and evidence are ignored and excluded from commits. The infrastructure files both lacked a final newline; the source hygiene gate detected this and both were corrected without changing their instructions or behavior.

Acceptance requires both **Foundation boundaries** and **Native application shells** fully successful on the latest pushed SHA, including actual hosted MSVC and Apple package/device/simulator/app execution. The approved realpath caller fix, native protections, SQLite bytes, ABI 1 and frozen contracts remain intact. No Step 4 work is authorized or implemented.

Fresh closeout evidence is retained under ignored `build/phase-2-step-3/closeout/`. Windows source tests pass 33/33 with zero skips; the Zig 0.14.1 native fallback passes all native consumers, allocation, storage/crash/concurrency and C storage boundary checks. Linux source passes 62/62 with zero skips; Clang Debug CTest passes 7/7; ASan/UBSan CTest passes 9/9 including both actual runtime instrumentation probes. Android strict offline Gradle Debug/Release, both lints, JVM test tasks and instrumentation APK succeed (147 tasks: 17 executed, 130 up-to-date); fresh connected Android 16 instrumentation passes 3/3 with zero failures/errors/skips. JVM test results are reused/up-to-date, with 2/2 passing in the retained XML. Local MSVC preflight confirms the existing missing `vcvarsall.bat` installation limitation; hosted MSVC remains mandatory. actionlint 1.7.12 passes both workflows with unavailable local shellcheck/pyflakes integrations disabled; every scripts/*.sh passes bash syntax validation in WSL. Published CI evidence will be recorded when observable.

Moving native temporary roots from Linux /tmp to the Windows-drive repository exposed an environment failure in the unwritable-directory child test: WSL DrvFS is mounted without POSIX metadata and cannot reproduce the required chmod behavior. Native and sanitizer gates were rerun successfully with TMPDIR at a private tmpfs mount beneath the repository's ignored build directory, using an isolated mount namespace that disappears on process exit. No production code, test expectation, system file or mount configuration changed. This run verifies process interruption and POSIX filesystem behavior, not physical power-loss durability. Windows native temporary roots and Android/Gradle caches and temporary directories were explicitly redirected beneath the repository for final execution.

The SQLite amalgamation SHA-256 is exactly `b1dd5d74ec7f29055a6684fa06fb3c2f6821c87dd38f9a458dfd2e8a1db28189`; no vendor source modification. Apple runtime remains unexecuted locally. Initial sandbox denials for Node subprocesses and WSL are diagnostic attempts, not failed product gates. Automatic approval review rejected the proposed existing-credential GitHub REST authentication method; explicit authorization for that method is pending while unaffected work continues. No credential is printed or stored.

## 13. Resumed autonomous CI closeout (2026-10-04)

Starting HEAD: `8343fe5e826cfbc3f5e58d8a9a61bc3ccae3158c`. The owner authorizes scoped fixes, commits, main pushes and repeated observation until both workflows pass. `scripts/codex-gh.ps1 run-list` selected Foundation run [37182772694](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37182772694) and shells run [37182772730](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37182772730). Failed logs retrieved through the wrapper confirm that exact full SHA in checkout for Windows, Apple, source and iOS jobs. Evidence is ignored under `build/phase-2-step-3/resume/`. No credential was read, exported or printed; the approved GitHub CLI wrapper supersedes the previous blocked REST access method.

Current failures and narrow fixes:

- MSVC C4996/C2220 rejects `fopen` in the independent C storage consumer. Use checked `fopen_s` only for MSVC, retaining portable `fopen` elsewhere and every header/status/sentinel assertion. Warnings-as-errors remain enabled.
- macOS native CTest and Swift tests pass, including both storage tests, but iOS simulator package linking reports undefined SQLite symbols. Xcode compiles SQLite, then partially links each package C target with `ld -r`, which localizes hidden external definitions by default. Add `-keep_private_externs` to the SQLite target's Apple linker settings to retain those definitions until final linkage, preserving hidden visibility, the reviewed vendored bytes and all SQLite compile/security settings. Actual Xcode confirmation is pending the pushed run.
- The early-exit fake emulator exits 23 but its fake ADB sometimes reports readiness before the process exit is observed, advancing the phase to boot-completion. Make fake ADB return offline in this scenario. The existing exact device-visibility assertion, failure status, retained stderr and no-instrumentation checks remain intact; production orchestration is unchanged.
- The owner-supplied AGENTS addition contained trailing whitespace, corrected without altering instructions. Commit the supplied read-only Actions wrapper with repository-confined log caching: GitHub CLI originally attempted its default outside-repository cache and received access denied. The wrapper now temporarily redirects LOCALAPPDATA for invocation and restores it afterward; authentication configuration is unchanged. A fresh failed-log retrieval through the corrected wrapper succeeded.

Executed local validation: Windows source 33/33, Linux source 62/62, zero failures/skips; Windows Zig native consumers/document allocation/storage/crash/concurrency and C schema/reopen PASS; Linux Clang Debug CTest 7/7; ASan/UBSan CTest 9/9 including actual negative instrumentation probes. Linux native/sanitizer temporary roots use the previously documented isolated repository tmpfs namespace. Android strict offline Debug/Release, unit tasks, both lints and instrumentation APK succeed (147 tasks, 15 executed, 132 up-to-date); newly executed connected API 36.1 instrumentation 3/3 with zero failures/errors/skips. Unchanged JVM assertions are reused/up-to-date. actionlint passes both workflows and every shell script passes syntax checks. Local MSVC was attempted and remains BLOCKED by missing vcvarsall.bat; Apple execution is CI-only. Initial sandbox subprocess/WSL denials and corrected WSL quoting are diagnostics, not passes. SQLite integrity checks pass with unchanged reviewed hashes.

Changed files: `AGENTS.md`, `scripts/codex-gh.ps1`, `packages/core/Package.swift`, `packages/core/tests/storage_c_consumer.c`, `tests/android-emulator.test.mjs`, `tests/persistence.test.mjs`, and this report. No architecture conflict, durability/security relaxation, vendor modification or Step 4 work. FULL PASS remains pending successful current-revision Foundation and Native application shells runs.

### Follow-up: Xcode partial-link setting

Commit `e220010d518ef0000e0fbdbae472051de6694698` was pushed through the Git wrapper. Foundation [37189981931](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37189981931) confirms hosted MSVC PASS and all non-Apple Foundation jobs PASS; shells [37189981936](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37189981936) confirms source PASS. Apple still reports the same undefined SQLite symbols. Fresh failed logs show SwiftPM's linkerSettings flag reaches the final XCTest bundle link but is absent from the earlier SQLite `ld -r` invocation. The first linker fix was insufficient and is replaced, not claimed successful.

Use Xcode's documented `KEEP_PRIVATE_EXTERNS=YES` build setting on every package/app device and simulator invocation in both Apple scripts; [Apple's build setting reference](https://developer.apple.com/library/archive/documentation/DeveloperTools/Reference/XcodeBuildSettingRef/1-Build_Setting_Reference/build_setting_ref.html) specifies preservation of private external symbols instead of localization. Remove the ineffective Package.swift linkerSettings, restoring the original manifest. This retains SQLite hidden visibility and applies at Xcode's intermediate target link stage. Source regression checks require this setting on all four build/test invocations. No vendor, warning, architecture, platform test or security policy changes.
