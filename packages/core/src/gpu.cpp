#include "pixaura/gpu.h"
#include "exposure_gain.hpp"
#include <algorithm>
#include <cmath>
#include <cfenv>
#include <cstring>
#include <limits>
#include <memory>
#include <string>
namespace {
constexpr int32_t cases[]={0,-5000,-4999,-3001,-1000,-1,1,333,999,1000,1001,2500,4999,5000};
void need(bool ok,int32_t code){if(!ok)throw code;}
template<class F> int32_t boundary(F&& f)noexcept {try{f();return 0;}catch(int32_t e){return e;}catch(const std::bad_alloc&){return 8;}catch(...){return 14;}}
uint32_t bits(float f){uint32_t n;std::memcpy(&n,&f,4);return n;}
bool finite(float f){return (bits(f)&0x7f800000u)!=0x7f800000u;}
double gain(int32_t e){need(e>=-5000&&e<=5000,7);return pixaura::evaluation::exposure_gain(e);}
}
int32_t pixaura_gpu_gain(uint32_t version,int32_t e,float* out){return boundary([&]{need(version==1,2);need(out!=nullptr,1);need(std::fegetround()==FE_TONEAREST,7);const auto g=static_cast<float>(gain(e));*out=g;});}
int32_t pixaura_gpu_tile(uint32_t version,const float* source,uint32_t count,int32_t e,
    pixaura_gpu_dispatch dispatch,void* backend,pixaura_gpu_current current,void* generation,
    uint32_t fallback,float* out,pixaura_gpu_result* result){return boundary([&]{
    need(version==1,2);need(source&&out&&result&&source!=out&&fallback<=1,1);
    need(count>0&&count<=PIXAURA_GPU_MAX_PIXELS,8);need(std::fegetround()==FE_TONEAREST,7);
    const auto source_address=reinterpret_cast<uintptr_t>(source),output_address=reinterpret_cast<uintptr_t>(out);
    const auto span=static_cast<uintptr_t>(count)*16;
    need(source_address<=UINTPTR_MAX-span&&output_address<=UINTPTR_MAX-span,1);
    need(source_address+span<=output_address||output_address+span<=source_address,1);
    const auto check=[&]{need(!current||current(generation)!=0,13);};check();
    const double g=gain(e);const std::size_t n=static_cast<std::size_t>(count)*4;
    auto cpu=std::make_unique<float[]>(n),candidate=std::make_unique<float[]>(n);
    for(std::size_t i=0;i<n;i+=4){if(i%4096==0)check();
        need(finite(source[i+3])&&source[i+3]>=0&&source[i+3]<=1,7);
        for(unsigned c=0;c<4;++c){need(finite(source[i+c]),7);
            if(c<3){need(source[i+3]!=0||source[i+c]==0,7);const double v=double(source[i+c])*g;
                need(std::isfinite(v)&&std::abs(v)<=std::numeric_limits<float>::max(),7);}
            if(c==3||e==0)std::memcpy(cpu.get()+i+c,source+i+c,4);
            else cpu[i+c]=static_cast<float>(double(source[i+c])*g);
        }
    }
    check();int32_t status=dispatch?dispatch(backend,source,candidate.get(),count,static_cast<float>(g),e==0?1u:0u):5;
    check();
    if(status==0){for(std::size_t i=0;i<n;++i){if(i%4096==0)check();
        const bool exact=e==0||i%4==3;
        if(!finite(candidate[i])||(exact?bits(cpu[i])!=bits(candidate[i]):
            std::abs(double(cpu[i])-double(candidate[i]))>4e-37+2e-6*std::abs(double(cpu[i])))){status=14;break;}
    }}
    if(status==13)throw int32_t{13};
    need(status==0||fallback==1,status);check();
    const pixaura_gpu_result r{1,sizeof(pixaura_gpu_result),status,status==0?1u:0u};
    std::memcpy(out,status==0?candidate.get():cpu.get(),n*4);*result=r;
});}
uint32_t pixaura_gpu_case_count(void){return static_cast<uint32_t>(sizeof(cases)/sizeof(cases[0]));}
int32_t pixaura_gpu_case(uint32_t index,float* out,uint32_t capacity,int32_t* ev){return boundary([&]{
    need(out&&ev,1);need(index<pixaura_gpu_case_count()&&capacity>=PIXAURA_GPU_CASE_PIXELS,8);
    uint32_t seed=0x12345678;for(uint32_t i=0;i<PIXAURA_GPU_CASE_PIXELS;++i){
        seed=seed*1664525u+1013904223u;const float a=i%7==0?0:float((seed>>16)&255)/255;
        for(unsigned c=0;c<3;++c){seed=seed*1664525u+1013904223u;out[i*4+c]=a==0?0:float(int32_t(seed>>16)-32768)/4096;}
        out[i*4+3]=a;
    }
    const float edge[]={-0.0f,0,0,-0.0f,1e-40f,-1e-40f,1e-38f,1e-30f,
        .25f,-.5f,2,.5f,1e30f,-1e30f,0,1};std::memcpy(out,edge,sizeof(edge));*ev=cases[index];
});}
int32_t pixaura_gpu_hardware(uint32_t type,uint32_t vendor,const char* name,uint32_t bytes){
    return boundary([&]{need(name&&bytes>0&&bytes<=256,5);need((type==1||type==2)&&vendor!=0&&vendor!=0x1ae0&&vendor!=0x10005,5);
        std::string s(name,bytes);for(char& c:s)if(c>='A'&&c<='Z')c=char(c-'A'+'a');
        for(const char* word:{"swiftshader","llvmpipe","lavapipe","software","virtual","emulator","venus"})need(s.find(word)==std::string::npos,5);
    });
}
int32_t pixaura_gpu_vulkan_capable(uint32_t version,const pixaura_vulkan_capabilities* c){return boundary([&]{
    need(version==1,2);need(c!=nullptr,1);need((c->api_version>>22)==1,5);
    need(c->queue_count>0&&(c->queue_flags&2u)!=0,5);
    need(c->invocations>=64&&c->group_size_x>=64&&c->group_count_x>=256&&
        c->storage_range>=PIXAURA_GPU_MAX_PIXELS*16&&c->push_bytes>=12&&c->storage_descriptors>=2,5);
});}
