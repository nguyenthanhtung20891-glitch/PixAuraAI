#include "capability_boundary.h"
#include <stdio.h>
int main(void) {
    int status = capability_boundary_check();
    if (status != 0) { fprintf(stderr, "Capability foundation failed at line %d\n", status); return 1; }
    puts("Capability v1: synthetic LOW/MID/HIGH, canonical bytes, unknown/validation/privacy boundaries and unchanged ceilings PASS");
    return 0;
}
