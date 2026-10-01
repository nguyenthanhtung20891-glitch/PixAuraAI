#include "pixaura/core.h"

static_assert(sizeof(pixaura_core_info) == 16, "ABI layout must remain stable");

int32_t pixaura_get_core_info(uint32_t requested_abi,
                             pixaura_core_info* output,
                             uint32_t output_bytes) {
    if (output == nullptr || output_bytes < sizeof(pixaura_core_info)) {
        return PIXAURA_INVALID_ARGUMENT;
    }
    if (requested_abi != PIXAURA_ABI_VERSION) {
        return PIXAURA_UNSUPPORTED_ABI;
    }
    *output = {PIXAURA_ABI_VERSION, sizeof(pixaura_core_info), 0u, 0u};
    return PIXAURA_OK;
}
