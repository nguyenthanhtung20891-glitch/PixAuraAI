#ifndef PIXAURA_MANUAL_H
#define PIXAURA_MANUAL_H
#include "document.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PIXAURA_MANUAL_API_VERSION 1u
#define PIXAURA_MANUAL_MAX_DESCRIPTORS 16u
#define PIXAURA_MANUAL_MAX_PARAMETERS 8u
#define PIXAURA_MANUAL_MAX_REGISTRY_BYTES 32768u
/* Read-only compiled registry. No registration/discovery/network or handles.
 * Two-call UTF-8 JSON: required includes LF, excludes NUL. A short buffer writes
 * only required; all other failures preserve outputs. Caller owns truthful,
 * nonoverlapping buffers; no exceptions or retained input pointers cross C. */
PIXAURA_API int32_t pixaura_manual_registry_json(uint32_t version,
    uint8_t* output, uint64_t capacity, uint64_t* required);
/* Exactly one existing schema-1 operation in {"operations":[OPERATION]}.
 * The shared document parser rejects unknown tuples/types/keys and noninteger
 * notation. Success returns the canonical evaluation envelope, not approval. */
PIXAURA_API int32_t pixaura_manual_canonical_operation(uint32_t version,
    const uint8_t* request, uint64_t bytes, uint8_t* output,
    uint64_t capacity, uint64_t* required);
#ifdef __cplusplus
}
#endif
#endif
