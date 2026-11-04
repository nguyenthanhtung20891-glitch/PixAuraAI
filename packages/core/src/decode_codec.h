#ifndef PIXAURA_DECODE_CODEC_H
#define PIXAURA_DECODE_CODEC_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
int32_t pixaura_codec_decode(uint32_t format,const uint8_t* input,size_t bytes,
    uint8_t* output,uint32_t width,uint32_t height,size_t stride,
    size_t scratch,int fail_after,int fault);
int32_t pixaura_codec_inflate(const uint8_t*,size_t,uint8_t*,size_t,size_t*,size_t);
int32_t pixaura_codec_encode_jpeg(const uint8_t* rgba,uint32_t width,uint32_t height,
    int quality,uint8_t* output,size_t capacity,size_t* written,size_t scratch);
int32_t pixaura_codec_encode_png(const uint8_t* rgba,uint32_t width,uint32_t height,
    uint8_t* output,size_t capacity,size_t* written,size_t scratch);
#ifdef __cplusplus
}
#endif
#endif
