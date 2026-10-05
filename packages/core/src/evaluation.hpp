#ifndef PIXAURA_EVALUATION_HPP
#define PIXAURA_EVALUATION_HPP
#include "working.hpp"
#include "pixaura/evaluation.h"
#include <atomic>
namespace pixaura::evaluation {
using Stack = document::Vector<document::EditOperation>;
Stack parse(std::string_view);
void validate(const Stack&);
double gain(int32_t milli_ev);
// Worst-case retained candidate capacity plus reusable permutation bitmap.
uint64_t reservation(uint32_t width, uint32_t height, const Stack&, const pixaura_working_limits&);
struct Cancellation { std::atomic<bool> requested{false}; };
enum class Checkpoint { admission, allocation, tile, publication };
// Optional internal observer supports deterministic checkpoint fault tests.
using Observer = void (*)(Checkpoint, void*);
void checkpoint(const Cancellation*, Checkpoint, Observer = nullptr, void* = nullptr);
working::Image evaluate(const working::Image&, const Stack&, const pixaura_working_limits&,
    const Cancellation* = nullptr, Observer = nullptr, void* = nullptr);
}
#endif
