#include "stage_plan_v2.hpp"
#include "../../detail.hpp"
#include <algorithm>
#include <cstring>
namespace pixaura::integration {
namespace {
void need(bool ok,int code=7){if(!ok){throw document::Failure{code};}}
working::Image image(uint32_t w,uint32_t h){
    working::Image out;out.metadata={1,sizeof(pixaura_working_metadata),w,h,1,1,1,0,uint64_t(w)*16,uint64_t(w)*h*16};
    out.pixels=std::make_unique<decode::Vector<float>>(std::size_t(w)*h*4);
    need(out.pixels->capacity()==std::size_t(w)*h*4,8);return out;
}
}
StagePlanV2::StagePlanV2(const OriginalReaderV2& source,const evaluation::Stack& ops,tiled::Budget& budget):source_(source){
    plan_lease_.reset(new tiled::Budget::Lease(budget.reserve(1048576)));
    evaluation::validate(ops);auto e=geometry::Extent{source.width(),source.height()};uint64_t visits=0;
    need(e.width<=16384&&e.height<=16384,8);operations_=ops;stages_.reserve(ops.size());
    for(const auto& op:ops){
        geometry::Stage stage{e,e,{0,0,e.width,e.height},0};
        if(op.type=="pixaura.crop"){
            stage.region=geometry::crop(e,std::get<document::Crop>(op.parameters));
            stage.output={stage.region.x1-stage.region.x0,stage.region.y1-stage.region.y0};
        }else if(op.type=="pixaura.rotate"){
            stage.turns=std::get<document::Rotate>(op.parameters).quarter_turns;stage.output=geometry::rotated(e,stage.turns);
        }
        visits+=uint64_t(e.width)*e.height;need(visits<=2147483648ULL,8);
        stages_.push_back(stage);e=stage.output;
    }
}
geometry::Extent StagePlanV2::extent()const{return stages_.empty()?geometry::Extent{source_.width(),source_.height()}:stages_.back().output;}
working::Image StagePlanV2::block(std::size_t count,geometry::Rect r,const evaluation::Cancellation* cancel)const{
    evaluation::checkpoint(cancel,evaluation::Checkpoint::tile);
    if(!count){
        auto out=image(r.x1-r.x0,r.y1-r.y0);
        for(uint32_t y=r.y0;y<r.y1;++y){
            evaluation::checkpoint(cancel,evaluation::Checkpoint::tile);
            for(uint32_t x=r.x0;x<r.x1;++x)source_.read(x,y,out.pixels->data()+(std::size_t(y-r.y0)*out.metadata.width+x-r.x0)*4);
        }
        return out;
    }
    const auto& s=stages_[count-1];const auto& op=operations_[count-1];auto input=r;
    if(op.type=="pixaura.crop"){
        input={r.x0+s.region.x0,r.y0+s.region.y0,r.x1+s.region.x0,r.y1+s.region.y0};
    }else if(op.type=="pixaura.rotate"){
        input={s.input.width,s.input.height,0,0};
        for(auto x:{r.x0,r.x1-1})for(auto y:{r.y0,r.y1-1}){
            const auto p=geometry::inverse(s.input,s.turns,{x,y});
            input.x0=std::min(input.x0,p.x);input.y0=std::min(input.y0,p.y);
            input.x1=std::max(input.x1,p.x+1);input.y1=std::max(input.y1,p.y+1);
        }
    }else if(detail::find(op.type)){
        input={r.x0?r.x0-1:0,r.y0?r.y0-1:0,std::min(r.x1+1,s.input.width),std::min(r.y1+1,s.input.height)};
    }
    auto previous=block(count-1,input,cancel);
    if(op.type!="pixaura.crop"&&op.type!="pixaura.rotate"){
        evaluation::Stack one;one.push_back(op);
        auto evaluated=evaluation::evaluate(previous,one,working::defaults(),cancel);
        previous.pixels.reset();previous=std::move(evaluated);
    }
    auto out=image(r.x1-r.x0,r.y1-r.y0);
    for(uint32_t y=r.y0;y<r.y1;++y){
        evaluation::checkpoint(cancel,evaluation::Checkpoint::tile);
        for(uint32_t x=r.x0;x<r.x1;++x){
            auto p=geometry::Point{x,y};
            if(op.type=="pixaura.crop"){p=geometry::crop_source(s,p);}
            else if(op.type=="pixaura.rotate"){p=geometry::inverse(s.input,s.turns,p);}
            need(p.x>=input.x0&&p.x<input.x1&&p.y>=input.y0&&p.y<input.y1);
            const auto at=(std::size_t(p.y-input.y0)*previous.metadata.width+p.x-input.x0)*4;
            std::memcpy(out.pixels->data()+(std::size_t(y-r.y0)*out.metadata.width+x-r.x0)*4,previous.pixels->data()+at,16);
        }
    }
    return out;
}
BlockV2 StagePlanV2::render(geometry::Rect r,tiled::Budget& budget,const evaluation::Cancellation* cancel)const{
    const auto e=extent();need(r.x0<r.x1&&r.y0<r.y1&&r.x1<=e.width&&r.y1<=e.height);
    need(r.x1-r.x0<=128&&r.y1-r.y0<=128,8);
    need(!cancel||((cancel->generation==nullptr)==(cancel->revoked==nullptr)),1);
    const uint64_t side=128+2*operations_.size();
    // Two region rasters, detail rows, bounded plan/frame bookkeeping. No full float image.
    auto lease=std::unique_ptr<tiled::Budget::Lease>(new tiled::Budget::Lease(budget.reserve(side*side*32+side*48+1048576)));
    auto result=block(operations_.size(),r,cancel);
    evaluation::checkpoint(cancel,evaluation::Checkpoint::publication);
    return {std::move(lease),std::move(result)};
}
}
