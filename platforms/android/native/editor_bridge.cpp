#include <jni.h>
#include "pixaura/editor.h"
#include <cstring>
#include <vector>
namespace {
template<class T>T* storage(JNIEnv* env,jobject buffer){if(!buffer||env->GetDirectBufferCapacity(buffer)!=static_cast<jlong>(sizeof(T)))return nullptr;
    auto* p=env->GetDirectBufferAddress(buffer);return p&&reinterpret_cast<uintptr_t>(p)%alignof(T)==0?static_cast<T*>(p):nullptr;}
struct Input{JNIEnv* env;jobject stream;jbyteArray buffer;jmethodID read;};
int64_t read(void* raw,uint8_t* output,uint64_t capacity){auto& r=*static_cast<Input*>(raw);if(capacity>65536)return -1;
    const auto n=r.env->CallIntMethod(r.stream,r.read,r.buffer,0,static_cast<jint>(capacity));
    if(r.env->ExceptionCheck()){r.env->ExceptionClear();return -1;}if(n==-1)return 0;if(n<=0||static_cast<uint64_t>(n)>capacity)return -1;
    r.env->GetByteArrayRegion(r.buffer,0,n,reinterpret_cast<jbyte*>(output));return r.env->ExceptionCheck()?-1:n;}
}
extern "C" JNIEXPORT jlong JNICALL Java_ai_pixaura_bridge_CoreProbe_nativeImportLimit(JNIEnv*,jobject){pixaura_decode_limits limits{};return pixaura_decode_default_limits(1,&limits)==0?static_cast<jlong>(limits.encoded_bytes):0;}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeStorageSize(JNIEnv*,jobject){return sizeof(pixaura_editor_context);}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeRequestSize(JNIEnv*,jobject){return sizeof(pixaura_editor_request);}
extern "C" JNIEXPORT jlong JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeGeneration(JNIEnv* env,jobject,jobject request){auto* r=storage<pixaura_editor_request>(env,request);jlong generation=0;if(r)std::memcpy(&generation,&r->ticket.generation,8);return generation;}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_ManualEditor_nativeOpen(JNIEnv* env,jobject,jobject buffer,jbyteArray root,jbyteArray identity,jobject input,jlong bytes){
    auto* c=storage<pixaura_editor_context>(env,buffer);if(!c||!root||!identity||bytes<0||env->GetArrayLength(identity)!=32)return 1;
    const auto n=env->GetArrayLength(root);if(n<1||n>1024)return 1;
    uint8_t path[1024],id[32];env->GetByteArrayRegion(root,0,n,reinterpret_cast<jbyte*>(path));env->GetByteArrayRegion(identity,0,32,reinterpret_cast<jbyte*>(id));
    if(env->ExceptionCheck())return 1;Input r{env,input,nullptr,nullptr};
    if(bytes){if(!input)return 1;const auto cls=env->GetObjectClass(input);r.read=env->GetMethodID(cls,"read","([BII)I");env->DeleteLocalRef(cls);
        if(!r.read||env->ExceptionCheck())return 12;r.buffer=env->NewByteArray(65536);if(!r.buffer)return 8;}
    const auto status=pixaura_editor_open(1,c,path,static_cast<uint64_t>(n),id,32,bytes?read:nullptr,&r,static_cast<uint64_t>(bytes));
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
