// Private registry backend; model admission and all semantics remain shared C++.
constexpr std::string_view metal_provider="apple.metal.v1";
struct AppleMetal final:PreparedExecution {
    Plan plan;
    explicit AppleMetal(Plan p):plan(p){input=p.input;output=p.output;}
    uint64_t transient_bytes()const override{return 1114112;}
    void run(const float* in,float* out,const ExecutionControl& execution_control)const override{
        const auto pixels=static_cast<size_t>(input[2])*input[3];const auto start=std::chrono::steady_clock::now();
        execution_control.check(execution_control.state,evaluation::Checkpoint::tile);
        uint32_t fault=0;
#ifdef PIXAURA_CERTIFICATION_TESTING
        fault=execution_control.backend_fault;if(fault==1)throw Error{4};if(fault==2)throw Error{11};
#endif
        auto worker=pixaura::apple::make_metal_worker(true);
        auto source=std::make_unique<float[]>(16384*4),result=std::make_unique<float[]>(16384*4);
        pixaura::apple::MetalTiming timing;
        for(size_t offset=0;offset<pixels;offset+=16384){
            execution_control.check(execution_control.state,evaluation::Checkpoint::tile);
            const auto count=static_cast<uint32_t>(std::min<size_t>(16384,pixels-offset));
            for(size_t j=0;j<count;++j){for(size_t channel=0;channel<3;++channel)source[j*4+channel]=in[channel*pixels+offset+j];source[j*4+3]=0;}
            uint32_t parameters[4]{count,0,0,0};for(size_t channel=0;channel<3;++channel)std::memcpy(&parameters[channel+1],&plan.weights[channel],4);
            const auto status=worker->dispatch(source.get(),result.get(),count,parameters,fault,timing);require(!status,status);
#ifdef PIXAURA_CERTIFICATION_TESTING
            if(fault==4)result[0]=std::numeric_limits<float>::quiet_NaN();
#endif
            execution_control.check(execution_control.state,evaluation::Checkpoint::tile);
            for(size_t j=0;j<count;++j){double reference=0;
                for(size_t channel=0;channel<3;++channel)reference=reference+binary32_value(source[j*4+channel])*binary32_value(plan.weights[channel]);
                reference=binary32_value(binary32_round(reference));require(metal_probability_matches(result[j*4],reference),13);out[offset+j]=result[j*4];
            }
        }
#ifdef PIXAURA_CERTIFICATION_TESTING
        const auto total=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-start).count();
        std::printf("Metal reference: pixels=%llu total_us=%lld upload_ns=%llu dispatch_wait_ns=%llu readback_ns=%llu transient_bound=1114112; hosted/simulator integration only\n",
            static_cast<unsigned long long>(pixels),static_cast<long long>(total),static_cast<unsigned long long>(timing.upload_ns),
            static_cast<unsigned long long>(timing.dispatch_ns),static_cast<unsigned long long>(timing.readback_ns));
#else
        (void)start;
#endif
    }
};
std::unique_ptr<const PreparedExecution> prepare_metal(const pixaura_model_package& value){
    const auto cpu=prepare_cpu(value);const auto plan=static_cast<const CpuReference&>(*cpu).plan;
    require(pixaura::apple::metal_available(),PIXAURA_INFERENCE_PROVIDER_UNAVAILABLE);return std::make_unique<AppleMetal>(plan);
}
