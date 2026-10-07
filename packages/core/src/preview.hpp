#ifndef PIXAURA_PREVIEW_HPP
#define PIXAURA_PREVIEW_HPP
#include "evaluation.hpp"
#include "pixaura/preview.h"
namespace pixaura::preview {
struct Image {
    pixaura_preview_metadata metadata{};
    std::unique_ptr<decode::Vector<uint8_t>> pixels;
};
pixaura_preview_metadata layout(const working::Image&);
pixaura_preview_metadata layout(const working::Image&, const pixaura_preview_request&);
uint8_t encode(double linear);
uint8_t alpha(double value);
Image render(const working::Image&, const evaluation::Cancellation* = nullptr,
    evaluation::Observer = nullptr, void* = nullptr);
Image render(const working::Image&, const pixaura_preview_request&,
    const evaluation::Cancellation* = nullptr, evaluation::Observer = nullptr, void* = nullptr);
}
#endif
