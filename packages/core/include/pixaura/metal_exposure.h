#ifndef PIXAURA_METAL_EXPOSURE_H
#define PIXAURA_METAL_EXPOSURE_H
#include "document.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Hardware-only owner for the retained exposure diagnostic. Not an inference
 * execution API. Exclusive init/dispatch/destroy; stable zeroed caller storage.
 * Double destroy rejects. No kernel/source choice or model input accepted.
 * Returns retained GPU diagnostic statuses 1/5/8/14. */
typedef pixaura_document_context pixaura_metal_exposure_context;
PIXAURA_API int32_t pixaura_metal_exposure_init(uint32_t,pixaura_metal_exposure_context*,uint32_t);
PIXAURA_API int32_t pixaura_metal_exposure_dispatch(pixaura_metal_exposure_context*,const float*,float*,uint32_t,float,uint32_t);
PIXAURA_API int32_t pixaura_metal_exposure_destroy(pixaura_metal_exposure_context*);
#ifdef __cplusplus
}
#endif
#endif
