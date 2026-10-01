#include "pixaura/core.h"
#include <cstdlib>

// Test-only caller violation: lie about capacity to prove the shared library's
// actual write is instrumented. This is never linked into the application.
int main() {
    auto* output = static_cast<pixaura_core_info*>(std::malloc(8u));
    if (output == nullptr) {
        return 2;
    }
    const auto status = pixaura_get_core_info(PIXAURA_ABI_VERSION, output,
                                             sizeof(pixaura_core_info));
    std::free(output);
    return status == PIXAURA_OK ? 0 : 3;
}
