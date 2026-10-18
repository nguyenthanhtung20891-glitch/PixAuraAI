#include "pixaura/agent_tools.h"
#include "../src/manual_tools.hpp"
#include "../src/agent_schema_testing.hpp"
#include "agent_schema_fixtures.hpp"
#include "agent_tools_fixture.hpp"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"tools line %d: %s\n",__LINE__,#x);std::abort();}}while(false)
static const uint8_t* b(const std::string& s){return reinterpret_cast<const uint8_t*>(s.data());}
static std::string change(std::string s,const std::string& from,const std::string& to){auto p=s.find(from);CHECK(p!=s.npos);s.replace(p,from.size(),to);return s;}
struct Owner{
 pixaura_document_context c{};pixaura_document_handle h{};
 explicit Owner(const char* id){CHECK(pixaura_document_context_init(1,&c,sizeof(c),reinterpret_cast<const uint8_t*>(id),32,nullptr)==0);CHECK(pixaura_document_open(1,&c,b(pixaura::agent_schema_fixtures::document),std::strlen(pixaura::agent_schema_fixtures::document),b("00000000000000000000000000000500"),32,&h,nullptr)==0);}
 ~Owner(){CHECK(pixaura_document_context_destroy(&c)==0);}
 std::string dump(){std::array<uint8_t,8192> out{};uint64_t n=0;CHECK(pixaura_document_serialize(&c,&h,out.data(),out.size(),&n,nullptr)==0);return {reinterpret_cast<char*>(out.data()),static_cast<std::size_t>(n)};}
};
struct Hook {Owner* owner;unsigned mode;};
static void checkpoint(void* p){
 auto& h=*static_cast<Hook*>(p);
 if(h.mode==1)CHECK(pixaura_agent_schema_cancel(1,&h.owner->c)==0);
 if(h.mode==2)CHECK(pixaura_document_release(&h.owner->c,&h.owner->h)==0);
 if(h.mode==3){
  pixaura_agent_schema_request r{};r.api_version=1;r.struct_size=sizeof(r);r.kind=2;r.require_canonical=1;
  const std::string input=pixaura::agent_schema_fixtures::cases[1].canonical;uint64_t n=777;
  CHECK(pixaura_agent_tools_bind(1,1,&h.owner->c,&h.owner->h,&r,b(input),input.size(),nullptr,0,&n)==8);CHECK(n==777);
 }
}
int main(){
 Owner o("000000000000000000000000000000a1"),other("000000000000000000000000000000a2");const auto before=o.dump(),other_before=other.dump();
 std::array<uint8_t,32768> registry{};uint64_t n=0;
 CHECK(pixaura_agent_tools_registry(1,1,nullptr,0,&n)==11);CHECK(n==std::strlen(agent_tools_golden));
 CHECK(pixaura_agent_tools_registry(1,1,registry.data(),registry.size(),&n)==0);CHECK(std::memcmp(registry.data(),agent_tools_golden,n)==0);
 const auto registry_copy=registry;
 CHECK(pixaura_agent_tools_registry(2,1,registry.data(),registry.size(),&n)==2);
 CHECK(pixaura_agent_tools_registry(1,0,registry.data(),registry.size(),&n)==2);
 CHECK(pixaura_agent_tools_registry(1,1,registry.data(),registry.size(),reinterpret_cast<uint64_t*>(registry.data()))==1);
 CHECK(pixaura_agent_tools_registry(1,1,nullptr,1,&n)==1);
 CHECK(pixaura_agent_tools_registry(1,1,registry.data(),registry.size(),nullptr)==1);
 CHECK(registry==registry_copy);
 pixaura_agent_schema_request r{};r.api_version=1;r.struct_size=sizeof(r);r.kind=2;r.require_canonical=1;
 uint32_t api=1,version=1;
 auto call=[&](const std::string& input,int expected){
  std::array<uint8_t,9216> out;out.fill(0x5a);uint64_t size=777;
  const auto result=pixaura_agent_tools_bind(api,version,&o.c,&o.h,&r,b(input),input.size(),out.data(),out.size(),&size);
  if(result!=expected){std::fprintf(stderr,"expected %d got %d: %.160s\n",expected,result,input.c_str());}
  CHECK(result==expected);
  if(result){CHECK(size==777);for(auto c:out)CHECK(c==0x5a);}else{
   const auto receipt=std::string("{\"approval\":\"not_approved\",\"binding_version\":1,\"plan\":")+input.substr(0,input.size()-1)+",\"registry_version\":1}\n";
   CHECK(size==receipt.size());CHECK(std::memcmp(out.data(),receipt.data(),size)==0);CHECK(out[size]==0x5a);
   pixaura_agent_proposal proposal{};
   CHECK(pixaura_agent_stage(1,&o.c,&o.h,out.data(),size,&proposal)==6);
   CHECK(proposal.serial==0); // a binding receipt cannot become execution authority
  }
 };
 for(const auto& f:pixaura::agent_schema_fixtures::cases)if(f.kind==2)call(f.canonical,f.status);
 const std::string valid=pixaura::agent_schema_fixtures::cases[1].canonical;
 call("",8);call("{",6);call(std::string(8193,' '),8);call(" "+valid,6);call(valid+"x",6);
 for(const auto& id:{"pixaura.unknown", "cpu.reference.v1", "android.vulkan.v1", "apple.metal.v1", "pixaura.preset"})call(change(valid,"pixaura.exposure",id),5);
 call(change(valid,"pixaura.exposure",""),6);
 call(change(valid,"pixaura.exposure","pixaura.background_removal"),6);
 call(change(valid,"pixaura.exposure",std::string(65,'a')),6);
 call(change(valid,"pixaura.exposure","pixaura.\\u0000exposure"),6);
 call(change(valid,"pixaura.exposure",std::string("\xc0\xaf",2)),6);
 call(change(valid,"\"operation_version\":1","\"operation_version\":2"),5);
 call(change(valid,"\"parameter_version\":1","\"parameter_version\":2"),5);
 call(change(valid,"\"milli_ev\":1000","\"milli_ev\":-5001"),7);call(change(valid,"\"milli_ev\":1000","\"milli_ev\":5001"),7);
 call(change(valid,"\"milli_ev\":1000","\"milli_ev\":9223372036854775808"),6);
 call(change(valid,"\"milli_ev\":1000","\"milli_ev\":1.0"),6);
 call(change(valid,"\"milli_ev\":1000","\"milli_ev\":true"),7);
 call(change(valid,"\"milli_ev\":1000","\"milli_ev\":\"1000\""),7);
 call(change(valid,"\"milli_ev\":1000","\"milli_ev\":0,\"milli_ev\":1000"),6);
 call(change(valid,"\"milli_ev\":1000","\"milli_ev\":1000,\"unknown\":0"),7);
 call(change(valid,"\"milli_ev\":1000",""),7);
 call(change(valid,"\"milli_ev\":1000","\"milli_ev\\u0000\":1000"),7);
 call(change(valid,"\"action_index\":0","\"action_index\":1"),6);
 call(change(valid,"\"generation\":0","\"generation\":1"),9);
 call(change(valid,"\"plan_version\":1","\"plan_version\":2"),2);
 call(change(valid,"\"plan_version\":1","\"plan_version\":1,\"approved\":true"),6);
 // Every descriptor's exact integer edges, defaults and rejection edges.
 for(const auto& d:pixaura::manual::registry()){
  CHECK(pixaura::manual::agent_exposable(d));
  auto denied=d;denied.tool_id="pixaura.test.only";denied.type=denied.tool_id;CHECK(!pixaura::manual::agent_exposable(denied));denied=d;denied.operation_version=2;CHECK(!pixaura::manual::agent_exposable(denied));
  for(std::size_t k=0;k<d.parameter_count;++k){
   const auto& p=d.parameters[k];
   for(int64_t value:{static_cast<int64_t>(p.minimum)-1,static_cast<int64_t>(p.minimum),static_cast<int64_t>(p.neutral),static_cast<int64_t>(p.maximum),static_cast<int64_t>(p.maximum)+1}){
    std::string parameters="{";
    for(std::size_t j=0;j<d.parameter_count;++j){if(j)parameters+=',';auto v=j==k?value:static_cast<int64_t>(d.parameters[j].neutral);
     // Crop edge values must satisfy the existing relational extent rules.
     if(d.tool_id=="pixaura.crop" && j!=k && ((p.name=="x_ppm"&&d.parameters[j].name=="width_ppm")||(p.name=="y_ppm"&&d.parameters[j].name=="height_ppm")))v=1;
     parameters+='"';parameters+=d.parameters[j].name;parameters+="\":"+std::to_string(v);
    }parameters+='}';
    auto plan=change(valid,"pixaura.exposure",std::string(d.tool_id));plan=change(plan,"{\"milli_ev\":1000}",parameters);
    call(plan,value<p.minimum||value>p.maximum?7:0);
   }
  }
 }
 api=2;call(valid,2);api=1;version=2;call(valid,2);version=1;
 r.api_version=2;call(valid,2);r.api_version=1;r.struct_size=31;call(valid,1);r.struct_size=sizeof(r);
 r.reserved[0]=1;call(valid,1);r.reserved[0]=0;r.kind=1;call(valid,1);r.kind=2;
 r.require_canonical=0;call(valid,1);r.require_canonical=1;r.pre_cancel=1;call(valid,13);r.pre_cancel=0;
 std::array<uint8_t,9216> out{};uint64_t size=777;
 CHECK(pixaura_agent_tools_bind(1,1,&other.c,&o.h,&r,b(valid),valid.size(),out.data(),out.size(),&size)==3);
 CHECK(pixaura_agent_tools_bind(1,1,&other.c,&other.h,&r,b(valid),valid.size(),out.data(),out.size(),&size)==9);
 CHECK(pixaura_agent_tools_bind(1,1,&o.c,&o.h,&r,b(valid),valid.size(),nullptr,0,&size)==11);CHECK(size>valid.size());
 CHECK(pixaura_agent_tools_bind(1,1,&o.c,&o.h,&r,b(valid),valid.size(),out.data(),1,&size)==11);CHECK(out[0]==0);
 CHECK(pixaura_agent_tools_bind(1,1,&o.c,&o.h,&r,b(valid),valid.size(),const_cast<uint8_t*>(b(valid)),valid.size(),&size)==1);
 CHECK(pixaura_agent_tools_bind(1,1,&o.c,&o.h,&r,b(valid),valid.size(),out.data(),UINT64_MAX,&size)==1);
 CHECK(pixaura_agent_tools_bind(1,1,&o.c,&o.h,&r,nullptr,valid.size(),out.data(),out.size(),&size)==1);
 Hook hook{&o,1};CHECK(pixaura_agent_schema_test_checkpoint(&o.c,checkpoint,&hook)==0);call(valid,13);
 hook={&other,1};call(valid,0); // another context's cancellation cannot revoke this binding
 hook={&o,3};call(valid,0); // nested work rejects BUSY as resource, no queue
 CHECK(pixaura_agent_schema_test_checkpoint(&o.c,nullptr,nullptr)==0);call(valid,0);
 // Immutable enumeration is safe to interleave without a registration lock.
 bool threads_ok[2]={false,false};std::thread threads[2];
 for(unsigned i=0;i<2;++i)threads[i]=std::thread([&,i]{std::array<uint8_t,32768> data{};uint64_t count=0;threads_ok[i]=pixaura_agent_tools_registry(1,1,data.data(),data.size(),&count)==0&&count==std::strlen(agent_tools_golden)&&std::memcmp(data.data(),agent_tools_golden,count)==0;});
 for(auto& t:threads){t.join();}
 CHECK(threads_ok[0]&&threads_ok[1]);
 CHECK(o.dump()==before);CHECK(other.dump()==other_before);
 hook={&o,2};CHECK(pixaura_agent_schema_test_checkpoint(&o.c,checkpoint,&hook)==0);call(valid,9);
 CHECK(pixaura_agent_schema_test_checkpoint(&o.c,nullptr,nullptr)==0);call(valid,3);CHECK(pixaura_document_release(&o.c,&o.h)==3);
 std::printf("Agent tools v1 checks=%u failures=0; registry=11 exact; binding/security/order/isolation PASS; side_effects=0; NOT_APPROVED\n",checks);
}
