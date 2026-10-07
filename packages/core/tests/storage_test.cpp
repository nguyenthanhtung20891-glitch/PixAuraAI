#include "../src/storage.hpp"
#include "../src/sha256.hpp"
#include "../vendor/sqlite/sqlite3.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <future>
#include <iterator>
#include <stdexcept>
#include <thread>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <winioctl.h>
#include <cstring>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif
using namespace pixaura;
using namespace pixaura::storage;
namespace fs=std::filesystem;
#define CHECK(x) do{if(!(x))throw std::runtime_error("line "+std::to_string(__LINE__)+": " #x);}while(false)
namespace {
#ifdef _WIN32
bool junction(const fs::path& link,const fs::path& target) {
    CHECK(fs::create_directory(link));
    struct Data {DWORD tag;WORD size,reserved,sub_offset,sub_length,print_offset,print_length;wchar_t names[4096];} data{};
    static_assert(offsetof(Data,names)==16,"mount-point reparse layout");
    const auto print=target.wstring(),sub=std::wstring(L"\\??\\")+print;
    CHECK(sub.size()+print.size()+2<4096);data.tag=IO_REPARSE_TAG_MOUNT_POINT;
    data.sub_length=static_cast<WORD>(sub.size()*sizeof(wchar_t));data.print_offset=static_cast<WORD>((sub.size()+1)*sizeof(wchar_t));data.print_length=static_cast<WORD>(print.size()*sizeof(wchar_t));
    std::copy(sub.begin(),sub.end(),data.names);std::copy(print.begin(),print.end(),data.names+sub.size()+1);
    const auto bytes=(sub.size()+print.size()+2)*sizeof(wchar_t);data.size=static_cast<WORD>(8+bytes);
    const auto h=CreateFileW(link.c_str(),GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT|FILE_FLAG_BACKUP_SEMANTICS,nullptr);
    if(h==INVALID_HANDLE_VALUE)return false;DWORD returned=0;
    const bool ok=DeviceIoControl(h,FSCTL_SET_REPARSE_POINT,&data,static_cast<DWORD>(16+bytes),nullptr,0,&returned,nullptr)!=0;CloseHandle(h);return ok;
}
#endif
template<class T>T good(document::Result<T> value){CHECK(value.code==0);return std::move(value.value);}
template<class F>void error(int code,F&& f){bool failed=false;try{f();}catch(const document::Failure& e){CHECK(e.code==code);failed=true;}CHECK(failed);}
void note(const char* name){std::fprintf(stderr,"storage scenario=%s\n",name);std::fflush(stderr);}
String fixture(const char* path){std::ifstream f(path,std::ios::binary);CHECK(f.good());const std::string result{std::istreambuf_iterator<char>(f),{}};return String(result);}
struct Input:Reader {
    uint64_t left;uint8_t byte;unsigned calls=0;std::size_t peak=0;bool fail=false;
    Input(uint64_t n,uint8_t b=7):left(n),byte(b){}
    std::size_t read(uint8_t* output,std::size_t capacity) override{CHECK(capacity<=stream_buffer);if(fail)throw document::Failure{12};peak=std::max(peak,capacity);++calls;const auto n=static_cast<std::size_t>(std::min<uint64_t>(left,capacity));std::fill(output,output+n,byte);left-=n;return n;}
};
struct Bytes:Reader {
    std::string_view data;explicit Bytes(std::string_view s):data(s){}
    std::size_t read(uint8_t* out,std::size_t capacity) override{auto n=std::min(data.size(),capacity);std::copy_n(data.data(),n,out);data.remove_prefix(n);return n;}
};
struct Directory {
    fs::path root;
    explicit Directory(const char* name) {
        const auto ticks=std::chrono::steady_clock::now().time_since_epoch().count();
        root=fs::temp_directory_path()/(std::string("pixaura-storage-")+name+"-"+std::to_string(ticks));CHECK(fs::create_directory(root));root=fs::canonical(root);
    }
    ~Directory(){std::error_code ec;for(auto it=fs::recursive_directory_iterator(root,ec);!ec&&it!=fs::recursive_directory_iterator();it.increment(ec))if(it->is_regular_file(ec))fs::permissions(it->path(),fs::perms::owner_all,fs::perm_options::add,ec);fs::remove_all(root,ec);}
    String path() const{return String(root.generic_string());}
};
String manifest(const String& golden,const Asset& asset) {
    String result(golden);auto pos=result.find(String(64,'a'));CHECK(pos!=String::npos);result.replace(pos,64,asset.digest);
    pos=result.find("\"byte_length\":128");CHECK(pos!=String::npos);result.replace(pos,17,"\"byte_length\":"+std::to_string(asset.bytes));return result;
}
String canonical(const Loaded& value){return good(document::serialize(*value.snapshot));}
String command(const Loaded& value,std::string_view kind,std::string_view tail={}) {
    String result("{\"command_version\":1,\"kind\":\"");result.append(kind);result.append("\",\"expected_session_id\":\"");result.append(value.snapshot->session().id.text());result.append("\",\"expected_generation\":");result.append(std::to_string(value.snapshot->session().generation));result.append(",\"expected_revision_id\":\"");result.append(value.snapshot->current().text());result+='"';result.append(tail);result+='}';return result;
}
String commit_tail(const char* revision,const char* operation,int ev) {
    String result(",\"revision_id\":\"");result+=revision;result+="\",\"actor\":\"manual\",\"plan_id\":null,\"stack\":[\"";result+=operation;result+="\"],\"operations\":[{\"id\":\"";result+=operation;result+="\",\"type\":\"pixaura.exposure\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":";result+=std::to_string(ev);result+="}}]";return result;
}
struct Raw {
    sqlite3* db=nullptr;
    explicit Raw(const fs::path& root){const auto path=(root/"catalog.sqlite").generic_string();CHECK(sqlite3_open(path.c_str(),&db)==SQLITE_OK);CHECK(sqlite3_exec(db,"PRAGMA foreign_keys=ON",nullptr,nullptr,nullptr)==SQLITE_OK);}
    ~Raw(){sqlite3_close(db);}
    int sql(const char* text){return sqlite3_exec(db,text,nullptr,nullptr,nullptr)&255;}
};
Asset prepare(const Directory& d,const String& golden) {
    AssetStore store(d.path());Input input(128);auto asset=store.ingest(input,128,{},"source.png");Repository repository(d.path());auto snapshot=good(document::deserialize(manifest(golden,asset),"00000000000000000000000000000500"));auto saved=repository.create(*snapshot);CHECK(saved.epoch==1&&saved.checkpoint_published);return asset;
}
void digest_tests() {
    note("SHA256.known_vectors");Sha256 empty;CHECK(empty.finish()=="e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    Sha256 abc;abc.update(reinterpret_cast<const uint8_t*>("abc"),3);CHECK(abc.finish()=="ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    // FIPS two-block padding vector exercises finish() with used_ > 56.
    constexpr char padding[]="abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    Sha256 padded;padded.update(reinterpret_cast<const uint8_t*>(padding),sizeof(padding)-1);CHECK(padded.finish()=="248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
    Directory d("assets");AssetStore store(d.path());Input large(1000000,'a');const auto a=store.ingest(large,1000000);CHECK(a.digest=="cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");CHECK(large.calls>15&&large.peak==65536);store.verify(a);
    note("asset.dedup_names_and_no_overwrite");Bytes b("abc"),again("abc"),different("abcd"),different_name("abc");auto abc_asset=store.ingest(b,3,{},"../same.png");CHECK(store.ingest(again,3,{},"../same.png").digest==abc_asset.digest);CHECK(store.ingest(different_name,3,{},"other.png").digest==abc_asset.digest);CHECK(store.ingest(different,4,{},"../same.png").digest!=abc_asset.digest);
    CHECK(!fs::exists(d.root.parent_path()/"same.png"));store.verify(abc_asset);
#ifdef _WIN32
    note("asset.persistent_sharing_conflict_rejects_then_recovers");
    const auto held_path=d.root/"assets/sha256"/std::string(abc_asset.digest);
    const auto held=CreateFileW(held_path.c_str(),GENERIC_READ,0,nullptr,OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT,nullptr);
    CHECK(held!=INVALID_HANDLE_VALUE);error(6,[&]{store.verify(abc_asset);});CHECK(CloseHandle(held)!=0);store.verify(abc_asset);
#endif
    note("asset.empty_truncated_read_write_mismatch");Input zero(0);error(8,[&]{store.ingest(zero,0);});Input truncated(2);error(6,[&]{store.ingest(truncated,3);});Input too_long(4);error(6,[&]{store.ingest(too_long,3);});Input read_failure(3);read_failure.fail=true;error(12,[&]{store.ingest(read_failure,3);});
    Input mismatch(3);error(6,[&]{store.ingest(mismatch,3,String(64,'0'));});
    for(int point=0;point<4;++point){Input i(stream_buffer+5,static_cast<uint8_t>(point));fault_point=point;error(12,[&]{store.ingest(i,stream_buffer+5);});fault_point=-1;}
    note("asset.exclusive_temp_collision_preserves_other_owner");
    const auto other_temp=d.root/"staging/temp-collision";{std::ofstream f(other_temp,std::ios::binary);f<<"other owner";}
    temp_name_override="temp-collision";Bytes collision("abc");error(12,[&]{store.ingest(collision,3);});temp_name_override=nullptr;
    CHECK(fixture(other_temp.string().c_str())=="other owner");
    note("asset.tamper_collision_rejected");const auto path=d.root/"assets/sha256"/std::string(abc_asset.digest);fs::permissions(path,fs::perms::owner_write,fs::perm_options::add);{std::ofstream f(path,std::ios::binary|std::ios::trunc);f<<"xyz";}error(6,[&]{store.verify(abc_asset);});Bytes same("abc");error(6,[&]{store.ingest(same,3);});CHECK(fixture(path.string().c_str())=="xyz");
    note("asset.concurrent_dedup");
    for(unsigned round=0;round<32;++round){
        std::atomic<unsigned> ready{0};
        auto ingest=[&]{AssetStore s(d.path());Input i(12345+round,91);++ready;while(ready.load()<2)std::this_thread::yield();return s.ingest(i,12345+round);};
        auto one=std::async(std::launch::async,ingest),two=std::async(std::launch::async,ingest);
        const auto x=one.get(),y=two.get();CHECK(x.digest==y.digest);store.verify(x);
    }
    note("asset.traversal_and_symlink");error(1,[&]{store.verify({"../escape",3});});error(1,[&]{AssetStore s(d.path()+"/../escape");});
    Directory links("links");std::error_code ec;fs::create_directory_symlink(d.root,links.root/"staging",ec);if(!ec){error(6,[&]{AssetStore s(links.path());});}
    else {
#ifdef _WIN32
        // NTFS junctions need no developer-mode symlink privilege. Test the same
        // reparse rejection through this supported OS mechanism when available.
        if(junction(links.root/"staging",d.root)) {
            struct Remove{fs::path p;~Remove(){RemoveDirectoryW(p.c_str());}}remove{links.root/"staging"};
            error(6,[&]{AssetStore s(links.path());});note("asset.Windows_junction_rejected");
        }else std::fprintf(stderr,"storage Windows link fixture unavailable os=%lu\n",GetLastError());
#else
        throw std::runtime_error("required POSIX symlink fixture unavailable");
#endif
    }
#ifndef _WIN32
    note("asset.search_only_ancestor_is_supported_without_following_links");
    const auto parent=d.root/"search-only";CHECK(fs::create_directory(parent));const auto leaf=parent/"private";CHECK(fs::create_directory(leaf));
    struct Restore{fs::path path;~Restore(){std::error_code e;fs::permissions(path,fs::perms::owner_all,fs::perm_options::add,e);}}restore{parent};
    fs::permissions(parent,fs::perms::owner_exec|fs::perms::group_exec|fs::perms::others_exec);
    AssetStore private_store(String(leaf.generic_string()));Bytes input("abc");private_store.verify(private_store.ingest(input,3));
#endif
}
void path_shape_tests() {
    note("db.canonical_private_var_shape_and_alias_rejection");
    Directory d("path-shape");
    const auto private_var=d.root/"private/var";
    const auto suffix=fs::path("folders/test user/T/project #%");
    const auto leaf=private_var/suffix;CHECK(fs::create_directories(leaf));
    const auto canonical_path=String(fs::canonical(leaf).generic_string());
    {Repository created(canonical_path);CHECK(created.version()==1);}
    CHECK(fs::is_regular_file(leaf/"catalog.sqlite"));
    {Repository reopened(canonical_path);CHECK(reopened.version()==1);}
    CHECK(sqlite3_libversion_number()==3053004);
    error(1,[&]{Repository invalid(canonical_path+"/../escape");});
    error(6,[&]{Repository invalid(String((leaf/"catalog.sqlite").generic_string()));});
    std::error_code ec;const auto alias=d.root/"var";
    fs::create_directory_symlink(private_var,alias,ec);
#ifdef _WIN32
    if(ec)CHECK(junction(alias,private_var));
    struct RemoveAlias{fs::path path;~RemoveAlias(){RemoveDirectoryW(path.c_str());}}remove_alias{alias};
#else
    CHECK(!ec);
#endif
    error(6,[&]{Repository untrusted(String((alias/suffix).generic_string()));});
#ifndef _WIN32
    note("db.unwritable_private_directory_rejected");
    const auto denied=d.root/"unwritable";CHECK(fs::create_directory(denied));
    struct Restore{fs::path path;~Restore(){std::error_code e;fs::permissions(path,fs::perms::owner_all,fs::perm_options::add,e);}}restore{denied};
    fs::permissions(denied,fs::perms::owner_read|fs::perms::owner_exec|fs::perms::group_read|fs::perms::group_exec|fs::perms::others_read|fs::perms::others_exec);
    // Root bypasses DAC; exercise rejection in an unprivileged child instead.
    if(geteuid()==0) {
        const auto pid=fork();CHECK(pid>=0);
        if(pid==0) {
            if(setuid(65534)!=0)std::_Exit(2);
            try{error(12,[&]{Repository invalid(String(denied.generic_string()));});std::_Exit(0);}catch(...){std::_Exit(1);}
        }
        int status=0;CHECK(waitpid(pid,&status,0)==pid&&WIFEXITED(status)&&WEXITSTATUS(status)==0);
    }else error(12,[&]{Repository invalid(String(denied.generic_string()));});
    CHECK(!fs::exists(denied/"catalog.sqlite"));
#endif
}
void database_tests(const String& golden) {
    note("db.fresh_reopen_history_and_canonical");Directory d("database");auto asset=prepare(d,golden);const auto expected=manifest(golden,asset);Repository r(d.path());CHECK(r.version()==1);auto initial=r.read();CHECK(canonical(initial)==expected);CHECK(initial.snapshot->revisions().size()==3);
    note("db.undo_redo_and_new_session");const auto stale=command(initial,"undo");r.apply(stale,initial.epoch);auto undone=r.read();CHECK(undone.snapshot->redo().size()==1&&undone.snapshot->session().generation==1);
    Repository reopened(d.path());CHECK(canonical(reopened.read())==canonical(undone));auto fresh=reopened.open_session("00000000000000000000000000000501");CHECK(fresh.snapshot->session().generation==0&&fresh.epoch==3);error(9,[&]{reopened.apply(stale,fresh.epoch);});reopened.apply(command(fresh,"redo"),fresh.epoch);CHECK(canonical(reopened.read())==expected);
    auto now=reopened.read();reopened.apply(command(now,"undo"),now.epoch);now=reopened.read();reopened.apply(command(now,"undo"),now.epoch);now=reopened.read();CHECK(now.snapshot->redo().size()==2);
    note("db.branch_retention_and_parameter_replacement");const auto branch=commit_tail("00000000000000000000000000000103","00000000000000000000000000000013",2000);reopened.apply(command(now,"commit",branch),now.epoch);now=reopened.read();CHECK(now.snapshot->revisions().size()==4&&now.snapshot->operations().size()==3&&now.snapshot->redo().empty());
    const auto replace=commit_tail("00000000000000000000000000000104","00000000000000000000000000000014",-1000);reopened.apply(command(now,"commit",replace),now.epoch);now=reopened.read();CHECK(now.snapshot->revisions().size()==5&&std::get<document::Exposure>(now.snapshot->operations()[0].parameters).milli_ev==1250);
    Repository again(d.path());CHECK(canonical(again.read())==canonical(now));CHECK(again.checkpoint());CHECK(fixture((d.root/"checkpoint.json").string().c_str())==canonical(now));
    note("db.checkpoint_temp_collision_preserves_other_owner");const auto other_temp=d.root/"staging/temp-collision";{std::ofstream f(other_temp,std::ios::binary);f<<"other owner";}
    temp_name_override="temp-collision";error(12,[&]{again.checkpoint();});temp_name_override=nullptr;CHECK(fixture(other_temp.string().c_str())=="other owner");CHECK(canonical(again.read())==canonical(now));
    note("db.foreign_keys_duplicates_constraints_immutability");Raw raw(d.root);
    CHECK(raw.sql("INSERT INTO revisions VALUES('00000000000000000000000000000002','00000000000000000000000000000100',0,NULL,'import',NULL)")==SQLITE_CONSTRAINT);
    CHECK(raw.sql("INSERT INTO revisions VALUES('00000000000000000000000000000002','00000000000000000000000000000105',5,'ffffffffffffffffffffffffffffffff','manual',NULL)")==SQLITE_CONSTRAINT);
    CHECK(raw.sql("UPDATE documents SET width=0")==SQLITE_CONSTRAINT);CHECK(raw.sql("UPDATE operations SET ev=77")==SQLITE_CONSTRAINT);CHECK(raw.sql("DELETE FROM revisions")==SQLITE_CONSTRAINT);
    CHECK(raw.sql("BEGIN IMMEDIATE; UPDATE documents SET current='ffffffffffffffffffffffffffffffff'")==SQLITE_OK);CHECK(raw.sql("COMMIT")==SQLITE_CONSTRAINT);CHECK(raw.sql("ROLLBACK")==SQLITE_OK);CHECK(canonical(again.read())==canonical(now));
    note("db.rollback_and_explicit_migration");fault_point=static_cast<int>(Point::in_transaction);error(12,[&]{again.apply(command(now,"undo"),now.epoch);});
    const auto failed_append=commit_tail("00000000000000000000000000000105","00000000000000000000000000000015",500);
    error(12,[&]{again.apply(command(now,"commit",failed_append),now.epoch);});
    error(12,[&]{again.migrate(1,1);});fault_point=-1;CHECK(canonical(again.read())==canonical(now)&&again.read().epoch==now.epoch);again.migrate(1,1);error(4,[&]{again.migrate(1,2);});error(4,[&]{again.migrate(0,1);});
    note("db.busy_is_bounded_and_readers_work");CHECK(raw.sql("BEGIN IMMEDIATE")==SQLITE_OK);const auto start=std::chrono::steady_clock::now();error(busy,[&]{again.apply(command(now,"undo"),now.epoch);});CHECK(std::chrono::steady_clock::now()-start<std::chrono::seconds(3));CHECK(canonical(r.read())==canonical(now));CHECK(raw.sql("ROLLBACK")==SQLITE_OK);
    note("db.WAL_budget_preserves_reader_and_committed_state");
    CHECK(raw.sql("BEGIN; SELECT count(*) FROM revisions")==SQLITE_OK);
    again.apply(command(now,"undo"),now.epoch);const auto budget_state=again.read();
    // Grow only unused physical WAL tail, leaving all committed frames intact.
    // A pinned reader prevents truncate; release permits bounded recovery.
    fs::resize_file(d.root/"catalog.sqlite-wal",33554433);
    error(busy,[&]{again.apply(command(budget_state,"redo"),budget_state.epoch);});
    CHECK(canonical(again.read())==canonical(budget_state));CHECK(raw.sql("ROLLBACK")==SQLITE_OK);
    again.apply(command(budget_state,"redo"),budget_state.epoch);CHECK(canonical(again.read())==canonical(now));
    note("db.checkpoint_failure_reports_committed_epoch");
    for(const auto p:{Point::after_commit,Point::checkpoint_write,Point::checkpoint_publish,Point::checkpoint_renamed}) {
        auto before=again.read();fault_point=static_cast<int>(p);const auto saved=again.apply(command(before,"undo"),before.epoch);fault_point=-1;
        CHECK(saved.epoch==before.epoch+1&&!saved.checkpoint_published);auto committed=again.read();CHECK(committed.epoch==saved.epoch&&committed.snapshot->current()!=before.snapshot->current());
        again.apply(command(committed,"redo"),committed.epoch);CHECK(canonical(again.read())==canonical(now));
    }
    note("db.asset_before_reference_and_corrupt_rows");Directory missing("missing");Repository m(missing.path());error(6,[&]{m.create(*initial.snapshot);});
    Directory corrupt("corrupt");prepare(corrupt,golden);{Raw c(corrupt.root);CHECK(c.sql("PRAGMA foreign_keys=OFF; UPDATE documents SET current='ffffffffffffffffffffffffffffffff'")==SQLITE_OK);}Repository c(corrupt.path());error(6,[&]{c.read();});
    Directory malformed("malformed");prepare(malformed,golden);{Raw bad(malformed.root);CHECK(bad.sql("PRAGMA ignore_check_constraints=ON; UPDATE documents SET generation=-1")==SQLITE_OK);}Repository bad(malformed.path());error(6,[&]{bad.read();});
    Directory invalid_redo("redo");prepare(invalid_redo,golden);{Raw red(invalid_redo.root);CHECK(red.sql("INSERT INTO redo VALUES('00000000000000000000000000000002',0,'00000000000000000000000000000100')")==SQLITE_OK);}Repository red(invalid_redo.path());error(6,[&]{red.read();});
    Directory future("future");prepare(future,golden);{Raw f(future.root);CHECK(f.sql("PRAGMA user_version=2")==SQLITE_OK);}error(4,[&]{Repository f(future.path());});
    Directory schema("schema");prepare(schema,golden);{Raw f(schema.root);CHECK(f.sql("DROP TRIGGER immutable_revision_delete")==SQLITE_OK);}error(6,[&]{Repository f(schema.path());});
    Directory tamper("referenced");auto a=prepare(tamper,golden);auto source=tamper.root/"assets/sha256"/std::string(a.digest);fs::permissions(source,fs::perms::owner_write,fs::perm_options::add);{std::ofstream f(source,std::ios::binary|std::ios::trunc);f<<'x';}Repository t(tamper.path());error(6,[&]{t.read();});
}
std::atomic<bool> entered{false},resume{false};
void pause_writer(Point p){if(p!=Point::in_transaction)return;entered.store(true);const auto end=std::chrono::steady_clock::now()+std::chrono::seconds(5);while(!resume.load()){CHECK(std::chrono::steady_clock::now()<end);std::this_thread::yield();}}
void concurrency_tests(const String& golden) {
    note("db.two_readers_reader_during_writer");Directory d("concurrent");prepare(d,golden);Repository writer(d.path()),reader1(d.path()),reader2(d.path());const auto prior=writer.read();
    auto f=std::async(std::launch::async,[&]{point_hook=pause_writer;const auto saved=writer.apply(command(prior,"undo"),prior.epoch);point_hook=nullptr;return saved;});
    const auto limit=std::chrono::steady_clock::now()+std::chrono::seconds(5);while(!entered.load()){CHECK(std::chrono::steady_clock::now()<limit);std::this_thread::yield();}
    auto read1=std::async(std::launch::async,[&]{return canonical(reader1.read());});auto read2=std::async(std::launch::async,[&]{return canonical(reader2.read());});CHECK(read1.get()==canonical(prior)&&read2.get()==canonical(prior));resume.store(true);CHECK(f.get().epoch==prior.epoch+1);
    note("db.competing_writes_epoch_CAS");const auto now=writer.read();auto write=[&](Repository& r){try{r.apply(command(now,"undo"),now.epoch);return 0;}catch(const document::Failure& e){return e.code;}};auto a=std::async(std::launch::async,[&]{return write(reader1);}),b=std::async(std::launch::async,[&]{return write(reader2);});const auto x=a.get(),y=b.get();CHECK((x==0&&(y==9||y==busy))||(y==0&&(x==9||x==busy)));CHECK(writer.read().epoch==now.epoch+1);
    note("db.retained_owner_during_operation");auto owner=std::make_shared<Repository>(d.path());std::weak_ptr<Repository> weak=owner;
    std::promise<void> ready,release;auto release_signal=release.get_future();
    auto operation=std::async(std::launch::async,[retained=owner,&ready,&release_signal]{ready.set_value();CHECK(release_signal.wait_for(std::chrono::seconds(5))==std::future_status::ready);return retained->read().epoch;});
    CHECK(ready.get_future().wait_for(std::chrono::seconds(5))==std::future_status::ready);owner.reset();CHECK(!weak.expired());release.set_value();CHECK(operation.get()==now.epoch+1);CHECK(weak.expired());
}
int launch(const char* self,int point,const String& root,const char* golden) {
#ifdef _WIN32
    const std::string text=std::string("\"")+self+"\" --crash "+std::to_string(point)+" \""+std::string(root)+"\" \""+golden+"\"";
    const int n=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,text.c_str(),-1,nullptr,0);CHECK(n>0);std::wstring cmd(static_cast<std::size_t>(n),'\0');CHECK(MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,text.c_str(),-1,cmd.data(),n)==n);
    STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION process{};CHECK(CreateProcessW(nullptr,cmd.data(),nullptr,nullptr,FALSE,0,nullptr,nullptr,&startup,&process)!=0);
    CHECK(WaitForSingleObject(process.hProcess,30000)==WAIT_OBJECT_0);DWORD code=0;CHECK(GetExitCodeProcess(process.hProcess,&code)!=0);CloseHandle(process.hThread);CloseHandle(process.hProcess);return static_cast<int>(code);
#else
    const auto pid=fork();CHECK(pid>=0);if(pid==0){const auto n=std::to_string(point);execl(self,self,"--crash",n.c_str(),root.c_str(),golden,static_cast<char*>(nullptr));std::_Exit(74);}int status=0;CHECK(waitpid(pid,&status,0)==pid&&WIFEXITED(status));return WEXITSTATUS(status);
#endif
}
void crash_tests(const char* self,const char* file,const String& golden) {
    for(int point=0;point<9;++point) {
        note("crash.boundary");std::fprintf(stderr,"storage crash_point=%d\n",point);Directory d("crash");{Repository init(d.path());CHECK(init.version()==1);}CHECK(launch(self,point,d.path(),file)==73);
        Repository reopened(d.path());if(point<5){error(6,[&]{reopened.read();});}
        else {const auto value=reopened.read();CHECK(value.epoch==1&&value.snapshot->revisions().size()==3);CHECK(reopened.checkpoint());CHECK(fixture((d.root/"checkpoint.json").string().c_str())==canonical(value));}
        // Staging bytes are never inferred to be trusted; SQLite references only
        // the published digest, and unreferenced published bytes remain harmless.
        if(point==1||point==2)CHECK(!fs::is_empty(d.root/"staging")&&fs::is_empty(d.root/"assets/sha256"));
    }
    (void)golden;
}
}
int main(int argc,char** argv) {
    try {
        if(argc==5&&std::string_view(argv[1])=="--crash") {
            fault_point=std::atoi(argv[2]);crash_fault=true;const auto golden=fixture(argv[4]);AssetStore store(argv[3]);Input input(stream_buffer+128);const auto asset=store.ingest(input,stream_buffer+128);Repository r(argv[3]);auto snapshot=good(document::deserialize(manifest(golden,asset),"00000000000000000000000000000500"));r.create(*snapshot);return 75;
        }
        CHECK(argc==2);const auto golden=fixture(argv[1]);digest_tests();path_shape_tests();database_tests(golden);concurrency_tests(golden);crash_tests(argv[0],argv[1],golden);std::puts("SQLite persistence, streamed immutable assets, crash/failure and concurrency PASS");return 0;
    }catch(const document::Failure& e){std::fprintf(stderr,"storage FAIL status=%d\n",e.code);return 1;}catch(const std::exception& e){std::fprintf(stderr,"storage FAIL %s\n",e.what());return 1;}
}
