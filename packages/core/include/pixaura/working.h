#ifndef PIXAURA_WORKING_H
#define PIXAURA_WORKING_H
#include "decode.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PIXAURA_WORKING_API_VERSION 1u
#define PIXAURA_WORKING_RGBA32F_LINEAR_SRGB_PREMULTIPLIED 1u
/* Working handles share the explicit decode context lifetime/64-handle budget.
 * All failures preserve outputs. Init/destroy require caller quiescence.
 * Limits are positive, reducible hard ceilings; scratch is zero (no heap scratch).
 */
typedef struct pixaura_working_limits {
    uint32_t api_version, struct_size, width, height;
    uint64_t pixel_count, row_stride, image_bytes;
} pixaura_working_limits;
typedef struct pixaura_working_metadata {
    uint32_t api_version, struct_size, width, height, orientation, pixel_format;
    uint32_t source_orientation, source_profile_type;
    uint64_t row_stride, image_bytes;
} pixaura_working_metadata;
PIXAURA_API int32_t pixaura_working_default_limits(uint32_t version, pixaura_working_limits* output);
/* Only decoded-image handles accepted. ICC/non-sRGB annotations return 16.
 * No color transform is claimed on unsupported input. Originals/profile retained.
 */
PIXAURA_API int32_t pixaura_working_normalize(pixaura_decode_context* context,
    const pixaura_decode_handle* image, const pixaura_working_limits* limits, pixaura_decode_handle* output);
/* Bounded identity evaluation produces an independent immutable working image. */
PIXAURA_API int32_t pixaura_working_identity(pixaura_decode_context* context,
    const pixaura_decode_handle* working, const pixaura_working_limits* limits, pixaura_decode_handle* output);
PIXAURA_API int32_t pixaura_working_query(pixaura_decode_context* context,
    const pixaura_decode_handle* working, pixaura_working_metadata* output);
/* Editing-session admission: only the normalized immutable original, with exact
 * verified encoded SHA-256/length. Evaluated/identity candidates reject. */
PIXAURA_API int32_t pixaura_working_validate_original(pixaura_decode_context*,
    const pixaura_decode_handle*, const uint8_t* sha256, uint64_t bytes, uint64_t encoded_bytes);
/* Bounded liveness check for an already verified original binding. Uses only
 * the existing registry publication mutex, so supersession need not wait for
 * synchronous pixel work. Released/derived/foreign handles reject 3. */
PIXAURA_API int32_t pixaura_working_original_current(pixaura_decode_context*, const pixaura_decode_handle*);
/* Caller-owned float buffer; offset/count in float components, never internal pointers.
 * Source metadata/profile query uses decode_query/copy_profile; release uses decode_release.
 */
PIXAURA_API int32_t pixaura_working_copy(pixaura_decode_context* context,
    const pixaura_decode_handle* working, uint64_t offset, float* output, uint64_t count);
#ifdef __cplusplus
}
#endif
#endif
