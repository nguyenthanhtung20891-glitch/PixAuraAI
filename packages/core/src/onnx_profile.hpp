#ifndef PIXAURA_ONNX_PROFILE_HPP
#define PIXAURA_ONNX_PROFILE_HPP
#include "pixaura/inference.h"
#include <array>
#include <cstddef>
#include <cstdint>
namespace pixaura::inference {
struct Error { int32_t code; };
inline void require(bool b,int32_t code=PIXAURA_INFERENCE_INVALID_ARGUMENT){if(!b)throw Error{code};}
struct Plan { std::array<uint32_t,4> input{},output{};std::array<float,3> weights{}; };
#if defined(__GNUC__) || defined(__clang__)
#define PIXAURA_INFERENCE_PRIVATE __attribute__((visibility("hidden")))
#else
#define PIXAURA_INFERENCE_PRIVATE
#endif
PIXAURA_INFERENCE_PRIVATE Plan parse_onnx(const uint8_t*,size_t);
PIXAURA_INFERENCE_PRIVATE bool valid_probabilities(const float*,size_t);
PIXAURA_INFERENCE_PRIVATE double binary32_value(float);
PIXAURA_INFERENCE_PRIVATE float binary32_round(double);
#undef PIXAURA_INFERENCE_PRIVATE
}
#endif
