#include "capability_boundary.h"
#include "budget_boundary.h"
#include <stdio.h>
int main(void) {
    int status = capability_boundary_check();
    if (status != 0) { fprintf(stderr, "Capability foundation failed at line %d\n", status); return 1; }
    puts("Capability v1: synthetic LOW/MID/HIGH, canonical bytes, unknown/validation/privacy boundaries and unchanged ceilings PASS");
    status = budget_boundary_check();
    if (status != 0) { fprintf(stderr, "Budget admission failed at line %d\n", status); return 1; }
    puts("Budget v1: shared LOW/MID/HIGH, thresholds +/-1, overflow, occupancy, static downgrade, GPU trust and hard ceilings PASS");
    return 0;
}
