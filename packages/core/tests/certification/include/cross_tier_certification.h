#ifndef PIXAURA_TEST_CROSS_TIER_CERTIFICATION_H
#define PIXAURA_TEST_CROSS_TIER_CERTIFICATION_H
#include <stdint.h>
#include "pixaura/editor.h"
#ifdef __cplusplus
extern "C" {
#endif
/* TEST SUPPORT ONLY: not part of PixAuraCore's production product/C ABI.
 * Existing private test parent, platform 0/3/4. No profile/trust injection API. */
int32_t pixaura_test_cross_tier_replay(const char* private_test_parent,uint32_t platform);
PIXAURA_API int32_t pixaura_model_test_boundary(void);
PIXAURA_API int32_t pixaura_inference_test_boundary(void);
/* Apple debug/test only; no release corpus or trust surface. */
PIXAURA_API int32_t pixaura_inference_apple_test_boundary(void);
/* Implemented only in native test/debug core builds, never release product.
 * Avoid a second SQLite amalgamation inspecting an active editor connection. */
PIXAURA_API int32_t pixaura_test_editor_document(pixaura_editor_context*,uint8_t*,uint64_t,uint64_t*);
/* Register/clear only with renders quiescent; callback state survives join. */
PIXAURA_API int32_t pixaura_test_editor_observe(pixaura_editor_context*,void (*)(uint32_t,void*),void*);
#ifdef __cplusplus
}
#endif
#endif
