#include "pixaura/review_inspection.h"
#include <stddef.h>
_Static_assert(sizeof(pixaura_review_parameter)==112,"parameter");
_Static_assert(sizeof(pixaura_review_action)==544,"action");
_Static_assert(sizeof(pixaura_review_inspection)==4808,"inspection");
_Static_assert(offsetof(pixaura_review_inspection,actions)==456,"actions offset");
int main(void){return pixaura_review_inspect(2,0,0,0,0,0)==2?0:1;}
