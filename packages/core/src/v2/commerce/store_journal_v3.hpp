#ifndef PIXAURA_STORE_JOURNAL_V3_HPP
#define PIXAURA_STORE_JOURNAL_V3_HPP
#include "quota.hpp"
#include "verified_store_v3.hpp"
namespace pixaura::commerce {
// Private store observation acknowledgment, never a native edit/Apply receipt.
class StoreAcknowledgmentV3 final {
    friend class StoreJournalV3;
    std::string product_,identity_,proof_digest_;bool replay_=false;
    StoreAcknowledgmentV3(std::string product,std::string identity,std::string proof_digest,bool replay):product_(std::move(product)),identity_(std::move(identity)),proof_digest_(std::move(proof_digest)),replay_(replay){}
public:
    const std::string& product()const{return product_;}
    const std::string& identity()const{return identity_;}
    const std::string& proof_digest()const{return proof_digest_;}
    bool replay()const{return replay_;}
};
class StoreJournalV3 final {
    Ledger& ledger_;storage::ReceiptAuthorityV2& key_;
    StoreProofAuthorityV3& authority_;
    StoreCatalogV2 catalog_;std::string configuration_,configuration_digest_;
    struct Entry {
        unsigned index=0;StoreObservationV2 value;uint32_t fraction=0;
        int64_t native_time=0;bool uncertain=false;
        std::string proof_digest,ack;
    };
    std::string configuration()const;
    std::string encode(const Entry&,bool with_ack=true)const;
    Entry decode(std::string_view)const;
    std::map<unsigned,Entry> read();
    std::string manifest(const std::map<unsigned,Entry>&)const;
    std::string authenticate(std::string_view,std::string_view)const;
public:
    // References must outlive this object; all storage uses ledger's connection.
    StoreJournalV3(Ledger&,storage::ReceiptAuthorityV2&,StoreProofAuthorityV3&);
    StoreJournalV3(const StoreJournalV3&)=delete;
    StoreJournalV3& operator=(const StoreJournalV3&)=delete;
    StoreAcknowledgmentV3 accept(const VerifiedStoreProofV3&,const Clock&);
    // Re-verifies the actual signed proofs, never regenerates a fresh SDK fence.
    // One invalid proof does not discard other independently verified products.
    std::map<std::string,Entitlement> restore();
};
}
#endif
