#ifndef PIXAURA_PREVIEW_BOUNDARY_TEST_H
#define PIXAURA_PREVIEW_BOUNDARY_TEST_H
#include "pixaura/preview.h"
#include <string.h>
/* Small C/JNI consumer scenario; no platform color formulas. */
static int preview_boundary_check(pixaura_decode_context* c,const pixaura_decode_handle* working,
    const pixaura_working_limits* limits){
    pixaura_decode_handle evaluated,preview,token,unchanged;
    pixaura_preview_metadata metadata;
    uint8_t bytes[24],repeat[24];
    if(pixaura_working_identity(c,working,limits,&evaluated)!=0)return 14;
    if(pixaura_preview_create(c,&evaluated,1,NULL,&preview)!=0)return 14;
    if(pixaura_preview_query(c,&preview,&metadata)!=0||metadata.width!=2||metadata.height!=3||metadata.pixel_format!=1||metadata.row_stride!=8||metadata.image_bytes!=24)return 14;
    if(pixaura_preview_copy(c,&preview,0,bytes,24)!=0)return 14;
    if(pixaura_decode_release(c,&evaluated)!=0)return 14;
    if(pixaura_preview_copy(c,&preview,0,repeat,24)!=0||memcmp(bytes,repeat,24))return 14;
    if(pixaura_preview_release(c,&preview)!=0||pixaura_preview_release(c,&preview)!=3)return 14;
    if(pixaura_cancel_create(c,1,&token)!=0||pixaura_cancel_signal(c,&token)!=0)return 14;
    memset(&preview,0x5a,sizeof(preview));unchanged=preview;
    if(pixaura_preview_create(c,working,1,&token,&preview)!=13||memcmp(&preview,&unchanged,sizeof(preview)))return 14;
    if(pixaura_cancel_release(c,&token)!=0)return 14;
    return 0;
}
#endif
