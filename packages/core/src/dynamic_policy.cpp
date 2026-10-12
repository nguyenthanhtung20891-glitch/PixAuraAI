#include "dynamic_policy.hpp"
#include <algorithm>
#include <cstring>
#include <limits>
namespace pixaura::policy {
namespace {
constexpr uint64_t mib=UINT64_C(1048576);
int32_t valid(const pixaura_policy_observation* o) {
    if(!o) return 1;
    if(o->version!=1||o->struct_size!=sizeof(*o)) return 2;
    if(o->thermal>PIXAURA_POLICY_CRITICAL||o->memory_pressure>PIXAURA_POLICY_CRITICAL||
        o->constrained>1||o->gpu_availability>PIXAURA_CAPABILITY_AVAILABLE||
        o->gpu_validation>PIXAURA_GPU_PHYSICAL_CERTIFIED||o->reserved||!o->sequence) return 1;
    return 0;
}
pixaura_capability_profile normalized(const State& s, const pixaura_policy_observation& o) {
    auto p=s.profile;
    p.hardware.gpu_validation=std::min(p.hardware.gpu_validation,o.gpu_validation);
    p.runtime.thermal=std::min(o.thermal,uint32_t(PIXAURA_THERMAL_SEVERE));
    p.runtime.memory_pressure=std::min(o.memory_pressure,uint32_t(PIXAURA_PRESSURE_SEVERE));
    p.runtime.constrained=o.constrained;p.runtime.gpu_availability=o.gpu_availability;
    p.runtime.process_budget_bytes=o.process_budget_bytes;p.runtime.sequence=o.sequence;
    return p;
}
bool same(const pixaura_execution_strategy& a,const pixaura_execution_strategy& b) {
    return a.tier==b.tier&&a.runtime_cap_bytes==b.runtime_cap_bytes&&a.preview_edge==b.preview_edge&&
        a.concurrency==b.concurrency&&a.backend==b.backend&&a.forced_cpu==b.forced_cpu;
}
bool known_normal(const pixaura_policy_observation& o) {
    return o.thermal==PIXAURA_POLICY_NORMAL&&o.memory_pressure==PIXAURA_POLICY_NORMAL&&
        !o.constrained&&o.process_budget_bytes&&o.gpu_availability!=PIXAURA_CAPABILITY_UNKNOWN;
}
int32_t derive(const State& state,const pixaura_policy_observation& o,uint64_t limit,pixaura_execution_strategy& out) {
    if(o.gpu_validation>state.profile.hardware.gpu_validation)return 1;
    const auto p=normalized(state,o);
    pixaura_budget_request q{};q.version=1;q.struct_size=sizeof(q);q.work_class=PIXAURA_BUDGET_COMMIT;
    q.width=q.height=1;q.bytes_per_pixel=16;q.raster_count=1;q.requested_handles=1;q.context_limit_bytes=limit;
    pixaura_budget_result b{};const auto code=pixaura_budget_admit(1,&p,&q,&b);if(code)return code;
    pixaura_budget_result ceiling{};const auto base=pixaura_budget_admit(1,&state.profile,&q,&ceiling);if(base)return base;
    out={};out.version=1;out.struct_size=sizeof(out);out.tier=b.tier;
    out.runtime_cap_bytes=b.runtime_cap_bytes;out.preview_edge=b.preview_edge;out.concurrency=1;out.forced_cpu=1;
    out.tier=std::min(out.tier,ceiling.tier);out.runtime_cap_bytes=std::min(out.runtime_cap_bytes,ceiling.runtime_cap_bytes);
    out.runtime_cap_bytes=std::min(out.runtime_cap_bytes,limit);
    out.preview_edge=std::min(out.preview_edge,ceiling.preview_edge);
    const auto severity=std::max(o.thermal,o.memory_pressure);
    if(severity>=PIXAURA_POLICY_SEVERE) out.reason=o.memory_pressure>=o.thermal?PIXAURA_POLICY_MEMORY:PIXAURA_POLICY_THERMAL;
    else if(o.constrained)out.reason=PIXAURA_POLICY_CONSTRAINED;
    else if(severity==PIXAURA_POLICY_ELEVATED)out.reason=o.memory_pressure>=o.thermal?PIXAURA_POLICY_MEMORY:PIXAURA_POLICY_THERMAL;
    else if(!known_normal(o)||!p.hardware.logical_cpu_count||!p.hardware.physical_memory_bytes)
        out.reason=PIXAURA_POLICY_UNKNOWN_FACTS;
    else if(out.tier<ceiling.tier||out.runtime_cap_bytes<ceiling.runtime_cap_bytes||out.preview_edge<ceiling.preview_edge)
        out.reason=PIXAURA_POLICY_BUDGET;
    if(severity==PIXAURA_POLICY_CRITICAL){out.tier=0;out.preview_edge=360;out.runtime_cap_bytes=std::min(out.runtime_cap_bytes,32*mib);}
    const auto& h=p.hardware;
    if(known_normal(o)&&h.gpu_availability==PIXAURA_CAPABILITY_AVAILABLE&&
        o.gpu_availability==PIXAURA_CAPABILITY_AVAILABLE&&h.gpu_validation>=PIXAURA_GPU_RUNTIME_VALIDATED&&h.gpu_backend){
        out.backend=h.gpu_backend;out.forced_cpu=0;
    }else if(out.reason==PIXAURA_POLICY_BASELINE&&h.gpu_backend)out.reason=PIXAURA_POLICY_BACKEND;
    return 0;
}
}
int32_t configure(State& s,const pixaura_capability_profile* p,const pixaura_policy_observation* o,uint64_t limit) {
    if(s.enabled)return 1;
    const auto code=pixaura_capability_validate(1,p);if(code)return code;
    const auto v=valid(o);if(v)return v;
    State next{};next.profile=*p;next.observation=*o;
    const auto d=derive(next,*o,limit,next.strategy);if(d)return d;
    next.strategy.epoch=1;next.enabled=true;s=next;return 0;
}
int32_t update(State& s,const pixaura_policy_observation* o,uint64_t limit) {
    if(!s.enabled)return 1;
    const auto v=valid(o);if(v)return v;
    if(o->sequence<s.observation.sequence)return 9;
    if(o->sequence==s.observation.sequence)return std::memcmp(o,&s.observation,sizeof(*o))==0?0:9;
    State next=s;next.observation=*o;pixaura_execution_strategy candidate{};
    const auto d=derive(s,*o,limit,candidate);if(d)return d;
    auto chosen=s.strategy;
    const bool worse=candidate.tier<chosen.tier||candidate.runtime_cap_bytes<chosen.runtime_cap_bytes||
        candidate.preview_edge<chosen.preview_edge||candidate.forced_cpu>chosen.forced_cpu;
    if(worse){
        chosen.tier=std::min(chosen.tier,candidate.tier);
        chosen.runtime_cap_bytes=std::min(chosen.runtime_cap_bytes,candidate.runtime_cap_bytes);
        chosen.preview_edge=std::min(chosen.preview_edge,candidate.preview_edge);
        if(candidate.forced_cpu){chosen.backend=0;chosen.forced_cpu=1;}
        chosen.reason=candidate.reason==PIXAURA_POLICY_BASELINE?uint32_t(PIXAURA_POLICY_BUDGET):candidate.reason;
        chosen.recovery_pending=chosen.stable_observations=0;next.recovery={};
    }else if(!same(candidate,chosen)){
        if(known_normal(*o)){
            const auto count=s.strategy.recovery_pending&&same(candidate,s.recovery)?s.strategy.stable_observations+1:1;
            if(count>=PIXAURA_POLICY_RECOVERY_OBSERVATIONS){chosen=candidate;next.recovery={};}
            else{chosen.recovery_pending=1;chosen.stable_observations=count;next.recovery=candidate;}
        }else{chosen.recovery_pending=chosen.stable_observations=0;next.recovery={};}
    }else{chosen.reason=candidate.reason;chosen.recovery_pending=chosen.stable_observations=0;next.recovery={};}
    const bool changed=!same(chosen,s.strategy);
    if(changed&&s.strategy.epoch==std::numeric_limits<uint64_t>::max())return 8;
    chosen.epoch=s.strategy.epoch+(changed?1:0);next.strategy=chosen;s=next;return 0;
}
int32_t admit(const State& s,const pixaura_budget_request* request,pixaura_budget_result* output) {
    if(!s.enabled||!request||!output)return 1;
    if(request->version!=1||request->struct_size!=sizeof(*request))return 2;
    auto q=*request;
    // Validate original fields first: clamping must never hide an oversized limit.
    pixaura_budget_result b{};auto p=normalized(s,s.observation);
    const auto finish=[&]{b.tier=s.strategy.tier;b.preview_edge=s.strategy.preview_edge;
        b.runtime_cap_bytes=std::min(b.runtime_cap_bytes,s.strategy.runtime_cap_bytes);
        if(b.decision==PIXAURA_BUDGET_REJECT)b.available_bytes=0;
        *output=b;return 0;};
    const auto code=pixaura_budget_admit(1,&p,&q,&b);if(code)return code;
    if(b.decision==PIXAURA_BUDGET_REJECT)return finish();
    if(!s.strategy.runtime_cap_bytes){b.decision=PIXAURA_BUDGET_REJECT;b.reason=PIXAURA_BUDGET_PROCESS_BUDGET;return finish();}
    q.context_limit_bytes=std::min(q.context_limit_bytes,s.strategy.runtime_cap_bytes);
    if(q.work_class==PIXAURA_BUDGET_PREVIEW)q.preview_edge=std::min(q.preview_edge,s.strategy.preview_edge);
    if(s.strategy.forced_cpu&&known_normal(s.observation)&&p.hardware.gpu_validation>=PIXAURA_GPU_RUNTIME_VALIDATED)
        p.runtime.gpu_availability=PIXAURA_CAPABILITY_UNAVAILABLE;
    const auto result=pixaura_budget_admit(1,&p,&q,&b);if(result)return result;
    return finish();
}
}
