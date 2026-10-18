#ifndef PIXAURA_AGENT_SCHEMA_H
#define PIXAURA_AGENT_SCHEMA_H
#include "agent.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PIXAURA_AGENT_SCHEMA_API_VERSION 1u
#define PIXAURA_AGENT_SCHEMA_INTENT 1u
#define PIXAURA_AGENT_SCHEMA_PLAN 2u
typedef struct pixaura_agent_schema_request {
    uint32_t api_version, struct_size, kind, require_canonical, pre_cancel;
    uint32_t reserved[3];
} pixaura_agent_schema_request;
/* Caller-owned ephemeral bytes, not handles or approval. All buffers disjoint,
 * aligned, stable and truthful; context lifetime retains ADR 0009 obligations.
 * Existing statuses, 8192-byte cap and 8-action cap. Short/query copies nothing
 * and returns 11 with required length (LF included, NUL excluded).
 * No output on any other failure. One parse per context; contention returns 8.
 */
PIXAURA_API int32_t pixaura_agent_schema_binding(uint32_t version,
    pixaura_document_context*, const pixaura_document_handle* live,
    uint8_t* output, uint64_t capacity, uint64_t* required);
PIXAURA_API int32_t pixaura_agent_schema_canonicalize(uint32_t version,
    pixaura_document_context*, const pixaura_document_handle* live,
    const pixaura_agent_schema_request*, const uint8_t* input, uint64_t bytes,
    uint8_t* output, uint64_t capacity, uint64_t* required);
/* Revokes in-flight schema work in this context only. No sticky cancellation
 * for future calls, no proposal/document mutation, no epoch wrap/reuse. */
PIXAURA_API int32_t pixaura_agent_schema_cancel(uint32_t version,
    pixaura_document_context*);
#ifdef __cplusplus
}
#endif
#endif
