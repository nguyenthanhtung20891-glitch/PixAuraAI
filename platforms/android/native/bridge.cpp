#include <jni.h>
#include "pixaura/core.h"
#include "pixaura/document.h"
#include "pixaura/storage.h"
#include "pixaura/decode.h"
#include "pixaura/working.h"
#include "pixaura/evaluation.h"
#include <vector>

extern "C" JNIEXPORT jint JNICALL
Java_ai_pixaura_bridge_CoreProbe_nativeDecodeCheck(JNIEnv* env,jobject,jbyteArray root,jbyteArray digest,
    jlong asset_bytes,jbyteArray identity,jlong encoded_limit) {
    if(!root||!digest||!identity||asset_bytes<=0||encoded_limit<=0)return -1;
    const auto n=env->GetArrayLength(root);
    if(n<=0||n>1024||env->GetArrayLength(digest)!=64||env->GetArrayLength(identity)!=32)return -1;
    uint8_t path[1024],hash[64],id[32];
    env->GetByteArrayRegion(root,0,n,reinterpret_cast<jbyte*>(path));
    env->GetByteArrayRegion(digest,0,64,reinterpret_cast<jbyte*>(hash));
    env->GetByteArrayRegion(identity,0,32,reinterpret_cast<jbyte*>(id));
    if(env->ExceptionCheck())return -1;
    pixaura_decode_limits limits{};pixaura_decode_default_limits(1,&limits);
    limits.encoded_bytes=static_cast<uint64_t>(encoded_limit);
    pixaura_decode_context context{};auto status=pixaura_decode_context_init(1,&context,sizeof(context),id,32,&limits);
    if(status!=0)return -status;
    struct Cleanup{pixaura_decode_context& c;~Cleanup(){pixaura_decode_context_destroy(&c);}} cleanup{context};
    pixaura_decode_handle source{},image{};pixaura_decode_metadata metadata{};
    status=pixaura_decode_open(&context,path,static_cast<uint64_t>(n),hash,64,static_cast<uint64_t>(asset_bytes),&source);
    if(status==0)status=pixaura_decode_query(&context,&source,&metadata);
    if(status==0)status=pixaura_decode_image(&context,&source,&image);
    if(status==0)status=pixaura_decode_release(&context,&source);
    if(status==0)status=pixaura_decode_query(&context,&image,&metadata);
    pixaura_working_limits working_limits{};pixaura_working_metadata working_metadata{};
    pixaura_decode_handle working{},identity_image{};
    if(status==0)status=pixaura_working_default_limits(1,&working_limits);
    if(status==0)status=pixaura_working_normalize(&context,&image,&working_limits,&working);
    if(status==0)status=pixaura_working_query(&context,&working,&working_metadata);
    if(status==0)status=pixaura_working_identity(&context,&working,&working_limits,&identity_image);
    if(status==0){
        const uint8_t request[]="{\"operations\":[{\"id\":\"00000000000000000000000000000001\",\"type\":\"pixaura.exposure\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":1000}}]}";
        pixaura_decode_handle evaluated{};
        status=pixaura_evaluation_validate(1,request,sizeof(request)-1);
        if(status==0)status=pixaura_working_evaluate(&context,&working,1,request,sizeof(request)-1,&working_limits,&evaluated);
        if(status==0)status=pixaura_working_query(&context,&evaluated,&working_metadata);
        if(status==0&&pixaura_evaluation_validate(1,reinterpret_cast<const uint8_t*>("{}"),2)==0)status=14;
        if(status==0)status=pixaura_decode_release(&context,&evaluated);
    }
    if(status==0)status=pixaura_decode_release(&context,&working);
    if(status==0)status=pixaura_working_query(&context,&identity_image,&working_metadata);
    if(status==0)status=pixaura_decode_release(&context,&identity_image);
    if(status==0)status=pixaura_decode_release(&context,&image);
    return status==0?static_cast<jint>(working_metadata.width):-status;
}

extern "C" JNIEXPORT jint JNICALL
Java_ai_pixaura_bridge_CoreProbe_nativeStorageVersion(JNIEnv* env, jobject, jbyteArray root) {
    if(root==nullptr)return -1;
    const auto length=env->GetArrayLength(root);if(length<=0||length>1024)return -1;
    uint8_t path[1024];env->GetByteArrayRegion(root,0,length,reinterpret_cast<jbyte*>(path));if(env->ExceptionCheck())return -1;
    pixaura_storage_info info{};
    const auto status=pixaura_storage_check(1,path,static_cast<uint64_t>(length),&info);
    return status==0?static_cast<jint>(info.storage_version):-status;
}

extern "C" JNIEXPORT jint JNICALL
Java_ai_pixaura_bridge_CoreProbe_nativeAbiVersion(JNIEnv*, jobject) {
    pixaura_core_info info{};
    if (pixaura_get_core_info(PIXAURA_ABI_VERSION, &info, sizeof(info)) != PIXAURA_OK) {
        return -1;
    }
    return static_cast<jint>(info.abi_version);
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_ai_pixaura_bridge_CoreProbe_nativeDocumentRoundTrip(JNIEnv* env, jobject,
    jbyteArray manifest, jbyteArray context_identity, jbyteArray session_identity) {
    if (manifest == nullptr || context_identity == nullptr || session_identity == nullptr) return nullptr;
    const auto length = env->GetArrayLength(manifest);
    if (length <= 0 || length > 8 * 1024 * 1024 || env->GetArrayLength(context_identity) != 32 || env->GetArrayLength(session_identity) != 32) return nullptr;
    try {
        std::vector<uint8_t> input(static_cast<std::size_t>(length)); uint8_t owner[32], session[32];
        env->GetByteArrayRegion(manifest, 0, length, reinterpret_cast<jbyte*>(input.data()));
        env->GetByteArrayRegion(context_identity, 0, 32, reinterpret_cast<jbyte*>(owner));
        env->GetByteArrayRegion(session_identity, 0, 32, reinterpret_cast<jbyte*>(session));
        if (env->ExceptionCheck()) return nullptr;
        struct OwnedContext {
            pixaura_document_context value{};
            bool initialized = false;
            ~OwnedContext() { if (initialized) pixaura_document_context_destroy(&value); }
        } context;
        if (pixaura_document_context_init(1, &context.value, sizeof(context.value), owner, 32, nullptr) != 0) return nullptr;
        context.initialized = true;
        pixaura_document_handle handle{};
        if (pixaura_document_open(1, &context.value, input.data(), input.size(), session, 32, &handle, nullptr) != 0) return nullptr;
        uint64_t required = 0;
        if (pixaura_document_serialize(&context.value, &handle, nullptr, 0, &required, nullptr) != 0 || required > 8 * 1024 * 1024) return nullptr;
        std::vector<uint8_t> output(static_cast<std::size_t>(required));
        if (pixaura_document_serialize(&context.value, &handle, output.data(), output.size(), &required, nullptr) != 0) return nullptr;
        const auto result = env->NewByteArray(static_cast<jsize>(required));
        if (result == nullptr) return nullptr;
        env->SetByteArrayRegion(result, 0, static_cast<jsize>(required), reinterpret_cast<const jbyte*>(output.data()));
        return env->ExceptionCheck() ? nullptr : result;
    } catch (...) { return nullptr; }
}
