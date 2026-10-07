#include "../src/preview.hpp"
#include "../src/preview_thresholds.hpp"
#include "../src/geometry.hpp"
#include "../src/storage.hpp"
#include "../../../tests/fixtures/decode/fixtures.h"
#include <array>
#include <atomic>
#include <cmath>
#include <cfenv>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <limits>
#include <thread>
using namespace pixaura;
namespace pixaura::preview { void test_context_budget(pixaura_decode_context*,uint64_t); }
static std::atomic<unsigned> checks{0};
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"preview check %d\n",__LINE__);std::exit(1);}}while(false)
template<class F> static int32_t failure(F&& f){try{f();return 0;}catch(const document::Failure& e){return e.code;}}
static working::Image image(uint32_t w,uint32_t h){working::Image out;out.metadata={1,sizeof(pixaura_working_metadata),w,h,1,1,1,0,uint64_t(w)*16,uint64_t(w)*h*16};out.pixels=std::make_unique<decode::Vector<float>>(static_cast<std::size_t>(w)*h*4);for(std::size_t i=3;i<out.pixels->size();i+=4)(*out.pixels)[i]=1;return out;}
static uint8_t reference(double v){v=std::max(0.,std::min(1.,v));const double s=v<=.0031308?12.92*v:1.055*std::pow(v,1./2.4)-.055;return static_cast<uint8_t>(std::floor(s*255+.5));}
struct CancelAt{evaluation::Cancellation cancel;unsigned count=0,target=UINT32_MAX;};
static void inject(evaluation::Checkpoint,void* p){auto& s=*static_cast<CancelAt*>(p);if(s.count++==s.target)s.cancel.requested.store(true);}
struct Reader:storage::Reader{std::size_t at=0;std::size_t read(uint8_t* p,std::size_t n)override{const auto bytes=std::min(n,sizeof(decode_png)-at);std::memcpy(p,decode_png+at,bytes);at+=bytes;return bytes;}};

static pixaura_preview_request fit(uint32_t w,uint32_t h){return {1,sizeof(pixaura_preview_request),1,w,h,0};}
// Independent four-weight oracle, no production layout/encode/resample calls.
static std::vector<uint8_t> fit_reference(const working::Image& s,uint32_t w,uint32_t h){
    std::vector<uint8_t> out(static_cast<std::size_t>(w)*h*4);
    for(uint32_t y=0;y<h;++y)for(uint32_t x=0;x<w;++x){
        const double px=(2.0*x+1)*s.metadata.width/(2.0*w)-.5,py=(2.0*y+1)*s.metadata.height/(2.0*h)-.5;
        const int left=static_cast<int>(std::floor(px)),top=static_cast<int>(std::floor(py));
        const double u=px-left,v=py-top;double channels[4]={};
        for(int j=0;j<2;++j)for(int i=0;i<2;++i){
            const auto xx=std::max(0,std::min(left+i,int(s.metadata.width)-1)),yy=std::max(0,std::min(top+j,int(s.metadata.height)-1));
            const double weight=(i?u:1-u)*(j?v:1-v);
            for(unsigned k=0;k<4;++k)channels[k]+=double((*s.pixels)[(static_cast<std::size_t>(yy)*s.metadata.width+static_cast<unsigned>(xx))*4+k])*weight;
        }
        const auto at=(static_cast<std::size_t>(y)*w+x)*4;
        for(unsigned k=0;k<3;++k)out[at+k]=channels[3]==0?0:reference(channels[k]/channels[3]);
        out[at+3]=static_cast<uint8_t>(std::floor(channels[3]*255+.5));
    }
    return out;
}
static void fit_tests(){
    auto checker=image(2,2);for(unsigned i=0;i<4;++i)for(unsigned k=0;k<3;++k)(*checker.pixels)[i*4+k]=(i==0||i==3)?1.f:0.f;
    auto result=preview::render(checker,fit(1,1));CHECK(*result.pixels==decode::Vector<uint8_t>({188,188,188,255}));
    auto edge=image(2,2);for(unsigned i=0;i<4;++i){(*edge.pixels)[i*4]=i%2?0.f:1.f;(*edge.pixels)[i*4+3]=i%2?0.f:1.f;}
    CHECK(*preview::render(edge,fit(1,1)).pixels==decode::Vector<uint8_t>({255,0,0,128}));
    auto half=image(2,2);for(unsigned i=0;i<4;++i){(*half.pixels)[i*4]=float(i)*.125f;(*half.pixels)[i*4+3]=.5f;}
    CHECK(*preview::render(half,fit(1,1)).pixels==decode::Vector<uint8_t>({165,0,0,128}));
    auto odd=image(3,3);for(unsigned i=0;i<9;++i)for(unsigned k=0;k<3;++k)(*odd.pixels)[i*4+k]=float(i)/8;
    CHECK(*preview::render(odd,fit(2,2)).pixels==decode::Vector<uint8_t>({99,99,99,255,152,152,152,255,216,216,216,255,240,240,240,255}));
    for(const auto size:{geometry::Extent{4,2},{2,4},{1,17},{17,1},{7,11},{11,7}}){auto input=image(size.width,size.height);for(std::size_t i=0;i<input.pixels->size();i+=4){(*input.pixels)[i]=float(i%20)/20;}
        const auto req=fit(2,2);auto out=preview::render(input,req);const auto expected=fit_reference(input,out.metadata.width,out.metadata.height);CHECK(std::equal(out.pixels->begin(),out.pixels->end(),expected.begin()));}
    for(const auto size:{geometry::Extent{1,16384},{16384,1}}){auto input=image(size.width,size.height);auto out=preview::render(input,fit(1,1));CHECK(out.metadata.width==1&&out.metadata.height==1&&out.pixels->size()==4);}
    for(const auto size:{geometry::Extent{4,2},{2,4}}){auto input=image(size.width,size.height);for(unsigned i=0;i<8;++i)(*input.pixels)[i*4]=float(i%5)/5;auto out=preview::render(input,fit(2,2));const decode::Vector<uint8_t> expected=size.width==4?decode::Vector<uint8_t>({137,0,0,255,170,0,0,255}):decode::Vector<uint8_t>({149,0,0,255,160,0,0,255});CHECK(*out.pixels==expected);}
    uint32_t state=0x510a123u;const auto next=[&](){state=state*1664525u+1013904223u;return state;};
    for(unsigned n=0;n<2048;++n){const uint32_t w=1+next()%23,h=1+next()%23,mw=1+next()%29,mh=1+next()%29;auto input=image(w,h);
        for(std::size_t i=0;i<input.pixels->size();i+=4){const float a=float(next()%65537)/65536;(*input.pixels)[i+3]=a;for(unsigned k=0;k<3;++k)(*input.pixels)[i+k]=a*float(next()%65537)/65536;}
        const auto before=*input.pixels;auto out=preview::render(input,fit(mw,mh));CHECK(out.metadata.width<=mw&&out.metadata.height<=mh&&out.metadata.width<=w&&out.metadata.height<=h);
        uint32_t ew=w,eh=h;if(w>mw||h>mh){if(uint64_t(mw)*h<=uint64_t(mh)*w){ew=mw;eh=std::max(1u,uint32_t(uint64_t(h)*mw/w));}else{eh=mh;ew=std::max(1u,uint32_t(uint64_t(w)*mh/h));}}
        CHECK(out.metadata.width==ew&&out.metadata.height==eh);CHECK(*input.pixels==before);CHECK(*out.pixels==*preview::render(input,fit(mw,mh)).pixels);
        const auto oracle=fit_reference(input,ew,eh);CHECK(std::equal(out.pixels->begin(),out.pixels->end(),oracle.begin()));
        const pixaura_preview_request exact{1,sizeof(pixaura_preview_request),0,0,0,0};CHECK(*preview::render(input,exact).pixels==*preview::render(input).pixels);
        if(w<=mw&&h<=mh)CHECK(*out.pixels==*preview::render(input).pixels);
        auto hostile=fit(mw,mh);switch(n%6){case 0:hostile.version=2;break;case 1:hostile.struct_size=0;break;case 2:hostile.mode=UINT32_MAX;break;case 3:hostile.max_width=0;break;case 4:hostile.max_height=0;break;default:hostile.reserved=1;break;}
        CHECK(failure([&]{preview::render(input,hostile);})!=0);CHECK(preview::render(input,fit(UINT32_MAX,UINT32_MAX)).pixels->size()==uint64_t(w)*h*4);
    }
    for(float a:{0.f,.5f,1.f}){auto solid=image(9,7);for(std::size_t i=0;i<solid.pixels->size();i+=4){for(unsigned k=0;k<3;++k)(*solid.pixels)[i+k]=a;(*solid.pixels)[i+3]=a;}auto out=preview::render(solid,fit(3,2));for(std::size_t i=0;i<out.pixels->size();i+=4){CHECK((*out.pixels)[i]==(a?255:0));CHECK((*out.pixels)[i+3]==std::floor(double(a)*255+.5));}}
    auto malformed=image(9,9);(*malformed.pixels)[0]=std::numeric_limits<float>::infinity();CHECK(failure([&]{preview::render(malformed,fit(1,1));})==7);
    auto tiled=image(257,259);CancelAt baseline;preview::render(tiled,fit(129,130),&baseline.cancel,inject,&baseline);
    for(unsigned i=0;i<baseline.count;++i){CancelAt cancel;cancel.target=i;CHECK(failure([&]{preview::render(tiled,fit(129,130),&cancel.cancel,inject,&cancel);})==13);}
    // Real working normalization admission, null input: rejection precedes any allocation/read.
    decode::Metadata oversized;oversized.srgb_compatible=true;oversized.value={};auto& m=oversized.value;m.api_version=1;m.struct_size=sizeof(m);m.width=4032;m.height=3024;m.display_width=4032;m.display_height=3024;m.orientation=1;m.pixel_format=1;m.row_stride=4032*4;m.decoded_bytes=uint64_t(4032)*3024*4;
    for(const auto target:{geometry::Extent{1440,1080},{1024,768}}){CHECK(failure([&]{working::normalize(oversized,nullptr,0,working::defaults());})==8);std::printf("fit rejection source=4032x3024 target=%ux%u status=8 preview_not_started=true (not benchmark)\n",target.width,target.height);}
    for(const auto dims:{std::array<uint32_t,4>{4032,2048,1440,1080},{4032,2048,1024,768},{1920,1080,1280,720},{1024,768,1024,768},{4032,2048,256,256}}){
        auto input=image(dims[0],dims[1]);const auto start=std::chrono::steady_clock::now();auto out=preview::render(input,fit(dims[2],dims[3]));
        std::printf("fit diagnostic %ux%u -> %ux%u scale=%.9f pixels=%llu input=%llu output=%llu tiles=%u ms=%.3f\n",dims[0],dims[1],out.metadata.width,out.metadata.height,double(out.metadata.width)/dims[0],static_cast<unsigned long long>(uint64_t(out.metadata.width)*out.metadata.height),static_cast<unsigned long long>(input.metadata.image_bytes),static_cast<unsigned long long>(out.metadata.image_bytes),geometry::tile_count({out.metadata.width,out.metadata.height}),std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count());
    }
    std::printf("fit properties=2048 adversaries=2048 cancellation_checkpoints=%u PASS\n",baseline.count);
}

int main(int argc,char** argv){
    fit_tests();
    CHECK(argc==2);
    const std::array<std::array<float,4>,13> inputs={{{0,0,0,1},{1,1,1,1},{.18f,.18f,.18f,1},{1,0,0,1},{0,1,0,1},{0,0,1,1},{0,0,0,0},{.25f,.25f,.25f,.5f},{-1,2,.5f,1},{-0.f,0,-0.f,-0.f},{1e-30f,0,0,1e-30f},{.0031308f,.0031307f,.0031309f,1},{.5f,.5f,.5f,1}}};
    const std::array<std::array<uint8_t,4>,13> goldens={{{0,0,0,255},{255,255,255,255},{118,118,118,255},{255,0,0,255},{0,255,0,255},{0,0,255,255},{0,0,0,0},{188,188,188,128},{0,255,188,255},{0,0,0,0},{255,0,0,0},{10,10,10,255},{188,188,188,255}}};
    auto in=image(13,1);for(unsigned i=0;i<13;++i)std::memcpy(in.pixels->data()+i*4,inputs[i].data(),16);
    const auto original=*in.pixels;auto golden=preview::render(in);for(unsigned i=0;i<13;++i)CHECK(std::memcmp(golden.pixels->data()+i*4,goldens[i].data(),4)==0);CHECK(*in.pixels==original);
    CHECK(preview::encode(0)==0&&preview::encode(1)==255);
    // Every frozen half-byte boundary: predecessor, tie and successor.
    for(unsigned k=0;k<255;++k){const double t=preview::srgb_half_byte[k];CHECK(preview::encode(std::nextafter(t,0.))==k);CHECK(preview::encode(t)==k+1);CHECK(preview::encode(std::nextafter(t,1.))==k+1);
        const auto f=static_cast<float>(t);for(const auto x:{std::nextafter(f,0.f),f,std::nextafter(f,1.f)})CHECK(preview::encode(x)==reference(x));}
    uint32_t seed=0x509a123u;auto next=[&]{seed=seed*1664525u+1013904223u;return seed;};
    for(unsigned n=0;n<2048;++n){auto s=image(1+next()%19,1+next()%19);for(std::size_t i=0;i<s.pixels->size();i+=4){const float a=float(next()%65537)/65536;(*s.pixels)[i+3]=a;for(unsigned k=0;k<3;++k)(*s.pixels)[i+k]=a*float(int(next()%131073)-32768)/32768;}
        const auto before=*s.pixels;auto a=preview::render(s),b=preview::render(s);CHECK(a.metadata.width==s.metadata.width&&a.metadata.height==s.metadata.height);CHECK(a.pixels->size()==uint64_t(s.metadata.width)*s.metadata.height*4);CHECK(*a.pixels==*b.pixels);CHECK(*s.pixels==before);
        for(std::size_t i=0;i<s.pixels->size();i+=4){const double alpha=(*s.pixels)[i+3];for(unsigned k=0;k<3;++k)CHECK((*a.pixels)[i+k]==(alpha?reference(double((*s.pixels)[i+k])/alpha):0));CHECK((*a.pixels)[i+3]==static_cast<uint8_t>(std::floor(alpha*255+.5)));}}
    uint8_t previous=0;for(unsigned i=0;i<=65536;++i){const auto a=preview::alpha(double(i)/65536);CHECK(a>=previous);previous=a;}
    for(unsigned k=0;k<255;++k){const float t=float((k+.5)/255);for(float a:{std::nextafter(t,0.f),t,std::nextafter(t,1.f)}){auto s=image(1,1);(*s.pixels)[0]=a*.5f;(*s.pixels)[3]=a;auto o=preview::render(s);CHECK((*o.pixels)[0]==188);CHECK((*o.pixels)[3]==std::floor(double(a)*255+.5));}}
    auto bad=image(1,1);for(float v:{std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity()}){(*bad.pixels)[0]=v;CHECK(failure([&]{preview::render(bad);})==7);}(*bad.pixels)[0]=1;(*bad.pixels)[3]=0;CHECK(failure([&]{preview::render(bad);})==7);(*bad.pixels)[0]=0;(*bad.pixels)[3]=-1;CHECK(failure([&]{preview::render(bad);})==7);
    bad=image(1,1);bad.pixels->pop_back();CHECK(failure([&]{preview::render(bad);})==19);bad=image(1,1);bad.metadata.width=0;CHECK(failure([&]{preview::render(bad);})!=0);bad.metadata.width=UINT32_MAX;bad.metadata.height=UINT32_MAX;CHECK(failure([&]{preview::render(bad);})==8);
    bad=image(1,1);bad.metadata.orientation=2;CHECK(failure([&]{preview::render(bad);})==17);bad=image(1,1);bad.metadata.row_stride=17;CHECK(failure([&]{preview::render(bad);})==19);
    CHECK(std::fesetround(FE_DOWNWARD)==0);CHECK(failure([&]{preview::render(in);})==7);CHECK(std::fesetround(FE_TONEAREST)==0);
    // Fixed-seed hostile layout/channel mutations, with recovery every time.
    for(unsigned n=0;n<2048;++n){auto s=image(2,3);switch(next()%8){case 0:s.metadata.width=0;break;case 1:s.metadata.height=UINT32_MAX;break;case 2:s.metadata.image_bytes^=1;break;case 3:s.metadata.row_stride^=1;break;case 4:s.metadata.pixel_format=0;break;case 5:s.pixels->pop_back();break;case 6:(*s.pixels)[next()%24]=std::numeric_limits<float>::infinity();break;default:(*s.pixels)[3]=2;break;}CHECK(failure([&]{preview::render(s);})!=0);CHECK(preview::render(in).pixels->size()==52);}
    bad=image(1,1);bad.metadata.width=8192;bad.metadata.height=1025;CHECK(failure([&]{preview::render(bad);})==8);
    auto tiled=image(129,130);CancelAt baseline;auto complete=preview::render(tiled,&baseline.cancel,inject,&baseline);for(unsigned t=0;t<baseline.count;++t){CancelAt state;state.target=t;CHECK(failure([&]{preview::render(tiled,&state.cancel,inject,&state);})==13);}CHECK(*preview::render(tiled).pixels==*complete.pixels);
    // Full existing source/asset/normalization boundary, small aggregate budget.
    const auto root=std::filesystem::absolute(std::filesystem::path(argv[1])/"preview-root");std::filesystem::create_directories(root);const auto path=root.generic_string();storage::AssetStore store(path);Reader reader;const auto asset=store.ingest(reader,sizeof(decode_png));
    const auto setup=[&](pixaura_decode_context& c,char letter,pixaura_decode_handle& w){pixaura_decode_limits limits=decode::defaults();std::array<uint8_t,32> id{};id.fill(static_cast<uint8_t>(letter));CHECK(pixaura_decode_context_init(1,&c,sizeof(c),id.data(),32,&limits)==0);pixaura_decode_handle source{},decoded{};CHECK(pixaura_decode_open(&c,reinterpret_cast<const uint8_t*>(path.data()),path.size(),reinterpret_cast<const uint8_t*>(asset.digest.data()),64,asset.bytes,&source)==0);CHECK(pixaura_decode_image(&c,&source,&decoded)==0);const auto l=working::defaults();CHECK(pixaura_working_normalize(&c,&decoded,&l,&w)==0);CHECK(pixaura_decode_release(&c,&source)==0);preview::test_context_budget(&c,sizeof(decode_png)+24+96+23);return decoded;};
    pixaura_decode_context c{},other{};pixaura_decode_handle w{},ow{};auto decoded=setup(c,'a',w);auto od=setup(other,'b',ow);
    pixaura_decode_handle out{},sentinel{};std::memset(&out,0x5a,sizeof(out));sentinel=out;
    CHECK(pixaura_preview_create(&c,&w,1,nullptr,&out)==8);CHECK(std::memcmp(&out,&sentinel,sizeof(out))==0);CHECK(pixaura_decode_release(&c,&decoded)==0);CHECK(pixaura_preview_create(&c,&w,1,nullptr,&out)==0);
    pixaura_preview_metadata m{},unchanged{};std::memset(&m,0x5a,sizeof(m));unchanged=m;CHECK(pixaura_preview_query(&c,&w,&m)==3);CHECK(std::memcmp(&m,&unchanged,sizeof(m))==0);CHECK(pixaura_preview_query(&c,&out,&m)==0&&m.width==2&&m.height==3&&m.image_bytes==24&&m.row_stride==8);
    std::array<uint8_t,24> bytes{},other_bytes{};CHECK(pixaura_preview_copy(&c,&out,0,bytes.data(),24)==0);CHECK(bytes[0]==255&&bytes[23]==128);const auto before=bytes;CHECK(pixaura_preview_copy(&c,&out,UINT64_MAX,bytes.data(),1)==1&&bytes==before);CHECK(pixaura_preview_copy(&c,&out,23,bytes.data(),2)==1&&bytes==before);CHECK(pixaura_preview_copy(&c,&out,24,nullptr,0)==0);
    pixaura_working_metadata wm{};pixaura_decode_metadata dm{};CHECK(pixaura_working_query(&c,&out,&wm)==3);CHECK(pixaura_decode_query(&c,&out,&dm)==3);CHECK(pixaura_decode_image(&c,&out,&sentinel)==3);CHECK(pixaura_preview_release(&c,&w)==3);CHECK(pixaura_preview_create(&c,&out,1,nullptr,&sentinel)==3);CHECK(pixaura_preview_create(&c,&w,2,nullptr,&sentinel)==2);CHECK(pixaura_preview_query(&other,&out,&m)==3);
    CHECK(pixaura_decode_release(&other,&od)==0);pixaura_decode_handle op{};CHECK(pixaura_preview_create(&other,&ow,1,nullptr,&op)==0);CHECK(pixaura_preview_copy(&other,&op,0,other_bytes.data(),24)==0&&other_bytes==bytes);CHECK(pixaura_decode_context_destroy(&other)==0);
    {
        auto req=fit(1,2);pixaura_decode_handle fitted=sentinel;
        for(unsigned i=0;i<6;++i){auto invalid=req;switch(i){case 0:invalid.version=2;break;case 1:invalid.struct_size=0;break;case 2:invalid.mode=2;break;case 3:invalid.max_width=0;break;case 4:invalid.max_height=0;break;default:invalid.reserved=1;break;}CHECK(pixaura_preview_render(&c,&w,&invalid,nullptr,&fitted)==(i==0?2:1));CHECK(std::memcmp(&fitted,&sentinel,sizeof(fitted))==0);}
        CHECK(pixaura_preview_render(&c,&out,&req,nullptr,&fitted)==3);
        // Keep existing preview and working intact when aggregate admission fails.
        preview::test_context_budget(&c,sizeof(decode_png)+96+24+3);CHECK(pixaura_preview_render(&c,&w,&req,nullptr,&fitted)==8);CHECK(std::memcmp(&fitted,&sentinel,sizeof(fitted))==0);
        preview::test_context_budget(&c,decode::defaults().context_bytes);CHECK(pixaura_preview_render(&c,&w,&req,nullptr,&fitted)==0);
        std::array<uint8_t,4> pixel{};CHECK(pixaura_preview_copy(&c,&fitted,0,pixel.data(),4)==0);CHECK((pixel==std::array<uint8_t,4>({188,188,255,255})));CHECK(pixaura_preview_release(&c,&fitted)==0);
    }
    CHECK(pixaura_preview_release(&c,&out)==0);CHECK(pixaura_preview_release(&c,&out)==3);CHECK(pixaura_preview_query(&c,&out,&m)==3);
    pixaura_decode_handle cancel{};CHECK(pixaura_cancel_create(&c,1,&cancel)==0);CHECK(pixaura_cancel_signal(&c,&cancel)==0);CHECK(pixaura_preview_create(&c,&w,1,&cancel,&sentinel)==13);CHECK(pixaura_cancel_release(&c,&cancel)==0);CHECK(pixaura_preview_create(&c,&w,1,&cancel,&sentinel)==3);
    // Combined cap includes cancellations. Resource rejection permits release/retry.
    std::array<pixaura_decode_handle,63> tokens{};for(auto& t:tokens)CHECK(pixaura_cancel_create(&c,1,&t)==0);CHECK(pixaura_preview_create(&c,&w,1,nullptr,&sentinel)==8);CHECK(pixaura_cancel_release(&c,&tokens.back())==0);CHECK(pixaura_preview_create(&c,&w,1,nullptr,&out)==0);CHECK(pixaura_preview_release(&c,&out)==0);for(unsigned i=0;i<62;++i)CHECK(pixaura_cancel_release(&c,&tokens[i])==0);
    // Serialized read/release races have only complete success or stale status.
    CHECK(pixaura_preview_create(&c,&w,1,nullptr,&out)==0);std::thread reading([&]{for(unsigned i=0;i<100;++i){std::array<uint8_t,24> b{};const auto status=pixaura_preview_copy(&c,&out,0,b.data(),24);CHECK(status==0||status==3);if(status==0)CHECK(b==bytes);}});CHECK(pixaura_preview_release(&c,&out)==0);reading.join();
    for(unsigned i=0;i<128;++i){pixaura_decode_handle token{},result=sentinel;CHECK(pixaura_cancel_create(&c,1,&token)==0);std::thread signal([&]{CHECK(pixaura_cancel_signal(&c,&token)==0);});const auto req=fit(1,2);const auto status=i%2?pixaura_preview_create(&c,&w,1,&token,&result):pixaura_preview_render(&c,&w,&req,&token,&result);signal.join();CHECK(status==0||status==13);if(status==0)CHECK(pixaura_preview_release(&c,&result)==0);else CHECK(std::memcmp(&result,&sentinel,sizeof(result))==0);CHECK(pixaura_cancel_release(&c,&token)==0);}
    preview::test_context_budget(&c,decode::defaults().context_bytes);
    for(unsigned i=0;i<64;++i){pixaura_decode_handle working{},result=sentinel;const auto limits=working::defaults();CHECK(pixaura_working_identity(&c,&w,&limits,&working)==0);std::thread release([&]{CHECK(pixaura_decode_release(&c,&working)==0);});const auto req=fit(1,2);const auto status=i%2?pixaura_preview_create(&c,&working,1,nullptr,&result):pixaura_preview_render(&c,&working,&req,nullptr,&result);release.join();CHECK(status==0||status==3);if(status==0)CHECK(pixaura_preview_release(&c,&result)==0);else CHECK(std::memcmp(&result,&sentinel,sizeof(result))==0);}
    CHECK(pixaura_preview_create(&c,&w,1,nullptr,&out)==0);CHECK(pixaura_decode_release(&c,&w)==0);CHECK(pixaura_preview_copy(&c,&out,0,bytes.data(),24)==0&&bytes==before);CHECK(pixaura_preview_create(&c,&w,1,nullptr,&sentinel)==3);CHECK(pixaura_preview_release(&c,&out)==0);CHECK(pixaura_decode_context_destroy(&c)==0);CHECK(pixaura_preview_query(&c,&out,&m)==3);store.verify(asset);
    for(const auto size:{geometry::Extent{256,256},{1024,768},{4096,2048}}){auto s=image(size.width,size.height);unsigned tiles=0;const auto observe=[](evaluation::Checkpoint p,void* state){if(p==evaluation::Checkpoint::tile)++*static_cast<unsigned*>(state);};const auto start=std::chrono::steady_clock::now();auto result=preview::render(s,nullptr,observe,&tiles);CHECK(result.pixels->size()==uint64_t(size.width)*size.height*4);std::printf("preview diagnostic %ux%u input=%llu output=%llu tiles=%u ms=%.3f\n",size.width,size.height,static_cast<unsigned long long>(s.metadata.image_bytes),static_cast<unsigned long long>(result.metadata.image_bytes),tiles,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count());}
    std::printf("preview checks=%u properties=2048 checkpoints=%u cancellation_races=128 PASS\n",checks.load(),baseline.count);
}
