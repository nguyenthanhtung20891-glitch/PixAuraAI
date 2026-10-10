#ifndef PIXAURA_MANUAL_H
#define PIXAURA_MANUAL_H
#include "document.h"
#include "preview.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PIXAURA_MANUAL_API_VERSION 1u
#define PIXAURA_MANUAL_MAX_DESCRIPTORS 16u
#define PIXAURA_MANUAL_MAX_PARAMETERS 8u
#define PIXAURA_MANUAL_MAX_REGISTRY_BYTES 32768u
/* Read-only compiled registry. No registration/discovery/network or handles.
 * Two-call UTF-8 JSON: required includes LF, excludes NUL. A short buffer writes
 * only required; all other failures preserve outputs. Caller owns truthful,
 * nonoverlapping buffers; no exceptions or retained input pointers cross C. */
PIXAURA_API int32_t pixaura_manual_registry_json(uint32_t version,
    uint8_t* output, uint64_t capacity, uint64_t* required);
/* Exactly one existing schema-1 operation in {"operations":[OPERATION]}.
 * The shared document parser rejects unknown tuples/types/keys and noninteger
 * notation. Success returns the canonical evaluation envelope, not approval. */
PIXAURA_API int32_t pixaura_manual_canonical_operation(uint32_t version,
    const uint8_t* request, uint64_t bytes, uint8_t* output,
    uint64_t capacity, uint64_t* required);
/* Geometry gestures share the document context's unchanged 64-owned-handle
 * budget. Tokens never alias document handles. Calls are serialized by that
 * context; init/destroy retain their exclusive lifetime requirements.
 * Only crop/rotate are admitted. One active gesture per document;
 * tool switching/background interruption must cancel then release.
 * Commit returns a detached proposal, never approval or durable publication.
 * Neutral commit returns the borrowed live handle (changed=0); changed commit
 * returns a new owned handle (changed=1). Failures preserve output/pending state
 * except stale binding, which cancels. Caller supplies fresh lowercase hex IDs.
 * Projection is an existing evaluation envelope, to be rendered through PRV1;
 * publication additionally requires current(sequence, live), under owner lock.
 */
typedef struct pixaura_manual_gesture {
    uint32_t api_version, struct_size, kind, reserved;
    uint8_t context_id[32];
    uint64_t serial;
} pixaura_manual_gesture;
PIXAURA_API int32_t pixaura_manual_geometry_begin(uint32_t version,
    pixaura_document_context* context, const pixaura_document_handle* live,
    const uint8_t* gesture_id, uint64_t gesture_bytes,
    const uint8_t* tool_id, uint64_t tool_bytes,
    const uint8_t* replace_id, uint64_t replace_bytes, pixaura_manual_gesture* output);
PIXAURA_API int32_t pixaura_manual_geometry_update(pixaura_document_context* context,
    const pixaura_manual_gesture* gesture, const pixaura_document_handle* live,
    const uint8_t* operation, uint64_t bytes, uint64_t* sequence);
PIXAURA_API int32_t pixaura_manual_geometry_projection(pixaura_document_context* context,
    const pixaura_manual_gesture* gesture, const pixaura_document_handle* live,
    uint8_t* output, uint64_t capacity, uint64_t* required, uint64_t* sequence);
PIXAURA_API int32_t pixaura_manual_geometry_current(pixaura_document_context* context,
    const pixaura_manual_gesture* gesture, const pixaura_document_handle* live, uint64_t sequence);
PIXAURA_API int32_t pixaura_manual_geometry_commit(pixaura_document_context* context,
    const pixaura_manual_gesture* gesture, const pixaura_document_handle* live,
    const uint8_t* revision_id, uint64_t revision_bytes,
    pixaura_document_handle* proposal, uint32_t* changed);
PIXAURA_API int32_t pixaura_manual_geometry_cancel(pixaura_document_context* context,
    const pixaura_manual_gesture* gesture);
PIXAURA_API int32_t pixaura_manual_geometry_release(pixaura_document_context* context,
    const pixaura_manual_gesture* gesture);
/* Worker-only composition of existing evaluation and PRV1. source must be the
 * verified immutable working original corresponding to live's source hash,
 * never the previously evaluated result. Optional sticky cancellation uses the
 * existing geometry token. Caller keeps both contexts alive until workers join.
 * Failure leaves output/sequence unchanged. This does not install a display;
 * owner must check BOTH PRV1 and geometry_current under its publication lock.
 */
PIXAURA_API int32_t pixaura_manual_geometry_render(pixaura_document_context* document,
    const pixaura_manual_gesture* gesture, const pixaura_document_handle* live,
    pixaura_decode_context* raster, const pixaura_decode_handle* source,
    const pixaura_working_limits* limits, const pixaura_decode_handle* cancellation,
    const pixaura_preview_ticket* ticket, const pixaura_preview_request* request,
    pixaura_decode_handle* output, uint64_t* sequence);
/* Shared Step 3 manual gestures use the same token/controller/PRV1 lifecycle.
 * Existing geometry_begin remains crop/rotate-only. Generic begin admits exactly
 * the compiled registry, with the unchanged shared ownership/commit rules. */
PIXAURA_API int32_t pixaura_manual_begin(uint32_t version,
    pixaura_document_context* context, const pixaura_document_handle* live,
    const uint8_t* gesture_id, uint64_t gesture_bytes,
    const uint8_t* tool_id, uint64_t tool_bytes,
    const uint8_t* replace_id, uint64_t replace_bytes, pixaura_manual_gesture* output);
PIXAURA_API int32_t pixaura_manual_update(pixaura_document_context* context,
    const pixaura_manual_gesture* gesture, const pixaura_document_handle* live,
    const uint8_t* operation, uint64_t bytes, uint64_t* sequence);
PIXAURA_API int32_t pixaura_manual_projection(pixaura_document_context* context,
    const pixaura_manual_gesture* gesture, const pixaura_document_handle* live,
    uint8_t* output, uint64_t capacity, uint64_t* required, uint64_t* sequence);
PIXAURA_API int32_t pixaura_manual_current(pixaura_document_context* context,
    const pixaura_manual_gesture* gesture, const pixaura_document_handle* live, uint64_t sequence);
PIXAURA_API int32_t pixaura_manual_commit(pixaura_document_context* context,
    const pixaura_manual_gesture* gesture, const pixaura_document_handle* live,
    const uint8_t* revision_id, uint64_t revision_bytes,
    pixaura_document_handle* proposal, uint32_t* changed);
PIXAURA_API int32_t pixaura_manual_cancel(pixaura_document_context* context,
    const pixaura_manual_gesture* gesture);
PIXAURA_API int32_t pixaura_manual_release(pixaura_document_context* context,
    const pixaura_manual_gesture* gesture);
/* Worker-only composition of existing evaluation and PRV1. source must be the
 * verified immutable working original corresponding to live's source hash,
 * never the previously evaluated result. Optional sticky cancellation uses the
 * existing geometry token. Caller keeps both contexts alive until workers join.
 * Failure leaves output/sequence unchanged. This does not install a display;
 * owner must check BOTH PRV1 and geometry_current under its publication lock.
 */
PIXAURA_API int32_t pixaura_manual_render(pixaura_document_context* document,
    const pixaura_manual_gesture* gesture, const pixaura_document_handle* live,
    pixaura_decode_context* raster, const pixaura_decode_handle* source,
    const pixaura_working_limits* limits, const pixaura_decode_handle* cancellation,
    const pixaura_preview_ticket* ticket, const pixaura_preview_request* request,
    pixaura_decode_handle* output, uint64_t* sequence);
#ifdef __cplusplus
}
#endif
#endif
