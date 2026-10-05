#ifndef PIXAURA_EVALUATION_HPP
#define PIXAURA_EVALUATION_HPP
#include "working.hpp"
#include "pixaura/evaluation.h"
namespace pixaura::evaluation {
using Stack = document::Vector<document::EditOperation>;
Stack parse(std::string_view);
void validate(const Stack&);
double gain(int32_t milli_ev);
working::Image evaluate(const working::Image&, const Stack&, const pixaura_working_limits&);
}
#endif
