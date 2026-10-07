#ifndef PIXAURA_PREVIEW_H
#define PIXAURA_PREVIEW_H
#include "evaluation.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PIXAURA_PREVIEW_API_VERSION 1u
#define PIXAURA_PREVIEW_RGBA8_SRGB_STRAIGHT_V1 1u
/* Derived SDR reference only. Shares context and combined 64-handle cap.
 * No persistence, raw pointers or implicit handle eviction.
 * All failures preserve outputs. Destroy requires quiescence.
 * Optional cancellation is a same-context Step 7 token.
 */
typedef struct pixaura_preview_metadata {
    uint32_t api_version, struct_size, width, height, pixel_format, reserved;
    uint64_t row_stride, image_bytes;
} pixaura_preview_metadata;
#define PIXAURA_PREVIEW_REQUEST_VERSION 1u
#define PIXAURA_PREVIEW_EXACT 0u
#define PIXAURA_PREVIEW_FIT 1u
typedef struct pixaura_preview_request {
    uint32_t version, struct_size, mode, max_width, max_height, reserved;
} pixaura_preview_request;
/* Interactive identities are values, never handles or owning pointers.
 * begin/cancel/stop bypass the ordinary render mutex. stop then join before destroy.
 * A platform owner serializes begin/cancel/check+display replacement with one lock.
 */
#define PIXAURA_PREVIEW_TICKET_VERSION 1u
#define PIXAURA_PREVIEW_TICKET_KIND 0x50525631u
typedef struct pixaura_preview_ticket {
    uint32_t version, struct_size, kind, reserved;
    uint8_t context_id[32];
    uint64_t generation;
} pixaura_preview_ticket;
PIXAURA_API int32_t pixaura_preview_begin(pixaura_decode_context*, uint32_t version, pixaura_preview_ticket*);
PIXAURA_API int32_t pixaura_preview_cancel(pixaura_decode_context*, const pixaura_preview_ticket*);
PIXAURA_API int32_t pixaura_preview_stop(pixaura_decode_context*);
/* Optional preview binds eligibility to the actual published native result. */
PIXAURA_API int32_t pixaura_preview_current(pixaura_decode_context*, const pixaura_preview_ticket*, const pixaura_decode_handle* preview);
PIXAURA_API int32_t pixaura_preview_render_interactive(pixaura_decode_context*, const pixaura_decode_handle* working,
    const pixaura_preview_ticket*, const pixaura_preview_request*, pixaura_decode_handle* output);
PIXAURA_API int32_t pixaura_preview_render(pixaura_decode_context* context,
    const pixaura_decode_handle* working, const pixaura_preview_request* request,
    const pixaura_decode_handle* cancellation, pixaura_decode_handle* output);
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
