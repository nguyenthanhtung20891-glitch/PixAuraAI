#include "pixaura/review_edit.h"
#include <stddef.h>
_Static_assert(sizeof(pixaura_review_edit_ticket)==136,"ticket size");
_Static_assert(sizeof(pixaura_review_edit_request)==776,"request size");
_Static_assert(sizeof(pixaura_review_edit_result)==480,"result size");
_Static_assert(offsetof(pixaura_review_edit_request,ticket)==32,"request ticket");
_Static_assert(offsetof(pixaura_review_edit_request,tool_id)==168,"request tool");
_Static_assert(offsetof(pixaura_review_edit_request,expected_action)==232,"exact action identity");
_Static_assert(offsetof(pixaura_review_edit_result,review)==152,"result review");
int main(void){
    return pixaura_review_edit(2,0,0,0)==2&&pixaura_review_edit_begin(2,0,0,0)==2&&pixaura_review_edit_current(2,0,0,0)==2?0:1;
}
