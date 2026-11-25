#ifndef PIXAURA_RECEIPT_KEY_V2_HPP
#define PIXAURA_RECEIPT_KEY_V2_HPP
#include "../../publication_v2.hpp"
#include <array>
namespace pixaura::commerce {
// Native key owner. Caller obtains 32 bytes from app-owned protected OS storage;
// never a product/signing identity, UI paid flag, source asset or public digest.
class ReceiptKeyV2 final:public storage::ReceiptAuthorityV2 {
    std::array<unsigned char,32> key_{};
public:
    ReceiptKeyV2(const uint8_t*,std::size_t);
    ~ReceiptKeyV2();
    ReceiptKeyV2(const ReceiptKeyV2&)=delete;
    ReceiptKeyV2& operator=(const ReceiptKeyV2&)=delete;
    document::String authenticate(std::string_view)override;
};
}
#endif
