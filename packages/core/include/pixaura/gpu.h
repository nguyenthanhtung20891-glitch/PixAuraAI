#ifndef PIXAURA_GPU_H
#define PIXAURA_GPU_H
#include "core.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PIXAURA_GPU_VERSION 1u
#define PIXAURA_GPU_MAX_PIXELS 16384u
/* Internal platform adapter contract. Synchronous, worker-thread only. Buffers
 * borrowed for the call; input immutable, output private. Never publish here.
 * Backend returns 0 only after completed dispatch and successful readback.
 * 5 = unavailable/unsupported, 8 = allocation, 14 = pipeline/submit/readback.
 * valid returns nonzero while request generation is current and uncancelled.
 */
typedef int32_t (*pixaura_gpu_dispatch)(void*, const float*, float*, uint32_t, float, uint32_t);
typedef int32_t (*pixaura_gpu_current)(void*);
typedef struct pixaura_gpu_result {
    uint32_t version;
    uint32_t struct_size;
    int32_t backend_status;
    uint32_t used_gpu;
} pixaura_gpu_result;
/* Versioned bounded tile candidate. Source/output are distinct caller-owned
 * RGBA32F arrays (4*pixel_count floats). Caller serializes generation check and
 * later display installation under the existing Step 11 publication owner.
 * Failure leaves output/result untouched. Backend failure/parity mismatch uses
 * CPU reference if allow_fallback=1; cancellation never falls back.
 */
PIXAURA_API int32_t pixaura_gpu_tile(uint32_t version, const float* source,
    uint32_t pixel_count, int32_t milli_ev, pixaura_gpu_dispatch dispatch,
    void* backend, pixaura_gpu_current current, void* generation,
    uint32_t allow_fallback, float* output, pixaura_gpu_result* result);
PIXAURA_API int32_t pixaura_gpu_gain(uint32_t version, int32_t milli_ev, float* output);
/* Shared deterministic certification corpus. Call case_count then case(index).
 * Exactly 257 pixels/case, including signed zero, subnormals, alpha, negative,
 * >1 and large finite RGB. No photos, content or identifiers in evidence.
 */
#define PIXAURA_GPU_CASE_PIXELS 257u
PIXAURA_API uint32_t pixaura_gpu_case_count(void);
PIXAURA_API int32_t pixaura_gpu_case(uint32_t index, float* output,
    uint32_t capacity_pixels, int32_t* milli_ev);
/* Conservative hardware classification: unknown/virtual/CPU/software rejects.
 * device_type: 1 integrated, 2 discrete; all others reject.
 */
PIXAURA_API int32_t pixaura_gpu_hardware(uint32_t device_type, uint32_t vendor,
    const char* name, uint32_t name_bytes);
typedef struct pixaura_vulkan_capabilities {
    uint32_t api_version, queue_count, queue_flags;
    uint32_t invocations, group_size_x, group_count_x;
    uint32_t storage_range, push_bytes, storage_descriptors;
} pixaura_vulkan_capabilities;
PIXAURA_API int32_t pixaura_gpu_vulkan_capable(uint32_t version,
    const pixaura_vulkan_capabilities* capabilities);
#ifdef __cplusplus
}
#endif
#endif
