#include "stream_writer_v2.hpp"
#include "stream_codec_v2.h"
#include "../../preview.hpp"
#include <algorithm>
#include <cfenv>
#include <cstring>
namespace pixaura::integration {
namespace {
struct Writer {
    const StagePlanV2& plan;tiled::Budget& budget;StagedSinkV2& sink;
    const evaluation::Cancellation* cancel;std::vector<uint8_t> band;
    uint32_t band_y=UINT32_MAX;uint64_t bytes=0;
    static int32_t row(void* pointer,uint32_t y,uint8_t* data,size_t count)noexcept{
        auto& s=*static_cast<Writer*>(pointer);
        try{
            evaluation::checkpoint(s.cancel,evaluation::Checkpoint::tile);
            const auto e=s.plan.extent();if(count!=std::size_t(e.width)*4)throw document::Failure{19};
            const uint32_t start=(y/128)*128;
            if(start!=s.band_y){
                const uint32_t end=std::min(start+128,e.height);
                for(uint32_t x=0;x<e.width;x+=128){
                    auto block=s.plan.render({x,start,std::min(x+128,e.width),end},s.budget,s.cancel);
                    for(uint32_t yy=0;yy<block.image.metadata.height;++yy)for(uint32_t xx=0;xx<block.image.metadata.width;++xx){
                        const auto* p=block.image.pixels->data()+(std::size_t(yy)*block.image.metadata.width+xx)*4;
                        auto* out=s.band.data()+(std::size_t(yy)*e.width+x+xx)*4;
                        const double a=p[3];for(unsigned k=0;k<3;++k)out[k]=a==0?0:preview::encode(double(p[k])/a);
                        out[3]=preview::alpha(a);
                    }
                }
                s.band_y=start;
            }
            std::memcpy(data,s.band.data()+std::size_t(y-start)*e.width*4,count);return 0;
        }catch(const document::Failure& e){return e.code;}catch(const std::bad_alloc&){return 8;}catch(...){return 12;}
    }
    static int32_t write(void* pointer,const uint8_t* bytes,size_t count)noexcept{
        auto& s=*static_cast<Writer*>(pointer);
        try{
            evaluation::checkpoint(s.cancel,evaluation::Checkpoint::tile);
            if(count>(64ULL<<20)-s.bytes)throw document::Failure{8};
            s.sink.write(bytes,count);s.bytes+=count;return 0;
        }catch(const document::Failure& e){return e.code;}catch(const std::bad_alloc&){return 8;}catch(...){return 12;}
    }
};
}
uint64_t stream_export_v2(const StagePlanV2& plan,uint32_t format,int quality,tiled::Budget& budget,StagedSinkV2& sink,const evaluation::Cancellation* cancel){
    try{
        if(cancel&&((cancel->generation==nullptr)!=(cancel->revoked==nullptr)))throw document::Failure{1};
        if(std::fegetround()!=FE_TONEAREST)throw document::Failure{7};
        evaluation::checkpoint(cancel,evaluation::Checkpoint::admission);
        const auto extent=plan.extent();const auto band_bytes=uint64_t(extent.width)*128*4;
        auto reservation=budget.reserve(band_bytes+(32ULL<<20)+65536);
        Writer writer{plan,budget,sink,cancel,std::vector<uint8_t>(static_cast<std::size_t>(band_bytes))};
        if(writer.band.capacity()>band_bytes)throw document::Failure{8};
        const auto code=pixaura_private_stream_encode_v2(format,extent.width,extent.height,quality,32u<<20,Writer::row,Writer::write,&writer);
        if(code)throw document::Failure{code};
        evaluation::checkpoint(cancel,evaluation::Checkpoint::publication);
        if(!writer.bytes)throw document::Failure{17};
        sink.seal();return writer.bytes;
    }catch(...){sink.abort();throw;}
}
}
