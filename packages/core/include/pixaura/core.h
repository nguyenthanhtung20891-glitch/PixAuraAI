#ifndef PIXAURA_CORE_H
#define PIXAURA_CORE_H

#include <stdint.h>

#if defined(_WIN32) && defined(PIXAURA_SHARED)
#if defined(PIXAURA_BUILDING)
#define PIXAURA_API __declspec(dllexport)
#else
#define PIXAURA_API __declspec(dllimport)
#endif
#else
#define PIXAURA_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define PIXAURA_ABI_VERSION 1u
#define PIXAURA_OK 0
#define PIXAURA_INVALID_ARGUMENT 1
#define PIXAURA_UNSUPPORTED_ABI 2

typedef struct pixaura_core_info {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t implemented_features;
    uint32_t reserved;
} pixaura_core_info;

/* Caller owns output. Invalid calls leave it untouched. No pixel tools yet. */
PIXAURA_API int32_t pixaura_get_core_info(uint32_t requested_abi,
                                         pixaura_core_info* output,
                                         uint32_t output_bytes);

#ifdef __cplusplus
}
#endif
#endif
