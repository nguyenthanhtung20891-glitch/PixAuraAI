#ifndef PIXAURA_REFERENCE_PLANNER_H
#define PIXAURA_REFERENCE_PLANNER_H
#include "agent_schema.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PIXAURA_REFERENCE_PLANNER_API_VERSION 1u
#define PIXAURA_REFERENCE_PLANNER_VERSION 1u
#define PIXAURA_REFERENCE_PLANNER_ID "reference.deterministic.v1"
#define PIXAURA_STRUCTURED_INTENT_MAX_BYTES 2048u
typedef struct pixaura_reference_planner_request {
    uint32_t api_version, struct_size, planner_version, registry_version, pre_cancel;
    uint32_t reserved[3];
} pixaura_reference_planner_request;
/* Separate structured-intent family; does not reinterpret free-form intent v1.
 * Canonicalizer accepts valid noncanonical structured JSON. Planner requires
 * canonical structured input and returns ordinary plan v1, NOT approval.
 * No handles, retained state or editing execution. Caller-owned disjoint ranges;
 * query/short output returns 11 and required length; other failures write nothing.
 * Cancellation uses pixaura_agent_schema_cancel and the same context parse slot.
 */
PIXAURA_API int32_t pixaura_structured_intent_canonicalize(uint32_t version,
    pixaura_document_context*, const pixaura_document_handle* live,
    const pixaura_reference_planner_request*, const uint8_t* input, uint64_t bytes,
    uint8_t* output, uint64_t capacity, uint64_t* required);
PIXAURA_API int32_t pixaura_reference_planner_plan(uint32_t version,
    pixaura_document_context*, const pixaura_document_handle* live,
    const pixaura_reference_planner_request*, const uint8_t* canonical_input, uint64_t bytes,
    uint8_t* output, uint64_t capacity, uint64_t* required);
#ifdef __cplusplus
}
#endif
#endif
