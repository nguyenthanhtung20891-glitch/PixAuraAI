#include "../src/storage.hpp"
#include "../src/evaluation.hpp"
#include "pixaura/manual.h"
#include "pixaura/preset.h"
#include "../../../tests/fixtures/decode/fixtures.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <mutex>
#include <new>
#include <thread>

// Independent test executable compiles production sources with fallible STL.
// Live payload accounting proves retained memory, not a device RSS claim.
static thread_local int64_t fail_after=-1;
static thread_local bool injected=false;
struct alignas(std::max_align_t) Allocation { std::size_t bytes; };
static std::atomic<std::size_t> live_bytes{0};
void* operator new(std::size_t n) {
    if(fail_after==0){fail_after=-1;injected=true;throw std::bad_alloc();}
    if(fail_after>0)--fail_after;
    auto* p=static_cast<Allocation*>(std::malloc(sizeof(Allocation)+(n?n:1)));
    if(!p)throw std::bad_alloc();
    p->bytes=n;live_bytes+=n;return p+1;
}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete(void* p) noexcept {if(p){auto* a=static_cast<Allocation*>(p)-1;live_bytes-=a->bytes;std::free(a);}}
void operator delete[](void* p) noexcept {::operator delete(p);}
void operator delete(void* p,std::size_t) noexcept {::operator delete(p);}
void operator delete[](void* p,std::size_t) noexcept {::operator delete(p);}
using namespace pixaura;
namespace fs=std::filesystem;
namespace pixaura::preview { void test_observer(pixaura_decode_context*,evaluation::Observer,void*);void test_generation(pixaura_decode_context*,uint64_t); }
static std::atomic<unsigned> checks{0};
#define CHECK(x) do{++checks;if(!(x)){fail_after=-1;std::fprintf(stderr,"gesture check %d: %s\n",__LINE__,#x);std::exit(1);}}while(false)
static std::string id(unsigned n){char b[33];std::snprintf(b,sizeof(b),"%032x",n);return b;}
static const uint8_t* bytes(const std::string& s){return reinterpret_cast<const uint8_t*>(s.data());}
static std::string op(const char* tool,const char* parameter,int value,unsigned identity=9000){return "{\"operations\":[{\"id\":\""+id(identity)+"\",\"type\":\"pixaura."+tool+"\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\""+parameter+"\":"+std::to_string(value)+"}}]}";}
struct Input:storage::Reader{std::size_t at=0;std::size_t read(uint8_t* p,std::size_t n)override{n=std::min(n,sizeof(decode_png)-at);std::memcpy(p,decode_png+at,n);at+=n;return n;}};
struct Fixture {
    static std::atomic<unsigned> identities;
    std::string path,manifest,session=id(3);unsigned fresh=identities.fetch_add(100000);
    pixaura_document_context document{};pixaura_document_handle base{};
    pixaura_decode_context raster{};pixaura_decode_handle original{};pixaura_working_limits limits{};
    pixaura_preview_request request{1,sizeof(pixaura_preview_request),PIXAURA_PREVIEW_FIT,2,3,0};
    Fixture(const fs::path& directory){
        path=directory.generic_string();CHECK(fs::create_directory(directory));storage::AssetStore assets(path);Input input;const auto asset=assets.ingest(input,sizeof(decode_png));
        manifest="{\"current_revision_id\":\""+id(4)+"\",\"document_id\":\""+id(2)+"\",\"operations\":[],\"project_id\":\""+id(1)+"\",\"redo\":[],\"revisions\":[{\"actor\":\"import\",\"id\":\""+id(4)+"\",\"parent_id\":null,\"plan_id\":null,\"stack\":[]}],\"schema_version\":1,\"source\":{\"byte_length\":"+std::to_string(sizeof(decode_png))+",\"metadata\":{\"codec\":\"png\",\"has_alpha\":true,\"height\":3,\"icc_sha256\":null,\"orientation\":1,\"width\":2},\"sha256\":\""+std::string(asset.digest)+"\"}}\n";
        const auto did=id(++fresh),rid=id(++fresh);pixaura_decode_limits dl{};pixaura_decode_handle source{},decoded{};
        CHECK(pixaura_document_context_init(1,&document,sizeof(document),bytes(did),32,nullptr)==0);
        CHECK(pixaura_document_create(1,&document,bytes(manifest),manifest.size(),bytes(session),32,&base,nullptr)==0);
        CHECK(pixaura_decode_default_limits(1,&dl)==0&&pixaura_working_default_limits(1,&limits)==0);
        CHECK(pixaura_decode_context_init(1,&raster,sizeof(raster),bytes(rid),32,&dl)==0);
        CHECK(pixaura_decode_open(&raster,bytes(path),path.size(),reinterpret_cast<const uint8_t*>(asset.digest.data()),64,sizeof(decode_png),&source)==0);
        CHECK(pixaura_decode_image(&raster,&source,&decoded)==0&&pixaura_working_normalize(&raster,&decoded,&limits,&original)==0);
        CHECK(pixaura_decode_release(&raster,&source)==0&&pixaura_decode_release(&raster,&decoded)==0);
    }
    ~Fixture(){CHECK(pixaura_manual_interrupt(&document)==0);CHECK(pixaura_document_context_destroy(&document)==0);CHECK(pixaura_preview_stop(&raster)==0);CHECK(pixaura_decode_context_destroy(&raster)==0);
        // Test-created immutable assets are made writable only after all owners close.
        for(const auto& item:fs::recursive_directory_iterator(path))if(item.is_regular_file())fs::permissions(item.path(),fs::perms::owner_write,fs::perm_options::add);
        fs::remove_all(fs::path(path));}
    pixaura_manual_gesture begin(const char* tool="pixaura.exposure",const pixaura_document_handle* live=nullptr){pixaura_manual_gesture g{};const auto gid=id(++fresh);
        CHECK(pixaura_manual_edit_begin(1,&document,live?live:&base,bytes(gid),32,reinterpret_cast<const uint8_t*>(tool),std::strlen(tool),nullptr,0,&raster,&original,&g)==0);return g;}
    uint64_t update(const pixaura_manual_gesture& g,const std::string& s,const pixaura_document_handle* live=nullptr){uint64_t seq=0;CHECK(pixaura_manual_update(&document,&g,live?live:&base,bytes(s),s.size(),&seq)==0);return seq;}
    pixaura_preview_ticket ticket(const pixaura_manual_gesture& g,uint64_t& seq){pixaura_preview_ticket t{};CHECK(pixaura_manual_preview_ticket(&document,&g,&base,&t,&seq)==0);return t;}
    uint32_t state(const pixaura_manual_gesture& g,uint64_t expected){uint32_t s=0;uint64_t seq=99;CHECK(pixaura_manual_state(&document,&g,&s,&seq)==0&&seq==expected);return s;}
    int render(const pixaura_manual_gesture& g,const pixaura_preview_ticket& t,pixaura_decode_handle& result,uint64_t& seq){return pixaura_manual_render(&document,&g,&base,&raster,&original,&limits,nullptr,&t,&request,&result,&seq);}
    std::string serialize(const pixaura_document_handle& h){uint64_t n=0;CHECK(pixaura_document_serialize(&document,&h,nullptr,0,&n,nullptr)==0);std::string s(static_cast<std::size_t>(n),' ');CHECK(pixaura_document_serialize(&document,&h,reinterpret_cast<uint8_t*>(s.data()),n,&n,nullptr)==0);return s;}
    std::string command(const pixaura_document_handle& h,const char* kind,uint64_t generation){auto d=document::deserialize(serialize(h),session);CHECK(d.code==0);
        const auto revision=d.value->current().text();
        return "{\"command_version\":1,\"kind\":\""+std::string(kind)+"\",\"expected_session_id\":\""+session+"\",\"expected_generation\":"+std::to_string(generation)+",\"expected_revision_id\":\""+std::string(revision)+"\"}";}
};
std::atomic<unsigned> Fixture::identities{10000};
static void baseline(Fixture& f){
    for(const auto* tool:{"pixaura.rotate","pixaura.exposure","pixaura.sharpen"}){
        auto g=f.begin(tool);CHECK(f.state(g,0)==PIXAURA_MANUAL_ACTIVE);
        pixaura_manual_gesture conflict{};const auto gid=id(++f.fresh);CHECK(pixaura_manual_begin(1,&f.document,&f.base,bytes(gid),32,reinterpret_cast<const uint8_t*>(tool),std::strlen(tool),nullptr,0,&conflict)==1);
        uint64_t seq=0;const auto initial=f.ticket(g,seq);pixaura_decode_handle display{};
        CHECK(f.render(g,initial,display,seq)==0&&seq==0);uint8_t copy[24];CHECK(pixaura_preview_copy(&f.raster,&display,0,copy,24)==0);
        const char* parameter=std::strcmp(tool,"pixaura.rotate")==0?"quarter_turns":std::strcmp(tool,"pixaura.exposure")==0?"milli_ev":"milli_amount";
        const char* name=tool+8;const auto operation=op(name,parameter,1);const auto retained=live_bytes.load();
        for(unsigned i=1;i<=10000;++i){CHECK(f.update(g,operation)==i);CHECK(live_bytes.load()<=retained+2048);}
        CHECK(f.serialize(f.base)==f.manifest);
        const auto newest=f.ticket(g,seq);CHECK(seq==10000&&newest.generation==initial.generation+10000);
        CHECK(pixaura_manual_preview_current(&f.document,&g,&f.base,0,&initial,&display)==13);
        const auto invalid=op(name,parameter,100000);uint64_t untouched=99;CHECK(pixaura_manual_update(&f.document,&g,&f.base,bytes(invalid),invalid.size(),&untouched)==7&&untouched==99);
        uint64_t after=0;CHECK(f.ticket(g,after).generation==newest.generation&&after==10000);
        auto bad_request=f.request;bad_request.max_width=0;pixaura_decode_handle failed{};CHECK(pixaura_manual_render(&f.document,&g,&f.base,&f.raster,&f.original,&f.limits,nullptr,&newest,&bad_request,&failed,&untouched)==1);
        uint8_t still[24];CHECK(pixaura_preview_copy(&f.raster,&display,0,still,24)==0&&std::memcmp(copy,still,24)==0);
        pixaura_document_handle proposal{};uint32_t changed=99;const auto revision=id(++f.fresh);
        const auto undo=f.command(f.base,"undo",0);CHECK(pixaura_document_apply(1,&f.document,&f.base,bytes(undo),undo.size(),&proposal,nullptr)==13);
        const std::string recipe="{\"preset_id\":\"reference\",\"name\":\"Lifecycle reference\",\"schema_version\":1,\"recipe_version\":1,\"operations\":[{\"id\":\""+id(90)+"\",\"type\":\"pixaura.exposure\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":1000}}]}";
        const std::string bindings="{\"operation_ids\":[\""+id(++f.fresh)+"\"]}";
        CHECK(pixaura_preset_propose(1,&f.document,&f.base,&f.base,bytes(recipe),recipe.size(),bytes(bindings),bindings.size(),bytes(revision),32,&proposal,&changed)==13);
        CHECK(pixaura_manual_commit(&f.document,&g,&f.base,bytes(revision),32,&proposal,&changed)==0&&changed==1);
        CHECK(f.state(g,10000)==PIXAURA_MANUAL_COMPLETED);CHECK(pixaura_preview_current(&f.raster,&newest,nullptr)==13);
        CHECK(pixaura_manual_commit(&f.document,&g,&f.base,bytes(revision),32,&proposal,&changed)==13);
        const auto doc=document::deserialize(f.serialize(proposal),f.session);CHECK(doc.code==0&&doc.value->revisions().size()==2&&doc.value->operations().size()==1);
        CHECK(std::visit([](const auto&){return true;},doc.value->operations()[0].parameters));
        const auto undo_command=f.command(proposal,"undo",1);pixaura_document_handle undone{};CHECK(pixaura_document_apply(1,&f.document,&proposal,bytes(undo_command),undo_command.size(),&undone,nullptr)==0);
        const auto redo=f.command(undone,"redo",2);
        pixaura_document_handle redone{};CHECK(pixaura_document_apply(1,&f.document,&undone,bytes(redo),redo.size(),&redone,nullptr)==0);CHECK(f.serialize(redone)==f.serialize(proposal));
        CHECK(pixaura_manual_cancel(&f.document,&g)==0&&f.state(g,10000)==PIXAURA_MANUAL_COMPLETED);
        CHECK(pixaura_manual_release(&f.document,&g)==0&&pixaura_preview_release(&f.raster,&display)==0);CHECK(copy[3]==255);
        CHECK(pixaura_document_release(&f.document,&undone)==0&&pixaura_document_release(&f.document,&redone)==0&&pixaura_document_release(&f.document,&proposal)==0);
        auto neutral=f.begin(tool);CHECK(f.update(neutral,op(name,parameter,1))==1);CHECK(f.update(neutral,op(name,parameter,0))==2);
        CHECK(pixaura_manual_commit(&f.document,&neutral,&f.base,bytes(id(++f.fresh)),32,&proposal,&changed)==0&&changed==0&&proposal.serial==f.base.serial);CHECK(pixaura_manual_release(&f.document,&neutral)==0);
    }
}
struct Pause {std::mutex mutex;std::condition_variable condition;bool entered=false,proceed=false;};
static void pause_preview(evaluation::Checkpoint,void* raw){auto& p=*static_cast<Pause*>(raw);std::unique_lock<std::mutex> lock(p.mutex);if(!p.entered){p.entered=true;p.condition.notify_all();CHECK(p.condition.wait_for(lock,std::chrono::seconds(10),[&]{return p.proceed;}));}}
static void preview_races(Fixture& f){
    for(unsigned action=0;action<4;++action){auto g=f.begin();f.update(g,op("exposure","milli_ev",1));uint64_t seq=0;const auto old=f.ticket(g,seq);
        Pause pause;preview::test_observer(&f.raster,pause_preview,&pause);pixaura_decode_handle output{};int result=-1;uint64_t rendered=99;
        std::thread worker([&]{result=f.render(g,old,output,rendered);});
        {std::unique_lock<std::mutex> lock(pause.mutex);CHECK(pause.condition.wait_for(lock,std::chrono::seconds(10),[&]{return pause.entered;}));}
        if(action==0)CHECK(f.update(g,op("exposure","milli_ev",2))==2);
        if(action==1)CHECK(pixaura_manual_cancel(&f.document,&g)==0);
        if(action==2)CHECK(pixaura_manual_interrupt(&f.document)==0);
        if(action==3){pixaura_document_handle proposal{};uint32_t changed=0;CHECK(pixaura_manual_commit(&f.document,&g,&f.base,bytes(id(++f.fresh)),32,&proposal,&changed)==0&&changed==1);CHECK(pixaura_document_release(&f.document,&proposal)==0);}
        {std::lock_guard<std::mutex> lock(pause.mutex);pause.proceed=true;pause.condition.notify_all();}worker.join();preview::test_observer(&f.raster,nullptr,nullptr);
        CHECK(result==13||result==3);CHECK(output.serial==0&&rendered==99);CHECK(pixaura_preview_current(&f.raster,&old,nullptr)==13);
        if(action!=2)CHECK(pixaura_manual_release(&f.document,&g)==0);
        auto b=f.begin("pixaura.sharpen");f.update(b,op("sharpen","milli_amount",500));const auto next=f.ticket(b,seq);CHECK(f.render(b,next,output,rendered)==0);CHECK(pixaura_manual_preview_current(&f.document,&b,&f.base,seq,&old,&output)==13);
        CHECK(pixaura_preview_release(&f.raster,&output)==0&&pixaura_manual_cancel(&f.document,&b)==0&&pixaura_manual_cancel(&f.document,&b)==0&&pixaura_manual_release(&f.document,&b)==0);
    }
    for(unsigned round=0;round<64;++round)for(bool commit:{false,true}){
        auto g=f.begin();f.update(g,op("exposure","milli_ev",1));std::atomic<bool> start{false};uint64_t sequence=0;int update=-1,terminal=-1;uint32_t changed=0;pixaura_document_handle proposal{};const auto request=op("exposure","milli_ev",2),revision=id(++f.fresh);
        std::thread updater([&]{while(!start.load())std::this_thread::yield();update=pixaura_manual_update(&f.document,&g,&f.base,bytes(request),request.size(),&sequence);});
        std::thread closer([&]{while(!start.load())std::this_thread::yield();terminal=commit?pixaura_manual_commit(&f.document,&g,&f.base,bytes(revision),32,&proposal,&changed):pixaura_manual_cancel(&f.document,&g);});
        start=true;updater.join();closer.join();CHECK((update==0||update==13)&&terminal==0);
        if(commit){CHECK(changed==1);const auto d=document::deserialize(f.serialize(proposal),f.session);CHECK(d.code==0&&d.value->revisions().size()==2);const auto value=std::get<document::Exposure>(d.value->operations()[0].parameters).milli_ev;CHECK(value==(update==0?2:1));CHECK(pixaura_document_release(&f.document,&proposal)==0);}
        CHECK(pixaura_manual_release(&f.document,&g)==0);CHECK(f.serialize(f.base)==f.manifest);
    }
}
static void faults(Fixture& f){
    unsigned failures=0;
    for(unsigned boundary=0;boundary<4;++boundary){bool completed=false;
        for(int64_t index=0;index<2048&&!completed;++index){pixaura_manual_gesture g{};uint64_t sequence=99;const auto request=op("exposure","milli_ev",2),gid=id(++f.fresh),revision=id(++f.fresh);pixaura_document_handle proposal{};uint32_t changed=99;pixaura_decode_handle result{};
            if(boundary!=0){g=f.begin();f.update(g,op("exposure","milli_ev",1));}
            uint64_t prior_sequence=0;pixaura_preview_ticket prior{};if(boundary!=0)prior=f.ticket(g,prior_sequence);
            injected=false;fail_after=index;int status=0;
            if(boundary==0)status=pixaura_manual_edit_begin(1,&f.document,&f.base,bytes(gid),32,reinterpret_cast<const uint8_t*>("pixaura.exposure"),16,nullptr,0,&f.raster,&f.original,&g);
            if(boundary==1)status=pixaura_manual_update(&f.document,&g,&f.base,bytes(request),request.size(),&sequence);
            if(boundary==2)status=pixaura_manual_commit(&f.document,&g,&f.base,bytes(revision),32,&proposal,&changed);
            if(boundary==3)status=f.render(g,prior,result,sequence);
            fail_after=-1;completed=!injected;
            CHECK(status==0||status==8);if(status==8){++failures;CHECK(proposal.serial==0&&changed==99&&result.serial==0&&sequence==99);
                if(boundary!=0){CHECK(f.state(g,1)==PIXAURA_MANUAL_ACTIVE);uint64_t retained=0;CHECK(f.ticket(g,retained).generation==prior.generation&&retained==1);}
            }
            if(g.serial){CHECK(pixaura_manual_cancel(&f.document,&g)==0);CHECK(pixaura_manual_release(&f.document,&g)==0);}
            if(proposal.serial)CHECK(pixaura_document_release(&f.document,&proposal)==0);
            if(result.serial)CHECK(pixaura_preview_release(&f.raster,&result)==0);
            CHECK(f.serialize(f.base)==f.manifest);
        }CHECK(completed);
    }
    auto g=f.begin();f.update(g,op("exposure","milli_ev",1));fail_after=0;injected=false;CHECK(pixaura_manual_cancel(&f.document,&g)==0);fail_after=-1;CHECK(!injected&&f.state(g,1)==PIXAURA_MANUAL_CANCELLED);CHECK(pixaura_manual_release(&f.document,&g)==0);
    // PRV1 counter overflow leaves pending state unchanged and retry/cancel explicit.
    g=f.begin();f.update(g,op("exposure","milli_ev",1));preview::test_generation(&f.raster,UINT64_MAX);uint64_t sequence=99;const auto request=op("exposure","milli_ev",2);CHECK(pixaura_manual_update(&f.document,&g,&f.base,bytes(request),request.size(),&sequence)==8&&sequence==99);CHECK(f.state(g,1)==PIXAURA_MANUAL_ACTIVE);CHECK(pixaura_manual_release(&f.document,&g)==0);preview::test_generation(&f.raster,0);
    std::printf("gesture allocation failures recovered=%u\n",failures);
}
static void stale_and_bounds(Fixture& f){
    auto g=f.begin();f.update(g,op("exposure","milli_ev",1));uint64_t seq=0;const auto old=f.ticket(g,seq);
    // A detached proposal is not current automatically; explicitly present changed revision.
    pixaura_document_handle stale{};const auto other=id(++f.fresh);CHECK(pixaura_document_open(1,&f.document,bytes(f.manifest),f.manifest.size(),bytes(other),32,&stale,nullptr)==0);
    uint32_t changed=99;pixaura_document_handle proposal{};CHECK(pixaura_manual_commit(&f.document,&g,&stale,bytes(id(++f.fresh)),32,&proposal,&changed)==9&&proposal.serial==0&&changed==99);CHECK(f.state(g,1)==PIXAURA_MANUAL_INVALIDATED&&pixaura_preview_current(&f.raster,&old,nullptr)==13);CHECK(pixaura_manual_release(&f.document,&g)==0&&pixaura_document_release(&f.document,&stale)==0);
    pixaura_decode_handle substituted{};CHECK(pixaura_working_identity(&f.raster,&f.original,&f.limits,&substituted)==0);const auto gid=id(++f.fresh);
    CHECK(pixaura_manual_edit_begin(1,&f.document,&f.base,bytes(gid),32,reinterpret_cast<const uint8_t*>("pixaura.exposure"),16,nullptr,0,&f.raster,&substituted,&g)==3);CHECK(pixaura_decode_release(&f.raster,&substituted)==0);
    std::vector<pixaura_document_handle> handles;for(unsigned i=0;i<62;++i){pixaura_document_handle h{};CHECK(pixaura_document_open(1,&f.document,bytes(f.manifest),f.manifest.size(),bytes(f.session),32,&h,nullptr)==0);handles.push_back(h);}g=f.begin();f.update(g,op("exposure","milli_ev",1));const auto revision=id(++f.fresh);
    CHECK(pixaura_manual_commit(&f.document,&g,&f.base,bytes(revision),32,&proposal,&changed)==8);CHECK(f.state(g,1)==PIXAURA_MANUAL_ACTIVE);CHECK(pixaura_document_release(&f.document,&handles.back())==0);handles.pop_back();CHECK(pixaura_manual_commit(&f.document,&g,&f.base,bytes(revision),32,&proposal,&changed)==0&&changed==1);CHECK(pixaura_manual_release(&f.document,&g)==0&&pixaura_document_release(&f.document,&proposal)==0);for(const auto& h:handles)CHECK(pixaura_document_release(&f.document,&h)==0);
    CHECK(pixaura_manual_interrupt(&f.document)==0&&pixaura_manual_interrupt(&f.document)==0);
}
static void durable_history(Fixture& f){
    auto root=document::deserialize(f.manifest,f.session);CHECK(root.code==0);
    auto repository=std::make_unique<storage::Repository>(f.path);CHECK(repository->create(*root.value).epoch==1);
    repository->migrate(1,2);repository->migrate(2,3);
    float original_pixels[24];CHECK(pixaura_working_copy(&f.raster,&f.original,0,original_pixels,24)==0);
    auto g=f.begin();const auto operation=op("exposure","milli_ev",1250);f.update(g,operation);
    const auto revision=id(++f.fresh);pixaura_document_handle proposal{};uint32_t changed=0;
    CHECK(pixaura_manual_commit(&f.document,&g,&f.base,bytes(revision),32,&proposal,&changed)==0&&changed==1);
    const auto canonical=f.serialize(proposal);auto detached=document::deserialize(canonical,f.session);CHECK(detached.code==0);
    const auto loaded=repository->read();
    const auto command="{\"command_version\":1,\"kind\":\"commit\",\"expected_session_id\":\""+f.session+"\",\"expected_generation\":0,\"expected_revision_id\":\""+id(4)+"\",\"revision_id\":\""+revision+"\",\"actor\":\"manual\",\"plan_id\":null,\"operations\":"+operation.substr(operation.find('['),operation.rfind(']')-operation.find('[')+1)+",\"stack\":[\""+id(9000)+"\"]}";
    CHECK(repository->apply(command,loaded.epoch).epoch==2);CHECK(pixaura_manual_release(&f.document,&g)==0);
    // Persistence rejection cannot turn a completed gesture into another commit.
    CHECK(pixaura_manual_commit(&f.document,&g,&f.base,bytes(revision),32,&proposal,&changed)==3);
    repository.reset();repository=std::make_unique<storage::Repository>(f.path);const auto reopened=repository->read();
    const auto checkpoint=document::serialize(*reopened.snapshot);CHECK(checkpoint.code==0&&checkpoint.value==canonical&&reopened.snapshot->revisions().size()==2&&reopened.snapshot->source().sha256==root.value->source().sha256);
    const auto replay=document::replay(*reopened.snapshot,reopened.snapshot->current());CHECK(replay.code==0&&replay.value.size()==1&&std::get<document::Exposure>(replay.value[0].parameters).milli_ev==1250);
    pixaura_decode_handle first{},again{};CHECK(pixaura_working_evaluate(&f.raster,&f.original,1,bytes(operation),operation.size(),&f.limits,&first)==0);CHECK(pixaura_working_evaluate(&f.raster,&f.original,1,bytes(operation),operation.size(),&f.limits,&again)==0);
    float a[24],b[24],source[24];CHECK(pixaura_working_copy(&f.raster,&first,0,a,24)==0&&pixaura_working_copy(&f.raster,&again,0,b,24)==0&&std::memcmp(a,b,sizeof(a))==0);CHECK(pixaura_working_copy(&f.raster,&f.original,0,source,24)==0&&std::memcmp(source,original_pixels,sizeof(source))==0);
    CHECK(pixaura_decode_release(&f.raster,&first)==0&&pixaura_decode_release(&f.raster,&again)==0);
    auto after=f.begin("pixaura.sharpen",&proposal);CHECK(f.update(after,op("sharpen","milli_amount",500,9001),&proposal)==1);pixaura_document_handle second{};
    CHECK(pixaura_manual_commit(&f.document,&after,&proposal,bytes(id(++f.fresh)),32,&second,&changed)==0&&changed==1);const auto second_doc=document::deserialize(f.serialize(second),f.session);CHECK(second_doc.code==0&&second_doc.value->revisions().size()==3&&second_doc.value->revisions().back().stack[0].text()==id(9000)&&second_doc.value->revisions().back().stack[1].text()==id(9001));
    CHECK(pixaura_manual_release(&f.document,&after)==0&&pixaura_document_release(&f.document,&second)==0&&pixaura_document_release(&f.document,&proposal)==0);
}
static void switch_and_stale_revision(Fixture& f){
    auto a=f.begin();f.update(a,op("exposure","milli_ev",1));uint64_t seq=0;const auto old=f.ticket(a,seq);pixaura_decode_handle display{};CHECK(f.render(a,old,display,seq)==0);
    pixaura_manual_gesture b{};const auto gid=id(++f.fresh);
    CHECK(pixaura_manual_edit_switch(1,&f.document,&f.base,bytes(gid),32,reinterpret_cast<const uint8_t*>("pixaura.sharpen"),15,nullptr,0,&f.raster,&f.original,&b)==0);
    uint32_t state=99;CHECK(pixaura_manual_state(&f.document,&a,&state,&seq)==3&&pixaura_preview_current(&f.raster,&old,&display)==13);
    CHECK(f.update(b,op("sharpen","milli_amount",500))==1);CHECK(pixaura_preview_release(&f.raster,&display)==0);
    const auto next=f.ticket(b,seq);CHECK(f.render(b,next,display,seq)==0);CHECK(pixaura_manual_preview_current(&f.document,&b,&f.base,seq,&old,&display)==13);CHECK(pixaura_preview_release(&f.raster,&display)==0);
    // Present an independently built detached revision as the owner's new live binding.
    auto snapshot=document::deserialize(f.manifest,f.session);CHECK(snapshot.code==0);
    const auto change="{\"command_version\":1,\"kind\":\"commit\",\"expected_session_id\":\""+f.session+"\",\"expected_generation\":0,\"expected_revision_id\":\""+id(4)+"\",\"revision_id\":\""+id(++f.fresh)+"\",\"actor\":\"manual\",\"plan_id\":null,\"operations\":[],\"stack\":[]}";
    const auto changed=document::transition(*snapshot.value,change);CHECK(changed.code==0);const auto manifest=document::serialize(*changed.value);CHECK(manifest.code==0);
    pixaura_document_handle live{};CHECK(pixaura_document_open(1,&f.document,reinterpret_cast<const uint8_t*>(manifest.value.data()),manifest.value.size(),bytes(f.session),32,&live,nullptr)==0);
    pixaura_document_handle proposal{};uint32_t mutated=99;CHECK(pixaura_manual_commit(&f.document,&b,&live,bytes(id(++f.fresh)),32,&proposal,&mutated)==9&&proposal.serial==0&&mutated==99);CHECK(f.state(b,1)==PIXAURA_MANUAL_INVALIDATED&&pixaura_preview_current(&f.raster,&next,nullptr)==13);
    CHECK(pixaura_manual_release(&f.document,&b)==0&&pixaura_document_release(&f.document,&live)==0);
    // Wrong source digest rejects admission without creating a gesture.
    auto mismatch=f.manifest;const auto digest=mismatch.find("\"sha256\":\"")+10;CHECK(digest!=std::string::npos);mismatch[digest]=mismatch[digest]=='0'?'1':'0';
    CHECK(pixaura_document_open(1,&f.document,bytes(mismatch),mismatch.size(),bytes(f.session),32,&live,nullptr)==0);CHECK(pixaura_manual_edit_begin(1,&f.document,&live,bytes(id(++f.fresh)),32,reinterpret_cast<const uint8_t*>("pixaura.exposure"),16,nullptr,0,&f.raster,&f.original,&b)==9);CHECK(pixaura_document_release(&f.document,&live)==0);
    // Releasing the actual base while active deterministically invalidates and revokes.
    CHECK(pixaura_document_open(1,&f.document,bytes(f.manifest),f.manifest.size(),bytes(f.session),32,&live,nullptr)==0);auto pinned=f.begin("pixaura.exposure",&live);CHECK(f.update(pinned,op("exposure","milli_ev",1),&live)==1);
    pixaura_preview_ticket owned{};CHECK(pixaura_manual_preview_ticket(&f.document,&pinned,&live,&owned,&seq)==0);CHECK(pixaura_document_release(&f.document,&live)==0);CHECK(f.state(pinned,1)==PIXAURA_MANUAL_INVALIDATED&&pixaura_preview_current(&f.raster,&owned,nullptr)==13);CHECK(pixaura_manual_release(&f.document,&pinned)==0);
}
static void preset_interaction(Fixture& f){
    const auto operation=op("exposure","milli_ev",1000,90);
    const std::string recipe="{\"preset_id\":\"reference.gesture\",\"name\":\"Lifecycle reference\",\"schema_version\":1,\"recipe_version\":1,\"operations\":"+operation.substr(operation.find('['),operation.rfind(']')-operation.find('[')+1)+"}";
    const std::string bindings="{\"operation_ids\":[\""+id(9100)+"\"]}";const auto revision=id(++f.fresh);
    auto g=f.begin();f.update(g,op("exposure","milli_ev",1000));pixaura_document_handle preset{};uint32_t changed=99;
    CHECK(pixaura_preset_propose(1,&f.document,&f.base,&f.base,bytes(recipe),recipe.size(),bytes(bindings),bindings.size(),bytes(revision),32,&preset,&changed)==13&&preset.serial==0&&changed==99);
    CHECK(pixaura_manual_cancel(&f.document,&g)==0&&pixaura_manual_release(&f.document,&g)==0);
    CHECK(pixaura_preset_propose(1,&f.document,&f.base,&f.base,bytes(recipe),recipe.size(),bytes(bindings),bindings.size(),bytes(revision),32,&preset,&changed)==0&&changed==1);
    auto after=f.begin("pixaura.sharpen",&preset);CHECK(f.update(after,op("sharpen","milli_amount",500,9101),&preset)==1);pixaura_document_handle manual{};
    CHECK(pixaura_manual_commit(&f.document,&after,&preset,bytes(id(++f.fresh)),32,&manual,&changed)==0&&changed==1);
    const auto d=document::deserialize(f.serialize(manual),f.session);CHECK(d.code==0&&d.value->revisions().size()==3&&d.value->operations().size()==2&&d.value->revisions().back().stack[0].text()==id(9100)&&d.value->revisions().back().stack[1].text()==id(9101));
    CHECK(pixaura_manual_release(&f.document,&after)==0&&pixaura_document_release(&f.document,&manual)==0&&pixaura_document_release(&f.document,&preset)==0&&f.serialize(f.base)==f.manifest);
}
int main(int argc,char** argv){CHECK(argc==2);const auto root=fs::absolute(argv[1])/std::string("gesture-").append(std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    auto run=[&]{
    {Fixture f(root);baseline(f);preview_races(f);stale_and_bounds(f);switch_and_stale_revision(f);preset_interaction(f);durable_history(f);faults(f);
        {Fixture isolated(fs::path(f.path)/"isolation");auto a=f.begin(),b=isolated.begin();f.update(a,op("exposure","milli_ev",1));isolated.update(b,op("exposure","milli_ev",2));uint64_t seq=0;const auto ticket=isolated.ticket(b,seq);CHECK(pixaura_manual_interrupt(&f.document)==0);CHECK(pixaura_manual_preview_current(&isolated.document,&b,&isolated.base,seq,&ticket,nullptr)==0);CHECK(pixaura_manual_interrupt(&isolated.document)==0);}
        {Fixture stopped(fs::path(f.path)/"stopped");CHECK(pixaura_preview_stop(&stopped.raster)==0);pixaura_manual_gesture token{};
            CHECK(pixaura_manual_edit_begin(1,&stopped.document,&stopped.base,bytes(id(++stopped.fresh)),32,reinterpret_cast<const uint8_t*>("pixaura.exposure"),16,nullptr,0,&stopped.raster,&stopped.original,&token)==13&&token.serial==0);
            // Failed begin left no active controller: reference begin remains admissible.
            CHECK(pixaura_manual_begin(1,&stopped.document,&stopped.base,bytes(id(++stopped.fresh)),32,reinterpret_cast<const uint8_t*>("pixaura.exposure"),16,nullptr,0,&token)==0);CHECK(pixaura_manual_interrupt(&stopped.document)==0);}
        auto g=f.begin();f.update(g,op("exposure","milli_ev",1));uint64_t seq=0;const auto ticket=f.ticket(g,seq);CHECK(pixaura_document_context_destroy(&f.document)==0);CHECK(pixaura_preview_current(&f.raster,&ticket,nullptr)==13);const auto did=id(++f.fresh);CHECK(pixaura_document_context_init(1,&f.document,sizeof(f.document),bytes(did),32,nullptr)==0);
    }
    };
    run();const auto warmed_bytes=live_bytes.load(); // shared parser's immutable key tables initialize once.
    run();CHECK(live_bytes.load()==warmed_bytes);std::printf("gesture lifecycle checks=%u PASS\n",checks.load());return 0;
}
