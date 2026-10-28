#include <jni.h>
#include "pixaura/editor.h"
#include "pixaura/assist.h"
#include "pixaura/auto_proposal.h"
#include "pixaura/review_preview.h"
#include <cstring>
#include <vector>
#include <algorithm>
namespace {
template<class T>T* storage(JNIEnv* env,jobject buffer){if(!buffer||env->GetDirectBufferCapacity(buffer)!=static_cast<jlong>(sizeof(T)))return nullptr;
    auto* p=env->GetDirectBufferAddress(buffer);return p&&reinterpret_cast<uintptr_t>(p)%alignof(T)==0?static_cast<T*>(p):nullptr;}
struct Input{JNIEnv* env;jobject stream;jbyteArray buffer;jmethodID read;};
int64_t read(void* raw,uint8_t* output,uint64_t capacity){auto& r=*static_cast<Input*>(raw);if(capacity>65536)return -1;
    const auto n=r.env->CallIntMethod(r.stream,r.read,r.buffer,0,static_cast<jint>(capacity));
    if(r.env->ExceptionCheck()){r.env->ExceptionClear();return -1;}if(n==-1)return 0;if(n<=0||static_cast<uint64_t>(n)>capacity)return -1;
    r.env->GetByteArrayRegion(r.buffer,0,n,reinterpret_cast<jbyte*>(output));return r.env->ExceptionCheck()?-1:n;}
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeAssistSize(JNIEnv*,jobject){return sizeof(pixaura_assist_advice);}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeAutoSize(JNIEnv*,jobject){return sizeof(pixaura_auto_proposal);}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeReviewBegin(JNIEnv* env,jobject,jobject buffer,jobject proposal,jbyteArray plan,jobject ticket){
    auto* c=storage<pixaura_editor_context>(env,buffer);auto* p=storage<pixaura_auto_proposal>(env,proposal);auto* t=storage<pixaura_review_preview_ticket>(env,ticket);
    if(!c||!p||!t||!plan)return 1;const auto n=env->GetArrayLength(plan);if(n<=0||n>8192)return 8;
    uint8_t input[8192];env->GetByteArrayRegion(plan,0,n,reinterpret_cast<jbyte*>(input));if(env->ExceptionCheck())return 1;
    return pixaura_review_preview_begin(1,c,p,input,static_cast<uint64_t>(n),512,512,t);
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeReviewRender(JNIEnv* env,jobject,jobject buffer,jobject ticket,jobject pair){
    auto* c=storage<pixaura_editor_context>(env,buffer);auto* t=storage<pixaura_review_preview_ticket>(env,ticket);auto* p=storage<pixaura_review_preview_pair>(env,pair);
    return c&&t&&p?pixaura_review_preview_render(1,c,t,p):1;
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeReviewCurrent(JNIEnv* env,jobject,jobject buffer,jobject pair){
    auto* c=storage<pixaura_editor_context>(env,buffer);auto* p=storage<pixaura_review_preview_pair>(env,pair);return c&&p?pixaura_review_preview_current(1,c,p):1;
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeReviewCancel(JNIEnv* env,jobject,jobject buffer,jobject ticket){
    auto* c=storage<pixaura_editor_context>(env,buffer);auto* t=storage<pixaura_review_preview_ticket>(env,ticket);return c&&t?pixaura_review_preview_cancel(1,c,t):1;
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeReviewRelease(JNIEnv* env,jobject,jobject buffer,jobject pair){
    auto* c=storage<pixaura_editor_context>(env,buffer);auto* p=storage<pixaura_review_preview_pair>(env,pair);return c&&p?pixaura_review_preview_release(1,c,p):1;
}
extern "C" JNIEXPORT jintArray JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeReviewImage(JNIEnv* env,jobject,jobject buffer,jobject pair,jint side){
    auto* c=storage<pixaura_editor_context>(env,buffer);auto* p=storage<pixaura_review_preview_pair>(env,pair);
    if(!c||!p||side<0||side>1)return nullptr;const auto current=pixaura_review_preview_current(1,c,p);
    if(current){
        auto failure=env->NewIntArray(3);if(!failure)return nullptr;
        const jint values[3]={current,0,0};env->SetIntArrayRegion(failure,0,3,values);
        return env->ExceptionCheck()?nullptr:failure;
    }
    const auto& m=side?p->after:p->before;if(!m.width||!m.height||m.width>1024||m.height>1024||m.image_bytes!=uint64_t(m.width)*m.height*4)return nullptr;
    auto output=env->NewIntArray(static_cast<jsize>(3+uint64_t(m.width)*m.height));if(!output)return nullptr;
    jint header[3]={0,static_cast<jint>(m.width),static_cast<jint>(m.height)};env->SetIntArrayRegion(output,0,3,header);
    uint8_t rgba[4096];jint row[1024];
    for(uint32_t y=0;y<m.height;++y){const auto status=pixaura_review_preview_copy(1,c,p,static_cast<uint32_t>(side),uint64_t(y)*m.width*4,rgba,uint64_t(m.width)*4);
        if(status){header[0]=status;env->SetIntArrayRegion(output,0,3,header);return env->ExceptionCheck()?nullptr:output;}
        for(uint32_t x=0;x<m.width;++x){const auto* pixel=rgba+x*4;const uint32_t word=(uint32_t(pixel[3])<<24)|(uint32_t(pixel[0])<<16)|(uint32_t(pixel[1])<<8)|pixel[2];std::memcpy(row+x,&word,4);}
        env->SetIntArrayRegion(output,static_cast<jsize>(3+uint64_t(y)*m.width),static_cast<jsize>(m.width),row);if(env->ExceptionCheck())return nullptr;
    }
    return output;
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeAutoRequest(JNIEnv* env,jobject,jobject buffer,jint index,jobject result){
    auto* c=storage<pixaura_editor_context>(env,buffer);auto* out=storage<pixaura_auto_proposal>(env,result);
    if(!c||!out||index<0)return 1;
    const pixaura_auto_request request{1,sizeof(pixaura_auto_request),static_cast<uint32_t>(index),2};
    return pixaura_editor_auto_request(1,c,&request,nullptr,0,out);
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeAutoPlan(JNIEnv* env,jobject,jobject buffer,jobject packet,jobject output){
    auto* c=storage<pixaura_editor_context>(env,buffer);auto* p=storage<pixaura_auto_proposal>(env,packet);
    if(!c||!p||!output||env->GetDirectBufferCapacity(output)!=8192)return -1;
    auto* bytes=static_cast<uint8_t*>(env->GetDirectBufferAddress(output));if(!bytes)return -1;
    uint64_t required=0;const auto code=pixaura_editor_auto_plan(1,c,p,bytes,8192,&required);
    return code?-code:static_cast<jint>(required);
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeAutoCancel(JNIEnv* env,jobject,jobject buffer){
    auto* c=storage<pixaura_editor_context>(env,buffer);return c?pixaura_editor_auto_cancel(1,c):1;
}
#ifdef PIXAURA_CERTIFICATION_TESTING
extern "C" int32_t pixaura_test_review_preview_control(pixaura_editor_context*,uint32_t,uint32_t);
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_app_ReviewPreviewTestHooks_nativeControl(JNIEnv* env,jobject,jobject buffer,jint command,jint value){
    auto* c=storage<pixaura_editor_context>(env,buffer);return c?pixaura_test_review_preview_control(c,static_cast<uint32_t>(command),static_cast<uint32_t>(value)):1;
}
extern "C" int32_t pixaura_test_auto_control(pixaura_editor_context*,uint32_t,uint32_t);
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_app_AutoTestHooks_nativeControl(JNIEnv* env,jobject,jobject buffer,jint command,jint value){
    auto* c=storage<pixaura_editor_context>(env,buffer);return c?pixaura_test_auto_control(c,static_cast<uint32_t>(command),static_cast<uint32_t>(value)):1;
}
#endif
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeAssistCount(JNIEnv*,jobject){return PIXAURA_ASSIST_CHOICES;}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeAssistChoice(JNIEnv* env,jobject,jint index,jobject result){
    auto* out=storage<pixaura_assist_advice>(env,result);return out&&index>=0?pixaura_assist_choice(1,static_cast<uint32_t>(index),out):1;
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeAssistRequest(JNIEnv* env,jobject,jobject buffer,jint index,jobject result){
    auto* c=storage<pixaura_editor_context>(env,buffer);auto* out=storage<pixaura_assist_advice>(env,result);
    if(!c||!out||index<0)return 1;
    const pixaura_assist_request request{1,sizeof(pixaura_assist_request),static_cast<uint32_t>(index),0};
    return pixaura_editor_assist_request(1,c,&request,out);
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeAssistCurrent(JNIEnv* env,jobject,jobject buffer,jobject result){
    auto* c=storage<pixaura_editor_context>(env,buffer);auto* out=storage<pixaura_assist_advice>(env,result);
    return c&&out?pixaura_editor_assist_current(1,c,out):1;
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeAssistCancel(JNIEnv* env,jobject,jobject buffer){
    auto* c=storage<pixaura_editor_context>(env,buffer);return c?pixaura_editor_assist_cancel(1,c):1;
}
#ifdef PIXAURA_CERTIFICATION_TESTING
extern "C" int32_t pixaura_test_assist_control(pixaura_editor_context*,uint32_t,uint32_t);
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_app_AssistTestHooks_nativeControl(JNIEnv* env,jobject,jobject buffer,jint command,jint value){
    auto* c=storage<pixaura_editor_context>(env,buffer);return c?pixaura_test_assist_control(c,static_cast<uint32_t>(command),static_cast<uint32_t>(value)):1;
}
#endif
namespace {
bool words(JNIEnv* env,jlongArray input,jlong* output,jsize count){
    if(!input||env->GetArrayLength(input)!=count)return false;
    env->GetLongArrayRegion(input,0,count,output);
    if(env->ExceptionCheck())return false;
    for(jsize i=0;i<count;++i)if(output[i]<0)return false;
    return true;
}
int32_t observation(JNIEnv* env,jlongArray input,pixaura_policy_observation& o){
    jlong v[7]{};if(!words(env,input,v,7))return 1;
    if(v[0]!=1)return 2;
    if(v[1]>4||v[2]>4||v[3]>1||v[4]>2||v[6]==0)return 1;
    o={};o.version=1;o.struct_size=sizeof(o);o.thermal=static_cast<uint32_t>(v[1]);
    o.memory_pressure=static_cast<uint32_t>(v[2]);o.constrained=static_cast<uint32_t>(v[3]);
    o.gpu_availability=static_cast<uint32_t>(v[4]);o.process_budget_bytes=static_cast<uint64_t>(v[5]);
    o.sequence=static_cast<uint64_t>(v[6]); // Trust is never accepted from Java.
    return 0;
}
jlongArray strategy(JNIEnv* env,int32_t status,const pixaura_execution_strategy& s){
    const jlong v[11]={status,s.tier,static_cast<jlong>(s.runtime_cap_bytes),s.preview_edge,s.concurrency,
        s.backend,s.forced_cpu,s.reason,s.recovery_pending,s.stable_observations,static_cast<jlong>(s.epoch)};
    auto result=env->NewLongArray(11);if(!result)return nullptr;
    env->SetLongArrayRegion(result,0,11,v);return env->ExceptionCheck()?nullptr:result;
}
int32_t profile(JNIEnv* env,jlongArray facts,pixaura_capability_profile& p){
    jlong v[6]{};if(!words(env,facts,v,6))return 1;
    if(v[0]!=1)return 2;
    if(v[1]>PIXAURA_CAPABILITY_MAX_CPUS||v[2]>UINT32_MAX||v[5]>2)return 1;
    const auto status=pixaura_capability_baseline(1,&p);if(status)return status;
    p.hardware.logical_cpu_count=static_cast<uint32_t>(v[1]);p.hardware.runtime_version=static_cast<uint32_t>(v[2]);
    p.hardware.physical_memory_bytes=static_cast<uint64_t>(v[3]);p.hardware.gpu_backend=PIXAURA_CAPABILITY_GPU_VULKAN;
    p.hardware.gpu_availability=static_cast<uint32_t>(v[5]);p.hardware.gpu_validation=PIXAURA_GPU_NOT_VALIDATED;
    // Conservative UNOBSERVED provenance: never assert physical trust from OS/model/emulator hints.
    p.runtime.thermal=PIXAURA_THERMAL_NORMAL;p.runtime.memory_pressure=PIXAURA_PRESSURE_NORMAL;p.runtime.constrained=0;
    p.runtime.gpu_availability=p.hardware.gpu_availability;p.runtime.process_budget_bytes=static_cast<uint64_t>(v[4]);p.runtime.sequence=1;
    return pixaura_capability_validate(1,&p);
}
}
extern "C" JNIEXPORT jlongArray JNICALL Java_ai_pixaura_bridge_ManualEditor_nativePolicyConfigure(
    JNIEnv* env,jobject,jobject buffer,jlongArray facts,jlongArray input){
    pixaura_execution_strategy s{};pixaura_policy_observation o{};
    auto* c=storage<pixaura_editor_context>(env,buffer);
    if(!c)return strategy(env,1,s);
    const auto observation_status=observation(env,input,o);if(observation_status)return strategy(env,observation_status,s);
    pixaura_capability_profile p{};auto status=profile(env,facts,p);
    if(status)return strategy(env,status,s);
    status=pixaura_editor_policy_configure(1,c,&p,&o,&s);return strategy(env,status,s);
}
extern "C" JNIEXPORT jlongArray JNICALL Java_ai_pixaura_bridge_ManualEditor_nativePolicyUpdate(
    JNIEnv* env,jobject,jobject buffer,jlongArray input){
    pixaura_execution_strategy s{};pixaura_policy_observation o{};auto* c=storage<pixaura_editor_context>(env,buffer);
    if(!c)return strategy(env,1,s);
    const auto parsed=observation(env,input,o);
    const auto status=parsed?parsed:pixaura_editor_policy_update(1,c,&o,&s);
    return strategy(env,status,s);
}
extern "C" JNIEXPORT jlongArray JNICALL Java_ai_pixaura_bridge_ManualEditor_nativePolicyQuery(JNIEnv* env,jobject,jobject buffer){
    pixaura_execution_strategy s{};auto* c=storage<pixaura_editor_context>(env,buffer);
    const auto status=c?pixaura_editor_policy_query(1,c,&s):1;return strategy(env,status,s);
}
extern "C" JNIEXPORT jlong JNICALL Java_ai_pixaura_bridge_CoreProbe_nativeImportLimit(JNIEnv*,jobject){pixaura_decode_limits limits{};return pixaura_decode_default_limits(1,&limits)==0?static_cast<jlong>(limits.encoded_bytes):0;}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeStorageSize(JNIEnv*,jobject){return sizeof(pixaura_editor_context);}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeRequestSize(JNIEnv*,jobject){return sizeof(pixaura_editor_request);}
extern "C" JNIEXPORT jlong JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeGeneration(JNIEnv* env,jobject,jobject request){auto* r=storage<pixaura_editor_request>(env,request);jlong generation=0;if(r)std::memcpy(&generation,&r->ticket.generation,8);return generation;}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeOpen(JNIEnv* env,jobject,jobject buffer,jbyteArray root,jbyteArray identity,jobject input,jlong bytes,jlongArray facts,jlongArray dynamic){
    auto* c=storage<pixaura_editor_context>(env,buffer);if(!c||!root||!identity||bytes<0||env->GetArrayLength(identity)!=32)return 1;
    const auto n=env->GetArrayLength(root);if(n<1||n>1024)return 1;
    uint8_t path[1024],id[32];env->GetByteArrayRegion(root,0,n,reinterpret_cast<jbyte*>(path));env->GetByteArrayRegion(identity,0,32,reinterpret_cast<jbyte*>(id));
    if(env->ExceptionCheck())return 1;Input r{env,input,nullptr,nullptr};
    pixaura_capability_profile p{};pixaura_policy_observation o{};pixaura_execution_strategy execution{};
    if(facts||dynamic){if(!facts||!dynamic)return 1;
        const auto fact_status=profile(env,facts,p);if(fact_status)return fact_status;
        const auto observation_status=observation(env,dynamic,o);if(observation_status)return observation_status;}
    if(bytes){if(!input)return 1;const auto cls=env->GetObjectClass(input);r.read=env->GetMethodID(cls,"read","([BII)I");env->DeleteLocalRef(cls);
        if(!r.read||env->ExceptionCheck())return 12;r.buffer=env->NewByteArray(65536);if(!r.buffer)return 8;}
    const auto status=facts?pixaura_editor_open_policy(1,c,path,static_cast<uint64_t>(n),id,32,bytes?read:nullptr,&r,static_cast<uint64_t>(bytes),&p,&o,&execution):
        pixaura_editor_open(1,c,path,static_cast<uint64_t>(n),id,32,bytes?read:nullptr,&r,static_cast<uint64_t>(bytes));
    if(r.buffer)env->DeleteLocalRef(r.buffer);return status;
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeSelect(JNIEnv* env,jobject,jobject buffer,jint index){auto* c=storage<pixaura_editor_context>(env,buffer);return c?pixaura_editor_select(c,static_cast<uint32_t>(index)):1;}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeUpdate(JNIEnv* env,jobject,jobject buffer,jintArray input){auto* c=storage<pixaura_editor_context>(env,buffer);if(!c||!input)return 1;
    const auto n=env->GetArrayLength(input);if(n<1||n>8)return 7;int32_t values[8];env->GetIntArrayRegion(input,0,n,values);return env->ExceptionCheck()?1:pixaura_editor_update(c,values,static_cast<uint32_t>(n));}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeAction(JNIEnv* env,jobject,jobject buffer,jint action){auto* c=storage<pixaura_editor_context>(env,buffer);return c?pixaura_editor_action(c,static_cast<uint32_t>(action)):1;}
extern "C" JNIEXPORT jstring JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeState(JNIEnv* env,jobject,jobject buffer){auto* c=storage<pixaura_editor_context>(env,buffer);if(!c)return nullptr;
    char bytes[1024]{};uint64_t n=0;if(pixaura_editor_state(c,reinterpret_cast<uint8_t*>(bytes),sizeof(bytes)-1,&n)!=0)return nullptr;return env->NewStringUTF(bytes);}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeRequest(JNIEnv* env,jobject,jobject buffer,jobject request){auto* c=storage<pixaura_editor_context>(env,buffer);auto* r=storage<pixaura_editor_request>(env,request);return c&&r?pixaura_editor_request_preview(c,r):1;}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeCurrent(JNIEnv* env,jobject,jobject buffer,jobject request){auto* c=storage<pixaura_editor_context>(env,buffer);auto* r=storage<pixaura_editor_request>(env,request);return c&&r?pixaura_editor_current(c,r):1;}
extern "C" JNIEXPORT jintArray JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeRender(JNIEnv* env,jobject,jobject buffer,jobject request){
    auto* c=storage<pixaura_editor_context>(env,buffer);auto* r=storage<pixaura_editor_request>(env,request);if(!c||!r)return nullptr;
    try{std::vector<uint8_t> pixels(512*512*4);uint32_t w=0,h=0;const auto status=pixaura_editor_render(c,r,512,512,pixels.data(),pixels.size(),&w,&h);
        auto out=env->NewIntArray(static_cast<jsize>(3+(status?0:uint64_t(w)*h)));if(!out)return nullptr;
        const jint header[3]={status,static_cast<jint>(w),static_cast<jint>(h)};env->SetIntArrayRegion(out,0,3,header);
        if(!status){jint row[512];for(uint32_t y=0;y<h;++y){for(uint32_t x=0;x<w;++x){const auto* p=pixels.data()+(uint64_t(y)*w+x)*4;
                    const uint32_t word=(uint32_t(p[3])<<24)|(uint32_t(p[0])<<16)|(uint32_t(p[1])<<8)|p[2];std::memcpy(row+x,&word,4);}
                env->SetIntArrayRegion(out,static_cast<jsize>(3+y*w),static_cast<jsize>(w),row);if(env->ExceptionCheck())return nullptr;}}
        return out;
    }catch(...){return nullptr;}
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeStop(JNIEnv* env,jobject,jobject buffer){auto* c=storage<pixaura_editor_context>(env,buffer);return c?pixaura_editor_stop(c):1;}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeDestroy(JNIEnv* env,jobject,jobject buffer){auto* c=storage<pixaura_editor_context>(env,buffer);return c?pixaura_editor_destroy(c):1;}
