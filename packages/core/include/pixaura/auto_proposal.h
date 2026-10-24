#ifndef PIXAURA_AUTO_PROPOSAL_H
#define PIXAURA_AUTO_PROPOSAL_H
#include "assist.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Read-only proposal transport, NOT approval or execution. Existing ownership:
 * stop -> join ALL calls -> destroy. No new handles or raised ceilings. */
typedef struct pixaura_auto_request {
    uint32_t api_version,struct_size,choice_index,kind; /* 2 structured, 3 Plan */
} pixaura_auto_request;
typedef struct pixaura_auto_action {
    uint32_t tool_index,parameter_count;
    int32_t values[4];
} pixaura_auto_action;
typedef struct pixaura_auto_proposal {
    uint32_t api_version,struct_size,action_count,kind,registry_version,reserved;
    uint8_t context_id[32];
    uint64_t request_serial,source_serial;
    char proposal_id[32],provider_id[32]; /* correlation only; zero terminated */
    pixaura_auto_action actions[8];
} pixaura_auto_proposal;
/* Structured requests use the existing explicit choice catalog and null payload.
 * Plan candidates supply canonical Plan v1 bytes (<=8192); choice_index is zero.
 * Both pass the frozen provider Envelope/Plan/Registry/binding validators.
 * One provider request per editor (shared with Assist), BUSY=8, no queue. */
PIXAURA_API int32_t pixaura_editor_auto_request(uint32_t,pixaura_editor_context*,
    const pixaura_auto_request*,const uint8_t*,uint64_t,pixaura_auto_proposal*);
PIXAURA_API int32_t pixaura_editor_auto_current(uint32_t,pixaura_editor_context*,const pixaura_auto_proposal*);
/* Caller-owned read-only canonical Plan bytes. Size query: null/0 -> 11.
 * Always rechecks current record/context/source. No approval or candidate. */
PIXAURA_API int32_t pixaura_editor_auto_plan(uint32_t,pixaura_editor_context*,const pixaura_auto_proposal*,uint8_t*,uint64_t,uint64_t*);
PIXAURA_API int32_t pixaura_editor_auto_cancel(uint32_t,pixaura_editor_context*);
#ifdef __cplusplus
}
#endif
#endif
