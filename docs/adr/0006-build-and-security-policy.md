# ADR 0006: Build gates and dependency security

Status: Accepted
Date: 2026-10-01

## Context
Windows is current host, but iOS and hardware guarantees need separate evidence. Local-first promises require enforceable data boundaries.

## Decision
Host C/C++ tests plus Linux sanitizer, macOS Swift and Android NDK boundary CI. Phase 1 adds app builds and locks. No third-party Phase 0 runtime dependencies; review/pin all additions. Default no content telemetry or upload; app-private storage/backup exclusion.

## Alternatives
Treating unrun CI as passing hides integration risks. Bulk dependencies and analytics defaults enlarge attack/privacy surface. Windows alone cannot validate Apple toolchains.

## Consequences
Phase promotion may be blocked by missing runners. Hosted runner images are not hermetic. Signing/store actions require human keys; unsigned compilation does not.

## Validation
Applicable evidence is required by [QUALITY_GATES.md](../../QUALITY_GATES.md). Implementation status is separate from acceptance of this decision.
