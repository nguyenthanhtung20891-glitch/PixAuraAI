#include "preview.hpp"
#include "preview_thresholds.hpp"
#include "geometry.hpp"
#include <cfenv>
#include <cmath>
#include <cstring>
namespace pixaura::preview {
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
