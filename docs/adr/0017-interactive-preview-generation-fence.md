# ADR 0017: Interactive preview generation and publication fence

Status: Accepted
Date: 2026-10-07
Scope: Product Owner authorized Phase 2 Step 11 latest-request-wins ownership.

## Context

Step 10 produces deterministic independent previews and platform copies. Cooperative cancellation alone cannot stop a late result from replacing newer application state.

## Decision

Use context identity plus a nonwrapping uint64 request generation. Fixed metadata and the existing separate cancellation mutex allow supersession without blocking on a full synchronous render. Publication checks current generation under that fence; source access and resource admission stay under the ordinary mutex. Reuse status 13 for revoked/duplicate execution, 8 for counter exhaustion. Each request renders once; retry begins a fresh generation.

Platform owners serialize begin/cancel/stop with native-check plus display replacement. Requested and last displayed generations are distinct; failed requests preserve previous display. Detached platform copies retain ordinary ownership. Stop plus caller join precedes quiescent native destruction. No native worker, scheduler, cache, hidden history, new dependency or increased ceiling.

## Alternatives

Cancellation-only publication permits stale completion. An unbounded request map violates bounded ownership. A lock shared with the entire render blocks supersession. Implicit destruction racing native calls contradicts ADR 0009; preserve explicit stop/join instead.

## Consequences

Applications must route lifecycle changes through one owner and keep native storage stable. Tickets are transient values, not document approval or permission. Cooperative latency is qualitative; correctness comes from fencing. Fixed control overhead remains subject to existing accounting classification.

## Validation

[Lifecycle contract](../contracts/interactive-preview-lifecycle-v1.md) specifies API/errors, races, overflow, ownership and limits. [Step 11 report](../reports/phase-2-step-11.md) records executed gates; this ADR alone is not certification.
