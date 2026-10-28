#include "pixaura/review_preview.h"
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
#include <fstream>
#include <iterator>
#include <string>
#include <thread>
#include <vector>
extern "C" int32_t pixaura_test_editor_document(pixaura_editor_context*,uint8_t*,uint64_t,uint64_t*);
extern "C" int32_t pixaura_test_agent_candidate_state(pixaura_editor_context*,uint32_t*,uint32_t*);
extern "C" int32_t pixaura_test_agent_durable(pixaura_editor_context*,uint8_t*,uint64_t,uint64_t*);
extern "C" int32_t pixaura_test_agent_external_session(pixaura_editor_context*);
extern "C" int32_t pixaura_test_auto_control(pixaura_editor_context*,uint32_t,uint32_t);
extern "C" int32_t pixaura_test_review_preview_control(pixaura_editor_context*,uint32_t,uint32_t);
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"review preview %d: %s\n",__LINE__,#x);std::abort();}}while(false)
static const uint8_t* b(const std::string& s){return reinterpret_cast<const uint8_t*>(s.data());}
struct Input{const std::vector<uint8_t>& bytes;std::size_t offset=0;};
static int64_t read(void* raw,uint8_t* out,uint64_t count){auto& r=*static_cast<Input*>(raw);count=std::min<uint64_t>(count,r.bytes.size()-r.offset);std::memcpy(out,r.bytes.data()+r.offset,static_cast<std::size_t>(count));r.offset+=static_cast<std::size_t>(count);return static_cast<int64_t>(count);}
struct Owner{
 pixaura_editor_context c{};bool destroyed=false;
 Owner(const std::filesystem::path& parent,unsigned serial,const std::vector<uint8_t>& image){char id[33];std::snprintf(id,sizeof(id),"%032x",serial);const auto dir=parent/id;CHECK(std::filesystem::create_directory(dir));const auto root=dir.generic_string();Input input{image};
  pixaura_capability_profile profile{};CHECK(pixaura_capability_baseline(1,&profile)==0);profile.hardware.logical_cpu_count=8;profile.hardware.physical_memory_bytes=UINT64_C(8589934592);
  profile.runtime={1,sizeof(pixaura_capability_dynamic),1,1,0,1,UINT64_C(536870912),1};
  const pixaura_policy_observation observation{1,sizeof(observation),1,1,0,1,0,0,UINT64_C(536870912),1};pixaura_execution_strategy strategy{};
  CHECK(pixaura_editor_open_policy(1,&c,b(root),root.size(),reinterpret_cast<uint8_t*>(id),32,read,&input,image.size(),&profile,&observation,&strategy)==0);}
 ~Owner(){if(!destroyed){CHECK(pixaura_editor_stop(&c)==0);CHECK(pixaura_editor_destroy(&c)==0);}}
 std::string dump(){uint64_t n=0;CHECK(pixaura_test_editor_document(&c,nullptr,0,&n)==0);std::string s(static_cast<std::size_t>(n),' ');CHECK(pixaura_test_editor_document(&c,reinterpret_cast<uint8_t*>(s.data()),n,&n)==0);return s;}
 std::string durable(){uint64_t n=0;CHECK(pixaura_test_agent_durable(&c,nullptr,0,&n)==0);std::string s(static_cast<std::size_t>(n),' ');CHECK(pixaura_test_agent_durable(&c,reinterpret_cast<uint8_t*>(s.data()),n,&n)==0);return s;}
 pixaura_auto_proposal proposal(unsigned index){pixaura_auto_proposal p{};const pixaura_auto_request r{1,sizeof(r),index,2};CHECK(pixaura_editor_auto_request(1,&c,&r,nullptr,0,&p)==0);return p;}
 std::string plan(const pixaura_auto_proposal& p){uint64_t n=0;CHECK(pixaura_editor_auto_plan(1,&c,&p,nullptr,0,&n)==11);std::string s(static_cast<std::size_t>(n),' ');CHECK(pixaura_editor_auto_plan(1,&c,&p,reinterpret_cast<uint8_t*>(s.data()),n,&n)==0);return s;}
 std::string candidate_bytes(const std::string& actions){uint64_t n=0;CHECK(pixaura_editor_agent_binding(1,&c,nullptr,0,&n)==11);std::string binding(static_cast<std::size_t>(n),' ');CHECK(pixaura_editor_agent_binding(1,&c,reinterpret_cast<uint8_t*>(binding.data()),n,&n)==0);binding.pop_back();return "{\"actions\":["+actions+"],\"binding\":"+binding+",\"plan_version\":1}\n";}
 int candidate(const std::string& input,pixaura_auto_proposal& p){const pixaura_auto_request r{1,sizeof(r),0,3};return pixaura_editor_auto_request(1,&c,&r,b(input),input.size(),&p);}
 pixaura_review_preview_ticket begin(const pixaura_auto_proposal& p,const std::string& s){pixaura_review_preview_ticket t{};CHECK(pixaura_review_preview_begin(1,&c,&p,b(s),s.size(),512,512,&t)==0);return t;}
 pixaura_review_preview_pair render(const pixaura_review_preview_ticket& t){pixaura_review_preview_pair p{};CHECK(pixaura_review_preview_render(1,&c,&t,&p)==0);return p;}
 std::vector<uint8_t> image(const pixaura_review_preview_pair& p,uint32_t side){const auto n=side?p.after.image_bytes:p.before.image_bytes;std::vector<uint8_t> out(static_cast<std::size_t>(n));CHECK(pixaura_review_preview_copy(1,&c,&p,side,0,out.data(),n)==0);return out;}
 std::vector<uint8_t> ordinary(){pixaura_editor_request r{};CHECK(pixaura_editor_request_preview(&c,&r)==0);std::vector<uint8_t> out(512*512*4);uint32_t w=0,h=0;CHECK(pixaura_editor_render(&c,&r,512,512,out.data(),out.size(),&w,&h)==0);out.resize(static_cast<std::size_t>(w)*h*4);return out;}
 void manual(){CHECK(pixaura_editor_select(&c,4)==0);const int32_t value=500;CHECK(pixaura_editor_update(&c,&value,1)==0);CHECK(pixaura_editor_action(&c,1)==0);}
 void apply(const std::string& s){const pixaura_agent_execution_request r{1,sizeof(r),1,0,{0,0,0,0}};pixaura_agent_approval a{};pixaura_agent_execution_receipt receipt{};
  CHECK(pixaura_editor_agent_approve(1,&c,&r,b(s),s.size(),&a)==0);CHECK(pixaura_editor_agent_execute(1,&c,&r,&a,b(s),s.size(),&receipt)==0);CHECK(receipt.approval_consumed==1);CHECK(pixaura_editor_agent_release(1,&c,&a)==0);}
};
static void wait(Owner& o){const auto end=std::chrono::steady_clock::now()+std::chrono::seconds(5);while(!pixaura_test_review_preview_control(&o.c,1,0)&&std::chrono::steady_clock::now()<end)std::this_thread::yield();CHECK(pixaura_test_review_preview_control(&o.c,1,0)==1);}
static std::string action(unsigned i,const char* tool,const char* parameters){return "{\"action_index\":"+std::to_string(i)+",\"operation_version\":1,\"parameter_version\":1,\"parameters\":"+parameters+",\"tool_id\":\""+tool+"\"}";}
int main(int argc,char** argv){CHECK(argc==3);const auto root=std::filesystem::u8path(argv[1])/("review-suite-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));CHECK(std::filesystem::create_directory(root));
 const std::vector<uint8_t> fixture(decode_png,decode_png+sizeof(decode_png));unsigned serial=880000;
 for(uint32_t tool=0;tool<11;++tool){Owner o(root,++serial,fixture);o.manual();const auto source=o.dump();const auto before=o.ordinary();const auto proposal=o.proposal(tool);const auto plan=o.plan(proposal);const auto ticket=o.begin(proposal,plan);
  const auto pair=o.render(ticket);const auto projected=o.image(pair,1);CHECK(o.image(pair,0)==before);CHECK(o.dump()==source);
  CHECK(pixaura_test_auto_control(&o.c,5,0)==0);uint32_t state=99,active=99;CHECK(pixaura_test_agent_candidate_state(&o.c,&state,&active)==0);CHECK(state==0&&active==0);
  CHECK(pair.owned_bytes==pair.before.image_bytes+pair.after.image_bytes);CHECK(pair.admission_peak_bytes>=pair.owned_bytes);
  CHECK(pixaura_review_preview_release(1,&o.c,&pair)==0);CHECK(pixaura_test_review_preview_control(&o.c,5,0)==0);CHECK(pixaura_review_preview_current(1,&o.c,&pair)==0);
  std::array<uint8_t,4> buffer{};CHECK(pixaura_review_preview_copy(1,&o.c,&pair,0,0,buffer.data(),4)==3);
  o.apply(plan);CHECK(pixaura_review_preview_current(1,&o.c,&pair)!=0);CHECK(o.ordinary()==projected);
 }
 Owner o(root,++serial,fixture),other(root,++serial,fixture);o.manual();
 auto proposal=o.proposal(2);auto plan=o.plan(proposal);auto ticket=o.begin(proposal,plan);auto pair=o.render(ticket);const auto source=o.dump();const auto pixels=o.image(pair,1);
 for(unsigned repeat=0;repeat<8;++repeat){const auto previous=ticket;ticket=o.begin(proposal,plan);CHECK(pixaura_review_preview_cancel(1,&o.c,&previous)==3);pair=o.render(ticket);CHECK(o.image(pair,1)==pixels);CHECK(o.dump()==source);}
 auto forged=pair;forged.after.width++;CHECK(pixaura_review_preview_current(1,&o.c,&forged)==3);CHECK(pixaura_review_preview_current(1,&other.c,&pair)==3);
 CHECK(pixaura_review_preview_cancel(1,&o.c,&ticket)==0);CHECK(pixaura_review_preview_current(1,&o.c,&pair)==13);CHECK(o.dump()==source);
 // Deterministic request/cancel/manual/undo/redo/mode/stop orderings, no sleeps.
 for(uint32_t scenario=0;scenario<8;++scenario){Owner race(root,++serial,fixture);race.manual();const auto p=race.proposal(2);
  const auto s=race.plan(p);const auto t=race.begin(p,s);const auto prior=race.dump();CHECK(pixaura_test_review_preview_control(&race.c,0,scenario%3)==0);
  pixaura_review_preview_pair result{};std::memset(&result,0xa5,sizeof(result));const auto sentinel=result;int status=0;
  std::thread worker([&]{status=pixaura_review_preview_render(1,&race.c,&t,&result);});wait(race);
  pixaura_review_preview_ticket fresh{};CHECK(pixaura_review_preview_begin(1,&race.c,&p,b(s),s.size(),512,512,&fresh)==8);
  if(scenario==0)CHECK(pixaura_review_preview_cancel(1,&race.c,&t)==0);
  if(scenario==1)race.manual();
  if(scenario==2)CHECK(pixaura_editor_action(&race.c,2)==0);
  if(scenario==3){CHECK(pixaura_editor_action(&race.c,2)==0);CHECK(pixaura_editor_action(&race.c,3)==0);}
  if(scenario==4)CHECK(pixaura_editor_auto_cancel(1,&race.c)==0);
  if(scenario==5)CHECK(pixaura_editor_stop(&race.c)==0);
  if(scenario==7)CHECK(pixaura_test_agent_external_session(&race.c)==0);
  CHECK(pixaura_test_review_preview_control(&race.c,2,0)==0);worker.join();
  if(scenario==6){CHECK(status==0);CHECK(pixaura_review_preview_cancel(1,&race.c,&t)==0);CHECK(race.dump()==prior);}
  else{CHECK(status==9||status==13);CHECK(std::memcmp(&result,&sentinel,sizeof(result))==0);}
  if(scenario==0||scenario==4)CHECK(race.dump()==prior);
  if(scenario==5)CHECK(race.durable()==prior);
 }
 for(uint32_t fault=1;fault<=3;++fault){CHECK(pixaura_test_review_preview_control(&o.c,3,fault)==0);ticket=o.begin(proposal,plan);pixaura_review_preview_pair result{};CHECK(pixaura_review_preview_render(1,&o.c,&ticket,&result)==(fault==3?14:8));CHECK(o.dump()==source);CHECK(pixaura_test_review_preview_control(&o.c,5,0)==0);}
 CHECK(pixaura_test_review_preview_control(&o.c,3,0)==0);
 {
  Owner constrained(root,++serial,fixture);
  pixaura_policy_observation pressure{1,sizeof(pressure),1,1,0,1,0,0,UINT64_C(4194304),2};pixaura_execution_strategy strategy{};
  CHECK(pixaura_editor_policy_update(1,&constrained.c,&pressure,&strategy)==0);
  const auto p=constrained.proposal(2);const auto s=constrained.plan(p);const auto t=constrained.begin(p,s);
  const auto prior=constrained.dump();pixaura_review_preview_pair failed{};
  CHECK(pixaura_review_preview_render(1,&constrained.c,&t,&failed)==8);CHECK(constrained.dump()==prior);
  CHECK(pixaura_test_review_preview_control(&constrained.c,5,0)==0);
  constrained.manual();CHECK(constrained.dump()!=prior); // no eviction or AI dependency
 }
 CHECK(pixaura_review_preview_begin(2,&o.c,&proposal,b(plan),plan.size(),512,512,&ticket)==2);
 CHECK(pixaura_review_preview_begin(1,&o.c,&proposal,b(plan),8193,512,512,&ticket)==8);
 CHECK(pixaura_review_preview_begin(1,&o.c,&proposal,b(plan),plan.size(),1025,512,&ticket)==1);
 CHECK(pixaura_review_preview_begin(1,&o.c,&proposal,b(plan),plan.size()-1,512,512,&ticket)==9);
 {
  // Preview correlation bytes are neither an editor owner nor approval/candidate.
  pixaura_editor_context wrong{};std::memcpy(&wrong,&ticket,sizeof(wrong));uint8_t data[1024];uint64_t n=0;
  CHECK(pixaura_editor_state(&wrong,data,sizeof(data),&n)!=0);
  auto t=o.begin(proposal,plan);const auto v=o.render(t);std::array<uint8_t,4> data4{};
  CHECK(pixaura_review_preview_copy(1,&o.c,&v,2,0,data4.data(),4)==1);
  CHECK(pixaura_review_preview_copy(1,&o.c,&v,0,UINT64_MAX,data4.data(),4)!=0);
  CHECK(pixaura_review_preview_copy(1,&o.c,&v,0,0,reinterpret_cast<uint8_t*>(&t),4194305)!=0);
  CHECK(pixaura_review_preview_release(1,&o.c,&v)==0);
 }
 for(const unsigned count:{2u,8u}){Owner multi(root,++serial,fixture);std::string actions;for(unsigned i=0;i<count;++i){if(i)actions+=',';actions+=action(i,"pixaura.exposure","{\"milli_ev\":100}");}
  const auto input=multi.candidate_bytes(actions);pixaura_auto_proposal p{};CHECK(multi.candidate(input,p)==0);const auto t=multi.begin(p,input);const auto pair_value=multi.render(t);const auto projected=multi.image(pair_value,1);multi.apply(input);CHECK(multi.ordinary()==projected);}
 {
  std::string actions;for(unsigned i=0;i<9;++i){if(i)actions+=',';actions+=action(i,"pixaura.exposure","{\"milli_ev\":100}");}
  pixaura_auto_proposal p{};CHECK(o.candidate(o.candidate_bytes(actions),p)!=0);
  CHECK(o.candidate("{}\n",p)==6);
  CHECK(o.candidate(o.candidate_bytes(action(0,"unknown","{\"milli_ev\":100}")),p)==5);
  CHECK(o.candidate(o.candidate_bytes(action(0,"pixaura.exposure","{\"milli_ev\":5001}")),p)==7);
 }
 std::ifstream stream(std::filesystem::u8path(argv[2]),std::ios::binary);CHECK(stream.good());const std::vector<uint8_t> large{std::istreambuf_iterator<char>(stream),std::istreambuf_iterator<char>()};
 for(unsigned kind=0;kind<3;++kind){Owner bench(root,++serial,large);pixaura_auto_proposal p{};std::string input;
  if(kind==0){p=bench.proposal(2);input=bench.plan(p);}
  else {const auto actions=kind==1? action(0,"pixaura.exposure","{\"milli_ev\":100}")+","+action(1,"pixaura.contrast","{\"milli_stops\":250}"):
    action(0,"pixaura.crop","{\"height_ppm\":800000,\"width_ppm\":800000,\"x_ppm\":100000,\"y_ppm\":100000}")+","+action(1,"pixaura.rotate","{\"quarter_turns\":1}")+","+action(2,"pixaura.brightness","{\"milli_linear\":100}");
    input=bench.candidate_bytes(actions);CHECK(bench.candidate(input,p)==0);}
  std::vector<uint8_t> expected;std::array<long long,6> times{};uint64_t peak=0,owned=0;long long begin_ns=0;
  for(std::size_t repeat=0;repeat<times.size();++repeat){const auto start=std::chrono::steady_clock::now();const auto t=bench.begin(p,input);const auto begun=std::chrono::steady_clock::now();const auto v=bench.render(t);const auto end=std::chrono::steady_clock::now();
    times[repeat]=std::chrono::duration_cast<std::chrono::nanoseconds>(end-begun).count();begin_ns+=std::chrono::duration_cast<std::chrono::nanoseconds>(begun-start).count();
    const auto pixels_value=bench.image(v,1);if(repeat)CHECK(pixels_value==expected);else expected=pixels_value;
    peak=v.admission_peak_bytes;owned=v.owned_bytes;CHECK(pixaura_review_preview_release(1,&bench.c,&v)==0);CHECK(pixaura_test_review_preview_control(&bench.c,5,0)==0);
  }
  const auto first=times[0];std::sort(times.begin()+1,times.end());bench.apply(input);CHECK(bench.ordinary()==expected);
  std::printf("Review preview benchmark kind=%u source=640x480 fit_edge=512 request_mean_ns=%lld first_render_ns=%lld warm_n=5 warm_median_ns=%lld warm_p95_ns=%lld admission_peak_bytes=%llu retained_native_pair_bytes=%llu post_release_native_bytes=0\n",kind,begin_ns/6,first,times[3],times[5],static_cast<unsigned long long>(peak),static_cast<unsigned long long>(owned));
 }
 std::printf("Review preview: %u checks; tools=11; exact approved-output equality; before=bound edited source; no preview approval/candidate/history authority\n",checks);
}
