#ifndef PIXAURA_DETAIL_HPP
#define PIXAURA_DETAIL_HPP
#include <string_view>
#include <cstdint>
namespace pixaura::detail {
struct Spec {std::string_view type,parameter,unit;int32_t low,high,neutral;};
inline constexpr Spec specs[]={
    {"pixaura.blur","milli_strength","thousandths_fixed_kernel_blend",0,1000,0},
    {"pixaura.sharpen","milli_amount","thousandths_fixed_kernel_unsharp",0,1000,0}};
inline const Spec* find(std::string_view type){for(const auto& spec:specs)if(spec.type==type)return &spec;return nullptr;}
}
#endif
