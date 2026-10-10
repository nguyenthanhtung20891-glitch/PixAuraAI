#ifndef PIXAURA_TONE_HPP
#define PIXAURA_TONE_HPP
#include <string_view>
#include <cstdint>
namespace pixaura::tone {
struct Spec {std::string_view type, parameter, unit;int32_t low,high,neutral;};
inline constexpr Spec specs[] = {
    {"pixaura.brightness","milli_linear","thousandths_linear_white",-1000,1000,0},
    {"pixaura.contrast","milli_stops","thousandths_log2_contrast_slope",-2000,2000,0},
    {"pixaura.highlights","milli_ev","thousandths_highlight_exposure_stop",-2000,2000,0},
    {"pixaura.saturation","milli_ratio","thousandths_chroma_multiplier",0,2000,1000},
    {"pixaura.shadows","milli_ev","thousandths_shadow_exposure_stop",-2000,2000,0},
    {"pixaura.temperature","kelvin","daylight_white_kelvin",4000,25000,6504}};
inline const Spec* find(std::string_view type){for(const auto& s:specs)if(s.type==type)return &s;return nullptr;}
}
#endif
