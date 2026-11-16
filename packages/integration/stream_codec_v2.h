#ifndef PIXAURA_PRIVATE_STREAM_CODEC_V2_H
#define PIXAURA_PRIVATE_STREAM_CODEC_V2_H
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef int32_t (*pixaura_private_row_v2)(void*,uint32_t,uint8_t*,size_t);
typedef int32_t (*pixaura_private_bytes_v2)(void*,const uint8_t*,size_t);
// Private static-link interface, not an installed or exported product C ABI.
int32_t pixaura_private_stream_encode_v2(uint32_t,uint32_t,uint32_t,int,size_t,
    pixaura_private_row_v2,pixaura_private_bytes_v2,void*);
#ifdef __cplusplus
}
#endif
#endif
