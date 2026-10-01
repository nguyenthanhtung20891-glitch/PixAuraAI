#include "pixaura/core.h"
#include <cstdio>
#include <cstring>

#define CHECK(expression) do { if (!(expression)) { \
    std::fprintf(stderr, "Failed line %d: %s\n", __LINE__, #expression); \
    return 1; } } while (false)

int main() {
    pixaura_core_info info{99u, 99u, 99u, 99u};
    const pixaura_core_info sentinel = info;
    CHECK(pixaura_get_core_info(1u, nullptr, sizeof(info)) == PIXAURA_INVALID_ARGUMENT);
    CHECK(pixaura_get_core_info(1u, &info, 0u) == PIXAURA_INVALID_ARGUMENT);
    CHECK(pixaura_get_core_info(1u, &info, sizeof(info) - 1u) == PIXAURA_INVALID_ARGUMENT);
    CHECK(std::memcmp(&info, &sentinel, sizeof(info)) == 0);
    CHECK(pixaura_get_core_info(0u, &info, sizeof(info)) == PIXAURA_UNSUPPORTED_ABI);
    CHECK(pixaura_get_core_info(2u, &info, sizeof(info)) == PIXAURA_UNSUPPORTED_ABI);
    CHECK(std::memcmp(&info, &sentinel, sizeof(info)) == 0);
    struct Extended { pixaura_core_info info; uint32_t tail; } extended{sentinel, 123u};
    CHECK(pixaura_get_core_info(1u, &extended.info, sizeof(extended)) == PIXAURA_OK);
    CHECK(extended.tail == 123u);
    CHECK(extended.info.abi_version == 1u);
    CHECK(extended.info.struct_size == 16u);
    CHECK(extended.info.implemented_features == 0u);
    CHECK(extended.info.reserved == 0u);
    std::puts("C++ ABI safety checks passed");
    return 0;
}
