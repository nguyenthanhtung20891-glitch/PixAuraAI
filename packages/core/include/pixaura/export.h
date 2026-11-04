#ifndef PIXAURA_EXPORT_H
#define PIXAURA_EXPORT_H
#include "editor.h"
#ifdef __cplusplus
extern "C" {
#endif
/* EXPORT IS NOT A REVISION. One committed revision is rendered with the
 * existing evaluation and preview path, then encoded. No approval, candidate,
 * history mutation, provider call, or second editing implementation.
 */
#define PIXAURA_EXPORT_JPEG 1u
#define PIXAURA_EXPORT_PNG 2u
#define PIXAURA_EXPORT_EXACT 0u
#define PIXAURA_EXPORT_FIT 1u
#define PIXAURA_EXPORT_IDLE 0u
#define PIXAURA_EXPORT_VALIDATING 1u
#define PIXAURA_EXPORT_RENDERING 2u
#define PIXAURA_EXPORT_ENCODING 3u
#define PIXAURA_EXPORT_READY_TO_SAVE 4u
#define PIXAURA_EXPORT_SAVING 5u
#define PIXAURA_EXPORT_SUCCEEDED 6u
#define PIXAURA_EXPORT_FAILED 7u
#define PIXAURA_EXPORT_CANCELLED 8u
#define PIXAURA_EXPORT_STALE 9u
typedef struct pixaura_export_request {
    uint32_t version, struct_size, format, quality, mode, max_width, max_height, reserved;
    uint64_t epoch;
    char revision_id[32];
} pixaura_export_request;
PIXAURA_API int32_t pixaura_export_bind(uint32_t version, pixaura_editor_context*,
    pixaura_export_request* request);
PIXAURA_API int32_t pixaura_export_encode(uint32_t version, pixaura_editor_context*,
    const pixaura_export_request*, const pixaura_decode_handle* cancellation,
    uint8_t* output, uint64_t capacity, uint64_t* bytes, uint32_t* width, uint32_t* height,
    uint32_t* state);
PIXAURA_API int32_t pixaura_export_cancel(pixaura_editor_context*);
#ifdef __cplusplus
}
#endif
#endif
