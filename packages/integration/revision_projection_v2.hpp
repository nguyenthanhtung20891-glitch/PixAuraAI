#ifndef PIXAURA_REVISION_PROJECTION_V2_HPP
#define PIXAURA_REVISION_PROJECTION_V2_HPP
#include "stage_plan_v2.hpp"
namespace pixaura::integration {
// Read-only native revision projection. No Apply, approval, quota or history authority.
class RevisionProjectionV2 final {
    std::unique_ptr<tiled::Budget::Lease> snapshot_lease_;
    document::Snapshot snapshot_;
    StagePlanV2 plan_;
    static evaluation::Stack bound(const document::Snapshot&,const OriginalReaderV2&);
public:
    RevisionProjectionV2(document::Snapshot,const OriginalReaderV2&,tiled::Budget&);
    const StagePlanV2& plan()const{return plan_;}
    const document::ImageDocument& document()const{return *snapshot_;}
};
}
#endif
