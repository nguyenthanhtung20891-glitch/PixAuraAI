#ifndef PIXAURA_REVIEW_EDIT_H
#define PIXAURA_REVIEW_EDIT_H
#include "review_inspection.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PIXAURA_REVIEW_EDIT_KIND 0x52454431u
#define PIXAURA_REVIEW_CHANGE_VALUE 1u
#define PIXAURA_REVIEW_REMOVE_ACTION 2u
#define PIXAURA_REVIEW_MOVE_ACTION 3u
#define PIXAURA_REVIEW_RESTORE_ORIGINAL 4u
#define PIXAURA_REVIEW_ORIGINAL 1u
#define PIXAURA_REVIEW_EDITED 2u
#define PIXAURA_REVIEW_NO_ACTIONS 3u
#define PIXAURA_REVIEW_EDIT_FAILED 4u
/* Ephemeral binding, not execution authority. No pointer or release obligation. */
typedef struct pixaura_review_edit_ticket {
    uint32_t api_version,struct_size,kind,reserved;
    uint64_t serial,source_serial,repository_epoch;
    char context_id[32],plan_sha256[64];
} pixaura_review_edit_ticket;
typedef struct pixaura_review_edit_request {
    uint32_t api_version,struct_size,operation,action_index;
    uint32_t parameter_index,destination;
    int32_t raw_value;
    uint32_t reserved;
    pixaura_review_edit_ticket ticket;
    char tool_id[32],parameter_name[32];
    pixaura_review_action expected_action;
} pixaura_review_edit_request;
typedef struct pixaura_review_edit_result {
    uint32_t api_version,struct_size,state,reserved;
    pixaura_review_edit_ticket ticket;
    pixaura_auto_proposal review;
} pixaura_review_edit_result;
/* Begin retains one immutable original Plan (<=8192 bytes). Current Plan stays
 * in existing review authority. Only one session; repeated begin is BUSY until
 * that session is abandoned by the existing review lifecycle. */
PIXAURA_API int32_t pixaura_review_edit_begin(uint32_t,pixaura_editor_context*,
    const pixaura_review_inspection*,pixaura_review_edit_result*);
/* One try-lock admission, no queue/retry. Valid owner/ticket admission permanently
 * invalidates old review/inspection/preview before candidate construction. Failed
 * admitted edits leave only explicit RESTORE eligible, never the old review.
 * Output is atomic; query current with the same ticket after a failed edit.
 * Zero actions returns success/NO_ACTIONS, never an executable empty Plan. */
PIXAURA_API int32_t pixaura_review_edit(uint32_t,pixaura_editor_context*,
    const pixaura_review_edit_request*,pixaura_review_edit_result*);
PIXAURA_API int32_t pixaura_review_edit_current(uint32_t,pixaura_editor_context*,
    const pixaura_review_edit_ticket*,pixaura_review_edit_result*);
#ifdef __cplusplus
}
#endif
#endif
