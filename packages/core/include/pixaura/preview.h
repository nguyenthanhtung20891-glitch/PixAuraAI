#ifndef PIXAURA_PREVIEW_H
#define PIXAURA_PREVIEW_H
#include "evaluation.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PIXAURA_PREVIEW_API_VERSION 1u
#define PIXAURA_PREVIEW_RGBA8_SRGB_STRAIGHT_V1 1u
/* Derived 1:1 SDR reference only. Shares context and combined 64-handle cap.
 * No resize, persistence, raw pointers or implicit handle eviction.
 * All failures preserve outputs. Destroy requires quiescence.
 * Optional cancellation is a same-context Step 7 token.
 */
typedef struct pixaura_preview_metadata {
    uint32_t api_version, struct_size, width, height, pixel_format, reserved;
    uint64_t row_stride, image_bytes;
} pixaura_preview_metadata;
PIXAURA_API int32_t pixaura_preview_create(pixaura_decode_context* context,
    const pixaura_decode_handle* working, uint32_t version,
    const pixaura_decode_handle* cancellation, pixaura_decode_handle* output);
PIXAURA_API int32_t pixaura_preview_query(pixaura_decode_context* context,
    const pixaura_decode_handle* preview, pixaura_preview_metadata* output);
PIXAURA_API int32_t pixaura_preview_copy(pixaura_decode_context* context,
    const pixaura_decode_handle* preview, uint64_t offset, uint8_t* output, uint64_t bytes);
PIXAURA_API int32_t pixaura_preview_release(pixaura_decode_context* context,
    const pixaura_decode_handle* preview);
#ifdef __cplusplus
}
#endif
#endif
