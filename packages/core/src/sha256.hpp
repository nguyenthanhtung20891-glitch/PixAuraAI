#ifndef PIXAURA_SHA256_HPP
#define PIXAURA_SHA256_HPP
#include "document_containers.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
namespace pixaura::storage {
// FIPS 180-4 SHA-256, content identity only; not encryption/authentication.
class Sha256 {
    std::array<uint32_t, 8> state_{{0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19}};
    std::array<uint8_t, 64> block_{};
    uint64_t bytes_ = 0;
    std::size_t used_ = 0;
    void compress();
public:
    void update(const uint8_t*, std::size_t);
    document::String finish();
};
}
#endif
