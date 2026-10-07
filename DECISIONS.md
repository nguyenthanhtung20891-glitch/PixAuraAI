# Architectural decision index

| ADR | Status | Decision |
| --- | --- | --- |
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
