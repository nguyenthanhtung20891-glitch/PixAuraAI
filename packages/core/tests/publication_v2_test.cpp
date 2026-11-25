#include "../src/storage.hpp"
#include "../src/storage_files.hpp"
#include "../src/sha256.hpp"
#include "../vendor/sqlite/sqlite3.h"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <future>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <stdexcept>
using namespace pixaura;
namespace fs=std::filesystem;
#define CHECK(x) do{if(!(x))throw std::runtime_error("publication test line "+std::to_string(__LINE__));}while(false)
template<class T>T good(document::Result<T> r){if(r.code)throw document::Failure{r.code};return std::move(r.value);}
template<class F>void fails(int code,F f){bool failed=false;try{f();}catch(const document::Failure& e){CHECK(e.code==code);failed=true;}CHECK(failed);}
// Synthetic authority ONLY: NOT an installation/store/signing credential. The
// shipping editor uses a keyed protected-installation authenticator instead.
struct FixtureAuthority:storage::ReceiptAuthorityV2{
    std::string key="SYNTHETIC_RECEIPT_TEST_AUTHORITY";
    document::String authenticate(std::string_view s)override{storage::Sha256 h;h.update(reinterpret_cast<const uint8_t*>(key.data()),key.size());h.update(reinterpret_cast<const uint8_t*>(s.data()),s.size());return h.finish();}
};
struct Clock:storage::PublicationClockV2{
    storage::PublicationTimeV2 now{20000LL*86400+86399,20000LL*86400+86399,100,std::string(32,'a'),false,false};int samples=0;
    storage::PublicationTimeV2 sample_at_publication()override{++samples;return now;}
};
struct Bytes:storage::Reader{
    std::string bytes=std::string(128,'x');std::size_t at=0;
    std::size_t read(uint8_t* p,std::size_t n)override{n=std::min(n,bytes.size()-at);std::memcpy(p,bytes.data()+at,n);at+=n;return n;}
};
static std::string read(const char* p){std::ifstream f(p,std::ios::binary);CHECK(f);return {std::istreambuf_iterator<char>(f),{}};}
static document::String canonical(const storage::Loaded& l){return good(document::serialize(*l.snapshot));}
static std::string cmd(const storage::Loaded& l,const std::string& kind,const std::string& tail=""){
    return "{\"command_version\":1,\"kind\":\""+kind+"\",\"expected_revision_id\":\""+l.snapshot->current().text()+"\",\"expected_session_id\":\""+l.snapshot->session().id.text()+"\",\"expected_generation\":"+std::to_string(l.snapshot->session().generation)+tail+"}";
}
static std::string edit(const storage::Loaded& l,unsigned number){
    char id[33],op[33];std::snprintf(id,sizeof(id),"%032u",number);std::snprintf(op,sizeof(op),"%032x",number+1000);
    return cmd(l,"commit",",\"revision_id\":\""+std::string(id)+"\",\"actor\":\"manual\",\"plan_id\":null,\"operations\":[{\"id\":\""+op+"\",\"type\":\"pixaura.exposure\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":1000}}],\"stack\":[\""+op+"\"]");
}
static void prepare(const std::string& root,const char* fixture){
    CHECK(fs::create_directory(root));Bytes input;storage::AssetStore assets(root);auto asset=assets.ingest(input,128);
    std::fprintf(stderr,"prepare asset PASS\n");
    auto text=read(fixture);auto at=text.find(std::string(64,'a'));CHECK(at!=std::string::npos);text.replace(at,64,asset.digest);
    auto doc=good(document::deserialize(text,"00000000000000000000000000000500"));storage::Repository old(root);old.create(*doc);old.migrate(1,2);old.migrate(2,3);std::fprintf(stderr,"prepare schema3 PASS\n");
}
static void tamper_seal(const std::string& root,const std::string& replacement){
    storage::Files files(root);sqlite3* db=nullptr;CHECK(sqlite3_open(files.database_path().c_str(),&db)==SQLITE_OK);
    sqlite3_stmt* q=nullptr;CHECK(sqlite3_prepare_v2(db,"SELECT sql FROM sqlite_schema WHERE name='immutable_receipt_update'",-1,&q,nullptr)==SQLITE_OK);CHECK(sqlite3_step(q)==SQLITE_ROW);std::string trigger(reinterpret_cast<const char*>(sqlite3_column_text(q,0)));sqlite3_finalize(q);
    CHECK(sqlite3_exec(db,"DROP TRIGGER immutable_receipt_update",nullptr,nullptr,nullptr)==SQLITE_OK);
    CHECK(sqlite3_prepare_v2(db,"UPDATE publication_receipts SET seal=? WHERE ordinal=1",-1,&q,nullptr)==SQLITE_OK);CHECK(sqlite3_bind_text(q,1,replacement.c_str(),-1,SQLITE_TRANSIENT)==SQLITE_OK);CHECK(sqlite3_step(q)==SQLITE_DONE);sqlite3_finalize(q);
    CHECK(sqlite3_exec(db,trigger.c_str(),nullptr,nullptr,nullptr)==SQLITE_OK);CHECK(sqlite3_close(db)==SQLITE_OK);
}
static void tamper_operation(const std::string& root,int value){
    storage::Files files(root);sqlite3* db=nullptr;CHECK(sqlite3_open(files.database_path().c_str(),&db)==SQLITE_OK);
    sqlite3_stmt* q=nullptr;CHECK(sqlite3_prepare_v2(db,"SELECT sql FROM sqlite_schema WHERE name='immutable_operation_update'",-1,&q,nullptr)==SQLITE_OK);CHECK(sqlite3_step(q)==SQLITE_ROW);std::string trigger(reinterpret_cast<const char*>(sqlite3_column_text(q,0)));sqlite3_finalize(q);
    CHECK(sqlite3_exec(db,"DROP TRIGGER immutable_operation_update",nullptr,nullptr,nullptr)==SQLITE_OK);
    CHECK(sqlite3_prepare_v2(db,"UPDATE operations SET ev=? WHERE id='0000000000000000000000000000044f'",-1,&q,nullptr)==SQLITE_OK);CHECK(sqlite3_bind_int(q,1,value)==SQLITE_OK);CHECK(sqlite3_step(q)==SQLITE_DONE&&sqlite3_changes(db)==1);sqlite3_finalize(q);
    CHECK(sqlite3_exec(db,trigger.c_str(),nullptr,nullptr,nullptr)==SQLITE_OK);CHECK(sqlite3_close(db)==SQLITE_OK);
}
int main(int argc,char** argv){try{
    CHECK(argc>=2);FixtureAuthority authority;Clock clock;
    if(std::string(argv[1])=="--prepare-schema3"){CHECK(argc==4);prepare(argv[2],argv[3]);return 0;}
    if(std::string(argv[1])=="--migrate-crash"){
        CHECK(argc==4);storage::Repository repo(argv[2],4);const auto point=std::string(argv[3]);
        storage::fault_point=point=="copy"?int(storage::Point::migration_copy):point=="schema"?int(storage::Point::migration_schema):point=="validate"?int(storage::Point::migration_validate):point=="commit"?int(storage::Point::migration_commit):int(storage::Point::migration_published);
        storage::crash_fault=true;repo.migrate(3,4);CHECK(false);
    }
    if(std::string(argv[1])=="--inspect-migration"){
        CHECK(argc==4);const auto version=static_cast<uint32_t>(std::stoul(argv[3]));storage::Repository repo(argv[2],4);
        CHECK(repo.version()==version&&repo.read().snapshot->revisions().size()==3);
        if(version==3){storage::Repository legacy(argv[2]);CHECK(legacy.version()==3);repo.migrate(3,4);}
        CHECK(repo.version()==4&&repo.receipts_v2(authority).empty());return 0;
    }
    if(std::string(argv[1])=="--prepare"){CHECK(argc==4);prepare(argv[2],argv[3]);storage::Repository repo(argv[2],4);repo.migrate(3,4);return 0;}
    if(std::string(argv[1])=="--crash"){
        CHECK(argc==4);storage::Repository repo(argv[2],4);const auto base=repo.read();storage::fault_point=std::string(argv[3])=="before"?int(storage::Point::in_transaction):int(storage::Point::after_commit);storage::crash_fault=true;
        (void)repo.publish_v2(edit(base,103),base.epoch,clock,authority);CHECK(false);
    }
    if(std::string(argv[1])=="--inspect"){
        CHECK(argc==4);storage::Repository repo(argv[2],4);const auto n=std::stoul(argv[3]);CHECK(repo.receipts_v2(authority).size()==n);CHECK(repo.read().snapshot->revisions().size()==3+n);return 0;
    }
    const auto root=(fs::current_path()/("publication-v2-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()))).string();prepare(root,argv[1]);
    storage::Repository repo(root,4);const auto original=repo.read();const auto bytes=canonical(original);
    storage::fault_point=int(storage::Point::in_transaction);fails(12,[&]{repo.migrate(3,4);});storage::fault_point=-1;
    CHECK(repo.version()==3&&canonical(repo.read())==bytes);{storage::Repository legacy(root);CHECK(legacy.version()==3);}
    std::fprintf(stderr,"migration rollback PASS\n");repo.migrate(3,4);std::fprintf(stderr,"migration4 PASS\n");CHECK(repo.version()==4&&canonical(repo.read())==bytes&&repo.read().epoch==original.epoch);CHECK(repo.receipts_v2(authority).empty()&&clock.samples==0);
    fails(4,[&]{storage::Repository legacy(root);});
    auto base=repo.read();const auto command=edit(base,103);
    storage::fault_point=int(storage::Point::in_transaction);fails(12,[&]{repo.publish_v2(command,base.epoch,clock,authority);});storage::fault_point=-1;
    CHECK(repo.receipts_v2(authority).empty()&&repo.read().snapshot->revisions().size()==3);
    std::fprintf(stderr,"publication rollback PASS\n");auto first=repo.publish_v2(command,base.epoch,clock,authority);CHECK(first.kind()=="manual_apply"&&first.source()==base.snapshot->source().sha256.text()&&first.project()==base.snapshot->identity().project.text()&&first.revision()=="00000000000000000000000000000103"&&!first.time().utc_authenticated);
    std::fprintf(stderr,"first publication PASS\n");CHECK(repo.receipts_v2(authority).size()==1&&repo.read().snapshot->revisions().size()==4);
    fails(9,[&]{repo.publish_v2(command,base.epoch,clock,authority);});CHECK(clock.samples==2);
    FixtureAuthority wrong;wrong.key="SYNTHETIC_WRONG_KEY";fails(6,[&]{repo.receipts_v2(wrong);});
    base=repo.read();repo.apply(cmd(base,"undo"),base.epoch);base=repo.read();repo.apply(cmd(base,"redo"),base.epoch);CHECK(repo.receipts_v2(authority).size()==1);
    base=repo.read();fails(4,[&]{repo.apply(edit(base,104),base.epoch);});
    clock.now.uptime_seconds+=2;clock.now.observed_wall_utc+=86400*999;clock.now.logical_utc+=86400*999;
    auto second=repo.publish_v2(edit(base,104),base.epoch,clock,authority);CHECK(second.time().logical_utc==first.time().logical_utc+2&&second.time().logical_utc/86400==20001&&second.event()!=first.event());
    base=repo.read();clock.now.boot=std::string(32,'b');clock.now.uptime_seconds=1;
    auto reboot=repo.publish_v2(edit(base,105),base.epoch,clock,authority);CHECK(reboot.time().uncertain&&reboot.time().logical_utc==second.time().logical_utc);
    // Simulate on-disk corruption while restoring identical schema/trigger SQL.
    // Shape checks alone pass; native authentication must independently reject.
    auto saved_seal=authority.authenticate(first.canonical());tamper_seal(root,std::string(64,'0'));fails(6,[&]{repo.receipts_v2(authority);});tamper_seal(root,saved_seal);CHECK(repo.receipts_v2(authority).size()==3);
    // A valid-range operation change with identical schema must fail even when
    // the original receipt seal is intact. Receipts bind the actual revision.
    tamper_operation(root,2000);fails(6,[&]{repo.receipts_v2(authority);});tamper_operation(root,1000);CHECK(repo.receipts_v2(authority).size()==3);
    base=repo.read();auto one=std::async(std::launch::async,[&]{try{(void)repo.publish_v2(edit(base,106),base.epoch,clock,authority);return 0;}catch(const document::Failure& e){return e.code;}});
    auto two=std::async(std::launch::async,[&]{try{(void)repo.publish_v2(edit(base,107),base.epoch,clock,authority);return 0;}catch(const document::Failure& e){return e.code;}});
    const auto a=one.get(),b=two.get();CHECK((a==0&&b==9)||(a==9&&b==0));CHECK(repo.receipts_v2(authority).size()==4);
    storage::Repository reopened(root,4);auto fresh=reopened.open_session("00000000000000000000000000000501");CHECK(fresh.snapshot->revisions().size()==7&&reopened.receipts_v2(authority).size()==4);
    std::puts("Schema3->4 exact history/source/epoch; v1failclosed; atomic seal; rollback; midnight; reboot; tamper; concurrency; reopen PASS");return 0;
}catch(const document::Failure& e){std::fprintf(stderr,"native failure%d\n",e.code);return 1;}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}}
