#include <jni.h>
#include "pixaura/agent_execution.h"
#include <array>
#include <new>

namespace {
template<class T>T* agent_storage(JNIEnv* env,jobject buffer){
    if(!buffer||env->GetDirectBufferCapacity(buffer)!=static_cast<jlong>(sizeof(T)))return nullptr;
    auto* p=env->GetDirectBufferAddress(buffer);
    return p&&reinterpret_cast<uintptr_t>(p)%alignof(T)==0?static_cast<T*>(p):nullptr;
}
int32_t agent_input(JNIEnv* env,jbyteArray input,std::array<uint8_t,8192>& data,uint64_t& bytes){
    if(!input)return 1;
    const auto n=env->GetArrayLength(input);if(n<=0||n>8192)return 8;
    env->GetByteArrayRegion(input,0,n,reinterpret_cast<jbyte*>(data.data()));
    if(env->ExceptionCheck())return 1;
    bytes=static_cast<uint64_t>(n);return 0;
}
const pixaura_agent_execution_request agent_request{1,sizeof(pixaura_agent_execution_request),1,0,{0,0,0,0}};
struct AgentBytes{uint64_t size;uint8_t data[8192];};
static_assert(sizeof(AgentBytes)==8200,"bounded JNI byte transport");
static_assert(sizeof(pixaura_agent_approval)==56,"approval transport");
static_assert(sizeof(pixaura_agent_execution_receipt)==144,"receipt transport");
}

extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeAgentBytes(
    JNIEnv* env,jobject,jobject context,jint mode,jbyteArray input,jobject output){
    try{
        auto* c=agent_storage<pixaura_editor_context>(env,context);auto* out=agent_storage<AgentBytes>(env,output);
        if(!c||!out)return 1;
        if(mode==1){if(input)return 1;return pixaura_editor_agent_binding(1,c,out->data,sizeof(out->data),&out->size);}
        if(mode!=2&&mode!=3)return 1;
        std::array<uint8_t,8192> data{};uint64_t bytes=0;const auto status=agent_input(env,input,data,bytes);if(status)return status;
        if(mode==2)return pixaura_editor_agent_validate(1,c,&agent_request,data.data(),bytes,out->data,sizeof(out->data),&out->size);
        const pixaura_reference_planner_request request{1,sizeof(pixaura_reference_planner_request),1,1,0,{0,0,0}};
        return pixaura_editor_agent_plan(1,c,&request,data.data(),bytes,out->data,sizeof(out->data),&out->size);
    }catch(const std::bad_alloc&){return 8;}catch(...){return 14;}
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeAgentApprove(
    JNIEnv* env,jobject,jobject context,jbyteArray input,jobject token){
    try{
        auto* c=agent_storage<pixaura_editor_context>(env,context);auto* out=agent_storage<pixaura_agent_approval>(env,token);
        if(!c||!out)return 1;
        std::array<uint8_t,8192> data{};uint64_t bytes=0;const auto status=agent_input(env,input,data,bytes);if(status)return status;
        return pixaura_editor_agent_approve(1,c,&agent_request,data.data(),bytes,out);
    }catch(const std::bad_alloc&){return 8;}catch(...){return 14;}
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeAgentTerminal(
    JNIEnv* env,jobject,jobject context,jobject token,jboolean release){
    try{
        auto* c=agent_storage<pixaura_editor_context>(env,context);auto* a=agent_storage<pixaura_agent_approval>(env,token);
        if(!c||!a||(release!=JNI_TRUE&&release!=JNI_FALSE))return 1;
        return release==JNI_TRUE?pixaura_editor_agent_release(1,c,a):pixaura_editor_agent_revoke(1,c,a);
    }catch(...){return 14;}
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeAgentExecute(
    JNIEnv* env,jobject,jobject context,jbyteArray input,jobject token,jobject receipt){
    try{
        auto* c=agent_storage<pixaura_editor_context>(env,context);auto* out=agent_storage<pixaura_agent_execution_receipt>(env,receipt);
        auto* a=token?agent_storage<pixaura_agent_approval>(env,token):nullptr;
        if(!c||!out||(token&&!a))return 1;
        std::array<uint8_t,8192> data{};uint64_t bytes=0;const auto status=agent_input(env,input,data,bytes);if(status)return status;
        return pixaura_editor_agent_execute(1,c,&agent_request,a,data.data(),bytes,out);
    }catch(const std::bad_alloc&){return 8;}catch(...){return 14;}
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeAgentCancel(JNIEnv* env,jobject,jobject context){
    try{auto* c=agent_storage<pixaura_editor_context>(env,context);return c?pixaura_editor_agent_cancel(1,c):1;}catch(...){return 14;}
}
#ifdef PIXAURA_CERTIFICATION_TESTING
extern "C" int32_t pixaura_test_agent_platform_control(pixaura_editor_context*,uint32_t,uint32_t);
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_app_AgentPlatformHooks_nativeControl(JNIEnv* env,jobject,jobject context,jint command,jint point){
    auto* c=agent_storage<pixaura_editor_context>(env,context);
    return c?pixaura_test_agent_platform_control(c,static_cast<uint32_t>(command),static_cast<uint32_t>(point)):-1;
}
#endif
