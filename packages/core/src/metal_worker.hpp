#ifndef PIXAURA_METAL_WORKER_HPP
#define PIXAURA_METAL_WORKER_HPP
#include <cstdint>
#include <memory>
namespace pixaura::apple {
struct MetalTiming {uint64_t upload_ns=0,dispatch_ns=0,readback_ns=0;};
// Private hardware interface. No model parsing, normalization or publication.
struct MetalWorker {
    virtual ~MetalWorker()=default;
    virtual int32_t dispatch(const float*,float*,uint32_t,const uint32_t*,uint32_t,MetalTiming&)noexcept=0;
};
__attribute__((visibility("hidden"))) bool metal_available()noexcept;
__attribute__((visibility("hidden"))) std::unique_ptr<MetalWorker> make_metal_worker(bool reference);
}
#endif
