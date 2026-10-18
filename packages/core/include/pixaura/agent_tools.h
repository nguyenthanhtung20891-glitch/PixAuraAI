#ifndef PIXAURA_AGENT_TOOLS_H
#define PIXAURA_AGENT_TOOLS_H
#include "agent_schema.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PIXAURA_AGENT_TOOLS_API_VERSION 1u
#define PIXAURA_AGENT_TOOLS_REGISTRY_VERSION 1u
#define PIXAURA_AGENT_TOOLS_MAX_REGISTRY_BYTES 32768u
#define PIXAURA_AGENT_TOOLS_MAX_BINDING_BYTES 9216u
/* Immutable editing metadata only. No hardware, inference, registration,
 * execution or approval authority. Exact registry version required.
 * Caller-owned disjoint buffers; query/short output returns 11 with required
 * bytes; all other failures preserve all outputs. JSON includes terminal LF.
 */
PIXAURA_API int32_t pixaura_agent_tools_registry(uint32_t api_version,
    uint32_t registry_version, uint8_t* output, uint64_t capacity, uint64_t* required);
/* Canonical PLAN input only: request.kind=2 and require_canonical=1.
 * Same 32-byte schema request, ownership, 8-action/8192-byte limits and context
 * cancellation fence. Receipt embeds the EXACT plan and says not_approved.
 * No bound handles, executable closures, retained state or document effects.
 */
PIXAURA_API int32_t pixaura_agent_tools_bind(uint32_t api_version,
    uint32_t registry_version, pixaura_document_context*,
    const pixaura_document_handle* live, const pixaura_agent_schema_request*,
    const uint8_t* canonical_plan, uint64_t bytes,
    uint8_t* output, uint64_t capacity, uint64_t* required);
#ifdef __cplusplus
}
#endif
#endif
