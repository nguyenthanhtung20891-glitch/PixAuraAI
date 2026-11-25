#include "../core/src/v2/integration/stream_codec_v2.h"
#include "../core/src/decode.hpp"
#include "../core/src/decode_codec.h"
#include "../../tests/fixtures/decode/fixtures.h"
#include <vector>
#include <cstring>
#include <cstdio>
#include <stdexcept>
using namespace pixaura;
#define CHECK(x) do{if(!(x))throw std::runtime_error("row codec line "+std::to_string(__LINE__));}while(false)
struct Rows {
    uint32_t width,height;std::vector<uint8_t> pixels;unsigned writes=0,reads=0;int fault=0;
    Rows(uint32_t w,uint32_t h):width(w),height(h),pixels(size_t(w)*h*4){}
    static int32_t exchange(void* raw,uint32_t y,uint8_t* row,size_t bytes,int write)noexcept{
        auto& s=*static_cast<Rows*>(raw);if(y>=s.height||bytes!=size_t(s.width)*4||!row||(write!=0&&write!=1))return 19;
        if(s.fault)return s.fault;
        auto* target=s.pixels.data()+size_t(y)*bytes;
        if(write){std::memcpy(target,row,bytes);++s.writes;}else{std::memcpy(row,target,bytes);++s.reads;}return 0;
    }
};
void certify(const std::vector<uint8_t>& bytes){
    const auto limits=decode::defaults();auto metadata=decode::admit(bytes.data(),bytes.size(),limits);const auto m=metadata.value;
    decode::Source source(std::make_unique<decode::Vector<uint8_t>>(bytes.begin(),bytes.end()),std::move(metadata));
    auto expected=decode::execute(source,limits);Rows actual(m.width,m.height);
    CHECK(pixaura_private_stream_decode_v2(m.format,bytes.data(),bytes.size(),m.width,m.height,32u<<20,Rows::exchange,&actual)==0);
    CHECK(actual.pixels.size()==expected->size()&&!std::memcmp(actual.pixels.data(),expected->data(),expected->size()));CHECK(actual.writes>=m.height);
    for(const auto error:{12,13}){Rows failed(m.width,m.height);failed.fault=error;CHECK(pixaura_private_stream_decode_v2(m.format,bytes.data(),bytes.size(),m.width,m.height,32u<<20,Rows::exchange,&failed)==error);}
    Rows limited(m.width,m.height);CHECK(pixaura_private_stream_decode_v2(m.format,bytes.data(),bytes.size(),m.width,m.height,64,Rows::exchange,&limited)==8&&limited.writes==0);
    for(size_t n=1;n<bytes.size();++n){Rows truncated(m.width,m.height);const auto code=pixaura_private_stream_decode_v2(m.format,bytes.data(),n,m.width,m.height,32u<<20,Rows::exchange,&truncated);CHECK(code!=0);}
}
int main(){try{
    unsigned cases=0;
    for(const auto& fixture:std::vector<std::vector<uint8_t>>{
        {std::begin(decode_png),std::end(decode_png)}, {std::begin(decode_jpeg),std::end(decode_jpeg)},
        {std::begin(decode_gray1),std::end(decode_gray1)}, {std::begin(decode_rgb),std::end(decode_rgb)},
        {std::begin(decode_grayalpha),std::end(decode_grayalpha)}, {std::begin(decode_palette),std::end(decode_palette)},
        {std::begin(decode_adam7),std::end(decode_adam7)}}){certify(fixture);++cases;}
    constexpr uint32_t w=129,h=257;std::vector<uint8_t> rgba(size_t(w)*h*4);
    uint32_t state=0x91e10da5;for(auto& b:rgba){state^=state<<13;state^=state>>17;state^=state<<5;b=static_cast<uint8_t>(state);}
    for(unsigned format=1;format<=2;++format){std::vector<uint8_t> encoded(rgba.size()*2+65536);size_t count=0;
        CHECK((format==1?pixaura_codec_encode_jpeg(rgba.data(),w,h,93,encoded.data(),encoded.size(),&count,32u<<20):pixaura_codec_encode_png(rgba.data(),w,h,encoded.data(),encoded.size(),&count,32u<<20))==0);
        encoded.resize(count);
        // Large fixtures use full-byte oracle/callback failures; exhaustive
        // truncation above covers small pinned fixtures without quadratic work.
        auto metadata=decode::admit(encoded.data(),encoded.size(),decode::defaults());decode::Source source(std::make_unique<decode::Vector<uint8_t>>(encoded.begin(),encoded.end()),std::move(metadata));
        auto expected=decode::execute(source,decode::defaults());Rows actual(w,h);
        CHECK(pixaura_private_stream_decode_v2(format,encoded.data(),encoded.size(),w,h,4u<<20,Rows::exchange,&actual)==0);
        CHECK(!std::memcmp(actual.pixels.data(),expected->data(),expected->size()));++cases;
    }
    std::printf("row codec v2: %u frozen RGBA byte oracles; gray/palette/alpha/Adam7/JPEG/PNG; all small truncations, callback storage/cancel and cap failure PASS\n",cases);return 0;
}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}catch(const document::Failure& e){std::fprintf(stderr,"row codec native%d\n",e.code);return 1;}}
