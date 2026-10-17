#ifndef PIXAURA_INFERENCE_ADMISSION_BOUNDARY_HPP
#define PIXAURA_INFERENCE_ADMISSION_BOUNDARY_HPP
#include "capability_fixtures.h"
#include "inference_admission_fixtures.hpp"
namespace pixaura::inference {
inline int admission_certification(){
    int failures=0,checks=0;const auto started=std::chrono::steady_clock::now();uint64_t admission_ns=0,admission_calls=0;
    auto expect=[&](int actual,int expected){++checks;if(actual!=expected){++failures;std::printf("admission check=%d actual=%d expected=%d\n",checks,actual,expected);}};
    auto check=[&](int line,bool value){if(!value)std::printf("admission assertion line=%d\n",line);expect(value?0:1,0);};
    static std::atomic<uint64_t> sequence{1};
    auto init=[&](pixaura_inference_context& c,uint32_t platform,uint64_t limit=268435456){c={};std::array<uint8_t,32> id{};id.fill('6');auto n=sequence.fetch_add(1);const char hex[]="0123456789abcdef";
        for(size_t i=0;i<16;++i){id[31-i]=static_cast<uint8_t>(hex[n&15]);n>>=4;}expect(pixaura_inference_context_init(1,&c,sizeof(c),id.data(),32,limit),0);live(&c).state->admission_test_platform=platform;};
    auto adopt_case=[&](pixaura_inference_context& c,const fixtures::Case& f){
        pixaura::model::Key key{7,2,1,1,pixaura::model::fixtures::key};pixaura_model_package* raw=nullptr;pixaura_model_diagnostic d{};
        expect(pixaura::model::verify_owned(1,{{"manifest.pxm",f.manifest},{"signature.pxs",f.signature},{"reference.onnx",f.payload}},{1,1,2,&key,1},67108864,&raw,&d),0);
        const auto h=live(&c);Entry e{};e.kind=1;e.package=adopt(h,std::unique_ptr<pixaura_model_package>(raw));return insert_locked(h,std::move(e));};
    auto package=[&](pixaura_inference_context& c,size_t index=SIZE_MAX){const auto data=fixtures::cases();return adopt_case(c,data[index==SIZE_MAX?data.size()-1:index]);};
    auto facts=[&](uint32_t platform,unsigned tier){pixaura_capability_profile p{};expect(pixaura_capability_baseline(1,&p),0);const auto& f=capability_fixtures[tier];
        p.hardware.platform=platform;p.hardware.architecture=PIXAURA_ARCH_ARM64;p.hardware.logical_cpu_count=f.cpus;p.hardware.physical_memory_bytes=f.memory;p.hardware.runtime_version=35;
        p.hardware.gpu_backend=platform==PIXAURA_PLATFORM_APPLE?PIXAURA_CAPABILITY_GPU_METAL:PIXAURA_CAPABILITY_GPU_VULKAN;
        p.hardware.gpu_availability=PIXAURA_CAPABILITY_AVAILABLE;p.hardware.gpu_validation=PIXAURA_GPU_RUNTIME_VALIDATED;p.hardware.provenance=PIXAURA_PROVENANCE_SYNTHETIC;
        p.runtime.thermal=p.runtime.memory_pressure=1;p.runtime.constrained=0;p.runtime.process_budget_bytes=f.budget;p.runtime.gpu_availability=2;p.runtime.sequence=1;return p;};
    auto observation=[](const pixaura_capability_profile& p){pixaura_policy_observation o{};o.version=1;o.struct_size=sizeof(o);o.thermal=o.memory_pressure=1;o.gpu_availability=2;o.gpu_validation=1;o.process_budget_bytes=p.runtime.process_budget_bytes;o.sequence=1;return o;};
    auto req=[](uint64_t generation){pixaura_inference_request r{};r.version=1;r.struct_size=sizeof(r);r.task=1;r.generation=generation;r.input=tensor({{1,3,2,2}},{{1000000,0,1000000,0,1000000,0}});return r;};
    std::array<float,12> input{};input.fill(0.5f);
    auto admit=[&](pixaura_inference_context& c,const pixaura_inference_handle& p,const pixaura_inference_request& r,pixaura_inference_handle& a,pixaura_inference_admission_result& result){const auto begin=std::chrono::steady_clock::now();const auto status=pixaura_inference_admit(&c,&p,&r,sizeof(r),reinterpret_cast<const uint8_t*>(input.data()),48,&a,sizeof(a),&result,sizeof(result));admission_ns+=static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()-begin).count());++admission_calls;return status;};
    // Hand-checkable LOW/MID/HIGH x dynamic severity x provider trust x platform.
    unsigned rows=0;
    for(uint32_t platform:{uint32_t(PIXAURA_PLATFORM_ANDROID),uint32_t(PIXAURA_PLATFORM_APPLE)})for(unsigned tier=0;tier<3;++tier)for(bool thermal:{false,true})for(uint32_t pressure=0;pressure<=4;++pressure)for(uint32_t trust=0;trust<=2;++trust){
        pixaura_inference_context c{};init(c,platform);const auto p=package(c);auto f=facts(platform,tier);auto o=observation(f);if(thermal)o.thermal=pressure;else o.memory_pressure=pressure;
        auto& s=*live(&c).state;s.admission_test_backend=true;s.admission_test_availability=2;s.admission_test_validation=trust;s.admission_test_transient=1024;
        pixaura_execution_strategy strategy{};expect(pixaura_inference_policy_configure(1,&c,&f,&o,&strategy),0);
        check(__LINE__,strategy.tier==(pressure==1?tier:0));
        pixaura_inference_handle a{},out{};pixaura_inference_admission_result result{};expect(admit(c,p,req(1),a,result),0);
        const bool accelerated=pressure==1&&trust>=1;check(__LINE__,std::strcmp(result.provider_id,accelerated?(platform==PIXAURA_PLATFORM_APPLE?"apple.metal.v1":"android.vulkan.v1"):"cpu.reference.v1")==0);
        expect(static_cast<int>(result.reason),accelerated?PIXAURA_ADMISSION_PRIMARY:trust==0?PIXAURA_ADMISSION_FALLBACK_TRUST:PIXAURA_ADMISSION_FALLBACK_CAPABILITY);
        check(__LINE__,result.peak_bytes<=result.runtime_cap_bytes&&result.epoch==strategy.epoch);
        expect(pixaura_inference_execute_admitted(&c,&a,&out,sizeof(out)),0);expect(pixaura_inference_admission_current(&c,&out),0);
        std::array<float,4> values{};expect(pixaura_inference_output_copy(&c,&out,reinterpret_cast<uint8_t*>(values.data()),16),0);for(auto v:values)check(__LINE__,v==0.5f);
        expect(pixaura_inference_release(&c,&out),0);check(__LINE__,s.used.load()==control+acquire_locked(live(&c),&p,1).package->lease->bytes);expect(pixaura_inference_context_destroy(&c),0);++rows;
    }
    pixaura_inference_context c{};init(c,PIXAURA_PLATFORM_ANDROID);auto p=package(c);auto f=facts(PIXAURA_PLATFORM_ANDROID,2);auto o=observation(f);pixaura_execution_strategy strategy{};
    auto& s=*live(&c).state;s.admission_test_backend=true;s.admission_test_validation=1;s.admission_test_availability=2;s.admission_test_transient=1024;
    expect(pixaura_inference_policy_configure(1,&c,&f,&o,&strategy),0);uint64_t generation=1;
    auto run=[&](uint32_t availability,uint32_t validation,bool unsupported,uint64_t transient,uint32_t reason){s.admission_test_availability=availability;s.admission_test_validation=validation;s.admission_test_unsupported=unsupported;s.admission_test_transient=transient;
        pixaura_inference_handle a{};pixaura_inference_admission_result result{};expect(admit(c,p,req(generation++),a,result),0);expect(static_cast<int>(result.reason),static_cast<int>(reason));expect(pixaura_inference_release(&c,&a),0);};
    run(1,1,false,1024,PIXAURA_ADMISSION_FALLBACK_UNAVAILABLE);run(0,1,false,1024,PIXAURA_ADMISSION_FALLBACK_UNAVAILABLE);
    run(2,0,false,1024,PIXAURA_ADMISSION_FALLBACK_TRUST);run(2,1,true,1024,PIXAURA_ADMISSION_FALLBACK_MODEL);
    run(2,1,false,268435456,PIXAURA_ADMISSION_FALLBACK_RESOURCE);
    // Exact resource boundary and one-byte-over: GPU falls back without changing input.
    const auto remaining=s.effective_ceiling.load()-s.used.load()-admission_metadata-64;
    run(2,1,false,remaining,PIXAURA_ADMISSION_PRIMARY);run(2,1,false,remaining+1,PIXAURA_ADMISSION_FALLBACK_RESOURCE);
    {pixaura_inference_handle a{};pixaura_inference_admission_result result{};s.admission_test_transient=UINT64_MAX;
        expect(admit(c,p,req(generation++),a,result),11);check(__LINE__,!a.serial&&result.reason==PIXAURA_ADMISSION_REJECT_RESOURCE);}
    s.admission_test_transient=1024;
    // Request errors are rejected before selection, never fallback.
    for(unsigned mode=0;mode<10;++mode){auto r=req(generation++);int code=0;
        switch(mode){case 0:r.input.type=2;code=9;break;case 1:r.input.rank=3;code=8;break;case 2:r.input.dimensions[2]=UINT32_MAX;code=8;break;
        case 3:r.input.bytes=UINT64_MAX;code=8;break;case 4:r.input.normalization[0]++;code=18;break;
        case 5:input[0]=-0.1f;code=10;break;case 6:input[0]=std::numeric_limits<float>::infinity();code=10;break;case 7:input[0]=std::numeric_limits<float>::quiet_NaN();code=10;break;
        case 8:r.pre_cancel=1;code=12;break;default:r.task=2;code=7;}
        pixaura_inference_handle a{};pixaura_inference_admission_result result{};expect(admit(c,p,r,a,result),code);check(__LINE__,!a.serial&&!result.admitted);input[0]=0.5f;
    }
    {auto invalid=p;invalid.context_id[0]='f';pixaura_inference_handle a{};pixaura_inference_admission_result result{};expect(admit(c,invalid,req(generation++),a,result),14);}
    // BUSY is context-wide; no queue or second CPU execution. Cancel wins over BUSY.
    pixaura_inference_handle a{},out{},other{};pixaura_inference_admission_result result{};
    expect(admit(c,p,req(generation++),a,result),0);auto r=req(generation++);expect(admit(c,p,r,other,result),16);r.pre_cancel=1;expect(admit(c,p,r,other,result),12);
    pixaura_inference_handle busy_model{};expect(pixaura_inference_prepare(&c,provider.data(),provider.size(),&p,&busy_model,sizeof(busy_model)),0);r=req(1);
    expect(pixaura_inference_execute(&c,&busy_model,&r,sizeof(r),reinterpret_cast<const uint8_t*>(input.data()),48,&out,sizeof(out)),16);expect(pixaura_inference_release(&c,&busy_model),0);
    expect(pixaura_inference_admission_cancel(&c,&a),0);expect(pixaura_inference_execute_admitted(&c,&a,&out,sizeof(out)),12);expect(pixaura_inference_release(&c,&a),0);expect(pixaura_inference_release(&c,&a),14);
    // Stale pre-execution plan, immediate downgrade, three-sequence recovery.
    expect(admit(c,p,req(generation++),a,result),0);const auto epoch=result.epoch;o.memory_pressure=3;o.sequence++;
    expect(pixaura_inference_policy_update(1,&c,&o,&strategy),0);check(__LINE__,strategy.epoch>epoch&&strategy.runtime_cap_bytes==67108864);
    expect(pixaura_inference_execute_admitted(&c,&a,&out,sizeof(out)),14);expect(pixaura_inference_release(&c,&a),0);
    o.memory_pressure=1;for(unsigned n=1;n<=3;++n){++o.sequence;expect(pixaura_inference_policy_update(1,&c,&o,&strategy),0);std::printf("admission recovery n=%u cap=%llu pending=%u stable=%u\n",n,static_cast<unsigned long long>(strategy.runtime_cap_bytes),strategy.recovery_pending,strategy.stable_observations);check(__LINE__,(strategy.runtime_cap_bytes==268435456)==(n==3));}
    auto malformed=o;malformed.memory_pressure=99;malformed.sequence++;expect(pixaura_inference_policy_update(1,&c,&malformed,&strategy),1);
    malformed=o;malformed.sequence--;expect(pixaura_inference_policy_update(1,&c,&malformed,&strategy),9);
    // Real update during execution at a deterministic checkpoint: no publication.
    struct Race {pixaura_inference_context* context;pixaura_policy_observation observation;int code=-1;bool once=false;};Race race{&c,o};race.observation.memory_pressure=4;++race.observation.sequence;
    s.observer=[](evaluation::Checkpoint phase,void* opaque){auto& v=*static_cast<Race*>(opaque);if(!v.once&&phase==evaluation::Checkpoint::tile){v.once=true;pixaura_execution_strategy next{};v.code=pixaura_inference_policy_update(1,v.context,&v.observation,&next);}};s.observer_state=&race;
    expect(admit(c,p,req(generation++),a,result),0);out={};expect(pixaura_inference_execute_admitted(&c,&a,&out,sizeof(out)),14);check(__LINE__,!out.serial&&race.once);expect(race.code,0);s.observer=nullptr;
    // No post-start replay on INTERNAL; an independently eligible CPU is not run.
    o=race.observation;o.memory_pressure=1;for(unsigned n=0;n<3;++n){++o.sequence;expect(pixaura_inference_policy_update(1,&c,&o,&strategy),0);}
    s.backend_fault=1;const auto calls=s.admission_test_calls;expect(admit(c,p,req(generation++),a,result),0);out={};expect(pixaura_inference_execute_admitted(&c,&a,&out,sizeof(out)),15);check(__LINE__,s.admission_test_calls==calls+1&&!out.serial);s.backend_fault=0;
    s.output_fault=true;expect(admit(c,p,req(generation++),a,result),0);out={};expect(pixaura_inference_execute_admitted(&c,&a,&out,sizeof(out)),13);check(__LINE__,!out.serial);s.output_fault=false;
    // Completed output remains readable, but cannot pass publication eligibility after epoch changes.
    expect(admit(c,p,req(generation++),a,result),0);expect(pixaura_inference_execute_admitted(&c,&a,&out,sizeof(out)),0);o.constrained=1;++o.sequence;
    expect(pixaura_inference_policy_update(1,&c,&o,&strategy),0);expect(pixaura_inference_admission_current(&c,&out),14);std::array<float,4> values{};expect(pixaura_inference_output_copy(&c,&out,reinterpret_cast<uint8_t*>(values.data()),16),0);expect(pixaura_inference_release(&c,&out),0);
    // Neither fits after severe process-budget shrink; no underflow/optimistic overcommit.
    o.process_budget_bytes=1;++o.sequence;expect(pixaura_inference_policy_update(1,&c,&o,&strategy),0);a={};expect(admit(c,p,req(generation++),a,result),11);check(__LINE__,!a.serial);expect(pixaura_inference_context_destroy(&c),0);
    // Two threads cannot individually admit overlapping requests in one context.
    init(c,PIXAURA_PLATFORM_ANDROID);p=package(c);f=facts(PIXAURA_PLATFORM_ANDROID,1);o=observation(f);expect(pixaura_inference_policy_configure(1,&c,&f,&o,&strategy),0);
    std::atomic<bool> start{false};int statuses[2]{-1,-1};pixaura_inference_handle plans[2]{};pixaura_inference_admission_result results[2]{};
    auto compete=[&](unsigned i){while(!start.load())std::this_thread::yield();const auto r=req(1);statuses[i]=pixaura_inference_admit(&c,&p,&r,sizeof(r),reinterpret_cast<const uint8_t*>(input.data()),48,&plans[i],sizeof(plans[i]),&results[i],sizeof(results[i]));};std::thread first(compete,0),second(compete,1);start=true;first.join();second.join();
    check(__LINE__,(statuses[0]==0&&statuses[1]==14)||(statuses[0]==14&&statuses[1]==0));for(auto plan:plans)if(plan.serial)expect(pixaura_inference_release(&c,&plan),0);
    expect(pixaura_inference_context_destroy(&c),0);
    // Context isolation: two simultaneous reservations, release/cancel only A.
    pixaura_inference_context independent{};init(c,PIXAURA_PLATFORM_ANDROID);init(independent,PIXAURA_PLATFORM_APPLE);
    p=package(c);const auto pb=package(independent);f=facts(PIXAURA_PLATFORM_ANDROID,0);o=observation(f);expect(pixaura_inference_policy_configure(1,&c,&f,&o,&strategy),0);
    f=facts(PIXAURA_PLATFORM_APPLE,0);o=observation(f);expect(pixaura_inference_policy_configure(1,&independent,&f,&o,&strategy),0);
    pixaura_inference_handle ab{};expect(admit(c,p,req(1),a,result),0);expect(admit(independent,pb,req(1),ab,result),0);
    const auto reserved=live(&independent).state->used.load();expect(pixaura_inference_admission_cancel(&c,&a),0);expect(pixaura_inference_release(&c,&a),0);check(__LINE__,reserved==live(&independent).state->used.load());
    expect(pixaura_inference_execute_admitted(&independent,&ab,&out,sizeof(out)),0);expect(pixaura_inference_release(&independent,&out),0);
    expect(pixaura_inference_context_destroy(&c),0);expect(pixaura_inference_context_destroy(&independent),0);
    // Production descriptor trust remains independent of platform facts, even
    // when those facts permit a backend. No synthetic backend in this context.
    init(c,native_platform());p=package(c);f=facts(native_platform(),1);o=observation(f);
    auto bad_profile=f;bad_profile.hardware.gpu_validation=99;expect(pixaura_inference_policy_configure(1,&c,&bad_profile,&o,&strategy),1);
    bad_profile=f;bad_profile.version=99;expect(pixaura_inference_policy_configure(1,&c,&bad_profile,&o,&strategy),2);
    expect(pixaura_inference_policy_configure(1,&c,&f,&o,&strategy),0);expect(admit(c,p,req(1),a,result),0);
    check(__LINE__,std::strcmp(result.provider_id,"cpu.reference.v1")==0);input[0]=1.0f;
    expect(pixaura_inference_execute_admitted(&c,&a,&out,sizeof(out)),0);input[0]=0.5f;std::array<float,4> immutable{};
    expect(pixaura_inference_output_copy(&c,&out,reinterpret_cast<uint8_t*>(immutable.data()),16),0);check(__LINE__,immutable[0]==0.5f);
    expect(pixaura_inference_admission_current(&c,&out),0);expect(admit(c,p,req(2),a,result),0);expect(pixaura_inference_admission_current(&c,&out),14);
    expect(pixaura_inference_release(&c,&a),0);expect(pixaura_inference_release(&c,&out),0);
    pixaura_inference_handle explicit_model{};expect(pixaura_inference_prepare(&c,provider.data(),provider.size(),&p,&explicit_model,sizeof(explicit_model)),0);
    expect(admit(c,explicit_model,req(2),a,result),3);expect(pixaura_inference_release(&c,&explicit_model),0);
    const auto unsupported_cases=fixtures::cases();for(size_t index=0;index<unsupported_cases.size();++index)if(unsupported_cases[index].status==5){
        const auto bad=package(c,index);const auto binding=acquire_locked(live(&c),&bad,1).package->value->binding;auto bad_request=req(3);bad_request.input=tensor(binding.input,binding.normalization);
        std::vector<float> pixels(static_cast<size_t>(bad_request.input.bytes/4),0.5f);a={};
        expect(pixaura_inference_admit(&c,&bad,&bad_request,sizeof(bad_request),reinterpret_cast<const uint8_t*>(pixels.data()),bad_request.input.bytes,&a,sizeof(a),&result,sizeof(result)),5);
        check(__LINE__,!a.serial&&result.reason==PIXAURA_ADMISSION_REJECT_MODEL);expect(pixaura_inference_release(&c,&bad),0);break;
    }
    // Read-only policy and accounting overhead, separate from provider execution.
    auto sample=std::chrono::steady_clock::now();for(unsigned n=0;n<16;++n)expect(pixaura_inference_policy_query(1,&c,&strategy),0);
    const auto policy_ns=std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()-sample).count();
    sample=std::chrono::steady_clock::now();for(unsigned n=0;n<16;++n){pixaura_inference_resources resources{};expect(pixaura_inference_resources_query(&c,&resources,sizeof(resources)),0);}
    const auto accounting_ns=std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()-sample).count();
    std::printf("Inference admission read-only overhead: samples=16 policy_total_ns=%lld accounting_total_ns=%lld\n",static_cast<long long>(policy_ns),static_cast<long long>(accounting_ns));
    expect(pixaura_inference_context_destroy(&c),0);
    for(const auto& requirement:fixtures::admission_cases()){
        init(c,native_platform());p=adopt_case(c,requirement);f=facts(native_platform(),1);o=observation(f);expect(pixaura_inference_policy_configure(1,&c,&f,&o,&strategy),0);
        a={};expect(admit(c,p,req(1),a,result),requirement.status);if(!requirement.status){check(__LINE__,result.reservation_bytes>=16777216);expect(pixaura_inference_release(&c,&a),0);}else check(__LINE__,!a.serial);
        expect(pixaura_inference_context_destroy(&c),0);
    }
    // Required certification can be satisfied only by a SYNTHETIC certified
    // provider in this test adapter, never by caller/platform trust assertions.
    init(c,PIXAURA_PLATFORM_ANDROID);p=adopt_case(c,fixtures::admission_cases()[4]);f=facts(PIXAURA_PLATFORM_ANDROID,1);o=observation(f);
    auto& certified=*live(&c).state;certified.admission_test_backend=true;certified.admission_test_validation=2;certified.admission_test_availability=2;
    expect(pixaura_inference_policy_configure(1,&c,&f,&o,&strategy),0);expect(admit(c,p,req(1),a,result),0);check(__LINE__,result.reason==PIXAURA_ADMISSION_PRIMARY);
    expect(pixaura_inference_release(&c,&a),0);expect(pixaura_inference_context_destroy(&c),0);
    // Admission and explicit CPU share the SAME frozen normalization authority.
    init(c,native_platform());p=package(c,4);f=facts(native_platform(),1);o=observation(f);expect(pixaura_inference_policy_configure(1,&c,&f,&o,&strategy),0);
    pixaura_inference_handle normalized_model{},explicit_output{};expect(pixaura_inference_prepare(&c,provider.data(),provider.size(),&p,&normalized_model,sizeof(normalized_model)),0);
    pixaura_inference_model_metadata metadata{};expect(pixaura_inference_model_query(&c,&normalized_model,&metadata,sizeof(metadata)),0);r=req(1);r.input=metadata.input;
    std::vector<float> normalized_input(static_cast<size_t>(metadata.input.bytes/4),0.5f);
    expect(pixaura_inference_execute(&c,&normalized_model,&r,sizeof(r),reinterpret_cast<const uint8_t*>(normalized_input.data()),metadata.input.bytes,&explicit_output,sizeof(explicit_output)),0);
    expect(pixaura_inference_admit(&c,&p,&r,sizeof(r),reinterpret_cast<const uint8_t*>(normalized_input.data()),metadata.input.bytes,&a,sizeof(a),&result,sizeof(result)),0);
    expect(pixaura_inference_execute_admitted(&c,&a,&out,sizeof(out)),0);std::vector<float> direct(64),automatic(64);
    expect(pixaura_inference_output_copy(&c,&explicit_output,reinterpret_cast<uint8_t*>(direct.data()),256),0);expect(pixaura_inference_output_copy(&c,&out,reinterpret_cast<uint8_t*>(automatic.data()),256),0);
    for(size_t n=0;n<64;++n)check(__LINE__,direct[n]==0.4375f&&direct[n]==automatic[n]);expect(pixaura_inference_context_destroy(&c),0);
    // Epoch exhaustion fails closed without wrapping, and no new reservation.
    init(c,native_platform());p=package(c);f=facts(native_platform(),1);o=observation(f);expect(pixaura_inference_policy_configure(1,&c,&f,&o,&strategy),0);
    expect(admit(c,p,req(1),a,result),0);live(&c).state->policy.strategy.epoch=UINT64_MAX;o.constrained=1;++o.sequence;
    expect(pixaura_inference_policy_update(1,&c,&o,&strategy),8);out={};expect(pixaura_inference_execute_admitted(&c,&a,&out,sizeof(out)),14);check(__LINE__,!out.serial);
    expect(pixaura_inference_release(&c,&a),0);a={};expect(admit(c,p,req(2),a,result),11);check(__LINE__,!a.serial);expect(pixaura_inference_context_destroy(&c),0);
    const auto elapsed=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-started).count();
    std::printf("Inference admission: matrix_rows=%u checks=%d failures=%d total_us=%lld admission_calls=%llu admission_total_ns=%llu; SYNTHETIC trust/platform only; no certification grant\n",rows,checks,failures,static_cast<long long>(elapsed),static_cast<unsigned long long>(admission_calls),static_cast<unsigned long long>(admission_ns));
    return failures;
}
}
#endif
