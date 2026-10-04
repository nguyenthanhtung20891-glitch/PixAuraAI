#ifndef PIXAURA_WORKING_HPP
#define PIXAURA_WORKING_HPP
#include "decode.hpp"
#include "pixaura/working.h"
namespace pixaura::working {
pixaura_working_limits defaults();
void validate(const pixaura_working_limits&);
pixaura_working_metadata layout(const decode::Metadata&, const pixaura_working_limits&);
struct Image {
    pixaura_working_metadata metadata{};
    std::unique_ptr<decode::Vector<float>> pixels;
};
Image normalize(const decode::Metadata&, const uint8_t*, std::size_t, const pixaura_working_limits&);
Image identity(const Image&, const pixaura_working_limits&);
}
#endif
