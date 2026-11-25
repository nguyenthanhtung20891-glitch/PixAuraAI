#include "pixaura/store_ingress_v3.h"
#include <atomic>
#include <charconv>
#include <string>
#include <vector>
namespace {
static_assert(sizeof(pixaura_store_v3_acknowledgment)==276,"version3 Kotlin receipt layout");
struct StoreJNIEnvV3 {
    JavaVM* vm;JNIEnv* env=nullptr;bool attached=false;
    explicit StoreJNIEnvV3(JavaVM* value):vm(value){
        const auto status=vm->GetEnv(reinterpret_cast<void**>(&env),JNI_VERSION_1_6);
        if(status==JNI_EDETACHED){attached=vm->AttachCurrentThread(&env,nullptr)==JNI_OK;if(!attached)env=nullptr;}
        else if(status!=JNI_OK)env=nullptr;
    }
    ~StoreJNIEnvV3(){if(attached)vm->DetachCurrentThread();}
};
bool storeStringV3(JNIEnv* env,jstring input,std::string& result,size_t cap){
    if(!input)return false;const auto size=env->GetStringUTFLength(input);if(size<=0||static_cast<size_t>(size)>cap)return false;
    const auto* bytes=env->GetStringUTFChars(input,nullptr);if(!bytes)return false;
    result.assign(bytes,static_cast<size_t>(size));env->ReleaseStringUTFChars(input,bytes);return !env->ExceptionCheck();
}
bool storeNumberV3(const std::string& value,int64_t& number){
    const auto parsed=std::from_chars(value.data(),value.data()+value.size(),number);
    return !value.empty()&&parsed.ec==std::errc{}&&parsed.ptr==value.data()+value.size()&&std::to_string(number)==value;
}
template<size_t N>bool storeCopyV3(char (&out)[N],const std::string& value){
    if(value.empty()||value.size()>=N||value.find('\0')!=std::string::npos)return false;
    std::memcpy(out,value.data(),value.size());return true;
}
struct StoreJNIContextV3 {
    JavaVM* vm=nullptr;jobject verifier=nullptr;jmethodID method=nullptr;std::atomic<unsigned> references{1};
    static int32_t retain(void* raw){static_cast<StoreJNIContextV3*>(raw)->references.fetch_add(1);return 0;}
    static void release(void* raw){
        auto* value=static_cast<StoreJNIContextV3*>(raw);
        if(value->references.fetch_sub(1)==1){StoreJNIEnvV3 current(value->vm);if(current.env)current.env->DeleteGlobalRef(value->verifier);delete value;}
    }
    static int32_t verify(void* raw,const uint8_t* proof,uint64_t count,uint32_t origin,const char* nonce,uint64_t nonce_count,pixaura_store_v3_verified* out){
        try{
            auto& value=*static_cast<StoreJNIContextV3*>(raw);StoreJNIEnvV3 current(value.vm);auto* env=current.env;
            if(!env||!proof||count==0||count>16384||nonce_count>32||env->PushLocalFrame(24)!=JNI_OK)return 6;
            struct Frame {JNIEnv* env;~Frame(){env->PopLocalFrame(nullptr);}} frame{env};
            auto bytes=env->NewByteArray(static_cast<jsize>(count));auto challenge=env->NewByteArray(static_cast<jsize>(nonce_count));
            if(!bytes||!challenge){env->ExceptionClear();return 8;}
            env->SetByteArrayRegion(bytes,0,static_cast<jsize>(count),reinterpret_cast<const jbyte*>(proof));
            if(nonce_count)env->SetByteArrayRegion(challenge,0,static_cast<jsize>(nonce_count),reinterpret_cast<const jbyte*>(nonce));
            auto result=static_cast<jobjectArray>(env->CallObjectMethod(value.verifier,value.method,bytes,static_cast<jint>(origin),challenge));
            if(env->ExceptionCheck()){env->ExceptionClear();return 6;}
            if(!result||env->GetArrayLength(result)!=8)return 6;
            std::string fields[8];
            for(unsigned i=0;i<8;++i){auto field=static_cast<jstring>(env->GetObjectArrayElement(result,static_cast<jsize>(i)));if(!storeStringV3(env,field,fields[i],i==0?128:64))return 6;}
            int64_t kind=0,status=0,signed_at=0,fraction=0,expires=0;
            if(!storeNumberV3(fields[3],kind)||kind<1||kind>2||!storeNumberV3(fields[4],status)||status<1||status>3||
               !storeNumberV3(fields[5],signed_at)||!storeNumberV3(fields[6],fraction)||fraction<0||fraction>999||!storeNumberV3(fields[7],expires))return 6;
            pixaura_store_v3_verified checked{};checked.version=3;checked.struct_size=sizeof(checked);checked.kind=static_cast<uint32_t>(kind);checked.status=static_cast<uint32_t>(status);
            checked.signed_seconds=signed_at;checked.signed_millisecond=static_cast<uint32_t>(fraction);checked.expires=expires;
            if(!storeCopyV3(checked.product,fields[0])||!storeCopyV3(checked.transaction_digest,fields[1])||!storeCopyV3(checked.verified_key_identity,fields[2]))return 6;
            *out=checked;return 0;
        }catch(const std::bad_alloc&){return 8;}catch(...){return 6;}
    }
};
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_storeBind(JNIEnv* env,jobject,jlong owner,jbyteArray input,jobject verifier){
    try{
        V2Text body;if(!verifier||!body.load(env,input,2048))return 6;
        std::vector<std::string> fields;size_t start=0;
        for(size_t i=0;i<body.count;++i)if(body.bytes[i]=='\n'){fields.emplace_back(body.bytes+start,i-start);start=i+1;}
        if(start!=body.count||fields.size()<8||fields[0]!="PIXAURA_STORE_CONFIGURATION_V3")return 6;
        int64_t store=0,environment=0,count=0;
        if(!storeNumberV3(fields[1],store)||store<1||store>2||!storeNumberV3(fields[2],environment)||environment<1||environment>2||
           !storeNumberV3(fields[5],count)||count<1||count>8||fields.size()!=6+2*static_cast<size_t>(count))return 6;
        pixaura_store_v3_configuration c{};c.version=3;c.struct_size=sizeof(c);c.store=static_cast<uint32_t>(store);c.environment=static_cast<uint32_t>(environment);c.product_count=static_cast<uint32_t>(count);
        if(!storeCopyV3(c.application,fields[3])||!storeCopyV3(c.key_identity,fields[4]))return 6;
        for(unsigned i=0;i<c.product_count;++i){int64_t kind=0;if(!storeCopyV3(c.products[i].identifier,fields[6+2*i])||!storeNumberV3(fields[7+2*i],kind)||kind<1||kind>2)return 6;c.products[i].kind=static_cast<uint32_t>(kind);}
        auto* context=new StoreJNIContextV3();
        if(env->GetJavaVM(&context->vm)!=JNI_OK){delete context;return 6;}
        context->verifier=env->NewGlobalRef(verifier);auto type=env->GetObjectClass(verifier);
        if(type)context->method=env->GetMethodID(type,"verifyForNativeV3","([BI[B)[Ljava/lang/String;");
        if(type)env->DeleteLocalRef(type);
        if(!context->verifier||!context->method){if(env->ExceptionCheck())env->ExceptionClear();StoreJNIContextV3::release(context);return 6;}
        c.context=context;c.retain=StoreJNIContextV3::retain;c.release=StoreJNIContextV3::release;c.verify=StoreJNIContextV3::verify;
        const auto status=pixaura_store_v3_bind(3,static_cast<uint64_t>(owner),&c);StoreJNIContextV3::release(context);return status;
    }catch(const std::bad_alloc&){return 8;}catch(...){return 6;}
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_storeBegin(JNIEnv* env,jobject,jlong owner,jobject output){
    if(!output||env->GetDirectBufferCapacity(output)!=33)return 1;auto* bytes=static_cast<char*>(env->GetDirectBufferAddress(output));
    return bytes?pixaura_store_v3_begin(3,static_cast<uint64_t>(owner),bytes,33):1;
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_storeCancel(JNIEnv* env,jobject,jlong owner,jbyteArray nonce){
    V2Text challenge;if(!challenge.load(env,nonce,32))return 1;return pixaura_store_v3_cancel(3,static_cast<uint64_t>(owner),challenge.bytes,challenge.count);
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_NativeEditorV2_storeAccept(JNIEnv* env,jobject,jlong owner,jbyteArray proof,jbyteArray nonce,jobject output){
    try{
        auto* out=storage<pixaura_store_v3_acknowledgment>(env,output);V2Text challenge;
        if(!out||!challenge.load(env,nonce,32)||!proof)return 1;const auto count=env->GetArrayLength(proof);if(count<1||count>16384)return 6;
        std::vector<uint8_t> bytes(static_cast<size_t>(count));env->GetByteArrayRegion(proof,0,count,reinterpret_cast<jbyte*>(bytes.data()));if(env->ExceptionCheck())return 6;
        return pixaura_store_v3_accept(3,static_cast<uint64_t>(owner),bytes.data(),bytes.size(),challenge.bytes,challenge.count,out);
    }catch(const std::bad_alloc&){return 8;}catch(...){return 6;}
}
