#include "pixaura/export.h"
#include <stddef.h>
_Static_assert(sizeof(pixaura_export_request)==72,"export request");
_Static_assert(offsetof(pixaura_export_request,epoch)==32,"epoch");
_Static_assert(offsetof(pixaura_export_request,revision_id)==40,"revision");
int main(void){
    return pixaura_export_bind(2,0,0)==2
        && pixaura_export_encode(2,0,0,0,0,0,0,0,0,0)==2
        && pixaura_export_cancel(0)==3 ? 0 : 1;
}
