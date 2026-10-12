#ifndef PIXAURA_DYNAMIC_POLICY_BOUNDARY_H
#define PIXAURA_DYNAMIC_POLICY_BOUNDARY_H
#include "budget_boundary.h"
#include "pixaura/dynamic_policy.h"
static pixaura_policy_observation policy_observation(const pixaura_capability_profile* p) {
    pixaura_policy_observation o;
    memset(&o,0,sizeof(o));o.version=1;o.struct_size=sizeof(o);o.sequence=1;
    o.thermal=o.memory_pressure=PIXAURA_POLICY_NORMAL;
    o.gpu_availability=PIXAURA_CAPABILITY_UNAVAILABLE;o.process_budget_bytes=p->runtime.process_budget_bytes;
    o.gpu_validation=p->hardware.gpu_validation;
    return o;
}
static int dynamic_policy_boundary_check(void) {
#define POLICY_CHECK(c) do { if(!(c)) return __LINE__; } while(0)
    const uint64_t mib=UINT64_C(1048576);
    unsigned tier,signal,severity,i;
    for(tier=0;tier<3;++tier){
        pixaura_decode_context c;pixaura_decode_limits limits;memset(&c,0,sizeof(c));
        pixaura_capability_profile p=budget_profile(tier),saved=p;
        pixaura_policy_observation o=policy_observation(&p),invalid;
        pixaura_execution_strategy s,prior;
        pixaura_budget_request q=budget_request();pixaura_budget_result b;uint64_t epoch=0;
        uint8_t identity[32];memset(identity,'a'+tier,32);
        POLICY_CHECK(pixaura_decode_default_limits(1,&limits)==0);
        POLICY_CHECK(pixaura_decode_context_init(1,&c,sizeof(c),identity,32,&limits)==0);
        POLICY_CHECK(pixaura_policy_configure(1,&c,&p,&o,&s)==0);
        POLICY_CHECK(s.tier==tier&&s.epoch==1&&s.concurrency==1&&s.backend==0);
        POLICY_CHECK(s.runtime_cap_bytes==(tier==0?96*mib:(tier==1?192*mib:256*mib)));
        POLICY_CHECK(pixaura_policy_configure(1,&c,&p,&o,&s)==1);
        POLICY_CHECK(pixaura_policy_admit(1,&c,&q,&b,&epoch)==0&&b.decision==0&&epoch==s.epoch);
        POLICY_CHECK(b.peak_bytes==1179648);
        for(signal=0;signal<2;++signal)for(severity=2;severity<=4;++severity){
            ++o.sequence;o.thermal=signal==0?severity:1;o.memory_pressure=signal==1?severity:1;
            prior=s;POLICY_CHECK(pixaura_policy_update(1,&c,&o,&s)==0);
            POLICY_CHECK(s.tier==0&&s.runtime_cap_bytes<=prior.runtime_cap_bytes&&s.forced_cpu==1);
            POLICY_CHECK(s.preview_edge==(severity==4?360u:720u));
            POLICY_CHECK(s.runtime_cap_bytes<=((severity==4?32:(severity==3?64:128))*mib));
            prior=s;POLICY_CHECK(pixaura_policy_update(1,&c,&o,&s)==0&&memcmp(&prior,&s,sizeof(s))==0);
            o.thermal=o.memory_pressure=1;
            for(i=0;i<3;++i){++o.sequence;POLICY_CHECK(pixaura_policy_update(1,&c,&o,&s)==0);
                if(i<2){POLICY_CHECK(s.epoch==prior.epoch&&s.runtime_cap_bytes==prior.runtime_cap_bytes);
                    q=budget_request();q.temporary_bytes=200*mib;
                    POLICY_CHECK(pixaura_policy_admit(1,&c,&q,&b,&epoch)==0&&b.decision==PIXAURA_BUDGET_REJECT&&b.runtime_cap_bytes<=s.runtime_cap_bytes);
                    q=budget_request();}}
            POLICY_CHECK(s.tier==tier&&s.preview_edge==(tier==0?720u:(tier==1?1280u:2048u)));
        }
        ++o.sequence;o.thermal=2;o.memory_pressure=3;o.constrained=1;
        POLICY_CHECK(pixaura_policy_update(1,&c,&o,&s)==0&&s.reason==PIXAURA_POLICY_MEMORY&&s.runtime_cap_bytes<=64*mib);
        ++o.sequence;o.thermal=4;o.memory_pressure=2;o.constrained=0;
        POLICY_CHECK(pixaura_policy_update(1,&c,&o,&s)==0&&s.reason==PIXAURA_POLICY_THERMAL&&s.runtime_cap_bytes<=32*mib);
        o.thermal=o.memory_pressure=1;
        for(i=0;i<3;++i){++o.sequence;POLICY_CHECK(pixaura_policy_update(1,&c,&o,&s)==0);}
        POLICY_CHECK(s.tier==tier);
        ++o.sequence;o.constrained=1;POLICY_CHECK(pixaura_policy_update(1,&c,&o,&s)==0&&s.tier==0);
        ++o.sequence;o.constrained=0;o.thermal=0;prior=s;
        POLICY_CHECK(pixaura_policy_update(1,&c,&o,&s)==0&&s.runtime_cap_bytes<=prior.runtime_cap_bytes&&s.tier==0);
        ++o.sequence;o.memory_pressure=4;o.thermal=1;
        POLICY_CHECK(pixaura_policy_update(1,&c,&o,&s)==0&&s.preview_edge==360&&s.runtime_cap_bytes<=32*mib);
        ++o.sequence;o.memory_pressure=1;o.process_budget_bytes=33*mib;
        POLICY_CHECK(pixaura_policy_update(1,&c,&o,&s)==0&&s.runtime_cap_bytes<=mib);
        POLICY_CHECK(pixaura_policy_admit(1,&c,&q,&b,&epoch)==0&&b.decision==PIXAURA_BUDGET_REJECT);
        ++o.sequence;o.process_budget_bytes=p.runtime.process_budget_bytes;
        for(i=0;i<3;++i){++o.sequence;POLICY_CHECK(pixaura_policy_update(1,&c,&o,&s)==0);}
        POLICY_CHECK(s.tier==tier);
        q.width=UINT64_MAX;q.height=2;
        POLICY_CHECK(pixaura_policy_admit(1,&c,&q,&b,&epoch)==0&&b.reason==PIXAURA_BUDGET_OVERFLOW);
        q=budget_request();q.width=16385;
        POLICY_CHECK(pixaura_policy_admit(1,&c,&q,&b,&epoch)==0&&b.reason==PIXAURA_BUDGET_HARD_CEILING);
        prior=s;invalid=o;invalid.thermal=5;++invalid.sequence;
        POLICY_CHECK(pixaura_policy_update(1,&c,&invalid,&s)==1&&memcmp(&s,&prior,sizeof(s))==0);
        invalid=o;invalid.version=2;
        POLICY_CHECK(pixaura_policy_update(1,&c,&invalid,&s)==2&&memcmp(&s,&prior,sizeof(s))==0);
        invalid=o;++invalid.sequence;invalid.memory_pressure=5;POLICY_CHECK(pixaura_policy_update(1,&c,&invalid,&s)==1);
        invalid=o;++invalid.sequence;invalid.reserved=1;POLICY_CHECK(pixaura_policy_update(1,&c,&invalid,&s)==1);
        invalid=o;invalid.sequence=0;POLICY_CHECK(pixaura_policy_update(1,&c,&invalid,&s)==1);
        invalid=o;invalid.struct_size=0;POLICY_CHECK(pixaura_policy_update(1,&c,&invalid,&s)==2);
        invalid=o;--invalid.sequence;POLICY_CHECK(pixaura_policy_update(1,&c,&invalid,&s)==9);
        invalid=o;invalid.constrained=1;POLICY_CHECK(pixaura_policy_update(1,&c,&invalid,&s)==9);
        for(i=0;i<10000;++i){++o.sequence;o.memory_pressure=i%4==0?3:1;
            POLICY_CHECK(pixaura_policy_update(1,&c,&o,&s)==0&&s.tier<=tier&&s.concurrency==1&&s.runtime_cap_bytes<=256*mib);}
        POLICY_CHECK(memcmp(&p,&saved,sizeof(p))==0);
        POLICY_CHECK(pixaura_preview_stop(&c)==0&&pixaura_decode_context_destroy(&c)==0);
        POLICY_CHECK(pixaura_policy_query(1,&c,&s)==3);
    }
    { // Virtual Metal trust is not physical certification; loss/restoration is dynamic only.
        pixaura_decode_context c;pixaura_decode_limits limits;uint8_t identity[32];memset(&c,0,sizeof(c));
        pixaura_capability_profile p=budget_profile(2);
        p.hardware.gpu_backend=PIXAURA_CAPABILITY_GPU_METAL;p.hardware.gpu_availability=2;
        p.hardware.gpu_validation=PIXAURA_GPU_RUNTIME_VALIDATED;p.hardware.provenance=PIXAURA_PROVENANCE_VIRTUAL;
        p.runtime.gpu_availability=2;
        pixaura_policy_observation o=policy_observation(&p);pixaura_execution_strategy s;
        o.gpu_availability=2;memset(identity,'e',32);
        POLICY_CHECK(pixaura_decode_default_limits(1,&limits)==0&&pixaura_decode_context_init(1,&c,sizeof(c),identity,32,&limits)==0);
        POLICY_CHECK(pixaura_policy_configure(1,&c,&p,&o,&s)==0&&s.backend==2&&s.forced_cpu==0);
        ++o.sequence;o.gpu_availability=1;POLICY_CHECK(pixaura_policy_update(1,&c,&o,&s)==0&&s.backend==0&&s.epoch==2);
        o.gpu_availability=2;
        for(i=0;i<3;++i){++o.sequence;POLICY_CHECK(pixaura_policy_update(1,&c,&o,&s)==0);if(i<2)POLICY_CHECK(s.backend==0);}
        POLICY_CHECK(s.backend==2&&s.epoch==3&&p.hardware.gpu_validation==PIXAURA_GPU_RUNTIME_VALIDATED);
        ++o.sequence;o.gpu_validation=0;POLICY_CHECK(pixaura_policy_update(1,&c,&o,&s)==0&&s.backend==0);
        {pixaura_budget_request q=budget_request();pixaura_budget_result b;uint64_t epoch=0;
            q.work_class=PIXAURA_BUDGET_GPU_DIAGNOSTIC;q.prefer_gpu=q.allow_cpu_fallback=1;
            POLICY_CHECK(pixaura_policy_admit(1,&c,&q,&b,&epoch)==0&&b.backend==0&&b.reason==PIXAURA_BUDGET_BACKEND_UNVALIDATED);
            ++o.sequence;o.gpu_validation=2;POLICY_CHECK(pixaura_policy_update(1,&c,&o,&s)==1);
        }
        POLICY_CHECK(pixaura_decode_context_destroy(&c)==0);
    }
    { // A legal context smaller than the control reserve still cannot raise cap.
        pixaura_decode_context c;pixaura_decode_limits limits;uint8_t identity[32];memset(&c,0,sizeof(c));
        pixaura_capability_profile p=budget_profile(2);pixaura_policy_observation o=policy_observation(&p);
        pixaura_execution_strategy s;pixaura_budget_request q=budget_request();pixaura_budget_result b;uint64_t epoch=0;
        memset(identity,'f',32);POLICY_CHECK(pixaura_decode_default_limits(1,&limits)==0);limits.context_bytes=512*1024;
        POLICY_CHECK(pixaura_decode_context_init(1,&c,sizeof(c),identity,32,&limits)==0);
        POLICY_CHECK(pixaura_policy_configure(1,&c,&p,&o,&s)==0&&s.runtime_cap_bytes==512*1024);
        q.context_limit_bytes=limits.context_bytes;
        POLICY_CHECK(pixaura_policy_admit(1,&c,&q,&b,&epoch)==0&&b.decision==PIXAURA_BUDGET_REJECT&&b.runtime_cap_bytes<=512*1024);
        POLICY_CHECK(pixaura_decode_context_destroy(&c)==0);
    }
    { // Baseline process observation qualifies only T1 despite high CPU/RAM.
        pixaura_decode_context c;pixaura_decode_limits limits;uint8_t identity[32];memset(&c,0,sizeof(c));
        pixaura_capability_profile p=budget_profile(2);p.runtime.process_budget_bytes=256*mib;
        pixaura_policy_observation o=policy_observation(&p);pixaura_execution_strategy s;
        memset(identity,'d',32);
        POLICY_CHECK(pixaura_decode_default_limits(1,&limits)==0&&pixaura_decode_context_init(1,&c,sizeof(c),identity,32,&limits)==0);
        POLICY_CHECK(pixaura_policy_configure(1,&c,&p,&o,&s)==0&&s.tier==1);
        o.process_budget_bytes=1024*mib;
        for(i=0;i<3;++i){++o.sequence;POLICY_CHECK(pixaura_policy_update(1,&c,&o,&s)==0&&s.tier==1&&s.runtime_cap_bytes==192*mib);}
        POLICY_CHECK(pixaura_decode_context_destroy(&c)==0);
    }
    return 0;
#undef POLICY_CHECK
}
#endif
