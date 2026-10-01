# ADR 0002: Local projects and immutable history

Status: Accepted
Date: 2026-10-01

## Context
Manual and AI must operate on the same recoverable state without modifying imported photos.

## Decision
Immutable managed originals, versioned operation graph, SQLite revision catalog and content-addressed large assets. Approval commits atomically; undo/redo changes revision pointer.

## Alternatives
Destructive edits violate source safety. Saving whole rendered images per edit wastes storage and loses inspectability. Cloud-authoritative state violates offline default.

## Consequences
Migrations, garbage collection and crash reconciliation are required. Source-copy storage costs are visible; pruning must preserve referenced assets.

## Validation
Applicable evidence is required by [QUALITY_GATES.md](../../QUALITY_GATES.md). Implementation status is separate from acceptance of this decision.
