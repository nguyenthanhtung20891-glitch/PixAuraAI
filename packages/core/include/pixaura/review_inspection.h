#ifndef PIXAURA_REVIEW_INSPECTION_H
#define PIXAURA_REVIEW_INSPECTION_H
#include "review_preview.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Caller-owned, read-only presentation, never a Plan or capability. No release
 * required; no retained native state. Strings are bounded ASCII, zero terminated
 * except the exact 64-byte SHA-256 hex and 32-byte revision identity. */
#define PIXAURA_REVIEW_INSPECTION_KIND 0x52494e31u
#define PIXAURA_REVIEW_WARNING_MULTIPLE 1u
#define PIXAURA_REVIEW_WARNING_GEOMETRY 2u
typedef struct pixaura_review_parameter {
    char name[32],unit[64];
    int32_t raw_value,minimum,maximum,neutral;
} pixaura_review_parameter;
typedef struct pixaura_review_action {
    uint32_t index,tool_index,operation_version,parameter_version;
    uint32_t parameter_count,geometry_changed,warnings,reserved;
    char tool_id[32],label[32];
    pixaura_review_parameter parameters[4];
} pixaura_review_action;
typedef struct pixaura_review_inspection {
    uint32_t api_version,struct_size,kind,action_count,warnings,registry_version;
    uint64_t repository_epoch;
    pixaura_auto_proposal review;
    char plan_sha256[64],source_revision[32];
    pixaura_review_action actions[8];
} pixaura_review_inspection;
/* Exact cached canonical Plan only, 1..8192 bytes, 1..8 actions. Atomic output:
 * failure leaves caller output unchanged. Existing owner stop/join/destroy rules.
 * Metadata is from the trusted native Registry; no localized text is parsed. */
PIXAURA_API int32_t pixaura_review_inspect(uint32_t,pixaura_editor_context*,
    const pixaura_auto_proposal*,const uint8_t*,uint64_t,pixaura_review_inspection*);
/* Recomputes exact projection and checks freshness, not approval eligibility. */
PIXAURA_API int32_t pixaura_review_inspection_current(uint32_t,pixaura_editor_context*,const pixaura_review_inspection*);
/* Explicit association check: same current source/Plan/review as preview pair. */
PIXAURA_API int32_t pixaura_review_inspection_preview(uint32_t,pixaura_editor_context*,
    const pixaura_review_inspection*,const pixaura_review_preview_pair*);
#ifdef __cplusplus
}
#endif
#endif
