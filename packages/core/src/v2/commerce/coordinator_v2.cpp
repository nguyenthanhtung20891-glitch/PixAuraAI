#include "coordinator_v2.hpp"
#include "sodium.h"
#include <stdexcept>
#include "../../sha256.hpp"
#include "../../../vendor/sqlite/sqlite3.h"
namespace pixaura::commerce {
namespace {
void need(bool ok){if(!ok)throw document::Failure{6};}
struct Query{
    sqlite3_stmt* p=nullptr;
    Query(sqlite3* db,const char* sql){need(sqlite3_prepare_v2(db,sql,-1,&p,nullptr)==SQLITE_OK);}
    ~Query(){sqlite3_finalize(p);}
    bool row(){const int rc=sqlite3_step(p);need(rc==SQLITE_ROW||rc==SQLITE_DONE);return rc==SQLITE_ROW;}
    void text(int i,std::string_view s){need(s.size()<=4096);need(sqlite3_bind_text(p,i,s.data(),static_cast<int>(s.size()),SQLITE_TRANSIENT)==SQLITE_OK);}
    void integer(int i,int64_t n){need(sqlite3_bind_int64(p,i,n)==SQLITE_OK);}
    int64_t number(int i){need(sqlite3_column_type(p,i)==SQLITE_INTEGER);return sqlite3_column_int64(p,i);}
    std::string text(int i){need(sqlite3_column_type(p,i)==SQLITE_TEXT);const auto n=sqlite3_column_bytes(p,i);need(n>=0&&n<=4096);return {reinterpret_cast<const char*>(sqlite3_column_text(p,i)),static_cast<size_t>(n)};}
};
struct Transaction{
    sqlite3* db;bool done=false;
    explicit Transaction(sqlite3* d):db(d){need(sqlite3_exec(db,"BEGIN IMMEDIATE",nullptr,nullptr,nullptr)==SQLITE_OK);}
    void commit(){need(sqlite3_exec(db,"COMMIT",nullptr,nullptr,nullptr)==SQLITE_OK);done=true;}
    ~Transaction(){if(!done)sqlite3_exec(db,"ROLLBACK",nullptr,nullptr,nullptr);}
};
bool bounded_capsule(std::string_view capsule){
    // Historical v2 capsules retain their bound. Version 3 stores only bounded,
    // untrusted operation data, never an approval or executable candidate.
    return capsule.size()<=2048||(capsule.size()<=3300&&capsule.substr(0,16)=="PXA_ADVISORY_V3\n");
}
bool same(std::string_view a,std::string_view b){if(a.size()!=64||b.size()!=64)return false;unsigned difference=0;for(unsigned i=0;i<64;++i)difference|=static_cast<unsigned char>(a[i])^static_cast<unsigned char>(b[i]);return !difference;}
std::string proof(std::string_view event,int64_t utc,std::string_view source,std::string_view project,std::string_view doc,std::string_view rev,std::string_view digest,bool uncertain){
    (void)document::Id::parse(event);(void)document::Digest::parse(source);(void)document::Id::parse(project);(void)document::Id::parse(doc);(void)document::Id::parse(rev);(void)document::Digest::parse(digest);need(utc>=0);
    std::string out="PIXAURA_POST_COMMIT_TRIAL_PROOF_V2";
    const auto add=[&](std::string_view s){out+=':';out.append(std::to_string(s.size()));out+=':';out.append(s);};
    add(event);add(std::to_string(utc));add(source);add(project);add(doc);add(rev);add(digest);add(uncertain?"1":"0");return out;
}
}
CommercialCoordinatorV2::CommercialCoordinatorV2(const std::string& path,CommercialClockV2& clock,storage::ReceiptAuthorityV2& key,uint32_t maximum_format,std::shared_ptr<StoreProofAuthorityV3> store):ledger_(path,maximum_format,&key),clock_(clock),authority_(key),store_authority_(std::move(store)){
    need(maximum_format==2||maximum_format==3);
    auto os=clock_.sample_os();os.trusted=false;(void)ledger_.state(os);validate_origin();
    if(store_authority_){
        need(maximum_format==3);
        try{store_journal_=std::make_unique<StoreJournalV3>(ledger_,authority_,*store_authority_);restore_store_projection();}
        catch(const document::Failure&){store_journal_.reset();store_authority_.reset();}
        catch(const std::runtime_error&){store_journal_.reset();store_authority_.reset();}
    }
}
void CommercialCoordinatorV2::restore_store_projection(){
    need(store_journal_!=nullptr);std::map<std::string,Entitlement> paid;
    try{paid=store_journal_->restore();}catch(...){std::lock_guard<std::mutex> lock(ledger_.mutex_);ledger_.paid_.clear();throw;}
    std::lock_guard<std::mutex> lock(ledger_.mutex_);ledger_.paid_=std::move(paid);
}
void CommercialCoordinatorV2::configure_store(std::shared_ptr<StoreProofAuthorityV3> store){
    std::lock_guard<std::mutex> lock(gate_);need(store&&ledger_.format_version_==3&&!store_authority_&&!store_journal_);
    auto journal=std::make_unique<StoreJournalV3>(ledger_,authority_,*store);
    auto paid=journal->restore();
    {std::lock_guard<std::mutex> lock(ledger_.mutex_);ledger_.paid_=std::move(paid);}
    store_authority_=std::move(store);store_journal_=std::move(journal);
}
std::string CommercialCoordinatorV2::begin_store_request(){
    std::lock_guard<std::mutex> lock(gate_);need(store_journal_&&store_nonce_.empty());
    unsigned char bytes[16];randombytes_buf(bytes,sizeof bytes);static const char digits[]="0123456789abcdef";
    for(unsigned char byte:bytes){store_nonce_+=digits[byte>>4];store_nonce_+=digits[byte&15];}sodium_memzero(bytes,sizeof bytes);return store_nonce_;
}
void CommercialCoordinatorV2::cancel_store_request(std::string_view nonce){
    std::lock_guard<std::mutex> lock(gate_);if(nonce==store_nonce_)store_nonce_.clear();
}
StoreAcknowledgmentV3 CommercialCoordinatorV2::accept_store(std::string signed_proof,std::string_view nonce){
    std::lock_guard<std::mutex> lock(gate_);need(store_journal_&&!store_nonce_.empty()&&nonce==store_nonce_);
    const auto expected=std::move(store_nonce_);store_nonce_.clear();
    const auto verified=store_authority_->verify(std::move(signed_proof),StoreProofOriginV3::fresh,expected);
    auto os=clock_.sample_os();os.trusted=false;auto ack=store_journal_->accept(verified,os);
    restore_store_projection();return ack;
}
void CommercialCoordinatorV2::validate_origin(){
    ledger_.verify_clock();
    // Cached evidence is a separately authenticated POST-COMMIT native proof,
    // not merely a pre-COMMIT seal copied out of a WAL/opaque proposal blob.
    ledger_.trial_verified_=false;Query q(ledger_.db_,"SELECT event,logical_utc,source,project,document,revision,receipt_digest,uncertain,seal FROM native_trial_origin WHERE singleton=1");
    if(!q.row())return;
    const auto utc=q.number(1),uncertain=q.number(7);need(uncertain==0||uncertain==1);
    const auto envelope=proof(q.text(0),utc,q.text(2),q.text(3),q.text(4),q.text(5),q.text(6),uncertain!=0);
    need(same(q.text(8),authority_.authenticate(envelope)));need(!q.row());need(ledger_.value("trial")==utc&&ledger_.value("utc")>=utc);ledger_.trial_verified_=true;
}
storage::PublicationTimeV2 CommercialCoordinatorV2::sample_at_publication(){
    std::lock_guard<std::mutex> lock(gate_);auto os=clock_.sample_os();os.trusted=false;const auto state=ledger_.state(os);
    return {ledger_.value("utc"),os.utc_seconds,os.uptime_seconds,document::String(os.boot),state.clock_uncertain,false};
}
bool CommercialCoordinatorV2::acknowledge(const storage::NativeCommitReceiptV2& r){
    need(r.validated_after_commit_); // Only Repository's completed transaction/read can mint this property.
    const auto canonical=r.canonical();need(same(r.seal_,authority_.authenticate(canonical)));
    storage::Sha256 hash;hash.update(reinterpret_cast<const uint8_t*>(canonical.data()),canonical.size());hash.update(reinterpret_cast<const uint8_t*>(r.seal_.data()),r.seal_.size());const auto digest=hash.finish();
    const auto& t=r.time();const auto envelope=proof(r.event(),t.logical_utc,r.source(),r.project(),r.document(),r.revision(),digest,t.uncertain);
    const auto seal=authority_.authenticate(envelope);
    std::lock_guard<std::mutex> lock(gate_);auto os=clock_.sample_os();os.trusted=false;(void)ledger_.state(os);
    Transaction tx(ledger_.db_);validate_origin();
    if(ledger_.trial_verified_&&ledger_.value("trial")<=t.logical_utc){tx.commit();return false;}
    Query q(ledger_.db_,"INSERT INTO native_trial_origin VALUES(1,?,?,?,?,?,?,?,?,?) ON CONFLICT(singleton) DO UPDATE SET event=excluded.event,logical_utc=excluded.logical_utc,source=excluded.source,project=excluded.project,document=excluded.document,revision=excluded.revision,receipt_digest=excluded.receipt_digest,uncertain=excluded.uncertain,seal=excluded.seal");
    q.text(1,r.event());q.integer(2,t.logical_utc);q.text(3,r.source());q.text(4,r.project());q.text(5,r.document());q.text(6,r.revision());q.text(7,digest);q.integer(8,t.uncertain);q.text(9,seal);need(!q.row());
    ledger_.value("trial",t.logical_utc);if(t.uncertain||t.logical_utc>ledger_.value("utc"))ledger_.value("uncertain",1);
    if(t.logical_utc>ledger_.value("utc"))ledger_.value("utc",t.logical_utc);ledger_.seal_clock();
    tx.commit();ledger_.trial_verified_=true;return true;
}
unsigned CommercialCoordinatorV2::reconcile(storage::Repository& repo){
    const auto receipts=repo.receipts_v2(authority_);unsigned changed=0;
    for(const auto& r:receipts)if(acknowledge(r))++changed;return changed;
}
State CommercialCoordinatorV2::state(){std::lock_guard<std::mutex> lock(gate_);validate_origin();auto os=clock_.sample_os();os.trusted=false;return ledger_.state(os);}
Admission CommercialCoordinatorV2::reserve(const std::string& id,const std::string& source,bool ai){std::lock_guard<std::mutex> lock(gate_);validate_origin();auto os=clock_.sample_os();os.trusted=false;return ledger_.reserve(id,source,ai,os);}
bool CommercialCoordinatorV2::finish(const std::string& token,bool success,const std::string& usable){need(usable.empty());std::lock_guard<std::mutex> lock(gate_);auto os=clock_.sample_os();os.trusted=false;return ledger_.complete(token,success,false,{},os);}
namespace {
std::string delivery_proof(std::string_view token,std::string_view receipt,std::string_view capsule){
    need(!token.empty()&&token.size()<=64&&bounded_capsule(capsule));(void)document::Digest::parse(receipt);
    return "PIXAURA_RECOVERABLE_NATIVE_DELIVERY_V2:"+std::to_string(token.size())+":"+std::string(token)+":"+std::string(receipt)+":"+std::to_string(capsule.size())+":"+std::string(capsule);
}
constexpr std::string_view delivery_prefix="PIXAURA_ADVISORY_DELIVERY_V2\n";
}
bool CommercialCoordinatorV2::deliver(const storage::NativeAdvisoryDeliveryV2& r){
    need(r.validated_at_fence_&&r.epoch_&&r.generation_&&bounded_capsule(r.capsule_));
    for(const auto* id:{&r.event_,&r.project_,&r.document_,&r.revision_,&r.session_,&r.context_})(void)document::Id::parse(*id);
    (void)document::Digest::parse(r.source_);(void)document::Digest::parse(r.operations_digest_);
    const auto canonical=r.canonical();need(same(r.seal_,authority_.authenticate(canonical)));
    storage::Sha256 hash;hash.update(reinterpret_cast<const uint8_t*>(canonical.data()),canonical.size());const auto receipt=hash.finish();
    const auto envelope=delivery_proof(r.token_,receipt,r.capsule_);const auto seal=authority_.authenticate(envelope);
    const auto payload=std::string(delivery_prefix)+std::string(seal)+"\n"+std::string(receipt)+"\n"+std::string(r.capsule_);need(payload.size()<=4096);
    std::lock_guard<std::mutex> lock(gate_);validate_origin();auto os=clock_.sample_os();os.trusted=false;return ledger_.complete(r.token_,true,false,payload,os);
}
std::string CommercialCoordinatorV2::recover_delivery(const std::string& token){
    std::lock_guard<std::mutex> lock(gate_);const auto payload=ledger_.recover_delivery(token);if(payload.empty())return {};
    const auto start=delivery_prefix.size();need(payload.size()>start+130&&payload.size()<=4096&&payload.compare(0,start,delivery_prefix)==0&&payload[start+64]=='\n'&&payload[start+129]=='\n');
    const auto seal=payload.substr(start,64),receipt=payload.substr(start+65,64),capsule=payload.substr(start+130);
    need(same(seal,authority_.authenticate(delivery_proof(token,receipt,capsule))));return capsule;
}
std::vector<std::string> CommercialCoordinatorV2::deliveries(){
    std::lock_guard<std::mutex> lock(gate_);Query rows(ledger_.db_,"SELECT token FROM request WHERE done=1 AND ai=1 AND payload<>'' ORDER BY token LIMIT 65");
    std::vector<std::string> values;values.reserve(64);while(rows.row()){
        auto token=rows.text(0);need(values.size()<64&&token.size()<=64);const auto colon=token.find(':');need(colon>0&&colon<=19&&colon+33==token.size());
        for(size_t i=0;i<colon;++i)need(token[i]>='0'&&token[i]<='9');(void)document::Id::parse(token.substr(colon+1));values.push_back(std::move(token));
    }return values;
}
void CommercialCoordinatorV2::dismiss_delivery(const std::string& token){std::lock_guard<std::mutex> lock(gate_);ledger_.acknowledge_delivery(token);}

}

#include "store_journal_v3.inc"
