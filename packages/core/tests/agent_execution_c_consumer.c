#include "pixaura/agent_execution.h"
#include <stddef.h>
typedef char approval_size_check[sizeof(pixaura_agent_approval)==56?1:-1];
typedef char approval_offset_check[offsetof(pixaura_agent_approval,serial)==48?1:-1];
typedef char request_size_check[sizeof(pixaura_agent_execution_request)==32?1:-1];
typedef char receipt_size_check[sizeof(pixaura_agent_execution_receipt)==144?1:-1];
int main(void){
    pixaura_agent_execution_request r={1,sizeof(r),1,0,{0,0,0,0}};
    pixaura_agent_execution_receipt out={0};
    if(pixaura_editor_agent_execute(1,NULL,&r,NULL,NULL,0,&out)!=1)return 1;
    if(pixaura_editor_agent_cancel(2,NULL)!=2)return 2;
    return out.api_version!=0;
}
