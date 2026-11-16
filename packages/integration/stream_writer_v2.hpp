#ifndef PIXAURA_STREAM_WRITER_V2_HPP
#define PIXAURA_STREAM_WRITER_V2_HPP
#include "stage_plan_v2.hpp"
namespace pixaura::integration {
// Trusted native staging capability. Sink-owned buffers must be separately leased
// from the SAME Budget. Callbacks may not accumulate unaccounted output.
// Platform permission/path selection is outside this private codec interface.
struct StagedSinkV2 {
    virtual ~StagedSinkV2()=default;
    virtual void write(const uint8_t*,std::size_t)=0;
    virtual void seal()=0; // Complete staging only. Never revision or public-destination authority.
    virtual void abort()noexcept=0;
};
uint64_t stream_export_v2(const StagePlanV2&,uint32_t format,int quality,tiled::Budget&,
    StagedSinkV2&,const evaluation::Cancellation* =nullptr);
}
#endif
