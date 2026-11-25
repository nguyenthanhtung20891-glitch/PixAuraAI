#include "coordinator_v2.hpp"
#include "receipt_key_v2.hpp"
#include "../core/src/storage_files.hpp"
#include "../core/vendor/sqlite/sqlite3.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
using namespace pixaura;
namespace fs=std::filesystem;
#define CHECK(x) do{if(!(x))throw std::runtime_error("coordinator test line "+std::to_string(__LINE__));}while(false)
namespace pixaura::commerce {
struct CommercialCoordinatorV2TestAccess {
    static Admission reserve(CommercialCoordinatorV2& o,const std::string& id,const std::string& source,bool ai){return o.reserve(id,source,ai);}
    static bool finish(CommercialCoordinatorV2& o,const std::string& id,bool ok,const std::string& usable={}){return o.finish(id,ok,usable);}
    static Ledger& ledger(CommercialCoordinatorV2& o){return o.ledger_;}
};
struct LedgerTestAccess {
    static sqlite3* db(Ledger& l){return l.db_;}
    static unsigned format(Ledger& l){return l.format_version_;}
    static int64_t stored_trial(Ledger& l){return l.value("trial");}
    static void exec(Ledger& l,const char* sql){l.exec(sql);}
    static void upgrade_store(Ledger& l){l.migrate_store_format();}
    static void clock(Ledger& l,const char* key,int64_t value){l.value(key,value);}
};
}
struct OSClock:commerce::CommercialClockV2{
    commerce::Clock now{20000LL*86400+86399,100,std::string(32,'a'),false};
    commerce::Clock sample_os()override{return now;}
};
struct Input:storage::Reader{
    std::size_t left=128;
    std::size_t read(uint8_t* p,std::size_t n)override{n=std::min(n,left);std::memset(p,'x',n);left-=n;return n;}
};
template<class T>T good(document::Result<T> r){if(r.code)throw document::Failure{r.code};return std::move(r.value);}
static void prepare(const std::string& root,const char* fixture){
    CHECK(fs::create_directory(root));Input input;storage::AssetStore assets(root);auto source=assets.ingest(input,128);
    std::ifstream f(fixture);CHECK(f);std::string text{std::istreambuf_iterator<char>(f),{}};auto at=text.find(std::string(64,'a'));CHECK(at!=std::string::npos);text.replace(at,64,source.digest);
    storage::Repository repo(root,4);repo.create(*good(document::deserialize(text,"00000000000000000000000000000500")));repo.migrate(1,2);repo.migrate(2,3);repo.migrate(3,4);
}
static std::string edit(const storage::Loaded& loaded){const auto& doc=*loaded.snapshot;return "{\"command_version\":1,\"kind\":\"commit\",\"expected_revision_id\":\""+doc.current().text()+"\",\"expected_session_id\":\""+doc.session().id.text()+"\",\"expected_generation\":"+std::to_string(doc.session().generation)+",\"revision_id\":\"00000000000000000000000000000103\",\"actor\":\"manual\",\"plan_id\":null,\"operations\":[{\"id\":\"00000000000000000000000000000019\",\"type\":\"pixaura.exposure\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":1000}}],\"stack\":[\"00000000000000000000000000000019\"]}";}
int main(int argc,char** argv){try{
    CHECK(argc>=2);std::array<uint8_t,32> fixture_key{};for(unsigned i=0;i<32;++i)fixture_key[i]=static_cast<uint8_t>(i);commerce::ReceiptKeyV2 key(fixture_key.data(),fixture_key.size());OSClock clock;
    CHECK(key.authenticate("PIXAURA_RECEIPT_AUTHORITY_V2_KNOWN_ANSWER")=="7754f0c963f492263bb0e7422d7ec582f81f1d30c1e4d6cacc6827047306e3d6"); // independent Python hashlib.blake2b KAT
    if(std::string(argv[1])=="--prepare"){CHECK(argc==4);prepare(argv[2],argv[3]);return 0;}
    if(std::string(argv[1])=="--crash"){
        CHECK(argc==4);const std::string root=argv[2],point=argv[3];storage::Repository repo(root,4);commerce::CommercialCoordinatorV2 owner(root+"/commercial.sqlite",clock,key);
        if(point=="before_native")std::_Exit(74);
        const auto base=repo.read();(void)repo.publish_v2(edit(base),base.epoch,owner,key);
        if(point=="after_native")std::_Exit(74);
        int commits=0;
        if(point=="before_ack_commit")sqlite3_commit_hook(commerce::LedgerTestAccess::db(commerce::CommercialCoordinatorV2TestAccess::ledger(owner)),[](void* p)->int{if(++*static_cast<int*>(p)==2)std::_Exit(74);return 0;},&commits);
        CHECK(owner.reconcile(repo)==1);if(point=="after_ack")std::_Exit(74);CHECK(false);
    }
    if(std::string(argv[1])=="--store-format-crash"){
        CHECK(argc==4);commerce::Ledger ledger(std::string(argv[2])+"/commercial.sqlite",2,&key);
        commerce::LedgerTestAccess::exec(ledger,"BEGIN IMMEDIATE");commerce::LedgerTestAccess::upgrade_store(ledger);
        if(std::string(argv[3])=="before_commit")std::_Exit(75);
        CHECK(std::string(argv[3])=="after_commit");commerce::LedgerTestAccess::exec(ledger,"COMMIT");std::_Exit(75);
    }
    if(std::string(argv[1])=="--inspect-store-format"){
        CHECK(argc==4);const std::string root=argv[2],path=root+"/commercial.sqlite";
        const unsigned version=std::string(argv[3])=="2"?2u:3u;
        storage::Repository repository(root,4);const auto receipts=repository.receipts_v2(key);CHECK(receipts.size()==1);
        {commerce::Ledger ledger(path,version,&key);CHECK(commerce::LedgerTestAccess::format(ledger)==version);
         const auto state=ledger.state(clock.now);CHECK(state.trial_start==-1&&state.access==0&&state.ai==0&&state.pending==0);
         CHECK(commerce::LedgerTestAccess::stored_trial(ledger)==receipts[0].time().logical_utc);
         const auto snapshot=repository.read().snapshot;
         // Independently read frozen fixture contains 100/101/102; native edit adds 103.
         CHECK(snapshot->revisions().size()==4&&snapshot->current().text()==receipts[0].revision()&&snapshot->source().sha256.text()==receipts[0].source());}
        if(version==3){
            const auto read=[&]{std::ifstream input(path,std::ios::binary);return std::string{std::istreambuf_iterator<char>(input),{}};};
            const auto before=read();bool rejected=false;try{commerce::Ledger old(path,2,&key);}catch(...){rejected=true;}CHECK(rejected&&read()==before);
            auto altered=fixture_key;altered[1]^=1;commerce::ReceiptKeyV2 wrong(altered.data(),altered.size());rejected=false;
            try{commerce::Ledger bad(path,3,&wrong);}catch(...){rejected=true;}CHECK(rejected&&read()==before);
        }
        return 0;
    }
    if(std::string(argv[1])=="--inspect"){
        CHECK(argc==4);const std::string root=argv[2];const bool committed=std::string(argv[3])=="yes";storage::Repository repo(root,4);commerce::CommercialCoordinatorV2 owner(root+"/commercial.sqlite",clock,key);
        const auto rows=repo.receipts_v2(key);CHECK(rows.size()==(committed?1:0));const auto before=owner.state();const auto changes=owner.reconcile(repo);CHECK(changes==(committed&&before.trial_start<0?1u:0u));CHECK(owner.reconcile(repo)==0);
        CHECK(owner.state().trial_start==(committed?rows[0].time().logical_utc:-1));return 0;
    }
    const auto root=(fs::current_path()/("commerce-v2-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()))).string();prepare(root,argv[1]);storage::Repository repo(root,4);
    // Foreign databases with a copied marker must be rejected BEFORE changing
    // their WAL mode, even if their object count looks like a legacy ledger.
    for(unsigned variant=0;variant<2;++variant){
        const auto path=root+"/foreign-shape-"+std::to_string(variant)+".sqlite";sqlite3* db=nullptr;
        CHECK(sqlite3_open(path.c_str(),&db)==SQLITE_OK);
        const std::string sql="PRAGMA journal_mode=WAL;CREATE TABLE format(version INTEGER,magic TEXT);INSERT INTO format VALUES(1,'PIXAURA_COMMERCIAL_V1');CREATE TABLE clock(k TEXT PRIMARY KEY,v INTEGER);CREATE TABLE session(boot TEXT);"+
            std::string(variant==0?"CREATE TABLE store_config_v3(dummy INTEGER);CREATE TABLE request(token TEXT);":"CREATE TABLE photos(day INTEGER,source TEXT,PRIMARY KEY(day,source));CREATE TABLE request(token TEXT);");
        CHECK(sqlite3_exec(db,sql.c_str(),nullptr,nullptr,nullptr)==SQLITE_OK);CHECK(sqlite3_close(db)==SQLITE_OK);
        const auto read=[&]{std::ifstream input(path,std::ios::binary);return std::string{std::istreambuf_iterator<char>(input),{}};};const auto before=read();
        bool rejected=false;try{commerce::Ledger foreign(path,3,&key);}catch(...){rejected=true;}CHECK(rejected&&read()==before);
    }
    std::string source=repo.read().snapshot->source().sha256.text();int64_t start=0;
    {
        commerce::CommercialCoordinatorV2 owner(root+"/commercial.sqlite",clock,key);CHECK(owner.state().trial_start==-1);CHECK(owner.reconcile(repo)==0); // old Schema3 manual history never guesses a trial time
        using A=commerce::CommercialCoordinatorV2TestAccess;
        auto first=A::reserve(owner,std::string(32,'1'),source,false);CHECK(first.decision==commerce::Admission::allowed);CHECK(A::finish(owner,first.token,true));CHECK(owner.state().photos==1&&owner.state().trial_start==-1);
        auto base=repo.read();auto receipt=repo.publish_v2(edit(base),base.epoch,owner,key);start=receipt.time().logical_utc;CHECK(owner.state().trial_start==-1);CHECK(owner.reconcile(repo)==1&&owner.state().trial_start==start);CHECK(owner.reconcile(repo)==0);
        // A first valid native committed edit activates trial; five/three Free
        // quotas are bypassed without a store grant or extra photo/AI charge.
        for(unsigned i=0;i<12;++i){char id[33],different[65];std::snprintf(id,sizeof id,"%032x",100+i);std::snprintf(different,sizeof different,"%064x",100+i);auto admitted=A::reserve(owner,id,different,false);CHECK(admitted.decision==commerce::Admission::allowed);CHECK(A::finish(owner,admitted.token,true));}
        CHECK(owner.state().photos==1);
        clock.now.uptime_seconds+=15*86400;clock.now.utc_seconds+=15*86400;
        const auto expired=owner.state();CHECK(expired.access==0);
        auto& ledger=A::ledger(owner);commerce::LedgerTestAccess::clock(ledger,"utc",start+1);commerce::LedgerTestAccess::clock(ledger,"mono",clock.now.uptime_seconds);
        bool rejected_clock=false;try{(void)owner.state();}catch(const document::Failure& e){CHECK(e.code==6);rejected_clock=true;}
        CHECK(rejected_clock); // An authentic old edit proof must NOT authenticate a rewritten clock.
        commerce::LedgerTestAccess::clock(ledger,"utc",expired.logical_utc);commerce::LedgerTestAccess::clock(ledger,"mono",clock.now.uptime_seconds);
        CHECK(owner.state().access==0);
    }
    {
        commerce::CommercialCoordinatorV2 restored(root+"/commercial.sqlite",clock,key);CHECK(restored.state().trial_start==start&&restored.reconcile(repo)==0);
        clock.now.boot=std::string(32,'b');clock.now.uptime_seconds=1;clock.now.utc_seconds+=900*86400;CHECK(restored.state().clock_uncertain);
        CHECK(restored.state().trial_start==start); // neither reboot nor wall jump renews trial
        CHECK(restored.state().access==0&&commerce::CommercialCoordinatorV2::recovery_allowed());
        // A retained installation after reboot is not a fresh active trial.
        // Reservation-only synthetic test: no usable AI delivery is fabricated.
        using A=commerce::CommercialCoordinatorV2TestAccess;
        std::vector<std::string> pending;
        for(unsigned i=0;i<3;++i){char id[33],photo[65];std::snprintf(id,sizeof id,"%032x",900+i);std::snprintf(photo,sizeof photo,"%064x",900+i);auto reservation=A::reserve(restored,id,photo,true);CHECK(reservation.decision==commerce::Admission::allowed);pending.push_back(reservation.token);}
        CHECK(A::reserve(restored,std::string(32,'d'),std::string(64,'d'),true).decision==commerce::Admission::ai_limit);
        for(const auto& token:pending)CHECK(!A::finish(restored,token,false));
        CHECK(restored.state().pending==0&&restored.state().ai==0&&restored.state().trial_start==start&&restored.state().clock_uncertain);
        auto retry=A::reserve(restored,std::string(32,'d'),std::string(64,'d'),true);CHECK(retry.decision==commerce::Admission::allowed);CHECK(!A::finish(restored,retry.token,false));
    }
    {
        auto wrong=fixture_key;wrong[0]^=1;commerce::ReceiptKeyV2 bad(wrong.data(),wrong.size());bool rejected=false;try{commerce::CommercialCoordinatorV2 restored(root+"/commercial.sqlite",clock,bad);}catch(const document::Failure& e){CHECK(e.code==6);rejected=true;}CHECK(rejected);CHECK(commerce::CommercialCoordinatorV2::recovery_allowed());
    }
    // Legacy component cannot reinterpret version2 accounting or signed evidence.
    bool rejected=false;try{commerce::Ledger legacy(root+"/commercial.sqlite");}catch(...){rejected=true;}CHECK(rejected);
    std::puts("Keyed BLAKE2b independent KAT; import/history noTrial; postCOMMIT native proof; exactlyonce trial/reopen; quota bypass; reboot uncertainty; wrongkey/recovery PASS");return 0;
}catch(const document::Failure& e){std::fprintf(stderr,"native failure%d\n",e.code);return 1;}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}}
