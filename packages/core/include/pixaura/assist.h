#ifndef PIXAURA_ASSIST_H
#define PIXAURA_ASSIST_H
#include "editor.h"
#ifdef __cplusplus
extern "C" {
#endif
enum { PIXAURA_ASSIST_CHOICES=11 };
/* ADVICE-only transport. No approval, plan, execution or persistence surface.
 * Choice values are explicit reference request options, never image analysis.
 * Caller owns buffers. Stop -> join ALL calls -> destroy editor remains required. */
typedef struct pixaura_assist_request {
    uint32_t api_version,struct_size,choice_index,reserved;
} pixaura_assist_request;
typedef struct pixaura_assist_advice {
    uint32_t api_version,struct_size,choice_index,tool_index,parameter_count,reserved;
    int32_t values[4];
    uint8_t context_id[32];
    uint64_t request_serial,source_serial;
} pixaura_assist_advice;
/* Immutable native choices in stable Structured Intent goal order. Catalog
 * records have zero serials and cannot pass current(). Unknown index rejects. */
PIXAURA_API int32_t pixaura_assist_choice(uint32_t,uint32_t,pixaura_assist_advice*);
/* Captures trusted current source, requests reference ADVICE, validates frozen
 * Envelope v1 + Registry v1, fences cancellation/source change, returns data only.
 * One synchronous request per editor; BUSY=8, no queue. No output on failure. */
PIXAURA_API int32_t pixaura_editor_assist_request(uint32_t,pixaura_editor_context*,
    const pixaura_assist_request*,pixaura_assist_advice*);
/* Read-only applicability inspection. Does not select a tool or apply values.
 * Forged/changed/foreign/stale records reject. A record is never a capability. */
PIXAURA_API int32_t pixaura_editor_assist_current(uint32_t,pixaura_editor_context*,const pixaura_assist_advice*);
PIXAURA_API int32_t pixaura_editor_assist_cancel(uint32_t,pixaura_editor_context*);
#ifdef __cplusplus
}
#endif
#endif
