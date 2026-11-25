#include "receipt_key_v2.hpp"
#include "sodium.h"
#include <algorithm>
namespace pixaura::commerce {
ReceiptKeyV2::ReceiptKeyV2(const uint8_t* bytes,std::size_t size){
    if(!bytes||size!=key_.size()||std::all_of(bytes,bytes+size,[](uint8_t b){return !b;}))throw document::Failure{1};
    if(sodium_init()<0)throw document::Failure{12};
    std::copy_n(bytes,size,key_.begin());
}
ReceiptKeyV2::~ReceiptKeyV2(){sodium_memzero(key_.data(),key_.size());}
document::String ReceiptKeyV2::authenticate(std::string_view canonical){
    if(canonical.empty()||canonical.size()>4096)throw document::Failure{8};
    std::array<unsigned char,32> mac{};
    if(crypto_generichash(mac.data(),mac.size(),reinterpret_cast<const unsigned char*>(canonical.data()),canonical.size(),key_.data(),key_.size()))throw document::Failure{12};
    static constexpr char hex[]="0123456789abcdef";document::String result;result.reserve(64);
    for(auto byte:mac){result+=hex[byte>>4];result+=hex[byte&15];}sodium_memzero(mac.data(),mac.size());return result;
}
}
