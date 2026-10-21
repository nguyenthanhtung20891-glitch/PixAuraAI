#ifndef PIXAURA_AGENT_EXECUTION_H
#define PIXAURA_AGENT_EXECUTION_H
#include "editor.h"
#include "agent.h"
#include "reference_planner.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PIXAURA_AGENT_EXECUTION_API_VERSION 1u
/* Trusted application boundary only. Never expose native authority to planners.
 * No input payload can assert human approval. All state is ephemeral. */
typedef struct pixaura_agent_approval {
    uint32_t api_version, struct_size, kind, reserved;
    uint8_t context_id[32];
    uint64_t serial;
} pixaura_agent_approval;
typedef struct pixaura_agent_execution_request {
    uint32_t api_version, struct_size, registry_version, pre_cancel;
    uint32_t reserved[4];
} pixaura_agent_execution_request;
typedef struct pixaura_agent_execution_receipt {
    uint32_t api_version, struct_size, approval_consumed, reserved;
    uint8_t plan_sha256[64], source_revision[32], resulting_revision[32];
} pixaura_agent_execution_receipt;
/* Inspect live authoritative binding for a future canonical Plan v1. */
PIXAURA_API int32_t pixaura_editor_agent_binding(uint32_t,pixaura_editor_context*,uint8_t*,uint64_t,uint64_t*);
/* Existing deterministic reference planner, using the editor's trusted live
 * context. Canonical structured intent in; ordinary unapproved Plan v1 out.
 * No internal document pointer, approval or editing authority is exposed. */
PIXAURA_API int32_t pixaura_editor_agent_plan(uint32_t,pixaura_editor_context*,const pixaura_reference_planner_request*,const uint8_t*,uint64_t,uint8_t*,uint64_t,uint64_t*);
/* Validate+bind only; copies exact canonical bytes for review, never approves. */
PIXAURA_API int32_t pixaura_editor_agent_validate(uint32_t,pixaura_editor_context*,const pixaura_agent_execution_request*,const uint8_t*,uint64_t,uint8_t*,uint64_t,uint64_t*);
/* Explicit human application confirmation. Validation is always repeated. */
PIXAURA_API int32_t pixaura_editor_agent_approve(uint32_t,pixaura_editor_context*,const pixaura_agent_execution_request*,const uint8_t*,uint64_t,pixaura_agent_approval*);
PIXAURA_API int32_t pixaura_editor_agent_revoke(uint32_t,pixaura_editor_context*,const pixaura_agent_approval*);
PIXAURA_API int32_t pixaura_editor_agent_release(uint32_t,pixaura_editor_context*,const pixaura_agent_approval*);
/* Null approval returns NOT_APPROVED=15. No output on failure. */
PIXAURA_API int32_t pixaura_editor_agent_execute(uint32_t,pixaura_editor_context*,const pixaura_agent_execution_request*,const pixaura_agent_approval*,const uint8_t*,uint64_t,pixaura_agent_execution_receipt*);
/* Revokes in-flight publication. Nonsticky for future calls. Context lifetime
 * remains stop -> join -> destroy, including concurrent cancellation calls. */
PIXAURA_API int32_t pixaura_editor_agent_cancel(uint32_t,pixaura_editor_context*);
#ifdef __cplusplus
}
#endif
#endif
