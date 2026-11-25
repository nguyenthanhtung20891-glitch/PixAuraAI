#ifndef PIXAURA_RESOURCE_ADMISSION_V2_HPP
#define PIXAURA_RESOURCE_ADMISSION_V2_HPP
#include "../../tiled_cpu.hpp"
#include <memory>
#include <mutex>
#include <algorithm>
#include <cstdint>

namespace pixaura::editor {
// Versioned admission around the frozen Budget, never a mutation of that Budget
// or a process-RSS limit. Withheld capacity is bookkeeping, NOT allocated RAM.
// Already-admitted work finishes within its previous hard bound. A changed cap
// blocks NEW allocating jobs until active jobs quiesce and the hold is balanced.
class ResourceAdmissionV2 final:public std::enable_shared_from_this<ResourceAdmissionV2> {
    static void need(bool value,int code){if(!value)throw document::Failure{code};}
    std::shared_ptr<tiled::Budget> budget_;
    const uint64_t maximum_;
    std::mutex gate_;
    std::unique_ptr<tiled::Budget::Lease> hold_;
    uint64_t target_,withheld_=0,active_=0;
    bool pending_=true,closed_=false;
    void balance(){
        need(active_==0,15);
        hold_.reset();withheld_=0;
        const auto used=budget_->inflight();need(used<=maximum_,8);
        const auto amount=std::min(maximum_-target_,maximum_-used);
        if(amount){hold_.reset(new tiled::Budget::Lease(budget_->reserve(amount)));withheld_=amount;}
        pending_=false;
    }
public:
    struct Observation{uint64_t maximum,target,allocated_reservation,withheld_capacity,active_jobs;bool pending,closed;};
    class Work final {
        friend class ResourceAdmissionV2;
        std::shared_ptr<ResourceAdmissionV2> owner_;
        bool admitted_=false;
    public:
        explicit Work(std::shared_ptr<ResourceAdmissionV2> owner):owner_(std::move(owner)){}
        Work(const Work&)=delete;Work& operator=(const Work&)=delete;
        void admit(){
            std::lock_guard<std::mutex> lock(owner_->gate_);
            if(admitted_)return;
            need(!owner_->closed_,9);
            if(!owner_->active_)owner_->balance();
            need(!owner_->pending_,8);
            const auto used=owner_->budget_->inflight();need(used>=owner_->withheld_,8);
            need(used-owner_->withheld_<=owner_->target_&&owner_->active_!=UINT64_MAX,8);
            ++owner_->active_;admitted_=true;
        }
        ~Work(){
            if(!admitted_)return;
            std::lock_guard<std::mutex> lock(owner_->gate_);
            --owner_->active_;
            if(!owner_->active_&&owner_->pending_){try{owner_->balance();}catch(...){owner_->pending_=true;}}
        }
    };
    ResourceAdmissionV2(std::shared_ptr<tiled::Budget> budget,uint64_t maximum,uint64_t target):budget_(std::move(budget)),maximum_(maximum),target_(target){
        need(budget_&&maximum&&maximum<=134217728&&target<=maximum,1);balance();
    }
    std::shared_ptr<Work> work(){return std::shared_ptr<Work>(new Work(shared_from_this()));}
    void update(uint64_t target){
        std::lock_guard<std::mutex> lock(gate_);need(!closed_,9);need(target<=maximum_,1);
        if(target==target_)return; // A fresh OS sequence with the SAME cap is not pressure.
        target_=target;pending_=true;if(!active_)balance();
    }
    void close(){std::lock_guard<std::mutex> lock(gate_);need(active_==0,15);closed_=true;hold_.reset();withheld_=0;pending_=false;}
    Observation observation(){
        std::lock_guard<std::mutex> lock(gate_);const auto used=budget_->inflight();need(used>=withheld_,8);
        return {maximum_,target_,used-withheld_,withheld_,active_,pending_,closed_};
    }
};
}
#endif
