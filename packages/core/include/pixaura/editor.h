#ifndef PIXAURA_EDITOR_H
#define PIXAURA_EDITOR_H
#include "manual.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Application-service composition, not an editing engine. Background-only IO,
 * approval and rendering. Stable caller-owned storage, fresh OS identity, and
 * stop -> join -> destroy lifetime policy, as for existing explicit contexts.
 * Platforms serialize lifecycle AND current+display replacement with one owner
 * lock; rendering/copying occurs outside it. No scheduler or display owner here.
 */
typedef pixaura_document_context pixaura_editor_context;
typedef int64_t (*pixaura_editor_reader)(void*, uint8_t*, uint64_t);
typedef struct pixaura_editor_request {
    pixaura_preview_ticket ticket;
    pixaura_manual_gesture gesture;
    pixaura_document_handle live;
    uint64_t sequence;
} pixaura_editor_request;
/* root must exist, be canonical/app-private, <=1024 UTF-8 bytes. import reader
 * streams <=64KiB per callback; negative means IO failure. bytes=0 reopens ONLY
 * schema 3; new import explicitly provisions 1->2->3 before creating document.
 * No automatic migration of existing projects. Failure leaves context untouched.
 */
PIXAURA_API int32_t pixaura_editor_open(uint32_t, pixaura_editor_context*,
    const uint8_t* root,uint64_t root_bytes,const uint8_t* identity,uint64_t identity_bytes,
    pixaura_editor_reader,void* reader_state,uint64_t import_bytes);
/* Descriptor index in existing ASCII registry order. Tool selection cancels A
 * before B; defaults/versions and parameter names come from that registry. */
PIXAURA_API int32_t pixaura_editor_select(pixaura_editor_context*,uint32_t index);
PIXAURA_API int32_t pixaura_editor_update(pixaura_editor_context*,const int32_t* values,uint32_t count);
/* 0 cancel, 1 explicit approve+persist, 2 undo, 3 redo, 4 interruption.
 * Commit persists the detached Step 6 proposal, retrying the same proposal after
 * storage failure (never another gesture commit). Stale storage failure cancels.
 */
PIXAURA_API int32_t pixaura_editor_action(pixaura_editor_context*,uint32_t action);
PIXAURA_API int32_t pixaura_editor_state(pixaura_editor_context*,uint8_t*,uint64_t,uint64_t*);
PIXAURA_API int32_t pixaura_editor_request_preview(pixaura_editor_context*,pixaura_editor_request*);
/* Max fit dimensions 1024. Caller buffer <=4MiB; preserves outputs on failure.
 * Candidate remains non-owning request+detached pixels. current MUST precede
 * actual platform display replacement under the same platform lifecycle lock.
 */
PIXAURA_API int32_t pixaura_editor_render(pixaura_editor_context*,const pixaura_editor_request*,
    uint32_t max_width,uint32_t max_height,uint8_t*,uint64_t,uint32_t* width,uint32_t* height);
PIXAURA_API int32_t pixaura_editor_current(pixaura_editor_context*,const pixaura_editor_request*);
PIXAURA_API int32_t pixaura_editor_stop(pixaura_editor_context*);
PIXAURA_API int32_t pixaura_editor_destroy(pixaura_editor_context*);
#ifdef __cplusplus
}
#endif
#endif
