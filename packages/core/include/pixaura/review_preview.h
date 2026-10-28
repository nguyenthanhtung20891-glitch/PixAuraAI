#ifndef PIXAURA_REVIEW_PREVIEW_H
#define PIXAURA_REVIEW_PREVIEW_H
#include "auto_proposal.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Preview-only family 1. Values are non-owning correlation/eligibility records,
 * never document, approval or execution-candidate handles. All output buffers
 * are caller-owned. stop -> join ALL calls -> release -> destroy editor. */
#define PIXAURA_REVIEW_PREVIEW_KIND 0x52565031u
typedef struct pixaura_review_preview_ticket {
    uint32_t api_version,struct_size,kind,reserved;
    uint8_t context_id[32];
    uint64_t request_serial;
} pixaura_review_preview_ticket;
typedef struct pixaura_review_preview_pair {
    pixaura_review_preview_ticket ticket;
    pixaura_preview_metadata before,after;
    uint32_t before_width,before_height,after_width,after_height;
    uint32_t geometry_changed,reserved;
    uint64_t owned_bytes,admission_peak_bytes;
} pixaura_review_preview_pair;
/* Exact current Auto proposal AND exact immutable reviewed canonical Plan bytes.
 * Positive fit bounds <=1024, further restricted by existing native policy.
 * One current request/pair; active/draining work returns BUSY(8), no queue. */
PIXAURA_API int32_t pixaura_review_preview_begin(uint32_t,pixaura_editor_context*,
    const pixaura_auto_proposal*,const uint8_t* plan,uint64_t bytes,
    uint32_t max_width,uint32_t max_height,pixaura_review_preview_ticket*);
PIXAURA_API int32_t pixaura_review_preview_render(uint32_t,pixaura_editor_context*,
    const pixaura_review_preview_ticket*,pixaura_review_preview_pair*);
/* MUST precede actual display install under the platform lifecycle owner gate.
 * Revalidates exact native record, source/Plan, repository and policy epochs. */
PIXAURA_API int32_t pixaura_review_preview_current(uint32_t,pixaura_editor_context*,const pixaura_review_preview_pair*);
/* side 0 BEFORE / 1 AFTER PREVIEW; checked offset/count <= actual image bytes.
 * Copy conveys no install authority; current must be checked at installation. */
PIXAURA_API int32_t pixaura_review_preview_copy(uint32_t,pixaura_editor_context*,
    const pixaura_review_preview_pair*,uint32_t side,uint64_t offset,uint8_t*,uint64_t bytes);
/* Drops native pixels while preserving the bound display-record eligibility.
 * Platform copies remain platform-owned; subsequent copy rejects. */
PIXAURA_API int32_t pixaura_review_preview_release(uint32_t,pixaura_editor_context*,const pixaura_review_preview_pair*);
/* Preview scope ONLY; old ticket cannot cancel newer work or approved execution. */
PIXAURA_API int32_t pixaura_review_preview_cancel(uint32_t,pixaura_editor_context*,const pixaura_review_preview_ticket*);
#ifdef __cplusplus
}
#endif
#endif
