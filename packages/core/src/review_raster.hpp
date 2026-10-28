#ifndef PIXAURA_REVIEW_RASTER_HPP
#define PIXAURA_REVIEW_RASTER_HPP
#include "evaluation.hpp"
#include "pixaura/review_preview.h"
namespace pixaura {
// Internal composition under the existing raster owner lock. Data/image access
// only: no document, approval, candidate or repository capability.
int32_t review_raster_pair(pixaura_decode_context*,const pixaura_decode_handle*,
    const pixaura_decode_handle*,const evaluation::Stack&,const evaluation::Stack&,
    const pixaura_working_limits&,uint32_t,uint64_t,
    pixaura_decode_handle*,pixaura_decode_handle*,pixaura_review_preview_pair*);
}
#endif
