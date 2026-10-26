#ifndef PIXAURA_INTELLIGENCE_RESOURCES_HPP
#define PIXAURA_INTELLIGENCE_RESOURCES_HPP
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <mutex>
#include <new>

// Internal additive mechanism, unconnected to editors/providers. Application
// construction is trusted. No production device policy is supplied in round 1.
namespace pixaura::intelligence {
enum class Status { success, unavailable, busy, cancelled, stale, resource, failed };
enum class Phase { unloaded, loading, ready, running, stopped, failed };
struct Costs {
    uint64_t mapping=0, session=0, tokenizer=0, cache=0;
    uint64_t preprocessing=0, scratch=0, output=0, device=0;
};
inline bool sum(const Costs& c,uint64_t& total) noexcept {
    total=0;
    for(auto n:{c.mapping,c.session,c.tokenizer,c.cache,c.preprocessing,c.scratch,c.output,c.device}) {
        if(n>UINT64_MAX-total)return false;
        total+=n;
    }
    return true;
}
struct Observation {
    uint64_t available=0, manual_reserve=0, platform_reserve=0, sequence=0;
    bool known=false, pressure=false, foreground=false;
    uint64_t process_rss=0; // Evidence only; never used as a lease.
};
inline bool fits(const Costs& c,const Observation& o) noexcept {
    uint64_t total=0;
    return o.known&&o.foreground&&!o.pressure&&o.sequence&&o.manual_reserve&&o.platform_reserve&&
        sum(c,total)&&total&&o.manual_reserve<=o.available&&o.platform_reserve<=o.available-o.manual_reserve&&
        total<=o.available-o.manual_reserve-o.platform_reserve;
}
struct Client {
    std::atomic<uint64_t> generation{1};
    void abandon() noexcept {auto n=generation.load();for(;;){
        if(n==UINT64_MAX)return;
        if(generation.compare_exchange_weak(n,n+1))return;
    }}
};
// Trusted future runtime adapter owns all memory in its declared upper bound.
// run is synchronous, cooperatively polls cancellation and has no edit interfaces.
class Runtime {
public:
    virtual ~Runtime()=default;
    virtual void load()=0;
    virtual void run(const std::atomic<bool>& cancelled)=0;
};
using Factory=std::unique_ptr<Runtime>(*)();
class Owner final {
    std::mutex mutex_;
    std::condition_variable idle_;
    std::unique_ptr<Runtime> runtime_;
    std::atomic<bool> cancellation_{false};
    Phase phase_=Phase::unloaded;
    bool active_=false, stopped_=false, shutdown_=false;
    uint64_t epoch_=1, observation_sequence_=0;
    Observation observation_{};
    Costs active_costs_{};
    Costs resident_{};
    Factory factory_=nullptr;
    Owner()=default;
    Status observe_locked(const Observation& value){
        if(!value.sequence)return Status::resource;
        if(value.sequence<observation_sequence_)return Status::stale;
        if(value.sequence==observation_sequence_&&(value.available!=observation_.available||
            value.manual_reserve!=observation_.manual_reserve||value.platform_reserve!=observation_.platform_reserve||
            value.known!=observation_.known||value.pressure!=observation_.pressure||value.foreground!=observation_.foreground))return Status::stale;
        observation_=value;observation_sequence_=value.sequence;
        if(active_&&!fits(active_costs_,value))cancellation_=true;
        return Status::success; // observation acceptance is not request admission
    }
public:
    Owner(const Owner&)=delete;
    Owner& operator=(const Owner&)=delete;
    static Owner& shared(); // Defined once in the shared native implementation.
    Phase phase(){std::lock_guard<std::mutex> lock(mutex_);return phase_;}
    Status observe(const Observation& value){std::lock_guard<std::mutex> lock(mutex_);return observe_locked(value);}
    void cancel(){std::lock_guard<std::mutex> lock(mutex_);cancellation_=true;}
    void stop(){std::lock_guard<std::mutex> lock(mutex_);stopped_=true;cancellation_=true;
        if(!active_)phase_=Phase::stopped;}
    void unload(){
        stop();std::unique_lock<std::mutex> lock(mutex_);
        idle_.wait(lock,[&]{return !active_;});runtime_.reset();phase_=shutdown_?Phase::stopped:Phase::unloaded;
        if(!shutdown_&&epoch_!=UINT64_MAX){++epoch_;stopped_=false;}
    }
    void shutdown(){
        {std::lock_guard<std::mutex> lock(mutex_);shutdown_=true;}
        unload(); // permanent process-owner teardown; later unload cannot revive it
    }
    Status request(Client& client,uint64_t generation,const Costs& costs,const Observation& observation,Factory factory){
        std::unique_lock<std::mutex> lock(mutex_);
        if(stopped_||shutdown_)return Status::unavailable;
        const auto observed=observe_locked(observation);if(observed!=Status::success)return observed;
        if(active_)return Status::busy;
        if(!factory||!generation||generation==UINT64_MAX||client.generation!=generation)return Status::stale;
        if(runtime_&&(factory!=factory_||costs.mapping!=resident_.mapping||costs.session!=resident_.session||
            costs.tokenizer!=resident_.tokenizer||costs.cache!=resident_.cache||costs.device!=resident_.device))return Status::resource;
        if(!fits(costs,observation))return Status::resource;
        active_costs_=costs;
        active_=true;cancellation_=false;
        const auto epoch=epoch_;phase_=runtime_?Phase::running:Phase::loading;
        lock.unlock();
        Status status=Status::success;
        try {
            if(!runtime_){auto next=factory();if(!next)throw 0;next->load();runtime_=std::move(next);resident_=costs;factory_=factory;}
            {std::lock_guard<std::mutex> stage(mutex_);phase_=Phase::running;}
            if(!cancellation_)runtime_->run(cancellation_);
        }catch(const std::bad_alloc&){status=Status::resource;}
        catch(...){status=Status::failed;}
        lock.lock();
        if(stopped_||cancellation_)status=Status::cancelled;
        else if(epoch!=epoch_||client.generation!=generation)status=Status::stale;
        if(status==Status::failed||status==Status::resource)runtime_.reset();
        phase_=stopped_?Phase::stopped:runtime_?Phase::ready:Phase::failed;
        active_=false;idle_.notify_all();return status;
    }
};
}
#endif
