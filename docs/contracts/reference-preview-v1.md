# Bounded reference preview, version 1

## Authority and scope

Preview is a derived, disposable, immutable SDR display reference. Input is an already evaluated canonical working handle (normalization and identity also produce the same working kind). Preview never changes source bytes, documents, operations, revisions, SQLite or assets. No GPU, export, resize, sampling, persistent cache, ICC, wide gamut, HDR or tone mapping. Shared C++ is the numerical authority; platform tests copy bytes only, off UI threads.

## Dimensions and resources

Product Owner approved Option A: exactly 1:1, width and height equal logical evaluated dimensions. No automatic downscale or handle eviction. Hard limits inherit working-image admission: width/height 16384, 8388608 pixels, working raster 128 MiB. Preview row bytes = width*4 <=65536, preview bytes = width*height*4 <=32 MiB. All arithmetic/layout is checked before allocation. Combined source/decoded/working/preview/cancellation cap remains 64; at most 63 simultaneous previews, because each creation requires a live working handle in the same combined cap.

Context payload/reservations remain <=256 MiB, reducible at init. Retained encoded/profile capacities are counted once per shared provenance; decoded and working capacities count independently, including retained source-sized cropped capacity; every preview vector capacity counts fully. Preview retains no provenance or working raster. One private destination is admitted before allocation, then capacity is verified against reservation. No heap scratch/parser/plan allocation in preview; lazy tile control is constant stack space. Existing bounded registry/object/STL/allocator overhead remains additional under ADR 0011; the payload ceiling is not peak RSS. No invisible cache. Ordinary context calls serialize, so active evaluation/preview reservations do not overlap within a context.

Some legal retained-context states reject preview with existing RESOURCE_LIMIT status 8. Example: 3072x2560, two 120 MiB working capacities plus a 30 MiB preview =270 MiB before provenance. This is expected admission behavior. Existing handles remain intact; caller may explicitly release unneeded handles and retry. No ceiling is raised.

## Numerical format

Input: tightly packed IEEE binary32 RGBA, linear-sRGB/D65, premultiplied alpha, top-left orientation 1. Output format identifier `PIXAURA_PREVIEW_RGBA8_SRGB_STRAIGHT_V1` =1: tightly packed byte order R,G,B,A, sRGB-encoded straight RGB, linear alpha, top-left rows. This is a reference SDR preview, not full color-management certification.

All four input channels must be finite. Alpha must be in [0,1]. Zero alpha (including signed zero) requires zero RGB, as in existing working validation, and outputs RGBA=(0,0,0,0). Hidden zero-alpha color rejects, never silently erases invalid working pixels. For alpha>0, divide binary64 RGB by binary64 alpha, then clamp display RGB to [0,1]; source floats remain unchanged. Finite negative or >1 working RGB is legal; only preview clips it. Tiny positive alpha is not thresholded, even if its output alpha quantizes to zero. Such pixels can retain straight RGB while A=0.

Standard sRGB OETF on clamped linear L: 12.92*L for L<=0.0031308, otherwise 1.055*L^(1/2.4)-0.055. Quantization is nearest byte, ties upward: floor(255*S+0.5). To avoid platform libm variation at transitions, the scalar implementation compares L against 255 frozen binary64 inverse-OETF half-byte thresholds in preview_thresholds.hpp. Threshold k is the binary64 approximation of inverse OETF((k+0.5)/255); equality selects k+1. The frozen constants, binary64 unpremultiply and threshold comparisons define the exact finite implementation. Threshold approximation error is <2e-15; tests independently compare the formula over broad samples and all binary32 neighborhoods. No runtime pow, platform conversion or mutable tables. Alpha independently uses floor(255*double(a)+0.5). FE_TONEAREST is required, otherwise status 7; no rounding mode is changed. Existing strict no-fast-math/no-contraction flags apply.

| Linear RGB / alpha | Exact RGBA8 |
| --- | --- |
| (0,0,0)/1 | 0,0,0,255 |
| (1,1,1)/1 | 255,255,255,255 |
| (.18,.18,.18)/1 | 118,118,118,255 |
| (.5,.5,.5)/1 | 188,188,188,255 |
| (.0031307,.0031308,.0031309)/1 | 10,10,10,255 |
| (.25,.25,.25)/.5 | 188,188,188,128 |
| (-1,2,.5)/1 | 0,255,188,255 |
| (1e-30,0,0)/1e-30 | 255,0,0,0 |
| (0,0,0)/0 | 0,0,0,0 |

## Lifecycle, cancellation and ABI

Request -> validate/admit -> allocate private RGBA8 destination -> convert -> fence publication -> publish handle -> metadata query/bounded byte copy -> explicit release. Source and preview are separately owned; releasing source does not invalidate preview. No owning pointer crosses C ABI. Additive preview.h family version 1 does not change ABI 1, feature bits, existing layouts or statuses. Metadata is explicitly versioned and contains dimensions, format, reserved zero, row bytes and byte length. Copy uses checked byte offset/count, caller-owned accessible buffers and permits zero bytes at end. Platform-owned copies are caller memory, not native cache or context ownership.

Handles use existing fresh context identity and monotonic non-reused serials, separate stable preview kind, combined cap and stale/cross-context/wrong-kind checks. Explicit preview_release requires preview kind; existing generic decode_release can also release an image registry entry. Decode metadata/profile/image operations reject previews. No source provenance is inferred from display bytes.

Optional same-context Step 7 cancellation token; null means uncancellable synchronous request. Check at admission, immediately before and after destination allocation, before each lazy 128x128 row-major tile, and immediately before publication. At most 16384 conversions between tile checkpoints; bounded allocation and lock waits are non-preemptible. Publication shares existing cancellation mutex: a signal committed before publication rejects with status 13; a later signal does not revoke an already published handle. Signal/release may run separately; acquired token lifetime survives token release. Ordinary context operations serialize; independent contexts can run concurrently; destroy/init require quiescence. No workers or asynchronous scheduling.

Any argument/version/handle/layout/numeric/resource/allocation/cancellation/registration failure publishes nothing, leaves all caller outputs and existing state unchanged, and releases private destination. Existing statuses: 1 argument/copy bounds, 2 family version, 3 stale/wrong kind, 7 invalid pixels/rounding environment, 8 resource/allocation, 13 cancellation, 17 invalid representation, 19 invalid layout. No new status. Recovery and failure-at-N allocations include vector buffer, vector object, registry insertion and debug STL control allocations. No scratch allocation exists to inject.

## Validation

Native independent scalar formula oracle, reviewable exact-byte goldens, all 255 RGB threshold predecessor/tie/successor cases and binary32 neighborhoods, alpha monotonicity and transition neighborhoods, 2048 deterministic properties, malformed layout/nonfinite/overflow inputs, max raster, aggregate budget rejection/retry, combined cap, lifecycle/read-release/signal races, cancellation at every checkpoint, failure-at-N sweep and C/JNI/Swift consumers. ASan/UBSan and real hosted MSVC/Apple/Android/Linux remain mandatory. Diagnostics 256x256,1024x768,4096x2048 report input/output bytes, tile count and wall time; no throughput/device performance claim.
