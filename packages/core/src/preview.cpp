#include "preview.hpp"
#include "preview_thresholds.hpp"
#include "geometry.hpp"
#include <cfenv>
#include <cmath>
#include <cstring>
#include <limits>
namespace pixaura::preview {
static_assert(sizeof(double)==8&&std::numeric_limits<double>::is_iec559,"IEEE binary64 required");
namespace {
void need(bool ok,int32_t code){if(!ok)throw document::Failure{code};}
bool finite(float v){uint32_t b=0;std::memcpy(&b,&v,4);return (b&0x7f800000u)!=0x7f800000u;}
}
pixaura_preview_metadata layout(const working::Image& source){
    const auto& m=source.metadata;
    need(m.api_version==1&&m.struct_size==sizeof(m)&&m.orientation==1&&m.pixel_format==1,17);
    const auto bytes=geometry::admit({m.width,m.height},working::defaults());
    need(m.row_stride==decode::multiply(m.width,16)&&m.image_bytes==bytes&&source.pixels&&source.pixels->size()==bytes/4,19);
    return {1,sizeof(pixaura_preview_metadata),m.width,m.height,1,0,decode::multiply(m.width,4),bytes/4};
}
uint8_t encode(double linear){
    // Quantized standard sRGB OETF. Frozen binary64 inverse half-byte
    // thresholds remove runtime libm variability. Equality rounds upward.
    need(std::isfinite(linear),7);
    if(linear<=0)return 0;
    if(linear>=1)return 255;
    unsigned lo=0,hi=255;
    while(lo<hi){const auto mid=(lo+hi)/2;if(linear>=srgb_half_byte[mid])lo=mid+1;else hi=mid;}
    return static_cast<uint8_t>(lo);
}
uint8_t alpha(double value){need(std::isfinite(value)&&value>=0&&value<=1,7);return static_cast<uint8_t>(std::floor(value*255.0+0.5));}
pixaura_preview_metadata layout(const working::Image& source,const pixaura_preview_request& request){
    need(request.version==1,2);
    need(request.struct_size==sizeof(request)&&request.reserved==0,1);
    need(request.mode==PIXAURA_PREVIEW_EXACT||request.mode==PIXAURA_PREVIEW_FIT,1);
    auto out=layout(source);
    if(request.mode==PIXAURA_PREVIEW_EXACT){need(request.max_width==0&&request.max_height==0,1);return out;}
    need(request.max_width>0&&request.max_height>0,1);
    if(out.width<=request.max_width&&out.height<=request.max_height)return out;
    const uint64_t w=out.width,h=out.height;
    if(decode::multiply(request.max_width,h)<=decode::multiply(request.max_height,w)){
        out.width=request.max_width;out.height=static_cast<uint32_t>(std::max(uint64_t(1),decode::multiply(h,out.width)/w));
    }else{
        out.height=request.max_height;out.width=static_cast<uint32_t>(std::max(uint64_t(1),decode::multiply(w,out.height)/h));
    }
    need(out.width<=w&&out.height<=h&&out.width<=request.max_width&&out.height<=request.max_height,8);
    out.row_stride=decode::multiply(out.width,4);out.image_bytes=decode::multiply(out.row_stride,out.height);return out;
}
Image render(const working::Image& source,const pixaura_preview_request& request,
    const evaluation::Cancellation* cancel,evaluation::Observer observer,void* state){
    const auto check=[&](evaluation::Checkpoint p){evaluation::checkpoint(cancel,p,observer,state);};
    check(evaluation::Checkpoint::admission);need(std::fegetround()==FE_TONEAREST,7);
    const auto metadata=layout(source,request);
    if(metadata.width==source.metadata.width&&metadata.height==source.metadata.height)
        return render(source,cancel,observer,state);
    // Validate even unsampled pixels. No scratch raster or persistent plan.
    const auto& pixels=*source.pixels;
    for(std::size_t i=0;i<pixels.size();i+=4){
        if(i%4096==0)check(evaluation::Checkpoint::tile);
        for(unsigned k=0;k<4;++k)need(finite(pixels[i+k]),7);
        const auto a=pixels[i+3];need(a>=0&&a<=1,7);
        need(a!=0||(pixels[i]==0&&pixels[i+1]==0&&pixels[i+2]==0),7);
    }
    check(evaluation::Checkpoint::allocation);Image out;out.metadata=metadata;
    out.pixels=std::make_unique<decode::Vector<uint8_t>>(static_cast<std::size_t>(metadata.image_bytes));
    need(out.pixels->capacity()<=metadata.image_bytes,8);check(evaluation::Checkpoint::allocation);
    const geometry::Extent size{metadata.width,metadata.height};uint32_t interval=0;
    for(uint32_t t=0;t<geometry::tile_count(size);++t){
        check(evaluation::Checkpoint::tile);const auto tile=geometry::tile(size,t);
        for(uint32_t y=tile.y0;y<tile.y1;++y)for(uint32_t x=tile.x0;x<tile.x1;++x){
            if(interval++%1024==0)check(evaluation::Checkpoint::tile);
            const double sx=((double(x)+0.5)*double(source.metadata.width))/double(size.width)-0.5;
            const double sy=((double(y)+0.5)*double(source.metadata.height))/double(size.height)-0.5;
            const auto ix=static_cast<int64_t>(std::floor(sx)),iy=static_cast<int64_t>(std::floor(sy));
            const double tx=sx-double(ix),ty=sy-double(iy);
            const auto clamp=[](int64_t v,uint32_t limit){return static_cast<uint32_t>(std::max(int64_t(0),std::min(v,int64_t(limit)-1)));};
            const auto x0=clamp(ix,source.metadata.width),x1=clamp(ix+1,source.metadata.width);
            const auto y0=clamp(iy,source.metadata.height),y1=clamp(iy+1,source.metadata.height);
            double value[4];
            for(unsigned k=0;k<4;++k){
                const auto at=[&](uint32_t xx,uint32_t yy){return double(pixels[static_cast<std::size_t>((uint64_t(yy)*source.metadata.width+xx)*4+k)]);};
                const double upper=at(x0,y0)*(1.0-tx)+at(x1,y0)*tx;
                const double lower=at(x0,y1)*(1.0-tx)+at(x1,y1)*tx;
                value[k]=upper*(1.0-ty)+lower*ty;
            }
            const auto i=static_cast<std::size_t>((uint64_t(y)*size.width+x)*4);const double a=value[3];
            for(unsigned k=0;k<3;++k)(*out.pixels)[i+k]=a==0?0:encode(value[k]/a);
            (*out.pixels)[i+3]=alpha(a);
        }
    }
    check(evaluation::Checkpoint::publication);return out;
}
Image render(const working::Image& source,const evaluation::Cancellation* cancel,evaluation::Observer observer,void* state){
    const auto check=[&](evaluation::Checkpoint p){evaluation::checkpoint(cancel,p,observer,state);};
    check(evaluation::Checkpoint::admission);
    need(std::fegetround()==FE_TONEAREST,7);
    Image out;out.metadata=layout(source);
    check(evaluation::Checkpoint::allocation);
    out.pixels=std::make_unique<decode::Vector<uint8_t>>(static_cast<std::size_t>(out.metadata.image_bytes));
    need(out.pixels->capacity()<=out.metadata.image_bytes,8);
    check(evaluation::Checkpoint::allocation);
    const geometry::Extent size{out.metadata.width,out.metadata.height};
    for(uint32_t t=0;t<geometry::tile_count(size);++t){
        check(evaluation::Checkpoint::tile);const auto tile=geometry::tile(size,t);
        for(uint32_t y=tile.y0;y<tile.y1;++y)for(uint32_t x=tile.x0;x<tile.x1;++x){
            const auto i=static_cast<std::size_t>((uint64_t(y)*size.width+x)*4);
            const auto* p=source.pixels->data()+i;
            for(unsigned k=0;k<4;++k)need(finite(p[k]),7);
            const double a=p[3];need(a>=0&&a<=1,7);
            need(a!=0||(p[0]==0&&p[1]==0&&p[2]==0),7);
            for(unsigned k=0;k<3;++k)(*out.pixels)[i+k]=a==0?0:encode(double(p[k])/a);
            (*out.pixels)[i+3]=alpha(a);
        }
    }
    check(evaluation::Checkpoint::publication);
    return out;
}
}
