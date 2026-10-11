#include <jni.h>
#include "../../../packages/core/tests/android_publication_boundary.hpp"
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_CoreProbe_nativeAndroidAssetPublication(JNIEnv* env,jobject,jbyteArray root) {
    if(!root)return -1;
    const auto n=env->GetArrayLength(root);if(n<1||n>1024)return -1;
    char path[1025]{};env->GetByteArrayRegion(root,0,n,reinterpret_cast<jbyte*>(path));if(env->ExceptionCheck())return -1;
    try{return static_cast<jint>(pixaura::testing::android_publication_boundary(path));}catch(...){return -1;}
}
