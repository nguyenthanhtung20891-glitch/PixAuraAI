#include "pixaura/editor.h"
#include "../../../tests/fixtures/decode/fixtures.h"
#include "../vendor/sqlite/sqlite3.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>
#include <new>
#include <atomic>
#include <array>
#include <thread>
#include <exception>
#ifdef _MSC_VER
#include <crtdbg.h>
#endif
static thread_local int64_t fail_after=-1;
static thread_local bool injected=false;
static thread_local unsigned fault_boundary=0, fault_index=0;
void* operator new(std::size_t n){if(fail_after==0){fail_after=-1;injected=true;
#ifdef _MSC_VER
    if(fault_index<128){std::fprintf(stderr,"editor inject boundary=%u index=%u bytes=%zu\n",fault_boundary,fault_index,n);std::fflush(stderr);}
#endif
    throw std::bad_alloc();}if(fail_after>0)--fail_after;
    auto* p=std::malloc(n?n:1);if(!p)throw std::bad_alloc();return p;}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete(void* p)noexcept{std::free(p);}
void operator delete[](void* p)noexcept{std::free(p);}
void operator delete(void* p,std::size_t)noexcept{std::free(p);}
void operator delete[](void* p,std::size_t)noexcept{std::free(p);}
namespace fs=std::filesystem;
static std::atomic<unsigned> checks{0};
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"editor %d %s\n",__LINE__,#x);std::exit(1);}}while(false)
static const uint8_t* bytes(const std::string& s){return reinterpret_cast<const uint8_t*>(s.data());}
struct Input{std::size_t at=0;};
static int64_t read(void* p,uint8_t* output,uint64_t count){auto& input=*static_cast<Input*>(p);count=std::min<uint64_t>(count,sizeof(decode_png)-input.at);
    std::memcpy(output,decode_png+input.at,static_cast<std::size_t>(count));input.at+=static_cast<std::size_t>(count);return static_cast<int64_t>(count);}
static std::string state(pixaura_editor_context& c){uint64_t n=0;CHECK(pixaura_editor_state(&c,nullptr,0,&n)==0);std::string s(static_cast<std::size_t>(n),' ');
    CHECK(pixaura_editor_state(&c,reinterpret_cast<uint8_t*>(s.data()),n,&n)==0);return s;}
static std::vector<uint8_t> preview(pixaura_editor_context& c,pixaura_editor_request* captured=nullptr){pixaura_editor_request r{};
    CHECK(pixaura_editor_request_preview(&c,&r)==0);std::vector<uint8_t> data(64);uint32_t w=0,h=0;
    CHECK(pixaura_editor_render(&c,&r,4,4,data.data(),data.size(),&w,&h)==0);CHECK(pixaura_editor_current(&c,&r)==0);
    data.resize(w*h*4);if(captured)*captured=r;return data;}
static void faults(const fs::path& parent){std::atomic<unsigned> recovered{0};
    const auto suite=parent/("editor-fault-suite-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));CHECK(fs::create_directory(suite));
    const auto baseline=suite/"baseline";CHECK(fs::create_directory(baseline));const auto baseline_path=baseline.generic_string();
    pixaura_editor_context baseline_context{};Input baseline_input;const std::string baseline_id="0000000000000000000000000000face";
    CHECK(pixaura_editor_open(1,&baseline_context,bytes(baseline_path),baseline_path.size(),bytes(baseline_id),32,read,&baseline_input,sizeof(decode_png))==0);
    CHECK(pixaura_editor_stop(&baseline_context)==0);CHECK(pixaura_editor_destroy(&baseline_context)==0);
    // Independent verified portability copies avoid repeating unrelated schema
    // provisioning/asset-sync work at every allocation site. All approval,
    // SQLite and post-durable checkpoint allocations are still fault injected.
    // Four fixed isolated projects partition every allocation index. Faults are
    // thread-local; no sites are skipped. Parallel disk sync avoids serial CRT/
    // filesystem overhead dominating the MSVC exhaustive sweep deadline.
    for(unsigned boundary=0;boundary<3;++boundary){std::array<std::thread,4> workers;
        for(unsigned worker=0;worker<workers.size();++worker){workers[worker]=std::thread([&,boundary,worker]{bool completed=false;
        for(unsigned index=worker;index<16384&&!completed;index+=4){
            if(index%512<4){std::fprintf(stderr,"editor fault boundary=%u worker=%u index=%u\n",boundary,worker,index);}
            const auto root=suite/("fault-"+std::to_string(boundary)+"-"+std::to_string(index));CHECK(fs::create_directory(root));
            fs::copy(baseline,root,fs::copy_options::recursive);
            const auto path=root.generic_string();char raw[33];std::snprintf(raw,sizeof(raw),"%032x",100000+boundary*16384+index);
            const std::string identity(raw);pixaura_editor_context c{};
            CHECK(pixaura_editor_open(1,&c,bytes(path),path.size(),bytes(identity),32,nullptr,nullptr,0)==0);
            int32_t value=1000;
            if(boundary>0)CHECK(pixaura_editor_select(&c,4)==0);
            if(boundary>1)CHECK(pixaura_editor_update(&c,&value,1)==0);
            injected=false;fail_after=index;
            fault_boundary=boundary;fault_index=index;
            const auto result=boundary==0?pixaura_editor_select(&c,4):boundary==1?pixaura_editor_update(&c,&value,1):pixaura_editor_action(&c,1);
            fail_after=-1;completed=!injected;if(injected)++recovered;
            CHECK(result==0||result==8);
            const auto after=state(c);
            CHECK(after.find("\"revision_count\":"+std::string(boundary==2&&result==0?"2":"1"))!=std::string::npos);
            if(result==8){
                if(boundary==0)CHECK(pixaura_editor_select(&c,4)==0);
                else if(boundary==1){CHECK(after.find("\"values\":[0]")!=std::string::npos);CHECK(pixaura_editor_update(&c,&value,1)==0);}
                else{CHECK(pixaura_editor_action(&c,1)==0);CHECK(state(c).find("\"revision_count\":2")!=std::string::npos);CHECK(pixaura_editor_action(&c,1)==13);}
            }
            CHECK(pixaura_editor_stop(&c)==0);CHECK(pixaura_editor_destroy(&c)==0);
            for(const auto& item:fs::recursive_directory_iterator(root))if(item.is_regular_file())fs::permissions(item.path(),fs::perms::owner_write,fs::perm_options::add);
            fs::remove_all(root);
        }
        CHECK(completed);});}
        for(auto& worker:workers){worker.join();}
    }
    for(const auto& item:fs::recursive_directory_iterator(suite))if(item.is_regular_file())fs::permissions(item.path(),fs::perms::owner_write,fs::perm_options::add);
    fs::remove_all(suite);
    std::printf("editor begin/update/approval allocation failures recovered=%u\n",recovered.load());
}
int main(int argc,char** argv){
#ifdef _MSC_VER
    _set_error_mode(_OUT_TO_STDERR);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#ifdef _DEBUG
    for(const int kind:{_CRT_WARN,_CRT_ERROR,_CRT_ASSERT}){
        _CrtSetReportMode(kind,_CRTDBG_MODE_FILE);_CrtSetReportFile(kind,_CRTDBG_FILE_STDERR);
    }
#endif
#endif
    std::set_terminate([]{std::fprintf(stderr,"editor terminate boundary=%u index=%u injected=%d\n",fault_boundary,fault_index,injected?1:0);std::fflush(stderr);std::_Exit(1);});
    CHECK(argc==2);const auto root=fs::canonical(argv[1])/("editor-project-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    CHECK(fs::create_directory(root));const auto path=root.generic_string();pixaura_editor_context c{};Input input;
    std::string id="987654321012345678901234567890ab";
    CHECK(pixaura_editor_open(1,&c,bytes(path),path.size(),bytes(id),32,read,&input,sizeof(decode_png))==0);
    const auto original=preview(c);CHECK(state(c).find("\"revision_count\":1")!=std::string::npos);
    CHECK(pixaura_editor_select(&c,4)==0);int32_t value=1000;CHECK(pixaura_editor_update(&c,&value,1)==0);
    pixaura_editor_request old{};const auto changed=preview(c,&old);CHECK(changed!=original);
    value=6000;CHECK(pixaura_editor_update(&c,&value,1)==7);CHECK(state(c).find("[1000]")!=std::string::npos);
    CHECK(pixaura_editor_action(&c,2)==13);
    sqlite3* blocker=nullptr;const auto database=(root/"catalog.sqlite").generic_string();CHECK(sqlite3_open(database.c_str(),&blocker)==0);
    CHECK(sqlite3_exec(blocker,"BEGIN IMMEDIATE",nullptr,nullptr,nullptr)==0);
    CHECK(pixaura_editor_action(&c,1)==15);
    CHECK(state(c).find("\"approval_pending\":true")!=std::string::npos);CHECK(state(c).find("\"revision_count\":1")!=std::string::npos);
    CHECK(state(c).find("\"gesture_state\":3")!=std::string::npos);
    CHECK(pixaura_editor_update(&c,&value,1)==13);CHECK(pixaura_editor_action(&c,2)==13);
    CHECK(sqlite3_exec(blocker,"ROLLBACK",nullptr,nullptr,nullptr)==0);CHECK(sqlite3_close(blocker)==0);
    CHECK(pixaura_editor_action(&c,1)==0);CHECK(pixaura_editor_action(&c,1)==13);
    CHECK(pixaura_editor_current(&c,&old)==13);CHECK(preview(c)==changed);CHECK(state(c).find("\"revision_count\":2")!=std::string::npos);
    CHECK(pixaura_editor_action(&c,2)==0);CHECK(preview(c)==original);CHECK(pixaura_editor_action(&c,3)==0);CHECK(preview(c)==changed);
    CHECK(pixaura_editor_select(&c,4)==0);value=-1000;CHECK(pixaura_editor_update(&c,&value,1)==0);(void)preview(c,&old);
    CHECK(pixaura_editor_action(&c,0)==0);CHECK(pixaura_editor_current(&c,&old)==13);CHECK(preview(c)==changed);
    CHECK(pixaura_editor_select(&c,4)==0);value=0;CHECK(pixaura_editor_update(&c,&value,1)==0);CHECK(pixaura_editor_action(&c,1)==0);
    CHECK(state(c).find("\"revision_count\":2")!=std::string::npos);
    CHECK(pixaura_editor_select(&c,3)==0);int32_t crop[4]={1000000,500000,0,0};CHECK(pixaura_editor_update(&c,crop,4)==0);
    (void)preview(c,&old);crop[2]=600000;CHECK(pixaura_editor_update(&c,crop,4)==7);
    CHECK(pixaura_editor_select(&c,6)==0);CHECK(pixaura_editor_current(&c,&old)==13);value=1;CHECK(pixaura_editor_update(&c,&value,1)==0);
    (void)preview(c);CHECK(pixaura_editor_action(&c,4)==0);CHECK(preview(c)==changed);
    // Every frozen category uses the same service and controller; neutral values
    // are independently recorded here to detect descriptor/API drift.
    const int32_t defaults[]={0,0,0,0,0,0,0,1000,0,0,6504};
    for(uint32_t i=0;i<11;++i){CHECK(pixaura_editor_select(&c,i)==0);
        if(i==3){crop[0]=1000000;crop[1]=1000000;crop[2]=0;crop[3]=0;CHECK(pixaura_editor_update(&c,crop,4)==0);}
        else CHECK(pixaura_editor_update(&c,&defaults[i],1)==0);
        CHECK(pixaura_editor_action(&c,1)==0);CHECK(state(c).find("\"revision_count\":2")!=std::string::npos);}
    CHECK(pixaura_editor_stop(&c)==0);CHECK(pixaura_editor_destroy(&c)==0);CHECK(pixaura_editor_destroy(&c)==3);
    c={};id="987654321012345678901234567890ac";
    const auto reopened=pixaura_editor_open(1,&c,bytes(path),path.size(),bytes(id),32,nullptr,nullptr,0);
    if(reopened)std::fprintf(stderr,"reopen status %d\n",reopened);
    CHECK(reopened==0);CHECK(preview(c)==changed);
    CHECK(state(c).find("\"can_undo\":true")!=std::string::npos);
    // Changed edits for every descriptor commit once, undo restores the prior
    // ordered revision. Retained branches prove no platform/local shadow history.
    const int32_t changed_values[]={500,100,100,0,100,100,1,1200,100,500,7000};
    for(uint32_t i=0;i<11;++i){CHECK(pixaura_editor_select(&c,i)==0);
        if(i==3){crop[0]=1000000;crop[1]=500000;crop[2]=0;crop[3]=0;CHECK(pixaura_editor_update(&c,crop,4)==0);}
        else CHECK(pixaura_editor_update(&c,&changed_values[i],1)==0);
        (void)preview(c);CHECK(pixaura_editor_action(&c,1)==0);
        CHECK(state(c).find("\"revision_count\":"+std::to_string(i+3))!=std::string::npos);
        const auto certified=preview(c);
        CHECK(pixaura_editor_action(&c,2)==0);CHECK(preview(c)==changed);
        CHECK(pixaura_editor_action(&c,3)==0);CHECK(preview(c)==certified);
        CHECK(pixaura_editor_stop(&c)==0);CHECK(pixaura_editor_destroy(&c)==0);c={};
        char session[33];std::snprintf(session,sizeof(session),"%032x",200000+i);id=session;
        CHECK(pixaura_editor_open(1,&c,bytes(path),path.size(),bytes(id),32,nullptr,nullptr,0)==0);
        CHECK(preview(c)==certified);CHECK(pixaura_editor_action(&c,2)==0);CHECK(preview(c)==changed);
    }
    CHECK(pixaura_editor_stop(&c)==0);CHECK(pixaura_editor_destroy(&c)==0);
    for(const auto& item:fs::recursive_directory_iterator(root))if(item.is_regular_file())fs::permissions(item.path(),fs::perms::owner_write,fs::perm_options::add);
    fs::remove_all(root);faults(fs::canonical(argv[1]));std::printf("editor application service %u checks PASS\n",checks.load());
}
