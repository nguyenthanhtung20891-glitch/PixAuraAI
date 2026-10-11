#ifndef PIXAURA_CAPABILITY_FIXTURES_H
#define PIXAURA_CAPABILITY_FIXTURES_H
#include "pixaura/capability.h"
/* SYNTHETIC POLICY PROFILE inputs, not measured devices or tier thresholds.
 * Labels describe representative test scenarios; Step 2 selects actual tiers.
 * Process budget is an observation, never an allowance to exceed hard ceilings.
 */
typedef struct capability_fixture { const char* label; uint32_t cpus; uint64_t memory, budget; } capability_fixture;
static const capability_fixture capability_fixtures[3] = {
    {"LOW", 2, UINT64_C(2147483648), UINT64_C(134217728)},
    {"MID", 4, UINT64_C(4294967296), UINT64_C(268435456)},
    {"HIGH", 8, UINT64_C(8589934592), UINT64_C(1073741824)}
};
#endif
