# Bounded preview resampling, version 1

## Authority and compatibility

Step 10 extends [reference-preview-v1](reference-preview-v1.md) through an additive `pixaura_preview_render` entry point. Existing `pixaura_preview_create` and exact bytes, ABI 1, format identifier 1, statuses, document operations and persistence remain unchanged. This is a disposable reference SDR preview, not an editing resize operation or full color-management certification. No GPU, cache, export, asynchronous scheduler or worker pool.

Request fields are fixed-width uint32: version=1, struct_size=sizeof(request), mode=EXACT(0) or FIT(1), max_width, max_height, reserved=0. EXACT requires both bounds zero. FIT requires both positive; bounds up to UINT32_MAX are valid but cannot enlarge source admission. Unsupported version returns 2; invalid fields return 1. All failures preserve caller outputs. Pointer arguments must refer to truthful accessible storage under existing C ABI rules.

## Dimensions

First validate canonical source layout against unchanged working limits. Let W,H be source dimensions, Bx,By positive bounds. If W<=Bx and H<=By, use W,H and the existing exact converter. Otherwise compare uint64 products Bx*H and By*W. If the former is <= the latter, output (Bx,max(1,floor(H*Bx/W))); otherwise (max(1,floor(W*By/H)),By). Products use checked multiplication. The limiting dimension is exact; the other floors the rational fit. Integer rounding can slightly change aspect ratio; a mathematically subpixel dimension becomes 1. Never exceed either bound or upscale either dimension. No arbitrary resize tool is introduced.

## Scalar bilinear numerical authority

Input remains binary32 RGBA, premultiplied linear-sRGB/D65, top-left. Validate every source pixel, including unsampled pixels: all channels finite, alpha in [0,1], zero alpha requires zero RGB. Negative and >1 finite RGB remain legal. Source is never modified.

For destination integer x,y, compute binary64 sx=((double(x)+0.5)*double(W))/double(DW)-0.5 and corresponding sy. Each operator rounds to binary64 nearest-even in the stated order; FE_TONEAREST is required. Compute floor neighbors and fractions tx=sx-floor(sx), ty=sy-floor(sy). Independently clamp neighbor indices to [0,W-1]/[0,H-1], retaining the original fractions. Coordinates/products remain small enough for exact integer-to-binary64 conversion under admitted source dimensions. Strict compiler flags prohibit fast-math and contraction; no FMA or platform resampler.

For each premultiplied R,G,B,A independently: upper=p00*(1-tx)+p10*tx; lower=p01*(1-tx)+p11*tx; value=upper*(1-ty)+lower*ty. Every multiplication/addition/subtraction/division rounds separately to binary64, in that order. No intermediate binary32 raster or rounding is introduced. Then apply the Step 9 binary64 unpremultiply, display-only clamp, frozen standard sRGB inverse half-byte threshold quantizer (ties upward) and independent linear-alpha quantizer. Exact zero filtered alpha emits RGB zero. Tiny positive alpha may quantize to zero without suppressing straight RGB, as before. Output is byte order R,G,B,A, straight alpha, sRGB encoded, top-left, tightly packed. No Kotlin/Swift numerical formula.

This single bilinear sample is a reference preview filter, not an area/antialiasing filter; strong reduction can alias. No quality or device-performance claim is implied.

## Bounded ownership and execution

Lifecycle: validate/admit -> private destination -> validate/convert -> fenced publication -> metadata/copy -> explicit release. Working images and previews remain independently owned immutable handles. Same-context stale/wrong-kind/cross-context safety, monotonic serials, combined 64-handle cap, serialized ordinary calls, concurrent cancellation signaling/release, independent contexts and quiescent destruction remain unchanged.

The working ceiling remains 128 MiB / 8388608 pixels / dimensions 16384. Context payload/reservations remain 256 MiB. Fully count retained vector capacities, shared provenance once, control reservations and preview capacity under existing ADR 0011 accounting; fixed bounded object/stack/allocator overhead remains additional, not a peak RSS claim. Destination DW*DH*4 is admitted before allocation. No full-size float destination, intermediate preview, heap plan/weights/scratch, hidden cache or eviction. Private destination capacity is verified. A legal retained context can reject with status 8; explicit caller release and retry is allowed.

Fit source validation checks cancellation every 1024 pixels. Destination uses lazy 128x128 row-major tiles, with a checkpoint before each tile and every 1024 destination pixels, plus admission, immediately before/after allocation and immediately before publication. Exact path preserves Step 9 checkpoints. Cancellation returns 13 without partial publication; signal/publication share the existing fence mutex. Allocation and lock waits remain non-preemptible. Source validation adds linear bounded work independent of hostile bounds.

Invalid layout/pixels, request, resource, allocation, cancellation and registry failure publish nothing, reclaim private storage and preserve all existing handles/history and caller outputs. No new status. Fail-at-N coverage includes destination vector/object and registry insertion; request/plan are stack-only.

## Platform consumption

Android `CoreProbe.referenceBitmap` is a one-shot background adapter with positive bounds <=1024 in each dimension. JNI decodes/normalizes/evaluates a verified private fixture asset, requests FIT, releases working images, copies preview rows through fixed stack buffers, and explicitly packs native RGBA bytes into Java ARGB integer words using shifts. Bitmap's documented integer-color API avoids native-endian buffer assumptions. The VM array (<=4 MiB+8 bytes) and Bitmap (<=4 MiB pixel payload) are app-owned copies; Android may premultiply internally for display. No native-owned heap temporary or conversion formula. JNI allocation exceptions return no object; native context cleanup reclaims handles.

iOS/macOS `ReferencePreview.copyImage` validates native metadata and limits copied dimensions to 1024. It copies <=4 MiB into Data and creates CGDataProvider/CGImage with explicit sRGB, 8-bit components, 32-bit pixels, RGBA big-endian byte layout, alpha-last straight semantics, exact row stride and interpolation disabled. Provider retains the derived Data independently of native release. UIImage can wrap the CGImage at the app edge; no production display loop here.

Platform allocations are external caller-owned copies, not native context payload/cache; their maximums are explicit and their lifecycle is platform-owned. Calls belong off the UI thread. Existing shell instrumentation/Swift test harnesses prove safe fixture -> evaluation -> FIT -> real Bitmap/CGImage -> native release; no production editor UX or per-frame array bridge is introduced. Copies never feed document/history or persistence.

## Diagnostics and verification

Product Owner approved 4032x3024 as intentional source-admission rejections: RGBA32F needs 195084288 bytes and 12192768 pixels, exceeding both frozen ceilings. Normalization returns RESOURCE_LIMIT(8) before reading input or allocating a working image; preview never starts. These are not benchmarks and have no oversized temporary source or diagnostic bypass.

Timing sources are legally admitted: 4032x2048 ->1440x731, ->1024x520, ->256x130; 1920x1080 ->1280x720; 1024x768 ->exact. Report source/output sizes, scale, pixels, destination tiles and time; no production performance gate.

Required evidence includes reviewable byte goldens, independent four-weight/OETF oracle, dimension/edge/alpha/exact properties, malformed requests/layout/nonfinite inputs, aggregate/handle exhaustion, cancellation at all checkpoints, races/stale handles, fail-at-N recovery, C/JNI/Swift consumers, ASan/UBSan and real MSVC/Apple/Android/Linux hosted gates.
