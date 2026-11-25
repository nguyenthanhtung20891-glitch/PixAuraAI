#include "../core/src/v2/commerce/store_record_v2.hpp"
#include "receipt_key_v2.hpp"
#include "../core/src/v2/commerce/verified_store_v3.hpp"
#include "sodium.h"
#include "../core/src/v2/commerce/store_journal_v3.hpp"
#include "coordinator_v2.hpp"
#include "pixaura/store_ingress_v3.h"
#include <cstring>
#include <atomic>
#include <condition_variable>
#include <thread>
#include "../core/vendor/sqlite/sqlite3.h"
#include <filesystem>
#include <chrono>
#include <cstdlib>
#include <cstdio>
#include <stdexcept>
#include <type_traits>
using namespace pixaura;
namespace pixaura::commerce {struct LedgerTestAccess {static sqlite3* db(Ledger& l){return l.db_;}};}
#define CHECK(x) do{if(!(x))throw std::runtime_error("store record line "+std::to_string(__LINE__));}while(false)
template<class F>void rejects(F action){bool rejected=false;try{action();}catch(const document::Failure&){rejected=true;}CHECK(rejected);}
// Real cryptographic synthetic protocol fixture only. This Ed25519 key is NOT
// an Apple/Google trust anchor, store transaction, entitlement or paid grant.
struct SyntheticStoreVerifier final:commerce::StoreSignatureVerifierV3 {
    std::array<unsigned char,crypto_sign_PUBLICKEYBYTES> pub{};
    std::array<unsigned char,crypto_sign_SECRETKEYBYTES> secret{};
    bool rotate_during_verify=false,rotated=false,bad_fraction=false;std::string declared_pin;
    explicit SyntheticStoreVerifier(unsigned fixture=1){CHECK(sodium_init()>=0);std::array<unsigned char,crypto_sign_SEEDBYTES> seed{};seed.fill(static_cast<unsigned char>(fixture));CHECK(crypto_sign_seed_keypair(pub.data(),secret.data(),seed.data())==0);}
    ~SyntheticStoreVerifier(){sodium_memzero(secret.data(),secret.size());}
    uint32_t store()const override{return 1;}
    std::string key_identity()const override{
        if(!declared_pin.empty())return declared_pin;
        if(rotated)return std::string(64,'f');storage::Sha256 hash;hash.update(pub.data(),pub.size());return hash.finish();
    }
    std::string sign(const commerce::StoreCatalogV2& catalog,const commerce::StoreObservationV2& value,const std::string& nonce){
        const auto body=commerce::store_v2::canonical(catalog,value)+nonce;
        std::array<unsigned char,crypto_sign_BYTES> signature{};
        CHECK(crypto_sign_detached(signature.data(),nullptr,reinterpret_cast<const unsigned char*>(body.data()),body.size(),secret.data())==0);
        return std::string(reinterpret_cast<const char*>(signature.data()),signature.size())+body;
    }
    commerce::StoreSignatureResultV3 verify(const commerce::StoreCatalogV2& catalog,std::string_view proof,
        commerce::StoreProofOriginV3 origin,std::string_view nonce)override{
        commerce::store_v2::need(proof.size()>crypto_sign_BYTES+32);
        const auto body=proof.substr(crypto_sign_BYTES);
        commerce::store_v2::need(crypto_sign_verify_detached(reinterpret_cast<const unsigned char*>(proof.data()),reinterpret_cast<const unsigned char*>(body.data()),body.size(),pub.data())==0);
        const auto signed_nonce=body.substr(body.size()-32);
        if(origin==commerce::StoreProofOriginV3::fresh)commerce::store_v2::need(signed_nonce==nonce);
        auto record=commerce::store_v2::parse(catalog,body.substr(0,body.size()-32));
        if(rotate_during_verify)rotated=true;
        return {record,bad_fraction?1000u:0u};
    }
};
struct NativeStoreFixtureV3 {
    std::shared_ptr<SyntheticStoreVerifier> signer;
    commerce::StoreCatalogV2 catalog;std::atomic<int> references{0};
    std::atomic<bool> block{false};std::mutex gate;std::condition_variable changed;bool entered=false,proceed=false;
    NativeStoreFixtureV3(std::shared_ptr<SyntheticStoreVerifier> s,commerce::StoreCatalogV2 c):signer(std::move(s)),catalog(std::move(c)){}
    static int32_t retain(void* raw){++static_cast<NativeStoreFixtureV3*>(raw)->references;return 0;}
    static void release(void* raw){--static_cast<NativeStoreFixtureV3*>(raw)->references;}
    static int32_t verify(void* raw,const uint8_t* proof,uint64_t size,uint32_t origin,const char* nonce,uint64_t nonce_size,pixaura_store_v3_verified* out){
        try{
            auto& context=*static_cast<NativeStoreFixtureV3*>(raw);
            if(context.block){std::unique_lock<std::mutex> lock(context.gate);context.entered=true;context.changed.notify_all();context.changed.wait(lock,[&]{return context.proceed;});}
            const auto verified=context.signer->verify(context.catalog,{reinterpret_cast<const char*>(proof),static_cast<size_t>(size)},static_cast<commerce::StoreProofOriginV3>(origin),nonce_size?std::string_view(nonce,static_cast<size_t>(nonce_size)):std::string_view{});
            const auto& v=verified.observation;pixaura_store_v3_verified value{};value.version=3;value.struct_size=sizeof(value);value.kind=v.kind;value.status=v.status;value.signed_millisecond=verified.signed_millisecond;value.signed_seconds=v.signed_at;value.expires=v.expires;
            std::memcpy(value.product,v.product.c_str(),v.product.size()+1);std::memcpy(value.transaction_digest,v.transaction_digest.c_str(),v.transaction_digest.size()+1);
            const auto pin=context.signer->key_identity();std::memcpy(value.verified_key_identity,pin.c_str(),pin.size()+1);*out=value;return 0;
        }catch(...){return 6;}
    }
};
int main(int argc,char** argv){try{
    using namespace commerce;
    static_assert(!std::is_constructible<store_v2::AuthenticatedCache,StoreObservationV2>::value,"unsigned metadata cannot construct authenticated cache");
    uint8_t bytes[32];for(unsigned i=0;i<32;++i)bytes[i]=static_cast<uint8_t>(i+1);ReceiptKeyV2 key(bytes,32);bytes[0]^=1;ReceiptKeyV2 other(bytes,32);
    // Synthetic metadata/key only: NOT a store transaction or paid grant.
    StoreCatalogV2 catalog{"test.synthetic.application",1,1,{{"test.synthetic.monthly",1},{"test.synthetic.lifetime",2}}};
    StoreObservationV2 record{"test.synthetic.monthly",std::string(64,'a'),1,1,1000,2000};
    if(argc>1){
        CHECK(argc==4);const std::string path=argv[2],mode=argv[1],point=argv[3];
        auto signature=std::make_shared<SyntheticStoreVerifier>();StoreProofAuthorityV3 proof_owner(catalog,signature->key_identity(),signature);
        Ledger ledger(path,3,&key);StoreJournalV3 journal(ledger,key,proof_owner);Clock os{1500,100,std::string(32,'c'),false};const std::string nonce(32,'a');
        const auto fact=[&](bool revoke){auto value=record;if(revoke){value.status=3;value.signed_at=1100;}return proof_owner.verify(signature->sign(catalog,value,nonce),StoreProofOriginV3::fresh,nonce);};
        if(mode=="--journal-prepare"){CHECK(point=="active");CHECK(!journal.accept(fact(false),os).replay());return 0;}
        if(mode=="--journal-crash"){
            if(point=="before_commit")sqlite3_commit_hook(LedgerTestAccess::db(ledger),[](void*)->int{std::_Exit(76);},nullptr);
            CHECK(point=="before_commit"||point=="after_commit");(void)journal.accept(fact(true),os);std::_Exit(76);
        }
        CHECK(mode=="--journal-inspect"&&(point=="active"||point=="revoked"));
        const auto values=journal.restore();CHECK(values.size()==1&&values.at(record.product).revoked==(point=="revoked"));
        const auto first=journal.accept(fact(point=="revoked"),os),again=journal.accept(fact(point=="revoked"),os);
        CHECK(first.replay()&&again.replay()&&first.identity()==again.identity()&&ledger.state(os).trial_start==-1);return 0;
    }
    const auto body=store_v2::canonical(catalog,record),seal=key.authenticate(body);
    auto verified=store_v2::AuthenticatedCache::verify(catalog,body,seal,key);CHECK(verified.value().expires==2000&&verified.value().signed_at==1000);
    rejects([&]{store_v2::AuthenticatedCache::verify(catalog,body,seal,other);});
    rejects([&]{store_v2::AuthenticatedCache::verify(catalog,body,"",key);});
    for(size_t i=0;i<body.size();++i){auto changed=body;changed[i]^=1;rejects([&]{store_v2::AuthenticatedCache::verify(catalog,changed,seal,key);});}
    rejects([&]{store_v2::parse(catalog,body+"extra\n");});rejects([&]{store_v2::parse(catalog,body.substr(0,body.size()-1));});
    auto changed=catalog;changed.application="test.other.application";rejects([&]{store_v2::AuthenticatedCache::verify(changed,body,seal,key);});
    changed=catalog;changed.environment=2;rejects([&]{store_v2::AuthenticatedCache::verify(changed,body,seal,key);});
    changed=catalog;changed.store=2;rejects([&]{store_v2::AuthenticatedCache::verify(changed,body,seal,key);});
    changed=catalog;changed.products[0].kind=2;rejects([&]{store_v2::AuthenticatedCache::verify(changed,body,seal,key);});
    changed=catalog;changed.products.push_back(changed.products.front());rejects([&]{store_v2::catalog(changed);});
    changed=catalog;changed.products.clear();rejects([&]{store_v2::catalog(changed);});
    auto invalid=record;invalid.signed_at=0;rejects([&]{store_v2::canonical(catalog,invalid);});
    invalid=record;invalid.expires=-1;rejects([&]{store_v2::canonical(catalog,invalid);});
    invalid=record;invalid.transaction_digest=std::string(64,'x');rejects([&]{store_v2::canonical(catalog,invalid);});
    for(uint32_t store=1;store<=2;++store)for(uint32_t environment=1;environment<=2;++environment)for(uint32_t kind=1;kind<=2;++kind)for(uint32_t status=1;status<=3;++status){
        auto configured=catalog;configured.store=store;configured.environment=environment;
        StoreObservationV2 value{kind==1?"test.synthetic.monthly":"test.synthetic.lifetime",std::string(64,'b'),kind,status,INT64_C(8640000000000),kind==1?INT64_C(8640000000000):0};
        if(kind==2&&status==2){rejects([&]{store_v2::canonical(configured,value);});continue;}
        const auto canonical=store_v2::canonical(configured,value);auto result=store_v2::AuthenticatedCache::verify(configured,canonical,key.authenticate(canonical),key);
        CHECK(result.value().status==status&&result.value().kind==kind&&result.value().expires==value.expires);
    }
    static_assert(!std::is_default_constructible<StoreAcknowledgmentV3>::value,"no fabricated post-COMMIT acknowledgment");
    static_assert(!std::is_copy_constructible<StoreProofAuthorityV3>::value,"an authority epoch cannot be copied/revived");
    static_assert(!std::is_default_constructible<VerifiedStoreProofV3>::value,"no unsigned/default store proof");
    static_assert(!std::is_constructible<VerifiedStoreProofV3,StoreObservationV2>::value,"metadata is not a cryptographic proof");
    auto signer=std::make_shared<SyntheticStoreVerifier>();const auto pin=signer->key_identity();
    StoreProofAuthorityV3 authority(catalog,pin,signer);const std::string nonce(32,'a');
    const auto signed_proof=signer->sign(catalog,record,nonce);
    auto fact=authority.verify(signed_proof,StoreProofOriginV3::fresh,nonce);
    CHECK(authority.owns(fact)&&fact.observation().product==record.product&&fact.signed_millisecond()==0);
    CHECK(fact.origin()==StoreProofOriginV3::fresh&&fact.authority_identity()==pin&&fact.raw_proof()==signed_proof);
    auto cached=authority.verify(signed_proof,StoreProofOriginV3::cached);CHECK(cached.origin()==StoreProofOriginV3::cached);
    unsigned char independent[32];CHECK(crypto_hash_sha256(independent,reinterpret_cast<const unsigned char*>(signed_proof.data()),signed_proof.size())==0);
    static const char hex[]="0123456789abcdef";std::string expected;for(auto c:independent){expected+=hex[c>>4];expected+=hex[c&15];}CHECK(fact.proof_digest()==expected);
    StoreProofAuthorityV3 another(catalog,pin,signer);CHECK(!another.owns(fact)&&!authority.owns(another.verify(signed_proof,StoreProofOriginV3::cached)));
    rejects([&]{authority.verify(body,StoreProofOriginV3::cached);}); // unsigned metadata
    rejects([&]{authority.verify(body+seal,StoreProofOriginV3::cached);}); // local MAC is NOT store signature
    rejects([&]{authority.verify(signed_proof,StoreProofOriginV3::fresh,std::string(32,'b'));});
    rejects([&]{authority.verify(signed_proof,StoreProofOriginV3::fresh);});
    rejects([&]{authority.verify(signed_proof,StoreProofOriginV3::cached,nonce);});
    rejects([&]{authority.verify(signed_proof,static_cast<StoreProofOriginV3>(3));});
    rejects([&]{authority.verify(std::string(16385,'x'),StoreProofOriginV3::cached);});
    for(size_t i=0;i<signed_proof.size();++i){auto changed=signed_proof;changed[i]^=1;rejects([&]{authority.verify(changed,StoreProofOriginV3::cached);});}
    auto wrong_pin=pin;wrong_pin[0]=pin[0]=='0'?'1':'0';rejects([&]{StoreProofAuthorityV3 wrong(catalog,wrong_pin,signer);});
    auto other_signer=std::make_shared<SyntheticStoreVerifier>(2);rejects([&]{authority.verify(other_signer->sign(catalog,record,nonce),StoreProofOriginV3::cached);});
    // Even a caller's verifier that CLAIMS the real pin cannot mint evidence
    // owned by the actual native authority. Identity strings are not authority.
    other_signer->declared_pin=pin;StoreProofAuthorityV3 caller_factory(catalog,pin,other_signer);
    auto caller_fact=caller_factory.verify(other_signer->sign(catalog,record,nonce),StoreProofOriginV3::cached);
    CHECK(caller_fact.authority_identity()==pin&&!authority.owns(caller_fact));
    auto other_catalog=catalog;other_catalog.environment=2;StoreProofAuthorityV3 foreign(other_catalog,pin,signer);rejects([&]{foreign.verify(signed_proof,StoreProofOriginV3::cached);});
    // Native durable metadata journal only, synthetic real signatures. These
    // projections are NOT genuine store transactions or shipping paid activation.
    const auto root=std::filesystem::current_path()/("store-journal-v3-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    CHECK(std::filesystem::create_directory(root));const auto path=(root/"ledger.sqlite").string();
    Clock os{1500,100,std::string(32,'c'),false};std::string original_ack,revoked_ack;
    auto life=record;life.product="test.synthetic.lifetime";life.kind=2;life.expires=0;
    auto revoked=record;revoked.status=3;revoked.signed_at=1100;
    const auto fresh=[&](const StoreObservationV2& o){return authority.verify(signer->sign(catalog,o,nonce),StoreProofOriginV3::fresh,nonce);};
    {
        Ledger ledger(path,3,&key);StoreJournalV3 journal(ledger,key,authority);
        CHECK(journal.restore().empty());
        rejects([&]{journal.accept(cached,os);}); // cached evidence cannot create a new acknowledgment
        rejects([&]{journal.accept(caller_fact,os);}); // caller's claimed pin is insufficient
        const auto ack=journal.accept(fresh(record),os);CHECK(!ack.replay()&&ack.identity().size()==64&&ack.product()==record.product);original_ack=ack.identity();
        auto again=journal.accept(fresh(record),os);CHECK(again.replay()&&again.identity()==original_ack);
        auto restored=journal.restore();CHECK(restored.size()==1&&restored.at(record.product).verified&&!restored.at(record.product).revoked);
        auto tied_revoke=record;tied_revoke.status=3;
        CHECK(!journal.accept(fresh(tied_revoke),os).replay());CHECK(journal.restore().at(record.product).revoked);
        rejects([&]{journal.accept(fresh(record),os);});
        auto new_active=record;new_active.signed_at++;
        CHECK(!journal.accept(fresh(new_active),os).replay());CHECK(!journal.restore().at(record.product).revoked);
        // Real SQLite COMMIT failure must not leave a newer fact/high-water/ack.
        sqlite3_commit_hook(LedgerTestAccess::db(ledger),[](void*){return 1;},nullptr);
        rejects([&]{journal.accept(fresh(revoked),os);});
        sqlite3_commit_hook(LedgerTestAccess::db(ledger),nullptr,nullptr);
        CHECK(!journal.restore().at(record.product).revoked);
        const auto rev=journal.accept(fresh(revoked),os);CHECK(!rev.replay()&&rev.identity()!=original_ack);revoked_ack=rev.identity();
        rejects([&]{journal.accept(fresh(record),os);});
        auto same_generation=revoked;same_generation.status=1;
        rejects([&]{journal.accept(fresh(same_generation),os);});
        // A newer ACTUAL signature may authoritatively restore status (for
        // example refund reversal). Never invent a permanent commercial ban.
        auto restored_fact=revoked;restored_fact.status=1;restored_fact.signed_at++;
        CHECK(!journal.accept(fresh(restored_fact),os).replay());CHECK(!journal.restore().at(record.product).revoked);
        revoked.signed_at=restored_fact.signed_at+1;
        revoked_ack=journal.accept(fresh(revoked),os).identity();CHECK(journal.restore().at(record.product).revoked);
        CHECK(journal.accept(fresh(life),os).identity().size()==64);
        CHECK(ledger.state(os).trial_start==-1&&ledger.state(os).access==0); // store journal never starts trial or directly grants access
    }
    {
        Ledger ledger(path,3,&key);StoreJournalV3 journal(ledger,key,authority);
        CHECK(journal.accept(fresh(revoked),os).identity()==revoked_ack);
        auto restored=journal.restore();CHECK(restored.size()==2&&restored.at(record.product).revoked&&restored.at(life.product).kind==Entitlement::lifetime);
        // Corrupt only monthly raw proof: preserve the independent lifetime and
        // authenticated monthly high-water. Never recover by replaying old active.
        CHECK(sqlite3_exec(LedgerTestAccess::db(ledger),"UPDATE store_fact_v3 SET proof=x'00' WHERE product='test.synthetic.monthly'",nullptr,nullptr,nullptr)==SQLITE_OK);
        restored=journal.restore();CHECK(restored.size()==1&&restored.count(life.product)==1);
        rejects([&]{journal.accept(fresh(record),os);});
        const auto repair=journal.accept(fresh(revoked),os);CHECK(repair.replay()&&repair.identity()==revoked_ack);
        CHECK(journal.restore().at(record.product).revoked);
        auto wrong_catalog=catalog;wrong_catalog.environment=2;StoreProofAuthorityV3 wrong_scope(wrong_catalog,pin,signer);
        rejects([&]{StoreJournalV3 wrong(ledger,key,wrong_scope);});
        CHECK(journal.restore().size()==2);
        CHECK(sqlite3_exec(LedgerTestAccess::db(ledger),"UPDATE store_manifest_v3 SET body='corrupt'",nullptr,nullptr,nullptr)==SQLITE_OK);
        rejects([&]{journal.restore();});rejects([&]{journal.accept(fresh(record),os);});
    }
    struct FixtureClock final:CommercialClockV2 {
        Clock now{1500,100,std::string(32,'c'),false};Clock sample_os()override{return now;}
    } fixture_clock;
    const auto owner_path=(root/"owner.sqlite").string();
    auto native_authority=std::make_shared<StoreProofAuthorityV3>(catalog,pin,signer);
    {
        CommercialCoordinatorV2 owner(owner_path,fixture_clock,key,3,native_authority);
        CHECK(owner.state().access==0&&owner.state().trial_start==-1);
        const auto challenge=owner.begin_store_request();CHECK(challenge.size()==32);
        rejects([&]{owner.begin_store_request();});
        rejects([&]{owner.accept_store(signed_proof,challenge);}); // old nonce is not a current native challenge
        rejects([&]{owner.accept_store(signed_proof,challenge);}); // failed verification consumed challenge
        const auto fresh_challenge=owner.begin_store_request();
        auto ack=owner.accept_store(signer->sign(catalog,record,fresh_challenge),fresh_challenge);
        CHECK(!ack.replay()&&owner.state().access==2&&owner.state().trial_start==-1);
        rejects([&]{owner.accept_store(signed_proof,fresh_challenge);});
        auto cancelled=owner.begin_store_request();owner.cancel_store_request(cancelled);
        rejects([&]{owner.accept_store(signer->sign(catalog,record,cancelled),cancelled);});
        // Exactly seven days after verified expiry is excluded, not extended
        // from the latest cache read or delivery acknowledgment.
        fixture_clock.now.uptime_seconds+=500;CHECK(owner.state().logical_utc==2000&&owner.state().access==2);
        fixture_clock.now.uptime_seconds+=7*86400-1;CHECK(owner.state().access==2);
        fixture_clock.now.uptime_seconds++;CHECK(owner.state().access==0);
        // Historical signed time never clears a reboot's uncertainty.
        fixture_clock.now.boot=std::string(32,'d');fixture_clock.now.uptime_seconds=1;
        CHECK(owner.state().clock_uncertain&&owner.state().access==0);
        fixture_clock.now.trusted=true; // raw platform flag is never trusted by store ingress
        const auto lifetime_challenge=owner.begin_store_request();
        owner.accept_store(signer->sign(catalog,life,lifetime_challenge),lifetime_challenge);
        CHECK(owner.state().clock_uncertain&&owner.state().access==3);
    }
    {
        CommercialCoordinatorV2 owner(owner_path,fixture_clock,key,3,native_authority);
        CHECK(owner.state().access==3&&owner.state().trial_start==-1&&owner.state().clock_uncertain);
        auto revoke_life=life;revoke_life.status=3;revoke_life.signed_at=1100;
        const auto challenge=owner.begin_store_request();owner.accept_store(signer->sign(catalog,revoke_life,challenge),challenge);
        CHECK(owner.state().access==0&&CommercialCoordinatorV2::recovery_allowed());
    }
    {
        // Missing actual store configuration leaves Free/recovery available;
        // durable metadata alone does not grant paid access.
        CommercialCoordinatorV2 unconfigured(owner_path,fixture_clock,key,3);
        CHECK(unconfigured.state().access==0&&unconfigured.state().trial_start==-1);
        rejects([&]{unconfigured.begin_store_request();});
    }
    {
        auto scope=catalog;scope.environment=2;auto foreign=std::make_shared<StoreProofAuthorityV3>(scope,pin,signer);
        CommercialCoordinatorV2 unconfigured(owner_path,fixture_clock,key,3,foreign);
        CHECK(unconfigured.state().access==0);rejects([&]{unconfigured.begin_store_request();});
    }
    // Exercise the actual exported runtime ingress, not a second publisher.
    const auto runtime_root=std::filesystem::canonical(root).string()+"/runtime";CHECK(std::filesystem::create_directory(runtime_root));
    uint64_t runtime=0;CHECK(pixaura_v2_runtime_create(2,runtime_root.data(),runtime_root.size(),bytes,32,&runtime)==0);
    NativeStoreFixtureV3 callback(signer,catalog);pixaura_store_v3_configuration configuration{};
    configuration.version=3;configuration.struct_size=sizeof(configuration);configuration.store=1;configuration.environment=1;configuration.product_count=2;
    std::memcpy(configuration.application,catalog.application.c_str(),catalog.application.size()+1);std::memcpy(configuration.key_identity,pin.c_str(),pin.size()+1);
    for(size_t i=0;i<catalog.products.size();++i){std::memcpy(configuration.products[i].identifier,catalog.products[i].id.c_str(),catalog.products[i].id.size()+1);configuration.products[i].kind=catalog.products[i].kind;}
    configuration.context=&callback;configuration.retain=NativeStoreFixtureV3::retain;configuration.release=NativeStoreFixtureV3::release;configuration.verify=NativeStoreFixtureV3::verify;
    CHECK(pixaura_store_v3_bind(2,runtime,&configuration)==2&&callback.references==0);
    // A declared short configuration must be rejected after its version/size
    // prefix, before copying the complete structure or touching callbacks.
    auto* prefix=static_cast<uint32_t*>(std::malloc(2*sizeof(uint32_t)));CHECK(prefix);
    prefix[0]=3;prefix[1]=2*sizeof(uint32_t);
    const auto short_status=pixaura_store_v3_bind(3,runtime,reinterpret_cast<const pixaura_store_v3_configuration*>(prefix));
    std::free(prefix);CHECK(short_status==6&&callback.references==0);

    auto bad_configuration=configuration;bad_configuration.product_count=9;
    CHECK(pixaura_store_v3_bind(3,runtime,&bad_configuration)==6&&callback.references==0);
    CHECK(pixaura_store_v3_bind(3,runtime,&configuration)==0&&callback.references==1);
    CHECK(pixaura_store_v3_bind(3,runtime,&configuration)!=0&&callback.references==1);
    char challenge[33]{};CHECK(pixaura_store_v3_begin(3,runtime,challenge,sizeof(challenge))==0&&std::strlen(challenge)==32);
    const auto raw_signed=signer->sign(catalog,life,challenge);pixaura_store_v3_acknowledgment receipt{};
    CHECK(pixaura_store_v3_accept(2,runtime,reinterpret_cast<const uint8_t*>(raw_signed.data()),raw_signed.size(),challenge,32,&receipt)==2&&receipt.version==0);
    callback.block=true;int32_t worker_status=-1;
    std::thread verification([&]{worker_status=pixaura_store_v3_accept(3,runtime,reinterpret_cast<const uint8_t*>(raw_signed.data()),raw_signed.size(),challenge,32,&receipt);});
    bool entered=false;{std::unique_lock<std::mutex> lock(callback.gate);entered=callback.changed.wait_for(lock,std::chrono::seconds(5),[&]{return callback.entered;});}
    const auto close_during_verification=entered?pixaura_v2_runtime_close(2,runtime):-1;
    {std::lock_guard<std::mutex> lock(callback.gate);callback.proceed=true;}callback.changed.notify_all();verification.join();callback.block=false;
    CHECK(entered&&close_during_verification==15&&worker_status==0&&callback.references==1);
    unsigned char proof_hash[32];CHECK(crypto_hash_sha256(proof_hash,reinterpret_cast<const unsigned char*>(raw_signed.data()),raw_signed.size())==0);
    std::string expected_proof_hash;for(auto byte:proof_hash){expected_proof_hash+=hex[byte>>4];expected_proof_hash+=hex[byte&15];}
    CHECK(std::string(receipt.proof_digest)==expected_proof_hash);
    CHECK(receipt.version==3&&receipt.struct_size==sizeof(receipt)&&std::string(receipt.product)==life.product&&std::strlen(receipt.identity)==64);
    pixaura_v2_commercial_state projection{};CHECK(pixaura_v2_runtime_commercial_state(2,runtime,&projection)==0&&projection.access==3&&projection.trial_start==-1);
    receipt.version=99;
    CHECK(pixaura_store_v3_accept(3,runtime,reinterpret_cast<const uint8_t*>(raw_signed.data()),raw_signed.size(),challenge,32,&receipt)!=0&&receipt.version==99);
    CHECK(pixaura_v2_runtime_close(2,runtime)==0&&callback.references==0);
    // Native journal alone cannot grant paid access without configured verifier.
    CHECK(pixaura_v2_runtime_create(2,runtime_root.data(),runtime_root.size(),bytes,32,&runtime)==0);
    CHECK(pixaura_v2_runtime_commercial_state(2,runtime,&projection)==0&&projection.access==0);
    CHECK(pixaura_store_v3_bind(3,runtime,&configuration)==0&&callback.references==1);
    CHECK(pixaura_v2_runtime_commercial_state(2,runtime,&projection)==0&&projection.access==3&&projection.trial_start==-1);
    CHECK(pixaura_v2_runtime_close(2,runtime)==0&&callback.references==0);
    CHECK(std::filesystem::remove_all(root)>0);
    std::puts("Store journal v3: same-connection atomic fact/high-water/ack; commit failure/reopen/idempotence/revocation/foreign authority/cache corruption PASS; synthetic only, no shipping paid grant");
    signer->bad_fraction=true;rejects([&]{authority.verify(signed_proof,StoreProofOriginV3::cached);});signer->bad_fraction=false;
    signer->rotate_during_verify=true;rejects([&]{authority.verify(signed_proof,StoreProofOriginV3::cached);});
    std::puts("Store proof v3 synthetic signature: configured key/store/scope/nonce, immutable authority binding, tamper and local-MAC rejection PASS; no paid grant or genuine store claim");
    std::puts("Store metadata cache v2: bounded canonical scope/catalog/expiry/status, domain MAC, wrong key/tamper rejection PASS; synthetic only, no paid grant or genuine store qualification");return 0;
}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}catch(const document::Failure& e){std::fprintf(stderr,"store record native%d\n",e.code);return 1;}}
