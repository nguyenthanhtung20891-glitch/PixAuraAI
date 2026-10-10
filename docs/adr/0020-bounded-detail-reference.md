# ADR 0020: Bounded detail reference and storage capability

Status: Accepted under explicit Phase 3 Step 4 authorization, 2026-10-10.

## Context

Sharpen/blur require neighborhood input, unlike accepted pointwise tone tools.
Schema 2's immutable exact SQL cannot represent either durable tuple. The owner
explicitly authorizes independent schema 3 and an explicit transactional 2->3.

## Decision

Freeze [detail v1](../contracts/manual-detail-v1.md): one integer scalar per tool,
fixed radius-one binomial kernel, clamp borders, premultiplied linear domain,
coverage-aware alpha rules and ordered strict reference arithmetic.
Preserve two request rasters. Reuse one scratch buffer for a three-row input ring
or rotation bitmap. The maximum, not sum, is admitted and stays <=1 MiB. This
extends ADR 0014's bitmap-only scratch and old CPU traversal bound only for newly
authorized detail stages; old operation meanings and all global ceilings remain.
Stage input-visit metadata still counts input area, not neighborhood instructions;
detail adds at most nine neighbor visits and bounded row copies per input pixel.

Preserve independent immutable v1/v2 SQL/fingerprints. Add strict schema 3 with
only blur/sharpen columns and exact /1/1 tuple bounds. The explicit transactional
2->3 pair uses ADR 0019's validated copy/DDL/verify/version/COMMIT protocol and
rollback/crash guarantees, now authorized for this pair. No auto migration.
Keep 1->2 unchanged; reject direct 1->3. Owners request both hops separately.
Verified 3->3 is a no-op; repeating 2->3 on v3 rejects without mutation.

## Alternatives

A third raster changes accepted memory ownership. In-place convolution without
original row retention changes pixel semantics. Platform filters lose shared
authority. Editable old SQL breaks compatibility; arbitrary blobs weaken checks.
Radius/threshold/algorithm controls expand the approved narrow v1 scope.

## Consequences

Blur changes coverage by definition; sharpen preserves alpha. Overshoot is finite
unclamped RGB or deterministic failure. Applications explicitly upgrade storage
before saving detail. No detail GPU, final UI or other tool is implemented.

## Validation

Independent exact-rational fixtures, order/restart/alpha/border tests, bounded
scratch admission/cancellation/allocation tests, historical fingerprints, both
explicit migration hops, malformed/SQL/fault/crash recovery and full local/hosted
platform gates are required. This decision alone claims no test PASS.
