#include "pixaura/agent_execution.h"
#include <stddef.h>
_Static_assert(sizeof(pixaura_agent_approval)==56,"approval ABI");
_Static_assert(offsetof(pixaura_agent_approval,serial)==48,"approval serial ABI");
_Static_assert(sizeof(pixaura_agent_execution_request)==32,"request ABI");
_Static_assert(sizeof(pixaura_agent_execution_receipt)==144,"receipt ABI");
int main(void){
    pixaura_agent_execution_request r={1,sizeof(r),1,0,{0,0,0,0}};
    pixaura_agent_execution_receipt out={0};
    if(pixaura_editor_agent_execute(1,NULL,&r,NULL,NULL,0,&out)!=1)return 1;
    if(pixaura_editor_agent_cancel(2,NULL)!=2)return 2;
    return out.api_version!=0;
}
