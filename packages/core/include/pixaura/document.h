#ifndef PIXAURA_DOCUMENT_H
#define PIXAURA_DOCUMENT_H

#include "core.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PIXAURA_DOCUMENT_API_VERSION 1u
#define PIXAURA_DOCUMENT_INVALID_HANDLE 3
#define PIXAURA_DOCUMENT_UNSUPPORTED_SCHEMA 4
#define PIXAURA_DOCUMENT_UNSUPPORTED_OPERATION 5
#define PIXAURA_DOCUMENT_INVALID_PROJECT 6
#define PIXAURA_DOCUMENT_INVALID_PARAMETERS 7
#define PIXAURA_DOCUMENT_RESOURCE_LIMIT 8
#define PIXAURA_DOCUMENT_STALE_BASE 9
#define PIXAURA_DOCUMENT_NO_HISTORY 10
#define PIXAURA_DOCUMENT_BUFFER_TOO_SMALL 11
#define PIXAURA_DOCUMENT_IO_ERROR 12
#define PIXAURA_DOCUMENT_CANCELLED 13
#define PIXAURA_DOCUMENT_INTERNAL_ERROR 14

/* Initialize to zero. Opaque ownership storage; never copy or modify it.
 * Keep storage accessible until all calls and destroy checks have finished.
 * Init/destroy require exclusive access; calls on a live context are synchronized.
 * Caller supplies a fresh 32-byte lowercase hex identity, unique across lifetimes.
 * The library owns registry allocations; destroy releases all document handles.
 * All pointers must have truthful accessible lengths and be nonoverlapping.
 */
typedef struct pixaura_document_context {
    uint32_t api_version;
    uint32_t struct_size;
    uint64_t reserved;
    uint64_t opaque[6];
} pixaura_document_context;

/* Token values are opaque, context-scoped, non-pointer identities. */
typedef struct pixaura_document_handle {
    uint32_t api_version;
    uint32_t struct_size;
    uint64_t reserved;
    uint8_t context_id[32];
    uint64_t serial;
} pixaura_document_handle;

typedef struct pixaura_document_error {
    uint32_t api_version;
    uint32_t struct_size;
    int32_t code;
    uint32_t field_index;
    uint32_t message_bytes;
    uint32_t reserved;
    uint8_t message[160];
} pixaura_document_error;

/* Optional error: initialize version=1, size=184, reserved=0.
 * Invalid layouts leave every output untouched. Other failures preserve outputs
 * except BUFFER_TOO_SMALL writes required_bytes (includes LF, excludes NUL).
 * Session ID is exactly 32 lowercase hex bytes; its generation starts at zero.
 * Open/create do not verify source bytes. Apply returns a detached proposal;
 * the application service alone approves, persists and publishes it.
 */
PIXAURA_API int32_t pixaura_document_context_init(uint32_t version,
    pixaura_document_context* context, uint32_t context_bytes,
    const uint8_t* context_id, uint64_t identity_bytes, pixaura_document_error* error);
PIXAURA_API int32_t pixaura_document_context_destroy(pixaura_document_context* context);
PIXAURA_API int32_t pixaura_document_open(uint32_t version, pixaura_document_context* context,
    const uint8_t* manifest, uint64_t manifest_bytes, const uint8_t* session_id,
    uint64_t session_bytes, pixaura_document_handle* output, pixaura_document_error* error);
/* Create requires a root-only manifest with no operations or redo path. */
PIXAURA_API int32_t pixaura_document_create(uint32_t version, pixaura_document_context* context,
    const uint8_t* manifest, uint64_t manifest_bytes, const uint8_t* session_id,
    uint64_t session_bytes, pixaura_document_handle* output, pixaura_document_error* error);
PIXAURA_API int32_t pixaura_document_apply(uint32_t version, pixaura_document_context* context,
    const pixaura_document_handle* snapshot, const uint8_t* command, uint64_t command_bytes,
    pixaura_document_handle* output, pixaura_document_error* error);
PIXAURA_API int32_t pixaura_document_serialize(pixaura_document_context* context,
    const pixaura_document_handle* snapshot, uint8_t* output, uint64_t capacity,
    uint64_t* required_bytes, pixaura_document_error* error);
PIXAURA_API int32_t pixaura_document_release(pixaura_document_context* context,
    const pixaura_document_handle* snapshot);

#ifdef __cplusplus
}
#endif
#endif
