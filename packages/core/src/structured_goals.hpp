#ifndef PIXAURA_STRUCTURED_GOALS_HPP
#define PIXAURA_STRUCTURED_GOALS_HPP
#include <array>
#include <string_view>
namespace pixaura::structured {
struct Goal { std::string_view id, tool; };
// Identity mapping ONLY. Parameters stay authoritative in document/manual tools.
inline constexpr std::array<Goal,11> goals{{
    {"adjust.brightness","pixaura.brightness"},
    {"adjust.contrast","pixaura.contrast"},
    {"adjust.exposure","pixaura.exposure"},
    {"adjust.highlights","pixaura.highlights"},
    {"adjust.saturation","pixaura.saturation"},
    {"adjust.shadows","pixaura.shadows"},
    {"adjust.temperature","pixaura.temperature"},
    {"detail.blur","pixaura.blur"},
    {"detail.sharpen","pixaura.sharpen"},
    {"transform.crop_normalized","pixaura.crop"},
    {"transform.rotate_quarter","pixaura.rotate"}}};
}
#endif
