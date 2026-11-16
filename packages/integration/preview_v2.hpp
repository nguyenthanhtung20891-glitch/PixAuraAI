#ifndef PIXAURA_PREVIEW_V2_HPP
#define PIXAURA_PREVIEW_V2_HPP
#include "stage_plan_v2.hpp"
namespace pixaura::integration {
struct PreviewV2 {
    std::unique_ptr<tiled::Budget::Lease> lease;
    geometry::Extent source;
    uint32_t width=0,height=0;
    std::vector<uint8_t> pixels;
};
PreviewV2 fit_preview_v2(const StagePlanV2&,uint32_t,uint32_t,tiled::Budget&,
    const evaluation::Cancellation* =nullptr);
}
#endif
