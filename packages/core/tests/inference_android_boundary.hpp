#ifndef PIXAURA_INFERENCE_ANDROID_BOUNDARY_HPP
#define PIXAURA_INFERENCE_ANDROID_BOUNDARY_HPP
#include "inference_android_fixtures.hpp"
// TEST ONLY, compiled into Android certification builds, never release trust.
namespace pixaura::inference {
inline int android_certification(){
    int failures=0;auto expect=[&](int actual,int expected){if(actual!=expected){++failures;__android_log_print(ANDROID_LOG_ERROR,"PixAuraInference","status actual=%d expected=%d",actual,expected);}};
    const auto cases=fixtures::cases();static std::atomic<uint64_t> sequence{1};
    auto init=[&](pixaura_inference_context& c,uint64_t ceiling=67108864){std::array<uint8_t,32> id{};id.fill('a');auto n=sequence.fetch_add(1);
        const char hex[]="0123456789abcdef";for(size_t i=0;i<16;++i){id[31-i]=static_cast<uint8_t>(hex[n&15]);n>>=4;}
        expect(pixaura_inference_context_init(1,&c,sizeof(c),id.data(),32,ceiling),0);};
    auto admit=[&](pixaura_inference_context& c,const fixtures::Case& f){
        pixaura::model::Key key{7,2,1,1,pixaura::model::fixtures::key};pixaura_model_package* raw=nullptr;pixaura_model_diagnostic d{};
        expect(pixaura::model::verify_owned(1,{{"manifest.pxm",f.manifest},{"signature.pxs",f.signature},{"reference.onnx",f.payload}},{1,1,2,&key,1},67108864,&raw,&d),0);
        const auto h=live(&c);Entry e{};e.kind=1;e.package=adopt(h,std::unique_ptr<pixaura_model_package>(raw));std::lock_guard<std::mutex> lock(h.state->mutex);return insert_locked(h,std::move(e));};
    auto prepare=[&](pixaura_inference_context& c,const pixaura_inference_handle& p,pixaura_inference_handle& m,std::string_view id=vulkan_provider){
        return pixaura_inference_prepare(&c,id.data(),id.size(),&p,&m,sizeof(m));};
    auto request=[&](pixaura_inference_context& c,const pixaura_inference_handle& m,uint64_t generation){pixaura_inference_model_metadata meta{};
        expect(pixaura_inference_model_query(&c,&m,&meta,sizeof(meta)),0);pixaura_inference_request r{};r.version=1;r.struct_size=sizeof(r);r.task=1;r.generation=generation;r.input=meta.input;return r;};
    auto execute=[&](pixaura_inference_context& c,const pixaura_inference_handle& m,const pixaura_inference_request& r,const std::vector<float>& in,pixaura_inference_handle& out){
        return pixaura_inference_execute(&c,&m,&r,sizeof(r),reinterpret_cast<const uint8_t*>(in.data()),in.size()*4,&out,sizeof(out));};
    auto copy=[&](pixaura_inference_context& c,const pixaura_inference_handle& out,size_t count){std::vector<float> values(count);
        expect(pixaura_inference_output_copy(&c,&out,reinterpret_cast<uint8_t*>(values.data()),count*4),0);return values;};
    pixaura_inference_descriptor descriptor{};expect(pixaura_inference_descriptor_query(1,vulkan_provider.data(),vulkan_provider.size(),&descriptor,sizeof(descriptor)),0);
    if(std::strcmp(descriptor.provider_id,"android.vulkan.v1")||descriptor.backend!=2||descriptor.validation!=0||descriptor.deterministic||descriptor.max_dimension!=1024||
       std::strcmp(descriptor.profile_id,"pixaura.onnx.reference-segmentation.v1"))++failures;
    // Integration must actually dispatch, not silently pass because Vulkan is absent.
    if(descriptor.availability!=2)return failures+1;
    for(const auto& f:cases){pixaura_inference_context c{};init(c);const auto p=admit(c,f);pixaura_inference_handle m{};
        expect(prepare(c,p,m),f.status);if(f.status&&m.serial)++failures;expect(pixaura_inference_context_destroy(&c),0);}
    // All accepted models/shapes/normalization, two real parsed weight sets.
    auto execution_cases=fixtures::android_cases();for(const auto& f:cases)if(!f.status)execution_cases.push_back(f);
    for(const auto& f:execution_cases){pixaura_inference_context c{};init(c);const auto p=admit(c,f);pixaura_inference_handle gpu{},cpu{};
        expect(prepare(c,p,gpu),0);expect(prepare(c,p,cpu,provider),0);auto r=request(c,gpu,1);auto cr=request(c,cpu,1);
        const auto count=static_cast<size_t>(r.input.bytes/4),pixels=count/3;std::vector<float> in(count);
        const std::array<uint32_t,12> bits{{0,1,0x007fffff,0x00800000,0x00800001,0x3f000001,0x3eaaaaab,0x3dcccccd,0x3f7fffff,0x3f800000,0x3e800001,0x80000000}};
        for(size_t i=0;i<count;++i)std::memcpy(&in[i],&bits[i%bits.size()],4);
        pixaura_inference_handle go{},co{};expect(execute(c,gpu,r,in,go),0);const auto start=std::chrono::steady_clock::now();expect(execute(c,cpu,cr,in,co),0);
        const auto cpu_us=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-start).count();
        const auto gv=copy(c,go,pixels),cv=copy(c,co,pixels);for(size_t i=0;i<pixels;++i)if(!vulkan_probability_matches(gv[i],binary32_value(cv[i])))++failures;
        pixaura_inference_output gi{},ci{};expect(pixaura_inference_output_info(&c,&go,&gi,sizeof(gi)),0);expect(pixaura_inference_output_info(&c,&co,&ci,sizeof(ci)),0);
        if(std::memcmp(&gi.tensor,&ci.tensor,sizeof(gi.tensor))||std::memcmp(gi.manifest_sha256,ci.manifest_sha256,32))++failures;
        __android_log_print(ANDROID_LOG_INFO,"PixAuraInference","model=%s pixels=%llu cpu_us=%lld parity_failures=%d",f.name,static_cast<unsigned long long>(pixels),static_cast<long long>(cpu_us),failures);
        expect(pixaura_inference_context_destroy(&c),0);
    }
    pixaura_inference_context c{};init(c);const auto p=admit(c,cases.back());pixaura_inference_handle m{};expect(prepare(c,p,m),0);uint64_t generation=1;
    pixaura_inference_model_metadata admitted{};expect(pixaura_inference_model_query(&c,&m,&admitted,sizeof(admitted)),0);
    if(admitted.execution_bytes!=1638400+64)++failures;
    pixaura_inference_handle unverified{};expect(prepare(c,m,unverified),3);if(unverified.serial)++failures;
    for(const auto& golden:fixtures::goldens()){auto r=request(c,m,generation++);std::vector<float> in;for(auto q:golden.input_units)in.push_back(static_cast<float>(q)/4.0f);
        pixaura_inference_handle out{};expect(execute(c,m,r,in,out),0);const auto values=copy(c,out,4);
        for(size_t i=0;i<4;++i){float expected=0;std::memcpy(&expected,&golden.output_bits[i],4);if(!vulkan_probability_matches(values[i],binary32_value(expected)))++failures;}
        expect(pixaura_inference_release(&c,&out),0);expect(pixaura_inference_release(&c,&out),14);
    }
    auto r=request(c,m,generation++);std::vector<float> in(12,0.5f);pixaura_inference_handle out{};
    auto bad=[&](pixaura_inference_request b,int status){b.generation=generation++;out={};expect(execute(c,m,b,in,out),status);if(out.serial)++failures;};
    auto b=r;b.input.rank=3;bad(b,8);b=r;b.input.dimensions[2]=UINT32_MAX;bad(b,8);b=r;b.input.type=2;bad(b,9);
    b=r;b.input.bytes--;bad(b,8);b=r;b.input.layout=2;bad(b,9);b=r;b.task=2;bad(b,7);
    b=r;b.input.normalization[0]=2000000;bad(b,18);b=r;b.pre_cancel=1;bad(b,12);
    for(float value:{-0.1f,1.1f,std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()}){in[0]=value;bad(r,10);}in[0]=0.5f;
    for(uint32_t fault=1;fault<=4;++fault){live(&c).state->backend_fault=fault;r.generation=generation++;out={};
        expect(execute(c,m,r,in,out),fault==1?4:fault==2?11:fault==3?15:13);if(out.serial)++failures;
        live(&c).state->backend_fault=0;r.generation=generation++;expect(execute(c,m,r,in,out),0);expect(pixaura_inference_release(&c,&out),0);}
    expect(pixaura_inference_release(&c,&m),0);r.generation=generation++;expect(execute(c,m,r,in,out),14);expect(pixaura_inference_context_destroy(&c),0);
    // Bounded context lease rejects before backend allocation, no ceiling increase.
    pixaura_inference_context small{};init(small,400000);const auto sp=admit(small,cases.back());pixaura_inference_handle sm{};expect(prepare(small,sp,sm),0);
    auto sr=request(small,sm,1);out={};expect(execute(small,sm,sr,in,out),11);if(out.serial)++failures;expect(pixaura_inference_context_destroy(&small),0);
    // Two Vulkan contexts, different models, interleaved/concurrent lifetimes.
    pixaura_inference_context a{},bb{};init(a);init(bb);const auto pa=admit(a,cases[0]),pb=admit(bb,cases[1]);pixaura_inference_handle ma{},mb{},oa{},ob{};
    expect(prepare(a,pa,ma),0);expect(prepare(bb,pb,mb),0);auto ra=request(a,ma,1),rb=request(bb,mb,1);in.assign(192,0);std::fill_n(in.begin(),64,1.0f);
    expect(execute(bb,ma,rb,in,oa),14);auto cancelled=ra;cancelled.pre_cancel=1;expect(execute(a,ma,cancelled,in,oa),12);ra.generation=2;
    int thread_status=-1;std::thread worker([&]{thread_status=execute(a,ma,ra,in,oa);});expect(execute(bb,mb,rb,in,ob),0);worker.join();expect(thread_status,0);
    if(copy(a,oa,64)[0]!=0.25f||copy(bb,ob,64)[0]!=1.0f)++failures;
    expect(pixaura_inference_context_destroy(&a),0);expect(pixaura_inference_context_destroy(&bb),0);
    // Cancel/release after a real completed tile: no successful publication.
    pixaura_inference_context large{};init(large);const auto lp=admit(large,cases[3]);pixaura_inference_handle lm{};expect(prepare(large,lp,lm),0);
    auto lr=request(large,lm,1);in.assign(3*1024*1024,1.0f);
    struct Hook{pixaura_inference_context* c;pixaura_inference_handle* m;uint64_t generation=1;unsigned tiles=0;bool release=false;int status=-1;};
    Hook hook{&large,&lm};auto* state=live(&large).state;state->observer_state=&hook;
    state->observer=[](evaluation::Checkpoint phase,void* opaque){auto& h=*static_cast<Hook*>(opaque);if(phase==evaluation::Checkpoint::tile&&++h.tiles==3)h.status=h.release?pixaura_inference_release(h.c,h.m):pixaura_inference_cancel(h.c,h.m,h.generation);};
    out={};expect(execute(large,lm,lr,in,out),12);expect(hook.status,0);if(out.serial||hook.tiles!=3)++failures;
    state->observer=nullptr;lr.generation=2;expect(execute(large,lm,lr,in,out),0);expect(pixaura_inference_release(&large,&out),0);
    hook.tiles=0;hook.release=true;hook.generation=3;hook.status=-1;state->observer=[](evaluation::Checkpoint phase,void* opaque){auto& h=*static_cast<Hook*>(opaque);if(phase==evaluation::Checkpoint::tile&&++h.tiles==3)h.status=pixaura_inference_release(h.c,h.m);};
    lr.generation=3;out={};expect(execute(large,lm,lr,in,out),12);expect(hook.status,0);if(out.serial)++failures;state->observer=nullptr;
    expect(pixaura_inference_context_destroy(&large),0);
    __android_log_print(ANDROID_LOG_INFO,"PixAuraInference","Android shared corpus: ONNX=32 goldens=4 model_cases=21 weight_triples=15 faults=4 failures=%d; integration only, no physical certification",failures);
    return failures;
}
}
#endif
