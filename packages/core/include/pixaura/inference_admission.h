#ifndef PIXAURA_INFERENCE_ADMISSION_H
#define PIXAURA_INFERENCE_ADMISSION_H
#include "inference.h"
#include "dynamic_policy.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PIXAURA_INFERENCE_ADMISSION_VERSION 1u
enum pixaura_inference_admission_reason {
    PIXAURA_ADMISSION_PRIMARY, PIXAURA_ADMISSION_CPU_ONLY,
    PIXAURA_ADMISSION_FALLBACK_UNAVAILABLE, PIXAURA_ADMISSION_FALLBACK_TRUST,
    PIXAURA_ADMISSION_FALLBACK_CAPABILITY, PIXAURA_ADMISSION_FALLBACK_RESOURCE,
    PIXAURA_ADMISSION_FALLBACK_MODEL, PIXAURA_ADMISSION_REJECT_INPUT,
    PIXAURA_ADMISSION_REJECT_MODEL, PIXAURA_ADMISSION_REJECT_RESOURCE,
    PIXAURA_ADMISSION_REJECT_CANCELLED, PIXAURA_ADMISSION_REJECT_STALE,
    PIXAURA_ADMISSION_REJECT_BUSY, PIXAURA_ADMISSION_REJECT_UNCONFIGURED,
    PIXAURA_ADMISSION_REJECT_SECURITY, PIXAURA_ADMISSION_REJECT_TRUST
};
typedef struct pixaura_inference_admission_result {
    uint32_t version, struct_size, admitted, reason, status, tier;
    uint64_t epoch, generation, reservation_bytes, runtime_cap_bytes, peak_bytes;
    char provider_id[32];
    uint8_t manifest_sha256[32];
} pixaura_inference_admission_result;
/* TRUSTED INTERNAL adapter seam, like Phase 4 policy_configure: immutable facts
 * from the existing platform capability service, never diagnostic/user input.
 * No provider-trust argument: only the reviewed native provider registry grants
 * provider validation. Platform facts cannot promote inference-provider trust.
 * Configure once. Update reuses Phase 4 sequencing/hysteresis; query is read-only.
 * These calls return existing Phase 4 status codes, NOT inference status codes.
 */
PIXAURA_API int32_t pixaura_inference_policy_configure(uint32_t, pixaura_inference_context*,
    const pixaura_capability_profile*, const pixaura_policy_observation*, pixaura_execution_strategy*);
PIXAURA_API int32_t pixaura_inference_policy_update(uint32_t, pixaura_inference_context*,
    const pixaura_policy_observation*, pixaura_execution_strategy*);
PIXAURA_API int32_t pixaura_inference_policy_query(uint32_t, pixaura_inference_context*, pixaura_execution_strategy*);
/* Atomic validation, preparation, input snapshot and reservation. Kind 4 is a
 * one-shot admission handle, not a prepared-model handle. No caller estimates,
 * provider ID, trust claims, reduced dimensions or raw paths. One pending/running
 * request per context. Inputs accessible/stable/nonoverlapping during this call.
 * Ordinary policy rejection writes bounded result, preserves output handle;
 * malformed ABI/pointers preserve both outputs. Returns existing inference codes.
 */
PIXAURA_API int32_t pixaura_inference_admit(pixaura_inference_context*,
    const pixaura_inference_handle*, const pixaura_inference_request*, uint32_t,
    const uint8_t*, uint64_t, pixaura_inference_handle*, uint32_t,
    pixaura_inference_admission_result*, uint32_t);
PIXAURA_API int32_t pixaura_inference_execute_admitted(pixaura_inference_context*,
    const pixaura_inference_handle*, pixaura_inference_handle*, uint32_t);
PIXAURA_API int32_t pixaura_inference_admission_cancel(pixaura_inference_context*, const pixaura_inference_handle*);
/* Publication eligibility only. Consumer's owner lock covers admission/cancel,
 * policy update, current check and installation, retaining its own request fences.
 * Already returned immutable outputs stay readable.
 */
PIXAURA_API int32_t pixaura_inference_admission_current(pixaura_inference_context*, const pixaura_inference_handle*);
#ifdef __cplusplus
}
#endif
#endif
