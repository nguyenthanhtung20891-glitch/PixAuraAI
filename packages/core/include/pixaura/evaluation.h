#ifndef PIXAURA_EVALUATION_H
#define PIXAURA_EVALUATION_H
#include "working.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PIXAURA_EVALUATION_API_VERSION 1u
#define PIXAURA_EVALUATION_MAX_OPERATIONS 256u
#define PIXAURA_EVALUATION_MAX_REQUEST_BYTES 65536u
#define PIXAURA_EVALUATION_MAX_OPERATION_BYTES 1024u
/* Version 1 request: {"operations":[schema-1 operation records in stack order]}.
 * Registered geometry/tone/detail /1/1 execute; unknown tuples reject, never skip.
 * Strict integer JSON; no fractional/exponent/non-finite parameters.
 * Validation uses the document parser and completes before raster allocation.
 * Context lifecycle/thread rules and release/query/copy are the decode/working API.
 * One independently owned output raster; reusable rotation bitmap/detail input-row scratch <=1 MiB.
 * Crop compacts without allocation; retained capacity is admitted/counts fully. All failures
 * preserve output. Numerical overflow returns INVALID_PARAMETERS (7).
 */
PIXAURA_API int32_t pixaura_evaluation_validate(uint32_t version,
    const uint8_t* request, uint64_t bytes);
PIXAURA_API int32_t pixaura_working_evaluate(pixaura_decode_context* context,
    const pixaura_decode_handle* source, uint32_t version,
    const uint8_t* request, uint64_t bytes, const pixaura_working_limits* limits,
    pixaura_decode_handle* output);
#ifdef __cplusplus
}
#endif
#endif
