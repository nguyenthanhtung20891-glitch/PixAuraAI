# ADR 0019: Explicit storage schema evolution

Status: Accepted Product Owner / Architect decision, 2026-10-10.
Scope: Phase 3 Step 3, Category A; no new roadmap step.

## Context

Schema 1's exact fingerprint and constraints admit only exposure/crop/rotate.
The six authorized Step 3 tools need durable typed storage. The Architect
explicitly resolves this conflict with schema 2 and a transactional 1->2 pair.

## Decision

Schema 1 is immutable historical SQL, including its exact fingerprint and old
operation payload meanings. Schema 2 has an independent canonical SQL definition
and fingerprint. It extends only typed operation storage for brightness, contrast,
highlights, shadows, saturation and temperature. Exposure/crop/rotate 1/1,
manifest schema 1, source identity, revision/history and resource ceilings remain.
Unknown types/versions and invalid mixed parameter tuples reject. There is no
generic parameter blob, plugin schema or capability for future tools.

Ordinary open accepts exact supported v1/v2 catalogs at their recorded version.
Fresh catalogs still initialize as v1; schema 2 requires explicit migrate(1,2).
There is no automatic migration, including on read, session acquisition or save.
Schema-1 writes retain their old capability; new tools need explicit upgrade.

## Explicit migration and durability

The Architect specifically authorizes a bounded SQLite BEGIN IMMEDIATE transform
for this pair. This supersedes the temporary-file/backup/publication requirement
of ADR 0010 and the document migration contract **only for storage 1->2**.
SQLite's transactional DDL and WAL/FULL journal provide the rollback copy and
atomic commit; replacing the whole catalog or discarding a WAL is unnecessary.
Other future pairs remain unsupported and gain no authorization from this ADR.

Under the writer transaction, verify source application ID, version and exact
v1 fingerprint, then validate the complete bounded native document and assets.
Copy old operation rows into a connection-private temporary table, replace only
the operations table with canonical v2 SQL, restore rows without changing any
value, and recreate immutable-operation triggers. Deferred foreign keys must
pass verification. Check the independent v2 fingerprint before setting version 2,
then validate the complete resulting state and compare canonical document,
session/generation and epoch to their prior values. COMMIT publishes all changes.

Assets, documents, revisions, stacks, redo, current revision, IDs, sequence,
actor/plan and old payloads are never rewritten by migration. No normalization,
compaction, original writes or semantic-version changes occur. Every exception,
SQLite error or pre-commit process interruption leaves the original logical v1
state. Physical database/WAL bytes may change while representing the same
committed state; this is not a byte-identical file guarantee. No checkpoint is
published or trusted during migration.

## Version and failure behavior

Explicit 1->1 and 2->2 calls are verified no-ops. A 1->2 call succeeds once;
repeating it against v2 returns UNSUPPORTED_SCHEMA (4), without mutation.
Unsupported pairs/future versions and wrong application IDs return 4. A changed
SQL fingerprint or malformed document returns 6. Existing bounded BUSY/resource/
IO mappings apply. Pre-commit fault/crash tests, exact schema comparison and
canonical replay equivalence are required, including retained branches and redo.

The Step 3 storage compatibility blocker is resolved by the explicit Architect
decision. Numerical contracts, complete tool integration and platform/hosted
certification remain Step 3 acceptance work. This ADR makes no PASS claim.

## Consequences

Applications must deliberately upgrade before persisting new tone operations.
Existing v1 readers/writers remain valid for their original capability. Migration
retains all old IDs and meanings; new tool contracts still require independent
pixel, failure, platform and hosted validation. Unknown future tools are excluded.

## Alternatives

Editing schema 1 in place breaks its exact fingerprint and old catalogs. An
opaque JSON parameter blob weakens typed constraints. Automatic migration changes
state on open. Whole-file publication requires quiescing/replacing catalog/WAL
names; this pair's explicitly approved transactional DDL avoids that complexity.

## Validation

Native tests compare independent v1/v2 schema rows, canonical documents and
session/generation/epoch. Empty/populated catalogs, exposure/crop/rotation,
branches/redo, corruption/version/application-ID rejection, allocation/SQLite
errors and pre-commit child-process crashes must preserve the accepted state.
Platform calls use the same checked migration API. See the Step 3 report for
actually executed checks; pending platform gates are not PASS.
