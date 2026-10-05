#include "evaluation.hpp"
#include "exposure_table.hpp"
#include <cmath>
#include <limits>
#include <cfenv>
#include <cstring>
namespace pixaura::evaluation {
static_assert(sizeof(double)==8 && std::numeric_limits<double>::is_iec559, "IEEE binary64 required");
namespace {
void need(bool ok, int32_t code) { if (!ok) throw document::Failure{code}; }
bool finite(float value) {
    uint32_t bits=0;
    std::memcpy(&bits,&value,sizeof(bits));
    return (bits & 0x7f800000u) != 0x7f800000u;
}
}
double gain(int32_t ev) {
    need(ev >= -5000 && ev <= 5000, 7);
    // Floor quotient, nonnegative remainder: exact power-of-two scale of a
    // frozen binary64 constant, with no runtime transcendental function.
    const int q = ev >= 0 ? ev / 1000 : (ev - 999) / 1000;
    const int r = ev - q * 1000;
    return std::ldexp(exposure_fraction[r], q);
}
void validate(const Stack& stack) {
    need(stack.size() <= PIXAURA_EVALUATION_MAX_OPERATIONS, 8);
    for (std::size_t i = 0; i < stack.size(); ++i) {
        const auto& op = stack[i];
        need(op.type == "pixaura.exposure" && op.operation_version == 1 && op.parameter_version == 1, 5);
        const auto* p = std::get_if<document::Exposure>(&op.parameters);
        need(p && p->milli_ev >= -5000 && p->milli_ev <= 5000, 7);
        for (std::size_t j = 0; j < i; ++j) need(op.id != stack[j].id, 6);
    }
}
Stack parse(std::string_view bytes) {
    auto parsed = document::parse_evaluation(bytes);
    if (parsed.code) throw document::Failure{parsed.code, parsed.index};
    validate(parsed.value);
    return std::move(parsed.value);
}
working::Image evaluate(const working::Image& source, const Stack& stack, const pixaura_working_limits& limits) {
    validate(stack);
    need(std::fegetround() == FE_TONEAREST, 7);
    // identity checks all layout/byte limits and allocates the sole destination.
    // The input remains read-only; any failure destroys this private candidate.
    auto out = working::identity(source, limits);
    auto& pixels = *out.pixels;
    for (std::size_t i = 0; i < pixels.size(); i += 4) {
        const float a = pixels[i + 3];
        need(finite(a) && a >= 0 && a <= 1, 7);
        for (unsigned c = 0; c < 3; ++c) {
            need(finite(pixels[i + c]), 7);
            need(a != 0 || pixels[i + c] == 0, 7);
        }
    }
    for (const auto& op : stack) {
        const auto ev = std::get<document::Exposure>(op.parameters).milli_ev;
        if (ev == 0) continue; // Exact identity, including signed zero/subnormals.
        const double multiplier = gain(ev);
        for (std::size_t i = 0; i < pixels.size(); i += 4) {
            for (unsigned c = 0; c < 3; ++c) {
                const double value = static_cast<double>(pixels[i + c]) * multiplier;
                need(std::isfinite(value) && std::abs(value) <= std::numeric_limits<float>::max(), 7);
                pixels[i + c] = static_cast<float>(value);
            }
        }
    }
    return out;
}
}
