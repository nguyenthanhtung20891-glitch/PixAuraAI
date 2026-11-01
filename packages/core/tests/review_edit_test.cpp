#include "pixaura/review_edit.h"
#include "../src/document.hpp"
#include "../src/review_edit_state.hpp"
#include "pixaura/review_inspection.h"
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

static std::string action(unsigned i,const char* tool,const char* parameters){return "{\"action_index\":"+std::to_string(i)+",\"operation_version\":1,\"parameter_version\":1,\"parameters\":"+parameters+",\"tool_id\":\""+tool+"\"}";}
static pixaura_review_inspection inspect(Owner& o,const pixaura_auto_proposal& p,const std::string& s){pixaura_review_inspection result{};CHECK(pixaura_review_inspect(1,&o.c,&p,b(s),s.size(),&result)==0);return result;}
static pixaura_review_edit_request request(const pixaura_review_edit_result& e,uint32_t kind){pixaura_review_edit_request q{};q.api_version=1;q.struct_size=sizeof(q);q.operation=kind;q.ticket=e.ticket;return q;}
extern "C" int32_t pixaura_test_agent_execution_checkpoint(pixaura_editor_context*,void (*)(uint32_t,void*),void*);
struct EditPause { std::atomic<bool> reached{false},resume{false}; };
static void edit_pause(uint32_t point,void* raw){if(point!=42)return;auto& p=*static_cast<EditPause*>(raw);p.reached=true;while(!p.resume.load())std::this_thread::yield();}
int main(int argc,char** argv){CHECK(argc==2);const auto root=std::filesystem::u8path(argv[1])/("edit-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));CHECK(std::filesystem::create_directory(root));
 const std::vector<uint8_t> fixture(decode_png,decode_png+sizeof(decode_png));unsigned serial=940000;
 for(unsigned tool=0;tool<11;++tool){Owner o(root,++serial,fixture);const auto p=o.proposal(tool);const auto original=o.plan(p);const auto r=inspect(o,p,original);pixaura_review_edit_result e{};CHECK(pixaura_review_edit_begin(1,&o.c,&r,&e)==0);const auto before=o.dump();
  const auto pair=o.render(o.begin(p,original));const auto& parameter=r.actions[0].parameters[0];
  auto q=request(e,1);q.raw_value=parameter.raw_value==parameter.minimum?parameter.maximum:parameter.minimum;
  if(tool==9)q.raw_value=700000;
  q.expected_action=r.actions[0];std::memcpy(q.tool_id,r.actions[0].tool_id,32);std::memcpy(q.parameter_name,parameter.name,32);
  pixaura_review_edit_result next{};CHECK(pixaura_review_edit(1,&o.c,&q,&next)==0);CHECK(next.ticket.serial!=e.ticket.serial);CHECK(next.review.request_serial!=p.request_serial);
  const auto edited=o.plan(next.review);const auto d=inspect(o,next.review,edited);CHECK(d.actions[0].parameters[0].raw_value==q.raw_value);
  CHECK(pixaura_review_inspection_current(1,&o.c,&r)!=0);CHECK(pixaura_review_preview_current(1,&o.c,&pair)!=0);CHECK(pixaura_editor_auto_current(1,&o.c,&p)!=0);CHECK(o.dump()==before);
  CHECK(pixaura_test_auto_control(&o.c,5,0)==0);uint32_t state=99,active=99;CHECK(pixaura_test_agent_candidate_state(&o.c,&state,&active)==0);CHECK(!state&&!active);
  auto restore=request(next,4);CHECK(pixaura_review_edit(1,&o.c,&restore,&e)==0);CHECK(e.ticket.serial!=next.ticket.serial);CHECK(o.plan(e.review)==original);CHECK(pixaura_review_inspection_current(1,&o.c,&r)!=0);CHECK(pixaura_review_preview_current(1,&o.c,&pair)!=0);
  const auto fresh=inspect(o,e.review,o.plan(e.review));auto bad=q;bad.ticket=e.ticket;bad.raw_value=INT32_MAX;const auto sentinel=next;
  CHECK(pixaura_review_edit(1,&o.c,&bad,&next)==7);CHECK(std::memcmp(&sentinel,&next,sizeof(next))==0);CHECK(pixaura_review_inspection_current(1,&o.c,&fresh)!=0);
  CHECK(pixaura_review_edit_current(1,&o.c,&e.ticket,&next)==0&&next.state==4);restore=request(next,4);CHECK(pixaura_review_edit(1,&o.c,&restore,&e)==0);
  auto remove=request(e,2);remove.expected_action=r.actions[0];std::memcpy(remove.tool_id,r.actions[0].tool_id,32);
  CHECK(pixaura_review_edit(1,&o.c,&remove,&next)==0&&next.state==3);CHECK(pixaura_editor_auto_current(1,&o.c,&e.review)!=0);CHECK(o.dump()==before);
  restore=request(next,4);CHECK(pixaura_review_edit(1,&o.c,&restore,&e)==0);CHECK(o.plan(e.review)==original);
 }
 for(unsigned count:{1u,2u,8u}){Owner o(root,++serial,fixture);std::string actions;for(unsigned i=0;i<count;++i){if(i)actions+=',';const auto parameters="{\"milli_ev\":"+std::to_string(100+i)+"}";actions+=action(i,"pixaura.exposure",parameters.c_str());}
  const auto bytes=o.candidate_bytes(actions);pixaura_auto_proposal p{};CHECK(o.candidate(bytes,p)==0);auto r=inspect(o,p,bytes);pixaura_review_edit_result e{};CHECK(pixaura_review_edit_begin(1,&o.c,&r,&e)==0);
  if(count>1){auto forged=request(e,1);forged.action_index=1;forged.raw_value=123;forged.expected_action=r.actions[0];std::memcpy(forged.tool_id,r.actions[0].tool_id,32);std::memcpy(forged.parameter_name,r.actions[0].parameters[0].name,32);pixaura_review_edit_result next{};
   CHECK(pixaura_review_edit(1,&o.c,&forged,&next)==3);auto restore=request(e,4);CHECK(pixaura_review_edit(1,&o.c,&restore,&next)==0);e=next;r=inspect(o,e.review,o.plan(e.review));
  }
  for(const auto positions:{std::pair<uint32_t,uint32_t>{0,count-1},{count-1,0}}){auto q=request(e,3);q.action_index=positions.first;q.destination=positions.second;q.expected_action=r.actions[q.action_index];std::memcpy(q.tool_id,r.actions[q.action_index].tool_id,32);
   pixaura_review_edit_result next{};const auto value=r.actions[q.action_index].parameters[0].raw_value;CHECK(pixaura_review_edit(1,&o.c,&q,&next)==0);e=next;r=inspect(o,e.review,o.plan(e.review));CHECK(r.actions[q.destination].parameters[0].raw_value==value);
  }
  CHECK(o.plan(e.review)==bytes);
  for(uint32_t kind=1;kind<=4;++kind){int64_t elapsed=0;
   for(unsigned repeat=0;repeat<20;++repeat){auto reset=request(e,4);pixaura_review_edit_result next{};CHECK(pixaura_review_edit(1,&o.c,&reset,&next)==0);e=next;r=inspect(o,e.review,o.plan(e.review));
    auto q=request(e,kind);
    if(kind!=4){q.expected_action=r.actions[0];std::memcpy(q.tool_id,r.actions[0].tool_id,32);if(kind==1){std::memcpy(q.parameter_name,r.actions[0].parameters[0].name,32);q.raw_value=-123;}if(kind==3)q.destination=count-1;}
    const auto begin=std::chrono::steady_clock::now();CHECK(pixaura_review_edit(1,&o.c,&q,&next)==0);elapsed+=std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()-begin).count();e=next;
   }
   std::printf("Edit benchmark actions=%u operation=%u repeats=20 mean_ns=%lld request_bytes=%zu result_bytes=%zu image_bytes=0\n",count,kind,static_cast<long long>(elapsed/20),sizeof(pixaura_review_edit_request),sizeof(pixaura_review_edit_result));
  }
 }
 {Owner o(root,++serial,fixture);const auto p=o.proposal(9);const auto r=inspect(o,p,o.plan(p));pixaura_review_edit_result e{},next{};CHECK(pixaura_review_edit_begin(1,&o.c,&r,&e)==0);
  auto q=request(e,1);q.parameter_index=2;q.raw_value=150000;q.expected_action=r.actions[0];std::memcpy(q.tool_id,r.actions[0].tool_id,32);std::memcpy(q.parameter_name,r.actions[0].parameters[2].name,32);
  CHECK(pixaura_review_edit(1,&o.c,&q,&next)==0);CHECK(inspect(o,next.review,o.plan(next.review)).actions[0].parameters[2].raw_value==150000);
  q.ticket=next.ticket;q.expected_action=inspect(o,next.review,o.plan(next.review)).actions[0];q.raw_value=300000;CHECK(pixaura_review_edit(1,&o.c,&q,&e)!=0);CHECK(pixaura_editor_auto_current(1,&o.c,&next.review)!=0);
 }
 {Owner o(root,++serial,fixture);const auto bytes=o.candidate_bytes(action(0,"pixaura.crop","{\"height_ppm\":800000,\"width_ppm\":800000,\"x_ppm\":100000,\"y_ppm\":100000}")+","+action(1,"pixaura.rotate","{\"quarter_turns\":1}")+","+action(2,"pixaura.brightness","{\"milli_linear\":123}"));
  pixaura_auto_proposal p{};CHECK(o.candidate(bytes,p)==0);auto r=inspect(o,p,bytes);pixaura_review_edit_result e{},out{};CHECK(pixaura_review_edit_begin(1,&o.c,&r,&e)==0);
  auto q=request(e,2);q.expected_action=r.actions[0];std::memcpy(q.tool_id,r.actions[0].tool_id,32);CHECK(pixaura_review_edit(1,&o.c,&q,&out)==0);r=inspect(o,out.review,o.plan(out.review));CHECK(r.action_count==2);CHECK(std::string(r.actions[0].tool_id)=="pixaura.rotate");CHECK(r.actions[1].parameters[0].raw_value==123);
  q=request(out,3);q.action_index=1;q.destination=0;q.expected_action=r.actions[1];std::memcpy(q.tool_id,r.actions[1].tool_id,32);CHECK(pixaura_review_edit(1,&o.c,&q,&e)==0);r=inspect(o,e.review,o.plan(e.review));CHECK(std::string(r.actions[0].tool_id)=="pixaura.brightness");
  q=request(e,4);CHECK(pixaura_review_edit(1,&o.c,&q,&out)==0);CHECK(o.plan(out.review)==bytes);
 }
 for(unsigned scenario=0;scenario<13;++scenario){Owner o(root,++serial,fixture);const auto p=o.proposal(2);const auto bytes=o.plan(p);const auto r=inspect(o,p,bytes);pixaura_review_edit_result e{};CHECK(pixaura_review_edit_begin(1,&o.c,&r,&e)==0);
  auto q=request(e,1);q.raw_value=101;q.expected_action=r.actions[0];std::memcpy(q.tool_id,r.actions[0].tool_id,32);std::memcpy(q.parameter_name,r.actions[0].parameters[0].name,32);
  if(scenario==0)q.action_index=8;if(scenario==1)q.parameter_index=4;if(scenario==2)q.tool_id[0]='X';if(scenario==3)q.parameter_name[0]='X';
  if(scenario==4)q.operation=999;if(scenario==5)q.reserved=1;if(scenario==6)q.api_version=2;if(scenario==7)q.ticket.serial++;
  if(scenario==8)o.manual();if(scenario==9){o.manual();CHECK(pixaura_editor_action(&o.c,2)==0);}if(scenario==10)CHECK(pixaura_test_agent_external_session(&o.c)==0);
  if(scenario==11)CHECK(pixaura_editor_auto_cancel(1,&o.c)==0);if(scenario==12)CHECK(pixaura_editor_stop(&o.c)==0);
  const auto before=scenario==12?std::string{}:o.dump();pixaura_review_edit_result output{};std::memset(&output,0xa5,sizeof(output));const auto sentinel=output;
  CHECK(pixaura_review_edit(1,&o.c,&q,&output)!=0);CHECK(std::memcmp(&output,&sentinel,sizeof(output))==0);if(scenario!=12)CHECK(o.dump()==before);
 }
 {Owner o(root,++serial,fixture);Owner other(root,++serial,fixture);const auto p=o.proposal(2);const auto r=inspect(o,p,o.plan(p));pixaura_review_edit_result e{},out{};CHECK(pixaura_review_edit_begin(1,&o.c,&r,&e)==0);auto q=request(e,4);
  const auto other_proposal=other.proposal(2);const auto other_description=inspect(other,other_proposal,other.plan(other_proposal));pixaura_review_edit_result other_session{};CHECK(pixaura_review_edit_begin(1,&other.c,&other_description,&other_session)==0);
  CHECK(pixaura_review_edit(1,&other.c,&q,&out)==9);
  CHECK(pixaura_editor_auto_current(1,&other.c,&other_proposal)==0);
  EditPause pause;CHECK(pixaura_test_agent_execution_checkpoint(&o.c,edit_pause,&pause)==0);int code=-1;std::thread first([&]{code=pixaura_review_edit(1,&o.c,&q,&out);});while(!pause.reached.load())std::this_thread::yield();
  pixaura_review_edit_result competing{};CHECK(pixaura_review_edit(1,&o.c,&q,&competing)==8);pause.resume=true;first.join();CHECK(code==0);CHECK(pixaura_test_agent_execution_checkpoint(&o.c,nullptr,nullptr)==0);
 }
 {Owner o(root,++serial,fixture);const auto p=o.proposal(2);const auto bytes=o.plan(p);const auto r=inspect(o,p,bytes);pixaura_review_edit_result e{},out{};CHECK(pixaura_review_edit_begin(1,&o.c,&r,&e)==0);
  const pixaura_agent_execution_request execution{1,sizeof(execution),1,0,{0,0,0,0}};pixaura_agent_approval approval{};CHECK(pixaura_editor_agent_approve(1,&o.c,&execution,b(bytes),bytes.size(),&approval)==0);
  auto q=request(e,1);q.raw_value=-250;q.expected_action=r.actions[0];std::memcpy(q.tool_id,r.actions[0].tool_id,32);std::memcpy(q.parameter_name,r.actions[0].parameters[0].name,32);
  const auto before=o.dump();CHECK(pixaura_review_edit(1,&o.c,&q,&out)==0);pixaura_agent_execution_receipt receipt{};
  CHECK(pixaura_editor_agent_execute(1,&o.c,&execution,&approval,b(bytes),bytes.size(),&receipt)==15);CHECK(pixaura_editor_agent_release(1,&o.c,&approval)==0);CHECK(o.dump()==before);
  const auto edited=o.plan(out.review);o.apply(edited);const auto committed=o.dump();CHECK(committed!=before);
  CHECK(committed.find("\"milli_ev\":-250")!=std::string::npos);CHECK(committed.find("\"milli_ev\":500")==std::string::npos);CHECK(pixaura_review_edit_current(1,&o.c,&out.ticket,&e)!=0);
  CHECK(pixaura_editor_action(&o.c,2)==0);CHECK(pixaura_editor_action(&o.c,3)==0);
 }
 {Owner o(root,++serial,fixture);const auto p=o.proposal(2);const auto r=inspect(o,p,o.plan(p));pixaura_review_edit_result e{},out{};CHECK(pixaura_review_edit_begin(1,&o.c,&r,&e)==0);auto q=request(e,4);
  EditPause pause;CHECK(pixaura_test_agent_execution_checkpoint(&o.c,edit_pause,&pause)==0);int code=-1,stopped=-1,destroyed=-1;
  std::thread call([&]{code=pixaura_review_edit(1,&o.c,&q,&out);});while(!pause.reached.load())std::this_thread::yield();
  std::atomic<bool> stopping{false};std::thread teardown([&]{stopping=true;stopped=pixaura_editor_stop(&o.c);call.join();destroyed=pixaura_editor_destroy(&o.c);});
  while(!stopping.load())std::this_thread::yield();pause.resume=true;teardown.join();CHECK(code==0&&stopped==0&&destroyed==0);o.destroyed=true;
  CHECK(pixaura_review_edit_current(1,&o.c,&out.ticket,&e)!=0);
 }
 std::printf("Review edit ownership state_bytes=%zu original_plan_bound=8192 existing_current_plan_bound=8192 review_undo_entries=0 image_bytes=0\n",sizeof(pixaura::ReviewEditState));
 std::printf("Review edits: %u checks; eleven tools; immutable old review; fresh restore; zero history authority\n",checks);
}
