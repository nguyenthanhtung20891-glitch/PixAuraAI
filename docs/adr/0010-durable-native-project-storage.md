# ADR 0010: Durable native project storage

Status: Accepted
Date: 2026-10-04
Scope: authorized Phase 2 Step 3 implementation choices; execution gates tracked separately

## Context

ADRs 0002/0008/0009 freeze SQLite authority, immutable originals, complete revision stacks, retained branches, explicit sessions/contexts, asset-before-reference, canonical checkpoints and explicit migrations. The user authorizes shared native persistence before decoding and forbids Step 4. Remaining choices are normalization, journal policy, file publication, dependency pin and service ownership. No frozen document decision is superseded.

## Decision

Use direct shared C++17 services with the verified official SQLite 3.53.4 amalgamation, without an ORM or system-version drift. One project root owns one database/document and deduplicates assets locally. Store normalized assets, document/source/envelope, append-only operations/revisions/stacks, and mutable redo rows. SQLite constraints plus the existing native bounded validator enforce structure and semantics. No authoritative JSON blob or duplicate current stack is stored. SQL values are bound parameters. The database schema fingerprint is derived from compiled schema SQL and compared before use; `quick_check`, `foreign_key_check` and native validation precede trusted reopen.

Use WAL, `synchronous=FULL`, foreign keys ON, 250 ms busy timeout and `BEGIN IMMEDIATE` writes. WAL allows readers during writes; FULL commits sync the WAL. Rollback DELETE/EXTRA would also be durable but would block the reader/writer use case. `fullfsync`/`checkpoint_fullfsync` request Apple's stronger barrier. Automatic WAL checkpointing uses 1000 pages and a 16 MiB recycled-journal target. Before writes, WAL above 32 MiB requires a bounded truncate checkpoint; a pinned reader causes BUSY instead of continuing unbounded growth. Database pages are 4096 bytes, maximum 65536 pages (256 MiB). These ceilings fail, never prune history. A single bounded transaction can temporarily exceed the WAL admission threshold.

Every accepted transition has an expected durable epoch. In one transaction insert fresh operation/revision/stack rows, move current/redo, persist session/generation and increment epoch. A fresh application session explicitly resets generation and increments epoch, invalidating older writers. Pure readers restore the current persisted envelope. Approval remains an application-service obligation; this service never grants approval based on actor strings.

Content identity is SHA-256 already frozen in ADR 0008. Stream at most 64 KiB per read, with expected positive size <=8 GiB. Stage under the private root using OS randomness plus exclusive creation. Sync/close complete bytes, publish the digest basename without replacing existing content, and verify hash/length. POSIX uses `openat`/`O_NOFOLLOW`, read-only permissions, `linkat` without overwrite and directory fsync. Darwin asset/checkpoint file sync uses F_FULLFSYNC. Windows pins directory ancestors against rename, rejects reparse points, flushes files, then uses same-volume MoveFileEx with WRITE_THROUGH and no asset replacement. Existing digest objects must reverify, including deduplication. Asset bytes never change in place.

The caller provides an existing canonical absolute OS-private local root and keeps its namespace stable during service lifetime. Linux/Android use O_PATH for intermediate directories, preserving NOFOLLOW without requiring permission to list OS ancestors; the owned leaf is opened readably for directory sync. Darwin opens the canonical root using O_NOFOLLOW_ANY, then retains directory capabilities. Android owns no-backup/data-protection policy; Apple callers must set protection/backup exclusion when creating production roots. SQLite's ordinary VFS resolves fixed database/sidecar paths after link checks and NOFOLLOW; hostile same-UID replacement of the private namespace is outside the OS-sandbox threat boundary. Asset IO itself uses pinned capabilities. No claim protects against a compromised process that can chmod/write app-private files; reopen deterministically detects content tampering. Unsupported filesystem sync/publication errors reject instead of claiming durability. Flush APIs cannot certify a lying device/cache or simulate power loss.

After DB commit, derive canonical JSON from a locked immutable DB snapshot, reparse/reproduce its bytes, sync a unique temporary checkpoint and atomically replace only checkpoint.json. Hold SQLite's writer reservation during checkpoint derivation/publication to prevent competing publications from regressing it. A failure returns the committed epoch with `checkpoint_published=false`; it never reports the durable document as rolled back. Reopen always uses DB state, never stale checkpoint fallback. Checkpoints are LF schema-1 interchange, without session or epoch.

Storage schema 1 and manifest schema 1 are distinct. Ordinary open initializes a genuinely empty unversioned database only; an existing version other than 1 rejects, without migration. Explicit migration admits a transactional verified 1->1 no-op, rejects all invented historical/future paths and fault-tests rollback. There is no historical representation to transform. Future real transforms must implement the already-frozen temporary-copy/backup/validation/atomic-publication protocol; this step neither invents one nor performs in-place upgrades.

## Alternatives

System SQLite introduces different feature/security versions on hosts/mobile. Platform persistence implementations duplicate transaction semantics. A JSON-only catalog contradicts the frozen live authority. Overwrite-by-rename for assets permits corrupt collisions. Sync=NORMAL sacrifices power-loss durability. Automatic migration or checkpoint restoration silently changes history. Automatic orphan deletion requires ownership/retention reconciliation beyond this step.

## Consequences

The new dependency adds a measured native binary footprint and source review surface. Reopen/write verification rehashes referenced originals, trading latency for integrity; calls are background-only. Caller-supplied metadata is validated but encoded dimensions/codec are not decoded in this milestone. OS/filesystem durability guarantees and actual MSVC/Darwin behavior need CI/device evidence. Failed imports may leave immutable orphans; crashed temps remain untrusted and are ignored, not promoted. No automatic cleanup/job system, registry singleton, pixel pipeline or new persistence handles are introduced. A checked one-shot storage capability API creates/reopens the schema without exposing SQLite handles. Internal Repository ownership is exclusive at destruction, or retained with shared ownership while work runs.

## Validation

See the [storage contract](../contracts/project-storage-v1.md) and [Step 3 report](../reports/phase-2-step-3.md). Native tests cover normalized reopen/branches/current/redo, constraints/corruption, version/migration admission, rollback/BUSY/WAL budget, known SHA-256 vectors, streamed ingestion/dedup/tamper/traversal/link checks, concurrency and nine actual abrupt-process crash points. Existing C/C++ ABI, allocation sweep and ASan/UBSan probes remain. Actual MSVC Debug, macOS CTest/Swift, iOS simulator and Android JNI execution are wired into existing workflows; wiring alone is not PASS.
