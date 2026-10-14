#include <jni.h>
#include <vulkan/vulkan.h>
#include "pixaura/capability.h"
#include "../../../packages/core/tests/capability_boundary.h"
#include "../../../packages/core/tests/budget_boundary.h"
#include "../../../packages/core/tests/dynamic_policy_boundary.h"
// Bounded availability probe only. No dispatch, device name, trust or cache.
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_CoreProbe_nativeVulkanAvailability(JNIEnv*,jobject){
    VkInstanceCreateInfo info{};info.sType=VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;VkInstance instance{};
    if(vkCreateInstance(&info,nullptr,&instance)!=VK_SUCCESS)return PIXAURA_CAPABILITY_UNAVAILABLE;
    struct Cleanup{VkInstance i;~Cleanup(){vkDestroyInstance(i,nullptr);}} cleanup{instance};
    uint32_t count=0;VkPhysicalDevice devices[32]{};
    if(vkEnumeratePhysicalDevices(instance,&count,nullptr)!=VK_SUCCESS||!count||count>32)return PIXAURA_CAPABILITY_UNAVAILABLE;
    if(vkEnumeratePhysicalDevices(instance,&count,devices)!=VK_SUCCESS)return PIXAURA_CAPABILITY_UNAVAILABLE;
    for(uint32_t i=0;i<count;++i){uint32_t n=0;VkQueueFamilyProperties queues[64]{};
        vkGetPhysicalDeviceQueueFamilyProperties(devices[i],&n,nullptr);if(!n||n>64)continue;
        vkGetPhysicalDeviceQueueFamilyProperties(devices[i],&n,queues);
        for(uint32_t j=0;j<n;++j)if(queues[j].queueCount&&(queues[j].queueFlags&VK_QUEUE_COMPUTE_BIT))return PIXAURA_CAPABILITY_AVAILABLE;
    }
    return PIXAURA_CAPABILITY_UNAVAILABLE;
}
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
#ifdef PIXAURA_CERTIFICATION_TESTING
extern "C" int32_t pixaura_inference_test_boundary();
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_CoreProbe_nativeInferenceBoundary(JNIEnv*, jobject) {
    return pixaura_inference_test_boundary();
}
extern "C" int32_t pixaura_model_test_boundary();
extern "C" JNIEXPORT jint JNICALL Java_ai_pixaura_bridge_CoreProbe_nativeModelPackageBoundary(JNIEnv*, jobject) {
    return pixaura_model_test_boundary();
}
#endif
