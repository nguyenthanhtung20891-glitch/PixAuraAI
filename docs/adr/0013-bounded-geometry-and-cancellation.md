# ADR 0013: Bounded geometry planning, tiles and cooperative cancellation

Status: Accepted
Date: 2026-10-05
Scope: Product Owner authorized Phase 2 Step 7 and explicitly approved crop outward rounding.

## Context

The frozen Step 1 crop tuple intentionally deferred integer raster rounding. Step 6 executes exposure on canonical immutable working images, with bounded full-raster ownership but no concrete cancellation token or tile grid. Future geometry needs exact planning without changing historical metadata or introducing a renderer.

## Decision

Preserve crop/1/1 millionth metadata and rotate/1/1 clockwise quarter turns. Freeze crop integer raster bounds using floor starting rational edges and ceil ending rational edges. Geometry uses canonical top-left pixel-edge space and half-open rectangles; no interpolation, implicit clamp or fractional resampling. The approval resolves the intentionally unspecified rasterization in the Step 1 contract; historical metadata is unchanged.

C++ owns transient ordered preflight stages, checked dimensions/rectangles, orthogonal mappings and a lazy row-major 128x128 tile grid. Geometry planning does not execute crop/rotate. Plans containing either reject execution explicitly; exposure remains the only executable tuple. No optimization or reordered/folded operations.

Retain all raster/context ceilings. Tiling bounds pointwise work between cancellation checks, not logical image admission. One immutable input and one private destination remain the maximum request rasters. Tile metadata is generated on demand, without a tile-array allocation.

Cancellation uses sticky context-scoped non-pointer handles, shared lifetime retained by synchronous evaluation, atomic signal state and a separate context signal mutex. Signal/release may run while evaluation holds the ordinary context mutex; publication shares the signal mutex so cancellation and publication have a defined ordering. Context destruction still requires caller quiescence. Reuse existing CANCELLED status 13; no status is repurposed. No unsafe termination, scheduler, global registry or pool.

## Alternatives

Nearest-edge rounding can collapse positive crops. Fractional edges would require resampling beyond this step.

## Consequences

The approved outward policy covers the requested region, sometimes including a boundary pixel whose center is outside the exact rational rectangle. A 128-pixel edge is an implementation detail, never persisted operation meaning. Cooperative cancellation bounds arithmetic work but cannot interrupt allocation, bounded parsing, lock acquisition or OS scheduling; no wall-clock cancellation latency is certified.

## Validation

Contracts: [geometry](../contracts/geometry-v1.md), [cancellation](../contracts/cancellation-v1.md). Evidence: [Step 7 report](../reports/phase-2-step-7.md).
