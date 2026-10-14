#ifndef PIXAURA_MODEL_PACKAGE_H
#define PIXAURA_MODEL_PACKAGE_H
#include "core.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PIXAURA_MODEL_PACKAGE_VERSION 1u
#define PIXAURA_MODEL_MANIFEST_LIMIT 65536u
#define PIXAURA_MODEL_FILE_LIMIT UINT64_C(67108864)
#define PIXAURA_MODEL_TOTAL_LIMIT UINT64_C(134217728)
enum pixaura_model_reason {
    PIXAURA_MODEL_OK, PIXAURA_MODEL_ARGUMENT, PIXAURA_MODEL_IO,
    PIXAURA_MODEL_UNSAFE_FILE, PIXAURA_MODEL_PATH, PIXAURA_MODEL_COUNT,
    PIXAURA_MODEL_MANIFEST_LIMIT_REASON, PIXAURA_MODEL_PARSE,
    PIXAURA_MODEL_VERSION, PIXAURA_MODEL_NONCANONICAL,
    PIXAURA_MODEL_ALGORITHM, PIXAURA_MODEL_SIGNATURE_VERSION,
    PIXAURA_MODEL_UNKNOWN_KEY, PIXAURA_MODEL_KEY_INVALID,
    PIXAURA_MODEL_SIGNATURE_SIZE, PIXAURA_MODEL_SIGNATURE_INVALID,
    PIXAURA_MODEL_TRUST_DOMAIN, PIXAURA_MODEL_PAYLOAD_MISSING,
    PIXAURA_MODEL_PAYLOAD_UNEXPECTED, PIXAURA_MODEL_SIZE,
    PIXAURA_MODEL_DIGEST, PIXAURA_MODEL_TASK, PIXAURA_MODEL_TENSOR,
    PIXAURA_MODEL_PROVIDER, PIXAURA_MODEL_LICENSE, PIXAURA_MODEL_RESOURCE,
    PIXAURA_MODEL_DUPLICATE, PIXAURA_MODEL_TRUST_VERSION
};
typedef struct pixaura_model_package pixaura_model_package;
typedef struct pixaura_model_diagnostic { uint32_t version, reason, ordinal, reserved; } pixaura_model_diagnostic;
typedef struct pixaura_model_info {
    uint32_t version, model_version, task, file_count, key_id, trust_generation;
    uint64_t payload_bytes, declared_memory_bytes;
    uint8_t manifest_sha256[32];
} pixaura_model_info;
/* Local flat directory; holds no-follow OS handles while reading into owned bytes.
 * Root is UTF-8, 1..4096 bytes. Verification is synchronous: call off UI threads.
 * No package handle published until all verification passes. Outputs unchanged on
 * failure except bounded diagnostic. Production trust store v1 is deliberately
 * empty pending approved production public-key provisioning. No caller can
 * supply production trust anchors. No load/infer/sign/key-generation API exists.
 * release requires exclusive caller ownership; never race destruction/use.
 */
PIXAURA_API int32_t pixaura_model_package_verify(uint32_t version,
    const char* root, uint64_t root_bytes, uint64_t memory_limit,
    pixaura_model_package** output, pixaura_model_diagnostic* diagnostic);
PIXAURA_API int32_t pixaura_model_package_info(uint32_t version,
    const pixaura_model_package* package, pixaura_model_info* output);
PIXAURA_API void pixaura_model_package_release(pixaura_model_package* package);
#ifdef __cplusplus
}
#endif
#endif
