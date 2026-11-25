// Included by editor_bridge.cpp, sharing its bounded trusted OS fact translators.
// No private SQLite, editing implementation, paid boolean or diagnostic loader.
#include "pixaura/editor_v2.h"
namespace {
struct V2Text {
    char bytes[4096]{};uint64_t count=0;
    bool load(JNIEnv* env,jbyteArray input,uint64_t limit){
        if(!input)return false;const auto n=env->GetArrayLength(input);
        if(n<=0||static_cast<uint64_t>(n)>limit||n>4095)return false;
        env->GetByteArrayRegion(input,0,n,reinterpret_cast<jbyte*>(bytes));if(env->ExceptionCheck())return false;
        count=static_cast<uint64_t>(n);return std::memchr(bytes,0,static_cast<size_t>(n))==nullptr;
    }
};
struct V2Key {
    uint8_t bytes[32]{};
    ~V2Key(){volatile uint8_t* p=bytes;for(unsigned i=0;i<32;++i)p[i]=0;}
};
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_runtimeCreate(JNIEnv* env,jobject,jbyteArray path,jbyteArray key,jlongArray facts,jlongArray dynamic,jobject output){
    auto* out=storage<uint64_t>(env,output);V2Text root;V2Key secret;
    if(!out||!root.load(env,path,4095)||!key||env->GetArrayLength(key)!=32)return 1;
    env->GetByteArrayRegion(key,0,32,reinterpret_cast<jbyte*>(secret.bytes));if(env->ExceptionCheck())return 1;
    if(!facts&&!dynamic)return pixaura_v2_runtime_create(2,root.bytes,root.count,secret.bytes,32,out);
    pixaura_capability_profile p{};pixaura_policy_observation o{};pixaura_execution_strategy s{};
    const auto a=profile(env,facts,p);if(a)return a;const auto b=observation(env,dynamic,o);if(b)return b;
    return pixaura_v2_runtime_create_policy(2,root.bytes,root.count,secret.bytes,32,&p,&o,&s,out);
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_runtimeRecovery(JNIEnv* env,jobject,jbyteArray path,jlongArray facts,jlongArray dynamic,jobject output){
    auto* out=storage<uint64_t>(env,output);V2Text root;if(!out||!root.load(env,path,4095))return 1;
    if(!facts&&!dynamic)return pixaura_v2_runtime_recovery_create(2,root.bytes,root.count,nullptr,nullptr,nullptr,out);
    pixaura_capability_profile p{};pixaura_policy_observation o{};pixaura_execution_strategy s{};
    const auto a=profile(env,facts,p);if(a)return a;const auto b=observation(env,dynamic,o);if(b)return b;
    return pixaura_v2_runtime_recovery_create(2,root.bytes,root.count,&p,&o,&s,out);
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_runtimeClose(JNIEnv*,jobject,jlong h){return pixaura_v2_runtime_close(2,static_cast<uint64_t>(h));}
extern "C" JNIEXPORT jlongArray JNICALL Java_ai_pixaura_bridge_NativeEditorV2_runtimePolicy(JNIEnv* env,jobject,jlong h,jlongArray dynamic){
    pixaura_execution_strategy s{};int32_t code=0;
    if(dynamic){pixaura_policy_observation o{};code=observation(env,dynamic,o);if(!code)code=pixaura_v2_runtime_update(2,static_cast<uint64_t>(h),&o,&s);}
    else code=pixaura_v2_runtime_strategy(2,static_cast<uint64_t>(h),&s);
    return strategy(env,code,s);
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_commercial(JNIEnv* env,jobject,jlong h,jobject output){auto* out=storage<pixaura_v2_commercial_state>(env,output);return out?pixaura_v2_runtime_commercial_state(2,static_cast<uint64_t>(h),out):1;}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_open(JNIEnv* env,jobject,jlong runtime,jbyteArray path,jint fd,jlong bytes,jboolean migrate,jobject output){
    auto* out=storage<uint64_t>(env,output);V2Text root;if(!out||bytes<0||!root.load(env,path,4095))return 1;
    return pixaura_v2_editor_open(2,static_cast<uint64_t>(runtime),root.bytes,root.count,fd,static_cast<uint64_t>(bytes),migrate?1:0,out);
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_state(JNIEnv* env,jobject,jlong h,jobject output){auto* out=storage<pixaura_v2_state>(env,output);return out?pixaura_v2_editor_state(2,static_cast<uint64_t>(h),out):1;}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_action(JNIEnv*,jobject,jlong h,jint kind,jint value){
    const auto owner=static_cast<uint64_t>(h);uint32_t changed=0;
    switch(kind){case 0:return pixaura_v2_editor_cancel(2,owner);case 1:return pixaura_v2_editor_apply(2,owner,&changed);
        case 2:return pixaura_v2_editor_history(2,owner,0);case 3:return pixaura_v2_editor_history(2,owner,1);
        case 4:return pixaura_v2_editor_stop(2,owner);case 5:return pixaura_v2_editor_close(2,owner);
        case 6:return value<0?1:pixaura_v2_editor_select(2,owner,static_cast<uint32_t>(value));default:return 1;}
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_update(JNIEnv* env,jobject,jlong h,jintArray input){
    if(!input)return 1;const auto n=env->GetArrayLength(input);if(n<1||n>4)return 1;int32_t values[4]{};env->GetIntArrayRegion(input,0,n,values);if(env->ExceptionCheck())return 1;
    return pixaura_v2_editor_update(2,static_cast<uint64_t>(h),values,static_cast<uint32_t>(n));
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_preview(JNIEnv* env,jobject,jlong h,jint width,jint height,jint kind,jbyteArray reference,jobject output){
    auto* out=storage<pixaura_v2_preview>(env,output);if(!out||width<=0||height<=0)return 1;
    if(kind==2||kind==3){V2Text id;if(!id.load(env,reference,32))return 1;
        if(kind==2)return pixaura_v2_advisory_preview(2,static_cast<uint64_t>(h),id.bytes,id.count,static_cast<uint32_t>(width),static_cast<uint32_t>(height),out);
        return pixaura_v2_editor_revision_preview(2,static_cast<uint64_t>(h),id.bytes,id.count,static_cast<uint32_t>(width),static_cast<uint32_t>(height),out);
    }
    if(kind<0||kind>1||reference)return 1;
    return pixaura_v2_editor_preview(2,static_cast<uint64_t>(h),static_cast<uint32_t>(width),static_cast<uint32_t>(height),static_cast<uint32_t>(kind),out);
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_history(JNIEnv* env,jobject,jlong h,jobject output){
    if(!output||env->GetDirectBufferCapacity(output)!=1048576)return -1;auto* p=static_cast<uint8_t*>(env->GetDirectBufferAddress(output));if(!p)return -1;
    uint64_t required=0;const auto code=pixaura_v2_editor_history_json(2,static_cast<uint64_t>(h),p,1048576,&required);return code?-code:static_cast<jint>(required);
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_frame(JNIEnv* env,jobject,jlong h,jlong f,jint kind,jobject output){
    if(kind==3||kind==4){auto* out=storage<uint32_t>(env,output);return out?pixaura_v2_frame_probe(2,static_cast<uint64_t>(h),static_cast<uint64_t>(f),kind==4?1u:0u,out):1;}
    if(kind==1)return pixaura_v2_frame_current(2,static_cast<uint64_t>(h),static_cast<uint64_t>(f));
    if(kind==2)return pixaura_v2_frame_release(2,static_cast<uint64_t>(f));
    if(kind!=0||!output)return 1;const auto n=env->GetDirectBufferCapacity(output);auto* p=static_cast<uint8_t*>(env->GetDirectBufferAddress(output));
    if(!p||n<0||n>4194304)return 1;return pixaura_v2_frame_copy(2,static_cast<uint64_t>(h),static_cast<uint64_t>(f),p,static_cast<uint64_t>(n));
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_export(JNIEnv* env,jobject,jlong h,jint format,jint quality,jint fd,jobject output){auto* out=storage<uint64_t>(env,output);return out?pixaura_v2_editor_export(2,static_cast<uint64_t>(h),static_cast<uint32_t>(format),quality,fd,out):1;}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_advisory(JNIEnv* env,jobject,jlong h,jint kind,jint choice,jbyteArray input,jbyteArray content,jobject output){
    auto* out=storage<pixaura_v2_advisory>(env,output);if(!out)return 1;const auto owner=static_cast<uint64_t>(h);
    if(kind==0)return choice<0?1:pixaura_v2_advisory_request(2,owner,static_cast<uint32_t>(choice),out);
    V2Text id;if(!id.load(env,input,kind==1?64:32))return 1;
    if(kind==1)return pixaura_v2_advisory_restore(2,owner,id.bytes,id.count,out);
    if(kind==2){if(!content)return 1;const auto n=env->GetArrayLength(content);if(n<1||n>8192)return 1;uint8_t bytes[8192];env->GetByteArrayRegion(content,0,n,reinterpret_cast<jbyte*>(bytes));if(env->ExceptionCheck())return 1;return pixaura_v2_advisory_edit(2,owner,id.bytes,id.count,bytes,static_cast<uint64_t>(n),out);}
    if(kind==3)return pixaura_v2_advisory_reset(2,owner,id.bytes,id.count,out);return 1;
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_content(JNIEnv* env,jobject,jlong h,jbyteArray input,jboolean operations,jobject output){
    V2Text id;if(!id.load(env,input,32)||!output||env->GetDirectBufferCapacity(output)!=8192)return -1;auto* p=static_cast<uint8_t*>(env->GetDirectBufferAddress(output));if(!p)return -1;
    uint64_t required=0;const auto code=pixaura_v2_advisory_content(2,static_cast<uint64_t>(h),id.bytes,id.count,operations?1:0,p,8192,&required);return code?-code:static_cast<jint>(required);
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_agentBytes(JNIEnv* env,jobject,jlong h,jint mode,jbyteArray input,jobject output){
    const auto n=input?env->GetArrayLength(input):0;std::array<uint8_t,8192> bytes{};
    if(n<0||n>8192||!output||env->GetDirectBufferCapacity(output)!=8192)return -1;
    auto* out=static_cast<uint8_t*>(env->GetDirectBufferAddress(output));if(!out)return -1;
    if(n)env->GetByteArrayRegion(input,0,n,reinterpret_cast<jbyte*>(bytes.data()));if(env->ExceptionCheck())return -1;
    uint64_t required=0;const auto code=pixaura_v2_editor_agent_bytes(2,static_cast<uint64_t>(h),static_cast<uint32_t>(mode),bytes.data(),static_cast<uint64_t>(n),out,8192,&required);
    return code?-code:static_cast<jint>(required);
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_planBegin(JNIEnv* env,jobject,jlong h,jbyteArray input,jobject output){
    const auto n=input?env->GetArrayLength(input):0;std::array<uint8_t,8192> bytes{};auto* out=storage<uint64_t>(env,output);
    if(n<=0||n>8192||!out)return 1;env->GetByteArrayRegion(input,0,n,reinterpret_cast<jbyte*>(bytes.data()));if(env->ExceptionCheck())return 1;
    return pixaura_v2_advisory_plan_begin(2,static_cast<uint64_t>(h),bytes.data(),static_cast<uint64_t>(n),out);
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_pending(JNIEnv* env,jobject,jlong h,jobject output){
    if(!output||env->GetDirectBufferCapacity(output)!=8192)return -1;auto* p=static_cast<uint8_t*>(env->GetDirectBufferAddress(output));if(!p)return -1;
    uint64_t required=0;const auto code=pixaura_v2_advisory_pending(2,static_cast<uint64_t>(h),p,8192,&required);return code?-code:static_cast<jint>(required);
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_humanApply(JNIEnv* env,jobject,jlong h,jbyteArray input){
    V2Text id;if(!id.load(env,input,32))return 1;char approval[33]{};uint32_t changed=0;
    const auto code=pixaura_v2_advisory_approve(2,static_cast<uint64_t>(h),id.bytes,id.count,approval,sizeof(approval));if(code)return code;
    return pixaura_v2_advisory_apply(2,static_cast<uint64_t>(h),id.bytes,id.count,approval,32,&changed);
}

extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_cancelAdvisory(JNIEnv* env,jobject,jlong h,jbyteArray input){
    V2Text id;if(!id.load(env,input,32))return 1;
    return pixaura_v2_advisory_cancel(2,static_cast<uint64_t>(h),id.bytes,id.count);
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_humanApplyReceipt(JNIEnv* env,jobject,jlong h,jbyteArray input,jobject output){
    auto* out=storage<pixaura_v2_commit_result>(env,output);V2Text id;
    if(!out||!id.load(env,input,32))return 1;char approval[33]{};
    const auto owner=static_cast<uint64_t>(h);
    const auto code=pixaura_v2_advisory_approve(2,owner,id.bytes,id.count,approval,sizeof(approval));if(code)return code;
    return pixaura_v2_advisory_apply_receipt(2,owner,id.bytes,id.count,approval,32,out);
}

extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_requestTicket(JNIEnv* env,jobject,jlong h,jint kind,jlong ticket,jint choice,jobject output){
    const auto owner=static_cast<uint64_t>(h);
    if(kind==0){auto* out=storage<uint64_t>(env,output);return out&&choice>=0?pixaura_v2_advisory_begin(2,owner,static_cast<uint32_t>(choice),out):1;}
    if(kind==1)return pixaura_v2_advisory_request_cancel(2,owner,static_cast<uint64_t>(ticket));
    if(kind==2){auto* out=storage<pixaura_v2_advisory>(env,output);return out?pixaura_v2_advisory_run(2,owner,static_cast<uint64_t>(ticket),out):1;}
    return 1;
}

extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_exportObserved(JNIEnv* env,jobject,jlong h,jint format,jint quality,jint fd,jobject output){
    auto* out=storage<pixaura_v2_export_result>(env,output);
    return out?pixaura_v2_editor_export_observed(2,static_cast<uint64_t>(h),static_cast<uint32_t>(format),quality,fd,out):1;
}

extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_reviewPreview(JNIEnv* env,jobject,jlong h,jint kind,jlong ticket,jbyteArray input,jint width,jint height,jobject output){
    const auto owner=static_cast<uint64_t>(h),token=static_cast<uint64_t>(ticket);
    if(kind==0){auto* out=storage<uint64_t>(env,output);V2Text id;if(!out||!id.load(env,input,32)||width<=0||height<=0)return 1;
        return pixaura_v2_review_preview_begin(2,owner,id.bytes,id.count,static_cast<uint32_t>(width),static_cast<uint32_t>(height),out);}
    if(kind==1){if(!output||env->GetDirectBufferCapacity(output)!=80)return 1;auto* out=static_cast<pixaura_v2_preview*>(env->GetDirectBufferAddress(output));
        if(!out||reinterpret_cast<uintptr_t>(out)%alignof(pixaura_v2_preview))return 1;
        return pixaura_v2_review_preview_render(2,owner,token,out,out+1);}
    if(kind==2)return pixaura_v2_review_preview_cancel(2,owner,token);
    if(kind==3)return pixaura_v2_review_preview_release(2,owner,token);
    return 1;
}

extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_exportTask(JNIEnv* env,jobject,jlong h,jint kind,jlong ticket,jint format,jint quality,jint fd,jobject output){
    const auto owner=static_cast<uint64_t>(h),token=static_cast<uint64_t>(ticket);
    if(kind==0){auto* out=storage<uint64_t>(env,output);return out&&format>=0?pixaura_v2_export_begin(2,owner,static_cast<uint32_t>(format),quality,out):1;}
    if(kind==1){auto* out=storage<pixaura_v2_export_result>(env,output);return out?pixaura_v2_export_run(2,owner,token,fd,out):1;}
    if(kind==2)return pixaura_v2_export_cancel(2,owner,token);
    if(kind==3)return pixaura_v2_export_release(2,owner,token);
    return 1;
}

extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_editIdentity(JNIEnv* env,jobject,jlong h,jobject output){
    if(!output||env->GetDirectBufferCapacity(output)!=33)return 1;auto* out=static_cast<char*>(env->GetDirectBufferAddress(output));
    return out?pixaura_v2_editor_edit_identity(2,static_cast<uint64_t>(h),out,33):1;
}

extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_inspectReview(JNIEnv* env,jobject,jlong h,jbyteArray input,jobject output){
    auto* out=storage<pixaura_v2_review_inspection>(env,output);V2Text id;if(!out||!id.load(env,input,32))return 1;
    return pixaura_v2_advisory_inspect(2,static_cast<uint64_t>(h),id.bytes,id.count,out);
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_reviewCommand(JNIEnv* env,jobject,jlong h,jbyteArray input,jint command,jint action,jint parameter,jint value,jint destination,jobject output){
    auto* out=storage<pixaura_v2_advisory>(env,output);V2Text id;if(!out||!id.load(env,input,32))return 1;
    return pixaura_v2_advisory_edit_command(2,static_cast<uint64_t>(h),id.bytes,id.count,static_cast<uint32_t>(command),static_cast<uint32_t>(action),static_cast<uint32_t>(parameter),value,static_cast<uint32_t>(destination),out);
}

#ifdef PIXAURA_CERTIFICATION_TESTING
extern "C" int32_t pixaura_test_editor_v2_control(uint64_t,uint32_t,uint32_t,uint32_t);
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_app_VersionedEditorTestHooks_nativeControl(JNIEnv*,jobject,jlong owner,jint family,jint command,jint value){
    return pixaura_test_editor_v2_control(static_cast<uint64_t>(owner),static_cast<uint32_t>(family),static_cast<uint32_t>(command),static_cast<uint32_t>(value));
}
#endif

#include "store_ingress_v3_bridge.inc"
