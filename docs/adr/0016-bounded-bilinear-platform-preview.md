# ADR 0016: Bounded bilinear preview and platform-owned copies

Status: Accepted
Date: 2026-10-07
Scope: Product Owner authorizes Phase 2 Step 10 and approves oversized-source rejection diagnostics.

## Context

Step 9 owns exact reference preview bytes. Platforms need a bounded one-shot display consumer and approved downscale semantics without changing editing authority or resource ceilings.

## Decision

Extend Step 9 with exact and no-upscale rational-floor bounded-fit requests. Shared scalar C++ bilinear filters binary32 premultiplied linear-sRGB input with explicitly ordered binary64 intermediates, then uses the unchanged Step 9 display quantization. No editing resize operation, GPU, float destination raster, cache or implicit eviction. Existing exact API/bytes, ABI, authority, resource limits and lifecycle remain unchanged.

This supersedes only Step 9's preview-only no-resampling scope. Historical Step 8 geometry movement and document operations retain their meanings. One-shot bounded app-owned Bitmap/CGImage copies are authorized for platform consumption/testing; the future production native-surface/per-frame architecture remains separate.

Preserve 128 MiB working raster, 8388608 pixels, 256 MiB context and 64 handles. 4032x3024 diagnostic sources intentionally reject normalization with status 8; timing uses admitted 4032x2048 and the other legal requested cases. No bypass or alternate oversized source.

## Alternatives

Nearest neighbor has poorer preview credibility; bicubic/area filters add unsupported scope. Filtering straight encoded RGBA8 violates the approved alpha/color domain. A full float destination wastes bounded memory.

## Consequences

Bilinear is simple and deterministic but strong reduction can alias. Platform one-shot copies add explicitly bounded app memory and must run off UI threads; they do not own native authority or persistence.

## Validation

Exact formulas, error and copy policies are in [bounded-preview-resampling-v1](../contracts/bounded-preview-resampling-v1.md). Implementation evidence belongs in the [Step 10 report](../reports/phase-2-step-10.md); this ADR alone is not PASS.
