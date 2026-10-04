#ifndef PIXAURA_DECODE_H
#define PIXAURA_DECODE_H
#include "document.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PIXAURA_DECODE_API_VERSION 1u
#define PIXAURA_DECODE_UNSUPPORTED 16
#define PIXAURA_DECODE_MALFORMED 17
#define PIXAURA_DECODE_TRUNCATED 18
#define PIXAURA_DECODE_OUTPUT_MISMATCH 19
#define PIXAURA_DECODE_JPEG 1u
#define PIXAURA_DECODE_PNG 2u
#define PIXAURA_DECODE_RGBA8_STRAIGHT 1u
/* Separate capability family; layouts preserve Step 2 lifetime obligations.
 * Init/destroy require exclusive caller access, including quiescing jobs.
 * Calls on one live context serialize; distinct contexts can execute in parallel.
 * Fresh identity per lifetime, no copying opaque storage, truthful pointer lengths.
 * Inputs, context storage and outputs must not overlap.
 */
typedef pixaura_document_context pixaura_decode_context;
typedef pixaura_document_handle pixaura_decode_handle;
typedef struct pixaura_decode_limits {
    uint32_t api_version, struct_size;
    uint64_t encoded_bytes, pixel_count, decoded_bytes, row_stride;
    uint64_t metadata_bytes, profile_bytes, exif_bytes, scratch_bytes, context_bytes;
    uint32_t width, height, marker_count, frame_count;
} pixaura_decode_limits;
typedef struct pixaura_decode_metadata {
    uint32_t api_version, struct_size, format, width, height;
    uint32_t display_width, display_height, orientation, bit_depth, channels;
    uint32_t has_alpha, profile_type, frame_count, pixel_format;
    uint64_t row_stride, decoded_bytes, profile_bytes;
    uint8_t profile_sha256[64];
} pixaura_decode_metadata;
/* All failures leave outputs unchanged. No decoder objects or pixel pointers
 * cross this API. Pixels retain encoded raster order; no color/orientation
 * transform occurs. Profile type: 0 absent/default sRGB, 1 embedded ICC,
 * 2 explicit PNG sRGB. The immutable original retains other color annotations.
 * Defaults are hard ceilings; callers may reduce each positive limit.
 */
PIXAURA_API int32_t pixaura_decode_default_limits(uint32_t version, pixaura_decode_limits* output);
PIXAURA_API int32_t pixaura_decode_context_init(uint32_t version, pixaura_decode_context* context,
    uint32_t context_bytes, const uint8_t* identity, uint64_t identity_bytes,
    const pixaura_decode_limits* limits);
PIXAURA_API int32_t pixaura_decode_context_destroy(pixaura_decode_context* context);
/* Opens ONLY a managed digest under a private root, rehashing the exact bytes
 * retained for decode. Filename extensions/provenance never affect admission.
 * Admission proves bounded structural metadata, not validity of entropy data.
 */
PIXAURA_API int32_t pixaura_decode_open(pixaura_decode_context* context,
    const uint8_t* root, uint64_t root_bytes, const uint8_t* digest, uint64_t digest_bytes,
    uint64_t encoded_bytes, pixaura_decode_handle* output);
PIXAURA_API int32_t pixaura_decode_query(pixaura_decode_context* context,
    const pixaura_decode_handle* source_or_image, pixaura_decode_metadata* output);
PIXAURA_API int32_t pixaura_decode_image(pixaura_decode_context* context,
    const pixaura_decode_handle* source, pixaura_decode_handle* output);
/* Bounded copies for diagnostics/future native consumers; no per-frame bridge
 * transport. Profile bytes remain untrusted data, never executable metadata.
 */
PIXAURA_API int32_t pixaura_decode_copy_pixels(pixaura_decode_context* context,
    const pixaura_decode_handle* image, uint64_t offset, uint8_t* output, uint64_t bytes);
PIXAURA_API int32_t pixaura_decode_copy_profile(pixaura_decode_context* context,
    const pixaura_decode_handle* source_or_image, uint64_t offset, uint8_t* output, uint64_t bytes);
PIXAURA_API int32_t pixaura_decode_release(pixaura_decode_context* context,
    const pixaura_decode_handle* handle);
#ifdef __cplusplus
}
#endif
#endif
