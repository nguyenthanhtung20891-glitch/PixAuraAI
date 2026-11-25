#ifndef PIXAURA_ORIGINAL_READER_V2_HPP
#define PIXAURA_ORIGINAL_READER_V2_HPP
#include "../../tiled_cpu.hpp"
#include "../../sha256.hpp"
namespace pixaura::storage {class Files;struct Asset;}
namespace pixaura::integration {
class DecodedRowsV2;
// Private C++ v2 owner, never a v1 working/original handle. Budget must outlive it.
// Caller input and all other retained owners must already be included in Budget.
class OriginalReaderV2 final {
    std::unique_ptr<tiled::Budget::Lease> encoded_lease_, decoded_lease_;
    std::unique_ptr<decode::Source> source_;
    std::unique_ptr<decode::Vector<uint8_t>> decoded_;
    std::unique_ptr<DecodedRowsV2> row_store_;
    document::String digest_;
    decode::Metadata metadata_;uint64_t encoded_bytes_=0,decode_scratch_bytes_=0;
    OriginalReaderV2();
    void decode_owned(std::unique_ptr<decode::Vector<uint8_t>>,tiled::Budget&,const evaluation::Cancellation*,bool,const std::string& ={});
public:
    // Sole ownership handoff from the managed immutable source, not a second
    // encoded copy. Reservation precedes Files allocation and lives through decode.
    static std::unique_ptr<OriginalReaderV2> from_verified(storage::Files&,const storage::Asset&,tiled::Budget&,const evaluation::Cancellation* =nullptr);
    OriginalReaderV2(const uint8_t*,std::size_t,tiled::Budget&,const evaluation::Cancellation* =nullptr);
    ~OriginalReaderV2();
    OriginalReaderV2(const OriginalReaderV2&)=delete;
    const decode::Metadata& metadata()const{return metadata_;}
    const document::String& digest()const{return digest_;}
    uint64_t encoded_bytes()const{return encoded_bytes_;}
    uint64_t decode_scratch_bytes()const{return decode_scratch_bytes_;}
    bool uses_row_store()const{return row_store_!=nullptr;}
    uint32_t width()const{return metadata().value.display_width;}
    uint32_t height()const{return metadata().value.display_height;}
    void read(uint32_t,uint32_t,float*)const;
};
}
#endif
