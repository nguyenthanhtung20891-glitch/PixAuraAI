#include "pixaura/assist.h"
#include <stddef.h>
_Static_assert(sizeof(pixaura_assist_advice)==88,"advice ABI");
_Static_assert(sizeof(pixaura_assist_request)==16,"request ABI");
_Static_assert(offsetof(pixaura_assist_advice,values)==24,"values ABI");
_Static_assert(offsetof(pixaura_assist_advice,request_serial)==72,"serial ABI");
int main(void){pixaura_assist_advice value={0};unsigned index;
 for(index=0;index<PIXAURA_ASSIST_CHOICES;++index)if(pixaura_assist_choice(1,index,&value)!=0||value.struct_size!=88||value.request_serial!=0)return 1;
 return pixaura_assist_choice(1,11,&value)==5?0:2;
}
