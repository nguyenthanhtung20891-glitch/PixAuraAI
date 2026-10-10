# Architectural decision index

Phase 3 Step 5: accepted operation batches already support one preset application as one detached manual revision proposal. [Preset v1](docs/contracts/presets-v1.md) freezes bounded canonical tone/detail recipes, excludes geometry, and ships an empty preset catalog. Normal operations/revisions retain inspectability and fit Schema 3; no Schema 4 or migration change is required. No parallel engine or AI behavior. Step 6 is not started.

Phase 3 Step 4 authorization freezes only scalar Sharpen/Blur under the [detail v1 contract](docs/contracts/manual-detail-v1.md). Schema 1/2 remain immutable; independent schema 3 and explicit transactional 2->3 are authorized Category A. Bounded three-row scratch preserves existing ceilings and two-raster ownership. [ADR 0020](docs/adr/0020-bounded-detail-reference.md) records this narrow extension; no automatic migration, direct 1->3, GPU expansion or Step 5 work.

Phase 3 Step 3 Architect decision, 2026-10-10: schema 1 remains immutable;
introduce independent storage schema 2 and explicit transactional 1->2 migration,
without automatic migration. Preserve every accepted source/history/envelope
value and canonical replay. [ADR 0019](docs/adr/0019-explicit-storage-schema-evolution.md)
records the narrowly approved SQLite transaction protocol and compatibility.
This resolves the Step 3 storage blocker; migration is Category A, not a new step.
The [tone/color contract](docs/contracts/manual-tone-color-v1.md) freezes the
initial six numerical recipes under Step 3 authorization; exposure 1/1 is unchanged.

## Phase 3 numbered roadmap freeze (2026-10-08)

Authority: explicit Product Owner / Architect decision. The official Phase 3
sequence is frozen as [Steps 1–8](ROADMAP.md#frozen-phase-3-numbered-roadmap--manual-tools).
Step 1 is COMPLETE / FULL PASS at `b29123037effd76d984780f59889f1fcbb3c8c76`;
Steps 2–8 are NOT STARTED. This supersedes the earlier absence of a numbered
roadmap, not the accepted Step 1 operation/parameter contracts. Approved future
tool scopes still require their specified units/ranges/versioned semantics or
separate recipe freezes before implementation/shipping as applicable.

Apply [A/B/C scope discipline](QUALITY_GATES.md#phase-3-scope-discipline) to every
new proposal. No additional steps, shipping tools, Phase 3.5, AI behavior or
Phase 4 work may be introduced without explicit Product Owner approval. Close
Phase 3 at Step 8 only after all exit criteria pass; no automatic Step 9.
This task records the plan only and does not authorize Step 2 implementation.
DH-APPLE-METAL-01 and its existing decision/requirements remain unchanged.

## Phase 3 Step 1 authorization (2026-10-08)

The Product Owner authorizes only Manual Tool Contract & Registry Foundation after
Phase 2 closure at 67454642c3f918ef67618d78ace41cef150c1c03. Reuse accepted Phase 2
document/history/storage/preview semantics; descriptors freeze only accepted
operation tuples. Other required MVP tools remain PROPOSED until separately
reviewed parameter/behavior freezes. No numbered Phase 3 roadmap is committed;
later steps must not be assigned numbers or scope. This refines ADR 0008/0009
without superseding architecture. See [contract](docs/contracts/manual-tools-v1.md)
and [report](docs/reports/phase-3-step-1.md).

Phase 2 is CLOSED by the explicit Product Owner/Architect decision below. Steps 1-11 are accepted FULL PASS; the original first GPU pipeline is complete, hosted compile/regression is green, and physical Android Vulkan certification is accepted PASS 14/14. Apple physical Metal certification is DEFERRED, not waived, under [DH-APPLE-METAL-01](QUALITY_GATES.md#dh-apple-metal-01-deferred-physical-apple-metal-certification). Existing ceilings, CPU authority and GPU semantics remain unchanged. Phase 3 has not started; no additional Phase 2 work is authorized.

## Phase 2 closure decision (2026-10-08)

Authority: explicit Product Owner/Architect instruction, accepted Android evidence publication `bd10239c38f069a2d683b0f513d5c77a02995598`.

**Apple physical Metal certification deferred due unavailable hardware; mandatory before Beta/production; Phase 2 closure allowed.**

Apple execution is blocked by unavailable physical hardware, not implementation. This decision supersedes only the requirement in [ADR 0018](docs/adr/0018-physical-gpu-certification.md) and [GPU tile v1](docs/contracts/gpu-tile-v1.md) that Apple physical execution block Phase 2 closure. The historical ADR is retained unchanged; the physical certification requirement and every technical acceptance criterion remain mandatory. The gate is not waived or removed.

Track the deferred gate as **DH-APPLE-METAL-01**, owned by the Product Owner/Architect, at Phase 11 hardening acceptance and before Phase 12 Beta readiness completion or any production release. A physical Apple GPU certification claim requires actual physical Metal dispatch/completion/readback and 14/14 shared CPU parity, fallback disabled, exact tested source SHA and reviewed device/runtime evidence. Hosted compilation, simulator execution, Android PASS and this phase-closure decision cannot satisfy it. Hardware unavailability must continue to block those later acceptance/release boundaries until the gate passes; agents must not silently forget or auto-waive it.

Phase 2 Steps 1-11 remain FULL PASS; no additional step is added. The [exit-closure report](docs/reports/phase-2-exit-closure.md) preserves implementation, hosted and accepted physical Android evidence and limitations. This decision closes Phase 2 only and does not authorize Phase 3 implementation or further Phase 2 work.

| ADR | Status | Decision |
| --- | --- | --- |
| [0018](docs/adr/0018-physical-gpu-certification.md) | Accepted; Phase 2 closure timing superseded by the decision above | Physical Vulkan/Metal dispatch and shared CPU parity; bounded FP32 tiles and fallback; separate hardware certification; technical criteria remain mandatory |
| [0017](docs/adr/0017-interactive-preview-generation-fence.md) | Accepted | Fixed nonwrapping context request generations; latest-wins native and platform publication fences; stop/join ownership |
| [0001](docs/adr/0001-native-shells-shared-core.md) | Accepted | Native mobile UIs; shared C++17 C ABI |
| [0002](docs/adr/0002-local-project-history.md) | Accepted | Immutable originals, local SQLite + asset store, shared revision graph |
| [0003](docs/adr/0003-deterministic-photo-pipeline.md) | Accepted | CPU semantic oracle, platform GPU adapters, SDR first |
| [0004](docs/adr/0004-ai-execution-boundary.md) | Accepted | Typed local plans, bounded executor, explicit approval |
| [0005](docs/adr/0005-local-inference-and-tiers.md) | Accepted | Provider-neutral AI; ONNX interchange, measured progressive tiers |
| [0006](docs/adr/0006-build-and-security-policy.md) | Accepted | Multi-host quality gates and dependency/privacy policy |
| [0007](docs/adr/0007-shell-navigation-and-build-inputs.md) | Accepted | Typed minimal shell coordinators, pinned inputs, deterministic Xcode project |
| [0008](docs/adr/0008-document-stacks-and-manifest.md) | Accepted | Ordered stacks per retained revision, explicit undo/redo path, bounded canonical JSON checkpoints; SQLite remains live authority |
| [0009](docs/adr/0009-explicit-document-context.md) | Accepted | Explicit caller-owned document registry contexts; context-scoped opaque tokens; ABI 1 unchanged |
| [0010](docs/adr/0010-durable-native-project-storage.md) | Accepted | Normalized SQLite WAL/FULL authority, durable epochs, verified streamed SHA-256 asset publication and derived atomic checkpoints |
| [0011](docs/adr/0011-shared-bounded-sdr-decode.md) | Accepted | Shared portable bounded JPEG/PNG SDR decoders; supersedes platform codec ownership for these formats only; transient RGBA8 and configurable hard limits |
| [0012](docs/adr/0012-bounded-cpu-exposure-evaluation.md) | Accepted | Frozen CPU exposure recipe, finite unclamped RGB, bounded ordered stacks and one private output raster |
| [0013](docs/adr/0013-bounded-geometry-and-cancellation.md) | Accepted | Approved outward crop rasterization, ordered geometry preflight, lazy tiles and cooperative atomic cancellation |
| [0016](docs/adr/0016-bounded-bilinear-platform-preview.md) | Accepted | Rational-floor no-upscale preview fit; binary64 premultiplied-linear bilinear; bounded platform-owned Bitmap/CGImage copies; unchanged ceilings |
| [0015](docs/adr/0015-bounded-reference-preview.md) | Accepted | Independently owned 1:1 SDR RGBA8 preview; unchanged ceilings and explicit aggregate resource rejection/retry |
| [0014](docs/adr/0014-two-raster-geometry-execution.md) | Accepted | Exact ordered geometry movement within two rasters using in-place compaction and a bounded bitmap |

Accepted architecture does not imply an implementation or verified runtime. Exact models, Android GPU backend fallback coverage, shader tolerance and subscription grace remain evaluation items in relevant phases. Major changes require new numbered ADRs and index updates.

Phase 2 Step 5 refines existing ADR 0003/0009/0011 in the [CPU working-image contract](docs/contracts/cpu-working-image-v1.md): canonical premultiplied linear-sRGB RGBA32F, fixed bounded transfer tables, explicit unsupported profile transformation, existing context-scoped lifetime. No new dependency or superseding architecture decision is introduced.

Phase 2 Step 6: [ADR 0012](docs/adr/0012-bounded-cpu-exposure-evaluation.md) freezes exposure/1/1 numerical evaluation, finite unclamped RGB policy, 256-operation admission and one-private-destination buffering. No historical tuple, document/storage schema or ABI 1 meaning changes.

Phase 2 Step 7: [ADR 0013](docs/adr/0013-bounded-geometry-and-cancellation.md) records the Product Owner approved outward crop rasterization, transient ordered geometry plans, lazy 128x128 tiles and cooperative cancellation. Raster/context ceilings, historical tuples and ABI 1 remain unchanged.

Phase 2 Step 8: [ADR 0014](docs/adr/0014-two-raster-geometry-execution.md) activates exact crop/rotate movement and specifies in-place compaction/permutation under the two-raster limit with a <=1 MiB admitted bitmap. Historical operation meanings, raster/context ceilings and ABI 1 remain unchanged.
