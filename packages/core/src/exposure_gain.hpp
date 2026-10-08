#ifndef PIXAURA_EXPOSURE_GAIN_HPP
#define PIXAURA_EXPOSURE_GAIN_HPP
#include "exposure_table.hpp"
#include <cmath>
#include <cstdint>
namespace pixaura::evaluation {
// Both CPU evaluation and GPU reference use the identical frozen recipe.
// Caller has validated the integer [-5000,5000] domain.
inline double exposure_gain(int32_t ev) {
    const int q=ev>=0?ev/1000:(ev-999)/1000;
    return std::ldexp(exposure_fraction[ev-q*1000],q);
}
}
#endif
