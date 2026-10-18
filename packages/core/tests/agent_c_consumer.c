#include "pixaura/agent.h"
#include "pixaura/agent_schema.h"
#include <stdio.h>
#include "pixaura/reference_planner.h"
#include "pixaura/agent_tools.h"
int main(void) {
    pixaura_agent_proposal proposal = {0};
    if (sizeof(proposal) != 56 || offsetof(pixaura_agent_proposal, serial) != 48) return 1;
    if (sizeof(pixaura_agent_schema_request) != 32) return 4;
    if (pixaura_agent_schema_cancel(2, NULL) != PIXAURA_UNSUPPORTED_ABI) return 5;
    if (pixaura_agent_validate(2, NULL, NULL, NULL) != PIXAURA_UNSUPPORTED_ABI) return 2;
    if (pixaura_agent_stage(1, NULL, NULL, NULL, 0, &proposal) != PIXAURA_DOCUMENT_RESOURCE_LIMIT) return 3;
    if (pixaura_agent_tools_registry(1, 2, NULL, 0, NULL) != PIXAURA_UNSUPPORTED_ABI) return 6;
    if (sizeof(pixaura_reference_planner_request) != 32) return 7;
    if (pixaura_reference_planner_plan(2, NULL, NULL, NULL, NULL, 0, NULL, 0, NULL) != PIXAURA_UNSUPPORTED_ABI) return 8;
    puts("Independent agent C ABI v1 PASS");
    return 0;
}
