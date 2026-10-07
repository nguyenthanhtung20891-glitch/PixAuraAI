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
int main(int argc,char** argv){
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
    CHECK(pixaura_preview_release(&c,&out)==0);CHECK(pixaura_preview_release(&c,&out)==3);CHECK(pixaura_preview_query(&c,&out,&m)==3);
    pixaura_decode_handle cancel{};CHECK(pixaura_cancel_create(&c,1,&cancel)==0);CHECK(pixaura_cancel_signal(&c,&cancel)==0);CHECK(pixaura_preview_create(&c,&w,1,&cancel,&sentinel)==13);CHECK(pixaura_cancel_release(&c,&cancel)==0);CHECK(pixaura_preview_create(&c,&w,1,&cancel,&sentinel)==3);
    // Combined cap includes cancellations. Resource rejection permits release/retry.
    std::array<pixaura_decode_handle,63> tokens{};for(auto& t:tokens)CHECK(pixaura_cancel_create(&c,1,&t)==0);CHECK(pixaura_preview_create(&c,&w,1,nullptr,&sentinel)==8);CHECK(pixaura_cancel_release(&c,&tokens.back())==0);CHECK(pixaura_preview_create(&c,&w,1,nullptr,&out)==0);CHECK(pixaura_preview_release(&c,&out)==0);for(unsigned i=0;i<62;++i)CHECK(pixaura_cancel_release(&c,&tokens[i])==0);
    // Serialized read/release races have only complete success or stale status.
    CHECK(pixaura_preview_create(&c,&w,1,nullptr,&out)==0);std::thread reading([&]{for(unsigned i=0;i<100;++i){std::array<uint8_t,24> b{};const auto status=pixaura_preview_copy(&c,&out,0,b.data(),24);CHECK(status==0||status==3);if(status==0)CHECK(b==bytes);}});CHECK(pixaura_preview_release(&c,&out)==0);reading.join();
    for(unsigned i=0;i<128;++i){pixaura_decode_handle token{},result=sentinel;CHECK(pixaura_cancel_create(&c,1,&token)==0);std::thread signal([&]{CHECK(pixaura_cancel_signal(&c,&token)==0);});const auto status=pixaura_preview_create(&c,&w,1,&token,&result);signal.join();CHECK(status==0||status==13);if(status==0)CHECK(pixaura_preview_release(&c,&result)==0);else CHECK(std::memcmp(&result,&sentinel,sizeof(result))==0);CHECK(pixaura_cancel_release(&c,&token)==0);}
    preview::test_context_budget(&c,decode::defaults().context_bytes);
    for(unsigned i=0;i<64;++i){pixaura_decode_handle working{},result=sentinel;const auto limits=working::defaults();CHECK(pixaura_working_identity(&c,&w,&limits,&working)==0);std::thread release([&]{CHECK(pixaura_decode_release(&c,&working)==0);});const auto status=pixaura_preview_create(&c,&working,1,nullptr,&result);release.join();CHECK(status==0||status==3);if(status==0)CHECK(pixaura_preview_release(&c,&result)==0);else CHECK(std::memcmp(&result,&sentinel,sizeof(result))==0);}
    CHECK(pixaura_preview_create(&c,&w,1,nullptr,&out)==0);CHECK(pixaura_decode_release(&c,&w)==0);CHECK(pixaura_preview_copy(&c,&out,0,bytes.data(),24)==0&&bytes==before);CHECK(pixaura_preview_create(&c,&w,1,nullptr,&sentinel)==3);CHECK(pixaura_preview_release(&c,&out)==0);CHECK(pixaura_decode_context_destroy(&c)==0);CHECK(pixaura_preview_query(&c,&out,&m)==3);store.verify(asset);
    for(const auto size:{geometry::Extent{256,256},{1024,768},{4096,2048}}){auto s=image(size.width,size.height);unsigned tiles=0;const auto observe=[](evaluation::Checkpoint p,void* state){if(p==evaluation::Checkpoint::tile)++*static_cast<unsigned*>(state);};const auto start=std::chrono::steady_clock::now();auto result=preview::render(s,nullptr,observe,&tiles);CHECK(result.pixels->size()==uint64_t(size.width)*size.height*4);std::printf("preview diagnostic %ux%u input=%llu output=%llu tiles=%u ms=%.3f\n",size.width,size.height,static_cast<unsigned long long>(s.metadata.image_bytes),static_cast<unsigned long long>(result.metadata.image_bytes),tiles,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count());}
    std::printf("preview checks=%u properties=2048 checkpoints=%u cancellation_races=128 PASS\n",checks.load(),baseline.count);
}
