#include "working.hpp"
#include "srgb_table.hpp"
#include <cstring>
#include <limits>
namespace pixaura::working {
namespace {
void need(bool ok,int32_t code=17){if(!ok)throw document::Failure{code};}
static_assert(sizeof(float)==4&&std::numeric_limits<float>::is_iec559,"IEEE binary32 required");
}
pixaura_working_limits defaults(){return {1,sizeof(pixaura_working_limits),16384,16384,8388608,262144,134217728};}
void validate(const pixaura_working_limits& l){const auto d=defaults();
    need(l.api_version==1,2);need(l.struct_size==sizeof(l),1);
    need(l.width&&l.width<=d.width&&l.height&&l.height<=d.height&&l.pixel_count&&l.pixel_count<=d.pixel_count&&l.row_stride&&l.row_stride<=d.row_stride&&l.image_bytes&&l.image_bytes<=d.image_bytes,1);
}
pixaura_working_metadata layout(const decode::Metadata& m,const pixaura_working_limits& l){validate(l);const auto& v=m.value;
    need(v.api_version==1&&v.struct_size==sizeof(v)&&v.orientation>=1&&v.orientation<=8&&v.width&&v.height&&v.pixel_format==1);
    need(v.profile_type<=2&&((v.profile_type==1&&v.profile_bytes==m.profile.size()&&!m.profile.empty())||(v.profile_type!=1&&v.profile_bytes==0&&m.profile.empty())));
    const uint64_t encoded_stride=decode::multiply(v.width,4);need(v.row_stride==encoded_stride&&v.decoded_bytes==decode::multiply(encoded_stride,v.height));
    const auto w=v.orientation>=5?v.height:v.width,h=v.orientation>=5?v.width:v.height;
    need(v.display_width==w&&v.display_height==h);
    need(v.profile_type!=1&&m.srgb_compatible,16);
    const auto pixels=decode::multiply(w,h),stride=decode::multiply(w,16),bytes=decode::multiply(stride,h);
    need(w<=l.width&&h<=l.height&&pixels<=l.pixel_count&&stride<=l.row_stride&&bytes<=l.image_bytes&&bytes<=SIZE_MAX,8);
    return {1,sizeof(pixaura_working_metadata),w,h,1,1,v.orientation,v.profile_type,stride,bytes};
}
Image normalize(const decode::Metadata& m,const uint8_t* input,std::size_t n,const pixaura_working_limits& l){
    Image out;out.metadata=layout(m,l);need(input&&n==m.value.decoded_bytes,19);
    out.pixels=std::make_unique<decode::Vector<float>>(static_cast<std::size_t>(out.metadata.image_bytes/4));
    const auto w=m.value.width,h=m.value.height,o=m.value.orientation;
    for(uint32_t y=0;y<h;++y)for(uint32_t x=0;x<w;++x){uint32_t dx=x,dy=y;
        switch(o){case 2:dx=w-1-x;break;case 3:dx=w-1-x;dy=h-1-y;break;case 4:dy=h-1-y;break;
        case 5:dx=y;dy=x;break;case 6:dx=h-1-y;dy=x;break;case 7:dx=h-1-y;dy=w-1-x;break;case 8:dx=y;dy=w-1-x;break;default:break;}
        const auto src=static_cast<std::size_t>((uint64_t(y)*w+x)*4),dst=static_cast<std::size_t>((uint64_t(dy)*out.metadata.width+dx)*4);
        const float alpha=alpha_table[input[src+3]];
        for(unsigned c=0;c<3;++c)(*out.pixels)[dst+c]=srgb_table[input[src+c]]*alpha;
        (*out.pixels)[dst+3]=alpha;
    }
    return out;
}
Image identity(const Image& in,const pixaura_working_limits& l){validate(l);const auto& m=in.metadata;
    need(m.api_version==1&&m.struct_size==sizeof(m)&&m.orientation==1&&m.pixel_format==1&&m.width&&m.height);
    const auto stride=decode::multiply(m.width,16),bytes=decode::multiply(stride,m.height);
    need(m.row_stride==stride&&m.image_bytes==bytes&&in.pixels&&in.pixels->size()==bytes/4,19);
    need(m.width<=l.width&&m.height<=l.height&&decode::multiply(m.width,m.height)<=l.pixel_count&&stride<=l.row_stride&&bytes<=l.image_bytes&&bytes<=SIZE_MAX,8);
    Image out;out.metadata=m;out.pixels=std::make_unique<decode::Vector<float>>(in.pixels->size());
    std::memcpy(out.pixels->data(),in.pixels->data(),static_cast<std::size_t>(bytes));return out;
}
}
