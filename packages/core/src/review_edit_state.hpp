#ifndef PIXAURA_REVIEW_EDIT_STATE_HPP
#define PIXAURA_REVIEW_EDIT_STATE_HPP
#include "pixaura/review_edit.h"
namespace pixaura {
struct ReviewEditState {
    document::String original;
    pixaura_review_edit_result result{};
    uint64_t sequence=0,auto_sequence=0;
    bool active=false;
    void abandon() noexcept { active=false;original.clear();result={}; }
};
}
#endif
