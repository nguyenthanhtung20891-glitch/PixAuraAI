#include "pixaura/agent_execution.h"
#include "../src/document.hpp"
#include "../src/sha256.hpp"
#include "agent_schema_fixtures.hpp"
#include "../../../tests/fixtures/decode/fixtures.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <thread>
extern "C" int32_t pixaura_test_editor_document(pixaura_editor_context*,uint8_t*,uint64_t,uint64_t*);
extern "C" int32_t pixaura_test_agent_execution_checkpoint(pixaura_editor_context*,void (*)(uint32_t,void*),void*);
extern "C" int32_t pixaura_test_editor_observe(pixaura_editor_context*,void (*)(uint32_t,void*),void*);
static std::atomic<unsigned> checks{0};
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"agent execution %d %s\n",__LINE__,#x);std::abort();}}while(false)
static const uint8_t* b(const std::string& s){return reinterpret_cast<const uint8_t*>(s.data());}
static int64_t reader(void* p,uint8_t* out,uint64_t n){auto& at=*static_cast<std::size_t*>(p);n=std::min<uint64_t>(n,sizeof(decode_png)-at);std::memcpy(out,decode_png+at,static_cast<std::size_t>(n));at+=static_cast<std::size_t>(n);return static_cast<int64_t>(n);}
static pixaura_agent_execution_request request(){return {1,sizeof(pixaura_agent_execution_request),1,0,{0,0,0,0}};}
struct Owner {
 pixaura_editor_context c{};std::filesystem::path root;
 Owner(const std::filesystem::path& parent,unsigned serial,bool policy=true){char id[33];std::snprintf(id,sizeof(id),"%032x",serial);root=parent/(std::string("agent-execution-")+id);CHECK(std::filesystem::create_directory(root));
  const auto path=root.generic_string();std::size_t at=0;
  pixaura_capability_profile profile{};CHECK(pixaura_capability_baseline(1,&profile)==0);
  profile.hardware.logical_cpu_count=8;profile.hardware.physical_memory_bytes=UINT64_C(8589934592);
  profile.runtime={1,sizeof(pixaura_capability_dynamic),1,1,0,1,UINT64_C(536870912),1};
  const pixaura_policy_observation observation{1,sizeof(pixaura_policy_observation),1,1,0,1,0,0,UINT64_C(536870912),1};
  pixaura_execution_strategy strategy{};
  CHECK((policy?pixaura_editor_open_policy(1,&c,b(path),path.size(),reinterpret_cast<const uint8_t*>(id),32,reader,&at,sizeof(decode_png),&profile,&observation,&strategy):pixaura_editor_open(1,&c,b(path),path.size(),reinterpret_cast<const uint8_t*>(id),32,reader,&at,sizeof(decode_png)))==0);}
 ~Owner(){CHECK(pixaura_editor_stop(&c)==0);CHECK(pixaura_editor_destroy(&c)==0);}
 std::string document(){uint64_t n=0;CHECK(pixaura_test_editor_document(&c,nullptr,0,&n)==0);std::string s(static_cast<std::size_t>(n),' ');CHECK(pixaura_test_editor_document(&c,reinterpret_cast<uint8_t*>(s.data()),n,&n)==0);return s;}
 std::string binding(){uint64_t n=0;CHECK(pixaura_editor_agent_binding(1,&c,nullptr,0,&n)==11);std::string s(static_cast<std::size_t>(n),' ');CHECK(pixaura_editor_agent_binding(1,&c,reinterpret_cast<uint8_t*>(s.data()),n,&n)==0);s.pop_back();return s;}
 std::string plan(std::string fixture){const auto begin=fixture.find("\"binding\":")+10;const auto end=fixture.find('}',begin)+1;fixture.replace(begin,end-begin,binding());return fixture;}
 pixaura_agent_approval approve(const std::string& p){pixaura_agent_approval a{};const auto r=request();CHECK(pixaura_editor_agent_approve(1,&c,&r,b(p),p.size(),&a)==0);return a;}
 int execute(const std::string& p,const pixaura_agent_approval* a,pixaura_agent_execution_receipt* receipt=nullptr){const auto r=request();pixaura_agent_execution_receipt out;std::memset(&out,0xa5,sizeof(out));const auto sentinel=out;
  const auto code=pixaura_editor_agent_execute(1,&c,&r,a,b(p),p.size(),receipt?receipt:&out);
  if(code&&!receipt)CHECK(std::memcmp(&out,&sentinel,sizeof(out))==0);return code;}
 void release(const pixaura_agent_approval& a){CHECK(pixaura_editor_agent_release(1,&c,&a)==0);}
};
static std::string exposure(){return std::string("{\"actions\":[{\"action_index\":0,\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":100},\"tool_id\":\"pixaura.exposure\"}],\"binding\":{},\"plan_version\":1}\n");}
struct Hook{Owner* owner;uint32_t target;bool fail;std::atomic<bool> entered{false},resume{false};bool wait=false,allocation=false;};
static void checkpoint(uint32_t point,void* raw){auto& h=*static_cast<Hook*>(raw);if(point!=h.target)return;
 if(h.allocation)throw std::bad_alloc();
 if(h.fail)throw pixaura::document::Failure{7};
 if(h.wait){h.entered=true;while(!h.resume.load())std::this_thread::yield();}
 else {const auto code=pixaura_editor_agent_cancel(1,&h.owner->c);if(code)std::abort();}
}
static void later_action_failure(uint32_t point,void* raw){auto& count=*static_cast<unsigned*>(raw);
 // evaluation admission, first action, second action: fail inside the later
 // deterministic tool stage after earlier private pixel work has completed.
 if(point==0&&++count==3)throw pixaura::document::Failure{7};
}
int main(int argc,char** argv){CHECK(argc==2);const auto parent=std::filesystem::u8path(argv[1])/("agent-execution-suite-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));CHECK(std::filesystem::create_directory(parent));
 // Repo-owned Step 2 full canonical plans cover single, geometry, tone, detail,
 // repeated tools and eight actions. Bind only the live trusted native envelope.
 unsigned serial=70000;
 for(const auto& f:pixaura::agent_schema_fixtures::cases){
  if(f.kind!=2||f.status!=0)continue;
  Owner o(parent,++serial);const auto before=o.document();const auto p=o.plan(f.canonical);
  CHECK(o.execute(p,nullptr)==15);CHECK(o.document()==before);
  const auto a=o.approve(p);CHECK(o.document()==before);
  pixaura_agent_execution_receipt receipt{};CHECK(o.execute(p,&a,&receipt)==0);
  CHECK(receipt.api_version==1&&receipt.struct_size==sizeof(receipt)&&receipt.approval_consumed==1);
  pixaura::storage::Sha256 hash;hash.update(b(p),p.size());CHECK(std::memcmp(receipt.plan_sha256,hash.finish().data(),64)==0);
  auto after=o.document();CHECK(after!=before);auto parsed=pixaura::document::deserialize(after,std::string(32,'a'));CHECK(parsed.code==0);
  CHECK(parsed.value->revisions().size()==2&&parsed.value->revisions().back().actor=="ai");
  std::size_t cursor=0;for(const auto& op:parsed.value->operations()){
   const auto needle=std::string("\"tool_id\":\"")+op.type+"\"";const auto at=p.find(needle,cursor);CHECK(at!=std::string::npos);cursor=at+needle.size();
  }
  std::size_t action_count=0;for(std::size_t at=0;(at=p.find("\"action_index\":",at))!=std::string::npos;++at)++action_count;
  CHECK(parsed.value->operations().size()==action_count&&parsed.value->revisions().back().stack.size()==action_count);
  CHECK(o.execute(p,&a)==15&&o.document()==after);
  CHECK(pixaura_editor_action(&o.c,2)==0);auto undone=pixaura::document::deserialize(o.document(),std::string(32,'a'));CHECK(undone.code==0&&undone.value->current()==parsed.value->revisions().front().id);
  CHECK(pixaura_editor_action(&o.c,3)==0);CHECK(o.document()==after);
  CHECK(pixaura_editor_action(&o.c,2)==0);const auto branch=o.plan(exposure());const auto branch_approval=o.approve(branch);CHECK(o.execute(branch,&branch_approval)==0);
  auto branched=pixaura::document::deserialize(o.document(),std::string(32,'a'));CHECK(branched.code==0&&branched.value->revisions().size()==3&&branched.value->redo().empty());CHECK(pixaura_editor_action(&o.c,3)==10);
  CHECK(branched.value->source().sha256==parsed.value->source().sha256);
  const auto asset=o.root/"assets"/"sha256"/parsed.value->source().sha256.text().c_str();
  std::ifstream original(asset,std::ios::binary);const std::string original_bytes((std::istreambuf_iterator<char>(original)),std::istreambuf_iterator<char>());
  CHECK(original_bytes.size()==sizeof(decode_png)&&std::memcmp(original_bytes.data(),decode_png,sizeof(decode_png))==0);
  o.release(a);o.release(branch_approval);CHECK(pixaura_editor_agent_release(1,&o.c,&a)==3);
 }
 Owner o(parent,++serial),other(parent,++serial);auto p=o.plan(exposure());auto before=o.document();auto a=o.approve(p);
 CHECK(other.execute(p,&a)==3);auto forged=a;forged.serial=UINT64_MAX;CHECK(o.execute(p,&forged)==3);forged=a;forged.kind=0;CHECK(o.execute(p,&forged)==3);
 forged=a;forged.serial=0;CHECK(o.execute(p,&forged)==3);
 auto changed=p;changed.replace(changed.find("100}"),4,"101}");CHECK(o.execute(changed,&a)==15);CHECK(o.document()==before);
 auto wrong_version=p;wrong_version.replace(wrong_version.find("\"plan_version\":1"),16,"\"plan_version\":2");CHECK(o.execute(wrong_version,&a)==2);
 CHECK(pixaura_editor_agent_revoke(1,&o.c,&a)==0);CHECK(o.execute(p,&a)==15);CHECK(o.document()==before);o.release(a);CHECK(o.execute(p,&a)==3);
 // Bounded approval ownership: leases consume the original combined document /
 // gesture / proposal budget; no serial is chosen by caller or reused.
 std::array<pixaura_agent_approval,8> approvals{};for(auto& x:approvals)x=o.approve(p);
 const auto r=request();pixaura_agent_approval sentinel{};CHECK(pixaura_editor_agent_approve(1,&o.c,&r,b(p),p.size(),&sentinel)==8);
 for(const auto& x:approvals)o.release(x);
 a=o.approve(p);CHECK(a.serial>approvals.back().serial);o.release(a);
 // ABI, schema and forged model authority always fail closed with no document effects.
 auto bad=request();bad.api_version=2;CHECK(pixaura_editor_agent_approve(1,&o.c,&bad,b(p),p.size(),&sentinel)==2);
 bad=request();bad.registry_version=2;CHECK(pixaura_editor_agent_approve(1,&o.c,&bad,b(p),p.size(),&sentinel)==2);
 bad=request();bad.reserved[0]=1;CHECK(pixaura_editor_agent_approve(1,&o.c,&bad,b(p),p.size(),&sentinel)==1);
 bad=request();bad.pre_cancel=1;CHECK(pixaura_editor_agent_approve(1,&o.c,&bad,b(p),p.size(),&sentinel)==13);
 bad=request();bad.struct_size=0;CHECK(pixaura_editor_agent_approve(1,&o.c,&bad,b(p),p.size(),&sentinel)==1);
 for(const auto& invalid:{std::string(),std::string(8193,'x'),std::string("{\"approved\":true}\n"),std::string("{}\n"),p+"x"})CHECK(pixaura_editor_agent_approve(1,&o.c,&r,b(invalid),invalid.size(),&sentinel)!=0);
 uint64_t required=123;std::array<uint8_t,8192> out{};CHECK(pixaura_editor_agent_validate(1,&o.c,&r,b(p),p.size(),out.data(),1,&required)==11&&required==p.size());
 CHECK(pixaura_editor_agent_validate(1,&o.c,&r,b(p),p.size(),out.data(),out.size(),&required)==0);CHECK(std::memcmp(out.data(),p.data(),p.size())==0);
 CHECK(pixaura_editor_agent_validate(1,&o.c,&r,b(p),p.size(),const_cast<uint8_t*>(b(p)),p.size(),&required)==1);
 CHECK(pixaura_editor_agent_approve(1,&o.c,nullptr,b(p),p.size(),&sentinel)==1);
 CHECK(o.document()==before);
 const auto two=o.plan("{\"actions\":[{\"action_index\":0,\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":100},\"tool_id\":\"pixaura.exposure\"},{\"action_index\":1,\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":200},\"tool_id\":\"pixaura.exposure\"}],\"binding\":{},\"plan_version\":1}\n");
 auto reordered=two;const auto one=reordered.find("100}"),second=reordered.find("200}");reordered.replace(one,3,"200");reordered.replace(second,3,"100");
 a=o.approve(two);CHECK(o.execute(reordered,&a)==15&&o.document()==before);o.release(a);
 alignas(pixaura_agent_execution_receipt) std::array<uint8_t,sizeof(pixaura_agent_execution_receipt)+1> unaligned{};
 a=o.approve(p);CHECK(pixaura_editor_agent_execute(1,&o.c,&r,&a,b(p),p.size(),reinterpret_cast<pixaura_agent_execution_receipt*>(unaligned.data()+1))==1);o.release(a);
 // Phase 4 remains authoritative: lower cap denies approval/execution, three
 // accepted recovery observations restore eligibility, never consume on denial.
 a=o.approve(p);pixaura_execution_strategy strategy{};
 pixaura_policy_observation observation{1,sizeof(pixaura_policy_observation),1,1,0,1,0,0,UINT64_C(1048576),2};
 CHECK(pixaura_editor_policy_update(1,&o.c,&observation,&strategy)==0);
 CHECK(o.execute(p,&a)==8);CHECK(pixaura_editor_agent_approve(1,&o.c,&r,b(p),p.size(),&sentinel)==8&&o.document()==before);
 observation.process_budget_bytes=UINT64_C(536870912);
 for(unsigned stable=0;stable<3;++stable){++observation.sequence;CHECK(pixaura_editor_policy_update(1,&o.c,&observation,&strategy)==0);}
 CHECK(o.execute(p,&a)==0);o.release(a);CHECK(pixaura_editor_action(&o.c,2)==0);p=o.plan(exposure());before=o.document();
 Owner legacy(parent,++serial,false);const auto legacy_plan=legacy.plan(exposure());const auto legacy_before=legacy.document();
 CHECK(pixaura_editor_agent_approve(1,&legacy.c,&r,b(legacy_plan),legacy_plan.size(),&sentinel)==1);
 CHECK(pixaura_editor_select(&legacy.c,0)==0&&pixaura_editor_action(&legacy.c,0)==0&&legacy.document()==legacy_before);
 for(const auto& f:pixaura::agent_schema_fixtures::cases){if(f.kind==2&&f.status!=0){const auto invalid=o.plan(f.input);CHECK(pixaura_editor_agent_approve(1,&o.c,&r,b(invalid),invalid.size(),&sentinel)!=0);}}
 // Cancellation at admission, submitted work and pre-publication. Later fault
 // after private full-stack evaluation must leave history byte-identical.
 for(uint32_t point=0;point<4;++point){a=o.approve(p);Hook h{&o,point,false};CHECK(pixaura_test_agent_execution_checkpoint(&o.c,checkpoint,&h)==0);
  CHECK(o.execute(p,&a)==13);CHECK(o.document()==before);CHECK(pixaura_test_agent_execution_checkpoint(&o.c,nullptr,nullptr)==0);
  if(point==0)CHECK(o.execute(p,&a)==0);else CHECK(o.execute(p,&a)==15);
  o.release(a);if(point==0){CHECK(pixaura_editor_action(&o.c,2)==0);p=o.plan(exposure());before=o.document();}
 }
 a=o.approve(p);Hook failure{&o,2,true};CHECK(pixaura_test_agent_execution_checkpoint(&o.c,checkpoint,&failure)==0);CHECK(o.execute(p,&a)==7);CHECK(o.document()==before);CHECK(o.execute(p,&a)==15);o.release(a);CHECK(pixaura_test_agent_execution_checkpoint(&o.c,nullptr,nullptr)==0);
 a=o.approve(p);Hook oom{&o,1,false};oom.allocation=true;CHECK(pixaura_test_agent_execution_checkpoint(&o.c,checkpoint,&oom)==0);
 CHECK(o.execute(p,&a)==8&&o.document()==before);CHECK(o.execute(p,&a)==15);o.release(a);CHECK(pixaura_test_agent_execution_checkpoint(&o.c,nullptr,nullptr)==0);
 for(const auto& f:pixaura::agent_schema_fixtures::cases){if(f.kind!=2||f.status!=0)continue;auto multi=o.plan(f.canonical);
  if(multi.find("\"action_index\":1")==std::string::npos)continue;
  a=o.approve(multi);unsigned count=0;CHECK(pixaura_test_editor_observe(&o.c,later_action_failure,&count)==0);
  CHECK(o.execute(multi,&a)==7);CHECK(count==3&&o.document()==before);CHECK(o.execute(multi,&a)==15);
  CHECK(pixaura_test_editor_observe(&o.c,nullptr,nullptr)==0);o.release(a);
 }
 // Two callers compete for one approval; try-lock bounds contention, one commit.
 a=o.approve(p);std::atomic<bool> go{false};int c1=-1,c2=-1;
 std::thread t1([&]{while(!go.load())std::this_thread::yield();c1=o.execute(p,&a);});std::thread t2([&]{while(!go.load())std::this_thread::yield();c2=o.execute(p,&a);});go=true;t1.join();t2.join();
 CHECK((c1==0&&(c2==8||c2==15))||(c2==0&&(c1==8||c1==15)));o.release(a);
 // Admission has won before release/revocation: both wait for that same owner
 // lock, cannot invalidate borrowed memory or mint a second commit.
 for(bool release:{false,true}){
  p=o.plan(exposure());a=o.approve(p);Hook h{&o,1,false};h.wait=true;
  CHECK(pixaura_test_agent_execution_checkpoint(&o.c,checkpoint,&h)==0);
  int executed=-1,terminal=-1;std::atomic<bool> terminal_started{false};
  std::thread executing([&]{executed=o.execute(p,&a);});while(!h.entered.load())std::this_thread::yield();
  std::thread revoking([&]{terminal_started=true;terminal=release?pixaura_editor_agent_release(1,&o.c,&a):pixaura_editor_agent_revoke(1,&o.c,&a);});
  while(!terminal_started.load())std::this_thread::yield();h.resume=true;executing.join();revoking.join();CHECK(executed==0&&terminal==0);
  CHECK(pixaura_test_agent_execution_checkpoint(&o.c,nullptr,nullptr)==0);
  CHECK(o.execute(p,&a)==(release?3:15));if(!release)o.release(a);
 }
 // Changing trusted live revision after approval invalidates it without caller claims.
 p=o.plan(exposure());a=o.approve(p);CHECK(pixaura_editor_action(&o.c,2)==0);before=o.document();CHECK(o.execute(p,&a)==9);CHECK(o.document()==before);o.release(a);
 // Recovery and manual mode remain available independently of rejected agent work.
 CHECK(pixaura_editor_select(&o.c,0)==0);CHECK(pixaura_editor_action(&o.c,0)==0);
 p=o.plan(exposure());a=o.approve(p);const auto other_plan=other.plan(exposure());const auto other_approval=other.approve(other_plan);
 int independent1=-1,independent2=-1;
 std::thread independent_a([&]{independent1=o.execute(p,&a);});std::thread independent_b([&]{independent2=other.execute(other_plan,&other_approval);});independent_a.join();independent_b.join();
 CHECK(independent1==0&&independent2==0);o.release(a);other.release(other_approval);
 std::printf("agent execution: %u assertions, zero failures\n",checks.load());
}
