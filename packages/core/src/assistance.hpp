#ifndef PIXAURA_ASSISTANCE_HPP
#define PIXAURA_ASSISTANCE_HPP
#include "document.hpp"
#include "manual_tools.hpp"
#include <mutex>
#include <limits>

// Internal shared-native interface. No platform ABI, editor, approval or callback.
namespace pixaura::assistance {
constexpr std::size_t envelope_limit=12288, payload_limit=8192, identifier_limit=64;
constexpr uint32_t contract_version=1, all_kinds=7;
constexpr std::string_view reference_id="reference.assistance.v1";
enum class Kind : uint32_t { Advice=1, StructuredIntent=2, PlanCandidate=3 };
// Failure construction must not allocate even with MSVC debug iterator proxies.
template<class T> struct Outcome { int32_t code; std::optional<T> value; };
struct Request {
    uint32_t version=1, registry_version=1;
    Kind kind=Kind::Advice;
    document::String proposal_id, binding, payload;
    bool pre_cancel=false;
};
struct Validated { document::String envelope, plan; Kind kind=Kind::Advice; };
struct Decoded { Validated value; document::Vector<document::EditOperation> operations; };
// Codec receives data only. The provider never receives a document or context.
Outcome<document::String> encode(const Request&,std::string_view provider);
// Trusted caller supplies its live immutable snapshot/context and selected owner ID.
// This is validation, NOT execution admission; Phase 6 rechecks live source serial/CAS.
Outcome<Decoded> decode(std::string_view,const document::ImageDocument&,
    std::string_view context,std::string_view selected_provider,uint32_t registry_version=1);
inline Outcome<Validated> validate(std::string_view bytes,const document::ImageDocument& doc,
    std::string_view context,std::string_view provider,uint32_t registry_version=1) {
    try {
    auto result=decode(bytes,doc,context,provider,registry_version);
    if(result.code)return {result.code,{}};
    if(!result.value)return {14,{}};
    for(const auto& op:result.value->operations) {
        const auto* descriptor=manual::find(op.type,op.operation_version,op.parameter_version);
        if(!descriptor || !manual::agent_exposable(*descriptor))return {5,{}};
    }
    return {0,std::move(result.value->value)};
    } catch(const std::bad_alloc&) { return {8,{}}; }
      catch(...) { return {14,{}}; }
}
struct Identity { std::string_view id; uint32_t version, capabilities; };
inline int32_t check_identity(const Identity& identity) noexcept {
    if(identity.id!=reference_id || identity.capabilities!=all_kinds)return 1;
    return identity.version==1?0:2;
}
class Provider {
public:
    virtual ~Provider()=default;
    virtual Identity identity() const noexcept=0;
    virtual Outcome<document::String> request(const Request&)=0;
    virtual void cancel()=0;
    virtual void stop()=0;
};
// Static known reference implementation. One synchronous request, no worker/queue.
// Owner must stop -> join ALL calls -> destroy. Different instances are isolated.
class ReferenceProvider final : public Provider {
    std::mutex active_, fence_;
    uint64_t epoch_=0;
    bool stopped_=false;
public:
#ifdef PIXAURA_ASSISTANCE_TESTING
    // Independent certification executable only; absent from production layout.
    void (*assistance_checkpoint)(uint32_t,void*)=nullptr;
    void* assistance_checkpoint_state=nullptr;
#endif
    Identity identity() const noexcept override { return {reference_id,1,all_kinds}; }
    Outcome<document::String> request(const Request& input) override {
        try {
        std::unique_lock<std::mutex> active(active_,std::try_to_lock);
        if(!active.owns_lock())return {8,{}};
        uint64_t epoch;
        { std::lock_guard<std::mutex> lock(fence_);
          if(stopped_ || input.pre_cancel)return {13,{}};
          epoch=epoch_; }
#ifdef PIXAURA_ASSISTANCE_TESTING
        if(assistance_checkpoint)assistance_checkpoint(0,assistance_checkpoint_state);
#endif
        auto result=encode(input,reference_id);
#ifdef PIXAURA_ASSISTANCE_TESTING
        if(assistance_checkpoint)assistance_checkpoint(1,assistance_checkpoint_state);
#endif
        std::lock_guard<std::mutex> lock(fence_);
        if(stopped_ || epoch!=epoch_)return {13,{}};
        return result;
        } catch(const std::bad_alloc&) { return {8,{}}; }
          catch(...) { return {14,{}}; }
    }
    void cancel() override {
        std::lock_guard<std::mutex> lock(fence_);
        if(epoch_==std::numeric_limits<uint64_t>::max())stopped_=true;
        else ++epoch_;
    }
    void stop() override { std::lock_guard<std::mutex> lock(fence_);stopped_=true; }
};
}
#endif
