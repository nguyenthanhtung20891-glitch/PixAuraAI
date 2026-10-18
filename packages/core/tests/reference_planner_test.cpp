#include "pixaura/reference_planner.h"
#include "pixaura/agent_tools.h"
#include "../src/agent_schema_testing.hpp"
#include "agent_schema_fixtures.hpp"
#include "reference_planner_fixtures.hpp"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"planner line %d: %s\n",__LINE__,#x);std::abort();}}while(false)
static const uint8_t* b(const std::string& s){return reinterpret_cast<const uint8_t*>(s.data());}
static std::string change(std::string s,const std::string& from,const std::string& to){const auto p=s.find(from);CHECK(p!=s.npos);s.replace(p,from.size(),to);return s;}
static pixaura_reference_planner_request request(){pixaura_reference_planner_request r{};r.api_version=1;r.struct_size=sizeof(r);r.planner_version=1;r.registry_version=1;return r;}
struct Owner {
 pixaura_document_context c{};pixaura_document_handle h{};
 explicit Owner(const char* id){CHECK(pixaura_document_context_init(1,&c,sizeof(c),reinterpret_cast<const uint8_t*>(id),32,nullptr)==0);CHECK(pixaura_document_open(1,&c,b(pixaura::agent_schema_fixtures::document),std::strlen(pixaura::agent_schema_fixtures::document),b("00000000000000000000000000000500"),32,&h,nullptr)==0);}
 ~Owner(){CHECK(pixaura_document_context_destroy(&c)==0);}
 std::string dump(){std::array<uint8_t,8192> out{};uint64_t n=0;CHECK(pixaura_document_serialize(&c,&h,out.data(),out.size(),&n,nullptr)==0);return {reinterpret_cast<char*>(out.data()),static_cast<std::size_t>(n)};}
};
struct Hook {Owner* owner;unsigned target,seen=0,mode=0;};
static void checkpoint(void* state){
 auto& h=*static_cast<Hook*>(state);++h.seen;
 if(h.seen!=h.target){return;}
 if(h.mode==0){CHECK(pixaura_agent_schema_cancel(1,&h.owner->c)==0);}
 if(h.mode==1){CHECK(pixaura_document_release(&h.owner->c,&h.owner->h)==0);}
 if(h.mode==2){const auto r=request();uint64_t n=777;const std::string input=pixaura::reference_planner_fixtures::cases[0].input;CHECK(pixaura_reference_planner_plan(1,&h.owner->c,&h.owner->h,&r,b(input),input.size(),nullptr,0,&n)==8);CHECK(n==777);}
}
int main(){
 CHECK(pixaura_agent_schema_test_boundary()==0);
 Owner a("000000000000000000000000000000a1"),other("000000000000000000000000000000a2");const auto before=a.dump(),other_before=other.dump();
 auto r=request();uint32_t api=1;
 auto call=[&](const std::string& input,int expected,bool normalize=false){
  std::array<uint8_t,8192> output;output.fill(0x5a);uint64_t n=777;
  const auto result=normalize?pixaura_structured_intent_canonicalize(api,&a.c,&a.h,&r,b(input),input.size(),output.data(),output.size(),&n):pixaura_reference_planner_plan(api,&a.c,&a.h,&r,b(input),input.size(),output.data(),output.size(),&n);
  if(result!=expected){std::fprintf(stderr,"expected %d got %d: %.100s\n",expected,result,input.c_str());}
  CHECK(result==expected);
  if(result){CHECK(n==777);for(auto byte:output){CHECK(byte==0x5a);}}
 };
 for(const auto& f:pixaura::reference_planner_fixtures::cases){
  const std::string input=f.input,plan=f.plan;
  for(unsigned repeat=0;repeat<50;++repeat){
   std::array<uint8_t,8192> output{};uint64_t n=0;
   CHECK(pixaura_reference_planner_plan(1,&a.c,&a.h,&r,b(input),input.size(),output.data(),output.size(),&n)==0);
   CHECK(n==plan.size() && std::memcmp(output.data(),plan.data(),n)==0);
  }
  pixaura_agent_proposal proposal{};
  CHECK(pixaura_agent_stage(1,&a.c,&a.h,b(plan),plan.size(),&proposal)==6);
  CHECK(pixaura_agent_stage(1,&a.c,&a.h,reinterpret_cast<const uint8_t*>(f.stage),std::strlen(f.stage),&proposal)==0);
  CHECK(pixaura_agent_execution_authority(1,&a.c,&proposal,&a.h)==PIXAURA_AGENT_NOT_APPROVED);
  CHECK(pixaura_agent_release(1,&a.c,&proposal)==0);
 }
 for(const auto& f:pixaura::reference_planner_fixtures::negative){call(f.input,f.status);call(f.input,f.status,true);}
 const std::string valid=pixaura::reference_planner_fixtures::cases[0].input;
 pixaura_agent_schema_request legacy{};legacy.api_version=1;legacy.struct_size=sizeof(legacy);legacy.kind=1;legacy.require_canonical=1;
 std::array<uint8_t,8192> legacy_out{};uint64_t legacy_size=0;
 const auto& freeform=pixaura::reference_planner_fixtures::negative[41];
 CHECK(pixaura_agent_schema_canonicalize(1,&a.c,&a.h,&legacy,reinterpret_cast<const uint8_t*>(freeform.input),std::strlen(freeform.input),legacy_out.data(),legacy_out.size(),&legacy_size)==0);
 CHECK(legacy_size==std::strlen(freeform.input) && std::memcmp(legacy_out.data(),freeform.input,legacy_size)==0);
 CHECK(pixaura_agent_schema_canonicalize(1,&a.c,&a.h,&legacy,b(valid),valid.size(),legacy_out.data(),legacy_out.size(),&legacy_size)==6);
 call("",8);call(std::string(2049,' '),8);call("{",6);call(valid+"garbage",6);call(valid.substr(0,valid.size()-3),6);call(" "+valid,6);call(" "+valid,0,true);
 call(change(valid,"adjust.brightness",std::string(65,'a')),6);call(change(valid,"adjust.brightness",std::string("\xc0\xaf",2)),6);call(change(valid,"adjust.brightness","adjust.\\u0000brightness"),5);
 call(change(valid,"\"structured_intent_version\":1","\"structured_intent_version\":1,\"structured_intent_version\":1"),6);
 call(change(valid,"\"structured_intent_version\":1","\"structured_intent_version\":1,\"approved\":true"),6);
 call(change(valid,"\"structured_intent_version\":1","\"structured_intent_version\":1,\"operation_version\":2"),6);
 call(change(valid,"\"goal_id\":\"adjust.brightness\",",""),6);
 call(change(valid,"\"parameters\":{\"milli_linear\":0}","\"parameters\":{\"milli_linear\":{}}"),6);
 call(change(valid,"\"parameters\":{\"milli_linear\":0}","\"parameters\":{\"milli_linear\":-0}"),6);
 call(change(valid,"\"structured_intent_version\":1","\"structured_intent_version\":1,\"actions\":[0,1,2,3,4,5,6,7,8]"),6);
 api=2;call(valid,2);api=1;r.api_version=2;call(valid,2);r=request();r.planner_version=2;call(valid,2);r=request();r.registry_version=2;call(valid,2);r=request();r.struct_size=31;call(valid,1);r=request();r.reserved[1]=1;call(valid,1);r=request();r.pre_cancel=2;call(valid,1);r=request();r.pre_cancel=1;call(valid,13);r=request();
 std::array<uint8_t,8192> out{};uint64_t n=777;
 CHECK(pixaura_reference_planner_plan(1,&other.c,&a.h,&r,b(valid),valid.size(),out.data(),out.size(),&n)==3);
 CHECK(pixaura_reference_planner_plan(1,&other.c,&other.h,&r,b(valid),valid.size(),out.data(),out.size(),&n)==9);
 CHECK(pixaura_reference_planner_plan(1,&a.c,&a.h,&r,b(valid),valid.size(),nullptr,0,&n)==11);CHECK(n==std::strlen(pixaura::reference_planner_fixtures::cases[0].plan));
 CHECK(pixaura_reference_planner_plan(1,&a.c,&a.h,&r,b(valid),valid.size(),out.data(),1,&n)==11);CHECK(out[0]==0);
 CHECK(pixaura_reference_planner_plan(1,&a.c,&a.h,&r,b(valid),valid.size(),const_cast<uint8_t*>(b(valid)),valid.size(),&n)==1);
 CHECK(pixaura_reference_planner_plan(1,&a.c,&a.h,&r,b(valid),valid.size(),out.data(),UINT64_MAX,&n)==1);
 CHECK(pixaura_reference_planner_plan(1,&a.c,&a.h,&r,nullptr,valid.size(),out.data(),out.size(),&n)==1);
 CHECK(pixaura_reference_planner_plan(1,&a.c,&a.h,&r,b(valid),valid.size(),out.data(),out.size(),nullptr)==1);
 alignas(8) std::array<uint8_t,40> bad_request{};
 CHECK(pixaura_reference_planner_plan(1,&a.c,&a.h,reinterpret_cast<const pixaura_reference_planner_request*>(bad_request.data()+1),b(valid),valid.size(),out.data(),out.size(),&n)==1);
 Hook hook{&a,1};CHECK(pixaura_agent_schema_test_checkpoint(&a.c,checkpoint,&hook)==0);call(valid,13);
 hook={&a,2};call(valid,13);hook={&other,2};call(valid,0);hook={&a,1,0,2};call(valid,0);
 CHECK(pixaura_agent_schema_test_checkpoint(&a.c,nullptr,nullptr)==0);call(valid,0);
 bool ok[2]={false,false};std::thread threads[2];
 const std::string inputs[]={valid,change(valid,"000000000000000000000000000000a1","000000000000000000000000000000a2")};
 const std::string original_plan=pixaura::reference_planner_fixtures::cases[0].plan;
 const std::string plans[]={original_plan,change(original_plan,"000000000000000000000000000000a1","000000000000000000000000000000a2")};
 for(unsigned i=0;i<2;++i){threads[i]=std::thread([&,i]{auto& owner=i?other:a;const auto& input=inputs[i];const auto& plan=plans[i];bool good=true;for(unsigned repeat=0;repeat<100;++repeat){std::array<uint8_t,8192> output{};uint64_t size=0;good=good&&pixaura_reference_planner_plan(1,&owner.c,&owner.h,&r,b(input),input.size(),output.data(),output.size(),&size)==0&&size==plan.size()&&std::memcmp(output.data(),plan.data(),size)==0;}ok[i]=good;});}
 for(auto& t:threads){t.join();}CHECK(ok[0]&&ok[1]);
 CHECK(a.dump()==before);CHECK(other.dump()==other_before);
 hook={&a,2,0,1};CHECK(pixaura_agent_schema_test_checkpoint(&a.c,checkpoint,&hook)==0);call(valid,9);CHECK(pixaura_agent_schema_test_checkpoint(&a.c,nullptr,nullptr)==0);call(valid,3);
 std::printf("Reference planner v1 checks=%u failures=0 positives=39 negatives=42 repeats=50; exact bytes; cancellation/context PASS; side_effects=0; NOT_APPROVED\n",checks);
}
