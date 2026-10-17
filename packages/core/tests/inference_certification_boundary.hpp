#ifndef PIXAURA_INFERENCE_CERTIFICATION_BOUNDARY_HPP
#define PIXAURA_INFERENCE_CERTIFICATION_BOUNDARY_HPP
#include "inference_certification_fixtures.hpp"
// TEST ONLY: all execution starts with the Step 1 canonical signed verifier.
namespace pixaura::inference {
namespace {
inline bool certification_all_equal(const std::vector<float>& values,float expected){
    return std::all_of(values.begin(),values.end(),[&](float value){return value==expected;});
}
inline int provider_security_certification(std::string_view selected){
    std::printf("PXSEC begin provider=%.*s\n",static_cast<int>(selected.size()),selected.data());
    int failures=0;uint64_t checks=0,vectors=0,values=0,threshold_band=0;
    auto check=[&](bool ok){++checks;if(!ok){++failures;if(failures<20){std::printf("PXSEC failed check=%llu\n",static_cast<unsigned long long>(checks));}}};
    auto expect=[&](int actual,int expected){if(actual!=expected){std::printf("PXSEC status actual=%d expected=%d\n",actual,expected);}check(actual==expected);};
    pixaura_inference_descriptor descriptor{};expect(pixaura_inference_descriptor_query(1,selected.data(),selected.size(),&descriptor,sizeof(descriptor)),0);
    check(descriptor.provider_version==1&&descriptor.availability==2&&descriptor.validation==(selected==provider?1u:0u));
    for(uint32_t version:{0u,2u,UINT32_MAX}){auto sentinel=descriptor;expect(pixaura_inference_descriptor_query(version,selected.data(),selected.size(),&sentinel,sizeof(sentinel)),2);check(!std::memcmp(&descriptor,&sentinel,sizeof(descriptor)));}
    for(uint32_t size:{0u,1u,UINT32_MAX}){auto sentinel=descriptor;expect(pixaura_inference_descriptor_query(1,selected.data(),selected.size(),&sentinel,size),2);check(!std::memcmp(&descriptor,&sentinel,sizeof(descriptor)));}
    static std::atomic<uint64_t> sequence{1};
    auto init=[&](pixaura_inference_context& c){c={};std::array<uint8_t,32> id{};id.fill('7');auto n=sequence.fetch_add(1);
        const char hex[]="0123456789abcdef";for(size_t i=0;i<16;++i){id[31-i]=static_cast<uint8_t>(hex[n&15]);n>>=4;}
        expect(pixaura_inference_context_init(1,&c,sizeof(c),id.data(),32,67108864),0);};
    auto package=[&](pixaura_inference_context& c,const fixtures::Case& f){
        pixaura::model::Key key{7,2,1,1,pixaura::model::fixtures::key};pixaura_model_package* raw=nullptr;pixaura_model_diagnostic diagnostic{};
        expect(pixaura::model::verify_owned(1,{{"manifest.pxm",f.manifest},{"signature.pxs",f.signature},{"reference.onnx",f.payload}},
            {1,1,2,&key,1},67108864,&raw,&diagnostic),0);
        const auto h=live(&c);Entry entry{};entry.kind=1;entry.package=adopt(h,std::unique_ptr<pixaura_model_package>(raw));
        std::lock_guard<std::mutex> lock(h.state->mutex);return insert_locked(h,std::move(entry));};
    auto prepare=[&](pixaura_inference_context& c,const pixaura_inference_handle& p,std::string_view id){pixaura_inference_handle m{};
        expect(pixaura_inference_prepare(&c,id.data(),id.size(),&p,&m,sizeof(m)),0);return m;};
    auto request=[&](pixaura_inference_context& c,const pixaura_inference_handle& m,uint64_t generation){pixaura_inference_model_metadata metadata{};
        expect(pixaura_inference_model_query(&c,&m,&metadata,sizeof(metadata)),0);pixaura_inference_request r{};
        r.version=1;r.struct_size=sizeof(r);r.task=1;r.generation=generation;r.input=metadata.input;return r;};
    auto execute=[&](pixaura_inference_context& c,const pixaura_inference_handle& m,const pixaura_inference_request& r,const std::vector<float>& input,pixaura_inference_handle& out){
        return pixaura_inference_execute(&c,&m,&r,sizeof(r),reinterpret_cast<const uint8_t*>(input.data()),input.size()*4,&out,sizeof(out));};
    auto copy=[&](pixaura_inference_context& c,const pixaura_inference_handle& out,size_t count){std::vector<float> result(count);
        expect(pixaura_inference_output_copy(&c,&out,reinterpret_cast<uint8_t*>(result.data()),count*4),0);return result;};
    for(const auto& malformed:fixtures::certification_cases()){std::printf("PXSEC malformed=%s\n",malformed.name);pixaura_inference_context c{};init(c);const auto p=package(c,malformed);pixaura_inference_handle m{};
        expect(pixaura_inference_prepare(&c,selected.data(),selected.size(),&p,&m,sizeof(m)),malformed.status);check(!m.serial);expect(pixaura_inference_context_destroy(&c),0);}
    auto all=fixtures::android_cases();for(const auto& f:fixtures::cases()){if(f.status==0){all.push_back(f);}}
    for(const auto& f:all){pixaura_inference_context c{};init(c);const auto p=package(c,f),m=prepare(c,p,selected),cpu=prepare(c,p,provider);
        auto r=request(c,m,1),cr=request(c,cpu,1);const auto pixels=static_cast<size_t>(r.input.bytes/12);std::vector<float> input(pixels*3);
        std::array<unsigned,3> weights{{1,2,1}};
        if(std::string_view(f.name).substr(0,8)=="weights-"){for(size_t ch=0;ch<3;++ch){weights[ch]=static_cast<unsigned>(f.name[8+ch]-'0');}}
        else if(std::strcmp(f.name,"valid-B")==0){weights={{4,0,0}};}
        // Zero, one, gradient, checker, IEEE rounding/threshold/subnormal vectors.
        for(unsigned pattern=0;pattern<5;++pattern){
            const std::array<uint32_t,16> bits{{0,1,0x007fffff,0x00800000,0x00800001,0x3efffffe,0x3effffff,0x3f000000,
                0x3f000001,0x3f000002,0x3eaaaaab,0x3dcccccd,0x3f7ffffe,0x3f7fffff,0x3f800000,0x80000000}};
            for(size_t ch=0;ch<3;++ch){for(size_t i=0;i<pixels;++i){const auto at=ch*pixels+i;
                if(pattern==4){const auto word=bits[(i+ch*5)%bits.size()];std::memcpy(&input[at],&word,4);}
                else{const auto units=pattern==0?0u:pattern==1?4u:pattern==2?static_cast<unsigned>((i+ch)%5):((i+ch)&1)?4u:0u;input[at]=static_cast<float>(units)/4.0f;}
            }}
            r.generation=cr.generation=pattern+1;pixaura_inference_handle out{},reference{};
            expect(execute(c,m,r,input,out),0);expect(execute(c,cpu,cr,input,reference),0);
            const auto actual=copy(c,out,pixels),expected=copy(c,reference,pixels);
            pixaura_inference_output a{},b{};expect(pixaura_inference_output_info(&c,&out,&a,sizeof(a)),0);expect(pixaura_inference_output_info(&c,&reference,&b,sizeof(b)),0);
            check(!std::memcmp(&a.tensor,&b.tensor,sizeof(a.tensor))&&a.generation==r.generation&&!std::memcmp(a.manifest_sha256,b.manifest_sha256,32));
            for(size_t i=0;i<pixels;++i){const auto ref=binary32_value(expected[i]);const auto error=std::abs(binary32_value(actual[i])-ref),bound=4e-38+3e-7*std::abs(ref);
                check(valid_probabilities(&actual[i],1));check(selected==provider?actual[i]==expected[i]:error<=bound);
                if(pattern<4){
                    // Independent dyadic integer oracle. Input quarters, weights
                    // quarters, and the sole half-scale fixture are exact here.
                    unsigned numerator=0;for(size_t ch=0;ch<3;++ch){auto units=static_cast<unsigned>(input[ch*pixels+i]*4);
                        numerator+=units*weights[ch]*(r.input.normalization[ch*2]==500000?1u:2u);}
                    const float oracle=static_cast<float>(numerator)/32.0f;check(expected[i]==oracle);
                }
                if(std::abs(ref-0.5)>bound){check((actual[i]>=0.5f)==(expected[i]>=0.5f));}else{++threshold_band;}
                ++values;
            }
            expect(pixaura_inference_release(&c,&out),0);expect(pixaura_inference_release(&c,&reference),0);++vectors;
        }
        expect(pixaura_inference_context_destroy(&c),0);
    }
    // The already accepted tolerance permits this exact threshold disagreement.
    const uint32_t below_bits=0x3effffff;float below=0;std::memcpy(&below,&below_bits,4);
    check(vulkan_probability_matches(below,0.5)&&metal_probability_matches(below,0.5));check(!(below>=0.5f)&&0.5f>=0.5f);
    pixaura_inference_context c{};init(c);const auto f=fixtures::cases().back();const auto p=package(c,f),m=prepare(c,p,selected);
    auto r=request(c,m,1);uint64_t generation=1;std::vector<float> input(12,0.5f);pixaura_inference_handle out{};
    auto reject=[&](pixaura_inference_request bad,int status){bad.generation=generation++;out={};expect(execute(c,m,bad,input,out),status);check(!out.serial);};
    // Deterministic structure mutations: do not send arbitrary pointers to C.
    for(uint32_t word:{0u,2u,UINT32_MAX}){auto bad=r;bad.version=word;reject(bad,2);bad=r;bad.input.version=word;reject(bad,2);}
    for(uint32_t word:{0u,1u,UINT32_MAX}){auto bad=r;bad.struct_size=word;reject(bad,2);bad=r;bad.input.struct_size=word;reject(bad,2);}
    for(uint32_t word:{0u,3u,UINT32_MAX}){auto bad=r;bad.input.rank=word;reject(bad,8);}
    for(size_t dimension=0;dimension<4;++dimension){for(uint32_t word:{0u,1025u,UINT32_MAX}){auto bad=r;bad.input.dimensions[dimension]=word;reject(bad,8);}}
    for(uint64_t bytes:{UINT64_C(0),UINT64_C(47),UINT64_C(49),UINT64_C(4294967295),UINT64_MAX}){auto bad=r;bad.input.bytes=bytes;reject(bad,8);}
    for(float value:{-std::numeric_limits<float>::infinity(),std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN(),-0.1f,1.1f}){
        input.back()=value;reject(r,10);}input.back()=0.5f;
    auto bad=r;bad.input.reserved=1;reject(bad,2);bad=r;bad.input.normalization[1]=1;reject(bad,18);
    bad=r;bad.input.type=UINT32_MAX;reject(bad,9);bad=r;bad.input.layout=UINT32_MAX;reject(bad,9);
    r.generation=generation++;out={};expect(pixaura_inference_execute(&c,&m,&r,sizeof(r),nullptr,48,&out,sizeof(out)),1);check(!out.serial);
    r.generation=generation++;expect(pixaura_inference_execute(&c,&m,&r,sizeof(r),reinterpret_cast<const uint8_t*>(input.data()),48,nullptr,sizeof(out)),1);
    alignas(8) std::array<uint8_t,256> aliased{};r.generation=generation++;
    expect(pixaura_inference_execute(&c,&m,&r,sizeof(r),aliased.data(),48,reinterpret_cast<pixaura_inference_handle*>(aliased.data()),sizeof(out)),1);
    check(checked_sum(UINT64_MAX-1,1)==UINT64_MAX);
    expect(boundary([&]{(void)checked_sum(UINT64_MAX,1);}),11);
    expect(boundary([&]{(void)tensor({{UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX}},{{0,0,0,0,0,0}});}),8);
    // Failures preserve ownership/accounting and subsequent requests recover.
    auto* state=live(&c).state;const auto baseline=state->used.load();
    for(unsigned fault=1;fault<=5;++fault){state->certification_output_fault=fault;r.generation=generation++;out={};
        expect(execute(c,m,r,input,out),13);check(!out.serial&&state->used.load()==baseline);
        state->certification_output_fault=0;r.generation=generation++;expect(execute(c,m,r,input,out),0);expect(pixaura_inference_release(&c,&out),0);check(state->used.load()==baseline);}
    for(unsigned mutation=0;mutation<9;++mutation){auto token=m;switch(mutation){case 0:token={};break;case 1:token.serial=UINT64_MAX;break;case 2:token.serial=0;break;
        case 3:token.kind=1;break;case 4:token.context_id[0]^=1;break;case 5:token.reserved=1;break;case 6:token.version=2;break;case 7:token.struct_size--;break;default:token.kind=UINT32_MAX;}
        r.generation=generation++;out={};expect(execute(c,token,r,input,out),14);check(!out.serial);}
    // Serial exhaustion never wraps or aliases a released token.
    const auto next=state->next;state->next=UINT64_MAX;r.generation=generation++;out={};expect(execute(c,m,r,input,out),11);check(!out.serial&&state->used.load()==baseline);state->next=next;
    r.generation=generation++;expect(execute(c,m,r,input,out),0);const auto old=out;expect(pixaura_inference_release(&c,&out),0);
    r.generation=generation++;expect(execute(c,m,r,input,out),0);check(old.serial!=out.serial);expect(pixaura_inference_release(&c,&old),14);
    expect(pixaura_inference_context_destroy(&c),0);pixaura_inference_output metadata{};
    expect(pixaura_inference_output_info(&c,&out,&metadata,sizeof(metadata)),14);
    // Cancellation concurrent with retirement at the final publication checkpoint.
    init(c);const auto cp=package(c,f),cm=prepare(c,cp,selected);r=request(c,cm,1);out={};
    struct Fence {pixaura_inference_context* c;const pixaura_inference_handle* m;int cancel=-1,release=-1;bool seen=false;};Fence fence{&c,&cm};
    state=live(&c).state;state->observer_state=&fence;state->observer=[](evaluation::Checkpoint phase,void* opaque){auto& v=*static_cast<Fence*>(opaque);
        if(phase==evaluation::Checkpoint::publication&&!v.seen){v.seen=true;std::thread worker([&]{v.cancel=pixaura_inference_cancel(v.c,v.m,1);});
            v.release=pixaura_inference_release(v.c,v.m);worker.join();}};
    expect(execute(c,cm,r,input,out),12);check(fence.seen&&!out.serial&&fence.release==0&&(fence.cancel==0||fence.cancel==14));
    state->observer=nullptr;expect(pixaura_inference_context_destroy(&c),0);
    pixaura_inference_descriptor after{};expect(pixaura_inference_descriptor_query(1,selected.data(),selected.size(),&after,sizeof(after)),0);check(after.validation==descriptor.validation);
    char record[384]{};std::snprintf(record,sizeof(record),"PXSEC v1 provider=%.*s vectors=%llu values=%llu threshold_band=%llu checks=%llu failures=%d release_trust=%u software_evidence_only",
        static_cast<int>(selected.size()),selected.data(),static_cast<unsigned long long>(vectors),static_cast<unsigned long long>(values),
        static_cast<unsigned long long>(threshold_band),static_cast<unsigned long long>(checks),failures,selected==provider?1u:0u);
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_INFO,"PixAuraInference","%s",record);
#else
    std::printf("%s\n",record);
#endif
    return failures;
}
}
}
#endif
