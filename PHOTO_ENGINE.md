# Non-destructive photo engine

Phase 2 Step 5 adds the [CPU working-image contract](docs/contracts/cpu-working-image-v1.md): all EXIF orientations normalize into bounded canonical RGBA32F linear-sRGB premultiplied pixels; absent/default or explicit sRGB uses fixed transfer tables. ICC and unresolved/contradictory PNG color annotations defer explicitly. Identity evaluation creates an independently owned exact copy; no editing tool or renderer exists. Step 4 admission/decode and retained immutable provenance remain under [ADR 0011](docs/adr/0011-shared-bounded-sdr-decode.md).

## Source and graph

The [schema 1 document contract](docs/contracts/image-document-v1.md) and ADR [0008](docs/adr/0008-document-stacks-and-manifest.md) refine this graph into a linear ordered stack for each revision, with a single-parent revision tree retaining branches. General multi-input pixel DAG evaluation is deferred until a concrete feature requires it. Operation envelopes, integer parameter units, unknown-version rejection, bounded canonical JSON, history transitions, source identity and render/export request ownership are frozen there. These are contracts, not implemented pixel tools.
Import copies to immutable private storage and records SHA-256, dimensions, orientation, source color profile and codec. External URI is provenance only, not the editable source. Source files are opened read-only. Immutable operation records contain stable IDs, operation/parameter versions and validated parameters. Schema 1 uses the preceding stack output as each operation's implicit input; a future mask extension adds declared immutable asset references. Ordered edits are never reordered implicitly. Revision identifies its complete ordered stack; branching after undo preserves approved historical revisions until explicit pruning.

Manual commits and approved AI batches call one command service. Parameter gestures render transient candidates; one gesture commits one history item. Undo/redo moves the active revision rather than applying inverse math. AI candidates do not enter approved history until approval. Audit stores actor and plan link; users inspect the actual tool list. Persist current revision and redo path atomically.

## Pixel semantics
Normalize EXIF orientation once into source coordinate space. Convert embedded profile to linear sRGB float working values for SDR MVP, with defined clamping at output; premultiplied alpha for compositing. Operations specify whether they act in linear luminance or perceptual space; no implicit platform-dependent defaults. Parameter units, valid ranges, kernel edges and operation order are versioned before Phase 2 implementation. Resize uses deterministic reference resampling; GPU approximations require measured tolerance. Export embeds sRGB profile and strips location metadata by default. Source profile and metadata remain with private original. Wide gamut/HDR require separate validated working-space ADR before support.

## Render pipeline
Bounded decode -> orientation/color conversion -> operation graph evaluation -> display transform -> native texture. Preview uses reduced resolution; export renders selected approved revision in tiles at requested resolution. Tile halos cover blur/sharpen kernels and stitch without seams; global statistics are a separate bounded reduction pass. Segmentation masks have declared source coordinates and geometry transforms. Cache keyed by source hash, operation version, parameters, model/mask hash and resolution; invalidated on relevant changes.

GPU backends share semantic fixtures with CPU reference. CPU tiling remains functional on low-tier devices. Estimate peak live buffers before starting; checked arithmetic, maximum dimensions/decoded bytes, job cancellation and memory-pressure downgrade are required. No full-resolution intermediate chain retained unnecessarily. Jobs run off the UI thread; only newest candidate can publish preview. Engine jobs retain resources until completion, even after cancellation, then release exactly once.

## Export and recovery
Reserve a new media destination, write temporary output, flush/close and atomically finalize where platform APIs allow; Android pending MediaStore entry and iOS add-only photo-library API provide platform-specific completion. On failure remove only the newly created incomplete output. Reject source/destination identity. Imported source checksum must survive edit/export tests. JPEG/PNG export validates dimensions, quality and color/alpha behavior. Project crash recovery replays last committed revision; rendering never changes persisted history.

## Phase 0 boundary
Only ABI/version contract exists. Codec, graph, tools, GPU kernels and export are specified here and must not be represented as implemented.
