#ifndef PIXAURA_TILED_CPU_HPP
#define PIXAURA_TILED_CPU_HPP
#include "evaluation.hpp"
#include <functional>
#include <mutex>
// Experimental synchronous normalized reader/staged sink. Not wired to document,
// import/export, GPU, approval or public ABI. Callbacks must account retained memory.
namespace pixaura::tiled {
struct Sink {
    // Borrowed pixels are valid only during this synchronous callback.
    virtual void write(uint32_t x,uint32_t y,uint32_t w,uint32_t h,
        const float* pixels,uint32_t stride_pixels)=0;
    virtual void finish()=0;
    virtual void abort() noexcept=0;
    virtual ~Sink()=default;
};
using Reader=std::function<void(uint32_t,uint32_t,float*)>;
struct Metrics {uint64_t peak_reserved=0,tiles=0;};
class Budget {
    std::mutex mutex_;uint64_t cap_,retained_,inflight_=0,peak_=0;
    void release(uint64_t) noexcept;
public:
    class Lease {
        Budget* owner_;uint64_t bytes_;
        friend class Budget;
        Lease(Budget* owner,uint64_t bytes):owner_(owner),bytes_(bytes){}
    public:
        Lease(const Lease&)=delete;Lease& operator=(const Lease&)=delete;
        ~Lease(){owner_->release(bytes_);}
    };
    Budget(uint64_t cap,uint64_t retained);
    Lease reserve(uint64_t);
    uint64_t peak();
    uint64_t inflight();
};
Metrics evaluate(uint32_t width,uint32_t height,const evaluation::Stack&,
    Budget&,const Reader&,Sink&,
    const evaluation::Cancellation* cancel=nullptr);
}
#endif
