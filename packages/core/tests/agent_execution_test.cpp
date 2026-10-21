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
extern "C" int32_t pixaura_test_agent_candidate_state(pixaura_editor_context*,uint32_t*,uint32_t*);
extern "C" void pixaura_test_agent_storage_failure();
extern "C" int32_t pixaura_test_agent_epoch_exhaust(pixaura_editor_context*);
extern "C" int32_t pixaura_test_agent_durable(pixaura_editor_context*,uint8_t*,uint64_t,uint64_t*);
extern "C" int32_t pixaura_test_agent_external_session(pixaura_editor_context*);
extern "C" int32_t pixaura_test_agent_platform_control(pixaura_editor_context*,uint32_t,uint32_t);
static std::atomic<unsigned> checks{0};
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"agent execution %d %s\n",__LINE__,#x);std::abort();}}while(false)
static const uint8_t* b(const std::string& s){return reinterpret_cast<const uint8_t*>(s.data());}
static int64_t reader(void* p,uint8_t* out,uint64_t n){auto& at=*static_cast<std::size_t*>(p);n=std::min<uint64_t>(n,sizeof(decode_png)-at);std::memcpy(out,decode_png+at,static_cast<std::size_t>(n));at+=static_cast<std::size_t>(n);return static_cast<int64_t>(n);}
static pixaura_agent_execution_request request(){return {1,sizeof(pixaura_agent_execution_request),1,0,{0,0,0,0}};}
static std::string replace(std::string s,const std::string& from,const std::string& to){const auto at=s.find(from);CHECK(at!=std::string::npos);s.replace(at,from.size(),to);return s;}
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
 std::string durable(){uint64_t n=0;CHECK(pixaura_test_agent_durable(&c,nullptr,0,&n)==0);std::string s(static_cast<std::size_t>(n),' ');
  CHECK(pixaura_test_agent_durable(&c,reinterpret_cast<uint8_t*>(s.data()),n,&n)==0);return s;}
 std::string binding(){uint64_t n=0;CHECK(pixaura_editor_agent_binding(1,&c,nullptr,0,&n)==11);std::string s(static_cast<std::size_t>(n),' ');CHECK(pixaura_editor_agent_binding(1,&c,reinterpret_cast<uint8_t*>(s.data()),n,&n)==0);s.pop_back();return s;}
 std::string plan(std::string fixture){const auto begin=fixture.find("\"binding\":")+10;const auto end=fixture.find('}',begin)+1;fixture.replace(begin,end-begin,binding());return fixture;}
 pixaura_agent_approval approve(const std::string& p){pixaura_agent_approval a{};const auto r=request();CHECK(pixaura_editor_agent_approve(1,&c,&r,b(p),p.size(),&a)==0);return a;}
 int execute(const std::string& p,const pixaura_agent_approval* a,pixaura_agent_execution_receipt* receipt=nullptr){const auto r=request();pixaura_agent_execution_receipt out;std::memset(&out,0xa5,sizeof(out));const auto sentinel=out;
  const auto code=pixaura_editor_agent_execute(1,&c,&r,a,b(p),p.size(),receipt?receipt:&out);
  if(code&&!receipt){CHECK(std::memcmp(&out,&sentinel,sizeof(out))==0);}
  return code;}
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
 for(uint32_t point:{1u,3u}){Owner o(parent,++serial);const auto before=o.document();const auto p=o.plan(exposure());auto a=o.approve(p);
  CHECK(pixaura_test_agent_platform_control(&o.c,0,point)==0);std::atomic<int> result{-1};
  std::thread worker([&]{result=o.execute(p,&a);});
  const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(10);
  while(pixaura_test_agent_platform_control(&o.c,1,0)==0&&std::chrono::steady_clock::now()<deadline)std::this_thread::yield();
  CHECK(pixaura_test_agent_platform_control(&o.c,1,0)==1);
  CHECK(pixaura_editor_agent_cancel(1,&o.c)==0);
  CHECK(pixaura_test_agent_platform_control(&o.c,2,0)==0);worker.join();
  CHECK(result==13);CHECK(o.document()==before);CHECK(o.execute(p,&a)==15);o.release(a);
 }
 {Owner o(parent,++serial);const auto before=o.document();
  const auto input=std::string("{\"binding\":")+o.binding()+",\"goal_id\":\"adjust.exposure\",\"parameters\":{\"milli_ev\":100},\"structured_intent_version\":1}\n";
  pixaura_reference_planner_request r{1,sizeof(r),1,1,0,{0,0,0}};
  std::array<uint8_t,8192> out{};uint64_t n=0;
  CHECK(pixaura_editor_agent_plan(1,&o.c,&r,b(input),input.size(),nullptr,0,&n)==11);
  CHECK(n==o.plan(exposure()).size());
  CHECK(pixaura_editor_agent_plan(1,&o.c,&r,b(input),input.size(),out.data(),out.size(),&n)==0);
  CHECK(std::string(reinterpret_cast<char*>(out.data()),static_cast<std::size_t>(n))==o.plan(exposure()));
  CHECK(o.execute(o.plan(exposure()),nullptr)==15);CHECK(o.document()==before);
  CHECK(pixaura_editor_agent_plan(2,&o.c,&r,b(input),input.size(),out.data(),out.size(),&n)==2);
  r.registry_version=2;CHECK(pixaura_editor_agent_plan(1,&o.c,&r,b(input),input.size(),out.data(),out.size(),&n)==2);r.registry_version=1;
  CHECK(pixaura_editor_agent_plan(1,&o.c,&r,b(input),input.size(),const_cast<uint8_t*>(b(input)),input.size(),&n)==1);
  CHECK(pixaura_editor_agent_plan(1,&o.c,nullptr,b(input),input.size(),out.data(),out.size(),&n)==1);
  const auto wrong=replace(input,"adjust.exposure","adjust.unlisted");
  CHECK(pixaura_editor_agent_plan(1,&o.c,&r,b(wrong),wrong.size(),out.data(),out.size(),&n)==5);
  CHECK(o.document()==before);
 }
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
 for(const auto& invalid:{replace(p,"pixaura.exposure","pixaura.unlisted"),replace(p,"\"operation_version\":1","\"operation_version\":2"),
     replace(p,"\"parameter_version\":1","\"parameter_version\":2"),replace(p,"100}","5001}"),replace(p,"milli_ev","unknown"),
     replace(p,"{\"milli_ev\":100}","{}"),replace(p,"100}","true}"),replace(p,"{\"milli_ev\":100}","{\"milli_ev\":100,\"milli_ev\":100}"),
     replace(p,"\"plan_version\":1","\"approved\":true,\"plan_version\":1"),replace(p,"100}","9223372036854775808}")}){
  CHECK(pixaura_editor_agent_approve(1,&o.c,&r,b(invalid),invalid.size(),&sentinel)!=0);
 }
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
 for(uint32_t point:{4u,0u,1u,2u,3u}){a=o.approve(p);Hook h{&o,point,false};CHECK(pixaura_test_agent_execution_checkpoint(&o.c,checkpoint,&h)==0);
  CHECK(o.execute(p,&a)==13);CHECK(o.document()==before);CHECK(pixaura_test_agent_execution_checkpoint(&o.c,nullptr,nullptr)==0);
  if(point==0||point==4)CHECK(o.execute(p,&a)==0);else CHECK(o.execute(p,&a)==15);
  o.release(a);if(point==0||point==4){CHECK(pixaura_editor_action(&o.c,2)==0);p=o.plan(exposure());before=o.document();}
 }
 a=o.approve(p);Hook failure{&o,2,true};CHECK(pixaura_test_agent_execution_checkpoint(&o.c,checkpoint,&failure)==0);CHECK(o.execute(p,&a)==7);CHECK(o.document()==before);CHECK(o.execute(p,&a)==15);o.release(a);CHECK(pixaura_test_agent_execution_checkpoint(&o.c,nullptr,nullptr)==0);
 a=o.approve(p);Hook oom{&o,1,false};oom.allocation=true;CHECK(pixaura_test_agent_execution_checkpoint(&o.c,checkpoint,&oom)==0);
 CHECK(o.execute(p,&a)==8&&o.document()==before);CHECK(o.execute(p,&a)==15);o.release(a);CHECK(pixaura_test_agent_execution_checkpoint(&o.c,nullptr,nullptr)==0);
 // More failures than the unchanged 64-handle ceiling prove temporary raster /
 // cancellation ownership is released, not merely reclaimed at context teardown.
 for(unsigned repeat=0;repeat<72;++repeat){a=o.approve(p);Hook h{&o,2,true};
  CHECK(pixaura_test_agent_execution_checkpoint(&o.c,checkpoint,&h)==0);CHECK(o.execute(p,&a)==7);
  o.release(a);CHECK(pixaura_test_agent_execution_checkpoint(&o.c,nullptr,nullptr)==0);
 }CHECK(o.document()==before);
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
  while(!terminal_started.load()){std::this_thread::yield();}
  h.resume=true;executing.join();revoking.join();CHECK(executed==0&&terminal==0);
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

 // Step 6: private READY-stage work yields owner authority to manual edits,
 // undo/redo ABA, cancellation, release and stop. No sleep-based race assertions.
 for(unsigned scenario=0;scenario<7;++scenario){
  Owner life(parent,++serial);auto seed=life.plan(exposure());auto seed_approval=life.approve(seed);
  CHECK(life.execute(seed,&seed_approval)==0);life.release(seed_approval);
  if(scenario==2)CHECK(pixaura_editor_action(&life.c,2)==0);
  auto plan=life.plan(exposure());auto approved=life.approve(plan);auto competing_approval=life.approve(plan);
  const auto initial=life.document();CHECK(life.durable()==initial);Hook h{&life,2,false};h.wait=true;
  CHECK(pixaura_test_agent_execution_checkpoint(&life.c,checkpoint,&h)==0);
  int outcome=-1;std::thread work([&]{outcome=life.execute(plan,&approved);});
  while(!h.entered.load())std::this_thread::yield();
  uint32_t state=99,active=99;CHECK(pixaura_test_agent_candidate_state(&life.c,&state,&active)==0&&state==2&&active==1);
  CHECK(life.document()==initial);CHECK(life.execute(plan,&competing_approval)==8);
  if(scenario==0){
   const int32_t value=200;CHECK(pixaura_editor_select(&life.c,4)==0);
   CHECK(pixaura_editor_update(&life.c,&value,1)==0);CHECK(pixaura_editor_action(&life.c,1)==0);
  }else if(scenario==1){CHECK(pixaura_editor_action(&life.c,2)==0);CHECK(pixaura_editor_action(&life.c,3)==0);
  }else if(scenario==2){CHECK(pixaura_editor_action(&life.c,3)==0);
  }else if(scenario==3){CHECK(pixaura_editor_agent_cancel(1,&life.c)==0);
  }else if(scenario==4){CHECK(pixaura_editor_stop(&life.c)==0);
  }else if(scenario==5){life.release(approved);
  }else {CHECK(pixaura_editor_select(&life.c,4)==0);}
  const auto winner=scenario==4?initial:life.document();h.resume=true;work.join();
  CHECK(outcome==(scenario<=2||scenario==6?9:scenario<=4?13:0));
  CHECK(pixaura_test_agent_candidate_state(&life.c,&state,&active)==0&&active==0);
  CHECK(state==(scenario<=2||scenario==6?6u:scenario<=4?5u:4u));
  if(scenario!=5&&scenario!=4)CHECK(life.document()==winner);
  if(scenario!=5){const auto disk=life.durable();if(disk!=winner){
   std::size_t delta=0;while(delta<disk.size()&&delta<winner.size()&&disk[delta]==winner[delta])++delta;
   std::fprintf(stderr,"durable scenario=%u offset=%zu disk=%zu live=%zu disk-tail=%.96s live-tail=%.96s\n",scenario,delta,disk.size(),winner.size(),disk.c_str()+delta,winner.c_str()+delta);
  }CHECK(disk==winner);}
  if(scenario!=4){
   CHECK(pixaura_test_agent_execution_checkpoint(&life.c,nullptr,nullptr)==0);
   CHECK(life.execute(plan,&approved)==(scenario==5?3:15));
   if(scenario==5){CHECK(life.execute(plan,&competing_approval)==9);}
   if(scenario!=5){life.release(approved);}life.release(competing_approval);
   if(scenario==6)CHECK(pixaura_editor_action(&life.c,0)==0);
   // Nonsticky recovery: fresh human approval, no implicit replan/reapproval.
   auto fresh=life.plan(exposure());auto fresh_approval=life.approve(fresh);
   CHECK(life.execute(fresh,&fresh_approval)==0);life.release(fresh_approval);
   const auto published=life.document();CHECK(pixaura_editor_agent_cancel(1,&life.c)==0);
   CHECK(life.document()==published);
   CHECK(pixaura_test_agent_candidate_state(&life.c,&state,&active)==0&&state==4&&active==0);
  }
 }
 // Independent authoritative session acquisition wins repository CAS even when
 // this service's local live serial has not changed. Never publish stale work.
 {Owner life(parent,++serial);auto plan=life.plan(exposure());auto approved=life.approve(plan);
  Hook h{&life,2,false};h.wait=true;CHECK(pixaura_test_agent_execution_checkpoint(&life.c,checkpoint,&h)==0);
  int outcome=-1;std::thread work([&]{outcome=life.execute(plan,&approved);});
  while(!h.entered.load())std::this_thread::yield();
  CHECK(pixaura_test_agent_external_session(&life.c)==0);
  const auto authority=life.durable();h.resume=true;work.join();
  CHECK(outcome==9&&life.durable()==authority);
  uint32_t state=99,active=99;CHECK(pixaura_test_agent_candidate_state(&life.c,&state,&active)==0&&state==6&&active==0);
  CHECK(life.execute(plan,&approved)==15);life.release(approved);
 }
 // Transaction failure occurs after SQL changes and before COMMIT: rollback
 // preserves live state, and consumed authority cannot replay the failed plan.
 {Owner life(parent,++serial);auto plan=life.plan(exposure());auto approved=life.approve(plan);
  const auto initial=life.document();pixaura_test_agent_storage_failure();
  CHECK(life.execute(plan,&approved)==12&&life.document()==initial);CHECK(life.durable()==initial);
  CHECK(life.execute(plan,&approved)==15);life.release(approved);
  auto fresh=life.approve(plan);CHECK(life.execute(plan,&fresh)==0);life.release(fresh);
 }
 {Owner life(parent,++serial);auto plan=life.plan(exposure());auto approved=life.approve(plan);
  const auto initial=life.document();CHECK(pixaura_test_agent_epoch_exhaust(&life.c)==0);
  CHECK(life.execute(plan,&approved)==8);CHECK(pixaura_editor_agent_cancel(1,&life.c)==8);
  CHECK(life.document()==initial&&life.durable()==initial);life.release(approved);
 }
 std::printf("agent execution: %u assertions, zero failures\n",checks.load());
}
