# Project storage schema 2

Authority: explicit Phase 3 Step 3 Architect decision and [ADR 0019](../adr/0019-explicit-storage-schema-evolution.md).
Schema 1 remains immutable historical SQL, with its independent exact fingerprint.
Manifest schema, C API layout/version and old operation versions remain 1.

## Exact typed representation

`storage_schema_v2.hpp` is a separate complete canonical SQL definition.
Fingerprint computation remains the ordered length-prefixed SQL catalog digest;
v1/v2 expectations are computed independently. Exact application ID/version/
fingerprint, quick_check, foreign keys, native document and asset verification
precede trusted use. Unknown future versions reject.

All v1 tables/columns/triggers are identical except operations' added capability
and user_version=2. The operations table adds six INTEGER columns. Its type CHECK
admits exactly exposure/crop/rotate and the following six /1/1 tuples:

| Type | Required new column | Inclusive bounds | Canonical integer parameter |
| --- | --- | --- | --- |
| brightness | brightness | -1000..1000 | milli_linear |
| contrast | contrast | -2000..2000 | milli_stops |
| highlights | highlights | -2000..2000 | milli_ev |
| shadows | shadows | -2000..2000 | milli_ev |
| saturation | saturation | 0..2000 | milli_ratio |
| temperature | temperature | 4000..25000 | kelvin |

The type-specific column must be non-NULL and satisfy its bound CHECK. Exactly
one parameter column must be non-NULL, except crop's exact four columns. Thus
NULL/incorrect-column/mixed tuples cannot exploit nullable SQL CHECK semantics.
STRICT typing, version constraints and bound parameters remain; no blob, generic
JSON, future tools, dynamic schema or plugin storage. Old payloads are unchanged.

## Explicit evolution and compatibility

Ordinary open accepts exact existing v1/v2 at its recorded version and never
migrates. Fresh catalogs remain v1. New tone persistence on v1 rejects (4),
preserving state. Upgrade requires Repository::migrate(1,2) or background-only
pixaura_storage_migrate(1,root,...,1,2,&info); platforms do not implement SQL.

BEGIN IMMEDIATE -> exact source verification -> bounded private temporary copy
of operations -> canonical table replacement and exact row copy -> immutable
triggers -> exact v2 fingerprint/FK/native/asset/canonical-envelope verification
-> user_version=2 -> COMMIT. No other table is modified. Asset/source identity,
IDs/sequence, full stacks/revisions/branches/redo/current, session/generation/
epoch and actor/plan remain identical. Canonical replay is unchanged.

SQLite transactional DDL and WAL/FULL provide rollback and atomic publication,
as explicitly approved for this pair. Failure/process termination before COMMIT
restores prior logical v1; physical WAL/database bytes need not be identical.
No checkpoint is trusted or generated during migration. Existing sync/private-
root and all asset/history/raster/context ceilings remain unchanged.

1->2 succeeds once. Repeating with expected version 1 against v2 returns 4 without
mutation. Explicit 1->1/2->2 calls verify and do nothing. Unsupported pairs,
future versions and wrong application IDs return 4; schema/data corruption 6;
existing BUSY=15/resource=8/IO=12 mappings apply. C outputs change only on success.

## Verification

Compare independent exact SQL catalogs and frozen v1 source hash. Test empty/
populated v1, each old payload, mixed order, branches/redo, reopen and canonical
replay/envelope equivalence. Reject invalid SQL shapes/types/versions/parameters.
Allocation and simulated SQLite return failures after DDL must roll back; actual
child-process termination tests cover copy/schema/validation/pre-commit points.
New tone commits must persist and replay through schema 2. See the Step 3 report
for executed platform/hosted evidence; this contract makes no PASS claim.
