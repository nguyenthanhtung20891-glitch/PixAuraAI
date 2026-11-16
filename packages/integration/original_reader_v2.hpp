#ifndef PIXAURA_ORIGINAL_READER_V2_HPP
#define PIXAURA_ORIGINAL_READER_V2_HPP
#include "../core/src/tiled_cpu.hpp"
#include "../core/src/sha256.hpp"
namespace pixaura::integration {
// Private C++ v2 owner, never a v1 working/original handle. Budget must outlive it.
// Caller input and all other retained owners must already be included in Budget.
class OriginalReaderV2 final {
    std::unique_ptr<tiled::Budget::Lease> encoded_lease_, decoded_lease_;
    std::unique_ptr<decode::Source> source_;
    std::unique_ptr<decode::Vector<uint8_t>> decoded_;
    document::String digest_;
public:
    OriginalReaderV2(const uint8_t*,std::size_t,tiled::Budget&,const evaluation::Cancellation* =nullptr);
    OriginalReaderV2(const OriginalReaderV2&)=delete;
    const decode::Metadata& metadata()const{return source_->metadata();}
    const document::String& digest()const{return digest_;}
    uint64_t encoded_bytes()const{return source_->encoded().size();}
    uint32_t width()const{return metadata().value.display_width;}
    uint32_t height()const{return metadata().value.display_height;}
    void read(uint32_t,uint32_t,float*)const;
};
}
#endif
