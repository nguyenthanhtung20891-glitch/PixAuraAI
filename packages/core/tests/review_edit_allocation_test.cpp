#include "pixaura/review_edit.h"
#include <array>
// Independently compiled shipping sources; replacement allocator never ships.
#include "pixaura/review_preview.h"
#include "pixaura/review_inspection.h"
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
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <dbghelp.h>
#endif
static thread_local int64_t fail_after=-1;
static thread_local bool meter=false;
static thread_local uint64_t allocated=0;
static thread_local unsigned active_path=0;
static thread_local int64_t active_index=-1;
static void termination_trace() noexcept {
#ifdef _MSC_VER
 const auto process=GetCurrentProcess();SymSetOptions(SYMOPT_DEFERRED_LOADS|SYMOPT_LOAD_LINES|SYMOPT_NO_PROMPTS|SYMOPT_IGNORE_NT_SYMPATH);
 if(SymInitialize(process,".",TRUE)){
  void* frames[32]{};const auto count=CaptureStackBackTrace(0,32,frames,nullptr);
  for(USHORT i=0;i<count;++i){alignas(SYMBOL_INFO) unsigned char bytes[sizeof(SYMBOL_INFO)+256]{};
   auto* symbol=reinterpret_cast<SYMBOL_INFO*>(bytes);symbol->SizeOfStruct=sizeof(SYMBOL_INFO);symbol->MaxNameLen=255;
   const auto address=reinterpret_cast<DWORD64>(frames[i]);DWORD64 displacement=0;
   if(SymFromAddr(process,address,&displacement,symbol))std::fprintf(stderr,"review terminate frame=%s\n",symbol->Name);
   IMAGEHLP_LINE64 line{};line.SizeOfStruct=sizeof(line);DWORD offset=0;
   if(SymGetLineFromAddr64(process,address,&offset,&line))std::fprintf(stderr,"review terminate source=%s:%lu\n",line.FileName,static_cast<unsigned long>(line.LineNumber));
  }
  SymCleanup(process);
 }
#endif
}
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
 std::set_terminate([]{fail_after=-1;std::fprintf(stderr,"edit allocation terminate path=%u index=%lld\n",active_path,static_cast<long long>(active_index));termination_trace();std::_Exit(1);});
 CHECK(argc==3);CHECK(argv[2][0]>='0'&&argv[2][0]<='3'&&argv[2][1]==0);const auto worker=static_cast<unsigned>(argv[2][0]-'0');const auto path=std::filesystem::u8path(argv[1])/("edit-alloc-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+"-"+std::to_string(worker));CHECK(std::filesystem::create_directory(path));
 const auto directory=path/std::to_string(worker);CHECK(std::filesystem::create_directory(directory));const auto worker_root=directory.generic_string();
 char id[33];std::snprintf(id,sizeof(id),"%032x",950000u+worker);pixaura_editor_context c{};std::size_t at=0;
 pixaura_capability_profile profile{};CHECK(pixaura_capability_baseline(1,&profile)==0);const pixaura_policy_observation observation{1,sizeof(observation),1,1,0,1,0,0,0,1};pixaura_execution_strategy strategy{};
 CHECK(pixaura_editor_open_policy(1,&c,b(worker_root),worker_root.size(),reinterpret_cast<const uint8_t*>(id),32,read,&at,sizeof(decode_png),&profile,&observation,&strategy)==0);
 const auto state=[&]{uint8_t bytes[1024];uint64_t n=0;CHECK(pixaura_editor_state(&c,bytes,sizeof(bytes),&n)==0);return std::string(reinterpret_cast<char*>(bytes),static_cast<std::size_t>(n));};const auto stable=state();
 for(unsigned kind=0;kind<5;++kind){bool complete=false;uint64_t maximum=0;unsigned failures=0;
  // Exact residue partition: every failure index is still injected, once.
  // Allocator controls and each editor/document are local to this worker.
  pixaura_auto_proposal proposal{};const pixaura_auto_request auto_request{1,sizeof(auto_request),2,2};CHECK(pixaura_editor_auto_request(1,&c,&auto_request,nullptr,0,&proposal)==0);
   uint8_t bytes[8192];uint64_t count=0;CHECK(pixaura_editor_auto_plan(1,&c,&proposal,bytes,sizeof(bytes),&count)==0);pixaura_review_inspection inspection{};CHECK(pixaura_review_inspect(1,&c,&proposal,bytes,count,&inspection)==0);
   pixaura_review_edit_result session{};if(kind)CHECK(pixaura_review_edit_begin(1,&c,&inspection,&session)==0);
  for(int64_t n=worker;n<16384;n+=4){
   pixaura_review_edit_request request{};request.api_version=1;request.struct_size=sizeof(request);request.operation=kind;request.ticket=session.ticket;
   if(kind&&kind!=4){request.expected_action=inspection.actions[0];std::memcpy(request.tool_id,inspection.actions[0].tool_id,32);if(kind==1){std::memcpy(request.parameter_name,inspection.actions[0].parameters[0].name,32);request.raw_value=123;}}
   pixaura_review_edit_result output{};std::memset(&output,0xa5,sizeof(output));const auto sentinel=output;
   active_path=kind;active_index=n;fail_after=n;meter=true;allocated=0;
   const auto status=kind?pixaura_review_edit(1,&c,&request,&output):pixaura_review_edit_begin(1,&c,&inspection,&output);
   meter=false;fail_after=-1;maximum=std::max(maximum,allocated);CHECK(state()==stable);
   if(!status){CHECK(failures>0);CHECK(output.api_version==1);complete=true;std::printf("Edit allocation worker=%u operation=%u failures=%u first_success=%lld cumulative_requested_max=%llu result_bytes=%zu\n",worker,kind,failures,static_cast<long long>(n),static_cast<unsigned long long>(maximum),sizeof(output));break;}
   CHECK(status==8);CHECK(std::memcmp(&output,&sentinel,sizeof(output))==0);++failures;
   if(kind){pixaura_review_edit_result current{};CHECK(pixaura_review_edit_current(1,&c,&session.ticket,&current)==0);
    if(current.state==4)CHECK(pixaura_editor_auto_current(1,&c,&proposal)!=0);
    // Explicit restore resets the owned metadata fixture outside fault injection.
    // Every next injection still begins with the same complete original Plan.
    pixaura_review_edit_request restore{};restore.api_version=1;restore.struct_size=sizeof(restore);restore.operation=4;restore.ticket=current.ticket;
    CHECK(pixaura_review_edit(1,&c,&restore,&session)==0);CHECK(session.state==1);
    CHECK(std::memcmp(session.ticket.plan_sha256,inspection.plan_sha256,64)==0);
   }
  }CHECK(complete);
 }
 CHECK(pixaura_editor_stop(&c)==0);CHECK(pixaura_editor_destroy(&c)==0);
 std::puts("Edit allocation worker: outputs atomic; ordinary document/history unchanged; no image resources; every failure index retained");
}
