#ifndef PIXAURA_SEGMENTATION_EVALUATION_BOUNDARY_HPP
#define PIXAURA_SEGMENTATION_EVALUATION_BOUNDARY_HPP
#include "segmentation_metrics.hpp"
#include "segmentation_fixtures.hpp"
#include "inference_fixtures.hpp"
#include "model_package_fixtures.hpp"
#include "inference_android_fixtures.hpp"
#include "../src/sha256.hpp"
#include <cstdio>
#include <string>
namespace pixaura::inference {
namespace {
std::string evaluation_hash(const uint8_t* data,size_t count){pixaura::storage::Sha256 sha;sha.update(data,count);const auto text=sha.finish();return std::string(text.data(),text.size());}
std::string evaluation_hex(const uint8_t* data,size_t count){const char digits[]="0123456789abcdef";std::string out;out.reserve(count*2);
    for(size_t i=0;i<count;++i){out+=digits[data[i]>>4];out+=digits[data[i]&15];}return out;}
using EvaluationClock=std::chrono::steady_clock;
uint64_t evaluation_ns(EvaluationClock::time_point start){return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(EvaluationClock::now()-start).count());}
std::string evaluation_ratio(pixaura::segmentation::Ratio r){return "["+std::to_string(r.numerator)+","+std::to_string(r.denominator)+"]";}
void evaluation_emit(const std::string& row){
#ifdef __ANDROID__
    // liblog's printf buffer can be smaller than logcat's entry limit. Keep each
    // call below 1 KiB, preserving the complete JSON through checked fragments.
    static std::atomic<uint64_t> sequence{1};const auto id=sequence.fetch_add(1);
    constexpr size_t width=700;const auto parts=(row.size()+width-1)/width;
    for(size_t part=0;part<parts;++part){const auto text=row.substr(part*width,width);
        __android_log_print(ANDROID_LOG_INFO,"PixAuraEvaluation","PXSEG_PART %llu %zu %zu %s",
            static_cast<unsigned long long>(id),part,parts,text.c_str());}
#else
    std::printf("PXSEG %s\n",row.c_str());
#endif
}
void evaluation_identity(const pixaura_inference_output& info,const pixaura_inference_model_metadata& model,uint64_t generation){
    segmentation::check(info.version==1&&info.struct_size==sizeof(info)&&info.generation==generation&&
        !std::memcmp(info.manifest_sha256,model.manifest_sha256,32)&&!std::memcmp(&info.tensor,&model.output,sizeof(info.tensor)),segmentation::Status::identity);
}
}
// All inputs enter existing verification/preparation/execute/copy APIs. Test trust
// is the retained isolated RFC fixture store, never a new runtime trust path.
inline int segmentation_evaluation(std::string_view selected){
    using namespace pixaura::segmentation;
    int failures=metric_goldens();static std::atomic<uint64_t> sequence{1};
    auto expect=[&](int actual,int wanted=0){if(actual!=wanted)throw segmentation::Error{Status::identity};};
    const auto cases=fixtures::cases(),weights=fixtures::android_cases();
    for(uint32_t size:{8u,256u}){
        pixaura_inference_context context{};std::array<uint8_t,32> id{};id.fill('5');auto serial=sequence.fetch_add(1);
        for(size_t i=0;i<16;++i){id[31-i]=static_cast<uint8_t>("0123456789abcdef"[serial&15]);serial>>=4;}
        try{
            expect(pixaura_inference_context_init(1,&context,sizeof(context),id.data(),32,67108864));
            struct Cleanup {pixaura_inference_context* c;~Cleanup(){pixaura_inference_context_destroy(c);}} cleanup{&context};
            const auto& f=size==8?cases.front():weights[7];
            pixaura::model::Key key{7,2,1,1,pixaura::model::fixtures::key};pixaura_model_package* raw=nullptr;pixaura_model_diagnostic diagnostic{};
            expect(pixaura::model::verify_owned(1,{{"manifest.pxm",f.manifest},{"signature.pxs",f.signature},{"reference.onnx",f.payload}},
                {1,1,2,&key,1},67108864,&raw,&diagnostic));
            const auto header=live(&context);Entry entry{};entry.kind=1;entry.package=adopt(header,std::unique_ptr<pixaura_model_package>(raw));
            pixaura_inference_handle package{};{std::lock_guard<std::mutex> lock(header.state->mutex);package=insert_locked(header,std::move(entry));}
            pixaura_inference_resources before{},prepared{};expect(pixaura_inference_resources_query(&context,&before,sizeof(before)));
            const auto prepare_start=EvaluationClock::now();pixaura_inference_handle model{};
            expect(pixaura_inference_prepare(&context,selected.data(),selected.size(),&package,&model,sizeof(model)));const auto prepare_ns=evaluation_ns(prepare_start);
            expect(pixaura_inference_resources_query(&context,&prepared,sizeof(prepared)));
            pixaura_inference_model_metadata metadata{};expect(pixaura_inference_model_query(&context,&model,&metadata,sizeof(metadata)));
            check(metadata.input.dimensions[2]==size&&metadata.input.dimensions[3]==size,Status::shape);
            const auto package_digest=evaluation_hex(metadata.manifest_sha256,32),payload_digest=evaluation_hash(f.payload.data(),f.payload.size());
            uint64_t generation=0;
            for(uint32_t scene_id=0;scene_id<scenes.size();++scene_id){
                const auto total_start=EvaluationClock::now(),pre_start=total_start;auto data=scene(scene_id,size);const auto pre_ns=evaluation_ns(pre_start);
                const size_t pixels=size_t(size)*size;const auto& golden=masks[(size==8?0:12)+scene_id];
                const auto expected_hash=evaluation_hash(data.expected.data(),data.expected.size());
                check(expected_hash==golden.digest,Status::identity);
                if(scene_id==0){auto corrupt=data.expected;corrupt[0]^=1;check(evaluation_hash(corrupt.data(),corrupt.size())!=golden.digest,Status::identity);}
                auto run=[&](std::vector<float>& values){
                    pixaura_inference_request request{};request.version=1;request.struct_size=sizeof(request);request.task=1;request.generation=++generation;request.input=metadata.input;
                    pixaura_inference_handle output{};const auto start=EvaluationClock::now();
                    expect(pixaura_inference_execute(&context,&model,&request,sizeof(request),reinterpret_cast<const uint8_t*>(data.rgb.data()),data.rgb.size()*4,&output,sizeof(output)));
                    const auto elapsed=evaluation_ns(start);pixaura_inference_output info{};
                    expect(pixaura_inference_output_info(&context,&output,&info,sizeof(info)));evaluation_identity(info,metadata,generation);
                    if(scene_id==0&&generation==1){auto stale=info;stale.generation--;bool rejected=false;try{evaluation_identity(stale,metadata,generation);}catch(const segmentation::Error&){rejected=true;}check(rejected,Status::identity);stale=info;stale.manifest_sha256[0]^=1;rejected=false;try{evaluation_identity(stale,metadata,generation);}catch(const segmentation::Error&){rejected=true;}check(rejected,Status::identity);}
                    expect(pixaura_inference_output_copy(&context,&output,reinterpret_cast<uint8_t*>(values.data()),values.size()*4));
                    expect(pixaura_inference_release(&context,&output));
                    if(scene_id==0&&generation==1){expect(pixaura_inference_output_info(&context,&output,&info,sizeof(info)),14);}
                    return elapsed;
                };
                std::vector<float> predicted(pixels),warm(pixels);const auto first_ns=run(predicted);std::array<uint64_t,3> warm_ns{};
                for(auto& time:warm_ns){time=run(warm);check(!std::memcmp(warm.data(),predicted.data(),pixels*4),Status::identity);}std::sort(warm_ns.begin(),warm_ns.end());
                const auto post_start=EvaluationClock::now();const auto measured=evaluate(1,size,size,data.expected.data(),pixels,predicted.data(),pixels);
                check(measured.counts.tp==golden.reference.tp&&measured.counts.fp==golden.reference.fp&&measured.counts.fn==golden.reference.fn&&measured.counts.tn==golden.reference.tn,Status::identity);
                const auto post_ns=evaluation_ns(post_start),total_ns=evaluation_ns(total_start);
                auto emit=[&](const char* predictor,const Metrics& m,bool executable){const auto& c=m.counts;
                    const auto text=std::string("{\"evaluation_version\":1,\"metric_version\":1,\"case_id\":\"")+scenes[scene_id]+"\",\"width\":"+std::to_string(size)+",\"height\":"+std::to_string(size)+
                        ",\"threshold\":[1,2],\"expected_sha256\":\""+expected_hash+"\",\"predictor\":\""+predictor+"\",\"candidate_state\":\""+(executable?"EXECUTION_FIXTURE":"BASELINE")+
                        "\",\"production_selection\":\"UNRESOLVED\",\"provider_id\":"+(executable?"\""+std::string(selected)+"\"":"null")+",\"manifest_sha256\":"+(executable?"\""+package_digest+"\"":"null")+
                        ",\"payload_sha256\":"+(executable?"\""+payload_digest+"\"":"null")+",\"counts\":["+std::to_string(c.tp)+","+std::to_string(c.fp)+","+std::to_string(c.fn)+","+std::to_string(c.tn)+
                        "],\"iou\":"+evaluation_ratio(m.iou)+",\"dice\":"+evaluation_ratio(m.dice)+",\"accuracy\":"+evaluation_ratio(m.accuracy)+",\"precision\":"+evaluation_ratio(m.precision)+",\"recall\":"+evaluation_ratio(m.recall);
                    std::string row=text;
                    if(executable)row+=",\"cold_prepare_ns\":"+std::to_string(prepare_ns)+",\"first_case_execute_ns\":"+std::to_string(first_ns)+",\"warm_count\":3,\"warm_median_ns\":"+std::to_string(warm_ns[1])+",\"warm_max_ns\":"+std::to_string(warm_ns[2])+
                        ",\"preprocess_ns\":"+std::to_string(pre_ns)+",\"postprocess_ns\":"+std::to_string(post_ns)+",\"case_total_ns\":"+std::to_string(total_ns)+",\"package_bytes\":"+std::to_string(f.manifest.size()+f.signature.size()+f.payload.size())+
                        ",\"model_bytes\":"+std::to_string(f.payload.size())+",\"prepared_reservation_bytes\":"+std::to_string(prepared.reserved_bytes-before.reserved_bytes)+",\"context_prepared_bytes\":"+std::to_string(prepared.reserved_bytes)+
                        ",\"input_bytes\":"+std::to_string(metadata.input.bytes)+",\"output_bytes\":"+std::to_string(metadata.output.bytes)+",\"backend_transient_bytes\":"+std::to_string(metadata.execution_bytes-metadata.input.bytes-metadata.output.bytes)+
                        ",\"evaluation_buffer_bytes\":"+std::to_string(data.expected.size()+data.rgb.size()*4+predicted.size()*4+warm.size()*4)+",\"additional_scratch_bytes\":0";
                    row+="}";check(row.size()<3500,Status::shape);evaluation_emit(row);
                };
                emit("reference-conv",measured,true);
                if(selected==provider){std::fill(warm.begin(),warm.end(),0.0f);emit("all-background",evaluate(1,size,size,data.expected.data(),pixels,warm.data(),pixels),false);
                    std::fill(warm.begin(),warm.end(),1.0f);emit("all-foreground",evaluate(1,size,size,data.expected.data(),pixels,warm.data(),pixels),false);}
            }
            expect(pixaura_inference_release(&context,&model));pixaura_inference_model_metadata invalid{};expect(pixaura_inference_model_query(&context,&model,&invalid,sizeof(invalid)),14);
            expect(pixaura_inference_release(&context,&package));pixaura_inference_resources final{};expect(pixaura_inference_resources_query(&context,&final,sizeof(final)));check(final.handles==0&&final.reserved_bytes==control,Status::identity);
        }catch(const segmentation::Error& error){++failures;std::fprintf(stderr,"Segmentation evaluation rejected: provider=%.*s size=%u reason=%d\n",static_cast<int>(selected.size()),selected.data(),size,static_cast<int>(error.status));}
        catch(...){++failures;}
    }
    return failures;
}
}
#endif
