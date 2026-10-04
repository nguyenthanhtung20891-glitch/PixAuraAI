#ifndef PIXAURA_DECODE_HPP
#define PIXAURA_DECODE_HPP
#include "pixaura/decode.h"
#include "document.hpp"
#include <memory>
namespace pixaura::decode {
using document::Vector;
struct Metadata {
    pixaura_decode_metadata value{};
    Vector<uint8_t> profile;
};
pixaura_decode_limits defaults();
void validate_limits(const pixaura_decode_limits&);
uint64_t multiply(uint64_t, uint64_t);
void layout(pixaura_decode_metadata&, const pixaura_decode_limits&);
Metadata admit(const uint8_t*, std::size_t, const pixaura_decode_limits&);
// Immutable admission value; only checked factories publish shared const sources.
class Source {
    std::unique_ptr<const Vector<uint8_t>> encoded_;
    Metadata metadata_;
public:
    Source(std::unique_ptr<Vector<uint8_t>> encoded, Metadata&& metadata):encoded_(std::move(encoded)) {
        metadata_.value=metadata.value;metadata_.profile.swap(metadata.profile);
    }
    Source(const Vector<uint8_t>& encoded, Metadata&& metadata):Source(std::make_unique<Vector<uint8_t>>(encoded),std::move(metadata)){}
    const Vector<uint8_t>& encoded() const { return *encoded_; }
    const Metadata& metadata() const { return metadata_; }
};
#ifdef PIXAURA_DECODE_TESTING
extern thread_local int allocation_fail_after;
extern thread_local int execution_fault;
#endif
std::unique_ptr<Vector<uint8_t>> execute(const Source&, const pixaura_decode_limits&);
}
#endif
