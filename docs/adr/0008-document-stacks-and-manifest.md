# ADR 0008: Ordered edit stacks, retained revisions and bounded manifests

Status: Accepted
Date: 2026-10-03
Scope: Phase 2 Step 1 contracts; production implementation pending

The process-local registry portion of the referenced C API design is superseded by accepted [ADR 0009](0009-explicit-document-context.md), following explicit user approval. All other decisions in this ADR remain accepted.

## Context
The user has authorized Phase 2 Step 1 after reporting all Phase 0/1 CI gates passed. ADRs 0001-0004 already require C++ domain authority, immutable originals, local SQLite revision storage, reproducible operations and explicit candidate approval. They do not require arbitrary multi-input pixel evaluation. Shell restoration is navigation state only; ABI 1 has no document feature.

## Decision
Refine, rather than supersede, those decisions with the [document contract](../contracts/image-document-v1.md). Each approved revision contains one ordered stack of immutable operation IDs. Each non-root revision has exactly one earlier parent; retained revisions can branch after undo. This is a revision tree, not a general processing DAG. A full stack per revision permits changing/removing an earlier adjustment by recording a new revision without rewriting its predecessors. Operations may be shared by reference, but evaluation is sequential and never implicitly reordered.

Undo/redo moves the current revision pointer and an explicit redo path. A commit after undo clears only the redo path, retaining all old revisions and operation records. Explicit checkout clears redo. One gesture or approved batch produces one revision; candidate previews are detached and never persisted as approved revisions. Stale-base validation applies to manual and future AI commands alike.

Use bounded, human-inspectable JSON schema 1 for project snapshots and operation envelopes. Canonical output sorts ASCII object keys, preserves arrays, uses integer units and records explicit versions; no platform float formatting is required. Stable IDs are caller-supplied 128-bit values and asset identity is SHA-256. Unsupported operation types/versions fail closed; no silent skip or reinterpretation. A future mask is an immutable asset input to an operation, not permission to merge revision branches.

SQLite remains the live transactional authority under ADR 0002. The manifest is a validated checkpoint/interchange representation, not a second independently writable authority or a replacement database. Schema migration, asset flush-before-reference, atomic pointer/redo updates and recovery rules are frozen in the contract; no database/parser/codec implementation is added in Step 1.

## Alternatives
A general DAG would introduce merge semantics, multi-input scheduling and much larger validation/cache scope without a current editing requirement. A destructively truncated linear history loses branches and violates ADR 0002. Inverse-operation undo loses numerical reproducibility. Floating-point parameter text requires avoidable normalization across toolchains. A binary-only manifest impedes inspection and recovery. Making the manifest and catalog coequal creates ambiguous crash recovery.

## Consequences
Full stacks cost bounded metadata (256 operations per stack; 4096 retained revisions in schema 1), not whole-image snapshots. Hitting a limit fails without automatic history deletion; compaction/pruning or larger limits need an explicit migration and later UX. Mask composition or a genuine multi-source feature can justify a new operation/schema ADR later. JSON is metadata only; pixels remain native assets/tiles. The test-only contract oracle is not a production parser or a replacement for the C++ core.

## Validation
[Contract fixtures and strategy](../contracts/image-document-v1.md) define ordering, parameter/version rejection, branching, malformed metadata, bounded parsing and deterministic output. Step 1 exercises them with Node built-ins. Existing independent C/C++ ABI, sanitizer, Android and Apple gates remain unchanged. Production C++/C ABI ownership, parser fuzzing, durable storage crash injection and source hash preservation are required before their implementations can be called complete. See the [Step 1 report](../reports/phase-2-step-1.md) for executed evidence and host limitations.
