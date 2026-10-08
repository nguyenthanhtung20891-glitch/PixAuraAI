#include "pixaura/manual.h"
#include <stdint.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) return __LINE__; } while (0)
int main(void) {
    uint64_t needed = 99;
    uint8_t output[8192];
    memset(output, 0xa5, sizeof output);
    CHECK(pixaura_manual_registry_json(2, output, sizeof output, &needed) == PIXAURA_UNSUPPORTED_ABI);
    CHECK(needed == 99 && output[0] == 0xa5);
    CHECK(pixaura_manual_registry_json(1, 0, 1, &needed) == PIXAURA_INVALID_ARGUMENT);
    CHECK(pixaura_manual_registry_json(1, output, 1, &needed) == PIXAURA_DOCUMENT_BUFFER_TOO_SMALL);
    CHECK(output[0] == 0xa5 && needed > 1 && needed < sizeof output);
    CHECK(pixaura_manual_registry_json(1, output, sizeof output, &needed) == 0);
    CHECK(output[needed - 1] == '\n');
    {
        const char* json = "{\"operations\":[{\"type\":\"pixaura.exposure\",\"parameters\":{\"milli_ev\":1},\"parameter_version\":1,\"operation_version\":1,\"id\":\"00000000000000000000000000000091\"}]}";
        const char* canonical = "{\"operations\":[{\"id\":\"00000000000000000000000000000091\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":1},\"type\":\"pixaura.exposure\"}]}\n";
        CHECK(pixaura_manual_canonical_operation(1, (const uint8_t*)json, strlen(json), output, sizeof output, &needed) == 0);
        CHECK(needed == strlen(canonical) && memcmp(output, canonical, (size_t)needed) == 0);
        needed = 99; output[0] = 0xa5;
        CHECK(pixaura_manual_canonical_operation(1, (const uint8_t*)json, 65537, output, sizeof output, &needed) == PIXAURA_DOCUMENT_RESOURCE_LIMIT);
        CHECK(needed == 99 && output[0] == 0xa5);
        CHECK(pixaura_manual_canonical_operation(1, 0, 0, output, sizeof output, &needed) == PIXAURA_INVALID_ARGUMENT);
        CHECK(pixaura_manual_canonical_operation(1, (const uint8_t*)"{}", 2, output, sizeof output, &needed) != 0);
        CHECK(needed == 99 && output[0] == 0xa5);
    }
    return 0;
}
