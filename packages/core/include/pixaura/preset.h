#ifndef PIXAURA_PRESET_H
#define PIXAURA_PRESET_H
#include "document.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PIXAURA_PRESET_API_VERSION 1u
#define PIXAURA_PRESET_MAX_BYTES 16384u
#define PIXAURA_PRESET_MAX_OPERATIONS 16u
#define PIXAURA_PRESET_MAX_BINDINGS_BYTES 1024u
/* Shared canonical recipe / empty shipping catalog. Two-call output includes LF,
 * excludes NUL. BUFFER_TOO_SMALL writes only required; other failures preserve it.
 * No exceptions, input retention, executable content or platform semantics. */
PIXAURA_API int32_t pixaura_preset_canonical(uint32_t version,const uint8_t* recipe,uint64_t bytes,uint8_t* output,uint64_t capacity,uint64_t* required);
PIXAURA_API int32_t pixaura_preset_catalog(uint32_t version,uint8_t* output,uint64_t capacity,uint64_t* required);
/* Owner supplies base/live, fresh operation_ids in recipe order and revision ID.
 * Returns detached proposal only; no persistence/publication/approval. All-neutral
 * borrows live (changed=0); changed proposal is an owned document handle (changed=1).
 * Existing 64-handle ceiling applies. Failure preserves all outputs/state.
 * Stale base/session/generation/source rejects. Owner cancellation discards proposal;
 * existing PRV1 handles pixel cancellation/stale publication. */
PIXAURA_API int32_t pixaura_preset_propose(uint32_t version,pixaura_document_context* context,
 const pixaura_document_handle* base,const pixaura_document_handle* live,
 const uint8_t* recipe,uint64_t recipe_bytes,const uint8_t* bindings,uint64_t bindings_bytes,
 const uint8_t* revision_id,uint64_t revision_bytes,pixaura_document_handle* output,uint32_t* changed);
#ifdef __cplusplus
}
#endif
#endif
