#include "pixaura/core.h"
#include <stdio.h>

int main(void) {
    pixaura_core_info info = {0u, 0u, 0u, 0u};
    if (pixaura_get_core_info(PIXAURA_ABI_VERSION, &info, sizeof(info)) != PIXAURA_OK ||
        info.abi_version != PIXAURA_ABI_VERSION || info.struct_size != 16u ||
        info.implemented_features != 0u || info.reserved != 0u) {
        return 1;
    }
    puts("Independent C consumer linked and passed");
    return 0;
}
