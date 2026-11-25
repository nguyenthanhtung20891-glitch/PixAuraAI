#include "original_reader_v2.hpp"
#include "decoded_rows_v2.hpp"
#include "stream_codec_v2.h"
#include "../../srgb_table.hpp"
#include "../../storage_files.hpp"
#include <cstring>
namespace pixaura::integration {
namespace {
// The frozen decoder already accepts a scratch cap <= its default maximum.
// Reserve that cap from the SAME aggregate owner before invoking it. Keeping
// successful binary-sized leases avoids a release/reacquire race and never
// exceeds either the original codec maximum or the current shared budget.
struct CodecScratchV2 {
    std::vector<std::unique_ptr<tiled::Budget::Lease>> leases;uint64_t bytes=0;
    CodecScratchV2(tiled::Budget& budget,uint64_t maximum){
        leases.reserve(32);uint64_t bit=1;while(bit<=maximum/2)bit*=2;
        for(;bit;bit/=2){
            if(bit>maximum-bytes)continue;
            try{auto lease=std::unique_ptr<tiled::Budget::Lease>(new tiled::Budget::Lease(budget.reserve(bit)));leases.push_back(std::move(lease));bytes+=bit;}
            catch(const document::Failure& error){if(error.code!=8)throw;}
        }
        if(!bytes)throw document::Failure{8};
    }
};
}
OriginalReaderV2::OriginalReaderV2()=default;
OriginalReaderV2::~OriginalReaderV2()=default;
OriginalReaderV2::OriginalReaderV2(const uint8_t* bytes,std::size_t count,tiled::Budget& budget,const evaluation::Cancellation* cancel){
    if(cancel&&((cancel->generation==nullptr)!=(cancel->revoked==nullptr)))throw document::Failure{1};
    evaluation::checkpoint(cancel,evaluation::Checkpoint::admission);
    const auto limits=decode::defaults();
    if(!bytes||!count||count>limits.encoded_bytes)throw document::Failure{8};
    // Reserve before copying/parsing; metadata/profile bounds come from frozen decode.
    encoded_lease_.reset(new tiled::Budget::Lease(budget.reserve(count+2*limits.metadata_bytes+65536)));
    auto encoded=std::make_unique<decode::Vector<uint8_t>>(count);
    if(encoded->capacity()>count)throw document::Failure{8};
    std::memcpy(encoded->data(),bytes,count);
    decode_owned(std::move(encoded),budget,cancel,false);
}
std::unique_ptr<OriginalReaderV2> OriginalReaderV2::from_verified(storage::Files& files,const storage::Asset& asset,tiled::Budget& budget,const evaluation::Cancellation* cancel){
    evaluation::checkpoint(cancel,evaluation::Checkpoint::admission);const auto limits=decode::defaults();
    if(!asset.bytes||asset.bytes>limits.encoded_bytes)throw document::Failure{8};
    auto result=std::unique_ptr<OriginalReaderV2>(new OriginalReaderV2());
    result->encoded_lease_.reset(new tiled::Budget::Lease(budget.reserve(asset.bytes+2*limits.metadata_bytes+65536)));
    auto encoded=files.read_verified(asset,limits.encoded_bytes);
    if(encoded->size()!=asset.bytes||encoded->capacity()>asset.bytes)throw document::Failure{8};
    const auto path=files.database_path();const auto root=std::filesystem::u8path(path.begin(),path.end()).parent_path().u8string();
    result->decode_owned(std::move(encoded),budget,cancel,true,root);return result;
}
void OriginalReaderV2::decode_owned(std::unique_ptr<decode::Vector<uint8_t>> encoded,tiled::Budget& budget,const evaluation::Cancellation* cancel,bool owned_source_admission,const std::string& scratch_root){
    if(cancel&&((cancel->generation==nullptr)!=(cancel->revoked==nullptr)))throw document::Failure{1};
    evaluation::checkpoint(cancel,evaluation::Checkpoint::admission);const auto limits=decode::defaults();const auto count=encoded->size();
    decode::Metadata metadata;
    if(owned_source_admission){
        CodecScratchV2 metadata_scratch(budget,limits.scratch_bytes);auto metadata_limits=limits;metadata_limits.scratch_bytes=metadata_scratch.bytes;
        metadata=decode::admit(encoded->data(),encoded->size(),metadata_limits);
    }else{auto metadata_scratch=budget.reserve(limits.scratch_bytes);metadata=decode::admit(encoded->data(),encoded->size(),limits);}
    if(metadata.value.profile_type==1||!metadata.srgb_compatible)throw document::Failure{16};
    storage::Sha256 hash;hash.update(encoded->data(),encoded->size());digest_=hash.finish();
    source_=std::make_unique<decode::Source>(std::move(encoded),std::move(metadata));
    try{
        decoded_lease_.reset(new tiled::Budget::Lease(budget.reserve(source_->metadata().value.decoded_bytes+65536)));
        // The legacy byte-reader retains its original fixed-scratch admission.
        std::unique_ptr<tiled::Budget::Lease> fixed_scratch;
        std::unique_ptr<CodecScratchV2> owned_scratch;auto decoding_limits=limits;
        if(owned_source_admission){owned_scratch.reset(new CodecScratchV2(budget,limits.scratch_bytes));decoding_limits.scratch_bytes=owned_scratch->bytes;}
        else fixed_scratch.reset(new tiled::Budget::Lease(budget.reserve(limits.scratch_bytes)));
        decode::validate_limits(decoding_limits);decode_scratch_bytes_=decoding_limits.scratch_bytes;
        decoded_=decode::execute(*source_,decoding_limits);
        if(decoded_->capacity()>source_->metadata().value.decoded_bytes)throw document::Failure{8};
    }catch(const document::Failure& error){
        if(error.code!=8||!owned_source_admission||scratch_root.empty())throw;
        decoded_.reset();decoded_lease_.reset();
        // Preserve all source bytes/precision under the existing LOW cap. This
        // is anonymous, bounded scratch, not a persistent image cache/revision.
        decoded_lease_.reset(new tiled::Budget::Lease(budget.reserve(65536)));
        const auto& m=source_->metadata().value;
        row_store_.reset(new DecodedRowsV2(scratch_root,m.width,m.height,budget));
        CodecScratchV2 row_scratch(budget,limits.scratch_bytes);decode_scratch_bytes_=row_scratch.bytes;
        struct Target {
            DecodedRowsV2& rows;const evaluation::Cancellation* cancellation;
            static int32_t exchange(void* raw,uint32_t y,uint8_t* data,size_t bytes,int write)noexcept{
                auto& target=*static_cast<Target*>(raw);
                try{evaluation::checkpoint(target.cancellation,evaluation::Checkpoint::tile);target.rows.row(y,data,bytes,write!=0);return 0;}
                catch(const document::Failure& e){return e.code;}catch(const std::bad_alloc&){return 8;}catch(...){return 12;}
            }
        } target{*row_store_,cancel};
        const auto code=pixaura_private_stream_decode_v2(m.format,source_->encoded().data(),source_->encoded().size(),m.width,m.height,
            static_cast<size_t>(row_scratch.bytes),Target::exchange,&target);
        if(code)throw document::Failure{code};
        row_store_->seal();
    }
    evaluation::checkpoint(cancel,evaluation::Checkpoint::publication);
    // Managed immutable encoded bytes remain in the native asset store. Once
    // decoding/provenance validation completes, the bounded reader retains only
    // RGBA8 + admitted metadata, never a redundant encoded copy for every edit.
    metadata_=source_->metadata();encoded_bytes_=count;
    source_.reset();encoded_lease_.reset();
}
void OriginalReaderV2::read(uint32_t x,uint32_t y,float* out)const{
    if(!out||x>=width()||y>=height())throw document::Failure{7};
    const auto& m=metadata().value;uint32_t sx=x,sy=y;
    switch(m.orientation){
    case 2:sx=m.width-1-x;break;
    case 3:sx=m.width-1-x;sy=m.height-1-y;break;
    case 4:sy=m.height-1-y;break;
    case 5:sx=y;sy=x;break;
    case 6:sx=y;sy=m.height-1-x;break;
    case 7:sx=m.width-1-y;sy=m.height-1-x;break;
    case 8:sx=m.width-1-y;sy=x;break;
    default:break;
    }
    const auto at=(std::size_t(sy)*m.width+sx)*4;uint8_t row_pixel[4];
    const uint8_t* pixel=nullptr;if(row_store_){row_store_->pixel(sx,sy,row_pixel);pixel=row_pixel;}else pixel=decoded_->data()+at;
    const float alpha=working::alpha_table[pixel[3]];
    for(unsigned c=0;c<3;++c)out[c]=working::srgb_table[pixel[c]]*alpha;
    out[3]=alpha;
}
}
