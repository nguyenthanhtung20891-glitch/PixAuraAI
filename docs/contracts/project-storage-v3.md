# Project storage schema 3

Authority: Phase 3 Step 4 owner authorization and ADR 0020. Immutable schemas
1/2 retain independent exact SQL/fingerprints, payload meanings and read/open
paths. Schema 3 adds only INTEGER blur/sharpen columns, each 0..1000, exact /1/1
types and exactly one non-NULL parameter (crop retains its four-column tuple).
STRICT storage, all old bounds, assets/history/IDs/sequence/redo/current/session/
generation/epoch/actor/plan and immutable source identity remain unchanged.
No JSON/blob parameters, plugins, unknown tools or future versions are admitted.

Fresh storage stays v1. Ordinary open/read/write never migrates. Owners explicitly
request 1->2 then 2->3 through Repository::migrate or pixaura_storage_migrate.
Direct 1->3 rejects (4); unsupported pairs/future versions reject. Verified
same-version calls are no-ops; a repeated 2->3 against v3 rejects without mutation.

2->3: BEGIN IMMEDIATE, validate exact source app ID/version/fingerprint, integrity,
native bounded state and assets; connection-private copy of all 18 operation
columns; replace only operations with exact schema-3 SQL; restore all old values
unchanged, with new fields NULL; restore immutable triggers; verify independent
v3 fingerprint, FKs, native/canonical replay and complete envelope equivalence;
set user_version=3 only after verification; COMMIT. Any pre-commit failure or
process interruption rolls back to prior logical v2. Post-commit recovery is v3.
Physical WAL/database byte identity is not promised. No IDs regenerate, operations
normalize, source writes, checkpoint authority or history compaction occurs.

Resource/IO/BUSY/corruption mappings and bounded outputs remain unchanged. Required
tests include empty/populated v2, all old operation payloads/order, branches/redo,
reopen/replay, fingerprint tamper/wrong app/future version, strict tuple rejection,
allocation/SQLite faults and actual child-process crash recovery. See Step 4 report
for observed validation; this contract itself makes no PASS claim.
