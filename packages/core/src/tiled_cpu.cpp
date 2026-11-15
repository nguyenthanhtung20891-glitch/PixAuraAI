#include "tiled_cpu.hpp"
#include "detail.hpp"
#include <algorithm>
#include <cstring>
namespace pixaura::tiled {
Budget::Budget(uint64_t cap,uint64_t retained):cap_(cap),retained_(retained){
    if(!cap||cap>134217728||retained>cap){throw document::Failure{8};}
    peak_=retained;
}
Budget::Lease Budget::reserve(uint64_t bytes){
    std::lock_guard<std::mutex> lock(mutex_);
    if(bytes>cap_-retained_-inflight_)throw document::Failure{8};
    inflight_+=bytes;peak_=std::max(peak_,retained_+inflight_);return Lease(this,bytes);
}
void Budget::release(uint64_t bytes) noexcept{std::lock_guard<std::mutex> lock(mutex_);inflight_-=bytes;}
uint64_t Budget::peak(){std::lock_guard<std::mutex> lock(mutex_);return peak_;}
uint64_t Budget::inflight(){std::lock_guard<std::mutex> lock(mutex_);return inflight_;}

Metrics evaluate(uint32_t width,uint32_t height,const evaluation::Stack& stack,
    Budget& budget,const Reader& read,Sink& sink,
    const evaluation::Cancellation* cancel) {
    const auto need=[](bool ok,int code){if(!ok)throw document::Failure{code};};
    Metrics metrics;
    try {
        need(width&&height&&width<=16384&&height<=16384&&read,7);
        need(!cancel||((cancel->generation==nullptr)==(cancel->revoked==nullptr)),1);
        evaluation::validate(stack);
        uint32_t halo=0;
        for(const auto& op:stack){
            need(op.type!="pixaura.crop"&&op.type!="pixaura.rotate",5);
            if(detail::find(op.type))++halo;
        }
        constexpr uint32_t edge=128;
        const uint64_t side=edge+uint64_t(halo)*2;
        // Two expanded rasters plus accepted three-row detail scratch. Actual
        // vector capacity is checked before evaluation; metadata is stack-owned.
        const uint64_t reserved=side*side*32+side*48+65536;
        auto lease=budget.reserve(reserved);
        metrics.peak_reserved=budget.peak();
        for(uint32_t y=0;y<height;y+=edge)for(uint32_t x=0;x<width;x+=edge){
            evaluation::checkpoint(cancel,evaluation::Checkpoint::tile);
            const auto w=std::min(edge,width-x),h=std::min(edge,height-y);
            const uint32_t ew=w+halo*2,eh=h+halo*2;
            working::Image input;
            input.metadata={1,sizeof(pixaura_working_metadata),ew,eh,1,1,1,0,uint64_t(ew)*16,uint64_t(ew)*eh*16};
            input.pixels=std::make_unique<decode::Vector<float>>(std::size_t(ew)*eh*4);
            need(input.pixels->capacity()*sizeof(float)<=side*side*16,8);
            for(uint32_t row=0;row<eh;++row){
                evaluation::checkpoint(cancel,evaluation::Checkpoint::tile);
                const auto sy=static_cast<uint32_t>(std::clamp<int64_t>(int64_t(y)+row-halo,0,height-1));
                for(uint32_t col=0;col<ew;++col){
                    const auto sx=static_cast<uint32_t>(std::clamp<int64_t>(int64_t(x)+col-halo,0,width-1));
                    read(sx,sy,input.pixels->data()+(std::size_t(row)*ew+col)*4);
                }
            }
            // True image edges must be clamped again after EVERY local pass.
            // Merely replicating the original halo is not equivalent for multi-pass.
            const auto clamp_edges=[&](working::Image& image){
                for(uint32_t row=0;row<eh;++row)for(uint32_t col=0;col<ew;++col){
                    const int64_t gx=int64_t(x)+col-halo,gy=int64_t(y)+row-halo;
                    const auto cx=std::clamp<int64_t>(gx,0,width-1),cy=std::clamp<int64_t>(gy,0,height-1);
                    if(cx!=gx||cy!=gy){
                        const auto sx=static_cast<uint32_t>(cx-int64_t(x)+halo),sy=static_cast<uint32_t>(cy-int64_t(y)+halo);
                        std::memcpy(image.pixels->data()+(std::size_t(row)*ew+col)*4,image.pixels->data()+(std::size_t(sy)*ew+sx)*4,16);
                    }
                }
            };
            if(stack.empty())input=evaluation::evaluate(input,{},working::defaults(),cancel);
            for(const auto& op:stack){
                evaluation::Stack stage;stage.push_back(op);
                auto next=evaluation::evaluate(input,stage,working::defaults(),cancel);
                clamp_edges(next);input=std::move(next);
            }
            auto output=std::move(input);
            evaluation::checkpoint(cancel,evaluation::Checkpoint::publication);
            sink.write(x,y,w,h,output.pixels->data()+(std::size_t(halo)*ew+halo)*4,ew);
            ++metrics.tiles;
        }
        evaluation::checkpoint(cancel,evaluation::Checkpoint::publication);
        sink.finish();return metrics;
    } catch(...) {sink.abort();throw;}
}
}
