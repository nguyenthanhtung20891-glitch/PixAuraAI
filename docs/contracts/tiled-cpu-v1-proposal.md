# CPU tiled processing v1 — compatibility proposal

Status: PROPOSED integration contract; Product Owner approved architectural direction,
not a waiver of frozen contracts. Experimental normalized-tile evaluator may be
implemented only with demonstrated numerical equivalence. No production12MP claim.

## Existing boundary and exact incompatible contracts

cpu-working-image-v1.md/working.h require a contiguous owned RGBA32F raster,
<=8,388,608pixels and128MiB. Geometry v1 plan admission also applies that raster
limit. encoded-image-v1/decode APIs produce whole decoded/normalized images;
export v1 accepts existing native document/working owners and complete output
buffers. A4000x3000 RGBA32F image needs192,000,000bytes before any second raster.
Those representations cannot implement full12MP under the frozen cap. No existing
ABI signature, layout, Schema3 operation or cap will be reinterpreted or raised.

Requested versioned replacement for Architect review: internal immutable source
reader + bounded normalized tile lease + stage-coordinate plan + transactional
stream encoder/writer interfaces, then an explicitly approved additive native bridge
binding those owners to existing document revisions. Keep existing v1 APIs unchanged;
new large-image owner/ABI, stream-output ownership and representation need approval
before production wiring. Existing v1 import still rejects unsupported12MP.

## Memory, authority and publication

One owner serializes snapshot/admission/reservation/allocation; reserve aggregate
retained source/decoder/index/ICC/encoder/tiles/halos/scratch/transport bytes, including
concurrent work. Existing hardware-tier cap and allocator checks remain binding.
Experimental execution accepts at most T0's128MiB total supplied cap and explicit
retained bytes, reserves worst-case tile buffers/scratch before reading, keeps one
synchronous tile in flight, checks actual capacities and never allocates full raster.
This is not production hardware admission: trusted codec/source occupancy and all
encoder buffers must be proven before integration. No process-RSS admission ceiling.

Read immutable source identity and ordinary committed revision stack. New preview
or staged output grants no edit authority. A generation/cancellation fence precedes
each tile and finalization; failure aborts staged output. A production durable writer
must atomically publish only complete encoded bytes after final native revalidation.
AI approval and HUMAN Apply remain existing native authority, never a quota token.

## Numerical and image behavior

Normalized input is the existing orientation1, premultiplied linear RGBA32F reference
representation, with exact signed-zero/subnormal/finite validation and rounding.
Reuse accepted evaluator kernels; no new approximate shader, interpolation or model.
For N local3x3 passes, require cumulative N-pixel halo (including neutral passes is
safe), clamp only at true global boundaries, retain exact kernel summation and
binary32 stage rounding. Pointwise stages preserve order between local passes.
Do not trim intermediate halos early. Deterministic fixtures cover1-pixel axes,
odd dimensions, every tile seam, alpha0/1/partial, negative/HDR samples, multi-pass
blur/sharpen, cancellation, stale generation and allocation/resource rejection.

Geometry: versioned stage plans must map crop/orthogonal rotation coordinates via
existing exact integer geometry rules at each stage, back-propagate required source
regions/halos, and preserve operation order. Geometry is explicitly unsupported by
this experimental tile executor until its replacement plan contract is accepted.
Future global-image operators need bounded multi-pass reductions and deterministic
order/rounding, or explicit rejection; tile-local statistics are never equivalent.

ICC/orientation/alpha: retain existing codec/color admission, supported profiles,
transfer tables, all8 EXIF orientation mappings and alpha semantics. The normalized
reader experiment is NOT proof of tiled codec/color conversion. Unsupported ICC
must still reject; no silent profile drop or reinterpretation. Color/orientation
conversion must be golden-tested independently before codec integration.

Streaming JPEG/PNG: exact full output dimensions, existing quality/quantization,
metadata/privacy policies, row order and cancellation. JPEG scanlines and PNG rows
need bounded codec state; rotated/revisited source may need indexed immutable
backing under aggregate admission. No silent downsampling/proxy substitution. An
encoder experiment alone is not product support or a12MP performancePASS.

## Acceptance boundaries

Bitwise oracle parity is required for the implemented normalized pointwise/local
subset against accepted CPU evaluation. Synthetic4000x3000 execution proves only
bounded normalized-tile traversal; real import/edit/preview/history/export, full
geometry/color compatibility and physical LOW/MID/HIGH budgets remain unqualified.
Original XR results remain at originalSHA. DH-APPLE-METAL-01 remainsPASS/CLOSED;
CPUVALIDATED and Metal/VulkanprovidersUNKNOWN. No GPU backend is enabled.

## First bounded integration candidate (not implemented/accepted)

Existing decoded immutable RGBA8 for12MP is48,000,000bytes; a full normalized
RGBA32F is192,000,000bytes and remains prohibited. Prefer reusing an admitted
immutable decoded RGBA8 owner with window normalization from the existing transfer/
orientation rules, rather than allocating a full float image. A128row RGBA8 output
band at4000pixels is2,048,000bytes. These are calculated representation sizes, not
measurements/admissionPASS: codec state, encoded output worst case, native retained
owners/UI copies, halos, concurrent work and provenance must all be reserved too.
Stream rows or a bounded band into the encoder; do not add another full RGBA8 image.

Proposed original-reader-v2 must bind encoded SHA256+length, full dimensions, profile/
orientation and immutable decoded provenance while exposing only bounded normalized
windows. It cannot masquerade as the existing contiguous working original handle.
A stage-plan-v2 must carry exact stage extents and backward region/halo maps for
crop/rotation and ordered local operations. A stream-writer-v2 must own staging,
errors/cancellation and final native publisher-lock validation. Each needs explicit
compatibility review before integration; the unchanged v1 ABI continues to reject
unsupported12MP work. This preserves the product target without silently admitting it.
