# ADR 0014: Two-raster ordered geometry pixel execution

Status: Accepted
Date: 2026-10-05
Scope: Product Owner authorized Phase 2 Step 8 minimal geometry execution and bounded buffering.

## Context

Step 7 freezes outward crop rasterization, clockwise quarter turns, transient ordered plans, scalar tiles and cancellation. The immutable source plus private candidate maximum is two working rasters. Ordinary ping-pong geometry would require a third simultaneous raster including the retained source. Crop cannot safely compact in arbitrary tile order because a write can overwrite a later tile's source. Rectangular orthogonal rotations permute the logical raster in cycles.

## Decision

Preserve the two-raster count and all raster/context ceilings. Copy/validate the source once into a private candidate with source-sized capacity. Execute every planned operation in order on that candidate. Crop compacts complete rows in increasing scanline order with overlap-safe memmove, then shrinks logical size without releasing capacity. Rotation executes exact inverse-mapping permutation cycles with a 16-byte saved pixel and a reused visitation bitmap <=ceil(8388608/8)=1 MiB. No alternate raster or raster per operation.

This authorized Step 8 extension supersedes only the earlier pointwise-only executable scope and zero heap kernel scratch assumptions of ADRs 0012/0013. Operation tuples, rounding, numerical exposure recipe, pixel representation, two-raster ownership, cancellation status/publication fencing and schema/ABI remain unchanged. Bitmap scratch is admitted in addition to the conservative parser/plan allowance under the unchanged 256 MiB context ceiling.

Initial source copy/exposure and rotation cycle roots use lazy row-major 128x128 tiles. Crop uses monotonic scanlines (each <=16384 pixels), a dependency-safe finer traversal that never changes crop meaning or tile partition definitions. Rotation cycles may leave their root tile; poll cancellation at least every 1024 moves and before each root tile. No per-pixel atomic polling, scheduler or parallel execution.

## Alternatives

Three-raster ping-pong would violate the explicit two-raster gate and increase admission demand. Replaying only retained final pixels would skip earlier exposure overflow in pixels subsequently cropped away, changing ordered failure semantics. A cycle algorithm without visitation storage can have pathological repeated work; a bounded bitmap gives linear stage work. An intermediate full-image scratch buffer violates the raster count even if called scratch.

## Consequences

A small crop retains source-sized candidate capacity, which is fully accounted in context resource admission/query limits; logical metadata describes only accessible output pixels. Planning computes maximum intermediate dimensions/raster/pixels and exact bitmap bound before execution. Rotation does at most one root scan plus one move per pixel, with bounded bitmap clearing; crop/exposure are linear. Per-stage input-visit metadata remains the sum of input areas, not a claim that all CPU instructions are counted. Allocation/bitmap initialization and locks remain non-preemptible bounded phases; no fixed latency claim.

## Validation

Independent allocating reference, human-readable golden matrices, bit-pattern preservation, mixed order/overflow, plan agreement, deterministic bounded properties, all checkpoint cancellation, allocation-site sweeps and C/JNI/Swift probes. [Execution contract](../contracts/geometry-execution-v1.md); [Step 8 report](../reports/phase-2-step-8.md).
