#ifndef PIXAURA_DYNAMIC_POLICY_H
#define PIXAURA_DYNAMIC_POLICY_H
#include "budget.h"
#include "preview.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PIXAURA_DYNAMIC_POLICY_VERSION 1u
#define PIXAURA_POLICY_RECOVERY_OBSERVATIONS 3u
#define PIXAURA_POLICY_CONTROL_RESERVE_BYTES UINT64_C(1048576)
/* Separate versioned family: Capability v1 enums and wire bytes are unchanged. */
enum pixaura_policy_pressure { PIXAURA_POLICY_UNKNOWN, PIXAURA_POLICY_NORMAL,
    PIXAURA_POLICY_ELEVATED, PIXAURA_POLICY_SEVERE, PIXAURA_POLICY_CRITICAL };
enum pixaura_policy_reason { PIXAURA_POLICY_BASELINE, PIXAURA_POLICY_THERMAL,
    PIXAURA_POLICY_MEMORY, PIXAURA_POLICY_CONSTRAINED, PIXAURA_POLICY_BACKEND,
    PIXAURA_POLICY_BUDGET, PIXAURA_POLICY_UNKNOWN_FACTS };
typedef struct pixaura_policy_observation {
    uint32_t version, struct_size, thermal, memory_pressure, constrained, gpu_availability;
    uint32_t gpu_validation, reserved;
    uint64_t process_budget_bytes, sequence;
} pixaura_policy_observation;
typedef struct pixaura_execution_strategy {
    uint32_t version, struct_size, tier, preview_edge, concurrency, backend;
    uint32_t forced_cpu, reason, recovery_pending, stable_observations;
    uint64_t epoch, runtime_cap_bytes;
} pixaura_execution_strategy;
/* Internal trusted adapter seam, not a diagnostic deserializer. Configure once
 * per raster lifetime under the existing platform display owner lock. Static
 * facts are copied and never replaced. No probes, queues, persistence or caches.
 * Live calls serialize under PRV1's cancellation mutex. Stop/join/quiesce before
 * destroy remains mandatory; racing actual destruction is never supported.
 * Failure preserves outputs/state. 1 malformed, 2 version/size, 3 stale context,
 * 8 nonwrapping epoch exhaustion, 9 stale observation, 13 revoked/stopped work.
 */
PIXAURA_API int32_t pixaura_policy_configure(uint32_t version, pixaura_decode_context*,
    const pixaura_capability_profile*, const pixaura_policy_observation*, pixaura_execution_strategy*);
PIXAURA_API int32_t pixaura_policy_update(uint32_t version, pixaura_decode_context*,
    const pixaura_policy_observation*, pixaura_execution_strategy*);
PIXAURA_API int32_t pixaura_policy_query(uint32_t version, pixaura_decode_context*, pixaura_execution_strategy*);
/* Truthful owner occupancy is mandatory. A query is not a reservation. */
PIXAURA_API int32_t pixaura_policy_admit(uint32_t version, pixaura_decode_context*,
    const pixaura_budget_request*, pixaura_budget_result*, uint64_t* epoch);
/* Complements, never replaces, native-current + platform check/display lock. */
PIXAURA_API int32_t pixaura_policy_current(uint32_t version, pixaura_decode_context*,
    uint64_t epoch, const pixaura_preview_ticket*, const pixaura_decode_handle*);
#ifdef __cplusplus
}
#endif
#endif
