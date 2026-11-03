#include "pixaura/result_review.h"
#include "pixaura/editor.h"
#include "../src/document.hpp"
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
#include <vector>
extern "C" int32_t pixaura_test_editor_document(pixaura_editor_context*,uint8_t*,uint64_t,uint64_t*);
extern "C" int32_t pixaura_test_agent_platform_control(pixaura_editor_context*,uint32_t,uint32_t);
static std::atomic<unsigned> checks{0};
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"result review %d %s\n",__LINE__,#x);std::abort();}}while(false)
static const uint8_t* bytes(const std::string& s){return reinterpret_cast<const uint8_t*>(s.data());}
static int64_t importer(void* p,uint8_t* out,uint64_t n){auto& at=*static_cast<std::size_t*>(p);n=std::min<uint64_t>(n,sizeof(decode_png)-at);std::memcpy(out,decode_png+at,static_cast<std::size_t>(n));at+=static_cast<std::size_t>(n);return static_cast<int64_t>(n);}
struct Owner{
 pixaura_editor_context c{};std::filesystem::path root;
 explicit Owner(const std::filesystem::path& parent,unsigned serial){char id[33];std::snprintf(id,sizeof(id),"%032x",serial);root=parent/(std::string("result-review-")+id);
  std::filesystem::create_directory(root);const auto path=root.generic_string();std::size_t at=0;
  pixaura_capability_profile profile{};CHECK(pixaura_capability_baseline(1,&profile)==0);
  profile.hardware.logical_cpu_count=8;profile.hardware.physical_memory_bytes=UINT64_C(8589934592);
  profile.runtime={1,sizeof(pixaura_capability_dynamic),1,1,0,1,UINT64_C(536870912),1};
  const pixaura_policy_observation observation{1,sizeof(pixaura_policy_observation),1,1,0,1,0,0,UINT64_C(536870912),1};
  pixaura_execution_strategy strategy{};
  CHECK(pixaura_editor_open_policy(1,&c,bytes(path),path.size(),reinterpret_cast<const uint8_t*>(id),32,importer,&at,sizeof(decode_png),&profile,&observation,&strategy)==0);}
 ~Owner(){CHECK(pixaura_editor_stop(&c)==0);CHECK(pixaura_editor_destroy(&c)==0);}
 std::string document(){uint64_t n=0;CHECK(pixaura_test_editor_document(&c,nullptr,0,&n)==0);std::string s(static_cast<std::size_t>(n),' ');CHECK(pixaura_test_editor_document(&c,reinterpret_cast<uint8_t*>(s.data()),n,&n)==0);return s;}
 std::string binding(){uint64_t n=0;CHECK(pixaura_editor_agent_binding(1,&c,nullptr,0,&n)==11);std::string s(static_cast<std::size_t>(n),' ');CHECK(pixaura_editor_agent_binding(1,&c,reinterpret_cast<uint8_t*>(s.data()),n,&n)==0);s.pop_back();return s;}
 std::string plan(std::string fixture){const auto begin=fixture.find("\"binding\":")+10;const auto end=fixture.find('}',begin)+1;fixture.replace(begin,end-begin,binding());return fixture;}
 pixaura_agent_approval approve(const std::string& p){pixaura_agent_approval a{};pixaura_agent_execution_request r{1,sizeof(r),1,0,{0,0,0,0}};CHECK(pixaura_editor_agent_approve(1,&c,&r,bytes(p),p.size(),&a)==0);return a;}
 int execute(const std::string& p,const pixaura_agent_approval* a,pixaura_agent_execution_receipt* receipt=nullptr){pixaura_agent_execution_request r{1,sizeof(r),1,0,{0,0,0,0}};
  pixaura_agent_execution_receipt out;std::memset(&out,0xa5,sizeof(out));const auto sentinel=out;const auto code=pixaura_editor_agent_execute(1,&c,&r,a,bytes(p),p.size(),receipt?receipt:&out);
  if(code&&!receipt){CHECK(std::memcmp(&out,&sentinel,sizeof(out))==0);}
  return code;}
};
static std::string exposure(){return std::string("{\"actions\":[{\"action_index\":0,\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":5000},\"tool_id\":\"pixaura.exposure\"}],\"binding\":{},\"plan_version\":1}\n");}
static std::string crop(){return std::string("{\"actions\":[{\"action_index\":0,\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"height_ppm\":500000,\"width_ppm\":500000,\"x_ppm\":0,\"y_ppm\":0},\"tool_id\":\"pixaura.crop\"}],\"binding\":{},\"plan_version\":1}\n");}
static pixaura_result_review query(Owner& o,const pixaura_agent_execution_receipt* receipt){pixaura_result_review out{};CHECK(pixaura_result_review_query(1,&o.c,receipt,&out)==0);return out;}
static void hex_fill(uint8_t* out,std::size_t n,char c){std::memset(out,c,n);}
static pixaura_agent_execution_receipt forged(){pixaura_agent_execution_receipt r{};r.api_version=1;r.struct_size=sizeof(r);r.approval_consumed=1;hex_fill(r.plan_sha256,64,'a');hex_fill(r.source_revision,32,'b');hex_fill(r.resulting_revision,32,'c');return r;}
static bool same(const char* value,std::string_view text){return text.size()==32&&std::memcmp(value,text.data(),32)==0;}
int main(int argc,char** argv){
 CHECK(argc==2);const auto parent=std::filesystem::u8path(argv[1])/("result-review-suite-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
 CHECK(std::filesystem::create_directory(parent));unsigned serial=86000;
 {Owner o(parent,++serial);const auto before=o.document();const auto root=query(o,nullptr);
  CHECK(root.state==PIXAURA_RESULT_NONE&&root.revision_count==1&&root.result_revision[0]==0&&(root.flags&PIXAURA_RESULT_PUBLICATION_VISIBLE)==0);
  const auto p=o.plan(exposure());auto a=o.approve(p);CHECK(query(o,nullptr).revision_count==1);CHECK(o.document()==before);
  pixaura_agent_execution_receipt receipt{};CHECK(o.execute(p,&a,&receipt)==0);
  CHECK(o.document()!=before);auto parsed=pixaura::document::deserialize(o.document(),std::string(32,'a'));
  CHECK(parsed.code==0&&parsed.value->revisions().size()==2&&parsed.value->revisions().back().actor=="ai");
  CHECK(query(o,&receipt).state==PIXAURA_RESULT_CURRENT&&query(o,&receipt).revision_count==2);
  CHECK(pixaura_editor_agent_release(1,&o.c,&a)==0);
 }
 {Owner o(parent,++serial);const auto before=o.document();const auto p=o.plan(exposure());auto a=o.approve(p);pixaura_agent_execution_receipt receipt{};
  CHECK(o.execute(p,&a,&receipt)==0);const auto after=o.document();CHECK(after!=before);
  auto described=query(o,&receipt);CHECK(described.state==PIXAURA_RESULT_CURRENT);CHECK(described.revision_count==2);
  CHECK(same(described.result_revision,std::string(reinterpret_cast<char*>(receipt.resulting_revision),32)));
  CHECK(same(described.previous_revision,std::string(reinterpret_cast<char*>(receipt.source_revision),32)));
  CHECK(same(described.live_revision,std::string(reinterpret_cast<char*>(receipt.resulting_revision),32)));
  CHECK((described.flags&PIXAURA_RESULT_PUBLICATION_VISIBLE)&&(described.flags&PIXAURA_RESULT_HAS_PREVIOUS)&&(described.flags&PIXAURA_RESULT_CAN_UNDO));
  CHECK(described.geometry_changed==0);CHECK(o.document()==after);
  auto unconfirmed=query(o,nullptr);CHECK(unconfirmed.state==PIXAURA_RESULT_UNCONFIRMED&&unconfirmed.revision_count==2);
  CHECK((unconfirmed.flags&PIXAURA_RESULT_PUBLICATION_VISIBLE)==0);CHECK(same(unconfirmed.live_revision,std::string(reinterpret_cast<char*>(receipt.resulting_revision),32)));
  CHECK(unconfirmed.result_revision[0]==0);
  const auto fake=forged();auto absent=query(o,&fake);CHECK(absent.state==PIXAURA_RESULT_ABSENT&&absent.revision_count==2);
  CHECK(absent.result_revision[0]==0&&(absent.flags&PIXAURA_RESULT_PUBLICATION_VISIBLE)==0);CHECK(o.document()==after);
  std::vector<uint8_t> before_pixels(32u*32u*4),after_pixels(32u*32u*4);uint32_t bw=0,bh=0,aw=0,ah=0;
  CHECK(pixaura_result_review_render(1,&o.c,&described,described.previous_revision,32,32,before_pixels.data(),before_pixels.size(),&bw,&bh)==0);
  CHECK(pixaura_result_review_render(1,&o.c,&described,described.result_revision,32,32,after_pixels.data(),after_pixels.size(),&aw,&ah)==0);
  CHECK(bw&&bh&&aw&&ah);CHECK(bw!=aw||bh!=ah||std::memcmp(before_pixels.data(),after_pixels.data(),static_cast<std::size_t>(bw)*bh*4)!=0);
  char preview_id[32];std::memset(preview_id,'d',32);uint32_t zw=7,zh=7;std::vector<uint8_t> pixels(32u*32u*4,0x5a),sentinel=pixels;
  CHECK(pixaura_result_review_render(1,&o.c,&described,preview_id,32,32,pixels.data(),pixels.size(),&zw,&zh)==3);
  CHECK(pixels==sentinel&&zw==7&&zh==7);CHECK(o.document()==after);
  CHECK(pixaura_editor_action(&o.c,2)==0);auto undone=query(o,&receipt);CHECK(undone.state==PIXAURA_RESULT_STALE&&undone.revision_count==2);
  CHECK(same(undone.result_revision,std::string(reinterpret_cast<char*>(receipt.resulting_revision),32)));
  CHECK(!same(undone.live_revision,std::string(reinterpret_cast<char*>(receipt.resulting_revision),32)));
  CHECK(pixaura_result_review_render(1,&o.c,&described,described.result_revision,32,32,after_pixels.data(),after_pixels.size(),&aw,&ah)==9);
  CHECK(pixaura_result_review_render(1,&o.c,&undone,undone.live_revision,32,32,before_pixels.data(),before_pixels.size(),&bw,&bh)==0);
  CHECK(pixaura_editor_action(&o.c,3)==0);auto redone=query(o,&receipt);CHECK(redone.state==PIXAURA_RESULT_CURRENT&&o.document()==after);
  CHECK(pixaura_editor_action(&o.c,2)==0);const auto branch=o.plan(exposure());auto branch_approval=o.approve(branch);CHECK(o.execute(branch,&branch_approval)==0);
  auto branched=query(o,&receipt);CHECK(branched.state==PIXAURA_RESULT_STALE);auto model=pixaura::document::deserialize(o.document(),std::string(32,'a'));
  CHECK(model.code==0&&model.value->revisions().size()==3&&model.value->redo().empty());CHECK(pixaura_editor_action(&o.c,3)==10);
  CHECK(pixaura_editor_agent_release(1,&o.c,&a)==0);CHECK(pixaura_editor_agent_release(1,&o.c,&branch_approval)==0);
 }
 {Owner o(parent,++serial);const auto p=o.plan(exposure());auto a=o.approve(p);pixaura_agent_execution_receipt receipt{};CHECK(o.execute(p,&a,&receipt)==0);
  CHECK(pixaura_editor_select(&o.c,4)==0);const int32_t value=250;CHECK(pixaura_editor_update(&o.c,&value,1)==0);CHECK(pixaura_editor_action(&o.c,1)==0);
  auto manual=query(o,&receipt);CHECK(manual.state==PIXAURA_RESULT_STALE&&manual.revision_count==3);CHECK(pixaura_editor_agent_release(1,&o.c,&a)==0);
 }
 pixaura_agent_execution_receipt persisted{};std::string persisted_root;
 {Owner o(parent,++serial);persisted_root=o.root.generic_string();const auto p=o.plan(exposure());auto a=o.approve(p);
  CHECK(o.execute(p,&a,&persisted)==0);CHECK(pixaura_editor_agent_release(1,&o.c,&a)==0);
 }
 {pixaura_editor_context reopened{};char id[33];std::snprintf(id,sizeof(id),"%032x",++serial);
  CHECK(pixaura_editor_open(1,&reopened,bytes(persisted_root),persisted_root.size(),reinterpret_cast<const uint8_t*>(id),32,nullptr,nullptr,0)==0);
  pixaura_result_review restored{};CHECK(pixaura_result_review_query(1,&reopened,&persisted,&restored)==0);
  CHECK(restored.state==PIXAURA_RESULT_CURRENT&&restored.revision_count==2);
  CHECK(pixaura_editor_stop(&reopened)==0);CHECK(pixaura_editor_destroy(&reopened)==0);
 }
 {Owner first(parent,++serial),second(parent,++serial);const auto p=first.plan(exposure());auto a=first.approve(p);pixaura_agent_execution_receipt receipt{};
  CHECK(first.execute(p,&a,&receipt)==0);auto foreign=query(second,&receipt);CHECK(foreign.state==PIXAURA_RESULT_ABSENT&&foreign.revision_count==1);
  CHECK(std::memcmp(foreign.document_id,query(first,&receipt).document_id,32)!=0);CHECK(first.document()!=second.document());
  CHECK(pixaura_editor_agent_release(1,&first.c,&a)==0);
 }
 {Owner o(parent,++serial);const auto p=o.plan(crop());auto a=o.approve(p);pixaura_agent_execution_receipt receipt{};CHECK(o.execute(p,&a,&receipt)==0);
  auto described=query(o,&receipt);CHECK(described.state==PIXAURA_RESULT_CURRENT&&described.geometry_changed==1);CHECK(pixaura_editor_agent_release(1,&o.c,&a)==0);
 }
 {Owner o(parent,++serial);const auto before=o.document();const auto p=o.plan(exposure());auto a=o.approve(p);
  CHECK(pixaura_test_agent_platform_control(&o.c,0,1)==0);std::atomic<int> result{-1};std::thread worker([&]{result=o.execute(p,&a);});
  const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(10);
  while(pixaura_test_agent_platform_control(&o.c,1,0)==0&&std::chrono::steady_clock::now()<deadline){std::this_thread::yield();}
  CHECK(pixaura_test_agent_platform_control(&o.c,1,0)==1);CHECK(pixaura_editor_agent_cancel(1,&o.c)==0);CHECK(pixaura_test_agent_platform_control(&o.c,2,0)==0);worker.join();
  CHECK(result==13);CHECK(o.document()==before);auto none=query(o,nullptr);CHECK(none.state==PIXAURA_RESULT_NONE&&none.revision_count==1);
  const auto fake=forged();auto absent=query(o,&fake);CHECK(absent.state==PIXAURA_RESULT_ABSENT&&absent.revision_count==1);CHECK(pixaura_editor_agent_release(1,&o.c,&a)==0);
 }
 {Owner o(parent,++serial);const auto p=o.plan(exposure());auto a=o.approve(p);pixaura_agent_execution_receipt receipt{};CHECK(o.execute(p,&a,&receipt)==0);
  CHECK(pixaura_editor_agent_cancel(1,&o.c)==0);auto kept=query(o,&receipt);CHECK(kept.state==PIXAURA_RESULT_CURRENT&&kept.revision_count==2);
  CHECK(pixaura_editor_agent_release(1,&o.c,&a)==0);
 }
 CHECK(pixaura_result_review_query(1,nullptr,nullptr,nullptr)==1);
 std::fprintf(stderr,"result review checks %u\n",checks.load());return 0;
}
