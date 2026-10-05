#include "../src/evaluation.hpp"
#include "../src/storage.hpp"
#include "../../../tests/fixtures/decode/fixtures.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <thread>
#include <array>
#include <filesystem>
#include <chrono>
#include <fstream>
#include <cfenv>
using namespace pixaura;
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"evaluation check %d\n",__LINE__);std::exit(1);}}while(false)
static std::string op(int ev, unsigned id=1) {
    char hex[33];std::snprintf(hex,sizeof(hex),"%032x",id);
    return std::string("{\"id\":\"")+hex+"\",\"type\":\"pixaura.exposure\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":"+std::to_string(ev)+"}}";
}
static std::string request(const std::string& operations) {return "{\"operations\":["+operations+"]}";}
static working::Image image(const std::array<float,16>& values) {
    working::Image out;out.metadata={1,sizeof(pixaura_working_metadata),2,2,1,1,1,0,32,64};
    out.pixels=std::make_unique<decode::Vector<float>>(values.begin(),values.end());return out;
}
static int32_t failure(const working::Image& in,const evaluation::Stack& stack) {
    try{evaluation::evaluate(in,stack,working::defaults());return 0;}catch(const document::Failure& e){return e.code;}
}
struct Reader:storage::Reader {
    std::size_t at=0;
    std::size_t read(uint8_t* p,std::size_t n)override{const auto bytes=std::min(n,sizeof(decode_png)-at);std::memcpy(p,decode_png+at,bytes);at+=bytes;return bytes;}
};
int main(int argc,char** argv) {
    CHECK(argc==3);
    const std::array<float,16> golden={0,0,0,0, 1,0,0,1, .25f,-.5f,2,.5f, 1e-30f,-1e-30f,0,1e-30f};
    auto in=image(golden);const auto original=*in.pixels;const auto limits=working::defaults();
    auto zero=evaluation::parse(request(op(0)));auto plus=evaluation::parse(request(op(1000)));
    auto minus=evaluation::parse(request(op(-1000,2)));
    for(const auto& stack:{evaluation::Stack{},zero}) {
        auto out=evaluation::evaluate(in,stack,limits);
        CHECK(std::memcmp(out.pixels->data(),original.data(),64)==0);CHECK(out.pixels->data()!=in.pixels->data());
    }
    auto bright=evaluation::evaluate(in,plus,limits);
    for(unsigned i=0;i<16;++i)CHECK((*bright.pixels)[i]==golden[i]*(i%4==3?1:2));
    auto composed=evaluation::parse(request(op(1000)+","+op(-1000,2)));
    auto out=evaluation::evaluate(in,composed,limits);auto separate=evaluation::evaluate(bright,minus,limits);
    CHECK(std::memcmp(out.pixels->data(),separate.pixels->data(),64)==0);
    CHECK(std::memcmp(in.pixels->data(),original.data(),64)==0);
    // Actual immutable historical replay supplies the same typed operations.
    std::ifstream manifest_file(argv[2]);const std::string manifest((std::istreambuf_iterator<char>(manifest_file)),{});
    const auto snapshot=document::deserialize(manifest,"00000000000000000000000000000500");CHECK(snapshot.code==0);
    const auto checkpoint=document::serialize(*snapshot.value);CHECK(checkpoint.code==0);
    const auto replay=document::replay(*snapshot.value,document::Id::parse("00000000000000000000000000000101"));CHECK(replay.code==0);
    auto historical=evaluation::evaluate(in,replay.value,limits);CHECK((*historical.pixels)[4]>2);
    const auto geometry=document::replay(*snapshot.value,snapshot.value->current());CHECK(geometry.code==0&&failure(in,geometry.value)==0);
    const auto after=document::serialize(*snapshot.value);CHECK(after.code==0&&after.value==checkpoint.value);
    // All 10001 legal parameters versus an independent double transcendental oracle.
    for(int ev=-5000;ev<=5000;++ev) {
        CHECK(std::abs(evaluation::gain(ev)-std::exp2(double(ev)/1000))<=1e-14*std::exp2(double(ev)/1000));
    }
    for(const auto& malformed:{"NaN","Infinity","-Infinity","1.0","1e0","-0","5001","-5001","\"0\"","null","true"}) {
        auto text=request(op(0));const auto pos=text.find("milli_ev\":0");text.replace(pos+10,1,malformed);
        CHECK(pixaura_evaluation_validate(1,reinterpret_cast<const uint8_t*>(text.data()),text.size())!=0);
    }
    for(const auto& text:{request(op(1)+","+op(1)),request("{}"),std::string("{}"),request(op(0))+"x"})
        CHECK(pixaura_evaluation_validate(1,reinterpret_cast<const uint8_t*>(text.data()),text.size())!=0);
    auto unsupported=request(op(1));unsupported.replace(unsupported.find("exposure"),8,"contrast");
    CHECK(pixaura_evaluation_validate(1,reinterpret_cast<const uint8_t*>(unsupported.data()),unsupported.size())==5);
    auto version=request(op(1));version.replace(version.find("operation_version\":1")+19,1,"2");
    CHECK(pixaura_evaluation_validate(1,reinterpret_cast<const uint8_t*>(version.data()),version.size())==5);
    auto parameter_version=request(op(1));parameter_version.replace(parameter_version.find("parameter_version\":1")+19,1,"2");
    CHECK(pixaura_evaluation_validate(1,reinterpret_cast<const uint8_t*>(parameter_version.data()),parameter_version.size())==5);
    for(const auto& parameters:{std::string("{}"),std::string("{\"milli_ev\":0,\"width_ppm\":1}"),std::string("{\"milli_ev\":0,\"unexpected\":1}")}) {
        auto text=request(op(0));const auto start=text.find("{\"milli_ev\"");text.replace(start,text.find('}',start)-start+1,parameters);
        CHECK(pixaura_evaluation_validate(1,reinterpret_cast<const uint8_t*>(text.data()),text.size())==7);
    }
    std::string long_ops;
    for(unsigned i=1;i<=256;++i){if(i>1)long_ops+=",";long_ops+=op(i%2?1:-1,i);}
    const auto legal=request(long_ops);CHECK(evaluation::parse(legal).size()==256);
    CHECK(pixaura_evaluation_validate(1,reinterpret_cast<const uint8_t*>(legal.data()),legal.size())==0);
    const auto illegal=request(long_ops+","+op(0,257));CHECK(pixaura_evaluation_validate(1,reinterpret_cast<const uint8_t*>(illegal.data()),illegal.size())==8);
    const std::string large(65537,' ');CHECK(pixaura_evaluation_validate(1,reinterpret_cast<const uint8_t*>(large.data()),large.size())==8);
    const auto padded=request("{"+std::string(1025,' ')+op(0).substr(1));CHECK(pixaura_evaluation_validate(1,reinterpret_cast<const uint8_t*>(padded.data()),padded.size())==8);
    // Finite overflow at N=2 fails, never publishes N-1; order cannot be folded.
    auto huge=image(golden);(*huge.pixels)[4]=std::numeric_limits<float>::max()/2;
    CHECK(failure(huge,evaluation::parse(request(op(1000)+","+op(1000,2))))==7);
    CHECK(failure(huge,minus)==0);
    (*huge.pixels)[4]=-std::numeric_limits<float>::max();
    CHECK(failure(huge,minus)==0);CHECK(failure(huge,plus)==7);
    for(int ev:{-5000,5000}) {
        auto endpoint=evaluation::evaluate(in,evaluation::parse(request(op(ev))),limits);
        CHECK((*endpoint.pixels)[4]==(ev<0?.03125f:32.f));CHECK((*endpoint.pixels)[7]==1);
    }
    auto order=image(golden);(*order.pixels)[4]=std::numeric_limits<float>::max();
    CHECK(failure(order,evaluation::parse(request(op(1000)+","+op(-1000,2))))==7);
    CHECK(failure(order,evaluation::parse(request(op(-1000)+","+op(1000,2))))==0);
    for(float bad:{std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity()}) {
        auto hostile=image(golden);(*hostile.pixels)[4]=bad;CHECK(failure(hostile,zero)==7);
        (*hostile.pixels)[4]=1;(*hostile.pixels)[7]=bad;CHECK(failure(hostile,zero)==7);
    }
    for(float alpha:{-.01f,1.01f}){auto hostile=image(golden);(*hostile.pixels)[7]=alpha;CHECK(failure(hostile,zero)==7);}
    auto signaling=image(golden);const uint32_t signaling_bits=0x7f800001u;
    std::memcpy(&(*signaling.pixels)[4],&signaling_bits,sizeof(signaling_bits));
    std::feclearexcept(FE_ALL_EXCEPT);CHECK(failure(signaling,zero)==7);CHECK(std::fetestexcept(FE_INVALID)==0);
    auto hidden=image(golden);(*hidden.pixels)[0]=1;CHECK(failure(hidden,zero)==7);
    auto tiny=image(golden);(*tiny.pixels)[4]=std::numeric_limits<float>::denorm_min();
    auto unchanged=evaluation::evaluate(tiny,zero,limits);CHECK(std::memcmp(tiny.pixels->data(),unchanged.pixels->data(),64)==0);
    CHECK(std::fesetround(FE_DOWNWARD)==0);CHECK(failure(in,zero)==7);CHECK(std::fesetround(FE_TONEAREST)==0);
    // Deterministic bounded numerical properties and malformed parser corpus.
    uint32_t seed=0x50495836;
    for(unsigned n=0;n<2048;++n) {
        seed=seed*1664525u+1013904223u;const int ev=int(seed%10001)-5000;
        auto values=golden;for(unsigned c=4;c<7;++c)values[c]=float(int((seed>>(c-4)*8)&255)-128)/16;
        auto source=image(values);auto result=evaluation::evaluate(source,evaluation::parse(request(op(ev))),limits);
        for(unsigned i=0;i<16;++i) {
            CHECK(std::isfinite((*result.pixels)[i]));
            if(i%4==3)CHECK((*result.pixels)[i]==values[i]);
            else CHECK(std::abs(double((*result.pixels)[i])-double(values[i])*std::exp2(double(ev)/1000))<=1e-7+2e-6*std::abs(double((*result.pixels)[i])));
        }
        auto text=request(op(ev));text[seed%text.size()]=char(seed>>24);
        const auto status=pixaura_evaluation_validate(1,reinterpret_cast<const uint8_t*>(text.data()),text.size());
        CHECK(status>=0&&status<=14);
    }
    auto maximal=evaluation::evaluate(in,evaluation::parse(legal),limits);CHECK((*maximal.pixels)[3]==0);
    // Real managed asset -> decode -> normalize -> stack -> query/copy/lifecycle.
    const auto root=std::filesystem::absolute(std::filesystem::path(argv[1])/"evaluation-root");std::filesystem::create_directories(root);
    const auto path=root.generic_string();storage::AssetStore store(path);Reader reader;const auto asset=store.ingest(reader,sizeof(decode_png));
    pixaura_decode_limits dl{};CHECK(pixaura_decode_default_limits(1,&dl)==0);
    pixaura_decode_context context{};const uint8_t id[]="eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee";
    CHECK(pixaura_decode_context_init(1,&context,sizeof(context),id,32,&dl)==0);
    pixaura_decode_handle encoded{},decoded{},working{},evaluated{};
    CHECK(pixaura_decode_open(&context,reinterpret_cast<const uint8_t*>(path.data()),path.size(),reinterpret_cast<const uint8_t*>(asset.digest.data()),64,asset.bytes,&encoded)==0);
    CHECK(pixaura_decode_image(&context,&encoded,&decoded)==0);CHECK(pixaura_working_normalize(&context,&decoded,&limits,&working)==0);
    const auto valid=request(op(1000));const auto* bytes=reinterpret_cast<const uint8_t*>(valid.data());
    CHECK(pixaura_working_evaluate(&context,&working,1,bytes,valid.size(),&limits,&evaluated)==0);
    pixaura_working_metadata metadata{};CHECK(pixaura_working_query(&context,&evaluated,&metadata)==0&&metadata.width==2&&metadata.height==3);
    float pixels[24];CHECK(pixaura_working_copy(&context,&evaluated,0,pixels,24)==0&&pixels[0]==2&&pixels[3]==1);
    const auto sentinel=evaluated;
    std::string explosive;
    for(unsigned i=1;i<=30;++i){if(i>1)explosive+=",";explosive+=op(5000,i);}
    const auto overflow=request(explosive);
    CHECK(pixaura_working_evaluate(&context,&working,1,reinterpret_cast<const uint8_t*>(overflow.data()),overflow.size(),&limits,&evaluated)==7);
    CHECK(std::memcmp(&sentinel,&evaluated,sizeof(sentinel))==0);
    CHECK(pixaura_working_evaluate(&context,&working,1,reinterpret_cast<const uint8_t*>(unsupported.data()),unsupported.size(),&limits,&evaluated)==5);
    CHECK(std::memcmp(&sentinel,&evaluated,sizeof(sentinel))==0);
    auto small=limits;small.image_bytes=1;
    CHECK(pixaura_working_evaluate(&context,&working,1,bytes,valid.size(),&small,&evaluated)==8);
    CHECK(std::memcmp(&sentinel,&evaluated,sizeof(sentinel))==0);
    CHECK(pixaura_working_evaluate(&context,&decoded,1,bytes,valid.size(),&limits,&evaluated)==3);
    CHECK(pixaura_working_evaluate(&context,&working,2,bytes,valid.size(),&limits,&evaluated)==2);
    CHECK(pixaura_working_evaluate(&context,&working,1,nullptr,1,&limits,&evaluated)==1);
    CHECK(std::memcmp(&sentinel,&evaluated,sizeof(sentinel))==0);
    // Joined workers share one immutable handle; context serializes publication.
    std::array<int32_t,4> statuses{};std::array<std::thread,4> workers;
    for(unsigned i=0;i<4;++i)workers[i]=std::thread([&,i]{pixaura_decode_handle h{};statuses[i]=pixaura_working_evaluate(&context,&working,1,bytes,valid.size(),&limits,&h);if(statuses[i]==0){float p[4];statuses[i]=pixaura_working_copy(&context,&h,0,p,4);if(p[0]!=2)statuses[i]=14;if(statuses[i]==0)statuses[i]=pixaura_working_copy(&context,&evaluated,0,p,4);if(p[0]!=2)statuses[i]=14;pixaura_decode_release(&context,&h);}});
    for(auto& worker:workers)worker.join();
    for(auto status:statuses)CHECK(status==0);
    // Independent contexts perform evaluation concurrently without global state.
    for(unsigned i=0;i<4;++i)workers[i]=std::thread([&,i]{
        char identity[33];std::snprintf(identity,sizeof(identity),"%032x",i+1000);
        pixaura_decode_context independent{};pixaura_decode_handle s{},d{},w{},h{};
        int32_t status=pixaura_decode_context_init(1,&independent,sizeof(independent),reinterpret_cast<const uint8_t*>(identity),32,&dl);
        if(status==0)status=pixaura_decode_open(&independent,reinterpret_cast<const uint8_t*>(path.data()),path.size(),reinterpret_cast<const uint8_t*>(asset.digest.data()),64,asset.bytes,&s);
        if(status==0)status=pixaura_decode_image(&independent,&s,&d);
        if(status==0)status=pixaura_working_normalize(&independent,&d,&limits,&w);
        if(status==0)status=pixaura_working_evaluate(&independent,&w,1,bytes,valid.size(),&limits,&h);
        float p[4]={};if(status==0)status=pixaura_working_copy(&independent,&h,0,p,4);
        if(status==0&&p[0]!=2)status=14;
        statuses[i]=status;pixaura_decode_context_destroy(&independent);
    });
    for(auto& worker:workers)worker.join();
    for(auto status:statuses)CHECK(status==0);
    CHECK(pixaura_working_copy(&context,&working,0,pixels,24)==0&&pixels[0]==1);
    std::array<pixaura_decode_handle,60> owned{};
    for(auto& h:owned)CHECK(pixaura_working_evaluate(&context,&working,1,bytes,valid.size(),&limits,&h)==0);
    CHECK(pixaura_working_evaluate(&context,&working,1,bytes,valid.size(),&limits,&evaluated)==8);
    CHECK(std::memcmp(&sentinel,&evaluated,sizeof(sentinel))==0);
    for(auto& h:owned)CHECK(pixaura_decode_release(&context,&h)==0);
    pixaura_decode_handle recovered{};
    CHECK(pixaura_working_evaluate(&context,&working,1,bytes,valid.size(),&limits,&recovered)==0&&recovered.serial>owned.back().serial);
    CHECK(pixaura_decode_release(&context,&recovered)==0);
    CHECK(pixaura_decode_release(&context,&evaluated)==0);CHECK(pixaura_decode_release(&context,&evaluated)==3);
    CHECK(pixaura_decode_context_destroy(&context)==0);CHECK(pixaura_decode_context_destroy(&context)==3);
    // Informational wall-clock baseline: 1 MP, 16 operations, two 16 MiB rasters.
    working::Image raster;raster.metadata={1,sizeof(pixaura_working_metadata),1024,1024,1,1,1,0,16384,16777216};
    raster.pixels=std::make_unique<decode::Vector<float>>(4194304,.25f);
    std::string ops;for(unsigned i=1;i<=16;++i){if(i>1)ops+=",";ops+=op(i%2?100:-100,i);}
    const auto benchmark=evaluation::parse(request(ops));const auto start=std::chrono::steady_clock::now();auto result=evaluation::evaluate(raster,benchmark,limits);
    const auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
    CHECK(result.pixels->size()==4194304);
    std::printf("evaluation checks=%u; properties=2048; fuzz=2048; benchmark 1024x1024 ops=16 ms=%.3f raster_bytes=33554432 scratch_bytes=0\n",checks,ms);
}
