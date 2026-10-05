#include "../src/geometry.hpp"
#include "../src/storage.hpp"
#include "../../../tests/fixtures/decode/fixtures.h"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <filesystem>
using namespace pixaura;
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"geometry check %d\n",__LINE__);std::exit(1);}}while(false)
template<class F> static int32_t failure(F&& f){try{f();return 0;}catch(const document::Failure& e){return e.code;}}
static std::string operation(const char* type,const char* parameters,unsigned id=1){
    char hex[33];std::snprintf(hex,sizeof(hex),"%032x",id);
    return std::string("{\"id\":\"")+hex+"\",\"type\":\"pixaura."+type+"\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":"+parameters+"}";
}
static evaluation::Stack stack(const std::string& ops){auto parsed=document::parse_evaluation("{\"operations\":["+ops+"]}");CHECK(parsed.code==0);return std::move(parsed.value);}
struct CheckState {evaluation::Cancellation cancel;unsigned at=0,target=UINT32_MAX;};
static void inject(evaluation::Checkpoint,void* ptr){auto& s=*static_cast<CheckState*>(ptr);if(s.at++==s.target)s.cancel.requested.store(true);}
struct Barrier {std::mutex mutex;std::condition_variable cv;bool entered=false,ready=false;};
static void pause(evaluation::Checkpoint p,void* ptr){if(p!=evaluation::Checkpoint::tile)return;auto& b=*static_cast<Barrier*>(ptr);std::unique_lock<std::mutex> lock(b.mutex);b.entered=true;b.cv.notify_all();b.cv.wait(lock,[&]{return b.ready;});}
struct Reader:storage::Reader {std::size_t at=0;std::size_t read(uint8_t* p,std::size_t n)override{const auto count=std::min(n,sizeof(decode_png)-at);std::memcpy(p,decode_png+at,count);at+=count;return count;}};
int main(int argc,char** argv){
    CHECK(argc==2);
    const auto l=working::defaults();
    // Human-readable 2x3 clockwise 90 raster: e c a / f d b.
    const std::array<geometry::Point,6> golden={geometry::Point{2,0},{2,1},{1,0},{1,1},{0,0},{0,1}};
    for(uint32_t y=0;y<3;++y)for(uint32_t x=0;x<2;++x){const auto p=geometry::forward({2,3},1,{x,y});CHECK(p.x==golden[y*2+x].x&&p.y==golden[y*2+x].y);}
    const std::array<geometry::Point,6> golden180={geometry::Point{1,2},{0,2},{1,1},{0,1},{1,0},{0,0}};
    const std::array<geometry::Point,6> golden270={geometry::Point{0,1},{0,0},{1,1},{1,0},{2,1},{2,0}};
    for(uint32_t y=0;y<3;++y)for(uint32_t x=0;x<2;++x){const auto a=geometry::forward({2,3},2,{x,y}),b=geometry::forward({2,3},3,{x,y});CHECK(a.x==golden180[y*2+x].x&&a.y==golden180[y*2+x].y);CHECK(b.x==golden270[y*2+x].x&&b.y==golden270[y*2+x].y);}
    const auto full=geometry::crop({3,2},{0,0,1000000,1000000});CHECK(full.x0==0&&full.x1==3&&full.y1==2);
    const auto edge=geometry::crop({3,2},{999999,999999,1,1});CHECK(edge.x0==2&&edge.x1==3&&edge.y0==1&&edge.y1==2);
    const auto rational=geometry::crop({3,2},{250000,0,500000,1000000});CHECK(rational.x0==0&&rational.x1==3);
    CHECK(geometry::crop({1,1},{999999,0,1,1}).x1==1);
    const auto max=geometry::crop({UINT32_MAX,UINT32_MAX},{999999,999999,1,1});CHECK(max.x1==UINT32_MAX&&max.x0<max.x1);
    for(const auto c:{document::Crop{0,0,0,1},document::Crop{1000000,0,1,1},document::Crop{999999,0,2,1},document::Crop{0,0,UINT32_MAX,1}})CHECK(failure([&]{geometry::crop({2,3},c);})==7);
    CHECK(failure([&]{geometry::crop({0,3},{0,0,1,1});})==7);
    CHECK(failure([&]{geometry::rotated({1,1},4);})==7);
    CHECK(failure([&]{geometry::forward({2,3},1,{2,0});})==7);
    CHECK(failure([&]{geometry::tile_count({UINT32_MAX,UINT32_MAX});})==8);
    CHECK(failure([&]{geometry::tile({128,128},1);})==7);
    CHECK(failure([&]{geometry::tile_count({0,1});})==7);
    CHECK(geometry::pixel_offset({2,3},{1,2})==20);
    CHECK(failure([&]{geometry::pixel_offset({UINT32_MAX,UINT32_MAX},{UINT32_MAX-1,UINT32_MAX-1});})==8);
    uint32_t seed=0x17a91u;
    auto next=[&]{seed=seed*1664525u+1013904223u;return seed;};
    for(unsigned n=0;n<2048;++n){
        const geometry::Extent e{1+next()%300,1+next()%300};
        const uint32_t x=next()%1000000,y=next()%1000000;
        const auto r=geometry::crop(e,{x,y,1+next()%(1000000-x),1+next()%(1000000-y)});
        CHECK(r.x0<r.x1&&r.y0<r.y1&&r.x1<=e.width&&r.y1<=e.height);
        const geometry::Point p{next()%e.width,next()%e.height};
        for(uint32_t t=0;t<4;++t){const auto q=geometry::forward(e,t,p),back=geometry::inverse(e,t,q);CHECK(back.x==p.x&&back.y==p.y);const auto d=geometry::rotated(e,t);CHECK(d.width==(t%2?e.height:e.width));}
        auto d=e;auto point=p;for(unsigned t=0;t<4;++t){point=geometry::forward(d,1,point);d=geometry::rotated(d,1);}CHECK(d.width==e.width&&d.height==e.height&&point.x==p.x&&point.y==p.y);
        std::vector<uint8_t> coverage(static_cast<std::size_t>(e.width)*e.height);uint64_t area=0;
        for(uint32_t i=0;i<geometry::tile_count(e);++i){const auto tile=geometry::tile(e,i);CHECK(tile.x0<tile.x1&&tile.y0<tile.y1&&tile.x1<=e.width&&tile.y1<=e.height);area+=uint64_t(tile.x1-tile.x0)*(tile.y1-tile.y0);
            for(uint32_t ty=tile.y0;ty<tile.y1;++ty)for(uint32_t tx=tile.x0;tx<tile.x1;++tx)++coverage[static_cast<std::size_t>(ty)*e.width+tx];}
        CHECK(area==uint64_t(e.width)*e.height);for(auto value:coverage)if(value!=1){CHECK(false);}
    }
    auto crop=operation("crop","{\"x_ppm\":0,\"y_ppm\":0,\"width_ppm\":500000,\"height_ppm\":1000000}");
    auto rotate=operation("rotate","{\"quarter_turns\":1}",2);
    auto exposure=operation("exposure","{\"milli_ev\":1000}",3);
    const auto planned=geometry::plan({2,3},stack(exposure+","+crop+","+rotate),l);
    CHECK(planned.summary.width==3&&planned.summary.height==1&&planned.summary.executable&&planned.stages.size()==3);
    CHECK(planned.stages[0].input.width==2&&planned.stages[1].output.width==1&&planned.stages[2].input.height==3);
    const auto reverse=geometry::plan({2,3},stack(rotate+","+crop),l);CHECK(reverse.summary.width==2&&reverse.summary.height==2);
    std::string maximum;
    for(unsigned i=1;i<=256;++i){if(i>1)maximum+=",";maximum+=operation("exposure","{\"milli_ev\":1}",i);}
    CHECK(geometry::plan({4096,2048},stack(maximum),l).summary.pixel_visits==2147483648ULL);
    auto too_many=document::parse_evaluation("{\"operations\":["+maximum+","+operation("rotate","{\"quarter_turns\":1}",257)+"]}");CHECK(too_many.code==8);
    CHECK(document::parse_evaluation("{\"operations\":["+operation("rotate","{\"quarter_turns\":4}")+"]}").code==7);
    CHECK(failure([&]{geometry::plan({16384,16384},{},l);})==8);
    CHECK(geometry::tile_count({1,16384})==128&&geometry::tile_count({16384,512})==512);
    pixaura_geometry_plan out{},sentinel;std::memset(&out,0x5a,sizeof(out));sentinel=out;
    const std::string request="{\"operations\":["+crop+"]}";
    CHECK(pixaura_geometry_preflight(1,2,3,reinterpret_cast<const uint8_t*>(request.data()),request.size(),&l,&out)==0&&out.width==1);
    out=sentinel;CHECK(pixaura_geometry_preflight(1,UINT32_MAX,1,reinterpret_cast<const uint8_t*>(request.data()),request.size(),&l,&out)==8&&std::memcmp(&out,&sentinel,sizeof(out))==0);
    for(unsigned n=0;n<2048;++n){auto mutated=request;const auto at=next()%mutated.size();mutated[at]=static_cast<char>(next()%128);out=sentinel;const auto status=pixaura_geometry_preflight(1,2,3,reinterpret_cast<const uint8_t*>(mutated.data()),mutated.size(),&l,&out);CHECK(status>=0&&status<=19);if(status)CHECK(std::memcmp(&out,&sentinel,sizeof(out))==0);}
    // All checkpoints deterministically cancelled; private candidate never returned.
    working::Image input;input.metadata={1,sizeof(pixaura_working_metadata),129,129,1,1,1,0,129*16,129*129*16};
    input.pixels=std::make_unique<decode::Vector<float>>(129*129*4,0.25f);const auto original=*input.pixels;
    const auto edits=stack(exposure);CheckState baseline;
    auto reference=evaluation::evaluate(input,edits,l,&baseline.cancel,inject,&baseline);CHECK(baseline.at==12);
    for(unsigned target=0;target<baseline.at;++target){CheckState s;s.target=target;CHECK(failure([&]{evaluation::evaluate(input,edits,l,&s.cancel,inject,&s);})==13);CHECK(*input.pixels==original);}
    auto recovered=evaluation::evaluate(input,edits,l);CHECK(*recovered.pixels==*reference.pixels);
    Barrier barrier;evaluation::Cancellation cancel;int32_t result=-1;
    std::thread worker([&]{result=failure([&]{evaluation::evaluate(input,edits,l,&cancel,pause,&barrier);});});
    {std::unique_lock<std::mutex> lock(barrier.mutex);barrier.cv.wait(lock,[&]{return barrier.entered;});cancel.requested.store(true);barrier.ready=true;barrier.cv.notify_all();}worker.join();CHECK(result==13);
    // Context scoped lifecycle, kinds, sticky signalling, exhaustion, recreation.
    pixaura_decode_limits dl{};CHECK(pixaura_decode_default_limits(1,&dl)==0);pixaura_decode_context context{},other{};
    const uint8_t id[]="11111111111111111111111111111111",id2[]="22222222222222222222222222222222";
    CHECK(pixaura_decode_context_init(1,&context,sizeof(context),id,32,&dl)==0);CHECK(pixaura_decode_context_init(1,&other,sizeof(other),id2,32,&dl)==0);
    pixaura_decode_handle token{},old{},dummy{},unchanged;std::memset(&dummy,0x5a,sizeof(dummy));unchanged=dummy;
    for(unsigned i=0;i<2048;++i){CHECK(pixaura_cancel_create(&context,1,&token)==0);CHECK(pixaura_cancel_signal(&other,&token)==3);CHECK(pixaura_decode_release(&context,&token)==3);CHECK(pixaura_cancel_signal(&context,&token)==0);CHECK(pixaura_cancel_signal(&context,&token)==0);
        CHECK(pixaura_working_evaluate_cancel(&context,&dummy,1,reinterpret_cast<const uint8_t*>(request.data()),request.size(),&l,&token,&dummy)==13&&std::memcmp(&dummy,&unchanged,sizeof(dummy))==0);
        CHECK(pixaura_cancel_release(&context,&token)==0);CHECK(pixaura_cancel_release(&context,&token)==3);CHECK(pixaura_cancel_signal(&context,&token)==3);old=token;}
    std::array<pixaura_decode_handle,64> tokens{};for(auto& t:tokens)CHECK(pixaura_cancel_create(&context,1,&t)==0);
    CHECK(pixaura_cancel_create(&context,1,&dummy)==8&&std::memcmp(&dummy,&unchanged,sizeof(dummy))==0);CHECK(pixaura_cancel_signal(&context,&old)==3);
    for(auto& t:tokens)CHECK(pixaura_cancel_release(&context,&t)==0);
    // Real C ABI: simultaneous cancellation/evaluation and read-only source access.
    const auto root=std::filesystem::absolute(std::filesystem::path(argv[1])/"geometry-root");std::filesystem::create_directories(root);
    const auto path=root.generic_string();storage::AssetStore assets(path);Reader reader;const auto asset=assets.ingest(reader,sizeof(decode_png));
    pixaura_decode_handle source{},decoded{},working{};
    CHECK(pixaura_decode_open(&context,reinterpret_cast<const uint8_t*>(path.data()),path.size(),reinterpret_cast<const uint8_t*>(asset.digest.data()),64,asset.bytes,&source)==0);
    CHECK(pixaura_decode_image(&context,&source,&decoded)==0);CHECK(pixaura_working_normalize(&context,&decoded,&l,&working)==0);
    // Admit copy+parser exactly, but reject rotation's additional bitmap byte.
    pixaura_decode_context budget_context{};auto budget_limits=dl;
    budget_limits.profile_bytes=1;budget_limits.scratch_bytes=131072;
    budget_limits.context_bytes=sizeof(decode_png)+24+96+96+1048576;
    const uint8_t budget_id[]="33333333333333333333333333333333";
    CHECK(pixaura_decode_context_init(1,&budget_context,sizeof(budget_context),budget_id,32,&budget_limits)==0);
    pixaura_decode_handle bs{},bd{},bw{},bo{};
    CHECK(pixaura_decode_open(&budget_context,reinterpret_cast<const uint8_t*>(path.data()),path.size(),reinterpret_cast<const uint8_t*>(asset.digest.data()),64,asset.bytes,&bs)==0);
    CHECK(pixaura_decode_image(&budget_context,&bs,&bd)==0);CHECK(pixaura_working_normalize(&budget_context,&bd,&l,&bw)==0);
    const std::string rotation_request="{\"operations\":["+rotate+"]}";
    CHECK(pixaura_working_evaluate(&budget_context,&bw,1,reinterpret_cast<const uint8_t*>(rotation_request.data()),rotation_request.size(),&l,&bo)==8&&bo.serial==0);
    CHECK(pixaura_working_evaluate(&budget_context,&bw,1,reinterpret_cast<const uint8_t*>(request.data()),request.size(),&l,&bo)==0);
    CHECK(pixaura_decode_context_destroy(&budget_context)==0);
    const std::string long_request="{\"operations\":["+maximum+"]}";
    for(unsigned i=0;i<64;++i){CHECK(pixaura_cancel_create(&context,1,&token)==0);pixaura_decode_handle output=unchanged;std::atomic<bool> started{false};int32_t status=-1,read_status=-1;float pixel=-1;
        std::thread evaluate([&]{started.store(true);status=pixaura_working_evaluate_cancel(&context,&working,1,reinterpret_cast<const uint8_t*>(long_request.data()),long_request.size(),&l,&token,&output);});
        std::thread read([&]{read_status=pixaura_working_copy(&context,&working,0,&pixel,1);});
        while(!started.load()){std::this_thread::yield();}CHECK(pixaura_cancel_signal(&context,&token)==0);
        evaluate.join();read.join();CHECK(read_status==0&&pixel==1);
        CHECK(status==0||status==13);if(status==13)CHECK(std::memcmp(&output,&unchanged,sizeof(output))==0);else CHECK(pixaura_decode_release(&context,&output)==0);
        CHECK(pixaura_cancel_release(&context,&token)==0);
    }
    CHECK(pixaura_cancel_signal(&context,&working)==3);CHECK(pixaura_decode_release(&context,&working)==0);CHECK(pixaura_decode_release(&context,&decoded)==0);CHECK(pixaura_decode_release(&context,&source)==0);
    CHECK(pixaura_decode_context_destroy(&context)==0);CHECK(pixaura_cancel_signal(&context,&old)==3);CHECK(pixaura_decode_context_destroy(&context)==3);CHECK(pixaura_decode_context_destroy(&other)==0);
    auto start=std::chrono::steady_clock::now();auto diagnostic=geometry::plan({4096,2048},edits,l);auto planned_at=std::chrono::steady_clock::now();uint64_t area=0;
    for(uint32_t i=0;i<diagnostic.summary.tile_count;++i){const auto r=geometry::tile({4096,2048},i);area+=uint64_t(r.x1-r.x0)*(r.y1-r.y0);}auto end=std::chrono::steady_clock::now();CHECK(area==8388608);
    std::printf("geometry checks=%u; properties=2048; parser mutations=2048; lifecycle=2048; checkpoints=%u\n",checks,baseline.at);
    std::printf("plan 4096x2048 tile=128 count=%u metadata=%llu plan_us=%.3f traversal_us=%.3f\n",diagnostic.summary.tile_count,static_cast<unsigned long long>(diagnostic.summary.metadata_bytes),std::chrono::duration<double,std::micro>(planned_at-start).count(),std::chrono::duration<double,std::micro>(end-planned_at).count());
}
