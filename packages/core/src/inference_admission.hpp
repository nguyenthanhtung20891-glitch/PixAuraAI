// Private implementation in inference.cpp: one registry, ownership and authority.
namespace pixaura::inference {
namespace {
constexpr uint64_t admission_metadata=8192;
static_assert(sizeof(pixaura_inference_admission_result)==128,"admission result ABI");
uint32_t native_platform(){
#ifdef __ANDROID__
    return PIXAURA_PLATFORM_ANDROID;
#elif defined(__APPLE__)
    return PIXAURA_PLATFORM_APPLE;
#elif defined(_WIN32)
    return PIXAURA_PLATFORM_WINDOWS;
#elif defined(__linux__)
    return PIXAURA_PLATFORM_LINUX;
#else
    return PIXAURA_PLATFORM_UNKNOWN;
#endif
}
uint32_t admission_platform(const State& s){
#ifdef PIXAURA_CERTIFICATION_TESTING
    if(s.admission_test_platform)return s.admission_test_platform;
#else
    (void)s;
#endif
    return native_platform();
}
void apply_policy_cap(State& s){
    // Phase 4's 1 MiB control/safety reserve includes our existing 128 KiB.
    const auto extra=PIXAURA_POLICY_CONTROL_RESERVE_BYTES-control;
    const auto cap=s.policy_stopped?0:s.policy.strategy.runtime_cap_bytes;
    std::lock_guard<std::mutex> resource_lock(s.resource_mutex);
    s.effective_ceiling=cap>extra?cap-extra:0;
}
void revoke_admissions(State& s){for(auto& item:s.entries)if(item.second.admission)item.second.admission->model->policy_revoked=true;}
// Return Phase 4 codes for policy calls. Inference's codes are kept separate.
template<class F> int32_t policy_boundary(F&& f)noexcept{
    try{return f();}catch(const Error& e){return e.code==PIXAURA_INFERENCE_STALE_HANDLE?3:1;}catch(...){return 8;}
}
uint64_t checked_sum(uint64_t a,uint64_t b){require(b<=UINT64_MAX-a,PIXAURA_INFERENCE_RESOURCE);return a+b;}
uint64_t execution_reservation(uint64_t exact,const pixaura_model_package& value){
    const auto declared=value.info.declared_memory_bytes;
    return std::max(exact,declared>admission_metadata?declared-admission_metadata:0);
}
uint32_t rejection_reason(int32_t status){
    switch(status){
    case PIXAURA_INFERENCE_RESOURCE:return PIXAURA_ADMISSION_REJECT_RESOURCE;
    case PIXAURA_INFERENCE_CANCELLED:return PIXAURA_ADMISSION_REJECT_CANCELLED;
    case PIXAURA_INFERENCE_STALE_HANDLE:return PIXAURA_ADMISSION_REJECT_STALE;
    case PIXAURA_INFERENCE_BUSY:return PIXAURA_ADMISSION_REJECT_BUSY;
    case PIXAURA_INFERENCE_PROVIDER_UNAVAILABLE:return PIXAURA_ADMISSION_REJECT_TRUST;
    case PIXAURA_INFERENCE_UNVERIFIED_MODEL:return PIXAURA_ADMISSION_REJECT_SECURITY;
    case PIXAURA_INFERENCE_MODEL_UNSUPPORTED:case PIXAURA_INFERENCE_MODEL_INVALID:case PIXAURA_INFERENCE_TASK_UNSUPPORTED:return PIXAURA_ADMISSION_REJECT_MODEL;
    default:return PIXAURA_ADMISSION_REJECT_INPUT;
    }
}
void validate_admission_input(const pixaura_inference_request& r,const pixaura_inference_tensor& expected,const uint8_t* bytes,uint64_t n){
    require(r.task==1,PIXAURA_INFERENCE_TASK_UNSUPPORTED);require(r.generation&&r.pre_cancel<=1);
    require(!r.pre_cancel,PIXAURA_INFERENCE_CANCELLED);
    const auto& t=r.input;require(t.version==1&&t.struct_size==sizeof(t)&&!t.reserved,PIXAURA_INFERENCE_UNSUPPORTED_ABI);
    require(t.type==1&&t.layout==1,PIXAURA_INFERENCE_INPUT_TYPE);
    require(t.rank==4&&!std::memcmp(t.dimensions,expected.dimensions,sizeof(t.dimensions))&&t.bytes==expected.bytes&&n==expected.bytes,PIXAURA_INFERENCE_INPUT_SHAPE);
    require(!std::memcmp(t.normalization,expected.normalization,sizeof(t.normalization)),PIXAURA_INFERENCE_NORMALIZATION);
    require(bytes);for(uint64_t offset=0;offset<n;offset+=4){float value;std::memcpy(&value,bytes+offset,4);require(valid_probabilities(&value,1),PIXAURA_INFERENCE_INPUT_RANGE);}
}
#ifdef PIXAURA_CERTIFICATION_TESTING
struct AdmissionTestExecution final:PreparedExecution {
    std::unique_ptr<const PreparedExecution> cpu;State& owner;
    AdmissionTestExecution(std::unique_ptr<const PreparedExecution> p,State& s):cpu(std::move(p)),owner(s){input=cpu->input;output=cpu->output;}
    uint64_t transient_bytes()const override{return owner.admission_test_transient;}
    void run(const float* in,float* out,const ExecutionControl& c)const override{++owner.admission_test_calls;if(owner.backend_fault)throw Error{15};cpu->run(in,out,c);}
};
#endif
}
}
extern "C" int32_t pixaura_inference_policy_configure(uint32_t v,pixaura_inference_context* c,const pixaura_capability_profile* p,const pixaura_policy_observation* o,pixaura_execution_strategy* out){return policy_boundary([&]{
    if(v!=1){return 2;}
    if(!p||!o||!out){return 1;}
    const auto h=live(c);
    separate(out,sizeof(*out),c,sizeof(*c));separate(out,sizeof(*out),p,sizeof(*p));separate(out,sizeof(*out),o,sizeof(*o));
    std::lock_guard<std::mutex> lock(h.state->mutex);auto& s=*h.state;
    if(s.executing||s.pending||p->hardware.platform!=admission_platform(s))return 1;
    const auto code=pixaura::policy::configure(s.policy,p,o,s.ceiling);if(code)return code;
    apply_policy_cap(s);*out=s.policy.strategy;return 0;
});}
extern "C" int32_t pixaura_inference_policy_update(uint32_t v,pixaura_inference_context* c,const pixaura_policy_observation* o,pixaura_execution_strategy* out){return policy_boundary([&]{
    if(v!=1){return 2;}
    if(!o||!out){return 1;}
    const auto h=live(c);
    separate(out,sizeof(*out),c,sizeof(*c));separate(out,sizeof(*out),o,sizeof(*o));
    std::lock_guard<std::mutex> lock(h.state->mutex);auto& s=*h.state;if(s.policy_stopped)return 8;
    const auto old=s.policy.strategy.epoch;const auto code=pixaura::policy::update(s.policy,o,s.ceiling);
    if(code){if(code==8){s.policy_stopped=true;apply_policy_cap(s);revoke_admissions(s);}return code;}
    apply_policy_cap(s);if(old!=s.policy.strategy.epoch)revoke_admissions(s);*out=s.policy.strategy;return 0;
});}
extern "C" int32_t pixaura_inference_policy_query(uint32_t v,pixaura_inference_context* c,pixaura_execution_strategy* out){return policy_boundary([&]{
    if(v!=1){return 2;}
    if(!out){return 1;}
    const auto h=live(c);separate(out,sizeof(*out),c,sizeof(*c));
    std::lock_guard<std::mutex> lock(h.state->mutex);if(!h.state->policy.enabled)return 1;if(h.state->policy_stopped)return 8;*out=h.state->policy.strategy;return 0;
});}
extern "C" int32_t pixaura_inference_admit(pixaura_inference_context* c,const pixaura_inference_handle* package,const pixaura_inference_request* request,uint32_t request_size,const uint8_t* input,uint64_t bytes,pixaura_inference_handle* out,uint32_t size,pixaura_inference_admission_result* result,uint32_t result_size){return boundary([&]{
    const auto h=live(c);output_pointer(h,c,out,size,sizeof(*out));output_pointer(h,c,result,result_size,sizeof(*result));
    require(package&&request&&request_size==sizeof(*request),PIXAURA_INFERENCE_UNSUPPORTED_ABI);
    require(request->version==1&&request->struct_size==sizeof(*request),PIXAURA_INFERENCE_UNSUPPORTED_ABI);
    separate(out,size,result,result_size);separate(out,size,package,sizeof(*package));separate(result,result_size,package,sizeof(*package));
    separate(out,size,request,sizeof(*request));separate(result,result_size,request,sizeof(*request));
    separate(out,size,input,bytes);separate(result,result_size,input,bytes);separate(input,bytes,c,sizeof(*c));
    std::lock_guard<std::mutex> lock(h.state->mutex);auto& s=*h.state;
    pixaura_inference_admission_result r{};r.version=1;r.struct_size=sizeof(r);r.generation=request->generation;
    try{
        require(s.policy.enabled,PIXAURA_INFERENCE_INVALID_ARGUMENT);require(!s.policy_stopped,PIXAURA_INFERENCE_RESOURCE);
        r.epoch=s.policy.strategy.epoch;r.tier=s.policy.strategy.tier;r.runtime_cap_bytes=s.policy.strategy.runtime_cap_bytes;
        const auto p=acquire_locked(h,package,1).package;const auto& value=*p->value;
        require(value.info.task==1,PIXAURA_INFERENCE_TASK_UNSUPPORTED);
        const auto expected=tensor(value.binding.input,value.binding.normalization);
        validate_admission_input(*request,expected,input,bytes);
        require(s.policy.strategy.tier>=value.binding.minimum_tier&&s.policy.profile.hardware.logical_cpu_count>=value.binding.cpu_threads,PIXAURA_INFERENCE_RESOURCE);
        require(request->generation>s.last_admission_generation,PIXAURA_INFERENCE_STALE_HANDLE);
        require(!s.executing&&!s.pending,PIXAURA_INFERENCE_BUSY);
        auto metadata=std::make_shared<Lease>(s,admission_metadata);
        auto prepared=prepare_cpu(value);require(prepared->input==value.binding.input&&prepared->output==value.binding.output,PIXAURA_INFERENCE_MODEL_UNSUPPORTED);
        const auto output_tensor=tensor(prepared->output,{{0,0,0,0,0,0}});
        const auto cpu_bytes=checked_sum(expected.bytes,output_tensor.bytes);
        pixaura_inference_descriptor reference{};require(pixaura_inference_descriptor_query(1,provider.data(),provider.size(),&reference,sizeof(reference))==0,PIXAURA_INFERENCE_INTERNAL);
        std::string_view selected=provider;uint32_t reason=PIXAURA_ADMISSION_CPU_ONLY;
        const auto platform=admission_platform(s);
        const std::string_view primary=platform==PIXAURA_PLATFORM_ANDROID?"android.vulkan.v1":platform==PIXAURA_PLATFORM_APPLE?"apple.metal.v1":"";
        if(!primary.empty()){
            pixaura_inference_descriptor d{};const auto status=pixaura_inference_descriptor_query(1,primary.data(),primary.size(),&d,sizeof(d));
#ifdef PIXAURA_CERTIFICATION_TESTING
            if(s.admission_test_backend){d=reference;d.availability=s.admission_test_availability;d.validation=s.admission_test_validation;}
#endif
            bool present=status==0;
#ifdef PIXAURA_CERTIFICATION_TESTING
            present=present||s.admission_test_backend;
#endif
            if(!present||d.availability!=PIXAURA_CAPABILITY_AVAILABLE)reason=PIXAURA_ADMISSION_FALLBACK_UNAVAILABLE;
            else if(d.validation<std::max(uint32_t(PIXAURA_GPU_RUNTIME_VALIDATED),value.binding.required_validation))reason=PIXAURA_ADMISSION_FALLBACK_TRUST;
            else if(s.policy.strategy.forced_cpu||s.policy.strategy.backend!=(platform==PIXAURA_PLATFORM_ANDROID?1u:2u))reason=PIXAURA_ADMISSION_FALLBACK_CAPABILITY;
            else if(d.provider_version!=1||d.task!=request->task||d.type!=expected.type||d.layout!=expected.layout||
                expected.dimensions[2]>d.max_dimension||expected.dimensions[3]>d.max_dimension||std::memcmp(d.profile_id,reference.profile_id,sizeof(d.profile_id)))reason=PIXAURA_ADMISSION_FALLBACK_MODEL;
            else{
                std::unique_ptr<const PreparedExecution> accelerated;
#ifdef PIXAURA_CERTIFICATION_TESTING
                if(s.admission_test_backend){if(s.admission_test_unsupported)reason=PIXAURA_ADMISSION_FALLBACK_MODEL;else accelerated=std::make_unique<AdmissionTestExecution>(std::move(prepared),s);}
                else
#endif
                {try{accelerated=find_provider(primary.data(),primary.size())->prepare(value);}catch(const Error& e){
                    if(e.code==4)reason=PIXAURA_ADMISSION_FALLBACK_UNAVAILABLE;else if(e.code==5)reason=PIXAURA_ADMISSION_FALLBACK_MODEL;else if(e.code==11)reason=PIXAURA_ADMISSION_FALLBACK_RESOURCE;else throw;}}
                if(accelerated){const auto needed=execution_reservation(checked_sum(cpu_bytes,accelerated->transient_bytes()),value);const auto cap=s.effective_ceiling.load(),used=s.used.load();
                    if(used<=cap&&needed<=cap-used){prepared=std::move(accelerated);selected=primary;reason=PIXAURA_ADMISSION_PRIMARY;}
                    else{reason=PIXAURA_ADMISSION_FALLBACK_RESOURCE;
#ifdef PIXAURA_CERTIFICATION_TESTING
                        if(s.admission_test_backend)prepared=prepare_cpu(value);
#endif
                    }
                }
            }
        }
        require(prepared!=nullptr,PIXAURA_INFERENCE_INTERNAL);
        require(selected!=provider||value.binding.required_validation<=reference.validation,PIXAURA_INFERENCE_PROVIDER_UNAVAILABLE);
        auto reservation=std::make_shared<Lease>(s,execution_reservation(checked_sum(cpu_bytes,prepared->transient_bytes()),value));
        auto a=std::make_shared<Admission>();a->model=std::make_shared<Model>();a->model->lease=std::move(metadata);a->model->package=p;a->model->prepared=std::move(prepared);
        a->reservation=std::move(reservation);a->request=*request;a->input=std::make_unique<float[]>(static_cast<size_t>(bytes/4));std::memcpy(a->input.get(),input,static_cast<size_t>(bytes));
        r.admitted=1;r.reason=reason;r.reservation_bytes=checked_sum(admission_metadata,a->reservation->bytes);r.peak_bytes=checked_sum(s.used.load(),PIXAURA_POLICY_CONTROL_RESERVE_BYTES-control);
        std::memcpy(r.provider_id,selected.data(),selected.size());std::memcpy(r.manifest_sha256,value.info.manifest_sha256,32);a->result=r;
        Entry e{};e.kind=4;e.admission=a;const auto token=insert_locked(h,std::move(e));s.pending=token.serial;s.last_admission_generation=request->generation;*out=token;*result=r;
    }catch(const Error& e){r.admitted=0;r.status=static_cast<uint32_t>(e.code);r.reason=!s.policy.enabled?uint32_t(PIXAURA_ADMISSION_REJECT_UNCONFIGURED):rejection_reason(e.code);*result=r;throw;}
});}
extern "C" int32_t pixaura_inference_execute_admitted(pixaura_inference_context* c,const pixaura_inference_handle* token,pixaura_inference_handle* out,uint32_t size){return boundary([&]{
    const auto h=live(c);output_pointer(h,c,out,size,sizeof(*out));require(token);separate(out,size,token,sizeof(*token));std::shared_ptr<Admission> a;
    {std::lock_guard<std::mutex> lock(h.state->mutex);auto& s=*h.state;a=acquire_locked(h,token,4).admission;
        require(!a->model->retired&&!a->model->cancellation.requested,PIXAURA_INFERENCE_CANCELLED);
        require(!a->model->policy_revoked&&!s.policy_stopped&&s.policy.strategy.epoch==a->result.epoch,PIXAURA_INFERENCE_STALE_HANDLE);
        require(!a->started&&!s.executing&&s.pending==token->serial,PIXAURA_INFERENCE_BUSY);a->started=true;s.executing=true;
    }
    struct Finish {Header h;uint64_t serial;~Finish(){std::lock_guard<std::mutex> lock(h.state->mutex);h.state->entries.erase(serial);if(h.state->pending==serial)h.state->pending=0;h.state->executing=false;}} finish{h,token->serial};
    auto m=a->model;require(std::fegetround()==FE_TONEAREST&&std::numeric_limits<double>::is_iec559,PIXAURA_INFERENCE_INTERNAL);
    checkpoint(m,*h.state,evaluation::Checkpoint::admission);
    const auto count=static_cast<size_t>(a->request.input.bytes/4),pixels=count/3;const auto& norm=a->request.input.normalization;
    normalize_input(a->input.get(),count,norm,m,*h.state);
    auto result=std::make_shared<Output>();result->lease=a->reservation;result->count=pixels;result->values=std::make_unique<float[]>(pixels);result->policy_epoch=a->result.epoch;
    struct Checks {std::shared_ptr<Model> model;State* state;} checks{m,h.state};
    ExecutionControl execution{[](void* opaque,evaluation::Checkpoint phase){auto& check=*static_cast<Checks*>(opaque);checkpoint(check.model,*check.state,phase);},&checks};
#ifdef PIXAURA_CERTIFICATION_TESTING
    execution.backend_fault=h.state->backend_fault;
#endif
    m->prepared->run(a->input.get(),result->values.get(),execution);
#ifdef PIXAURA_CERTIFICATION_TESTING
    if(h.state->output_fault)result->values[0]=std::numeric_limits<float>::quiet_NaN();
#endif
    require(valid_probabilities(result->values.get(),pixels),PIXAURA_INFERENCE_OUTPUT_INVALID);
    checkpoint(m,*h.state,evaluation::Checkpoint::publication);
    result->info.version=1;result->info.struct_size=sizeof(result->info);result->info.generation=a->request.generation;
    result->info.tensor=tensor(m->prepared->output,{{0,0,0,0,0,0}});std::memcpy(result->info.manifest_sha256,a->result.manifest_sha256,32);
    std::lock_guard<std::mutex> lock(h.state->mutex);require(!m->retired&&!m->cancellation.requested,PIXAURA_INFERENCE_CANCELLED);
    require(!m->policy_revoked&&h.state->policy.strategy.epoch==a->result.epoch,PIXAURA_INFERENCE_STALE_HANDLE);
    h.state->entries.erase(token->serial);Entry e{};e.kind=3;e.output=result;*out=insert_locked(h,std::move(e));
});}
extern "C" int32_t pixaura_inference_admission_cancel(pixaura_inference_context* c,const pixaura_inference_handle* token){return boundary([&]{const auto h=live(c);std::lock_guard<std::mutex> lock(h.state->mutex);acquire_locked(h,token,4).admission->model->cancellation.requested=true;});}
extern "C" int32_t pixaura_inference_admission_current(pixaura_inference_context* c,const pixaura_inference_handle* token){return boundary([&]{const auto h=live(c);std::lock_guard<std::mutex> lock(h.state->mutex);const auto output=acquire_locked(h,token,3).output;
    require(output->policy_epoch&&h.state->policy.enabled&&!h.state->policy_stopped&&output->policy_epoch==h.state->policy.strategy.epoch&&
        output->info.generation==h.state->last_admission_generation,PIXAURA_INFERENCE_STALE_HANDLE);
});}
