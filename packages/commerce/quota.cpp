#include "quota.hpp"
#include "../core/vendor/sqlite/sqlite3.h"
#include <algorithm>
#include <stdexcept>
#include <limits>
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
bool hex(const std::string& s,std::size_t n){return s.size()==n&&std::all_of(s.begin(),s.end(),[](char c){return (c>='0'&&c<='9')||(c>='a'&&c<='f');});}
bool valid_token(const std::string& s){const auto at=s.find(':');return at>=1&&at<=19&&s.size()==at+33&&std::all_of(s.begin(),s.begin()+at,[](char c){return c>='0'&&c<='9';})&&hex(s.substr(at+1),32);}

struct Transaction {sqlite3* db;bool committed=false;explicit Transaction(sqlite3* d):db(d){need(sqlite3_exec(db,"BEGIN IMMEDIATE",nullptr,nullptr,nullptr)==SQLITE_OK);}void commit(){need(sqlite3_exec(db,"COMMIT",nullptr,nullptr,nullptr)==SQLITE_OK);committed=true;}~Transaction(){if(!committed)sqlite3_exec(db,"ROLLBACK",nullptr,nullptr,nullptr);}};
}
void Ledger::exec(const char* sql){need(sqlite3_exec(db_,sql,nullptr,nullptr,nullptr)==SQLITE_OK);}
int64_t Ledger::value(const char* key){Query q(db_,"SELECT v FROM clock WHERE k=?");q.text(1,key);need(q.row());return q.n(0);}
void Ledger::value(const char* key,int64_t v){Query q(db_,"INSERT INTO clock(k,v) VALUES(?,?) ON CONFLICT(k) DO UPDATE SET v=excluded.v");q.text(1,key);q.number(2,v);q.row();}
Ledger::Ledger(const std::string& path){
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
                need(objects.n(0)==5);
                Query known(db_,"SELECT count(*) FROM sqlite_schema WHERE type='table' AND name IN('format','clock','session','photos','request')");need(known.row()&&known.n(0)==5);
                Query marker(db_,"SELECT version,magic FROM format");need(marker.row()&&marker.n(0)==1&&marker.s(1)=="PIXAURA_COMMERCIAL_V1"&&!marker.row());
            }
        }
        exec("PRAGMA trusted_schema=OFF;PRAGMA journal_mode=DELETE;PRAGMA synchronous=FULL;PRAGMA secure_delete=ON;PRAGMA locking_mode=EXCLUSIVE;");
        Transaction tx(db_);
        exec("CREATE TABLE IF NOT EXISTS format(version INTEGER NOT NULL CHECK(version=1),magic TEXT NOT NULL CHECK(magic='PIXAURA_COMMERCIAL_V1'));INSERT INTO format SELECT 1,'PIXAURA_COMMERCIAL_V1' WHERE NOT EXISTS(SELECT 1 FROM format);CREATE TABLE IF NOT EXISTS clock(k TEXT PRIMARY KEY,v INTEGER NOT NULL);CREATE TABLE IF NOT EXISTS session(boot TEXT NOT NULL);CREATE TABLE IF NOT EXISTS photos(day INTEGER NOT NULL,source TEXT NOT NULL,PRIMARY KEY(day,source));CREATE TABLE IF NOT EXISTS request(token TEXT PRIMARY KEY,request_id TEXT NOT NULL UNIQUE,day INTEGER NOT NULL,source TEXT NOT NULL,ai INTEGER NOT NULL,charged INTEGER NOT NULL,done INTEGER NOT NULL DEFAULT 0,payload TEXT NOT NULL DEFAULT '');");
        Query version(db_,"SELECT version FROM format");need(version.row()&&version.n(0)==1&&!version.row());
        // Exclusive connection ownership means a previous process cannot still
        // own these reservations. Unsuccessful requests never survive as charges.
        exec("DELETE FROM request WHERE done=0;");tx.commit();
    }catch(...){if(db_)sqlite3_close_v2(db_);db_=nullptr;throw;}
}
Ledger::~Ledger(){if(db_)sqlite3_close_v2(db_);}
int64_t Ledger::tick(const Clock& c){
    need(c.utc_seconds>=0&&c.utc_seconds<=maximum_time&&c.uptime_seconds>=0&&hex(c.boot,32));
    Query exists(db_,"SELECT count(*) FROM clock");need(exists.row());
    if(!exists.n(0)){value("utc",c.utc_seconds);value("mono",c.uptime_seconds);value("uncertain",0);value("trial",-1);Query q(db_,"INSERT INTO session VALUES(?)");q.text(1,c.boot);q.row();return c.utc_seconds;}
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
    Query photos(db_,"DELETE FROM photos WHERE day<?");photos.number(1,utc/86400);photos.row();return utc;
}
bool Ledger::unlimited(int64_t now){
    for(const auto& entry:paid_){const auto& paid=entry.second;
        if(paid.verified&&!paid.revoked){if(paid.kind==Entitlement::lifetime)return true;if(paid.kind==Entitlement::subscription&&!value("uncertain")&&paid.expires>=0&&paid.expires<=maximum_time&&(now<paid.expires||(paid.offline&&now<paid.expires+7*86400)))return true;}
    }
    const auto start=value("trial");return start>=0&&!value("uncertain")&&now<start+14*86400;
}
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
    if(!valid_token(token))return false;
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
State Ledger::state(const Clock& c){std::lock_guard<std::mutex> lock(mutex_);Transaction tx(db_);State s;const auto utc=tick(c);s.day=utc/86400;s.trial_start=value("trial");s.clock_uncertain=value("uncertain")!=0;Query p(db_,"SELECT count(*) FROM photos WHERE day=?");p.number(1,s.day);need(p.row());s.photos=int(p.n(0));Query a(db_,"SELECT count(*) FROM request WHERE day=? AND ai=1 AND done=1 AND charged=1");a.number(1,s.day);need(a.row());s.ai=int(a.n(0));Query pending(db_,"SELECT count(*) FROM request WHERE done=0");need(pending.row());s.pending=int(pending.n(0));tx.commit();return s;}
}
