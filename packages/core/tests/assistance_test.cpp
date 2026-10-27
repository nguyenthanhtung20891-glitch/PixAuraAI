#include "../src/assistance.hpp"
#include "agent_schema_fixtures.hpp"
#include "reference_planner_fixtures.hpp"
#include "../src/assist_experience.hpp"
#include <atomic>
#include <algorithm>
#include <chrono>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <thread>
using namespace pixaura;
using namespace assistance;
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"assistance line %d: %s\n",__LINE__,#x);std::abort();}}while(false)
static document::String change(document::String s,std::string_view from,std::string_view to){auto p=s.find(from);CHECK(p!=s.npos);s.replace(p,from.size(),to);return s;}
struct Hook { ReferenceProvider& provider;const Request& request;uint32_t point,mode;unsigned seen=0; };
static void checkpoint(uint32_t point,void* state){auto& h=*static_cast<Hook*>(state);if(point!=h.point)return;++h.seen;
 if(h.mode==0)h.provider.cancel();else if(h.mode==1)h.provider.stop();else CHECK(h.provider.request(h.request).code==8);
}
int main(){
 const auto doc=document::deserialize(agent_schema_fixtures::document,"00000000000000000000000000000500");CHECK(doc.code==0);
 const auto& d=*doc.value;const std::string_view context="000000000000000000000000000000a1";
 const auto before=document::serialize(d);CHECK(before.code==0);
 Request r;r.proposal_id="proposal.1";r.binding=document::agent_schema_binding(d,context).value;
 ReferenceProvider provider;CHECK(provider.identity().id==reference_id && provider.identity().capabilities==7);
 CHECK(check_identity(provider.identity())==0);CHECK(check_identity({reference_id,1,8})==1);
 CHECK(check_identity({reference_id,2,7})==2);CHECK(check_identity({"unknown",1,7})==1);
 auto validate_bytes=[&](std::string_view s){return validate(s,d,context,reference_id);};
 unsigned positives=0;uint64_t fingerprint=UINT64_C(14695981039346656037);
 uint64_t request_ns=0,validation_ns=0,max_request_ns=0,max_validation_ns=0;
 for(const auto& fixture:reference_planner_fixtures::cases){
  r.payload=fixture.input;
  for(Kind kind:{Kind::Advice,Kind::StructuredIntent,Kind::PlanCandidate}){
   r.kind=kind;if(kind==Kind::PlanCandidate)r.payload=fixture.plan;
   auto started=std::chrono::steady_clock::now();const auto first=provider.request(r);
   const auto request_elapsed=static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()-started).count());
   request_ns+=request_elapsed;max_request_ns=std::max(max_request_ns,request_elapsed);CHECK(first.code==0);
   for(unsigned char byte:first.value.value()){fingerprint^=byte;fingerprint*=UINT64_C(1099511628211);}
   for(unsigned repeat=0;repeat<10;++repeat){const auto output=provider.request(r);CHECK(output.code==0 && output.value.value()==first.value.value());}
   started=std::chrono::steady_clock::now();const auto valid=validate_bytes(first.value.value());
   const auto validation_elapsed=static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()-started).count());
   validation_ns+=validation_elapsed;max_validation_ns=std::max(max_validation_ns,validation_elapsed);
   CHECK(valid.code==0);CHECK(valid.value->envelope==first.value.value());
   CHECK(valid.value->plan==(kind==Kind::Advice?"":fixture.plan));++positives;
   const auto forged=change(first.value.value(),"proposal.1","forged.2");const auto correlation=validate_bytes(forged);
   CHECK(correlation.code==0 && correlation.value->plan==valid.value->plan); // ID grants NOTHING
  }
 }
 CHECK(fingerprint==UINT64_C(0x162cbcfb525cd535)); // independently constructed JSON golden stream
 // Typical values come from the SAME existing application catalog, not a new recipe.
 for(uint32_t index=0;index<11;++index){
  const auto choice=assist_experience::choice(index);
  auto typical=assist_experience::request(d,context,index,manual::registry()[choice.tool_index],choice,42);
  CHECK(typical.code==0);
  for(Kind kind:{Kind::Advice,Kind::StructuredIntent}){
   typical.value->kind=kind;auto out=provider.request(*typical.value);CHECK(out.code==0);
   auto decoded=validate_bytes(*out.value);CHECK(decoded.code==0);
   if(kind==Kind::Advice){CHECK(decoded.value->plan.empty());
    const auto shown=assist_experience::presentation(*decoded.value,manual::registry().data(),manual::registry().size());
    CHECK(shown.code==0&&shown.value->tool_index==choice.tool_index&&shown.value->values==choice.values);
   }else CHECK(!decoded.value->plan.empty());
  }
 }
 r.kind=Kind::StructuredIntent;r.payload=reference_planner_fixtures::cases[0].input;
 r.proposal_id=document::String(64,'a');CHECK(provider.request(r).code==0);r.proposal_id="proposal.1";
 CHECK(encode(r,"unknown.provider").code==1);
 for(uint32_t point:{0u,1u}){
  ReferenceProvider target;Hook hook{target,r,point,0};target.assistance_checkpoint=checkpoint;target.assistance_checkpoint_state=&hook;
  CHECK(target.request(r).code==13 && hook.seen==1);
  target.assistance_checkpoint=nullptr;CHECK(target.request(r).code==0);
  hook.mode=2;target.assistance_checkpoint=checkpoint;CHECK(target.request(r).code==0);
  hook.mode=1;CHECK(target.request(r).code==13);CHECK(target.request(r).code==13);
 }
 r.payload=document::String(8193,' ');CHECK(provider.request(r).code==8);r.payload=reference_planner_fixtures::cases[0].input;
 const auto envelope=provider.request(r);CHECK(envelope.code==0);
 // Independent hand-reviewed golden: sorted keys, nested unchanged canonical payload,
 // exact LF, no random/time/owner address fields.
 auto binding=r.binding;binding.pop_back();auto payload=r.payload;payload.pop_back();
 const auto expected=document::String("{\"binding\":")+binding+",\"envelope_version\":1,\"kind\":2,\"payload\":"+payload+",\"proposal_id\":\"proposal.1\",\"provider_id\":\"reference.assistance.v1\",\"provider_version\":1,\"registry_version\":1}\n";
 CHECK(envelope.value.value()==expected);
 auto reject=[&](const document::String& bytes){const auto result=validate_bytes(bytes);if(!result.code)std::fprintf(stderr,"unexpected accepted: %s\n",bytes.c_str());CHECK(result.code!=0);CHECK(!result.value);};
 for(const auto& bad:reference_planner_fixtures::negative){r.payload=bad.input;const auto out=provider.request(r);if(!out.code)reject(out.value.value());else CHECK(!out.value);}
 r.payload=reference_planner_fixtures::cases[0].input;
 reject("");reject("{");reject(envelope.value.value()+"x");reject(" "+envelope.value.value());reject(document::String(envelope_limit+1,' '));
 for(const auto key:{"envelope_version","provider_version","registry_version"})reject(change(envelope.value.value(),document::String("\"")+key+"\":1",document::String("\"")+key+"\":2"));
 reject(change(envelope.value.value(),"\"kind\":2","\"kind\":99"));
 reject(change(envelope.value.value(),"\"kind\":2","\"kind\":-1"));
 reject(change(envelope.value.value(),"\"kind\":2","\"kind\":2,\"kind\":2"));
 reject(change(envelope.value.value(),"\"kind\":2","\"kind\":2,\"approved\":true"));
 for(const auto value:{"", "UPPER", "bad..id", "bad/id", "bad\\u0000id", "bad\\ud800"})reject(change(envelope.value.value(),"proposal.1",value));
 reject(change(envelope.value.value(),"proposal.1",document::String(65,'a')));
 reject(change(envelope.value.value(),"proposal.1",document::String("\xc0\xaf",2)));
 reject(change(envelope.value.value(),"reference.assistance.v1","other.provider.v1"));
 for(const auto token:{"000000000000000000000000000000a1","00000000000000000000000000000002","00000000000000000000000000000001","00000000000000000000000000000102","00000000000000000000000000000500","aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"}){
  auto replacement=document::String(token);replacement.back()=replacement.back()=='0'?'1':'0';reject(change(envelope.value.value(),token,replacement));
 }
 reject(change(envelope.value.value(),"\"generation\":0","\"generation\":1"));
 reject(change(envelope.value.value(),"\"generation\":0","\"generation\":9007199254740992"));
 CHECK(validate(envelope.value.value(),d,"000000000000000000000000000000a2",reference_id).code==9);
 CHECK(validate(envelope.value.value(),d,context,reference_id,2).code==2);
 CHECK(validate(envelope.value.value(),d,context,"other.provider.v1").code==1);
 const auto advanced=document::restore_generation(doc.value,1);CHECK(advanced.code==0);
 CHECK(validate(envelope.value.value(),*advanced.value,context,reference_id).code==9); // ABA: same visible revision, changed generation
 for(const auto& f:agent_schema_fixtures::cases)if(f.kind==2){
  r.kind=Kind::PlanCandidate;r.payload=f.status==0?f.canonical:f.input;
  const auto out=provider.request(r);
  if(f.status==0){CHECK(out.code==0);CHECK(validate_bytes(out.value.value()).code==0);}
  else if(!out.code)reject(out.value.value());
 }
 r.kind=Kind::StructuredIntent;r.payload=reference_planner_fixtures::cases[0].input;
 r.version=2;CHECK(provider.request(r).code==2);r.version=1;r.registry_version=2;CHECK(provider.request(r).code==2);r.registry_version=1;
 r.kind=static_cast<Kind>(99);CHECK(provider.request(r).code!=0);r.kind=Kind::StructuredIntent;
 r.proposal_id=document::String(65,'a');CHECK(provider.request(r).code!=0);r.proposal_id="proposal.1";
 r.pre_cancel=true;CHECK(provider.request(r).code==13);r.pre_cancel=false;
 provider.cancel();CHECK(provider.request(r).code==0); // nonsticky
 // No handle/token API exists. Separate owner shutdown/cancel cannot affect this owner.
 ReferenceProvider other;other.stop();CHECK(other.request(r).code==13);CHECK(provider.request(r).code==0);
 std::atomic<unsigned> success{0},busy{0},cancelled{0};std::atomic<bool> go{false};
 std::thread requests[3];for(auto& thread:requests)thread=std::thread([&]{while(!go.load()){}for(unsigned i=0;i<200;++i){auto result=provider.request(r);if(result.code==0){if(result.value!=envelope.value.value())std::abort();++success;}else if(result.code==8)++busy;else if(result.code==13)++cancelled;else std::abort();}});
 go=true;for(unsigned i=0;i<100;++i)provider.cancel();provider.stop();for(auto& thread:requests)thread.join();
 CHECK(success+busy+cancelled==600);CHECK(provider.request(r).code==13);
 CHECK(document::serialize(d).value==before.value); // original, revisions, history unchanged
 CHECK(d.operations().size()==2 && d.revisions().size()==3);
 std::printf("Assistance admission v1 provider=reference.assistance.v1 corpus=%u typical=22 fingerprint=%016" PRIx64 " owner_bytes=%zu request_mean_ns=%" PRIu64 " request_max_ns=%" PRIu64 " validation_mean_ns=%" PRIu64 " validation_max_ns=%" PRIu64 "\n",
  positives,fingerprint,sizeof(ReferenceProvider),request_ns/positives,max_request_ns,validation_ns/positives,max_validation_ns);
 std::printf("Assistance v1 checks=%u positives=%u deterministic repeats=10 concurrency=600 side_effects=0 no_approval_or_candidate_API\n",checks,positives);
}
