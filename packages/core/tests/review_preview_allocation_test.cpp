// Independently compiled shipping sources; replacement allocator never ships.
#include "pixaura/review_preview.h"
#include "../../../tests/fixtures/decode/fixtures.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <exception>
#include <new>
#include <string>
#ifdef _MSC_VER
#include <crtdbg.h>
#endif
static thread_local int64_t fail_after=-1;
static thread_local bool meter=false;
static thread_local uint64_t allocated=0;
static unsigned active_path=0;
static int64_t active_index=-1;
void* operator new(std::size_t bytes){if(fail_after==0)throw std::bad_alloc();if(fail_after>0)--fail_after;if(meter)allocated+=bytes;if(auto* p=std::malloc(bytes?bytes:1))return p;throw std::bad_alloc();}
void* operator new[](std::size_t bytes){return ::operator new(bytes);}
void operator delete(void* p) noexcept{std::free(p);}
void operator delete[](void* p) noexcept{std::free(p);}
void operator delete(void* p,std::size_t) noexcept{std::free(p);}
void operator delete[](void* p,std::size_t) noexcept{std::free(p);}
#define CHECK(x) do{if(!(x)){fail_after=-1;std::fprintf(stderr,"review allocation line=%d %s\n",__LINE__,#x);std::_Exit(1);}}while(false)
static const uint8_t* b(const std::string& s){return reinterpret_cast<const uint8_t*>(s.data());}
static int64_t read(void* raw,uint8_t* out,uint64_t count){auto& at=*static_cast<std::size_t*>(raw);count=std::min<uint64_t>(count,sizeof(decode_png)-at);std::memcpy(out,decode_png+at,static_cast<std::size_t>(count));at+=static_cast<std::size_t>(count);return static_cast<int64_t>(count);}
int main(int argc,char** argv){
#ifdef _MSC_VER
 _set_error_mode(_OUT_TO_STDERR);_set_abort_behavior(0,_WRITE_ABORT_MSG|_CALL_REPORTFAULT);
#ifdef _DEBUG
 for(const int kind:{_CRT_WARN,_CRT_ERROR,_CRT_ASSERT}){_CrtSetReportMode(kind,_CRTDBG_MODE_FILE);_CrtSetReportFile(kind,_CRTDBG_FILE_STDERR);}
#endif
#endif
 std::set_terminate([]{fail_after=-1;std::fprintf(stderr,"review allocation terminate path=%u index=%lld\n",active_path,static_cast<long long>(active_index));std::fflush(stderr);std::_Exit(1);});
 CHECK(argc==2);const auto path=std::filesystem::u8path(argv[1])/("review-alloc-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));CHECK(std::filesystem::create_directory(path));const auto root=path.generic_string();
 const uint8_t id[]="abc00000000000000000000000000001";pixaura_editor_context c{};std::size_t at=0;
 pixaura_capability_profile profile{};CHECK(pixaura_capability_baseline(1,&profile)==0);
 const pixaura_policy_observation observation{1,sizeof(observation),1,1,0,1,0,0,0,1};pixaura_execution_strategy strategy{};
 CHECK(pixaura_editor_open_policy(1,&c,b(root),root.size(),id,32,read,&at,sizeof(decode_png),&profile,&observation,&strategy)==0);
 const auto state=[&]{uint8_t bytes[1024];uint64_t n=0;CHECK(pixaura_editor_state(&c,bytes,sizeof(bytes),&n)==0);return std::string(reinterpret_cast<char*>(bytes),static_cast<std::size_t>(n));};
 const auto stable=state();pixaura_auto_proposal proposal{};const pixaura_auto_request request{1,sizeof(request),2,2};CHECK(pixaura_editor_auto_request(1,&c,&request,nullptr,0,&proposal)==0);
 uint8_t bytes[8192];uint64_t count=0;CHECK(pixaura_editor_auto_plan(1,&c,&proposal,bytes,sizeof(bytes),&count)==0);
 uint64_t maximum[2]={0,0};
 for(unsigned path_index=0;path_index<2;++path_index){unsigned failures=0;bool complete=false;
  for(int64_t n=0;n<4096;++n){pixaura_review_preview_ticket ticket{};
   if(path_index)CHECK(pixaura_review_preview_begin(1,&c,&proposal,bytes,count,512,512,&ticket)==0);
   pixaura_review_preview_pair pair{};std::memset(&pair,0xa5,sizeof(pair));const auto old_pair=pair;
   pixaura_review_preview_ticket output{};std::memset(&output,0xa5,sizeof(output));const auto old_ticket=output;
   active_path=path_index;active_index=n;
   std::fprintf(stderr,"review allocation path=%u index=%lld\n",path_index,static_cast<long long>(n));std::fflush(stderr);
   fail_after=n;allocated=0;meter=true;
   const auto status=path_index?pixaura_review_preview_render(1,&c,&ticket,&pair):pixaura_review_preview_begin(1,&c,&proposal,bytes,count,512,512,&output);
   meter=false;fail_after=-1;maximum[path_index]=std::max(maximum[path_index],allocated);
   CHECK(state()==stable);
   if(!status){CHECK(failures>0);if(path_index)CHECK(pixaura_review_preview_release(1,&c,&pair)==0);else ticket=output;
    CHECK(pixaura_review_preview_cancel(1,&c,&ticket)==0);complete=true;std::printf("Review allocation path=%u failures=%u cumulative_requested_max=%llu\n",path_index,failures,static_cast<unsigned long long>(maximum[path_index]));break;}
   CHECK(status==8);CHECK(std::memcmp(&pair,&old_pair,sizeof(pair))==0);CHECK(std::memcmp(&output,&old_ticket,sizeof(output))==0);++failures;
   if(path_index)CHECK(pixaura_review_preview_cancel(1,&c,&ticket)==0);
  }
  CHECK(complete);
 }
 // No leaked private image/cancellation handles across failed pair publication.
 for(unsigned i=0;i<70;++i){pixaura_review_preview_ticket t{};pixaura_review_preview_pair p{};
  CHECK(pixaura_review_preview_begin(1,&c,&proposal,bytes,count,512,512,&t)==0);CHECK(pixaura_review_preview_render(1,&c,&t,&p)==0);
  CHECK(pixaura_review_preview_release(1,&c,&p)==0);CHECK(pixaura_review_preview_cancel(1,&c,&t)==0);}
 CHECK(state()==stable);CHECK(pixaura_editor_stop(&c)==0);CHECK(pixaura_editor_destroy(&c)==0);
 std::puts("Review allocation: exhaustive begin/render failures preserve outputs and history; repeated release recovers all native handles");
}
