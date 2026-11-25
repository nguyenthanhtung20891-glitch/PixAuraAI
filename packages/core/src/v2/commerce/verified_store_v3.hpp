#ifndef PIXAURA_VERIFIED_STORE_V3_HPP
#define PIXAURA_VERIFIED_STORE_V3_HPP
#include "store_record_v2.hpp"
#include "../../sha256.hpp"
#include <memory>
namespace pixaura::commerce {
enum class StoreProofOriginV3:uint32_t {fresh=1,cached=2};
struct StoreSignatureResultV3 {
    StoreObservationV2 observation;
    // Explicit additive precision; v2 signed_at remains whole issuer seconds.
    uint32_t signed_millisecond=0;
};
// Trusted platform cryptographic boundary, not a UI supplied paid boolean.
// Apple implementations MUST verify pinned Apple JWS chain/device/app binding;
// Google implementations MUST verify the configured P-256 key and scope/nonce.
// Fresh Apple proofs additionally require the private actual-SDK delivery fence;
// a valid cached JWS cannot become fresh merely by supplying a new nonce.
// No permissive/default verifier is supplied by production code.
struct StoreSignatureVerifierV3 {
    virtual ~StoreSignatureVerifierV3()=default;
    virtual uint32_t store()const=0;
    virtual std::string key_identity()const=0;
    virtual StoreSignatureResultV3 verify(const StoreCatalogV2&,std::string_view,
        StoreProofOriginV3,std::string_view expected_native_nonce)=0;
};
class StoreProofAuthorityV3;
class VerifiedStoreProofV3 final {
    friend class StoreProofAuthorityV3;
    std::string proof_,proof_digest_,authority_,scope_;
    StoreSignatureResultV3 result_;
    StoreProofOriginV3 origin_;
    std::shared_ptr<const unsigned> binding_;
    VerifiedStoreProofV3(std::string proof,std::string digest,std::string authority,
        std::string scope,StoreSignatureResultV3 result,StoreProofOriginV3 origin,std::shared_ptr<const unsigned> binding):
        proof_(std::move(proof)),proof_digest_(std::move(digest)),authority_(std::move(authority)),
        scope_(std::move(scope)),result_(std::move(result)),origin_(origin),binding_(std::move(binding)){}
public:
    const StoreObservationV2& observation()const{return result_.observation;}
    uint32_t signed_millisecond()const{return result_.signed_millisecond;}
    const std::string& raw_proof()const{return proof_;}
    const std::string& proof_digest()const{return proof_digest_;}
    const std::string& authority_identity()const{return authority_;}
    const std::string& scope()const{return scope_;}
    StoreProofOriginV3 origin()const{return origin_;}
};
class StoreProofAuthorityV3 final {
    const StoreCatalogV2 catalog_;
    const std::string pin_;
    const std::shared_ptr<StoreSignatureVerifierV3> verifier_;
    const std::shared_ptr<const unsigned> binding_=std::make_shared<const unsigned>(0);
    static std::string digest(std::string_view bytes){
        storage::Sha256 hash;hash.update(reinterpret_cast<const uint8_t*>(bytes.data()),bytes.size());return hash.finish();
    }
public:
    StoreProofAuthorityV3(StoreCatalogV2 catalog,std::string configured_pin,
        std::shared_ptr<StoreSignatureVerifierV3> verifier):catalog_(std::move(catalog)),
        pin_(std::move(configured_pin)),verifier_(std::move(verifier)){
        store_v2::catalog(catalog_);(void)document::Digest::parse(pin_);
        store_v2::need(verifier_&&verifier_->store()==catalog_.store&&verifier_->key_identity()==pin_);
    }
    StoreProofAuthorityV3(const StoreProofAuthorityV3&)=delete;
    StoreProofAuthorityV3& operator=(const StoreProofAuthorityV3&)=delete;
    const StoreCatalogV2& catalog()const{return catalog_;}
    const std::string& pin()const{return pin_;}
    // Consumers must use their OWN native authority, never a caller's factory.
    bool owns(const VerifiedStoreProofV3& proof)const{return proof.binding_==binding_;}
    VerifiedStoreProofV3 verify(std::string proof,StoreProofOriginV3 origin,std::string_view nonce={})const{
        store_v2::need(!proof.empty()&&proof.size()<=16384);
        store_v2::need(origin==StoreProofOriginV3::fresh||origin==StoreProofOriginV3::cached);
        if(origin==StoreProofOriginV3::fresh){
            store_v2::need(nonce.size()==32);for(char c:nonce)store_v2::need((c>='0'&&c<='9')||(c>='a'&&c<='f'));
        }else store_v2::need(nonce.empty());
        store_v2::need(verifier_->store()==catalog_.store&&verifier_->key_identity()==pin_);
        auto result=verifier_->verify(catalog_,proof,origin,nonce);
        store_v2::observation(catalog_,result.observation);store_v2::need(result.signed_millisecond<1000);
        // Detect a mutable verifier changing configured key identity during work.
        store_v2::need(verifier_->store()==catalog_.store&&verifier_->key_identity()==pin_);
        const auto scope=catalog_.application+'\n'+std::to_string(catalog_.store)+'\n'+std::to_string(catalog_.environment)+'\n'+pin_;
        const auto hash=digest(proof);
        return VerifiedStoreProofV3(std::move(proof),hash,pin_,scope,std::move(result),origin,binding_);
    }
};
}
#endif
