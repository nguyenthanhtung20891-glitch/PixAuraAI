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
int main(int argc,char** argv){CHECK(argc==2);const auto root=std::filesystem::u8path(argv[1])/("inspection-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));CHECK(std::filesystem::create_directory(root));
 const std::vector<uint8_t> fixture(decode_png,decode_png+sizeof(decode_png));unsigned serial=930000;
 const char* tools[]={"pixaura.brightness","pixaura.contrast","pixaura.exposure","pixaura.highlights","pixaura.saturation","pixaura.shadows","pixaura.temperature","pixaura.blur","pixaura.sharpen","pixaura.crop","pixaura.rotate"};
 for(unsigned i=0;i<11;++i){Owner o(root,++serial,fixture);o.manual();const auto p=o.proposal(i);const auto s=o.plan(p);const auto before=o.dump();const auto r=inspect(o,p,s);
  CHECK(r.action_count==1&&r.registry_version==1&&r.kind==PIXAURA_REVIEW_INSPECTION_KIND);CHECK(r.actions[0].index==0);CHECK(std::string(r.actions[0].tool_id)==tools[i]);
  const char* names[]={"milli_linear","milli_stops","milli_ev","milli_ev","milli_ratio","milli_ev","kelvin","milli_strength","milli_amount","height_ppm","quarter_turns"};
  const char* units[]={"thousandths_linear_white","thousandths_log2_contrast_slope","thousandths_exposure_stop","thousandths_highlight_exposure_stop","thousandths_chroma_multiplier","thousandths_shadow_exposure_stop","daylight_white_kelvin","thousandths_fixed_kernel_blend","thousandths_fixed_kernel_unsharp","millionths_current_extent","clockwise_quarter_turns"};
  const int32_t low[]={-1000,-2000,-5000,-2000,0,-2000,4000,0,0,1,0},high[]={1000,2000,5000,2000,2000,2000,25000,1000,1000,1000000,3},neutral[]={0,0,0,0,1000,0,6504,0,0,1000000,0};
  const auto& a=r.actions[0];CHECK(a.parameter_count==(i==9?4u:1u));CHECK(a.operation_version==1&&a.parameter_version==1);CHECK(a.label[0]);
  for(unsigned j=0;j<a.parameter_count;++j){const auto& v=a.parameters[j];CHECK(v.raw_value==p.actions[0].values[j]);CHECK(std::string(v.unit)==units[i]);
   if(i!=9||j==0){CHECK(std::string(v.name)==names[i]);CHECK(v.minimum==low[i]&&v.maximum==high[i]&&v.neutral==neutral[i]);}
   else{const char* crop[]={"height_ppm","width_ppm","x_ppm","y_ppm"};CHECK(std::string(v.name)==crop[j]);CHECK(v.minimum==(j==1?1:0)&&v.maximum==(j==1?1000000:999999)&&v.neutral==(j==1?1000000:0));}}
  CHECK(a.geometry_changed==((i==9||i==10)?1u:0u));CHECK(pixaura_review_inspection_current(1,&o.c,&r)==0);
  for(unsigned repeat=0;repeat<10;++repeat){const auto again=inspect(o,p,s);CHECK(std::memcmp(&r,&again,sizeof(r))==0);}
  const auto pair=o.render(o.begin(p,s));CHECK(pixaura_review_inspection_preview(1,&o.c,&r,&pair)==0);CHECK(pixaura_review_preview_release(1,&o.c,&pair)==0);CHECK(pixaura_review_inspection_preview(1,&o.c,&r,&pair)==0);
  CHECK(o.dump()==before);CHECK(pixaura_test_auto_control(&o.c,5,0)==0);uint32_t state=99,active=99;CHECK(pixaura_test_agent_candidate_state(&o.c,&state,&active)==0);CHECK(!state&&!active);
  Owner other(root,++serial,fixture);const auto other_proposal=other.proposal(i);const auto other_description=inspect(other,other_proposal,other.plan(other_proposal));
  CHECK(std::memcmp(other_description.plan_sha256,r.plan_sha256,64)!=0);CHECK(pixaura_review_inspection_current(1,&other.c,&r)!=0);CHECK(pixaura_review_inspection_preview(1,&other.c,&r,&pair)!=0);
  auto forged=r;forged.actions[0].parameters[0].raw_value++;CHECK(pixaura_review_inspection_current(1,&o.c,&forged)==3);forged=r;forged.action_count=0;CHECK(pixaura_review_inspection_current(1,&o.c,&forged)==3);forged=r;forged.actions[0].label[0]='X';CHECK(pixaura_review_inspection_current(1,&o.c,&forged)==3);
  pixaura_review_inspection untouched{};std::memset(&untouched,0xa5,sizeof(untouched));const auto sentinel=untouched;
  CHECK(pixaura_review_inspect(1,&o.c,&p,b(s),s.size()-1,&untouched)!=0);CHECK(std::memcmp(&untouched,&sentinel,sizeof(untouched))==0);
  CHECK(pixaura_review_inspect(2,&o.c,&p,b(s),s.size(),&untouched)==2);CHECK(pixaura_review_inspect(1,&o.c,&p,b(s),8193,&untouched)==8);
  const auto newer=o.proposal((i+1)%11);const auto fresh=inspect(o,newer,o.plan(newer));CHECK(pixaura_review_inspection_current(1,&o.c,&r)!=0);CHECK(pixaura_review_inspection_preview(1,&o.c,&fresh,&pair)!=0);
 }
 for(unsigned scenario=0;scenario<6;++scenario){Owner o(root,++serial,fixture);o.manual();const auto p=o.proposal(2);const auto r=inspect(o,p,o.plan(p));
  if(scenario==0){o.manual();}
  if(scenario==1){CHECK(pixaura_editor_action(&o.c,2)==0);}
  if(scenario==2){CHECK(pixaura_editor_action(&o.c,2)==0);CHECK(pixaura_editor_action(&o.c,3)==0);}
  if(scenario==3){CHECK(pixaura_editor_auto_cancel(1,&o.c)==0);}
  if(scenario==4){CHECK(pixaura_test_agent_external_session(&o.c)==0);}
  if(scenario==5){CHECK(pixaura_editor_stop(&o.c)==0);}
  CHECK(pixaura_review_inspection_current(1,&o.c,&r)!=0);
 }
 for(unsigned count:{1u,8u}){Owner o(root,++serial,fixture);std::string actions;for(unsigned i=0;i<count;++i){if(i)actions+=',';actions+=action(i,i%2?"pixaura.brightness":"pixaura.exposure",i%2?"{\"milli_linear\":-123}":"{\"milli_ev\":101}");}
  const auto s=o.candidate_bytes(actions);pixaura_auto_proposal p{};CHECK(o.candidate(s,p)==0);const auto r=inspect(o,p,s);
  for(unsigned i=0;i<count;++i){CHECK(r.actions[i].index==i);CHECK(std::string(r.actions[i].tool_id)==(i%2?"pixaura.brightness":"pixaura.exposure"));CHECK(r.actions[i].parameters[0].raw_value==(i%2?-123:101));}
  if(count==8){auto forged=r;std::swap(forged.actions[0],forged.actions[1]);CHECK(pixaura_review_inspection_current(1,&o.c,&forged)==3);}
  const auto before=o.dump();const auto start=std::chrono::steady_clock::now();for(unsigned i=0;i<100;++i){const auto v=inspect(o,p,s);CHECK(std::memcmp(&v,&r,sizeof(v))==0);}
  const auto ns=std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()-start).count()/100;
  CHECK(o.dump()==before);std::printf("Inspection benchmark actions=%u repeats=100 mean_ns=%lld record_bytes=%zu retained_native_bytes=0 image_bytes=0\n",count,static_cast<long long>(ns),sizeof(r));
 }
 {Owner o(root,++serial,fixture);const auto p=o.proposal(2);const auto s=o.plan(p);const auto r=inspect(o,p,s);const auto t=o.begin(p,s);
  CHECK(pixaura_test_review_preview_control(&o.c,0,0)==0);pixaura_review_preview_pair pair{};int code=0;
  std::thread worker([&]{code=pixaura_review_preview_render(1,&o.c,&t,&pair);});
  const auto end=std::chrono::steady_clock::now()+std::chrono::seconds(5);
  while(!pixaura_test_review_preview_control(&o.c,1,0)&&std::chrono::steady_clock::now()<end)std::this_thread::yield();
  CHECK(pixaura_test_review_preview_control(&o.c,1,0)==1);CHECK(pixaura_review_inspection_current(1,&o.c,&r)==0);
  const auto during=inspect(o,p,s);CHECK(std::memcmp(&during,&r,sizeof(r))==0);
  o.manual();CHECK(pixaura_review_inspection_current(1,&o.c,&r)!=0);
  CHECK(pixaura_test_review_preview_control(&o.c,2,0)==0);worker.join();CHECK(code==9||code==13);
  CHECK(pixaura_test_auto_control(&o.c,5,0)==0);
 }
 {Owner o(root,++serial,fixture);const auto prior=o.dump();
  const auto s=o.candidate_bytes(action(0,"pixaura.crop","{\"height_ppm\":800000,\"width_ppm\":800000,\"x_ppm\":100000,\"y_ppm\":100000}")+","+action(1,"pixaura.rotate","{\"quarter_turns\":1}")+","+action(2,"pixaura.brightness","{\"milli_linear\":-123}"));
  pixaura_auto_proposal p{};CHECK(o.candidate(s,p)==0);const auto r=inspect(o,p,s);CHECK(r.action_count==3&&r.warnings==3);
  CHECK(r.actions[0].geometry_changed==1&&r.actions[1].geometry_changed==1&&r.actions[2].geometry_changed==0);
  CHECK(r.actions[0].parameters[2].raw_value==100000);CHECK(r.actions[1].parameters[0].raw_value==1);CHECK(r.actions[2].parameters[0].raw_value==-123);
  const auto pair=o.render(o.begin(p,s));CHECK(pixaura_review_inspection_preview(1,&o.c,&r,&pair)==0);CHECK(pixaura_review_preview_release(1,&o.c,&pair)==0);
  pixaura_agent_approval fake{};std::memcpy(&fake,&r,std::min(sizeof(fake),sizeof(r)));
  const pixaura_agent_execution_request request{1,sizeof(request),1,0,{0,0,0,0}};pixaura_agent_execution_receipt receipt{};
  CHECK(pixaura_editor_agent_execute(1,&o.c,&request,&fake,b(s),s.size(),&receipt)!=0);CHECK(o.dump()==prior);CHECK(pixaura_test_auto_control(&o.c,5,0)==0);
 }
 {Owner o(root,++serial,fixture);pixaura_auto_proposal p{};std::string actions;for(unsigned i=0;i<9;++i){if(i)actions+=',';actions+=action(i,"pixaura.exposure","{\"milli_ev\":100}");}CHECK(o.candidate(o.candidate_bytes(actions),p)!=0);
  CHECK(o.candidate("{}\n",p)==6);CHECK(o.candidate(o.candidate_bytes(action(0,"unknown","{\"milli_ev\":100}")),p)==5);CHECK(o.candidate(o.candidate_bytes(action(0,"pixaura.exposure","{\"milli_ev\":5001}")),p)==7);
  auto wrong=o.candidate_bytes(action(0,"pixaura.exposure","{\"milli_ev\":100}"));wrong.replace(wrong.find("operation_version\":1")+19,1,"2");CHECK(o.candidate(wrong,p)!=0);
 }
 std::printf("Plan inspection: %u checks; tools=11; exact order/raw/units; zero approval/candidate/history authority\n",checks);
}
