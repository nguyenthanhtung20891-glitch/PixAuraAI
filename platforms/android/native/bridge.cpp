#include <jni.h>
#include "pixaura/core.h"
#include "pixaura/document.h"
#include "pixaura/storage.h"
#include "pixaura/decode.h"
#include "pixaura/working.h"
#include "pixaura/evaluation.h"
#include "../../../packages/core/tests/geometry_boundary.h"
#include "../../../packages/core/tests/preview_boundary.h"
#include <vector>
#include <cstring>
#include <new>

namespace {
// Java owns this fixed direct-buffer storage. No native owning address is exported.
struct InteractiveStorage { pixaura_decode_context context{};pixaura_decode_handle working{};uint8_t identity[32]{}; };
InteractiveStorage* interactive_storage(JNIEnv* env,jobject buffer){
    if(!buffer||env->GetDirectBufferCapacity(buffer)!=static_cast<jlong>(sizeof(InteractiveStorage)))return nullptr;
    auto* p=env->GetDirectBufferAddress(buffer);
    if(!p||reinterpret_cast<uintptr_t>(p)%alignof(InteractiveStorage)!=0)return nullptr;
    return static_cast<InteractiveStorage*>(p);
}
pixaura_preview_ticket interactive_ticket(const InteractiveStorage& s,jlong generation){
    pixaura_preview_ticket t{};t.version=1;t.struct_size=sizeof(t);t.kind=PIXAURA_PREVIEW_TICKET_KIND;
    std::memcpy(t.context_id,s.identity,32);std::memcpy(&t.generation,&generation,8);return t;
}
jintArray preview_words(JNIEnv* env,pixaura_decode_context& context,const pixaura_decode_handle& preview){
    pixaura_preview_metadata m{};if(pixaura_preview_query(&context,&preview,&m)!=0)return nullptr;
    if(m.width==0||m.height==0||m.width>1024||m.height>1024||m.image_bytes!=uint64_t(m.width)*m.height*4)return nullptr;
    const auto result=env->NewIntArray(static_cast<jsize>(2+uint64_t(m.width)*m.height));if(!result)return nullptr;
    const jint dims[2]={static_cast<jint>(m.width),static_cast<jint>(m.height)};env->SetIntArrayRegion(result,0,2,dims);
    uint8_t row[4096];jint words[1024];
    for(uint32_t y=0;y<m.height;++y){
        if(pixaura_preview_copy(&context,&preview,uint64_t(y)*m.row_stride,row,m.row_stride)!=0)return nullptr;
        for(uint32_t x=0;x<m.width;++x){const auto* p=row+x*4;const uint32_t word=(uint32_t(p[3])<<24)|(uint32_t(p[0])<<16)|(uint32_t(p[1])<<8)|p[2];std::memcpy(&words[x],&word,4);}
        env->SetIntArrayRegion(result,static_cast<jsize>(2+uint64_t(y)*m.width),static_cast<jsize>(m.width),words);
        if(env->ExceptionCheck())return nullptr;
    }
    return env->ExceptionCheck()?nullptr:result;
}
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_InteractivePreview_nativeStorageSize(JNIEnv*,jobject){return sizeof(InteractiveStorage);}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_InteractivePreview_nativeInit(JNIEnv* env,jobject,jobject buffer,jbyteArray root,jbyteArray digest,jlong bytes,jbyteArray identity){
    auto* s=interactive_storage(env,buffer);if(!s||!root||!digest||!identity||bytes<=0)return 1;
    const auto n=env->GetArrayLength(root);if(n<=0||n>1024||env->GetArrayLength(digest)!=64||env->GetArrayLength(identity)!=32)return 1;
    new(s) InteractiveStorage{};uint8_t path[1024],hash[64];env->GetByteArrayRegion(root,0,n,reinterpret_cast<jbyte*>(path));
    env->GetByteArrayRegion(digest,0,64,reinterpret_cast<jbyte*>(hash));env->GetByteArrayRegion(identity,0,32,reinterpret_cast<jbyte*>(s->identity));
    if(env->ExceptionCheck())return 1;
    pixaura_decode_limits limits{};pixaura_decode_default_limits(1,&limits);
    auto status=pixaura_decode_context_init(1,&s->context,sizeof(s->context),s->identity,32,&limits);if(status)return status;
    pixaura_decode_handle source{},decoded{},working{};pixaura_working_limits wl{};
    status=pixaura_decode_open(&s->context,path,static_cast<uint64_t>(n),hash,64,static_cast<uint64_t>(bytes),&source);
    if(!status)status=pixaura_decode_image(&s->context,&source,&decoded);
    if(!status)status=pixaura_working_default_limits(1,&wl);
    if(!status)status=pixaura_working_normalize(&s->context,&decoded,&wl,&working);
    const uint8_t request[]="{\"operations\":[]}";
    if(!status)status=pixaura_working_evaluate(&s->context,&working,1,request,sizeof(request)-1,&wl,&s->working);
    if(status){pixaura_decode_context_destroy(&s->context);return status;}
    pixaura_decode_release(&s->context,&source);pixaura_decode_release(&s->context,&decoded);pixaura_decode_release(&s->context,&working);return 0;
}
extern "C" JNIEXPORT jlong JNICALL Java_ai_pixaura_bridge_InteractivePreview_nativeBegin(JNIEnv* env,jobject,jobject buffer){
    auto* s=interactive_storage(env,buffer);pixaura_preview_ticket t{};if(!s||pixaura_preview_begin(&s->context,1,&t)!=0)return 0;
    jlong generation;std::memcpy(&generation,&t.generation,8);return generation;
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_InteractivePreview_nativeCancel(JNIEnv* env,jobject,jobject buffer,jlong generation){
    auto* s=interactive_storage(env,buffer);if(!s)return 1;const auto t=interactive_ticket(*s,generation);return pixaura_preview_cancel(&s->context,&t);
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_InteractivePreview_nativeCurrent(JNIEnv* env,jobject,jobject buffer,jlong generation){
    auto* s=interactive_storage(env,buffer);if(!s)return 1;const auto t=interactive_ticket(*s,generation);return pixaura_preview_current(&s->context,&t,nullptr);
}
extern "C" JNIEXPORT jintArray JNICALL Java_ai_pixaura_bridge_InteractivePreview_nativeRender(JNIEnv* env,jobject,jobject buffer,jlong generation,jint width,jint height){
    auto* s=interactive_storage(env,buffer);if(!s||width<1||height<1||width>1024||height>1024)return nullptr;
    const auto t=interactive_ticket(*s,generation);const pixaura_preview_request fit{1,sizeof(pixaura_preview_request),1,static_cast<uint32_t>(width),static_cast<uint32_t>(height),0};
    pixaura_decode_handle preview{};if(pixaura_preview_render_interactive(&s->context,&s->working,&t,&fit,&preview)!=0)return nullptr;
    const auto result=preview_words(env,s->context,preview);pixaura_preview_release(&s->context,&preview);return result;
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_InteractivePreview_nativeStop(JNIEnv* env,jobject,jobject buffer){
    auto* s=interactive_storage(env,buffer);return s?pixaura_preview_stop(&s->context):1;
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_InteractivePreview_nativeDestroy(JNIEnv* env,jobject,jobject buffer){
    auto* s=interactive_storage(env,buffer);return s?pixaura_decode_context_destroy(&s->context):1;
}

extern "C" JNIEXPORT jintArray JNICALL
Java_ai_pixaura_bridge_CoreProbe_nativePreviewPixels(JNIEnv* env,jobject,jbyteArray root,jbyteArray digest,
    jlong asset_bytes,jbyteArray identity,jint max_width,jint max_height){
    if(!root||!digest||!identity||asset_bytes<=0||max_width<1||max_height<1||max_width>1024||max_height>1024)return nullptr;
    const auto n=env->GetArrayLength(root);
    if(n<=0||n>1024||env->GetArrayLength(digest)!=64||env->GetArrayLength(identity)!=32)return nullptr;
    uint8_t path[1024],hash[64],id[32];
    env->GetByteArrayRegion(root,0,n,reinterpret_cast<jbyte*>(path));
    env->GetByteArrayRegion(digest,0,64,reinterpret_cast<jbyte*>(hash));
    env->GetByteArrayRegion(identity,0,32,reinterpret_cast<jbyte*>(id));
    if(env->ExceptionCheck())return nullptr;
    pixaura_decode_limits limits{};if(pixaura_decode_default_limits(1,&limits)!=0)return nullptr;
    pixaura_decode_context context{};
    if(pixaura_decode_context_init(1,&context,sizeof(context),id,32,&limits)!=0)return nullptr;
    struct Cleanup{pixaura_decode_context& c;~Cleanup(){pixaura_decode_context_destroy(&c);}} cleanup{context};
    pixaura_decode_handle source{},decoded{},working{},evaluated{},preview{};pixaura_working_limits wl{};
    if(pixaura_decode_open(&context,path,static_cast<uint64_t>(n),hash,64,static_cast<uint64_t>(asset_bytes),&source)!=0||
       pixaura_decode_image(&context,&source,&decoded)!=0||pixaura_working_default_limits(1,&wl)!=0||
       pixaura_working_normalize(&context,&decoded,&wl,&working)!=0)return nullptr;
    if(pixaura_decode_release(&context,&source)!=0||pixaura_decode_release(&context,&decoded)!=0)return nullptr;
    const uint8_t request[]="{\"operations\":[]}";
    if(pixaura_working_evaluate(&context,&working,1,request,sizeof(request)-1,&wl,&evaluated)!=0)return nullptr;
    if(pixaura_decode_release(&context,&working)!=0)return nullptr;
    const pixaura_preview_request fit{1,sizeof(pixaura_preview_request),1,static_cast<uint32_t>(max_width),static_cast<uint32_t>(max_height),0};
    if(pixaura_preview_render(&context,&evaluated,&fit,nullptr,&preview)!=0||pixaura_decode_release(&context,&evaluated)!=0)return nullptr;
    pixaura_preview_metadata m{};if(pixaura_preview_query(&context,&preview,&m)!=0)return nullptr;
    if(m.width>1024||m.height>1024||m.image_bytes!=uint64_t(m.width)*m.height*4)return nullptr;
    const auto result=env->NewIntArray(static_cast<jsize>(2+uint64_t(m.width)*m.height));if(!result)return nullptr;
    const jint dims[2]={static_cast<jint>(m.width),static_cast<jint>(m.height)};env->SetIntArrayRegion(result,0,2,dims);
    // Fixed bounded stack row only; all returned pixels are VM-owned copies.
    uint8_t row[4096];jint words[1024];
    for(uint32_t y=0;y<m.height;++y){
        if(pixaura_preview_copy(&context,&preview,uint64_t(y)*m.row_stride,row,m.row_stride)!=0)return nullptr;
        for(uint32_t x=0;x<m.width;++x){const auto* p=row+x*4;const uint32_t word=(uint32_t(p[3])<<24)|(uint32_t(p[0])<<16)|(uint32_t(p[1])<<8)|p[2];std::memcpy(&words[x],&word,4);}
        env->SetIntArrayRegion(result,static_cast<jsize>(2+uint64_t(y)*m.width),static_cast<jsize>(m.width),words);
        if(env->ExceptionCheck())return nullptr;
    }
    if(pixaura_preview_release(&context,&preview)!=0)return nullptr;
    return env->ExceptionCheck()?nullptr:result;
}

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
    if(status==0)status=preview_boundary_check(&context,&working,&working_limits);
    if(status==0)status=geometry_boundary_check(&context,&working,&working_limits);
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
