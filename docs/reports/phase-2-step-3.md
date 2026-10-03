# Phase 2 Step 3: durable persistence and immutable asset storage

Status: IMPLEMENTED; local validation completed, real MSVC/Apple and corrected-revision CI gates pending. 2026-10-04. No commit/push; Step 4 remains unauthorized. The user attests Foundation boundaries and Native application shells fully green for approved Step 2 revision `f78ba096872249bf80ff1e4ee9e51a0a4ecfb6ef`. That attestation authorizes this step; it is not an independently observed Apple/MSVC execution for this working tree.

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
