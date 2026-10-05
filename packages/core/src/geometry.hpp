#ifndef PIXAURA_GEOMETRY_HPP
#define PIXAURA_GEOMETRY_HPP
#include "evaluation.hpp"
#include "pixaura/geometry.h"
namespace pixaura::geometry {
struct Extent { uint32_t width, height; };
struct Rect { uint32_t x0, y0, x1, y1; };
struct Point { uint32_t x, y; };
struct Stage { Extent input, output; Rect region; uint32_t turns; };
struct Plan { pixaura_geometry_plan summary{}; document::Vector<Stage> stages; };
uint64_t admit(Extent, const pixaura_working_limits&);
Rect crop(Extent, document::Crop);
Extent rotated(Extent, uint32_t);
Point forward(Extent, uint32_t, Point);
Point inverse(Extent, uint32_t, Point);
uint32_t tile_count(Extent);
Rect tile(Extent, uint32_t);
std::size_t pixel_offset(Extent, Point);
Plan plan(Extent, const evaluation::Stack&, const pixaura_working_limits&);
}
#endif
