// Compiled once with api_v2.cpp, sharing its runtime registry/resource admission.
#include "pixaura/store_ingress_v3.h"
namespace {
template<size_t N>std::string store_text(const char (&value)[N]){
    const auto end=static_cast<const char*>(std::memchr(value,0,N));need(end&&end!=value,6);
    return {value,static_cast<size_t>(end-value)};
}
struct PlatformStoreVerifierV3 final:commerce::StoreSignatureVerifierV3 {
    const pixaura_store_v3_configuration configuration;
    bool retained=false;
    explicit PlatformStoreVerifierV3(const pixaura_store_v3_configuration& c):configuration(c){
        need(c.context&&c.retain&&c.release&&c.verify,6);need(c.retain(c.context)==0,6);retained=true;
    }
    ~PlatformStoreVerifierV3(){if(retained)configuration.release(configuration.context);}
    uint32_t store()const override{return configuration.store;}
    std::string key_identity()const override{return store_text(configuration.key_identity);}
    commerce::StoreSignatureResultV3 verify(const commerce::StoreCatalogV2&,std::string_view proof,
        commerce::StoreProofOriginV3 origin,std::string_view nonce)override{
        pixaura_store_v3_verified result{};result.version=3;result.struct_size=sizeof(result);
        need(configuration.verify(configuration.context,reinterpret_cast<const uint8_t*>(proof.data()),proof.size(),static_cast<uint32_t>(origin),nonce.data(),nonce.size(),&result)==0,6);
        need(result.version==3&&result.struct_size==sizeof(result)&&result.reserved==0&&store_text(result.verified_key_identity)==key_identity(),6);
        return {{store_text(result.product),store_text(result.transaction_digest),result.kind,result.status,result.signed_seconds,result.expires},result.signed_millisecond};
    }
};
template<class F>int32_t store_call(uint32_t version,F&& action)noexcept{
    return call(2,[&]{need(version==3,2);action();});
}
}
extern "C" {
int32_t pixaura_store_v3_bind(uint32_t version,uint64_t runtime,const pixaura_store_v3_configuration* configuration){return store_call(version,[&]{
    pointer(configuration);
    // Only the fixed version/size prefix is readable until the caller's declared
    // structure length is validated. Do not copy callbacks/catalog from a short buffer.
    need(configuration->version==3&&configuration->struct_size==sizeof(pixaura_store_v3_configuration),6);
    const auto c=*configuration;
    need(c.reserved==0&&c.product_count>0&&c.product_count<=8,6);
    commerce::StoreCatalogV2 catalog{store_text(c.application),c.store,c.environment,{}};
    const auto pin=store_text(c.key_identity);(void)document::Digest::parse(pin);
    for(unsigned i=0;i<c.product_count;++i)catalog.products.push_back({store_text(c.products[i].identifier),c.products[i].kind});
    commerce::store_v2::catalog(catalog);
    auto owner=lookup(runtimes,runtime);need(owner->commercial!=nullptr,3003);
    WorkBinding work(owner->admission);const auto admission=admit_resources(owner->admission);
    std::unique_ptr<tiled::Budget::Lease> lease(new tiled::Budget::Lease(owner->budget->reserve(262144)));
    auto verifier=std::make_shared<PlatformStoreVerifierV3>(c);
    auto authority=std::make_shared<commerce::StoreProofAuthorityV3>(std::move(catalog),pin,std::move(verifier));
    owner->commercial->configure_store(std::move(authority));
    owner->store_control=std::move(lease);
});}
int32_t pixaura_store_v3_begin(uint32_t version,uint64_t runtime,char* nonce,uint64_t capacity){return store_call(version,[&]{
    need(nonce&&capacity>=33);auto owner=lookup(runtimes,runtime);need(owner->commercial!=nullptr,3003);
    WorkBinding work(owner->admission);const auto admission=admit_resources(owner->admission);
    const auto challenge=owner->commercial->begin_store_request();char out[33]{};need(challenge.size()==32,6);std::memcpy(out,challenge.data(),32);std::memcpy(nonce,out,sizeof(out));
});}
int32_t pixaura_store_v3_cancel(uint32_t version,uint64_t runtime,const char* nonce,uint64_t size){return store_call(version,[&]{
    const auto challenge=text(nonce,size,32);need(challenge.size()==32,6);auto owner=lookup(runtimes,runtime);need(owner->commercial!=nullptr,3003);
    owner->commercial->cancel_store_request(challenge);
});}
int32_t pixaura_store_v3_accept(uint32_t version,uint64_t runtime,const uint8_t* proof,uint64_t size,const char* nonce,uint64_t nonce_size,pixaura_store_v3_acknowledgment* acknowledgment){return store_call(version,[&]{
    pointer(acknowledgment);need(proof&&size>0&&size<=16384,6);const auto challenge=text(nonce,nonce_size,32);need(challenge.size()==32,6);
    auto owner=lookup(runtimes,runtime);need(owner->commercial!=nullptr,3003);
    WorkBinding work(owner->admission);const auto admission=admit_resources(owner->admission);
    tiled::Budget::Lease lease(owner->budget->reserve(262144));
    const auto committed=owner->commercial->accept_store(std::string(reinterpret_cast<const char*>(proof),static_cast<size_t>(size)),challenge);
    pixaura_store_v3_acknowledgment out{};out.version=3;out.struct_size=sizeof(out);out.replay=committed.replay();
    copy(out.product,committed.product());copy(out.identity,committed.identity());copy(out.proof_digest,committed.proof_digest());*acknowledgment=out;
});}
}
