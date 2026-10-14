#ifndef PIXAURA_INFERENCE_H
#define PIXAURA_INFERENCE_H
#include "document.h"
#include "model_package.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PIXAURA_INFERENCE_VERSION 1u
enum pixaura_inference_status {
    PIXAURA_INFERENCE_OK, PIXAURA_INFERENCE_INVALID_ARGUMENT, PIXAURA_INFERENCE_UNSUPPORTED_ABI,
    PIXAURA_INFERENCE_UNVERIFIED_MODEL, PIXAURA_INFERENCE_PROVIDER_UNAVAILABLE,
    PIXAURA_INFERENCE_MODEL_UNSUPPORTED, PIXAURA_INFERENCE_MODEL_INVALID,
    PIXAURA_INFERENCE_TASK_UNSUPPORTED, PIXAURA_INFERENCE_INPUT_SHAPE,
    PIXAURA_INFERENCE_INPUT_TYPE, PIXAURA_INFERENCE_INPUT_RANGE, PIXAURA_INFERENCE_RESOURCE,
    PIXAURA_INFERENCE_CANCELLED, PIXAURA_INFERENCE_OUTPUT_INVALID,
    PIXAURA_INFERENCE_STALE_HANDLE, PIXAURA_INFERENCE_INTERNAL, PIXAURA_INFERENCE_BUSY,
    PIXAURA_INFERENCE_BUFFER_TOO_SMALL, PIXAURA_INFERENCE_NORMALIZATION
};
typedef pixaura_document_context pixaura_inference_context;
typedef struct pixaura_inference_handle {
    uint32_t version, struct_size, kind, reserved;
    uint8_t context_id[32];
    uint64_t serial;
} pixaura_inference_handle;
typedef struct pixaura_inference_tensor {
    uint32_t version, struct_size, type, layout, rank, reserved;
    uint32_t dimensions[4];
    uint32_t normalization[6];
    uint64_t bytes;
} pixaura_inference_tensor;
typedef struct pixaura_inference_request {
    uint32_t version, struct_size, task, pre_cancel;
    uint64_t generation;
    pixaura_inference_tensor input;
} pixaura_inference_request;
typedef struct pixaura_inference_output {
    uint32_t version, struct_size;
    uint64_t generation;
    pixaura_inference_tensor tensor;
    uint8_t manifest_sha256[32];
} pixaura_inference_output;
typedef struct pixaura_inference_descriptor {
    uint32_t version, struct_size, provider_version, backend, task, type, layout;
    uint32_t deterministic, availability, validation, max_handles, max_dimension;
    uint64_t max_context_bytes;
    char provider_id[32], profile_id[64];
} pixaura_inference_descriptor;
typedef struct pixaura_inference_resources {
    uint32_t version, struct_size, handles, active;
    uint64_t ceiling, reserved_bytes;
} pixaura_inference_resources;
/* See inference-provider-v1.md. Stable accessible caller storage, fresh identity,
 * exclusive init/destroy; no caller storage races/aliases. Other calls synchronized.
 * kind 1 verified package, 2 prepared model, 3 owned output. Tokens contain no pointer.
 * All structs/input buffers require truthful lengths. No exceptions cross C.
 */
PIXAURA_API int32_t pixaura_inference_descriptor_query(uint32_t, const char*, uint64_t,
    pixaura_inference_descriptor*, uint32_t);
PIXAURA_API int32_t pixaura_inference_context_init(uint32_t, pixaura_inference_context*, uint32_t,
    const uint8_t*, uint64_t, uint64_t);
PIXAURA_API int32_t pixaura_inference_context_destroy(pixaura_inference_context*);
/* Verification only: delegates Step 1. No raw-path execution API. */
PIXAURA_API int32_t pixaura_inference_package_verify(pixaura_inference_context*, const char*, uint64_t,
    pixaura_inference_handle*, uint32_t, pixaura_model_diagnostic*);
PIXAURA_API int32_t pixaura_inference_prepare(pixaura_inference_context*, const char*, uint64_t,
    const pixaura_inference_handle*, pixaura_inference_handle*, uint32_t);
PIXAURA_API int32_t pixaura_inference_execute(pixaura_inference_context*, const pixaura_inference_handle*,
    const pixaura_inference_request*, uint32_t, const uint8_t*, uint64_t,
    pixaura_inference_handle*, uint32_t);
PIXAURA_API int32_t pixaura_inference_cancel(pixaura_inference_context*, const pixaura_inference_handle*, uint64_t);
PIXAURA_API int32_t pixaura_inference_output_info(pixaura_inference_context*, const pixaura_inference_handle*,
    pixaura_inference_output*, uint32_t);
PIXAURA_API int32_t pixaura_inference_output_copy(pixaura_inference_context*, const pixaura_inference_handle*,
    uint8_t*, uint64_t);
PIXAURA_API int32_t pixaura_inference_release(pixaura_inference_context*, const pixaura_inference_handle*);
PIXAURA_API int32_t pixaura_inference_resources_query(pixaura_inference_context*, pixaura_inference_resources*, uint32_t);
#ifdef __cplusplus
}
#endif
#endif
