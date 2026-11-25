#ifndef PIXAURA_STAGE_PLAN_V2_HPP
#define PIXAURA_STAGE_PLAN_V2_HPP
#include "original_reader_v2.hpp"
#include "../../geometry.hpp"
namespace pixaura::integration {
struct BlockV2 {
    std::unique_ptr<tiled::Budget::Lease> lease;
    working::Image image;
};
class StagePlanV2 final {
    std::unique_ptr<tiled::Budget::Lease> plan_lease_;
    const OriginalReaderV2& source_;
    evaluation::Stack operations_;
    std::vector<geometry::Stage> stages_;
    working::Image block(std::size_t,geometry::Rect,const evaluation::Cancellation*)const;
public:
    StagePlanV2(const OriginalReaderV2&,const evaluation::Stack&,tiled::Budget&);
    geometry::Extent extent()const;
    BlockV2 render(geometry::Rect,tiled::Budget&,const evaluation::Cancellation* =nullptr)const;
};
}
#endif
