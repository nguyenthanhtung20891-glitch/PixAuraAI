// Compiled once with coordinator_v2.cpp; shares its private SQLite query/transaction code.
#include "store_journal_v3.hpp"
#include <algorithm>
namespace pixaura::commerce {
namespace {
constexpr int64_t store_maximum_time=INT64_MAX-366LL*86400;
struct StoreQuery {
    sqlite3_stmt* p=nullptr;
    StoreQuery(sqlite3* db,const char* sql){need(sqlite3_prepare_v2(db,sql,-1,&p,nullptr)==SQLITE_OK);}
    ~StoreQuery(){sqlite3_finalize(p);}
    bool row(){const auto rc=sqlite3_step(p);need(rc==SQLITE_ROW||rc==SQLITE_DONE);return rc==SQLITE_ROW;}
    void text(int i,std::string_view text){need(text.size()<=4096&&sqlite3_bind_text(p,i,text.data(),static_cast<int>(text.size()),SQLITE_TRANSIENT)==SQLITE_OK);}
    int64_t n(int i){need(sqlite3_column_type(p,i)==SQLITE_INTEGER);return sqlite3_column_int64(p,i);}
    std::string s(int i){need(sqlite3_column_type(p,i)==SQLITE_TEXT);const auto size=sqlite3_column_bytes(p,i);need(size>=0&&size<=4096);return {reinterpret_cast<const char*>(sqlite3_column_text(p,i)),static_cast<size_t>(size)};}
};
struct StoreTransaction {
    sqlite3* db;bool done=false;
    explicit StoreTransaction(sqlite3* d):db(d){need(sqlite3_exec(db,"BEGIN IMMEDIATE",nullptr,nullptr,nullptr)==SQLITE_OK);}
    void commit(){need(sqlite3_exec(db,"COMMIT",nullptr,nullptr,nullptr)==SQLITE_OK);done=true;}
    ~StoreTransaction(){if(!done)sqlite3_exec(db,"ROLLBACK",nullptr,nullptr,nullptr);}
};
std::string store_hash(std::string_view text){storage::Sha256 h;h.update(reinterpret_cast<const uint8_t*>(text.data()),text.size());return h.finish();}
void store_blob(StoreQuery& q,int index,std::string_view value){
    need(value.size()<=16384);need(sqlite3_bind_blob(q.p,index,value.data(),static_cast<int>(value.size()),SQLITE_TRANSIENT)==SQLITE_OK);
}
std::string store_blob(StoreQuery& q,int index){
    need(sqlite3_column_type(q.p,index)==SQLITE_BLOB);const auto size=sqlite3_column_bytes(q.p,index);
    need(size>0&&size<=16384);return {static_cast<const char*>(sqlite3_column_blob(q.p,index)),static_cast<size_t>(size)};
}
}
std::string StoreJournalV3::configuration()const{
    std::string body="PIXAURA_STORE_CONFIGURATION_V3\n"+catalog_.application+'\n'+std::to_string(catalog_.store)+'\n'+std::to_string(catalog_.environment)+'\n'+authority_.pin()+'\n';
    for(const auto& p:catalog_.products)body+=p.id+'\t'+std::to_string(p.kind)+'\n';
    need(body.size()<=2048);return body;
}
std::string StoreJournalV3::authenticate(std::string_view domain,std::string_view body)const{
    std::string envelope(domain);envelope+='\n';envelope+=configuration_digest_;envelope+='\n';envelope+=body;
    need(envelope.size()<=4096);return std::string(key_.authenticate(envelope));
}
std::string StoreJournalV3::encode(const Entry& e,bool with_ack)const{
    const auto& v=e.value;
    auto out=std::to_string(e.index)+'\t'+std::to_string(v.kind)+'\t'+std::to_string(v.status)+'\t'+std::to_string(v.signed_at)+'\t'+std::to_string(e.fraction)+'\t'+std::to_string(v.expires)+'\t'+v.transaction_digest+'\t'+std::to_string(e.native_time)+'\t'+(e.uncertain?"1":"0");
    if(with_ack)out+='\t'+e.proof_digest+'\t'+e.ack;
    return out+'\n';
}
StoreJournalV3::Entry StoreJournalV3::decode(std::string_view body)const{
    need(!body.empty()&&body.size()<=1024&&body.back()=='\n');body.remove_suffix(1);
    std::vector<std::string_view> fields;size_t start=0;
    for(size_t i=0;i<body.size();++i)if(body[i]=='\t'){fields.push_back(body.substr(start,i-start));start=i+1;}
    fields.push_back(body.substr(start));need(fields.size()==11);
    const auto number=[&](size_t i){int64_t n=0;const auto f=fields.at(i);const auto r=std::from_chars(f.data(),f.data()+f.size(),n);need(!f.empty()&&r.ec==std::errc{}&&r.ptr==f.data()+f.size()&&std::to_string(n)==f);return n;};
    Entry e;const auto index=number(0),kind=number(1),status=number(2),fraction=number(4),uncertain=number(8);
    need(index>=0&&static_cast<size_t>(index)<catalog_.products.size()&&kind>=1&&kind<=2&&status>=1&&status<=3&&fraction>=0&&fraction<1000&&(uncertain==0||uncertain==1));
    e.index=static_cast<unsigned>(index);e.value={catalog_.products[e.index].id,std::string(fields[6]),static_cast<uint32_t>(kind),static_cast<uint32_t>(status),number(3),number(5)};
    e.fraction=static_cast<uint32_t>(fraction);e.native_time=number(7);e.uncertain=uncertain!=0;e.proof_digest=fields[9];e.ack=fields[10];
    store_v2::observation(catalog_,e.value);need(e.native_time>=0&&e.native_time<=store_maximum_time&&e.proof_digest.size()==64&&e.ack.size()==64);
    need(store_v2::same_seal(e.ack,authenticate("PIXAURA_STORE_ACK_V3",encode(e,false))));
    return e;
}
std::string StoreJournalV3::manifest(const std::map<unsigned,Entry>& entries)const{
    std::string out="PIXAURA_STORE_MANIFEST_V3\n";
    for(const auto& pair:entries)out+=encode(pair.second);
    need(out.size()<=3584);return out;
}
std::map<unsigned,StoreJournalV3::Entry> StoreJournalV3::read(){
    StoreQuery config(ledger_.db_,"SELECT body,seal FROM store_config_v3 WHERE singleton=1");
    need(config.row()&&config.s(0)==configuration_&&store_v2::same_seal(config.s(1),authenticate("PIXAURA_STORE_CONFIG_V3",configuration_))&&!config.row());
    StoreQuery row(ledger_.db_,"SELECT body,seal FROM store_manifest_v3 WHERE singleton=1");need(row.row());const auto body=row.s(0);
    need(body.size()<=3584&&store_v2::same_seal(row.s(1),authenticate("PIXAURA_STORE_MANIFEST_SEAL_V3",body))&&!row.row());
    const std::string prefix="PIXAURA_STORE_MANIFEST_V3\n";need(body.substr(0,prefix.size())==prefix);
    std::map<unsigned,Entry> entries;size_t at=prefix.size();
    while(at<body.size()){
        const auto end=body.find('\n',at);need(end!=std::string::npos&&entries.size()<8);
        auto e=decode(std::string_view(body).substr(at,end-at+1));need(entries.emplace(e.index,e).second);at=end+1;
    }
    need(manifest(entries)==body);return entries;
}
StoreJournalV3::StoreJournalV3(Ledger& ledger,storage::ReceiptAuthorityV2& key,StoreProofAuthorityV3& authority):ledger_(ledger),key_(key),authority_(authority),catalog_(authority.catalog()){
    std::sort(catalog_.products.begin(),catalog_.products.end(),[](const auto& a,const auto& b){return a.id<b.id;});
    configuration_=configuration();configuration_digest_=store_hash(configuration_);
    std::lock_guard<std::mutex> lock(ledger_.mutex_);need(ledger_.format_version_==3&&ledger_.clock_authority_==&key_);
    StoreTransaction tx(ledger_.db_);StoreQuery config(ledger_.db_,"SELECT count(*) FROM store_config_v3");need(config.row());
    if(config.n(0)==0){
        StoreQuery remaining(ledger_.db_,"SELECT (SELECT count(*) FROM store_manifest_v3)+(SELECT count(*) FROM store_fact_v3)");need(remaining.row()&&remaining.n(0)==0);
        StoreQuery put(ledger_.db_,"INSERT INTO store_config_v3 VALUES(1,?,?)");put.text(1,configuration_);put.text(2,authenticate("PIXAURA_STORE_CONFIG_V3",configuration_));need(!put.row());
        const auto body=manifest({});StoreQuery initial(ledger_.db_,"INSERT INTO store_manifest_v3 VALUES(1,?,?)");initial.text(1,body);initial.text(2,authenticate("PIXAURA_STORE_MANIFEST_SEAL_V3",body));need(!initial.row());
    }
    (void)read();tx.commit();
}
StoreAcknowledgmentV3 StoreJournalV3::accept(const VerifiedStoreProofV3& proof,const Clock& clock){
    need(authority_.owns(proof)&&proof.origin()==StoreProofOriginV3::fresh);
    const auto& value=proof.observation();store_v2::observation(catalog_,value);
    unsigned index=0;while(index<catalog_.products.size()&&catalog_.products[index].id!=value.product)++index;need(index<catalog_.products.size());
    std::lock_guard<std::mutex> lock(ledger_.mutex_);StoreTransaction tx(ledger_.db_);auto entries=read();
    auto os=clock;os.trusted=false;const auto now=ledger_.tick(os);
    Entry e;e.index=index;e.value=value;e.fraction=proof.signed_millisecond();e.native_time=now;e.uncertain=ledger_.value("uncertain")!=0;e.proof_digest=proof.proof_digest();
    bool replay=false;const auto old=entries.find(index);
    if(old!=entries.end()){
        const auto& before=old->second;const auto a=std::make_pair(value.signed_at,e.fraction),b=std::make_pair(before.value.signed_at,before.fraction);need(a>=b);
        if(a==b){
            if(store_v2::canonical(catalog_,value)==store_v2::canonical(catalog_,before.value)){
                e.native_time=before.native_time;e.uncertain=before.uncertain;replay=true;
            }else{
                // An authenticated more restrictive observation for the SAME
                // transaction wins a timestamp tie. The reverse can never
                // reactivate access; ambiguous renewal/identity ties still fail.
                need(value.transaction_digest==before.value.transaction_digest&&value.status>before.value.status);
            }
        }
    }
    e.ack=authenticate("PIXAURA_STORE_ACK_V3",encode(e,false));entries[index]=e;
    const auto body=encode(e);StoreQuery put(ledger_.db_,"INSERT INTO store_fact_v3 VALUES(?,?,?,?,?) ON CONFLICT(product) DO UPDATE SET body=excluded.body,proof=excluded.proof,ack=excluded.ack,seal=excluded.seal");
    put.text(1,value.product);put.text(2,body);store_blob(put,3,proof.raw_proof());put.text(4,e.ack);put.text(5,authenticate("PIXAURA_STORE_FACT_V3",body));need(!put.row());
    const auto all=manifest(entries);StoreQuery seal(ledger_.db_,"UPDATE store_manifest_v3 SET body=?,seal=? WHERE singleton=1");seal.text(1,all);seal.text(2,authenticate("PIXAURA_STORE_MANIFEST_SEAL_V3",all));need(!seal.row());
    tx.commit();return {value.product,e.ack,e.proof_digest,replay};
}
std::map<std::string,Entitlement> StoreJournalV3::restore(){
    std::lock_guard<std::mutex> lock(ledger_.mutex_);const auto entries=read();std::map<std::string,Entitlement> out;
    for(const auto& pair:entries){const auto& e=pair.second;
        try{
            StoreQuery row(ledger_.db_,"SELECT body,proof,ack,seal FROM store_fact_v3 WHERE product=?");row.text(1,e.value.product);need(row.row());
            const auto body=encode(e);need(row.s(0)==body&&row.s(2)==e.ack&&store_v2::same_seal(row.s(3),authenticate("PIXAURA_STORE_FACT_V3",body)));
            const auto raw=store_blob(row,1);need(!row.row()&&store_hash(raw)==e.proof_digest);
            const auto verified=authority_.verify(raw,StoreProofOriginV3::cached);
            need(authority_.owns(verified)&&verified.signed_millisecond()==e.fraction&&store_v2::canonical(catalog_,verified.observation())==store_v2::canonical(catalog_,e.value));
            out.emplace(e.value.product,Entitlement{e.value.kind==2?Entitlement::lifetime:Entitlement::subscription,true,e.value.status!=1,e.value.expires,true});
        }catch(const document::Failure&){/* This product fails closed; other proofs remain independent. */}
        catch(const std::runtime_error&){/* Retain authenticated high-water; never repair from a stale cache. */}
    }
    return out;
}
}
