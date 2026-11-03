#include "pixaura/result_review.h"
#include <stddef.h>
_Static_assert(sizeof(pixaura_result_review)==152,"result review size");
_Static_assert(offsetof(pixaura_result_review,document_id)==16,"document identity");
_Static_assert(offsetof(pixaura_result_review,previous_revision)==48,"previous revision");
_Static_assert(offsetof(pixaura_result_review,result_revision)==80,"result revision");
_Static_assert(offsetof(pixaura_result_review,live_revision)==112,"live revision");
_Static_assert(offsetof(pixaura_result_review,revision_count)==144,"revision count");
int main(void){return pixaura_result_review_query(2,0,0,0)==2&&pixaura_result_review_render(2,0,0,0,0,0,0,0,0,0)==2?0:1;}
