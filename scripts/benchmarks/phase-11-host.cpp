// Phase 11 successor instrumentation. Original Phase 10.1 harness is immutable.
#include "pixaura/export.h"
#include "pixaura/review_inspection.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#include <sys/resource.h>
#include <mach/mach.h>
using Clock = std::chrono::steady_clock;
static void checked(int code,int line) { if (code) { std::fprintf(stderr,"native status %d at line %d\n",code,line); std::exit(1); } }
#define ok(call) checked((call),__LINE__)
static void require(bool value) { if (!value) { std::fprintf(stderr,"invariant failed\n"); std::exit(1); } }
static long long us(Clock::time_point start) { return std::chrono::duration_cast<std::chrono::microseconds>(Clock::now()-start).count(); }
struct Reader { const std::vector<uint8_t>& data; size_t at=0; };
static int64_t read_image(void* state,uint8_t* out,uint64_t capacity) {
 auto& r=*static_cast<Reader*>(state); auto n=std::min<size_t>(capacity,r.data.size()-r.at);
 std::memcpy(out,r.data.data()+r.at,n);r.at+=n;return static_cast<int64_t>(n);
}
static std::string state(pixaura_editor_context& c) {
 uint64_t n=0;ok(pixaura_editor_state(&c,nullptr,0,&n));std::string s(n,' ');
 ok(pixaura_editor_state(&c,reinterpret_cast<uint8_t*>(s.data()),n,&n));return s;
}
static std::string revision(pixaura_editor_context& c) {
 auto s=state(c);auto p=s.find("\"revision_id\":\"");require(p!=std::string::npos);return s.substr(p+15,32);
}
int main(int argc,char** argv) {
 require(argc==3);std::ifstream f(argv[1],std::ios::binary);require(f.good());
 const std::vector<uint8_t> image{std::istreambuf_iterator<char>(f),{}};
 const auto parent=std::filesystem::canonical(argv[2]);
 // Buffers are platform-owned copies, included in process peak RSS observation.
 std::vector<uint8_t> preview(4*1024*1024),encoded(32*1024*1024);
 for(unsigned i=0;i<105;++i) {
  char id[33];std::snprintf(id,sizeof(id),"%032x",i+1);auto root=parent/id;
  require(std::filesystem::create_directory(root));auto path=root.generic_string();
  pixaura_editor_context c{};Reader reader{image};auto start=Clock::now();
  ok(pixaura_editor_open(1,&c,reinterpret_cast<const uint8_t*>(path.data()),path.size(),reinterpret_cast<const uint8_t*>(id),32,read_image,&reader,image.size()));
  const auto open_us=us(start);const auto original=revision(c);
  long long inspection_us=0,proposal_us=0;
  long long preview_each[4]{};
  long long preview_us=0,apply_us=0,png_us=0,jpeg_us=0,metadata_us=0,cancel_us=0;
  uint32_t w=0,h=0;uint64_t png_bytes=0,jpeg_bytes=0;
  for(int iteration=0;iteration<4;++iteration) {
   const auto before=revision(c);ok(pixaura_editor_select(&c,4)); // exposure, registry ASCII order
   const int32_t value=100;ok(pixaura_editor_update(&c,&value,1));
   pixaura_editor_request request{};ok(pixaura_editor_request_preview(&c,&request));start=Clock::now();
   ok(pixaura_editor_render(&c,&request,1024,1024,preview.data(),preview.size(),&w,&h));preview_each[iteration]=us(start);preview_us+=preview_each[iteration];
   ok(pixaura_editor_current(&c,&request));require(revision(c)==before);
   start=Clock::now();ok(pixaura_editor_action(&c,0));cancel_us+=us(start);require(revision(c)==before);
   require(pixaura_editor_current(&c,&request)!=0); // cancelled preview cannot regain authority
  }
  require(revision(c)==original);ok(pixaura_editor_select(&c,4));const int32_t value=100;ok(pixaura_editor_update(&c,&value,1));
  start=Clock::now();ok(pixaura_editor_action(&c,1));apply_us=us(start);const auto applied=revision(c);require(applied!=original);
  ok(pixaura_editor_action(&c,2));require(revision(c)==original);ok(pixaura_editor_action(&c,3));require(revision(c)==applied);
  pixaura_auto_request auto_request{1,sizeof(pixaura_auto_request),2,2};pixaura_auto_proposal proposal{};
  start=Clock::now();ok(pixaura_editor_auto_request(1,&c,&auto_request,nullptr,0,&proposal));proposal_us=us(start);
  uint64_t plan_size=0;require(pixaura_editor_auto_plan(1,&c,&proposal,nullptr,0,&plan_size)==11);
  std::vector<uint8_t> plan(plan_size);ok(pixaura_editor_auto_plan(1,&c,&proposal,plan.data(),plan.size(),&plan_size));
  // Auto plan transport is exact canonical bytes, including its final newline.
  pixaura_review_inspection inspection{};start=Clock::now();
  for(int j=0;j<100;++j)ok(pixaura_review_inspect(1,&c,&proposal,plan.data(),plan.size(),&inspection));
  inspection_us=us(start);ok(pixaura_review_inspection_current(1,&c,&inspection));
  require(revision(c)==applied);ok(pixaura_editor_auto_cancel(1,&c));
  require(pixaura_review_inspection_current(1,&c,&inspection)!=0);require(revision(c)==applied);
  start=Clock::now();for(int j=0;j<100;++j)require(!state(c).empty());metadata_us=us(start);
  pixaura_export_request request{};ok(pixaura_export_bind(1,&c,&request));uint32_t export_state=0;
  start=Clock::now();ok(pixaura_export_encode(1,&c,&request,nullptr,encoded.data(),encoded.size(),&png_bytes,&w,&h,&export_state));png_us=us(start);require(export_state==PIXAURA_EXPORT_SUCCEEDED);
  request.format=PIXAURA_EXPORT_JPEG;request.quality=90;start=Clock::now();
  ok(pixaura_export_encode(1,&c,&request,nullptr,encoded.data(),encoded.size(),&jpeg_bytes,&w,&h,&export_state));jpeg_us=us(start);require(export_state==PIXAURA_EXPORT_SUCCEEDED);
  require(revision(c)==applied);require(state(c).find("\"revision_count\":2")!=std::string::npos);
  // Short export buffer failure preserves caller output and ordinary manual availability.
  uint8_t sentinel=0x5a;uint64_t count=0;require(pixaura_export_encode(1,&c,&request,nullptr,&sentinel,1,&count,&w,&h,&export_state)!=0);require(sentinel==0x5a);
  ok(pixaura_editor_select(&c,4));ok(pixaura_editor_update(&c,&value,1));ok(pixaura_editor_action(&c,4));require(revision(c)==applied);
  start=Clock::now();ok(pixaura_editor_stop(&c));ok(pixaura_editor_destroy(&c));auto close_us=us(start);
  // Reopen the same durable source/history under a fresh lifetime, then release it.
  std::snprintf(id,sizeof(id),"%032x",i+1000);pixaura_editor_context reopened{};ok(pixaura_editor_open(1,&reopened,reinterpret_cast<const uint8_t*>(path.data()),path.size(),reinterpret_cast<const uint8_t*>(id),32,nullptr,nullptr,0));
  require(revision(reopened)==applied);ok(pixaura_editor_stop(&reopened));ok(pixaura_editor_destroy(&reopened));
  rusage usage{};require(getrusage(RUSAGE_SELF,&usage)==0);
  mach_task_basic_info_data_t info{};mach_msg_type_number_t info_count=MACH_TASK_BASIC_INFO_COUNT;
  require(task_info(mach_task_self(),MACH_TASK_BASIC_INFO,reinterpret_cast<task_info_t>(&info),&info_count)==KERN_SUCCESS);
  std::printf("{\"sample\":%u,\"warmup\":%s,\"open_us\":%lld,\"preview_four_us\":%lld,\"cancel_four_us\":%lld,\"apply_us\":%lld,\"proposal_us\":%lld,\"inspection_100_us\":%lld,\"state_100_us\":%lld,\"png_us\":%lld,\"jpeg_us\":%lld,\"close_us\":%lld,\"peak_rss_bytes\":%ld,\"live_rss_bytes\":%llu,\"preview_each_us\":[%lld,%lld,%lld,%lld],\"png_bytes\":%llu,\"jpeg_bytes\":%llu}\n",i,i<5?"true":"false",open_us,preview_us,cancel_us,apply_us,proposal_us,inspection_us,metadata_us,png_us,jpeg_us,close_us,usage.ru_maxrss,static_cast<unsigned long long>(info.resident_size),preview_each[0],preview_each[1],preview_each[2],preview_each[3],static_cast<unsigned long long>(png_bytes),static_cast<unsigned long long>(jpeg_bytes));
  std::filesystem::remove_all(root);
 }
}
