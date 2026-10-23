#include "pixaura/assist.h"
#include "pixaura/agent_execution.h"
#include "../../../tests/fixtures/decode/fixtures.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <thread>
extern "C" int32_t pixaura_test_editor_document(pixaura_editor_context*,uint8_t*,uint64_t,uint64_t*);
extern "C" int32_t pixaura_test_agent_candidate_state(pixaura_editor_context*,uint32_t*,uint32_t*);
extern "C" int32_t pixaura_test_assist_control(pixaura_editor_context*,uint32_t,uint32_t);
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"assist experience %d: %s\n",__LINE__,#x);std::abort();}}while(false)
static const uint8_t* bytes(const std::string& s){return reinterpret_cast<const uint8_t*>(s.data());}
static int64_t read(void* raw,uint8_t* out,uint64_t count){auto& at=*static_cast<std::size_t*>(raw);count=std::min<uint64_t>(count,sizeof(decode_png)-at);std::memcpy(out,decode_png+at,static_cast<std::size_t>(count));at+=static_cast<std::size_t>(count);return static_cast<int64_t>(count);}
struct Owner {
 pixaura_editor_context c{};std::filesystem::path path;bool destroyed=false;
 Owner(const std::filesystem::path& parent,unsigned serial){char id[33];std::snprintf(id,sizeof(id),"%032x",serial);
  path=parent/(std::string("assist-")+id);CHECK(std::filesystem::create_directory(path));auto root=path.generic_string();std::size_t at=0;
  CHECK(pixaura_editor_open(1,&c,bytes(root),root.size(),reinterpret_cast<const uint8_t*>(id),32,read,&at,sizeof(decode_png))==0);}
 ~Owner(){if(!destroyed){CHECK(pixaura_editor_stop(&c)==0);CHECK(pixaura_editor_destroy(&c)==0);}}
 std::string dump(){uint64_t n=0;CHECK(pixaura_test_editor_document(&c,nullptr,0,&n)==0);std::string result(static_cast<std::size_t>(n),' ');CHECK(pixaura_test_editor_document(&c,reinterpret_cast<uint8_t*>(result.data()),n,&n)==0);return result;}
 int request(uint32_t index,pixaura_assist_advice& out){const pixaura_assist_request r{1,sizeof(r),index,0};return pixaura_editor_assist_request(1,&c,&r,&out);}
 void manual(){pixaura_assist_advice choice{};CHECK(pixaura_assist_choice(1,2,&choice)==0);CHECK(pixaura_editor_select(&c,choice.tool_index)==0);const int32_t value=500;CHECK(pixaura_editor_update(&c,&value,1)==0);CHECK(pixaura_editor_action(&c,1)==0);}
};
static void wait(Owner& owner){const auto until=std::chrono::steady_clock::now()+std::chrono::seconds(5);while(pixaura_test_assist_control(&owner.c,1,0)==0 && std::chrono::steady_clock::now()<until)std::this_thread::yield();CHECK(pixaura_test_assist_control(&owner.c,1,0)==1);}
int main(int argc,char** argv){CHECK(argc==2);const auto root=std::filesystem::u8path(argv[1])/("assist-suite-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));CHECK(std::filesystem::create_directory(root));unsigned serial=98000;
 Owner owner(root,++serial),other(root,++serial);const auto before=owner.dump();
 pixaura_assist_advice result{};
 // Hand-reviewed UI request goldens, in frozen Structured Intent goal order.
 // Tool indices refer to the separately certified sorted manual registry v1.
 const uint32_t tools[]={1,2,4,5,7,8,10,0,9,3,6};
 const int32_t amounts[][4]={{100,0,0,0},{250,0,0,0},{500,0,0,0},{-500,0,0,0},
  {1200,0,0,0},{500,0,0,0},{7000,0,0,0},{250,0,0,0},{250,0,0,0},
  {800000,800000,100000,100000},{1,0,0,0}};
 for(uint32_t i=0;i<PIXAURA_ASSIST_CHOICES;++i){
  pixaura_assist_advice choice{};CHECK(pixaura_assist_choice(1,i,&choice)==0);CHECK(choice.api_version==1&&choice.struct_size==88&&choice.parameter_count>=1&&choice.parameter_count<=4&&choice.request_serial==0);
  CHECK(choice.tool_index==tools[i]);CHECK(std::memcmp(choice.values,amounts[i],sizeof(choice.values))==0);
  CHECK(pixaura_editor_assist_current(1,&owner.c,&choice)==9);
  const auto code=owner.request(i,result);if(code)std::fprintf(stderr,"choice %u status %d\n",i,code);CHECK(code==0);CHECK(result.choice_index==i&&result.tool_index==choice.tool_index&&std::memcmp(result.values,choice.values,sizeof(result.values))==0);
  CHECK(pixaura_editor_assist_current(1,&owner.c,&result)==0);CHECK(pixaura_editor_assist_current(1,&other.c,&result)==9);
  uint32_t candidate=99,active=99;CHECK(pixaura_test_agent_candidate_state(&owner.c,&candidate,&active)==0);CHECK(candidate==0&&active==0);
  const pixaura_agent_execution_request execution{1,sizeof(execution),1,0,{0,0,0,0}};pixaura_agent_execution_receipt receipt{};
  const uint8_t invalid_plan[]={'{','}'};CHECK(pixaura_editor_agent_execute(1,&owner.c,&execution,nullptr,invalid_plan,sizeof(invalid_plan),&receipt)==15);
  CHECK(owner.dump()==before);CHECK(pixaura_editor_assist_cancel(1,&owner.c)==0);CHECK(pixaura_editor_assist_current(1,&owner.c,&result)==9);
 }
 CHECK(owner.request(2,result)==0);const auto old=result;CHECK(owner.request(2,result)==0);CHECK(result.request_serial>old.request_serial);CHECK(pixaura_editor_assist_current(1,&owner.c,&old)==9);
 auto forged=result;forged.values[0]++;CHECK(pixaura_editor_assist_current(1,&owner.c,&forged)==9);forged=result;forged.request_serial++;CHECK(pixaura_editor_assist_current(1,&owner.c,&forged)==9);
 forged=result;forged.api_version=2;CHECK(pixaura_editor_assist_current(1,&owner.c,&forged)==2);forged=result;forged.struct_size--;CHECK(pixaura_editor_assist_current(1,&owner.c,&forged)==1);forged=result;forged.reserved=1;CHECK(pixaura_editor_assist_current(1,&owner.c,&forged)==1);
 // Use suggestion is normal tool navigation, never setting/previewing the advised value.
 CHECK(pixaura_editor_select(&owner.c,result.tool_index)==0);CHECK(pixaura_editor_assist_current(1,&owner.c,&result)==9);CHECK(owner.dump()==before);
 CHECK(pixaura_editor_action(&owner.c,1)==0);CHECK(owner.dump()==before);CHECK(pixaura_editor_assist_current(1,&owner.c,&result)==9);
 owner.manual();CHECK(owner.dump()!=before);CHECK(owner.request(0,result)==0);CHECK(pixaura_editor_action(&owner.c,2)==0);CHECK(pixaura_editor_assist_current(1,&owner.c,&result)==9);
 CHECK(owner.request(0,result)==0);CHECK(pixaura_editor_action(&owner.c,3)==0);CHECK(pixaura_editor_assist_current(1,&owner.c,&result)==9);
 // Full source/ABA, cancel, stop and duplicate-request orderings at both checkpoints.
 for(uint32_t point:{0u,1u})for(unsigned event=0;event<6;++event){
  Owner o(root,++serial);if(event==2||event==3)o.manual();if(event==3)CHECK(pixaura_editor_action(&o.c,2)==0);
  const auto prior=o.dump();CHECK(pixaura_test_assist_control(&o.c,0,point)==0);
  pixaura_assist_advice packet{};std::memset(&packet,0xa5,sizeof(packet));const auto sentinel=packet;std::atomic<int> status{-1};
  std::thread request([&]{status=o.request(0,packet);});wait(o);
  pixaura_assist_advice duplicate{};CHECK(o.request(0,duplicate)==8);
  if(event==0)CHECK(pixaura_editor_assist_cancel(1,&o.c)==0);
  if(event==1)o.manual();
  if(event==2)CHECK(pixaura_editor_action(&o.c,2)==0);
  if(event==3)CHECK(pixaura_editor_action(&o.c,3)==0);
  if(event==4)CHECK(pixaura_editor_stop(&o.c)==0);
  CHECK(pixaura_test_assist_control(&o.c,2,0)==0);request.join();
  CHECK(status==(event==5?0:event==0||event==4?13:9));
  if(event!=5)CHECK(std::memcmp(&packet,&sentinel,sizeof(packet))==0);
  if(event==0||event==5)CHECK(o.dump()==prior);
  if(event==5){CHECK(pixaura_editor_assist_current(1,&o.c,&packet)==0);CHECK(pixaura_editor_assist_cancel(1,&o.c)==0);CHECK(pixaura_editor_assist_current(1,&o.c,&packet)==9);}
  if(event==4){CHECK(o.request(0,duplicate)==13);CHECK(pixaura_editor_destroy(&o.c)==0);o.destroyed=true;CHECK(o.request(0,duplicate)==3);}
 }
 for(uint32_t fault=1;fault<=5;++fault){Owner o(root,++serial);const auto prior=o.dump();CHECK(pixaura_test_assist_control(&o.c,3,fault)==0);
  const int expected[]={0,13,6,5,8,14};CHECK(o.request(0,result)==expected[fault]);CHECK(o.dump()==prior);o.manual();CHECK(o.dump()!=prior);
 }
 {Owner o(root,++serial);CHECK(pixaura_test_assist_control(&o.c,4,0)==0);CHECK(o.request(0,result)==8);o.manual();}
 pixaura_assist_request request{1,sizeof(request),0,0};pixaura_assist_advice output{};
 CHECK(pixaura_editor_assist_request(2,&owner.c,&request,&output)==2);request.api_version=2;CHECK(pixaura_editor_assist_request(1,&owner.c,&request,&output)==2);request.api_version=1;
 request.struct_size--;CHECK(pixaura_editor_assist_request(1,&owner.c,&request,&output)==1);request.struct_size=sizeof(request);request.reserved=1;CHECK(pixaura_editor_assist_request(1,&owner.c,&request,&output)==1);request.reserved=0;
 request.choice_index=UINT32_MAX;CHECK(pixaura_editor_assist_request(1,&owner.c,&request,&output)==5);request.choice_index=0;
 CHECK(pixaura_editor_assist_request(1,&owner.c,nullptr,&output)==1);CHECK(pixaura_editor_assist_request(1,&owner.c,&request,nullptr)==1);
 CHECK(pixaura_editor_assist_request(1,&owner.c,&request,reinterpret_cast<pixaura_assist_advice*>(&owner.c))==1);
 alignas(8) std::array<uint8_t,100> misaligned{};CHECK(pixaura_editor_assist_request(1,&owner.c,reinterpret_cast<pixaura_assist_request*>(misaligned.data()+1),&output)==1);
 CHECK(pixaura_assist_choice(2,0,&output)==2);CHECK(pixaura_assist_choice(1,11,&output)==5);CHECK(pixaura_assist_choice(1,0,nullptr)==1);
 std::printf("AI Assist experience: %u checks, zero failures; choices=11, race_orderings=12; no approval/candidate/history effects; manual navigation only\n",checks);
}
