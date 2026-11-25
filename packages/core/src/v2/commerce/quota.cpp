#include "quota.hpp"
#include "../../publication_v2.hpp"
#include "../../../vendor/sqlite/sqlite3.h"
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <initializer_list>
#include <utility>
namespace pixaura::commerce {
namespace {
constexpr int64_t maximum_time=std::numeric_limits<int64_t>::max()-366LL*86400;
void need(bool ok){if(!ok)throw std::runtime_error("commercial state unavailable; recovery remains available");}
struct Query {
    sqlite3_stmt* p=nullptr;
    Query(sqlite3* db,const char* sql){need(sqlite3_prepare_v2(db,sql,-1,&p,nullptr)==SQLITE_OK);}
    ~Query(){sqlite3_finalize(p);}
    void text(int n,const std::string& s){need(sqlite3_bind_text(p,n,s.c_str(),int(s.size()),SQLITE_TRANSIENT)==SQLITE_OK);}
    void number(int n,int64_t v){need(sqlite3_bind_int64(p,n,v)==SQLITE_OK);}
    bool row(){const int rc=sqlite3_step(p);need(rc==SQLITE_ROW||rc==SQLITE_DONE);return rc==SQLITE_ROW;}
    int64_t n(int i){need(sqlite3_column_type(p,i)==SQLITE_INTEGER);return sqlite3_column_int64(p,i);}
    std::string s(int i){need(sqlite3_column_type(p,i)==SQLITE_TEXT);const auto* v=sqlite3_column_text(p,i);return v?std::string(reinterpret_cast<const char*>(v),std::size_t(sqlite3_column_bytes(p,i))):std::string{};}
};
// Validate recognized columns before journal changes can write to a foreign file.
void columns(sqlite3* db,const char* table,std::initializer_list<std::pair<const char*,const char*>> expected){
    const auto sql=std::string("PRAGMA table_info(")+table+")";
    Query q(db,sql.c_str());
    for(const auto& field:expected)need(q.row()&&q.s(1)==field.first&&q.s(2)==field.second);
    need(!q.row());
}
void recognized_schema(sqlite3* db,int64_t version){
    Query tables(db,"SELECT name FROM sqlite_schema WHERE name NOT GLOB 'sqlite_*' AND type='table' ORDER BY name");
    const auto names=version==1?std::initializer_list<const char*>{"clock","format","photos","request","session"}:
        version==2?std::initializer_list<const char*>{"clock","clock_auth_v2","format","native_trial_origin","photos","request","session"}:
        std::initializer_list<const char*>{"clock","clock_auth_v2","format","native_trial_origin","photos","request","session","store_config_v3","store_fact_v3","store_manifest_v3"};
    for(const auto* name:names)need(tables.row()&&tables.s(0)==name);
    need(!tables.row());
    columns(db,"format",{{"version","INTEGER"},{"magic","TEXT"}});
    columns(db,"clock",{{"k","TEXT"},{"v","INTEGER"}});
    columns(db,"session",{{"boot","TEXT"}});
    columns(db,"photos",{{"day","INTEGER"},{"source","TEXT"}});
    columns(db,"request",{{"token","TEXT"},{"request_id","TEXT"},{"day","INTEGER"},{"source","TEXT"},{"ai","INTEGER"},{"charged","INTEGER"},{"done","INTEGER"},{"payload","TEXT"}});
    if(version>=2){
        columns(db,"native_trial_origin",{{"singleton","INTEGER"},{"event","TEXT"},{"logical_utc","INTEGER"},{"source","TEXT"},{"project","TEXT"},{"document","TEXT"},{"revision","TEXT"},{"receipt_digest","TEXT"},{"uncertain","INTEGER"},{"seal","TEXT"}});
        columns(db,"clock_auth_v2",{{"singleton","INTEGER"},{"seal","TEXT"}});
    }
    if(version>=3){
        columns(db,"store_config_v3",{{"singleton","INTEGER"},{"body","TEXT"},{"seal","TEXT"}});
        columns(db,"store_manifest_v3",{{"singleton","INTEGER"},{"body","TEXT"},{"seal","TEXT"}});
        columns(db,"store_fact_v3",{{"product","TEXT"},{"body","TEXT"},{"proof","BLOB"},{"ack","TEXT"},{"seal","TEXT"}});
    }
}
bool hex(const std::string& s,std::size_t n){return s.size()==n&&std::all_of(s.begin(),s.end(),[](char c){return (c>='0'&&c<='9')||(c>='a'&&c<='f');});}
bool valid_token(const std::string& s){const auto at=s.find(':');return at>=1&&at<=19&&s.size()==at+33&&std::all_of(s.begin(),s.begin()+at,[](char c){return c>='0'&&c<='9';})&&hex(s.substr(at+1),32);}

struct Transaction {sqlite3* db;bool committed=false;explicit Transaction(sqlite3* d):db(d){need(sqlite3_exec(db,"BEGIN IMMEDIATE",nullptr,nullptr,nullptr)==SQLITE_OK);}void commit(){need(sqlite3_exec(db,"COMMIT",nullptr,nullptr,nullptr)==SQLITE_OK);committed=true;}~Transaction(){if(!committed)sqlite3_exec(db,"ROLLBACK",nullptr,nullptr,nullptr);}};
}
void Ledger::exec(const char* sql){need(sqlite3_exec(db_,sql,nullptr,nullptr,nullptr)==SQLITE_OK);}
int64_t Ledger::value(const char* key){Query q(db_,"SELECT v FROM clock WHERE k=?");q.text(1,key);need(q.row());return q.n(0);}
void Ledger::value(const char* key,int64_t v){Query q(db_,"INSERT INTO clock(k,v) VALUES(?,?) ON CONFLICT(k) DO UPDATE SET v=excluded.v");q.text(1,key);q.number(2,v);q.row();}
Ledger::Ledger(const std::string& path):Ledger(path,1){}
Ledger::Ledger(const std::string& path,uint32_t maximum_format,storage::ReceiptAuthorityV2* authority):clock_authority_(authority){
    need(maximum_format>=1&&maximum_format<=3);need(maximum_format==1||authority);trial_verified_=maximum_format==1;
    try {
        need(!path.empty()&&path!=":memory:"&&path.rfind("file:",0)!=0&&path.size()<4096&&path.find('\0')==std::string::npos);
        need(sqlite3_open_v2(path.c_str(),&db_,SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_FULLMUTEX|SQLITE_OPEN_NOFOLLOW,nullptr)==SQLITE_OK);
        sqlite3_busy_timeout(db_,100);
        // Read-only recognition BEFORE any journal/schema change. A mistaken
        // path must never mutate an existing project or unrelated database.
        exec("PRAGMA trusted_schema=OFF;");
        {
            Query objects(db_,"SELECT count(*) FROM sqlite_schema WHERE name NOT GLOB 'sqlite_*'");need(objects.row());
            if(objects.n(0)){
                need(objects.n(0)==5||(maximum_format>=2&&objects.n(0)==7)||(maximum_format>=3&&objects.n(0)==10));
                Query known(db_,"SELECT count(*) FROM sqlite_schema WHERE type='table' AND name IN('format','clock','session','photos','request','native_trial_origin','clock_auth_v2','store_config_v3','store_manifest_v3','store_fact_v3')");need(known.row()&&known.n(0)==objects.n(0));
                Query marker(db_,"SELECT version,magic FROM format");need(marker.row());const auto found=marker.n(0);need(found>=1&&found<=maximum_format&&marker.s(1)==(found==1?"PIXAURA_COMMERCIAL_V1":found==2?"PIXAURA_COMMERCIAL_V2":"PIXAURA_COMMERCIAL_V3")&&!marker.row());need(objects.n(0)==(found==1?5:found==2?7:10));recognized_schema(db_,found);
            }
        }
        exec("PRAGMA trusted_schema=OFF;PRAGMA journal_mode=DELETE;PRAGMA synchronous=FULL;PRAGMA secure_delete=ON;PRAGMA locking_mode=EXCLUSIVE;");
        Transaction tx(db_);
        exec("CREATE TABLE IF NOT EXISTS format(version INTEGER NOT NULL CHECK(version=1),magic TEXT NOT NULL CHECK(magic='PIXAURA_COMMERCIAL_V1'));INSERT INTO format SELECT 1,'PIXAURA_COMMERCIAL_V1' WHERE NOT EXISTS(SELECT 1 FROM format);CREATE TABLE IF NOT EXISTS clock(k TEXT PRIMARY KEY,v INTEGER NOT NULL);CREATE TABLE IF NOT EXISTS session(boot TEXT NOT NULL);CREATE TABLE IF NOT EXISTS photos(day INTEGER NOT NULL,source TEXT NOT NULL,PRIMARY KEY(day,source));CREATE TABLE IF NOT EXISTS request(token TEXT PRIMARY KEY,request_id TEXT NOT NULL UNIQUE,day INTEGER NOT NULL,source TEXT NOT NULL,ai INTEGER NOT NULL,charged INTEGER NOT NULL,done INTEGER NOT NULL DEFAULT 0,payload TEXT NOT NULL DEFAULT '');");
        {Query version(db_,"SELECT version FROM format");need(version.row());format_version_=static_cast<uint32_t>(version.n(0));need(format_version_>=1&&format_version_<=maximum_format&&!version.row());}
        bool upgraded=false;
        if(format_version_==1&&maximum_format>=2){
            upgraded=true;
            exec("ALTER TABLE format RENAME TO old_commercial_format;CREATE TABLE format(version INTEGER NOT NULL CHECK(version=2),magic TEXT NOT NULL CHECK(magic='PIXAURA_COMMERCIAL_V2'));INSERT INTO format VALUES(2,'PIXAURA_COMMERCIAL_V2');DROP TABLE old_commercial_format;CREATE TABLE native_trial_origin(singleton INTEGER PRIMARY KEY CHECK(singleton=1),event TEXT NOT NULL CHECK(length(event)=32),logical_utc INTEGER NOT NULL CHECK(logical_utc>=0),source TEXT NOT NULL CHECK(length(source)=64),project TEXT NOT NULL CHECK(length(project)=32),document TEXT NOT NULL CHECK(length(document)=32),revision TEXT NOT NULL CHECK(length(revision)=32),receipt_digest TEXT NOT NULL CHECK(length(receipt_digest)=64),uncertain INTEGER NOT NULL CHECK(uncertain IN(0,1)),seal TEXT NOT NULL CHECK(length(seal)=64)) STRICT;CREATE TABLE clock_auth_v2(singleton INTEGER PRIMARY KEY CHECK(singleton=1),seal TEXT NOT NULL CHECK(length(seal)=64)) STRICT;");format_version_=2;
        }
        // Exclusive connection ownership means a previous process cannot still
        // own these reservations. Unsuccessful requests never survive as charges.
        if(format_version_>=2){if(upgraded)seal_clock();else verify_clock();}
        if(format_version_==2&&maximum_format>=3)migrate_store_format();
        exec("DELETE FROM request WHERE done=0;");tx.commit();
    }catch(...){if(db_)sqlite3_close_v2(db_);db_=nullptr;throw;}
}
void Ledger::migrate_store_format(){
    // Caller owns the same SQLite transaction; no additional connection or
    // editing authority. Authenticate old clock/trial state before any DDL.
    need(format_version_==2);verify_clock();
            exec("ALTER TABLE format RENAME TO old_commercial_format;CREATE TABLE format(version INTEGER NOT NULL CHECK(version=3),magic TEXT NOT NULL CHECK(magic='PIXAURA_COMMERCIAL_V3'));INSERT INTO format VALUES(3,'PIXAURA_COMMERCIAL_V3');DROP TABLE old_commercial_format;CREATE TABLE store_config_v3(singleton INTEGER PRIMARY KEY CHECK(singleton=1),body TEXT NOT NULL CHECK(length(body)<=2048),seal TEXT NOT NULL CHECK(length(seal)=64)) STRICT;CREATE TABLE store_manifest_v3(singleton INTEGER PRIMARY KEY CHECK(singleton=1),body TEXT NOT NULL CHECK(length(body)<=3584),seal TEXT NOT NULL CHECK(length(seal)=64)) STRICT;CREATE TABLE store_fact_v3(product TEXT PRIMARY KEY CHECK(length(product) BETWEEN 1 AND 128),body TEXT NOT NULL CHECK(length(body)<=1024),proof BLOB NOT NULL CHECK(length(proof)<=16384),ack TEXT NOT NULL CHECK(length(ack)=64),seal TEXT NOT NULL CHECK(length(seal)=64)) STRICT;");
    format_version_=3;
}
Ledger::~Ledger(){if(db_)sqlite3_close_v2(db_);}
std::string Ledger::clock_canonical(){
    Query rows(db_,"SELECT count(*) FROM clock");need(rows.row());if(rows.n(0)==0){Query sessions(db_,"SELECT count(*) FROM session");need(sessions.row()&&sessions.n(0)==0);return {};}
    need(rows.n(0)==4);Query session(db_,"SELECT boot FROM session");need(session.row());const auto boot=session.s(0);need(hex(boot,32)&&!session.row());
    const auto utc=value("utc"),mono=value("mono"),trial=value("trial"),uncertain=value("uncertain");
    need(utc>=0&&utc<=maximum_time&&mono>=0&&(trial==-1||(trial>=0&&trial<=maximum_time))&&(uncertain==0||uncertain==1));
    return "PIXAURA_NATIVE_CLOCK_V2:"+std::to_string(utc)+":"+std::to_string(mono)+":"+std::to_string(trial)+":"+std::to_string(uncertain)+":"+boot;
}
void Ledger::verify_clock(){
    if(format_version_<2)return;need(clock_authority_);const auto canonical=clock_canonical();Query row(db_,"SELECT seal FROM clock_auth_v2 WHERE singleton=1");
    if(canonical.empty()){need(!row.row());return;}if(!row.row())throw document::Failure{6};const auto actual=row.s(0);if(!hex(actual,64)||row.row())throw document::Failure{6};
    const auto expected=clock_authority_->authenticate(canonical);if(expected.size()!=64)throw document::Failure{6};unsigned difference=0;for(size_t i=0;i<64;++i)difference|=static_cast<unsigned char>(actual[i])^static_cast<unsigned char>(expected[i]);if(difference)throw document::Failure{6};
}
void Ledger::seal_clock(){
    if(format_version_<2)return;need(clock_authority_);const auto canonical=clock_canonical();if(canonical.empty()){Query row(db_,"SELECT count(*) FROM clock_auth_v2");need(row.row()&&row.n(0)==0);return;}
    const auto seal=clock_authority_->authenticate(canonical);need(hex(std::string(seal),64));Query row(db_,"INSERT INTO clock_auth_v2 VALUES(1,?) ON CONFLICT(singleton) DO UPDATE SET seal=excluded.seal");row.text(1,std::string(seal));need(!row.row());
}
int64_t Ledger::tick(const Clock& c){
    verify_clock();
    need(c.utc_seconds>=0&&c.utc_seconds<=maximum_time&&c.uptime_seconds>=0&&hex(c.boot,32));
    Query exists(db_,"SELECT count(*) FROM clock");need(exists.row());
    if(!exists.n(0)){value("utc",c.utc_seconds);value("mono",c.uptime_seconds);value("uncertain",0);value("trial",-1);Query q(db_,"INSERT INTO session VALUES(?)");q.text(1,c.boot);q.row();seal_clock();return c.utc_seconds;}
    need(exists.n(0)==4);
    Query session(db_,"SELECT boot FROM session");need(session.row());const bool same=session.s(0)==c.boot;need(!session.row());int64_t utc=value("utc"),mono=value("mono");
    const auto trial=value("trial"),uncertain=value("uncertain");
    need(utc>=0&&utc<=maximum_time&&mono>=0&&(trial==-1||(trial>=0&&trial<=maximum_time))&&(uncertain==0||uncertain==1));
    if(same&&c.uptime_seconds>=mono){const auto delta=c.uptime_seconds-mono;need(delta<=maximum_time-utc);utc+=delta;}
    else {value("uncertain",1);Query q(db_,"UPDATE session SET boot=?");q.text(1,c.boot);q.row();}
    if(c.trusted&&c.utc_seconds>=utc){utc=c.utc_seconds;value("uncertain",0);}
    value("utc",utc);value("mono",c.uptime_seconds);
    // Acknowledged old-day deliveries no longer need local event history.
    Query clean(db_,"DELETE FROM request WHERE day<? AND done=1 AND payload=''");clean.number(1,utc/86400);clean.row();
    Query photos(db_,"DELETE FROM photos WHERE day<?");photos.number(1,utc/86400);photos.row();seal_clock();return utc;
}
int Ledger::access(int64_t now){
    bool subscription=false;
    for(const auto& entry:paid_){const auto& paid=entry.second;
        if(paid.verified&&!paid.revoked){if(paid.kind==Entitlement::lifetime)return 3;if(paid.kind==Entitlement::subscription&&!value("uncertain")&&paid.expires>=0&&paid.expires<=maximum_time&&(now<paid.expires||(paid.offline&&now<paid.expires+7*86400)))subscription=true;}
    }
    if(subscription)return 2;
    const auto start=value("trial");return trial_verified_&&start>=0&&now>=start&&!value("uncertain")&&now<start+14*86400?1:0;
}
bool Ledger::unlimited(int64_t now){return access(now)!=0;}

void Ledger::verified_store_fact(const std::string& product,const Entitlement& e){
    std::lock_guard<std::mutex> lock(mutex_);
    if(!e.verified)return; // A verification failure never erases an existing verified lifetime fact.
    need(!product.empty()&&product.size()<=128&&(paid_.count(product)||paid_.size()<8));
    paid_[product]=e;
}
Admission Ledger::reserve(const std::string& id,const std::string& source,bool ai,const Clock& c){
    if(!hex(id,32)||!hex(source,64))return {Admission::invalid,{}};
    std::lock_guard<std::mutex> lock(mutex_);Transaction tx(db_);const auto now=tick(c),day=now/86400;const std::string token=std::to_string(day)+":"+id;
    Query prior(db_,"SELECT token,source,ai FROM request WHERE request_id=?");prior.text(1,id);
    if(prior.row()){need(prior.s(1)==source&&prior.n(2)==int(ai));const auto existing=prior.s(0);tx.commit();return {Admission::allowed,existing};}
    Query all(db_,"SELECT count(*) FROM request WHERE ai=? AND (done=0 OR payload<>'')");all.number(1,ai);need(all.row());if(all.n(0)>=64){tx.commit();return {Admission::busy,{}};}
    const bool charge=!unlimited(now);
    if(charge){
        Query photo(db_,"SELECT count(*) FROM photos WHERE day=? AND source=?");photo.number(1,day);photo.text(2,source);need(photo.row());const bool seen=photo.n(0)!=0;
        if(!seen){
            Query pending(db_,"SELECT count(*) FROM request WHERE source=? AND done=0 AND charged=1");pending.text(1,source);need(pending.row());if(pending.n(0)){tx.commit();return {Admission::busy,{}};}
            Query count(db_,"SELECT count(*) FROM (SELECT source FROM photos WHERE day=? UNION SELECT source FROM request WHERE done=0 AND charged=1)");count.number(1,day);need(count.row());if(count.n(0)>=5){tx.commit();return {Admission::photo_limit,{}};}
        }
        if(ai){Query count(db_,"SELECT count(*) FROM request WHERE ai=1 AND charged=1 AND ((day=? AND done=1) OR done=0)");count.number(1,day);need(count.row());if(count.n(0)>=3){tx.commit();return {Admission::ai_limit,{}};}}

    }
    Query add(db_,"INSERT INTO request(token,request_id,day,source,ai,charged) VALUES(?,?,?,?,?,?)");add.text(1,token);add.text(2,id);add.number(3,day);add.text(4,source);add.number(5,ai);add.number(6,charge);add.row();tx.commit();return {Admission::allowed,token};
}
bool Ledger::complete(const std::string& token,bool success,bool edit,const std::string& proposal,const Clock& c){
    if(!valid_token(token)|| (edit&&format_version_!=1))return false;
    std::lock_guard<std::mutex> lock(mutex_);Transaction tx(db_);const auto now=tick(c);
    Query q(db_,"SELECT day,source,ai,charged,done FROM request WHERE token=?");q.text(1,token);if(!q.row()){tx.commit();return false;}
    if(q.n(4)){tx.commit();return true;}
    const bool ai=q.n(2)!=0;
    if(!success||(ai&&edit)||(ai&&(proposal.empty()||proposal.size()>8192))){Query remove(db_,"DELETE FROM request WHERE token=?");remove.text(1,token);remove.row();tx.commit();return false;}
    if(q.n(3)){Query photo(db_,"INSERT OR IGNORE INTO photos VALUES(?,?)");photo.number(1,now/86400);photo.text(2,q.s(1));photo.row();}
    Query update(db_,"UPDATE request SET done=1,payload=?,day=? WHERE token=?");update.text(1,ai?proposal:std::string{});update.number(2,now/86400);update.text(3,token);update.row();
    if(edit&&value("trial")<0)value("trial",now);
    if(!ai){Query remove(db_,"DELETE FROM request WHERE token=?");remove.text(1,token);remove.row();}
    tx.commit();return true;
}
std::string Ledger::recover_delivery(const std::string& token){if(!valid_token(token))return {};std::lock_guard<std::mutex> lock(mutex_);Query q(db_,"SELECT payload FROM request WHERE token=? AND done=1");q.text(1,token);return q.row()?q.s(0):std::string{};}
void Ledger::acknowledge_delivery(const std::string& token){if(!valid_token(token))return;std::lock_guard<std::mutex> lock(mutex_);Transaction tx(db_);Query q(db_,"UPDATE request SET payload='' WHERE token=? AND done=1");q.text(1,token);q.row();Query clean(db_,"DELETE FROM request WHERE token=? AND done=1 AND charged=0");clean.text(1,token);clean.row();tx.commit();}
State Ledger::state(const Clock& c){std::lock_guard<std::mutex> lock(mutex_);Transaction tx(db_);State s;const auto utc=tick(c);s.logical_utc=utc;s.access=access(utc);s.day=utc/86400;s.trial_start=trial_verified_?value("trial"):-1;s.trial_end=s.trial_start<0?-1:s.trial_start+14*86400;s.clock_uncertain=value("uncertain")!=0;Query p(db_,"SELECT count(*) FROM photos WHERE day=?");p.number(1,s.day);need(p.row());s.photos=int(p.n(0));Query a(db_,"SELECT count(*) FROM request WHERE day=? AND ai=1 AND done=1 AND charged=1");a.number(1,s.day);need(a.row());s.ai=int(a.n(0));Query pending(db_,"SELECT count(*) FROM request WHERE done=0");need(pending.row());s.pending=int(pending.n(0));tx.commit();return s;}
}

