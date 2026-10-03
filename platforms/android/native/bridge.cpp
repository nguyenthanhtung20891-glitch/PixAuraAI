#include <jni.h>
#include "pixaura/core.h"
#include "pixaura/document.h"
#include <vector>

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
