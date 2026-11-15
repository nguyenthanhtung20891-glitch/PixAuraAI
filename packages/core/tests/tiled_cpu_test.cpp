#include "../src/tiled_cpu.hpp"
#include "../src/tone.hpp"
#include <thread>
#include <condition_variable>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
using namespace pixaura;
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"tile check %d\n",__LINE__);std::exit(1);}}while(false)
static void sample(uint32_t x,uint32_t y,float* p){
    if(x==0&&y==0){p[0]=-0.0f;p[1]=p[2]=p[3]=0;return;}
    if(x==1&&y==0){p[0]=1e-30f;p[1]=-1e-30f;p[2]=0;p[3]=1e-30f;return;}
    const float a=float((x+3*y)%5)/4;p[0]=a*float(int(x%17)-8)/8;
    p[1]=a*float(int(y%13)-6)/4;p[2]=a*float((x+y)%19)/8;p[3]=a;
}
struct Output:tiled::Sink {
    uint32_t width;decode::Vector<float> data;bool done=false,aborted=false,fail=false;
    explicit Output(uint32_t w,uint32_t h):width(w),data(std::size_t(w)*h*4){}
    void write(uint32_t x,uint32_t y,uint32_t w,uint32_t h,const float* p,uint32_t stride)override{
        if(fail)throw document::Failure{11};
        for(uint32_t row=0;row<h;++row)std::memcpy(data.data()+(std::size_t(y+row)*width+x)*4,p+std::size_t(row)*stride*4,w*16);
    }
    void finish()override{done=true;}void abort()noexcept override{aborted=true;}
};
static evaluation::Stack operations(const std::string& body){return evaluation::parse("{\"operations\":["+body+"]}");}
static std::string op(const char* type,const char* parameter,int value,int id){char b[512];std::snprintf(b,sizeof b,"{\"id\":\"%032x\",\"type\":\"pixaura.%s\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"%s\":%d}}",id,type,parameter,value);return b;}
int main(){
    const auto exposure=op("exposure","milli_ev",1000,1);
    std::vector<std::string> cases={"",exposure,op("blur","milli_strength",1000,1),op("sharpen","milli_amount",750,1),op("blur","milli_strength",500,1)+","+op("exposure","milli_ev",-300,2)+","+op("sharpen","milli_amount",200,3)};
    for(const auto& spec:tone::specs)for(auto value:{spec.low,spec.neutral,spec.high})cases.push_back(op(std::string(spec.type.substr(8)).c_str(),std::string(spec.parameter).c_str(),value,1));
    unsigned parity=0;
    tiled::Budget budget(134217728,0),small(128,0);
    for(auto extent:std::array<std::array<uint32_t,2>,7>{{{1,1},{1,259},{257,1},{127,129},{128,128},{129,127},{257,259}}})for(const auto& c:cases){
        const auto w=extent[0],h=extent[1];working::Image image;
        image.metadata={1,sizeof(pixaura_working_metadata),w,h,1,1,1,0,uint64_t(w)*16,uint64_t(w)*h*16};
        image.pixels=std::make_unique<decode::Vector<float>>(std::size_t(w)*h*4);
        for(uint32_t y=0;y<h;++y)for(uint32_t x=0;x<w;++x)sample(x,y,image.pixels->data()+(std::size_t(y)*w+x)*4);
        auto stack=operations(c);auto expected=evaluation::evaluate(image,stack,working::defaults());Output actual(w,h);
        const auto m=tiled::evaluate(w,h,stack,budget,sample,actual);
        CHECK(actual.done&&!actual.aborted&&m.peak_reserved<1048576);
        CHECK(std::memcmp(actual.data.data(),expected.pixels->data(),actual.data.size()*4)==0);++parity;
    }
    auto stack=operations(exposure);unsigned reads=0;Output denied(1,1);
    try{tiled::evaluate(1,1,stack,small,[&](uint32_t,uint32_t,float*){++reads;},denied);CHECK(false);}catch(const document::Failure& e){CHECK(e.code==8&&reads==0&&denied.aborted);}
    evaluation::Cancellation cancel;cancel.requested=true;Output stopped(1,1);
    try{tiled::evaluate(1,1,stack,budget,sample,stopped,&cancel);CHECK(false);}catch(const document::Failure& e){CHECK(e.code==13&&!stopped.done&&stopped.aborted);}
    std::atomic<uint64_t> invalid_generation{1};evaluation::Cancellation invalid_binding;invalid_binding.generation=&invalid_generation;
    try{tiled::evaluate(1,1,stack,budget,sample,stopped,&invalid_binding);CHECK(false);}catch(const document::Failure& e){CHECK(e.code==1&&budget.inflight()==0);}
    std::atomic<uint64_t> generation{2};std::atomic<bool> revoked{false};cancel.requested=false;cancel.generation=&generation;cancel.revoked=&revoked;cancel.expected=1;
    try{tiled::evaluate(1,1,stack,budget,sample,stopped,&cancel);CHECK(false);}catch(const document::Failure& e){CHECK(e.code==13);}
    Output failed(1,1);failed.fail=true;
    try{tiled::evaluate(1,1,stack,budget,sample,failed);CHECK(false);}catch(const document::Failure& e){CHECK(e.code==11&&failed.aborted&&!failed.done);}
    for(int mode=0;mode<3;++mode){Output bad(1,1);try{tiled::evaluate(1,1,stack,budget,[&](uint32_t,uint32_t,float* p){if(mode==0)throw document::Failure{11};sample(0,0,p);p[0]=mode==1?std::numeric_limits<float>::infinity():1;},bad);CHECK(false);}catch(const document::Failure&){CHECK(bad.aborted&&!bad.done);}}
    Output allocation_failed(1,1);try{tiled::evaluate(1,1,stack,budget,[](uint32_t,uint32_t,float*){throw std::bad_alloc();},allocation_failed);CHECK(false);}catch(const std::bad_alloc&){CHECK(allocation_failed.aborted&&!allocation_failed.done&&budget.inflight()==0);}
    struct LateCancel:Output{evaluation::Cancellation& cancel;explicit LateCancel(evaluation::Cancellation& c):Output(1,1),cancel(c){}void write(uint32_t x,uint32_t y,uint32_t w,uint32_t h,const float* p,uint32_t stride)override{Output::write(x,y,w,h,p,stride);cancel.requested=true;}};
    evaluation::Cancellation late;LateCancel cancelled_at_delivery(late);
    try{tiled::evaluate(1,1,stack,budget,sample,cancelled_at_delivery,&late);CHECK(false);}catch(const document::Failure& e){CHECK(e.code==13&&cancelled_at_delivery.aborted&&!cancelled_at_delivery.done&&budget.inflight()==0);}
    Output geometry(1,1);try{tiled::evaluate(1,1,operations(op("rotate","quarter_turns",1,1)),budget,sample,geometry);CHECK(false);}catch(const document::Failure& e){CHECK(e.code==5&&geometry.aborted);}
    struct Large:tiled::Sink{uint64_t count=0;bool done=false;void write(uint32_t x,uint32_t y,uint32_t w,uint32_t h,const float* p,uint32_t stride)override{for(uint32_t r=0;r<h;++r)for(uint32_t c=0;c<w;++c){float v[4];sample(x+c,y+r,v);for(unsigned k=0;k<4;++k)CHECK(p[(std::size_t(r)*stride+c)*4+k]==v[k]*(k==3?1:2));++count;}}void finish()override{done=true;}void abort()noexcept override{done=false;}}large;
    const auto m=tiled::evaluate(4000,3000,stack,budget,sample,large);CHECK(large.done&&large.count==12000000&&m.peak_reserved<1048576);
    CHECK(budget.inflight()==0);
    Output impulse(5,5);auto impulse_source=[](uint32_t x,uint32_t y,float* p){p[0]=p[1]=p[2]=(x==2&&y==2)?1.0f:0.0f;p[3]=1;};
    tiled::evaluate(5,5,operations(op("blur","milli_strength",1000,1)),budget,impulse_source,impulse);
    const int weights[3]={1,2,1};for(int y=0;y<5;++y)for(int x=0;x<5;++x){const float expected=x>=1&&x<=3&&y>=1&&y<=3?float(weights[x-1]*weights[y-1])/16:0;for(unsigned c=0;c<3;++c)CHECK(impulse.data[(y*5+x)*4+c]==expected);CHECK(impulse.data[(y*5+x)*4+3]==1);}
    tiled::Budget shared(700000,1000);std::mutex mutex;std::condition_variable cv;bool entered=false,release=false;
    Output first(1,1);std::thread worker([&]{tiled::evaluate(1,1,stack,shared,[&](uint32_t x,uint32_t y,float* p){std::unique_lock<std::mutex> lock(mutex);entered=true;cv.notify_all();cv.wait(lock,[&]{return release;});sample(x,y,p);},first);});
    {std::unique_lock<std::mutex> lock(mutex);cv.wait(lock,[&]{return entered;});}
    Output concurrent(1,1);try{tiled::evaluate(1,1,stack,shared,sample,concurrent);CHECK(false);}catch(const document::Failure& e){CHECK(e.code==8&&concurrent.aborted);}
    {std::lock_guard<std::mutex> lock(mutex);release=true;}cv.notify_all();worker.join();CHECK(shared.inflight()==0&&shared.peak()<=700000);
    std::printf("tile parity %u; normalized12MP pixels %llu; peak reserved %llu (not physical RSS/import/export)\n",parity,static_cast<unsigned long long>(large.count),static_cast<unsigned long long>(m.peak_reserved));
    return 0;
}
