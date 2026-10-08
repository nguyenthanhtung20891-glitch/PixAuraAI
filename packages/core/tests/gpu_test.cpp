#include "pixaura/gpu.h"
#include "pixaura/preview.h"
#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"GPU check failed line %d: %s\n",__LINE__,#x);return 1;}}while(false)
namespace {
unsigned checks=0;
struct State{int32_t status=0;bool corrupt=false;int calls=0;int32_t current=1;bool cancel=false;};
int32_t current(void* p){return static_cast<State*>(p)->current;}
int32_t dispatch(void* p,const float* in,float* out,uint32_t n,float gain,uint32_t identity){auto& s=*static_cast<State*>(p);++s.calls;
    if(s.cancel)s.current=0;
    if(s.status)return s.status;
    for(uint32_t i=0;i<n*4;++i){if(i%4==3||identity)std::memcpy(out+i,in+i,4);else out[i]=in[i]*gain;}
    if(s.corrupt)out[0]=std::numeric_limits<float>::infinity();
    return 0;
}
struct PreviewOwner{pixaura_decode_context context{};pixaura_preview_ticket ticket{};};
int32_t preview_current(void* p){auto& s=*static_cast<PreviewOwner*>(p);return pixaura_preview_current(&s.context,&s.ticket,nullptr)==0?1:0;}
int32_t supersede(void* p,const float*,float*,uint32_t,float,uint32_t){auto& s=*static_cast<PreviewOwner*>(p);pixaura_preview_ticket newer{};return pixaura_preview_begin(&s.context,1,&newer);}
}
int main(){
    for(const char* name:{"SwiftShader Device (LLVM)","llvmpipe","lavapipe","Software GPU","Virtual GPU","Android Emulator","Venus"})CHECK(pixaura_gpu_hardware(1,0x5143,name,static_cast<uint32_t>(std::strlen(name)))==5);
    CHECK(pixaura_gpu_hardware(4,0x5143,"Adreno",6)==5);CHECK(pixaura_gpu_hardware(3,0x5143,"Adreno",6)==5);
    CHECK(pixaura_gpu_hardware(1,0x1ae0,"GPU",3)==5);CHECK(pixaura_gpu_hardware(1,0,"GPU",3)==5);
    CHECK(pixaura_gpu_hardware(1,0x5143,"Adreno",6)==0);
    const pixaura_vulkan_capabilities capable{1u<<22,1,2,64,64,256,262144,12,2};
    CHECK(pixaura_gpu_vulkan_capable(1,&capable)==0);
    for(unsigned field=0;field<9;++field){auto c=capable;
        switch(field){case 0:c.api_version=0;break;case 1:c.queue_count=0;break;case 2:c.queue_flags=1;break;
        case 3:c.invocations=63;break;case 4:c.group_size_x=63;break;case 5:c.group_count_x=255;break;
        case 6:c.storage_range=262143;break;case 7:c.push_bytes=11;break;default:c.storage_descriptors=1;break;}
        CHECK(pixaura_gpu_vulkan_capable(1,&c)==5);
    }
    std::array<float,PIXAURA_GPU_CASE_PIXELS*4> in{},out{};State state;
    unsigned executed=0;
    for(uint32_t test=0;test<pixaura_gpu_case_count();++test){int32_t ev=0;CHECK(pixaura_gpu_case(test,in.data(),PIXAURA_GPU_CASE_PIXELS,&ev)==0);const auto source=in;
        pixaura_gpu_result r{};CHECK(pixaura_gpu_tile(1,in.data(),PIXAURA_GPU_CASE_PIXELS,ev,dispatch,&state,current,&state,0,out.data(),&r)==0);
        CHECK(r.used_gpu==1&&r.backend_status==0);CHECK(std::memcmp(in.data(),source.data(),sizeof(in))==0);++executed;
        for(uint32_t i=0;i<PIXAURA_GPU_CASE_PIXELS*4;++i)if(ev==0||i%4==3)CHECK(std::memcmp(&out[i],&in[i],4)==0);
    }
    in.fill(.5f);out.fill(-99);const auto sentinel=out;pixaura_gpu_result r{9,9,9,9},old=r;
    // Adapter error protocol covers capability, queue, pipeline, allocation,
    // submission and readback; real platform failures use these same statuses.
    for(int32_t fault:{5,8,14}){state.status=fault;state.corrupt=false;
        CHECK(pixaura_gpu_tile(1,in.data(),PIXAURA_GPU_CASE_PIXELS,1000,dispatch,&state,current,&state,0,out.data(),&r)==fault);
        CHECK(out==sentinel&&std::memcmp(&r,&old,sizeof(r))==0);
        CHECK(pixaura_gpu_tile(1,in.data(),PIXAURA_GPU_CASE_PIXELS,1000,dispatch,&state,current,&state,1,out.data(),&r)==0);
        CHECK(r.used_gpu==0&&r.backend_status==fault&&out[0]==1&&out[3]==.5f);out=sentinel;r=old;
    }
    state.status=0;state.corrupt=true;
    CHECK(pixaura_gpu_tile(1,in.data(),PIXAURA_GPU_CASE_PIXELS,1000,dispatch,&state,current,&state,0,out.data(),&r)==14);CHECK(out==sentinel);
    CHECK(pixaura_gpu_tile(1,in.data(),PIXAURA_GPU_CASE_PIXELS,1000,dispatch,&state,current,&state,1,out.data(),&r)==0);CHECK(r.used_gpu==0&&r.backend_status==14&&out[0]==1);out=sentinel;r=old;
    state.cancel=true;
    CHECK(pixaura_gpu_tile(1,in.data(),PIXAURA_GPU_CASE_PIXELS,0,dispatch,&state,current,&state,1,out.data(),&r)==13);CHECK(out==sentinel);state.cancel=false;
    const int calls=state.calls;CHECK(pixaura_gpu_tile(1,in.data(),PIXAURA_GPU_CASE_PIXELS,0,dispatch,&state,current,&state,1,out.data(),&r)==13);CHECK(calls==state.calls);
    state.current=1;state.corrupt=false;
    CHECK(pixaura_gpu_tile(1,in.data(),PIXAURA_GPU_CASE_PIXELS,0,nullptr,nullptr,nullptr,nullptr,1,out.data(),&r)==0);CHECK(r.used_gpu==0&&r.backend_status==5);out=sentinel;r=old;
    CHECK(pixaura_gpu_tile(2,in.data(),1,0,dispatch,&state,nullptr,nullptr,1,out.data(),&r)==2);
    CHECK(pixaura_gpu_tile(1,in.data(),PIXAURA_GPU_MAX_PIXELS+1,0,dispatch,&state,nullptr,nullptr,1,out.data(),&r)==8);
    CHECK(pixaura_gpu_tile(1,in.data(),1,5001,dispatch,&state,nullptr,nullptr,1,out.data(),&r)==7);
    in[0]=std::numeric_limits<float>::max();CHECK(pixaura_gpu_tile(1,in.data(),1,1000,dispatch,&state,nullptr,nullptr,1,out.data(),&r)==7);CHECK(out==sentinel);
    in[0]=1;in[3]=0;CHECK(pixaura_gpu_tile(1,in.data(),1,0,dispatch,&state,nullptr,nullptr,1,out.data(),&r)==7);CHECK(out==sentinel);
    in[3]=1;in[0]=std::numeric_limits<float>::quiet_NaN();CHECK(pixaura_gpu_tile(1,in.data(),1,0,dispatch,&state,nullptr,nullptr,1,out.data(),&r)==7);
    in.fill(.5f);PreviewOwner owner;pixaura_decode_limits limits{};CHECK(pixaura_decode_default_limits(1,&limits)==0);
    const uint8_t identity[]="eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee";CHECK(pixaura_decode_context_init(1,&owner.context,sizeof(owner.context),identity,32,&limits)==0);
    CHECK(pixaura_preview_begin(&owner.context,1,&owner.ticket)==0);
    CHECK(pixaura_gpu_tile(1,in.data(),1,0,supersede,&owner,preview_current,&owner,1,out.data(),&r)==13);CHECK(out==sentinel);
    CHECK(pixaura_preview_stop(&owner.context)==0);CHECK(pixaura_decode_context_destroy(&owner.context)==0);
    for(int ev=-5000;ev<=5000;++ev){float g=0;CHECK(pixaura_gpu_gain(1,ev,&g)==0);CHECK(std::abs(double(g)-std::exp2(double(ev)/1000))<=1e-7*std::exp2(double(ev)/1000));}
    std::printf("GPU protocol checks=%u corpus=%u; injected adapters are regression evidence, not hardware certification\n",checks,executed);return 0;
}
