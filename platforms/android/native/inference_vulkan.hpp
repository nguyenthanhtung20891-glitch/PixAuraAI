// Included inside the private shared provider registry implementation.
// No JNI, caller paths, test trust or model-supplied executable code here.
constexpr std::string_view vulkan_provider="android.vulkan.v1";
inline int32_t inference_vulkan_status(int32_t status){
    return status==5?PIXAURA_INFERENCE_PROVIDER_UNAVAILABLE:
        status==8?PIXAURA_INFERENCE_RESOURCE:PIXAURA_INFERENCE_INTERNAL;
}
inline bool vulkan_available()noexcept{
    try{pixaura::android_gpu::Vulkan gpu;gpu.probe(false);return true;}catch(...){return false;}
}
struct AndroidVulkan final:PreparedExecution {
    Plan plan;
    explicit AndroidVulkan(Plan p):plan(p){input=p.input;output=p.output;}
    uint64_t transient_bytes()const override{return 1638400;}
    void run(const float* in,float* out,const ExecutionControl& execution_control)const override{
        const auto pixels=static_cast<size_t>(input[2])*input[3];
        const auto start=std::chrono::steady_clock::now();
        execution_control.check(execution_control.state,evaluation::Checkpoint::tile);
#ifdef PIXAURA_CERTIFICATION_TESTING
        if(execution_control.backend_fault==1)throw Error{PIXAURA_INFERENCE_PROVIDER_UNAVAILABLE};
        if(execution_control.backend_fault==2)throw Error{PIXAURA_INFERENCE_RESOURCE};
#endif
        pixaura::android_gpu::Vulkan gpu;
        try{gpu.initialize(segmentation_spirv,sizeof(segmentation_spirv),16,false);}
        catch(int32_t status){throw Error{inference_vulkan_status(status)};}
        auto source=std::make_unique<float[]>(PIXAURA_GPU_MAX_PIXELS*4);
        auto result=std::make_unique<float[]>(PIXAURA_GPU_MAX_PIXELS*4);
        for(size_t offset=0;offset<pixels;offset+=PIXAURA_GPU_MAX_PIXELS){
            execution_control.check(execution_control.state,evaluation::Checkpoint::tile);
            const auto count=static_cast<uint32_t>(std::min<size_t>(PIXAURA_GPU_MAX_PIXELS,pixels-offset));
            for(size_t j=0;j<count;++j){for(size_t channel=0;channel<3;++channel)source[j*4+channel]=in[channel*pixels+offset+j];source[j*4+3]=0;}
            struct Parameters{uint32_t count;float r,g,b;}params{count,plan.weights[0],plan.weights[1],plan.weights[2]};
            static_assert(sizeof(params)==16,"reference Conv push layout");
            uint32_t fault=0;
#ifdef PIXAURA_CERTIFICATION_TESTING
            fault=execution_control.backend_fault;
#endif
            const auto status=gpu.run_kernel(source.get(),result.get(),count,&params,sizeof(params),fault);
            if(status)throw Error{inference_vulkan_status(status)};
#ifdef PIXAURA_CERTIFICATION_TESTING
            if(execution_control.backend_fault==4)result[0]=std::numeric_limits<float>::quiet_NaN();
#endif
            execution_control.check(execution_control.state,evaluation::Checkpoint::tile);
            for(size_t j=0;j<count;++j){double reference=0;
                for(size_t channel=0;channel<3;++channel)reference=reference+binary32_value(source[j*4+channel])*binary32_value(plan.weights[channel]);
                reference=binary32_value(binary32_round(reference));
                require(vulkan_probability_matches(result[j*4],reference),PIXAURA_INFERENCE_OUTPUT_INVALID);
                out[offset+j]=result[j*4];
            }
        }
#ifdef PIXAURA_CERTIFICATION_TESTING
        const auto total=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-start).count();
        __android_log_print(ANDROID_LOG_INFO,"PixAuraInference","Vulkan reference: pixels=%llu physical_class=%u total_us=%lld upload_ns=%llu dispatch_wait_ns=%llu readback_ns=%llu transient_bound=1638400",
            static_cast<unsigned long long>(pixels),gpu.hardware?1u:0u,static_cast<long long>(total),
            static_cast<unsigned long long>(gpu.upload_ns),static_cast<unsigned long long>(gpu.dispatch_ns),static_cast<unsigned long long>(gpu.readback_ns));
#else
        (void)start;
#endif
    }
};
std::unique_ptr<const PreparedExecution> prepare_vulkan(const pixaura_model_package& value){
    // The approved portable CPU ONNX record is mandatory, never reinterpreted
    // as a new Vulkan manifest field or permission to run a broader graph.
    const auto cpu=prepare_cpu(value);
    const auto& plan=static_cast<const CpuReference&>(*cpu).plan;
    require(vulkan_available(),PIXAURA_INFERENCE_PROVIDER_UNAVAILABLE);
    return std::make_unique<AndroidVulkan>(plan);
}
