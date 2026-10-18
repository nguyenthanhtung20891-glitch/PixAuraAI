#ifndef PIXAURA_AGENT_H
#define PIXAURA_AGENT_H
#include "document.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PIXAURA_AGENT_API_VERSION 1u
#define PIXAURA_AGENT_MAX_ACTIONS 8u
#define PIXAURA_AGENT_MAX_BYTES 8192u
#define PIXAURA_AGENT_MAX_PROPOSALS 8u
#define PIXAURA_AGENT_NOT_APPROVED 15
/* Other statuses reuse document.h: 0 OK, 1 INVALID_ARGUMENT, 2 unsupported ABI,
 * 3 invalid context/handle, 5 unknown tool/version, 6 malformed envelope, 7 invalid parameters,
 * 8 resource, 9 stale, 11 short buffer, 13 cancelled, 14 internal. */
typedef struct pixaura_agent_proposal {
    uint32_t api_version, struct_size, kind, reserved;
    uint8_t context_id[32];
    uint64_t serial;
} pixaura_agent_proposal;
/* Trusted application boundary only, never directly exposed to a planner.
 * Reuses ADR 0009 document context and its 64 combined handle ceiling.
 * live is supplied by the existing application owner, not model output.
 * Stage accepts the EXISTING evaluation envelope (not an intent/plan schema).
 * 1..8 registered operations; copied canonical bytes; no executable payload.
 * No authority, approval token, transition, persistence or execution is created.
 * Calls synchronized by context; exclusive init/destroy and lifetime obligations
 * remain unchanged. All accessible, stable, truthful buffers must be disjoint.
 * Failures preserve outputs, except short/query inspect writes required bytes.
 * Serial/type/context checks prevent tokens becoming document/gesture handles.
 */
PIXAURA_API int32_t pixaura_agent_stage(uint32_t version,
    pixaura_document_context*, const pixaura_document_handle* live,
    const uint8_t* operations, uint64_t bytes, pixaura_agent_proposal* output);
PIXAURA_API int32_t pixaura_agent_validate(uint32_t version,
    pixaura_document_context*, const pixaura_agent_proposal*, const pixaura_document_handle* live);
/* Inspection returns complete canonical operation envelope, LF, no NUL.
 * Query/short buffer returns BUFFER_TOO_SMALL, no partial write. */
PIXAURA_API int32_t pixaura_agent_inspect(uint32_t version,
    pixaura_document_context*, const pixaura_agent_proposal*, const pixaura_document_handle* live,
    uint8_t* output, uint64_t capacity, uint64_t* required);
/* Closed gate: a valid proposal ALWAYS returns NOT_APPROVED. Step 1 has no
 * approval issuer or execution bridge. This function performs no editing. */
PIXAURA_API int32_t pixaura_agent_execution_authority(uint32_t version,
    pixaura_document_context*, const pixaura_agent_proposal*, const pixaura_document_handle* live);
PIXAURA_API int32_t pixaura_agent_cancel(uint32_t version,
    pixaura_document_context*, const pixaura_agent_proposal*);
PIXAURA_API int32_t pixaura_agent_release(uint32_t version,
    pixaura_document_context*, const pixaura_agent_proposal*);
#ifdef __cplusplus
}
#endif
#endif
