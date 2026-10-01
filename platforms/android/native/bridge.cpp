#include <jni.h>
#include "pixaura/core.h"

extern "C" JNIEXPORT jint JNICALL
Java_ai_pixaura_bridge_CoreProbe_nativeAbiVersion(JNIEnv*, jobject) {
    pixaura_core_info info{};
    if (pixaura_get_core_info(PIXAURA_ABI_VERSION, &info, sizeof(info)) != PIXAURA_OK) {
        return -1;
    }
    return static_cast<jint>(info.abi_version);
}
