#include "pixaura/review_preview.h"
#include <stddef.h>
_Static_assert(sizeof(pixaura_review_preview_ticket)==56,"ticket ABI");
_Static_assert(sizeof(pixaura_review_preview_pair)==176,"pair ABI");
_Static_assert(offsetof(pixaura_review_preview_pair,before)==56,"before ABI");
_Static_assert(offsetof(pixaura_review_preview_pair,after)==96,"after ABI");
int main(void){return pixaura_review_preview_current(2,NULL,NULL)==2?0:1;}
