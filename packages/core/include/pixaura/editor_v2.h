#ifndef PIXAURA_EDITOR_V2_H
#define PIXAURA_EDITOR_V2_H
#include "core.h"
#include "dynamic_policy.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Additive family. No v1 handle or structure is accepted by these functions.
 * Platform workers own calls; stop -> join -> close. FD inputs are duplicated;
 * export destinations must be platform-owned private staging files. These calls
 * never acquire permissions or publish an external destination on the user's behalf.
 * The protected installation key is unrelated to Apple/store signing identities.
 */
#define PIXAURA_EDITOR_V2_VERSION 2u
typedef uint64_t pixaura_v2_handle;
typedef struct pixaura_v2_state {
    uint32_t version,struct_size,revisions,selected;
    uint64_t epoch,generation;
    uint32_t pending,can_undo,can_redo,recovery_only;
    uint32_t commerce_unavailable,value_count;
    int32_t values[4];
    char revision[33];
    uint8_t reserved[7];
} pixaura_v2_state;
typedef struct pixaura_v2_preview {
    uint32_t version,struct_size,width,height;
    uint32_t source_width,source_height;
    uint64_t bytes,frame;
} pixaura_v2_preview;
typedef struct pixaura_v2_advisory {
    uint32_t version,struct_size,choice,tool;
    uint64_t epoch;
    char id[33],delivery[65];
    uint8_t reserved[6];
} pixaura_v2_advisory;
/* Native inspection DTOs. Platforms display raw values; they never parse or
 * construct Plan JSON or infer operation semantics. These are versioned v2
 * records, not reinterpretations of v1 inspection/approval structures. */
typedef struct pixaura_v2_review_parameter {
    char name[32],unit[64];int32_t raw,minimum,maximum,neutral;
} pixaura_v2_review_parameter;
typedef struct pixaura_v2_review_action {
    uint32_t index,tool_index,operation_version,parameter_version,parameter_count,geometry;
    char tool_id[32],label[64];pixaura_v2_review_parameter parameters[4];
} pixaura_v2_review_action;
typedef struct pixaura_v2_review_inspection {
    uint32_t version,struct_size,action_count,warnings;uint64_t epoch;
    char review_id[33];uint8_t reserved[7];char provider_id[32];
    pixaura_v2_review_action actions[8];
} pixaura_v2_review_inspection;
/* Observation of an actual committed native event, never approval/execution
 * authority and never accepted as an input for trial/paid/quota grants. */
typedef struct pixaura_v2_commit_result {
    uint32_t version,struct_size,kind,reserved;
    uint64_t epoch;
    int64_t logical_utc;
    uint32_t clock_uncertain,utc_authenticated;
    char event[33],source[65],project[33],document[33],parent[33],revision[33],revision_digest[65];
    uint8_t tail_reserved[1];
} pixaura_v2_commit_result;
/* Export observation is bound under the native snapshot lock. It is never an
 * edit/revision/approval receipt and cannot start trial or authorize publication. */
typedef struct pixaura_v2_export_result {
    uint32_t version,struct_size,width,height,format,reserved;
    uint64_t bytes,epoch;
    char source[65],document[33],revision[33];
    uint8_t tail_reserved[5];
} pixaura_v2_export_result;
typedef struct pixaura_v2_commercial_state {
    uint32_t version,struct_size,access,clock_uncertain;
    uint32_t photos,ai,pending,reserved;
    int64_t day,logical_utc,trial_start,trial_end;
} pixaura_v2_commercial_state;
/* Read-only access: 0 Free, 1 trial, 2 verified subscription, 3 verified lifetime.
 * No checkout is enabled by a projection; failed verification never grants paid.
 */
PIXAURA_API int32_t pixaura_v2_runtime_commercial_state(uint32_t,pixaura_v2_handle,pixaura_v2_commercial_state*);
/* One runtime per app-private commerce root, shared by all open documents. It
 * owns ONE aggregate 128MiB v2 budget and the commercial coordinator. A cache
 * failure leaves recovery available and commercial editing fail-closed.
 * No public API supplies paid grants, timestamps, trial starts or store facts.
 */
PIXAURA_API int32_t pixaura_v2_runtime_create(uint32_t,const char*,uint64_t,const uint8_t*,uint64_t,pixaura_v2_handle*);
/* Trusted platform snapshots only, never deserialized external diagnostics.
 * A lower runtime cap stops existing editors. Close their workers/frames and
 * recreate the runtime with the new snapshot; retained leases are never erased.
 * CPU-only regardless of detected GPU capability/provider state. */
/* Explicit key-loss recovery: never creates/opens a commercial ledger, grants
 * new editing or authenticates a receipt. Saved inspection/export only. Optional
 * profile/observation must be paired; normal resource admission still applies. */
PIXAURA_API int32_t pixaura_v2_runtime_recovery_create(uint32_t,const char*,uint64_t,const pixaura_capability_profile*,const pixaura_policy_observation*,pixaura_execution_strategy*,pixaura_v2_handle*);
PIXAURA_API int32_t pixaura_v2_runtime_create_policy(uint32_t,const char*,uint64_t,const uint8_t*,uint64_t,const pixaura_capability_profile*,const pixaura_policy_observation*,pixaura_execution_strategy*,pixaura_v2_handle*);
PIXAURA_API int32_t pixaura_v2_runtime_strategy(uint32_t,pixaura_v2_handle,pixaura_execution_strategy*);
PIXAURA_API int32_t pixaura_v2_runtime_update(uint32_t,pixaura_v2_handle,const pixaura_policy_observation*,pixaura_execution_strategy*);
PIXAURA_API int32_t pixaura_v2_runtime_close(uint32_t,pixaura_v2_handle);
PIXAURA_API int32_t pixaura_v2_editor_open(uint32_t,pixaura_v2_handle,const char*,uint64_t,int32_t,uint64_t,uint32_t,pixaura_v2_handle*);
PIXAURA_API int32_t pixaura_v2_editor_stop(uint32_t,pixaura_v2_handle);
PIXAURA_API int32_t pixaura_v2_editor_close(uint32_t,pixaura_v2_handle);
PIXAURA_API int32_t pixaura_v2_editor_state(uint32_t,pixaura_v2_handle,pixaura_v2_state*);
PIXAURA_API int32_t pixaura_v2_editor_edit_identity(uint32_t,pixaura_v2_handle,char*,uint64_t);
/* Read-only ordinary approved history. No candidate or proposal is a revision. */
PIXAURA_API int32_t pixaura_v2_editor_history_json(uint32_t,pixaura_v2_handle,uint8_t*,uint64_t,uint64_t*);
PIXAURA_API int32_t pixaura_v2_editor_revision_preview(uint32_t,pixaura_v2_handle,const char*,uint64_t,uint32_t,uint32_t,pixaura_v2_preview*);
PIXAURA_API int32_t pixaura_v2_editor_select(uint32_t,pixaura_v2_handle,uint32_t);
PIXAURA_API int32_t pixaura_v2_editor_update(uint32_t,pixaura_v2_handle,const int32_t*,uint32_t);
PIXAURA_API int32_t pixaura_v2_editor_apply(uint32_t,pixaura_v2_handle,uint32_t*);
PIXAURA_API int32_t pixaura_v2_editor_cancel(uint32_t,pixaura_v2_handle);
PIXAURA_API int32_t pixaura_v2_editor_history(uint32_t,pixaura_v2_handle,uint32_t);
PIXAURA_API int32_t pixaura_v2_editor_preview(uint32_t,pixaura_v2_handle,uint32_t,uint32_t,uint32_t,pixaura_v2_preview*);
/* Copy on a worker lane: waits for metadata readers, then validates immutable
 * frame source/epoch/generation. frame_current remains nonblocking for UI use. */
PIXAURA_API int32_t pixaura_v2_frame_copy(uint32_t,pixaura_v2_handle,pixaura_v2_handle,uint8_t*,uint64_t);
PIXAURA_API int32_t pixaura_v2_frame_current(uint32_t,pixaura_v2_handle,pixaura_v2_handle);
/* Additive read-only probe: worker_wait 0 is nonblocking, 1 is worker-only.
 * On success, output is 0 CURRENT, 1 BUSY or
 * 2 STALE. Errors leave output untouched. Only BUSY is retryable: resource
 * denial remains an API error, never a transient-lock indication. No allocation,
 * revision or approval authority. Legacy frame_current behavior is unchanged. */
PIXAURA_API int32_t pixaura_v2_frame_probe(uint32_t,pixaura_v2_handle,pixaura_v2_handle,uint32_t,uint32_t*);
PIXAURA_API int32_t pixaura_v2_frame_release(uint32_t,pixaura_v2_handle);
/* Saved-revision export tickets are read-only and quota-independent. Begin on
 * the command lane before dispatch; later edits do not retarget the snapshot.
 * Cancellation affects only this export. Release every ticket after quiescence. */
PIXAURA_API int32_t pixaura_v2_export_begin(uint32_t,pixaura_v2_handle,uint32_t,int32_t,uint64_t*);
PIXAURA_API int32_t pixaura_v2_export_run(uint32_t,pixaura_v2_handle,uint64_t,int32_t,pixaura_v2_export_result*);
PIXAURA_API int32_t pixaura_v2_export_cancel(uint32_t,pixaura_v2_handle,uint64_t);
PIXAURA_API int32_t pixaura_v2_export_release(uint32_t,pixaura_v2_handle,uint64_t);
PIXAURA_API int32_t pixaura_v2_editor_export_observed(uint32_t,pixaura_v2_handle,uint32_t,int32_t,int32_t,pixaura_v2_export_result*);
PIXAURA_API int32_t pixaura_v2_editor_export(uint32_t,pixaura_v2_handle,uint32_t,int32_t,int32_t,uint64_t*);
PIXAURA_API int32_t pixaura_v2_advisory_cancel(uint32_t,pixaura_v2_handle,const char*,uint64_t);
/* Begin before dispatching a worker. A cancelled ticket cannot consume quota.
 * Run is single-use, source/generation-bound; no token grants Human Apply. */
PIXAURA_API int32_t pixaura_v2_advisory_begin(uint32_t,pixaura_v2_handle,uint32_t,uint64_t*);
PIXAURA_API int32_t pixaura_v2_advisory_run(uint32_t,pixaura_v2_handle,uint64_t,pixaura_v2_advisory*);
PIXAURA_API int32_t pixaura_v2_advisory_request_cancel(uint32_t,pixaura_v2_handle,uint64_t);
PIXAURA_API int32_t pixaura_v2_advisory_request(uint32_t,pixaura_v2_handle,uint32_t,pixaura_v2_advisory*);
/* Bounded JSON token list for the actual current native project/revision.
 * Tokens identify untrusted advisory data; they carry no Apply authority. */
PIXAURA_API int32_t pixaura_v2_advisory_pending(uint32_t,pixaura_v2_handle,uint8_t*,uint64_t,uint64_t*);
PIXAURA_API int32_t pixaura_v2_advisory_restore(uint32_t,pixaura_v2_handle,const char*,uint64_t,pixaura_v2_advisory*);
/* Content kind 0 = provenance Plan; 1 = editable operations. Bounded 8192 bytes.
 * Size query returns BUFFER_TOO_SMALL (11); required is written on that result.
 */
/* Untrusted Plan transport only. Modes: 1 live binding (empty input), 2 Plan
 * validation, 3 deterministic structured-intent planning. No Apply authority. */
PIXAURA_API int32_t pixaura_v2_editor_agent_bytes(uint32_t,pixaura_v2_handle,uint32_t,const uint8_t*,uint64_t,uint8_t*,uint64_t,uint64_t*);
PIXAURA_API int32_t pixaura_v2_advisory_plan_begin(uint32_t,pixaura_v2_handle,const uint8_t*,uint64_t,uint64_t*);
PIXAURA_API int32_t pixaura_v2_advisory_content(uint32_t,pixaura_v2_handle,const char*,uint64_t,uint32_t,uint8_t*,uint64_t,uint64_t*);
PIXAURA_API int32_t pixaura_v2_advisory_inspect(uint32_t,pixaura_v2_handle,const char*,uint64_t,pixaura_v2_review_inspection*);
/* command 1 raw value, 2 remove, 3 move, 4 restore. Source-bound native review
 * identity fixes the action/parameter ordering. Failed edits retire eligibility. */
PIXAURA_API int32_t pixaura_v2_advisory_edit_command(uint32_t,pixaura_v2_handle,const char*,uint64_t,uint32_t,uint32_t,uint32_t,int32_t,uint32_t,pixaura_v2_advisory*);
PIXAURA_API int32_t pixaura_v2_advisory_edit(uint32_t,pixaura_v2_handle,const char*,uint64_t,const uint8_t*,uint64_t,pixaura_v2_advisory*);
PIXAURA_API int32_t pixaura_v2_advisory_reset(uint32_t,pixaura_v2_handle,const char*,uint64_t,pixaura_v2_advisory*);
PIXAURA_API int32_t pixaura_v2_advisory_approve(uint32_t,pixaura_v2_handle,const char*,uint64_t,char*,uint64_t);
PIXAURA_API int32_t pixaura_v2_advisory_apply(uint32_t,pixaura_v2_handle,const char*,uint64_t,const char*,uint64_t,uint32_t*);
PIXAURA_API int32_t pixaura_v2_advisory_apply_receipt(uint32_t,pixaura_v2_handle,const char*,uint64_t,const char*,uint64_t,pixaura_v2_commit_result*);
/* Scoped comparison work: cancelling it preserves the usable review and quotas.
 * Retain ticket while displaying its frames; release retires both frame currents.
 * Render outputs must be separate non-overlapping records; release frames too. */
PIXAURA_API int32_t pixaura_v2_review_preview_begin(uint32_t,pixaura_v2_handle,const char*,uint64_t,uint32_t,uint32_t,uint64_t*);
PIXAURA_API int32_t pixaura_v2_review_preview_render(uint32_t,pixaura_v2_handle,uint64_t,pixaura_v2_preview*,pixaura_v2_preview*);
PIXAURA_API int32_t pixaura_v2_review_preview_cancel(uint32_t,pixaura_v2_handle,uint64_t);
PIXAURA_API int32_t pixaura_v2_review_preview_release(uint32_t,pixaura_v2_handle,uint64_t);
PIXAURA_API int32_t pixaura_v2_advisory_preview(uint32_t,pixaura_v2_handle,const char*,uint64_t,uint32_t,uint32_t,pixaura_v2_preview*);
#ifdef __cplusplus
}
#endif
#endif
