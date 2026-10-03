#ifndef PIXAURA_STORAGE_H
#define PIXAURA_STORAGE_H
#include "core.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PIXAURA_STORAGE_API_VERSION 1u
typedef struct pixaura_storage_info {
    uint32_t api_version;
    uint32_t struct_size;
    uint32_t storage_version;
    uint32_t sqlite_version;
} pixaura_storage_info;
/* Synchronous background-only capability check. Root is a caller-owned,
 * existing absolute app-private directory (UTF-8, <=1024 bytes, no NUL).
 * Creates a fresh schema if absent; never migrates an existing database.
 * No handles escape. Serialize destruction/lifetime of caller memory with calls.
 * ABI probe 1 and its zero feature flags are unchanged. Failure preserves output.
 * Status codes follow document.h; 15 additionally means a bounded busy timeout.
 */
PIXAURA_API int32_t pixaura_storage_check(uint32_t api_version,
    const uint8_t* root, uint64_t root_bytes, pixaura_storage_info* output);
#ifdef __cplusplus
}
#endif
#endif
