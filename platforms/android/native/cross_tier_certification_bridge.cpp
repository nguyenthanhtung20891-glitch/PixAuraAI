#include <jni.h>
#include "../../../packages/core/tests/certification/include/cross_tier_certification.h"
// Only debug/certification builds contain this symbol. Its caller class lives
// solely in androidTest; no production profile injection or exported component.
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_app_CrossTierReplayTest_nativeCertify(JNIEnv* env,jobject,jstring root) {
    if(!root)return 1;const auto count=env->GetStringUTFLength(root);if(count<=0||count>800)return 1;
    const auto* path=env->GetStringUTFChars(root,nullptr);if(!path)return 1;
    const auto status=pixaura_test_cross_tier_replay(path,3);env->ReleaseStringUTFChars(root,path);return status;
}
