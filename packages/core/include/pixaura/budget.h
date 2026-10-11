#ifndef PIXAURA_BUDGET_H
#define PIXAURA_BUDGET_H
#include "capability.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PIXAURA_BUDGET_VERSION 1u
enum pixaura_budget_work { PIXAURA_BUDGET_IMPORT, PIXAURA_BUDGET_PREVIEW,
    PIXAURA_BUDGET_COMMIT, PIXAURA_BUDGET_GPU_DIAGNOSTIC };
enum pixaura_budget_tier { PIXAURA_T0, PIXAURA_T1, PIXAURA_T2 };
enum pixaura_budget_decision { PIXAURA_BUDGET_ADMIT, PIXAURA_BUDGET_DOWNGRADE,
    PIXAURA_BUDGET_REJECT };
enum pixaura_budget_backend { PIXAURA_BUDGET_CPU, PIXAURA_BUDGET_VULKAN,
    PIXAURA_BUDGET_METAL };
enum pixaura_budget_reason { PIXAURA_BUDGET_BASELINE, PIXAURA_BUDGET_HIGHER_TIER,
    PIXAURA_BUDGET_PREVIEW_BOUND, PIXAURA_BUDGET_PREVIEW_MEMORY,
    PIXAURA_BUDGET_BACKEND_UNAVAILABLE, PIXAURA_BUDGET_BACKEND_UNVALIDATED,
    PIXAURA_BUDGET_PRESSURE, PIXAURA_BUDGET_UNKNOWN_FACTS,
    PIXAURA_BUDGET_HARD_CEILING, PIXAURA_BUDGET_PROCESS_BUDGET,
    PIXAURA_BUDGET_OVERFLOW, PIXAURA_BUDGET_MALFORMED,
    PIXAURA_BUDGET_OCCUPANCY };
typedef struct pixaura_budget_request {
    uint32_t version, struct_size, work_class, bytes_per_pixel, raster_count;
    uint32_t prefer_gpu, allow_cpu_fallback, preview_edge;
    uint32_t retained_handles, inflight_handles, requested_handles, admitted_work_count;
    uint64_t width, height, encoded_bytes, scratch_bytes, temporary_bytes;
    uint64_t retained_bytes, inflight_bytes, context_limit_bytes;
} pixaura_budget_request;
typedef struct pixaura_budget_result {
    uint32_t version, struct_size, decision, tier, backend, reason;
    uint32_t max_concurrent_work, preview_edge, preview_width, preview_height;
    uint64_t runtime_cap_bytes, available_bytes, raster_bytes, peak_bytes;
} pixaura_budget_result;
/* Pure preflight, not a reservation, ownership token or certification authority.
 * Trusted internal adapters supply immutable profile + truthful owner occupancy.
 * Owners must serialize snapshot/admission/allocation and retain ALL existing
 * operation-specific checks. No externally loaded profiles, context access,
 * allocations, callbacks, state, persistence or dynamic orchestration.
 * ABI/profile errors preserve output (1 invalid, 2 unsupported).
 * Versioned requests with invalid fields return OK + REJECT/MALFORMED.
 * Resource/overflow rejection returns OK + REJECT with a stable reason.
 */
PIXAURA_API int32_t pixaura_budget_admit(uint32_t version,
    const pixaura_capability_profile* profile, const pixaura_budget_request* request,
    pixaura_budget_result* output);
#ifdef __cplusplus
}
#endif
#endif
