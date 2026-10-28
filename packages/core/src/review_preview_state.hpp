#ifndef PIXAURA_REVIEW_PREVIEW_STATE_HPP
#define PIXAURA_REVIEW_PREVIEW_STATE_HPP
#include "pixaura/review_preview.h"
namespace pixaura {
// Gate-protected preview resources only. No document/repository/approval owner.
struct ReviewPreviewState {
    pixaura_review_preview_ticket ticket{};
    pixaura_review_preview_pair pair{};
    pixaura_auto_proposal proposal{};
    document::String plan;
    uint64_t sequence=0,repository_epoch=0,policy_epoch=0;
    uint32_t width=0,height=0;
    bool active=false,pending=false,ready=false,cancelled=true;
    pixaura_decode_handle before{},after{},cancellation{};
#ifdef PIXAURA_CERTIFICATION_TESTING
    std::atomic<uint32_t> point{UINT32_MAX},reached{0},resume{0};
    uint32_t fault=0;
#endif
    void release(pixaura_decode_context& raster) noexcept {
        if(before.serial)(void)pixaura_preview_release(&raster,&before);
        if(after.serial)(void)pixaura_preview_release(&raster,&after);
        before={};after={};
    }
    void invalidate(pixaura_decode_context& raster) noexcept {
        cancelled=true;pending=false;ready=false;
        if(cancellation.serial)(void)pixaura_cancel_signal(&raster,&cancellation);
        release(raster);plan.clear();
    }
};
}
#endif
