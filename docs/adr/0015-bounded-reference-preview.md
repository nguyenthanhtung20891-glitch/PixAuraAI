# ADR 0015: Bounded independently owned 1:1 SDR reference preview

Status: Accepted
Date: 2026-10-07
Scope: Product Owner authorized Phase 2 Step 9 and explicitly approved Option A aggregate rejection policy.

## Context

Step 8 produces immutable evaluated RGBA32F working images but no display ownership. Two retained 120 MiB rasters plus a 30 MiB RGBA8 preview exceed the accepted 256 MiB payload ceiling. Resampling is not frozen and cannot be introduced implicitly. Platform bridges must share native conversion semantics.

## Decision

Implement a synchronous CPU 1:1 reference SDR preview, independently owned immutable RGBA8 straight-alpha sRGB bytes in top-left order. Preserve every existing memory ceiling and the combined 64-handle cap. Explicitly admit and fully count preview capacity. Resource status 8 is expected for legal retained-context states that lack room; caller can release unneeded handles and retry. Never evict, replace or invalidate existing working images. No resize or sampling.

Use binary64 unpremultiplication and a scalar quantized standard sRGB OETF with frozen inverse half-byte thresholds to avoid runtime libm/platform conversion variation. Clamp only derived display RGB; nearest quantization ties upward; alpha independently quantizes linearly. Existing finite/zero-alpha validation, cancellation status/publication fencing and C ABI ownership rules remain authoritative. Preview retains no source/provenance payload. Shared native bounded copy is exercised by C/JNI/Swift tests.

This additive decision supplies display-copy test capability under the existing caller-owned bounded-output architecture. It does not authorize per-frame bridge byte arrays or a production image display pipeline. Historical source, document, persistence, operation, geometry and exposure contracts are unchanged.

## Alternatives

Nearest-neighbor downscale needs new sampling semantics; bilinear/bicubic add broader numerical/ownership scope. Raising context ceiling breaks frozen admission. Implicit eviction invalidates caller ownership. Platform OETF duplicates shared numerical authority. Runtime pow can vary near byte transitions. Preview cache persistence creates additional lifecycle and authority questions.

## Consequences

Preview is disposable and non-authoritative. Full-size previews may reject despite legal working images. One private RGBA8 destination, zero heap scratch and lazy tiles bound conversion. Payload budgets retain existing additional fixed control/allocator overhead; no RSS promise. Tiny alpha may quantize to zero with nonzero straight RGB. Reference SDR clipping does not feed back into edits. Production display, resize, color management, export and Step 10 remain deferred.

## Validation

[Reference contract](../contracts/reference-preview-v1.md) and [Step 9 report](../reports/phase-2-step-9.md) contain exact rules and observed gate evidence. Tests include independent formulas, golden bytes, all byte transitions, bounded properties/adversaries, lifecycle/races, aggregate retry, cancellation and allocation recovery. Real hosted platforms and sanitizers must execute before FULL PASS.
