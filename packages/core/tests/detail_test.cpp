#include "../src/evaluation.hpp"
#include "../src/geometry.hpp"
#include "detail_fixtures.hpp"
#include <cmath>
#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <limits>
#define CHECK(x) do{if(!(x))throw std::runtime_error("line "+std::to_string(__LINE__)+": " #x);}while(false)
using namespace pixaura;
static float value(unsigned b){float f;std::memcpy(&f,&b,4);return f;}
struct Stop{evaluation::Cancellation* cancel;unsigned count=0,target=0;};
static void stop(evaluation::Checkpoint,void* state){auto& s=*static_cast<Stop*>(state);if(++s.count==s.target)s.cancel->requested.store(true);}
int main(){try{
 unsigned count=0;
 for(const auto& f:detail_fixtures){
  working::Image source;const uint64_t row=uint64_t(f.w)*16,bytes=row*f.h;
  source.metadata={1,sizeof(pixaura_working_metadata),f.w,f.h,1,1,1,0,row,bytes};
  source.pixels=std::make_unique<decode::Vector<float>>();for(unsigned i=0;i<f.w*f.h*4;++i)source.pixels->push_back(value(f.input[i]));
  const auto stack=evaluation::parse(f.request);const auto result=evaluation::evaluate(source,stack,working::defaults());
  CHECK(result.metadata.width==f.ow&&result.metadata.height==f.oh);
  for(unsigned i=0;i<f.ow*f.oh*4;++i){const double expected=value(f.expected[i]);CHECK(std::abs(double((*result.pixels)[i])-expected)<=1e-7+2e-6*std::abs(expected)+1e-44);}
  for(unsigned i=0;i<f.w*f.h*4;++i){unsigned b;std::memcpy(&b,&(*source.pixels)[i],4);CHECK(b==f.input[i]);}
  if(stack.size()==1){const auto amount=std::get<document::Detail>(stack[0].parameters).value;if(!amount)CHECK(std::memcmp(result.pixels->data(),source.pixels->data(),static_cast<std::size_t>(bytes))==0);if(stack[0].type=="pixaura.sharpen")for(unsigned i=3;i<f.w*f.h*4;i+=4)CHECK(std::memcmp(result.pixels->data()+i,source.pixels->data()+i,4)==0);}
  if(stack.size()>1){auto sequential=evaluation::evaluate(source,{},working::defaults());for(const auto& op:stack)sequential=evaluation::evaluate(sequential,{op},working::defaults());CHECK(sequential.metadata.width==f.ow&&sequential.metadata.height==f.oh&&std::memcmp(sequential.pixels->data(),result.pixels->data(),static_cast<std::size_t>(result.metadata.image_bytes))==0);}
  ++count;
 }
 for(const auto* tool:{"blur","sharpen"})for(const auto* bad:{"-1","1001","0.1","1e0","\"1\"","true","null","-0"}){const std::string parameter=std::strcmp(tool,"blur")==0?"milli_strength":"milli_amount";const std::string request=std::string("{\"operations\":[{\"id\":\"00000000000000000000000000000011\",\"type\":\"pixaura.")+tool+"\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\""+parameter+"\":"+bad+"}}]}";CHECK(document::parse_evaluation(request).code!=0);}
 working::Image wide;wide.metadata={1,sizeof(pixaura_working_metadata),16384,1,1,1,1,0,262144,262144};wide.pixels=std::make_unique<decode::Vector<float>>(65536,0.25f);for(unsigned i=3;i<65536;i+=4)(*wide.pixels)[i]=1;
 for(const auto* tool:{"blur","sharpen"}){
  const std::string parameter=std::strcmp(tool,"blur")==0?"milli_strength":"milli_amount";
  const std::string request=std::string("{\"operations\":[{\"id\":\"00000000000000000000000000000011\",\"type\":\"pixaura.")+tool+"\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\""+parameter+"\":1000}}]}";
  const auto stack=evaluation::parse(request);CHECK(geometry::plan({16384,1},stack,working::defaults()).scratch_bytes==786432);
  auto limit=working::defaults();limit.width=16383;bool resource=false;try{evaluation::evaluate(wide,stack,limit);}catch(const document::Failure& e){resource=e.code==8;}CHECK(resource);
  evaluation::Cancellation cancel;Stop state{&cancel,0,0};const auto result=evaluation::evaluate(wide,stack,working::defaults(),&cancel,stop,&state);CHECK(std::memcmp(result.pixels->data(),wide.pixels->data(),262144)==0);
  const auto points=state.count;for(unsigned n=1;n<=points;++n){cancel.requested.store(false);state.count=0;state.target=n;bool cancelled=false;try{evaluation::evaluate(wide,stack,working::defaults(),&cancel,stop,&state);}catch(const document::Failure& e){cancelled=e.code==13;}CHECK(cancelled);}
  std::atomic<uint64_t> generation{2};std::atomic<bool> revoked{false};cancel.requested.store(false);cancel.generation=&generation;cancel.revoked=&revoked;cancel.expected=1;bool stale=false;try{evaluation::evaluate(wide,stack,working::defaults(),&cancel);}catch(const document::Failure& e){stale=e.code==13;}CHECK(stale);
  auto version=request;const auto at=version.find("operation_version\":1");version.replace(at,20,"operation_version\":2");CHECK(document::parse_evaluation(version).code==5);
 }
 working::Image impulse;impulse.metadata={1,sizeof(pixaura_working_metadata),3,3,1,1,1,0,48,144};impulse.pixels=std::make_unique<decode::Vector<float>>(36,0);for(unsigned i=3;i<36;i+=4)(*impulse.pixels)[i]=1;(*impulse.pixels)[16]=std::numeric_limits<float>::max();
 const auto overflow=evaluation::parse("{\"operations\":[{\"id\":\"00000000000000000000000000000011\",\"type\":\"pixaura.sharpen\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_amount\":1000}}]}");bool rejected=false;try{evaluation::evaluate(impulse,overflow,working::defaults());}catch(const document::Failure& e){rejected=e.code==7;}CHECK(rejected&&(*impulse.pixels)[16]==std::numeric_limits<float>::max());
 std::printf("Independent detail reference PASS %u fixtures; ordered/repeated/geometry/alpha/borders/neutral/source invariance; 786432-byte scratch/admission/all-checkpoint cancellation/stale generation/overflow rejection PASS\n",count);return 0;
}catch(const document::Failure& e){std::fprintf(stderr,"detail status %d\n",e.code);return 1;}catch(const std::exception& e){std::fprintf(stderr,"detail %s\n",e.what());return 1;}}
