#include "original_reader_v2.hpp"
#include "../core/src/srgb_table.hpp"
#include <cstring>
namespace pixaura::integration {
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
    decode::Metadata metadata;
    {auto metadata_scratch=budget.reserve(limits.scratch_bytes);
        metadata=decode::admit(encoded->data(),encoded->size(),limits);
    }
    if(metadata.value.profile_type==1||!metadata.srgb_compatible)throw document::Failure{16};
    storage::Sha256 hash;hash.update(encoded->data(),encoded->size());digest_=hash.finish();
    decoded_lease_.reset(new tiled::Budget::Lease(budget.reserve(metadata.value.decoded_bytes+65536)));
    source_=std::make_unique<decode::Source>(std::move(encoded),std::move(metadata));
    auto scratch_lease=budget.reserve(limits.scratch_bytes);
    decoded_=decode::execute(*source_,limits);
    if(decoded_->capacity()>source_->metadata().value.decoded_bytes)throw document::Failure{8};
    evaluation::checkpoint(cancel,evaluation::Checkpoint::publication);
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
    const auto at=(std::size_t(sy)*m.width+sx)*4;
    const float alpha=working::alpha_table[(*decoded_)[at+3]];
    for(unsigned c=0;c<3;++c)out[c]=working::srgb_table[(*decoded_)[at+c]]*alpha;
    out[3]=alpha;
}
}
