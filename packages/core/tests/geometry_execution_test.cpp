#include "../src/geometry.hpp"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <limits>
#include <chrono>
using namespace pixaura;
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"geometry execution check %d\n",__LINE__);std::exit(1);}}while(false)
static document::EditOperation op(const char* name,document::Parameters parameters,unsigned id=1){char hex[33];std::snprintf(hex,sizeof(hex),"%032x",id);return {document::Id::parse(hex),document::String("pixaura.")+name,1,1,parameters};}
static working::Image image(uint32_t w,uint32_t h,bool unusual=false){
    working::Image out;out.metadata={1,sizeof(pixaura_working_metadata),w,h,1,1,1,0,uint64_t(w)*16,uint64_t(w)*h*16};
    out.pixels=std::make_unique<decode::Vector<float>>(static_cast<std::size_t>(w)*h*4);
    const std::array<uint32_t,8> bits={0x80000000u,0x00000001u,0x80000001u,0xbf800000u,0x40000000u,0x007fffffu,0x00800000u,0x00000000u};
    for(std::size_t i=0;i<out.pixels->size();i+=4){(*out.pixels)[i]=float(i/4+1);(*out.pixels)[i+1]=-float(i/4+1);(*out.pixels)[i+2]=0;(*out.pixels)[i+3]=1;
        if(unusual){for(unsigned c=0;c<3;++c){const auto b=bits[(i/4+c)%bits.size()];std::memcpy(out.pixels->data()+i+c,&b,4);}if(i/4%5==0){(*out.pixels)[i]=-0.f;(*out.pixels)[i+1]=0;(*out.pixels)[i+2]=-0.f;(*out.pixels)[i+3]=0;}else if(i/4%5==1)(*out.pixels)[i+3]=std::numeric_limits<float>::denorm_min();}
    }return out;
}
static working::Image reference(const working::Image& input,const evaluation::Stack& stack){
    auto current=working::identity(input,working::defaults());
    for(const auto& operation:stack){
        if(const auto* exposure=std::get_if<document::Exposure>(&operation.parameters)){if(exposure->milli_ev){const double multiplier=std::exp2(double(exposure->milli_ev)/1000);for(std::size_t i=0;i<current.pixels->size();++i)if(i%4!=3)(*current.pixels)[i]=static_cast<float>(double((*current.pixels)[i])*multiplier);}continue;}
        const auto w=current.metadata.width,h=current.metadata.height;uint32_t ow=w,oh=h,x0=0,y0=0,t=0;
        if(const auto* crop=std::get_if<document::Crop>(&operation.parameters)){x0=static_cast<uint32_t>(uint64_t(w)*crop->x_ppm/1000000);y0=static_cast<uint32_t>(uint64_t(h)*crop->y_ppm/1000000);ow=static_cast<uint32_t>((uint64_t(w)*(crop->x_ppm+crop->width_ppm)+999999)/1000000)-x0;oh=static_cast<uint32_t>((uint64_t(h)*(crop->y_ppm+crop->height_ppm)+999999)/1000000)-y0;}
        else{t=std::get<document::Rotate>(operation.parameters).quarter_turns;if(t%2){ow=h;oh=w;}}
        auto output=image(ow,oh);
        for(uint32_t y=0;y<oh;++y)for(uint32_t x=0;x<ow;++x){uint32_t sx=x+x0,sy=y+y0;
            switch(t){case 1:sx=y;sy=h-1-x;break;case 2:sx=w-1-x;sy=h-1-y;break;case 3:sx=w-1-y;sy=x;break;default:break;}
            std::memcpy(output.pixels->data()+(uint64_t(y)*ow+x)*4,current.pixels->data()+(uint64_t(sy)*w+sx)*4,16);
        }
        current=std::move(output);
    }return current;
}
static void equal(const working::Image& a,const working::Image& b){CHECK(a.metadata.width==b.metadata.width&&a.metadata.height==b.metadata.height);CHECK(a.metadata.row_stride==b.metadata.row_stride&&a.metadata.image_bytes==b.metadata.image_bytes);CHECK(a.pixels->size()==b.pixels->size());CHECK(std::memcmp(a.pixels->data(),b.pixels->data(),a.pixels->size()*4)==0);}
static int32_t failure(const working::Image& in,const evaluation::Stack& stack,const pixaura_working_limits& l){try{evaluation::evaluate(in,stack,l);return 0;}catch(const document::Failure& e){return e.code;}}
struct CancelAt{evaluation::Cancellation cancel;unsigned count=0,target=UINT32_MAX;};
static void inject(evaluation::Checkpoint,void* state){auto& s=*static_cast<CancelAt*>(state);if(s.count++==s.target)s.cancel.requested.store(true);}
int main(){
    const auto limits=working::defaults();const document::Crop full{0,0,1000000,1000000},edge{999999,999999,1,1},center{333334,333334,1,1},left{0,0,500000,1000000};
    // Reviewable labels 1 2 / 3 4 / 5 6: 90=5 3 1 / 6 4 2.
    auto non_square=image(2,3);auto ninety=evaluation::evaluate(non_square,{op("rotate",document::Rotate{1})},limits);
    const std::array<float,6> golden90={5,3,1,6,4,2},golden180={6,5,4,3,2,1},golden270={2,4,6,1,3,5};
    for(unsigned t=1;t<=3;++t){auto rotated=evaluation::evaluate(non_square,{op("rotate",document::Rotate{t})},limits);const auto& golden=t==1?golden90:t==2?golden180:golden270;for(std::size_t i=0;i<6;++i)CHECK((*rotated.pixels)[i*4]==golden[i]);}
    for(const auto size:{geometry::Extent{1,1},{2,2},{2,3},{3,2},{3,3},{129,130},{1,257},{257,1}}){auto in=image(size.width,size.height,true);const auto original=*in.pixels;
        for(unsigned t=0;t<=3;++t){evaluation::Stack stack{op("rotate",document::Rotate{t})};auto out=evaluation::evaluate(in,stack,limits);equal(out,reference(in,stack));const auto plan=geometry::plan(size,stack,limits);CHECK(out.metadata.width==plan.summary.width&&out.metadata.height==plan.summary.height);}
        for(const auto crop:{full,edge,center,left}){evaluation::Stack stack{op("crop",crop)};equal(evaluation::evaluate(in,stack,limits),reference(in,stack));}
        equal(evaluation::evaluate(in,{},limits),in);equal(evaluation::evaluate(in,{op("crop",full)},limits),in);
        for(const auto turns:std::array<std::array<unsigned,4>,3>{{{{1,1,1,1}},{{2,2,0,0}},{{1,3,0,0}}}}){evaluation::Stack stack;for(unsigned i=0;i<4;++i)stack.push_back(op("rotate",document::Rotate{turns[i]},i+1));equal(evaluation::evaluate(in,stack,limits),in);}
        CHECK(*in.pixels==original);
    }
    auto square=image(3,3);auto c=evaluation::evaluate(square,{op("crop",center)},limits);CHECK(c.metadata.width==1&&c.metadata.height==1&&(*c.pixels)[0]==5);
    const auto crop=op("crop",left,1),rotate=op("rotate",document::Rotate{1},2),exposure=op("exposure",document::Exposure{1000},3);
    for(const evaluation::Stack& mixed: {evaluation::Stack{exposure,crop},{crop,exposure},{rotate,exposure},{exposure,rotate},{crop,rotate},{rotate,crop},{crop,exposure,rotate},{rotate,crop,exposure}})equal(evaluation::evaluate(non_square,mixed,limits),reference(non_square,mixed));
    const auto cr=evaluation::evaluate(non_square,{crop,rotate},limits),rc=evaluation::evaluate(non_square,{rotate,crop},limits);CHECK(cr.metadata.width!=rc.metadata.width||cr.metadata.height!=rc.metadata.height);
    auto overflow=image(2,1);(*overflow.pixels)[0]=std::numeric_limits<float>::max();const auto right=op("crop",document::Crop{500000,0,500000,1000000},4);
    CHECK(failure(overflow,{exposure,right},limits)==7);CHECK(failure(overflow,{right,exposure},limits)==0);
    auto narrow=limits;narrow.width=2;CHECK(failure(non_square,{rotate,crop},narrow)==8);
    CHECK(failure(non_square,{op("rotate",document::Rotate{4})},limits)==7);CHECK(failure(non_square,{op("crop",document::Crop{999999,0,2,1})},limits)==7);
    CHECK(evaluation::reservation(2,3,{rotate},limits)==97);
    CHECK(evaluation::reservation(2,3,{crop},limits)==96);
    auto bad_layout=image(2,3);bad_layout.pixels->pop_back();CHECK(failure(bad_layout,{rotate},limits)==19);
    // Bounded deterministic mixed-stack property corpus, independent allocating oracle.
    uint32_t seed=0x508a123u;auto next=[&]{seed=seed*1664525u+1013904223u;return seed;};
    for(unsigned n=0;n<2048;++n){auto in=image(1+next()%19,1+next()%19,true);evaluation::Stack stack;const auto count=1+next()%8;
        for(unsigned i=0;i<count;++i){const auto kind=next()%3;if(kind==0)stack.push_back(op("rotate",document::Rotate{next()%4},i+1));else if(kind==1){const auto x=next()%1000000,y=next()%1000000;stack.push_back(op("crop",document::Crop{x,y,1+next()%(1000000-x),1+next()%(1000000-y)},i+1));}else stack.push_back(op("exposure",document::Exposure{int32_t(next()%3)*1000-1000},i+1));}
        auto out=evaluation::evaluate(in,stack,limits);equal(out,reference(in,stack));const auto plan=geometry::plan({in.metadata.width,in.metadata.height},stack,limits);CHECK(out.metadata.width==plan.summary.width&&out.metadata.height==plan.summary.height);
    }
    evaluation::Stack maximum;for(unsigned i=0;i<256;++i)maximum.push_back(op(i%2?"crop":"rotate",i%2?document::Parameters(full):document::Parameters(document::Rotate{1}),i+1));equal(evaluation::evaluate(non_square,maximum,limits),non_square);maximum.push_back(op("rotate",document::Rotate{0},257));CHECK(failure(non_square,maximum,limits)==8);
    auto checkpoint_image=image(129,130,true);const evaluation::Stack checkpoint_stack{rotate,crop,exposure};CancelAt baseline;auto out=evaluation::evaluate(checkpoint_image,checkpoint_stack,limits,&baseline.cancel,inject,&baseline);
    const auto original=*checkpoint_image.pixels;for(unsigned target=0;target<baseline.count;++target){CancelAt state;state.target=target;bool cancelled=false;try{evaluation::evaluate(checkpoint_image,checkpoint_stack,limits,&state.cancel,inject,&state);}catch(const document::Failure& e){cancelled=e.code==13;}CHECK(cancelled);CHECK(*checkpoint_image.pixels==original);}equal(evaluation::evaluate(checkpoint_image,checkpoint_stack,limits),out);
    auto large=image(1024,768);for(const evaluation::Stack& diagnostic:{evaluation::Stack{op("crop",full)}, {rotate}, {crop,rotate}, {exposure,crop,rotate}}){uint64_t work_checkpoints=0;const auto observe=[](evaluation::Checkpoint p,void* state){if(p==evaluation::Checkpoint::tile)++*static_cast<uint64_t*>(state);};const auto begin=std::chrono::steady_clock::now();auto result=evaluation::evaluate(large,diagnostic,limits,nullptr,observe,&work_checkpoints);const auto end=std::chrono::steady_clock::now();const auto plan=geometry::plan({1024,768},diagnostic,limits);
        uint64_t tile_visits=geometry::tile_count({1024,768});for(std::size_t i=0;i<diagnostic.size();++i){if(diagnostic[i].type=="pixaura.rotate")tile_visits+=geometry::tile_count(plan.stages[i].output);else if(diagnostic[i].type=="pixaura.exposure")tile_visits+=geometry::tile_count(plan.stages[i].input);}
        std::printf("geometry diagnostic source=1024x768 output=%ux%u ops=%zu output_tiles=%u root_tiles_visited=%llu work_checkpoints=%llu ms=%.3f rasters=%llu scratch=%llu\n",result.metadata.width,result.metadata.height,diagnostic.size(),plan.summary.tile_count,static_cast<unsigned long long>(tile_visits),static_cast<unsigned long long>(work_checkpoints),std::chrono::duration<double,std::milli>(end-begin).count(),static_cast<unsigned long long>(large.metadata.image_bytes+plan.max_raster_bytes),static_cast<unsigned long long>(plan.scratch_bytes));}
    std::printf("geometry pixel checks=%u mixed_properties=2048 checkpoints=%u bit_exact=PASS\n",checks,baseline.count);
}
