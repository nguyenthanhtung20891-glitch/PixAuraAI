#include "pixaura/gpu.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <new>
static thread_local int fail_after=-1;
void* operator new(std::size_t n){if(fail_after==0)throw std::bad_alloc();if(fail_after>0)--fail_after;if(auto p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete(void* p)noexcept{std::free(p);}
void operator delete[](void* p)noexcept{std::free(p);}
void operator delete(void* p,std::size_t)noexcept{std::free(p);}
void operator delete[](void* p,std::size_t)noexcept{std::free(p);}
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"GPU allocation check %d\n",__LINE__);std::_Exit(1);}}while(false)
int main(){
    const float source[4]={.25f,-.5f,2,.5f};unsigned failures=0;
    for(int n=0;n<3;++n){float output[4]={99,99,99,99},sentinel[4];std::memcpy(sentinel,output,sizeof(output));pixaura_gpu_result result{9,9,9,9},old=result;
        fail_after=n;const auto status=pixaura_gpu_tile(1,source,1,1000,nullptr,nullptr,nullptr,nullptr,1,output,&result);fail_after=-1;
        if(status){CHECK(status==8);CHECK(std::memcmp(output,sentinel,sizeof(output))==0);CHECK(std::memcmp(&old,&result,sizeof(old))==0);++failures;}
        else CHECK(output[0]==.5f&&output[1]==-1&&output[2]==4&&output[3]==.5f&&result.used_gpu==0);
        CHECK(pixaura_gpu_tile(1,source,1,1000,nullptr,nullptr,nullptr,nullptr,1,output,&result)==0);
    }
    CHECK(failures==2);std::printf("GPU private allocation failures recovered=2/2; output and source preserved\n");return 0;
}
