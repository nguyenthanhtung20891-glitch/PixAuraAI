#include "preview_v2.hpp"
#include "../../preview.hpp"
#include <algorithm>
#include <cfenv>
#include <cmath>
#include <cstring>
namespace pixaura::integration {
PreviewV2 fit_preview_v2(const StagePlanV2& plan,uint32_t max_width,uint32_t max_height,tiled::Budget& budget,const evaluation::Cancellation* cancel){
    if(!max_width||!max_height||max_width>16384||max_height>16384||std::fegetround()!=FE_TONEAREST)throw document::Failure{7};
    if(cancel&&((cancel->generation==nullptr)!=(cancel->revoked==nullptr)))throw document::Failure{1};
    const auto source=plan.extent();uint32_t width=source.width,height=source.height;
    if(width>max_width||height>max_height){
        if(uint64_t(max_width)*height<=uint64_t(max_height)*width){width=max_width;height=static_cast<uint32_t>(std::max(uint64_t(1),uint64_t(source.height)*width/source.width));}
        else{height=max_height;width=static_cast<uint32_t>(std::max(uint64_t(1),uint64_t(source.width)*height/source.height));}
    }
    const uint64_t output_bytes=uint64_t(width)*height*4,row_bytes=uint64_t(source.width)*2*16;
    auto lease=std::unique_ptr<tiled::Budget::Lease>(new tiled::Budget::Lease(budget.reserve(output_bytes+row_bytes+65536)));
    // Like frozen v1 FIT, validate EVERY evaluated pixel, even those not sampled.
    for(uint32_t y=0;y<source.height;y+=128)for(uint32_t x=0;x<source.width;x+=128){
        auto block=plan.render({x,y,std::min(x+128,source.width),std::min(y+128,source.height)},budget,cancel);
        const auto& pixels=*block.image.pixels;
        for(std::size_t i=0;i<pixels.size();i+=4){
            const float a=pixels[i+3];if(!std::isfinite(a)||a<0||a>1)throw document::Failure{7};
            for(unsigned k=0;k<3;++k)if(!std::isfinite(pixels[i+k])||(a==0&&pixels[i+k]!=0))throw document::Failure{7};
        }
    }
    PreviewV2 result{std::move(lease),source,width,height,std::vector<uint8_t>(static_cast<std::size_t>(output_bytes))};
    std::vector<float> rows(static_cast<std::size_t>(row_bytes/4));
    if(result.pixels.capacity()>output_bytes||rows.capacity()*4>row_bytes)throw document::Failure{8};
    const auto clamp=[](int64_t v,uint32_t limit){return static_cast<uint32_t>(std::max(int64_t(0),std::min(v,int64_t(limit)-1)));};
    for(uint32_t y=0;y<height;++y){
        evaluation::checkpoint(cancel,evaluation::Checkpoint::tile);
        const double sy=((double(y)+0.5)*double(source.height))/double(height)-0.5;
        const auto iy=static_cast<int64_t>(std::floor(sy));const double ty=sy-double(iy);
        const auto y0=clamp(iy,source.height),y1=clamp(iy+1,source.height);
        for(uint32_t x=0;x<source.width;x+=128){
            auto block=plan.render({x,y0,std::min(x+128,source.width),y1+1},budget,cancel);
            for(unsigned row=0;row<2;++row){
                const auto local=row==0?0:y1-y0;
                std::memcpy(rows.data()+(std::size_t(row)*source.width+x)*4,block.image.pixels->data()+std::size_t(local)*block.image.metadata.width*4,block.image.metadata.width*16);
            }
        }
        for(uint32_t x=0;x<width;++x){
            const double sx=((double(x)+0.5)*double(source.width))/double(width)-0.5;
            const auto ix=static_cast<int64_t>(std::floor(sx));const double tx=sx-double(ix);
            const auto x0=clamp(ix,source.width),x1=clamp(ix+1,source.width);double values[4];
            for(unsigned k=0;k<4;++k){
                const double upper=double(rows[x0*4+k])*(1.0-tx)+double(rows[x1*4+k])*tx;
                const double lower=double(rows[(std::size_t(source.width)+x0)*4+k])*(1.0-tx)+double(rows[(std::size_t(source.width)+x1)*4+k])*tx;
                values[k]=upper*(1.0-ty)+lower*ty;
            }
            auto* out=result.pixels.data()+(std::size_t(y)*width+x)*4;const double a=values[3];
            for(unsigned k=0;k<3;++k)out[k]=a==0?0:preview::encode(values[k]/a);
            out[3]=preview::alpha(a);
        }
    }
    evaluation::checkpoint(cancel,evaluation::Checkpoint::publication);return result;
}
}
