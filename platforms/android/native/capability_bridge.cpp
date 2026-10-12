#include <jni.h>
#include "pixaura/capability.h"
#include "../../../packages/core/tests/capability_boundary.h"
#include "../../../packages/core/tests/budget_boundary.h"
#include "../../../packages/core/tests/dynamic_policy_boundary.h"
extern "C" JNIEXPORT jbyteArray JNICALL Java_ai_pixaura_bridge_CoreProbe_nativeCapabilityBaseline(JNIEnv* env, jobject) {
    pixaura_capability_profile profile{};
    uint8_t bytes[PIXAURA_CAPABILITY_WIRE_BYTES]{};
    uint64_t required = 0;
    if (pixaura_capability_baseline(1, &profile) != 0 ||
        pixaura_capability_write(1, &profile, bytes, sizeof(bytes), &required) != 0) return nullptr;
    auto output = env->NewByteArray(static_cast<jsize>(required));
    if (!output) return nullptr;
    env->SetByteArrayRegion(output, 0, static_cast<jsize>(required), reinterpret_cast<const jbyte*>(bytes));
    return env->ExceptionCheck() ? nullptr : output;
}
// Argument-free CoreProbe diagnostics only. Synthetic corpus contexts are
// isolated: no editor state, persistent profile or GPU dispatch is created.
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_CoreProbe_nativeCapabilityFoundationBoundary(JNIEnv*, jobject) {
    return capability_boundary_check();
}
// Argument-free diagnostics only. No untrusted profile/occupancy injection.
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_CoreProbe_nativeBudgetBoundary(JNIEnv*, jobject) {
    return budget_boundary_check();
}
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_CoreProbe_nativeDynamicPolicyBoundary(JNIEnv*, jobject) {
    return dynamic_policy_boundary_check();
}
