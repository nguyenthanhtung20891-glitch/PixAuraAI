#ifndef PIXAURA_ASSIST_EXPERIENCE_HPP
#define PIXAURA_ASSIST_EXPERIENCE_HPP
#include "assistance.hpp"
#include "structured_goals.hpp"
#include "pixaura/assist.h"
#include <array>
namespace pixaura::assist_experience {
struct Choice { uint32_t tool_index=0,count=0;std::array<int32_t,4> values{}; };
// Explicit UI choices only, NOT a planner/heuristic or a second parameter schema.
// Amounts are visible before the user's request; native tool validation is final.
inline constexpr std::array<std::array<int32_t,4>,11> amounts{{
    {{100,0,0,0}},{{250,0,0,0}},{{500,0,0,0}},{{-500,0,0,0}},
    {{1200,0,0,0}},{{500,0,0,0}},{{7000,0,0,0}},{{250,0,0,0}},
    {{250,0,0,0}},{{800000,800000,100000,100000}},{{1,0,0,0}}}};
inline Choice choice(uint32_t index) {
    if(index>=structured::goals.size())throw document::Failure{5};
    const auto& tools=manual::registry();
    for(std::size_t i=0;i<tools.size();++i)if(tools[i].tool_id==structured::goals[index].tool){
        if(!manual::agent_exposable(tools[i]) || tools[i].parameter_count>4)throw document::Failure{5};
        return {static_cast<uint32_t>(i),static_cast<uint32_t>(tools[i].parameter_count),amounts[index]};
    }
    throw document::Failure{5};
}
assistance::Outcome<assistance::Request> request(const document::ImageDocument&,std::string_view context,
    uint32_t index,const manual::Descriptor&,const Choice&,uint64_t serial);
assistance::Outcome<Choice> presentation(const assistance::Validated&,const manual::Descriptor*,std::size_t);
}
#endif
