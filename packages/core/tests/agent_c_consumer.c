#include "pixaura/agent.h"
#include <stdio.h>
int main(void) {
    pixaura_agent_proposal proposal = {0};
    if (sizeof(proposal) != 56 || offsetof(pixaura_agent_proposal, serial) != 48) return 1;
    if (pixaura_agent_validate(2, NULL, NULL, NULL) != PIXAURA_UNSUPPORTED_ABI) return 2;
    if (pixaura_agent_stage(1, NULL, NULL, NULL, 0, &proposal) != PIXAURA_DOCUMENT_RESOURCE_LIMIT) return 3;
    puts("Independent agent C ABI v1 PASS");
    return 0;
}
