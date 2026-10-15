#ifndef PIXAURA_INFERENCE_PARITY_HPP
#define PIXAURA_INFERENCE_PARITY_HPP
#include "onnx_profile.hpp"
#include <cmath>
namespace pixaura::inference {
// Frozen before GPU execution. Quarter weights summing to one imply either
// exact power-of-two products + <=2 inexact adds, or one 0.75 product + one
// inexact add. Allow implementation-defined adjacent rounding plus CPU RN32.
#if defined(__GNUC__) || defined(__clang__)
__attribute__((visibility("hidden")))
#endif
inline bool vulkan_probability_matches(float actual,double reference){
    return valid_probabilities(&actual,1)&&
        std::abs(binary32_value(actual)-reference)<=4e-38+3e-7*std::abs(reference);
}
// Apple-specific derivation: explicit rounded fma product/add chain, safe math;
// allow RTE or RTZ and FTZ. The resulting bound coincides numerically with Vulkan,
// but is checked independently against Metal output and is not imported policy.
#if defined(__GNUC__) || defined(__clang__)
__attribute__((visibility("hidden")))
#endif
inline bool metal_probability_matches(float actual,double reference){
    return valid_probabilities(&actual,1)&&std::abs(binary32_value(actual)-reference)<=4e-38+3e-7*std::abs(reference);
}
}
#endif
