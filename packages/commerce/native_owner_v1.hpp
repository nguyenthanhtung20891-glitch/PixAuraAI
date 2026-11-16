#ifndef PIXAURA_PRIVATE_COMMERCIAL_OWNER_V1_HPP
#define PIXAURA_PRIVATE_COMMERCIAL_OWNER_V1_HPP
#include "quota.hpp"
#include "../integration/revision_projection_v2.hpp"
#include "../core/src/storage.hpp"
namespace pixaura::commerce {
// Trusted platform OS sampler, not UI-provided timestamps or paid evidence.
struct NativeClockSourceV1 {virtual ~NativeClockSourceV1()=default;virtual Clock sample()=0;};
struct NativeSourceReceiptV1 {
    std::string source,document,revision;uint64_t encoded_bytes=0,epoch=0;
    // Data receipt, never an editing/approval capability.
};
enum class NativeCancelV1 {requested,already_delivered,not_active};
struct CommercialDenialV1 {Admission::Decision reason;};
struct AdvisoryV1 {std::string token,plan;NativeSourceReceiptV1 receipt;};
// Application-owned private coordinator. Synchronous callers must quiesce before
// destruction; source/budget outlive this owner. No public C/JNI/Swift ABI yet.
// No committed-edit acknowledgment until an atomic native timestamp is available.
class NativeCommercialOwner final {
    friend struct NativeCommercialOwnerTestAccess;
#ifdef PIXAURA_V18_TESTING
    static void (*checkpoint_)(unsigned);
    static void checkpoint(unsigned point){if(checkpoint_)checkpoint_(point);}
#else
    static void checkpoint(unsigned){}
#endif
    friend class VerifiedPlatformStoreBridgeV1;
    Ledger ledger_;storage::Repository repository_;
    const integration::OriginalReaderV2& source_;tiled::Budget& budget_;NativeClockSourceV1& clock_;
    std::string context_,project_root_;std::mutex gate_,completion_gate_;
    evaluation::Cancellation cancellation_;
    std::string active_request_,last_delivered_;bool running_=false;
    void begin_request(const std::string&);
    void retire_request();
    void completed_request(const std::string&);
    storage::Loaded current();
    document::ReferencePlan plan(const storage::Loaded&,const std::string&,const std::string&,const evaluation::Cancellation* =nullptr);
    AdvisoryV1 restore_locked(const std::string&,const storage::Loaded&);
    bool terminal_request(const std::string&);
    static NativeSourceReceiptV1 receipt(const storage::Loaded&);
    void verified_store_fact(const std::string&,const Entitlement&);
public:
    NativeCommercialOwner(const std::string& ledger_path,const std::string& project_root,
        const integration::OriginalReaderV2&,tiled::Budget&,NativeClockSourceV1&,std::string native_context);
    Admission admit_editable_source(const std::string& native_request);
    AdvisoryV1 deliver_advisory(const std::string& native_request,const std::string& goal,
        const std::string& parameter_json);
    AdvisoryV1 restore_advisory(const std::string& token);
    NativeCancelV1 cancel_advisory(const std::string& native_request);
    void dismiss_advisory(const std::string& token);
    State state();
    static constexpr bool recovery_allowed()noexcept{return true;}
};
}
#endif
