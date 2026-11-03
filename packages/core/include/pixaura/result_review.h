#ifndef PIXAURA_RESULT_REVIEW_H
#define PIXAURA_RESULT_REVIEW_H
#include "agent_execution.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Read-only view of ordinary history. Not a revision, approval, candidate,
 * preview ticket or publisher. RESULT REVIEW != NEW HISTORY. */
#define PIXAURA_RESULT_NONE 0u
#define PIXAURA_RESULT_CURRENT 1u
#define PIXAURA_RESULT_STALE 2u
#define PIXAURA_RESULT_UNCONFIRMED 3u
#define PIXAURA_RESULT_ABSENT 4u
#define PIXAURA_RESULT_HAS_PREVIOUS 1u
#define PIXAURA_RESULT_CAN_UNDO 2u
#define PIXAURA_RESULT_CAN_REDO 4u
#define PIXAURA_RESULT_PUBLICATION_VISIBLE 8u
typedef struct pixaura_result_review {
    uint32_t api_version,struct_size,state,flags;
    char document_id[32],previous_revision[32],result_revision[32],live_revision[32];
    uint32_t revision_count,geometry_changed;
} pixaura_result_review;
/* receipt may be null. A receipt is reconciled with live history; it never
 * creates, selects or publishes a revision. Output is atomic. */
PIXAURA_API int32_t pixaura_result_review_query(uint32_t,pixaura_editor_context*,
    const pixaura_agent_execution_receipt*,pixaura_result_review*);
/* Renders one ordinary committed revision into caller memory and releases the
 * native image before return. revision_id is exactly 32 lowercase hex bytes.
 * Preview pixels and preview tickets are not accepted as revision identity. */
PIXAURA_API int32_t pixaura_result_review_render(uint32_t,pixaura_editor_context*,
    const pixaura_result_review*,const char* revision_id,uint32_t max_width,uint32_t max_height,
    uint8_t* output,uint64_t capacity,uint32_t* width,uint32_t* height);
#ifdef __cplusplus
}
#endif
#endif
