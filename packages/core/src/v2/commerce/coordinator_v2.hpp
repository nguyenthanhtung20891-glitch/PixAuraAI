#ifndef PIXAURA_COMMERCIAL_COORDINATOR_V2_HPP
#define PIXAURA_COMMERCIAL_COORDINATOR_V2_HPP
#include "quota.hpp"
#include "store_journal_v3.hpp"
#include <vector>
#include "../../storage.hpp"
namespace pixaura::editor {class EditorContextV2;}
namespace pixaura::commerce {
struct CommercialClockV2 {virtual ~CommercialClockV2()=default;virtual Clock sample_os()=0;};
class CommercialCoordinatorV2 final:public storage::PublicationClockV2 {
    friend class pixaura::editor::EditorContextV2;
    friend struct CommercialCoordinatorV2TestAccess;
    Ledger ledger_;CommercialClockV2& clock_;storage::ReceiptAuthorityV2& authority_;std::mutex gate_;
    std::shared_ptr<StoreProofAuthorityV3> store_authority_;
    std::unique_ptr<StoreJournalV3> store_journal_;
    std::string store_nonce_;
    void restore_store_projection();
    void validate_origin();
    bool acknowledge(const storage::NativeCommitReceiptV2&);
    Admission reserve(const std::string&,const std::string&,bool);
    bool finish(const std::string&,bool,const std::string&);
    bool deliver(const storage::NativeAdvisoryDeliveryV2&);
    std::string recover_delivery(const std::string&);
    std::vector<std::string> deliveries();
    void dismiss_delivery(const std::string&);
public:
    CommercialCoordinatorV2(const std::string& private_ledger,CommercialClockV2&,storage::ReceiptAuthorityV2&,
        uint32_t maximum_format=2,std::shared_ptr<StoreProofAuthorityV3> configured_store={});
    // Private application-owned ingress; no caller-provided paid state or clock.
    // Challenge is single-use even on failure. Cached proofs never use this path.
    void configure_store(std::shared_ptr<StoreProofAuthorityV3>);
    std::string begin_store_request();
    void cancel_store_request(std::string_view);
    StoreAcknowledgmentV3 accept_store(std::string signed_proof,std::string_view native_nonce);
    storage::PublicationTimeV2 sample_at_publication()override;
    // Obtains validated committed native evidence BEFORE holding commerce lock;
    // lock ordering never creates a second SQLite editing/publishing authority.
    unsigned reconcile(storage::Repository&);
    State state();
    static constexpr bool recovery_allowed()noexcept{return true;}
};
}
#endif
