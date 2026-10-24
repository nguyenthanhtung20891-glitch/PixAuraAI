#include "pixaura/auto_proposal.h"
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
extern "C" int32_t pixaura_test_auto_control(pixaura_editor_context*,uint32_t,uint32_t);
extern "C" int32_t pixaura_test_assist_control(pixaura_editor_context*,uint32_t,uint32_t);
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"auto proposal %d: %s\n",__LINE__,#x);std::abort();}}while(false)
static const uint8_t* bytes(const std::string& s){return reinterpret_cast<const uint8_t*>(s.data());}
static int64_t read(void* raw,uint8_t* out,uint64_t count){auto& at=*static_cast<std::size_t*>(raw);count=std::min<uint64_t>(count,sizeof(decode_png)-at);std::memcpy(out,decode_png+at,static_cast<std::size_t>(count));at+=static_cast<std::size_t>(count);return static_cast<int64_t>(count);}
struct Owner {
 pixaura_editor_context c{};bool destroyed=false;
 Owner(const std::filesystem::path& parent,unsigned serial){char id[33];std::snprintf(id,sizeof(id),"%032x",serial);const auto path=parent/(std::string("auto-")+id);
  CHECK(std::filesystem::create_directory(path));const auto root=path.generic_string();std::size_t at=0;
  CHECK(pixaura_editor_open(1,&c,bytes(root),root.size(),reinterpret_cast<const uint8_t*>(id),32,read,&at,sizeof(decode_png))==0);}
 ~Owner(){if(!destroyed){CHECK(pixaura_editor_stop(&c)==0);CHECK(pixaura_editor_destroy(&c)==0);}}
 std::string dump(){uint64_t n=0;CHECK(pixaura_test_editor_document(&c,nullptr,0,&n)==0);std::string out(static_cast<std::size_t>(n),' ');CHECK(pixaura_test_editor_document(&c,reinterpret_cast<uint8_t*>(out.data()),n,&n)==0);return out;}
 std::string binding(){std::array<uint8_t,512> b{};uint64_t n=0;CHECK(pixaura_editor_agent_binding(1,&c,b.data(),b.size(),&n)==0);std::string s(reinterpret_cast<char*>(b.data()),static_cast<std::size_t>(n));CHECK(s.back()=='\n');s.pop_back();return s;}
 int request(uint32_t index,pixaura_auto_proposal& out){const pixaura_auto_request r{1,sizeof(r),index,2};return pixaura_editor_auto_request(1,&c,&r,nullptr,0,&out);}
 int candidate(const std::string& plan,pixaura_auto_proposal& out){const pixaura_auto_request r{1,sizeof(r),0,3};return pixaura_editor_auto_request(1,&c,&r,bytes(plan),plan.size(),&out);}
 std::string plan(const pixaura_auto_proposal& p){uint64_t n=0;CHECK(pixaura_editor_auto_plan(1,&c,&p,nullptr,0,&n)==11);CHECK(n>0&&n<=8192);std::string result(static_cast<std::size_t>(n),' ');CHECK(pixaura_editor_auto_plan(1,&c,&p,reinterpret_cast<uint8_t*>(result.data()),n,&n)==0);return result;}
 void manual(){CHECK(pixaura_editor_select(&c,4)==0);const int32_t v=500;CHECK(pixaura_editor_update(&c,&v,1)==0);CHECK(pixaura_editor_action(&c,1)==0);}
};
static std::string action(unsigned i,const std::string& tool,const std::string& parameters){return "{\"action_index\":"+std::to_string(i)+",\"operation_version\":1,\"parameter_version\":1,\"parameters\":"+parameters+",\"tool_id\":\""+tool+"\"}";}
static std::string plan(Owner& o,const std::string& actions){return "{\"actions\":["+actions+"],\"binding\":"+o.binding()+",\"plan_version\":1}\n";}
static void wait(Owner& o){const auto end=std::chrono::steady_clock::now()+std::chrono::seconds(5);while(!pixaura_test_auto_control(&o.c,1,0)&&std::chrono::steady_clock::now()<end)std::this_thread::yield();CHECK(pixaura_test_auto_control(&o.c,1,0)==1);}
int main(int argc,char** argv){CHECK(argc==2);const auto root=std::filesystem::u8path(argv[1])/("auto-suite-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));CHECK(std::filesystem::create_directory(root));unsigned serial=188000;
 Owner owner(root,++serial),other(root,++serial);const auto original=owner.dump();pixaura_auto_proposal result{};
 const char* tools[]={"pixaura.brightness","pixaura.contrast","pixaura.exposure","pixaura.highlights","pixaura.saturation","pixaura.shadows","pixaura.temperature","pixaura.blur","pixaura.sharpen","pixaura.crop","pixaura.rotate"};
 const char* parameters[]={"{\"milli_linear\":100}","{\"milli_stops\":250}","{\"milli_ev\":500}","{\"milli_ev\":-500}","{\"milli_ratio\":1200}","{\"milli_ev\":500}","{\"kelvin\":7000}","{\"milli_strength\":250}","{\"milli_amount\":250}","{\"height_ppm\":800000,\"width_ppm\":800000,\"x_ppm\":100000,\"y_ppm\":100000}","{\"quarter_turns\":1}"};
 for(uint32_t i=0;i<11;++i){const auto expected=plan(owner,action(0,tools[i],parameters[i]));
  for(unsigned repeat=0;repeat<3;++repeat){CHECK(owner.request(i,result)==0);CHECK(result.action_count==1&&result.kind==2&&result.registry_version==1);CHECK(owner.plan(result)==expected);
   CHECK(pixaura_editor_auto_current(1,&owner.c,&result)==0);CHECK(pixaura_editor_auto_current(1,&other.c,&result)==9);
   uint32_t state=99,active=99;CHECK(pixaura_test_agent_candidate_state(&owner.c,&state,&active)==0);CHECK(state==0&&active==0);
   CHECK(pixaura_test_auto_control(&owner.c,5,0)==0);
   const pixaura_agent_execution_request r{1,sizeof(r),1,0,{0,0,0,0}};pixaura_agent_execution_receipt receipt{};
   CHECK(pixaura_editor_agent_execute(1,&owner.c,&r,nullptr,bytes(expected),expected.size(),&receipt)==15);
   CHECK(owner.dump()==original);CHECK(pixaura_editor_auto_cancel(1,&owner.c)==0);CHECK(pixaura_editor_auto_current(1,&owner.c,&result)==9);
  }
 }
 // Explicit machine-semantic sequences: no inferred recipes or reordering.
 for(const unsigned count:{1u,2u,3u,8u}){std::string actions;for(unsigned i=0;i<count;++i){if(i)actions+=',';actions+=action(i,tools[(i+9)%11],parameters[(i+9)%11]);}
  const auto input=plan(owner,actions);CHECK(owner.candidate(input,result)==0);CHECK(result.action_count==count&&result.kind==3);CHECK(owner.plan(result)==input);CHECK(owner.dump()==original);
 }
 const auto good=plan(owner,action(0,"pixaura.exposure","{\"milli_ev\":100}"));CHECK(owner.candidate(good,result)==0);
 auto forged=result;forged.proposal_id[0]='x';CHECK(pixaura_editor_auto_current(1,&owner.c,&forged)==9);
 forged=result;forged.actions[0].values[0]++;CHECK(pixaura_editor_auto_current(1,&owner.c,&forged)==9);
 forged=result;forged.request_serial++;CHECK(pixaura_editor_auto_current(1,&owner.c,&forged)==9);
 uint64_t required=0;std::array<uint8_t,8192> output{};CHECK(pixaura_editor_auto_plan(1,&owner.c,&result,output.data(),1,&required)==11);
 CHECK(pixaura_editor_auto_plan(1,&other.c,&result,output.data(),output.size(),&required)==9);
 for(const auto& bad:std::array<std::string,9>{{"",good+"x","{}\n",plan(owner,action(0,"unknown","{\"milli_ev\":100}")),plan(owner,action(0,"pixaura.exposure","{\"milli_ev\":5001}")),plan(owner,action(1,"pixaura.exposure","{\"milli_ev\":100}")),std::string(8193,'x'),plan(other,action(0,"pixaura.exposure","{\"milli_ev\":100}")),std::string("\xff",1)}}){CHECK(owner.candidate(bad,result)!=0);CHECK(owner.dump()==original);}
 std::string nine;for(unsigned i=0;i<9;++i){if(i)nine+=',';nine+=action(i,"pixaura.exposure","{\"milli_ev\":100}");}CHECK(owner.candidate(plan(owner,nine),result)!=0);
 for(const auto& from:std::array<std::string,3>{{"\"operation_version\":1","\"parameter_version\":1","\"plan_version\":1"}}){auto bad=good;const auto at=bad.find(from);CHECK(at!=std::string::npos);bad[at+from.size()-1]='2';CHECK(owner.candidate(bad,result)!=0);}
 CHECK(owner.candidate(good,result)==0);owner.manual();CHECK(pixaura_editor_auto_current(1,&owner.c,&result)==9);CHECK(owner.candidate(good,result)==9);
 CHECK(owner.request(0,result)==0);CHECK(pixaura_editor_action(&owner.c,2)==0);CHECK(pixaura_editor_auto_current(1,&owner.c,&result)==9);
 CHECK(owner.request(0,result)==0);CHECK(pixaura_editor_action(&owner.c,3)==0);CHECK(pixaura_editor_auto_current(1,&owner.c,&result)==9);
 for(uint32_t point=0;point<3;++point)for(unsigned event=0;event<6;++event){Owner o(root,++serial);if(event==2||event==3)o.manual();if(event==3)CHECK(pixaura_editor_action(&o.c,2)==0);
  const auto before=o.dump();CHECK(pixaura_test_auto_control(&o.c,0,point)==0);std::atomic<int> code{-1};pixaura_auto_proposal packet{};std::memset(&packet,0xa5,sizeof(packet));const auto sentinel=packet;
  std::thread run([&]{code=o.request(0,packet);});wait(o);pixaura_auto_proposal duplicate{};CHECK(o.request(0,duplicate)==8);
  const pixaura_assist_request ar{1,sizeof(ar),0,0};pixaura_assist_advice advice{};CHECK(pixaura_editor_assist_request(1,&o.c,&ar,&advice)==8);
  if(event==0)CHECK(pixaura_editor_auto_cancel(1,&o.c)==0);
  if(event==1)o.manual();
  if(event==2)CHECK(pixaura_editor_action(&o.c,2)==0);
  if(event==3)CHECK(pixaura_editor_action(&o.c,3)==0);
  if(event==4)CHECK(pixaura_editor_stop(&o.c)==0);
  CHECK(pixaura_test_auto_control(&o.c,2,0)==0);run.join();CHECK(code==(event==5?0:event==0||event==4?13:9));
  if(event!=5)CHECK(std::memcmp(&packet,&sentinel,sizeof(packet))==0);
  if(event==0||event==5)CHECK(o.dump()==before);
  if(event==4){CHECK(pixaura_editor_destroy(&o.c)==0);o.destroyed=true;CHECK(o.request(0,duplicate)==3);}
 }
 for(uint32_t fault=1;fault<=12;++fault){Owner o(root,++serial);const auto before=o.dump();CHECK(pixaura_test_auto_control(&o.c,3,fault)==0);CHECK(o.request(0,result)!=0);CHECK(o.dump()==before);o.manual();CHECK(o.dump()!=before);}
 {Owner o(root,++serial);CHECK(pixaura_test_assist_control(&o.c,0,1)==0);const pixaura_assist_request r{1,sizeof(r),0,0};pixaura_assist_advice advice{};std::atomic<int> status{-1};
  std::thread run([&]{status=pixaura_editor_assist_request(1,&o.c,&r,&advice);});const auto end=std::chrono::steady_clock::now()+std::chrono::seconds(5);
  while(!pixaura_test_assist_control(&o.c,1,0)&&std::chrono::steady_clock::now()<end)std::this_thread::yield();
  CHECK(pixaura_test_assist_control(&o.c,1,0)==1);CHECK(o.request(0,result)==8);CHECK(pixaura_editor_assist_cancel(1,&o.c)==0);CHECK(pixaura_test_assist_control(&o.c,2,0)==0);run.join();CHECK(status==13);
 }
 {Owner o(root,++serial);CHECK(pixaura_test_auto_control(&o.c,4,0)==0);CHECK(o.request(0,result)==8);o.manual();}
 {Owner o(root,++serial);const auto before=o.dump();CHECK(pixaura_test_auto_control(&o.c,4,1)==0);CHECK(pixaura_test_auto_control(&o.c,0,2)==0);
  std::atomic<int> code{-1};pixaura_auto_proposal packet{};std::thread run([&]{code=o.request(0,packet);});wait(o);
  CHECK(pixaura_editor_auto_cancel(1,&o.c)==0);CHECK(pixaura_test_auto_control(&o.c,2,0)==0);run.join();CHECK(code==13);CHECK(o.dump()==before);CHECK(o.request(0,packet)==8);
 }
 pixaura_auto_request request{1,sizeof(request),0,2};CHECK(pixaura_editor_auto_request(2,&owner.c,&request,nullptr,0,&result)==2);
 request.api_version=2;CHECK(pixaura_editor_auto_request(1,&owner.c,&request,nullptr,0,&result)==2);request.api_version=1;
 request.struct_size--;CHECK(pixaura_editor_auto_request(1,&owner.c,&request,nullptr,0,&result)==1);request.struct_size=sizeof(request);
 request.kind=1;CHECK(pixaura_editor_auto_request(1,&owner.c,&request,nullptr,0,&result)==1);request.kind=2;
 CHECK(pixaura_editor_auto_request(1,&owner.c,nullptr,nullptr,0,&result)==1);CHECK(pixaura_editor_auto_request(1,&owner.c,&request,nullptr,0,nullptr)==1);
 CHECK(pixaura_editor_auto_request(1,&owner.c,&request,nullptr,0,reinterpret_cast<pixaura_auto_proposal*>(&owner.c))==1);
 std::printf("Auto proposal v1: %u checks PASS; goldens=11, race_orderings=18; no approval/candidate/execution/history mutation\n",checks);
}
