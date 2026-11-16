#ifndef PIXAURA_RUNTIME_BUDGET_V2_HPP
#define PIXAURA_RUNTIME_BUDGET_V2_HPP
#include "../core/include/pixaura/budget.h"
#include "../core/src/tiled_cpu.hpp"
#include <algorithm>
namespace pixaura::integration {
// Trusted native OS profile only, never externally deserialized diagnostics.
// Reuse frozen admission to derive pressure/tier cap; v2 remains <=T0 cap.
// This probe is NOT image admission. Actual immutable-source/tile allocations
// still reserve all payloads from this same owner, before allocation.
class RuntimeBudgetV2 final {
    pixaura_budget_result policy_{};
    std::unique_ptr<tiled::Budget> budget_;
public:
    explicit RuntimeBudgetV2(const pixaura_capability_profile& profile,uint64_t retained=0){
        pixaura_budget_request query{};query.version=1;query.struct_size=sizeof(query);
        query.work_class=PIXAURA_BUDGET_IMPORT;query.bytes_per_pixel=4;query.raster_count=1;
        query.width=query.height=1;query.encoded_bytes=1;query.requested_handles=1;
        query.retained_bytes=retained;query.context_limit_bytes=256ULL<<20;
        const auto code=pixaura_budget_admit(1,&profile,&query,&policy_);
        if(code)throw document::Failure{code};
        if(policy_.decision==PIXAURA_BUDGET_REJECT)throw document::Failure{8};
        budget_=std::make_unique<tiled::Budget>(std::min<uint64_t>(policy_.runtime_cap_bytes,128ULL<<20),retained);
    }
    tiled::Budget& owner(){return *budget_;}
    const pixaura_budget_result& policy()const{return policy_;}
};
}
#endif
