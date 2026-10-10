# Project storage contract, version 1

Phase 3 Step 3 compatibility extension: this v1 SQL/fingerprint and all old
payload meanings remain immutable. [Schema 2](project-storage-v2.md) and
[ADR 0019](../adr/0019-explicit-storage-schema-evolution.md) add an explicitly
approved transactional 1->2 migration and exact v1/v2 open support. Ordinary
open never upgrades. The historical v1-only implementation description below
is superseded only where those documents explicitly describe the new capability.

Status: Phase 2 Step 3 implementation contract, 2026-10-04. [ADR 0010](../adr/0010-durable-native-project-storage.md) refines filesystem/journal choices without changing [document schema 1](image-document-v1.md), ADR 0008 or ADR 0009.

## Frozen versus selected

Already frozen: original bytes immutable; verified SHA-256 addressing; a single source/document per project; ordered complete stacks; retained single-parent branches; SQLite live authority; canonical JSON checkpoint/interchange only; explicit migrations; asset-before-reference; fresh session/generation plus durable epochs; native semantics and checked context C handles. Selected here: one normalized DB per private root, direct pinned SQLite, WAL/FULL, bounded writer admission, OS no-overwrite file publication, recovery by rejection/ignore and internal service ownership. No decoding, metadata extraction, image processing or Step 4 implementation.

## Service and ownership

`storage.hpp` defines background-only C++ `AssetStore` and `Repository`. Caller-created roots are existing canonical absolute OS-private **local** directories, stable during service lifetime, without terminal separators or symlink/reparse components. Callers own backup exclusion/data protection. Linux/Android retain search-only O_PATH ancestor capabilities and a readable owned leaf; Darwin uses O_NOFOLLOW_ANY root opening. Source filenames are bounded optional metadata, never path inputs and currently not persisted because schema 1 has no filename field. Ingest reads from caller-owned Reader until EOF, verifies exact expected positive byte length <=8 GiB, and optionally checks an expected digest. Reader returns 0 at EOF, never more than capacity, and throws IO_ERROR on failure. A 64 KiB fixed buffer is the sole photo-content buffer. No whole asset is loaded into memory.

Internal methods may throw typed document::Failure (the existing statuses, plus BUSY=15) or allocation failures; application services must preserve their active snapshot on failure. `Saved` separates a successful DB epoch from checkpoint publication. Fresh session acquisition is explicit; pure read restores the persisted current envelope. A candidate is never persisted by merely parsing it: apply delegates to the Step 2 command transition and the caller supplies approval. One Repository serializes its calls; independent connections use SQLite isolation and epoch comparison. Destruction requires exclusive access or retained shared ownership until work ends. There is no process-global application registry or new persistence handle.

`storage.h` adds only a synchronous checked schema-capability entry point, `pixaura_storage_check`, with a 16-byte caller-owned info result. It creates a fresh schema or checks the existing current schema, never migrates, and exposes no SQLite handle, pointer ownership or photo data. Failure leaves output untouched. This is the current integration seam; platforms do not implement persistence semantics. ABI 1/version probe and document family 1 are unchanged.

## SQLite representation and integrity

| Table | Authority and constraints |
| --- | --- |
| assets | SHA-256 primary identity and positive verified length; immutable rows. Source/ICC references require published files before insertion. |
| documents | Unique project/document identity and singleton, manifest version, source/ICC FKs, validated dimension/orientation/codec/alpha metadata, current revision FK (deferred within commit), fresh session/generation, durable bounded epoch. |
| operations | Immutable (document,id), unique bounded append order, registered type/version, typed integer parameter columns with exact mutually exclusive parameter shape/ranges. Shared across revision stacks. |
| revisions | Immutable (document,id), unique bounded append order, earlier single parent FK, exactly one import root, manual/AI actor-plan consistency. Parent-order trigger excludes dangling/cyclic insertions. |
| stacks | Immutable full ordered stack per revision; bounded unique positions/operation references and FKs. No duplicate operation in a stack. |
| redo | Mutable navigation path, unique bounded position/revision with FKs; stored in the same transaction as current/epoch. |

No timestamps are semantically necessary; IDs/order/epoch determine behavior. No serialized document blob, duplicate current stack, duplicated source size in documents or competing checkpoint catalog is stored. Required duplication of operation IDs across complete stacks follows the frozen contract, not a convenience cache.

STRICT tables, CHECK/UNIQUE/FKs and immutability triggers enforce local invariants. The native bounded validator additionally enforces complete reference use, empty root stack, redo contiguity, all history/operation semantics and canonical output limits. Loader checks contiguous SQL position/sequence indices. Reopen compares the actual SQL schema fingerprint with the compiled expected schema, checks application_id/user_version, runs quick_check/foreign_key_check, counts bounded rows, reconstructs normalized data in one read transaction through SQLite JSON encoding, parses via the Step 2 core, restores the envelope, and verifies referenced bytes. Missing/tampered assets, manipulated schema, malformed rows or future versions reject the whole state; no reset/quarantine rewrite/deletion occurs.

Bound SQL statements receive every identity/parameter. SQLite extensions/shared cache/DQS are disabled. Per-connection limits: metadata/value <=8 MiB, SQL <=64 KiB, expression depth 64, columns 128, variables 64, attached DBs 0, 2 MiB page-cache target, 20 million VM-step budget per guarded verification/write admission. Catalog page budget is 256 MiB; all schema-1 entity/stack/redo bounds remain. Source data uses checked 64-bit sizes and fixed buffers. SQLite limits/cache targets are not a certified whole-process RSS cap; sanitizer/adversarial tests and future device admission remain mandatory.

## Transactions and durability

Connections enforce WAL, FULL, foreign_keys=ON, fullfsync/checkpoint_fullfsync and 250 ms busy timeout. Writes use BEGIN IMMEDIATE. Fresh schema creation also uses WAL/FULL before DDL. Automatic WAL checkpointing is 1000 pages with a 16 MiB recycled-file target. A physical WAL >32 MiB requires a bounded successful truncate before the next write; busy readers return BUSY. No committed WAL is discarded. Metadata/file limits remain separate from original-asset bounds.

Create validates the native snapshot and published source/profile, inserts asset descriptors, normalized history and initial pointer/redo/envelope in one transaction. Interchange admission can retain a complete already-approved schema-1 history; root-only native create remains unchanged. Append/parameter replacement passes through native immutable transition then inserts only fresh tail operations/revisions/stacks. Undo/redo/checkout change navigation/envelope only. A branch clears redo but retains every old row. Expected epoch, session, generation and base revision prevent stale/ABA/cross-session publication. Epoch exhaustion rejects without wrapping. Active application state is swapped only after Saved returns a committed epoch.

Asset order is stream -> exclusive temp -> SHA/length verification -> file sync/close -> no-overwrite digest publication -> directory durability where supported -> destination verification -> SQLite references. Failure before DB commit leaves no broken reference; a fully published orphan is allowed. Existing digest objects must hash-match, never overwrite. POSIX file/directory capabilities reject symlinks and asset hard-link aliases; Windows directory pins and OPEN_REPARSE_POINT checks reject reparse objects. Checkpoint namespace replacement never follows its old destination. SQLite fixed paths/sidecars reject links on open and NOFOLLOW is enabled. Caller-private, stable namespace ownership is essential to prevent SQLite path TOCTOU; the library does not claim to isolate a compromised same-UID writer or certify network filesystems.

Checkpoint generation follows DB commit and holds a writer reservation while reading an immutable snapshot, canonicalizing/reparsing it, streaming its bounded metadata to a unique temp, syncing and atomically replacing checkpoint.json. Other readers remain possible. DB commit success survives any checkpoint failure; Saved carries false and callers can retry checkpoint. Reopen ignores checkpoints and uses committed SQLite. Checkpoint bytes can represent an older committed snapshot after an interruption; they cannot override DB authority.

## Recovery and migration

| Interrupted phase | Allowed reopen state |
| --- | --- |
| Before/during staging | Prior DB; incomplete temp ignored, never trusted/promoted. |
| After write/before asset publication | Prior DB; complete but untrusted staging bytes ignored. |
| After durable asset publication/before DB | Prior DB; verified orphan may remain. |
| Inside SQLite transaction | Previous complete DB/history/pointer; SQLite rolls back or ignores uncommitted WAL. |
| After SQLite commit/before checkpoint | New complete DB; prior/missing checkpoint harmless. |
| During checkpoint temp/rename/sync | New complete DB; old/new/missing checkpoint harmless; temp untrusted. |

Recovery never rewrites an original, implicitly restores a checkpoint, promotes a temp, removes retained history or performs migration. Orphan/temp GC is deferred; normal failed calls best-effort remove only their own staging name. Schema storage version is discoverable through the checked info/Repository version and PRAGMA user_version, independently of manifest/ABI versions. Versions other than 1 reject on ordinary open. Explicit migrate(1,1) verifies transactionally and leaves representation/epoch unchanged; injected failure rolls back. Unsupported transforms reject. There are no invented legacy migrations. A future real transform must satisfy the frozen temporary-copy/backup/validation/atomic publication contract before enabling a new source/target pair.

## Verification

`storage_test` exercises database/asset/failure/concurrency contracts and forks/launches nine children that exit without destructors at durability boundaries. `storage_c_consumer` independently compiles as C and checks creation/reopen/layout/error sentinels. JNI instrumentation and Swift package tests call the same checked native seam. Existing document allocation sweeps, C consumers and real ASan/UBSan instrumentation probes remain. Exact executed evidence, platform limitations and dependency review are in the [Step 3 report](../reports/phase-2-step-3.md).
