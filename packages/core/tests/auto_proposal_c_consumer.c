#include "pixaura/auto_proposal.h"
#include <stddef.h>
_Static_assert(sizeof(pixaura_auto_request)==16,"request ABI");
_Static_assert(sizeof(pixaura_auto_action)==24,"action ABI");
_Static_assert(sizeof(pixaura_auto_proposal)==328,"proposal ABI");
_Static_assert(offsetof(pixaura_auto_proposal,actions)==136,"actions ABI");
int main(void){pixaura_auto_request r={1,sizeof(r),0,2};pixaura_auto_proposal p={0};
 if(pixaura_editor_auto_request(2,NULL,&r,NULL,0,&p)!=2)return 1;
 if(pixaura_editor_auto_cancel(2,NULL)!=2)return 2;
 return p.api_version!=0;
}
