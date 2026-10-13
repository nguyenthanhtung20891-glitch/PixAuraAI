#include "include/cross_tier_certification.h"
#include "pixaura/editor.h"
#include "../../src/storage.hpp"
#include "../../src/sha256.hpp"
#include "../capability_fixtures.h"
#include "../../../../tests/fixtures/cross-tier/fixtures.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "../../../../tests/fixtures/cross-tier/golden.h"

namespace {
namespace fs=std::filesystem;
using namespace pixaura;
struct Failed { int line; };
#define VERIFY(x) do { if (!(x)) { std::fprintf(stderr,"cross-tier %d: %s\n",__LINE__,#x); throw Failed{__LINE__}; } } while(false)
const uint8_t* bytes(std::string_view value) { return reinterpret_cast<const uint8_t*>(value.data()); }
std::string identity(uint64_t value) { char out[33];std::snprintf(out,sizeof(out),"%032llx",static_cast<unsigned long long>(value));return out; }
std::string digest(const uint8_t* input,std::size_t count) { storage::Sha256 sha;sha.update(input,count);return std::string(sha.finish()); }
std::string digest(std::string_view input) { return digest(bytes(input),input.size()); }
struct Scope {
    fs::path root;
    explicit Scope(const fs::path& parent):root(parent/("replay-suite-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()))) {
        VERIFY(fs::create_directory(root));
    }
    ~Scope() {
        std::error_code error;
        #ifdef _WIN32
        // Only this newly-created test suite. Windows immutable fixtures need
        // their read-only attribute cleared for test cleanup, never for editing.
        for(fs::recursive_directory_iterator it(root,error),end;!error&&it!=end;it.increment(error))
            if(it->is_regular_file(error))fs::permissions(it->path(),fs::perms::owner_write,fs::perm_options::add,error);
        #endif
        fs::remove_all(root,error);
    }
    std::string project(const char* name) { const auto path=root/name;VERIFY(fs::create_directory(path));return path.generic_string(); }
};
struct Input : storage::Reader {
    const uint8_t* data;std::size_t count,position=0;
    Input(const uint8_t* p,std::size_t n):data(p),count(n) {}
    std::size_t read(uint8_t* output,std::size_t size) override {
        size=std::min(size,count-position);std::memcpy(output,data+position,size);position+=size;return size;
    }
};
int64_t reader(void* raw,uint8_t* output,uint64_t count) {
    VERIFY(count<=65536);return static_cast<int64_t>(static_cast<Input*>(raw)->read(output,static_cast<std::size_t>(count)));
}
pixaura_capability_profile profile(unsigned tier,uint32_t platform,uint32_t backend=0,uint32_t available=1,uint32_t validated=0) {
    VERIFY(tier<3);pixaura_capability_profile result{};VERIFY(pixaura_capability_baseline(1,&result)==0);
    const auto& fixture=capability_fixtures[tier];result.hardware.platform=platform;
    result.hardware.logical_cpu_count=fixture.cpus;result.hardware.physical_memory_bytes=fixture.memory;
    result.hardware.provenance=PIXAURA_PROVENANCE_SYNTHETIC;result.hardware.gpu_backend=backend;
    result.hardware.gpu_availability=available;result.hardware.gpu_validation=validated;
    result.runtime={1,sizeof(pixaura_capability_dynamic),1,1,0,available,fixture.budget,1};
    VERIFY(pixaura_capability_validate(1,&result)==0);return result;
}
pixaura_policy_observation observation(const pixaura_capability_profile& p) {
    return {1,sizeof(pixaura_policy_observation),1,1,0,p.hardware.gpu_availability,p.hardware.gpu_validation,0,p.runtime.process_budget_bytes,1};
}
document::Snapshot snapshot(const std::string& root) { storage::Repository repository(root);return repository.read().snapshot; }
std::string canonical(const std::string& root) {
    const auto value=document::serialize(*snapshot(root));VERIFY(value.code==0&&value.value.size()<=65536);return std::string(value.value);
}
struct Frame { std::vector<uint8_t> rgba;uint32_t width=0,height=0; };
struct Editor {
    pixaura_editor_context context{};pixaura_capability_profile facts;
    pixaura_policy_observation readings;pixaura_execution_strategy strategy{};
    std::string root;std::recursive_mutex gate;
    Editor(std::string path,pixaura_capability_profile p,uint64_t seed,bool importing=false,bool large=false):facts(p),readings(observation(p)),root(std::move(path)) {
        const auto id=identity(seed);Input input(large?cross_tier_large:cross_tier_small,large?sizeof(cross_tier_large):sizeof(cross_tier_small));
        VERIFY(pixaura_editor_open_policy(1,&context,bytes(root),root.size(),bytes(id),32,importing?reader:nullptr,importing?&input:nullptr,importing?input.count:0,&facts,&readings,&strategy)==0);
        VERIFY(strategy.epoch==1&&strategy.runtime_cap_bytes<=UINT64_C(268435456));
    }
    ~Editor() { if(context.api_version){(void)pixaura_editor_stop(&context);(void)pixaura_editor_destroy(&context);} }
    std::string document() const {
        uint64_t required=0;auto* c=const_cast<pixaura_editor_context*>(&context);
        VERIFY(pixaura_test_editor_document(c,nullptr,0,&required)==0&&required<=65536);
        std::string result(static_cast<std::size_t>(required),' ');
        VERIFY(pixaura_test_editor_document(c,reinterpret_cast<uint8_t*>(result.data()),result.size(),&required)==0);return result;
    }
    document::Snapshot model() const { const auto value=document::deserialize(document(),identity(9999));VERIFY(value.code==0);return value.value; }
    unsigned count() const { return static_cast<unsigned>(model()->revisions().size()); }
    void select(uint32_t index,const std::vector<int32_t>& values) {
        std::lock_guard<std::recursive_mutex> lock(gate);
        VERIFY(pixaura_editor_select(&context,index)==0);VERIFY(pixaura_editor_update(&context,values.data(),static_cast<uint32_t>(values.size()))==0);
    }
    void action(uint32_t value) { std::lock_guard<std::recursive_mutex> lock(gate);VERIFY(pixaura_editor_action(&context,value)==0); }
    pixaura_editor_request request() {
        std::lock_guard<std::recursive_mutex> lock(gate);pixaura_editor_request value{};VERIFY(pixaura_editor_request_preview(&context,&value)==0);return value;
    }
    Frame render(const pixaura_editor_request& value,uint32_t edge=64) {
        Frame result;result.rgba.resize(static_cast<std::size_t>(edge)*edge*4);
        VERIFY(pixaura_editor_render(&context,&value,edge,edge,result.rgba.data(),result.rgba.size(),&result.width,&result.height)==0);
        result.rgba.resize(static_cast<std::size_t>(result.width)*result.height*4);return result;
    }
    Frame preview(uint32_t edge=64) { const auto value=request();auto frame=render(value,edge);VERIFY(current(value)==0);return frame; }
    int32_t current(const pixaura_editor_request& value) { std::lock_guard<std::recursive_mutex> lock(gate);return pixaura_editor_current(&context,&value); }
    void pressure(uint32_t thermal,uint32_t memory,uint32_t available=1) {
        std::lock_guard<std::recursive_mutex> lock(gate);readings.thermal=thermal;readings.memory_pressure=memory;
        readings.gpu_availability=available;++readings.sequence;
        VERIFY(pixaura_editor_policy_update(1,&context,&readings,&strategy)==0);
    }
    void edit(uint32_t index,std::vector<int32_t> values) {
        const auto before=count();select(index,values);(void)preview();action(1);VERIFY(count()==before+1);
    }
};

// Full-source CPU working result, not display-preview equality. Serialize IEEE
// float words big-endian to avoid host-memory byte order in reference artifacts.
std::string reference(const Editor& editor,uint64_t seed) {
    const auto& root=editor.root;const auto& facts=editor.facts;
    const auto doc=editor.model();auto stack=document::replay(*doc,doc->current());VERIFY(stack.code==0);
    const auto operations=document::serialize_evaluation(stack.value);VERIFY(operations.code==0);
    pixaura_decode_context context{};pixaura_decode_limits limits{};VERIFY(pixaura_decode_default_limits(1,&limits)==0);
    const auto id=identity(seed);VERIFY(pixaura_decode_context_init(1,&context,sizeof(context),bytes(id),32,&limits)==0);
    struct Destroy { pixaura_decode_context* p;~Destroy(){(void)pixaura_preview_stop(p);(void)pixaura_decode_context_destroy(p);} } destroy{&context};
    auto observed=observation(facts);pixaura_execution_strategy selected{};VERIFY(pixaura_policy_configure(1,&context,&facts,&observed,&selected)==0);
    pixaura_decode_handle source{},decoded{},original{},evaluated{};const auto& asset=doc->source();
    VERIFY(pixaura_decode_open(&context,bytes(root),root.size(),bytes(asset.sha256.text()),64,asset.byte_length,&source)==0);
    VERIFY(pixaura_decode_image(&context,&source,&decoded)==0);pixaura_working_limits working{};VERIFY(pixaura_working_default_limits(1,&working)==0);
    VERIFY(pixaura_working_normalize(&context,&decoded,&working,&original)==0);
    VERIFY(pixaura_decode_release(&context,&decoded)==0);VERIFY(pixaura_decode_release(&context,&source)==0);
    VERIFY(pixaura_working_evaluate(&context,&original,1,bytes(operations.value),operations.value.size(),&working,&evaluated)==0);
    pixaura_working_metadata metadata{};VERIFY(pixaura_working_query(&context,&evaluated,&metadata)==0);
    if(metadata.width==1024&&metadata.height==512) {
        pixaura_preview_request exact{1,sizeof(pixaura_preview_request),PIXAURA_PREVIEW_EXACT,0,0,0};
        pixaura_preview_ticket ticket{};pixaura_decode_handle display{};
        VERIFY(pixaura_preview_begin(&context,1,&ticket)==0);
        const auto status=pixaura_preview_render_interactive(&context,&evaluated,&ticket,&exact,&display);
        VERIFY(status==(selected.preview_edge<1024?8:0));
        if(status==0){pixaura_preview_metadata shape{};VERIFY(pixaura_preview_query(&context,&display,&shape)==0&&shape.width==1024&&shape.height==512);VERIFY(pixaura_preview_release(&context,&display)==0);}
        VERIFY(pixaura_preview_render(&context,&evaluated,&exact,nullptr,&display)==0);
        pixaura_preview_metadata shape{};VERIFY(pixaura_preview_query(&context,&display,&shape)==0&&shape.width==1024&&shape.height==512);
        VERIFY(pixaura_preview_release(&context,&display)==0);
        observed.memory_pressure=4;++observed.sequence;VERIFY(pixaura_policy_update(1,&context,&observed,&selected)==0&&selected.preview_edge==360);
        display={};display.api_version=99;VERIFY(pixaura_preview_begin(&context,1,&ticket)==0);
        VERIFY(pixaura_preview_render_interactive(&context,&evaluated,&ticket,&exact,&display)==8&&display.api_version==99);
        pixaura_preview_request fit{1,sizeof(pixaura_preview_request),PIXAURA_PREVIEW_FIT,1024,1024,0};
        VERIFY(pixaura_preview_begin(&context,1,&ticket)==0);VERIFY(pixaura_preview_render_interactive(&context,&evaluated,&ticket,&fit,&display)==0);
        VERIFY(pixaura_preview_query(&context,&display,&shape)==0&&shape.width==360&&shape.height==180);
        VERIFY(pixaura_preview_release(&context,&display)==0);
    }
    VERIFY(metadata.image_bytes<=UINT64_C(16777216));std::vector<float> pixels(static_cast<std::size_t>(metadata.image_bytes/4));
    VERIFY(pixaura_working_copy(&context,&evaluated,0,pixels.data(),pixels.size())==0);
    storage::Sha256 sha;
    auto word=[&](uint32_t value){const uint8_t data[]={static_cast<uint8_t>(value>>24),static_cast<uint8_t>(value>>16),static_cast<uint8_t>(value>>8),static_cast<uint8_t>(value)};sha.update(data,sizeof(data));};
    word(metadata.width);word(metadata.height);for(float value:pixels){uint32_t bits=0;std::memcpy(&bits,&value,sizeof(bits));word(bits);}
    return std::string(sha.finish());
}
struct Matrix {
    std::string text="profile_type\tprofile\ttier\tthermal\tpressure\tbackend\tavailability\tvalidation\tcpus\tphysical_memory_bytes\tprocess_budget_bytes\tcap_bytes\tpreview_edge\treopen\trevisions\tsource_hash\tcanonical_hash\treference_hash\n";
    unsigned rows=0;
    void record(const char* label,const Editor& editor,const char* reopen,const std::string& ref) {
        VERIFY(++rows<=64);const auto& p=editor.facts;const auto& s=editor.strategy;const auto doc=editor.model();
        text+="SYNTHETIC POLICY EVIDENCE\t";text+=label;
        for(const auto value:{s.tier,editor.readings.thermal,editor.readings.memory_pressure,p.hardware.gpu_backend,editor.readings.gpu_availability,p.hardware.gpu_validation})text+='\t'+std::to_string(value);
        text+='\t'+std::to_string(p.hardware.logical_cpu_count)+'\t'+std::to_string(p.hardware.physical_memory_bytes)+'\t'+std::to_string(editor.readings.process_budget_bytes);
        text+='\t'+std::to_string(s.runtime_cap_bytes)+'\t'+std::to_string(s.preview_edge)+'\t'+reopen+'\t'+std::to_string(doc->revisions().size());
        text+='\t'+std::string(doc->source().sha256.text())+'\t'+digest(editor.document())+'\t'+ref+'\n';VERIFY(text.size()<=32768);
    }
    void rejected(const std::string& root,const pixaura_capability_profile& facts) {
        VERIFY(++rows<=64);const auto doc=snapshot(root);
        text+="SYNTHETIC POLICY EVIDENCE\tCRITICAL REJECTION\t0\t1\t4\t0\t1\t0\t"+std::to_string(facts.hardware.logical_cpu_count);
        text+='\t'+std::to_string(facts.hardware.physical_memory_bytes)+'\t'+std::to_string(facts.runtime.process_budget_bytes);
        text+="\t33554432\t360\tREJECTED_RESOURCE_8\t"+std::to_string(doc->revisions().size());
        text+='\t'+std::string(doc->source().sha256.text())+'\t'+digest(canonical(root))+"\tNOT_RENDERED_RESOURCE_REJECTION\n";VERIFY(text.size()<=32768);
    }
};
struct Checkpoint {
    std::mutex mutex;std::condition_variable changed;bool entered=false,released=false;
    static void pause(uint32_t point,void* raw) {
        if(point!=0)return;auto& value=*static_cast<Checkpoint*>(raw);std::unique_lock<std::mutex> lock(value.mutex);
        if(value.entered)return;value.entered=true;value.changed.notify_all();value.changed.wait(lock,[&]{return value.released;});
    }
    void wait() { std::unique_lock<std::mutex> lock(mutex);changed.wait(lock,[&]{return entered;}); }
    void release() { std::lock_guard<std::mutex> lock(mutex);released=true;changed.notify_all(); }
};
void verify_source(const Editor& editor,bool large=false) {
    const auto doc=editor.model();VERIFY(doc->source().sha256.text()==(large?cross_tier_large_sha:cross_tier_small_sha));
    storage::AssetStore assets(editor.root);assets.verify({doc->source().sha256.text(),doc->source().byte_length});
}
void write(const fs::path& path,const std::string& value) { VERIFY(value.size()<=65536);std::ofstream out(path,std::ios::binary|std::ios::trunc);VERIFY(bool(out));out<<value;out.close();VERIFY(bool(out)); }

void certify(const fs::path& parent,uint32_t platform) {
    Scope suite(parent);Matrix matrix;const auto primary=suite.project("high-mid-low");
    std::string before_undo,redo_state,reference_hash;
    {
        Editor high(primary,profile(2,platform),1,true);
        VERIFY(high.strategy.tier==2&&high.strategy.runtime_cap_bytes==UINT64_C(268435456));
        (void)high.preview();high.edit(3,{750000,500000,125000,250000});high.edit(6,{1});
        high.edit(4,{1000});high.edit(2,{1000});high.edit(7,{1500});high.edit(0,{500});high.edit(9,{500});
        VERIFY(high.count()==8);before_undo=high.document();reference_hash=reference(high,100);
        high.select(4,{0});high.action(1);VERIFY(high.document()==before_undo);
        high.select(1,{200});high.action(0);VERIFY(high.document()==before_undo);
        matrix.record("SYNTHETIC HIGH",high,"CREATE_PASS",reference_hash);
        high.action(2);high.action(2);redo_state=high.document();VERIFY(high.model()->redo().size()==2);
    }
    for(unsigned tier:{1u,0u}) {
        Editor lower(primary,profile(tier,platform),200+tier);
        VERIFY(lower.strategy.tier==tier&&lower.document()==redo_state);
        lower.action(3);lower.action(3);VERIFY(lower.document()==before_undo);
        VERIFY(reference(lower,300+tier)==reference_hash);verify_source(lower);
        matrix.record(tier==1?"SYNTHETIC MID":"SYNTHETIC LOW",lower,"REOPEN_PASS",reference_hash);
        lower.action(2);lower.action(2);VERIFY(lower.document()==redo_state);
    }
    // The same canonical persisted artifact is produced and consumed by every
    // Android/Apple/host harness. It contains all branches/redo/ordered stacks.
    write(parent/"cross-tier-canonical.json",before_undo+'\n');
    write(parent/"cross-tier-redo.json",redo_state+'\n');
    write(parent/"cross-tier-reference.sha256",reference_hash+'\n');
    VERIFY(before_undo==cross_tier_golden_document&&redo_state==cross_tier_golden_redo);VERIFY(reference_hash==cross_tier_golden_reference);
    const auto imported=suite.project("peer-canonical");Input peer(cross_tier_small,sizeof(cross_tier_small));
    storage::AssetStore asset_store(imported);const auto asset=asset_store.ingest(peer,sizeof(cross_tier_small));VERIFY(asset.digest==cross_tier_small_sha);
    auto parsed=document::deserialize(cross_tier_golden_redo,identity(400));VERIFY(parsed.code==0);
    {storage::Repository repository(imported);repository.migrate(1,2);repository.migrate(2,3);repository.create(*parsed.value);}
    {Editor peer_editor(imported,profile(0,platform),401);VERIFY(peer_editor.document()==redo_state);peer_editor.action(3);peer_editor.action(3);
     VERIFY(peer_editor.document()==before_undo&&reference(peer_editor,402)==reference_hash);
     matrix.record("PEER CANONICAL LOW",peer_editor,"IMPORT_REOPEN_PASS",reference_hash);}
    {
        Editor low(primary,profile(0,platform),500);low.action(3);low.action(3);low.action(2);
        low.edit(1,{200});VERIFY(low.count()==9&&low.model()->redo().empty());
        const auto branch=low.document();low.action(2);VERIFY(low.model()->redo().size()==1);
        low.action(3);VERIFY(low.document()==branch);low.action(2);
    }
    const auto branch_redo=canonical(primary);
    {Editor high(primary,profile(2,platform),501);VERIFY(high.document()==branch_redo);high.action(3);VERIFY(high.count()==9);}

    const auto reverse=suite.project("low-high");std::string reverse_document,reverse_reference;
    {Editor low(reverse,profile(0,platform),600,true);VERIFY(low.strategy.tier==0&&low.strategy.runtime_cap_bytes==UINT64_C(100663296));
     (void)low.preview();low.edit(3,{750000,500000,125000,250000});low.edit(6,{1});low.edit(4,{1000});
     reverse_document=low.document();low.select(4,{-1000});low.action(0);VERIFY(low.document()==reverse_document);
     low.action(2);low.action(3);VERIFY(low.document()==reverse_document);reverse_reference=reference(low,601);
     matrix.record("SYNTHETIC LOW",low,"T0_MANUAL_FLOW_PASS",reverse_reference);}
    {Editor high(reverse,profile(2,platform),602);VERIFY(high.document()==reverse_document&&reference(high,603)==reverse_reference);
     verify_source(high);matrix.record("SYNTHETIC HIGH",high,"LOW_HIGH_PASS",reverse_reference);}

    const std::array<std::array<uint32_t,2>,6> orders={{{3,6},{6,3},{4,2},{2,4},{0,9},{9,0}}};
    std::array<std::string,6> order_results;
    for(std::size_t index=0;index<orders.size();++index) {
        const auto name="order-"+std::to_string(index),path=suite.project(name.c_str());std::string doc;
        {Editor low(path,profile(0,platform),700+index,true);
         for(const auto tool:orders[index])low.edit(tool,tool==3?std::vector<int32_t>{750000,500000,125000,250000}:std::vector<int32_t>{tool==6?1:tool==0||tool==9?500:1000});
         doc=low.document();order_results[index]=reference(low,800+index);}
        {Editor high(path,profile(2,platform),900+index);VERIFY(high.document()==doc&&reference(high,1000+index)==order_results[index]);
         matrix.record(name.c_str(),high,"ORDER_REOPEN_PASS",order_results[index]);}
    }
    VERIFY(order_results[0]!=order_results[1]&&order_results[2]!=order_results[3]);

    const auto pressure=suite.project("pressure"),isolated=suite.project("isolated");std::string pressure_doc,pressure_reference;
    {
        Editor high(pressure,profile(2,platform),1100,true),low(isolated,profile(0,platform),1101,true);
        const auto untouched=low.document();const auto independent=low.strategy;
        high.select(4,{1000});const auto before_pressure=high.document();const auto old=high.request();Checkpoint checkpoint;
        VERIFY(pixaura_test_editor_observe(&high.context,Checkpoint::pause,&checkpoint)==0);
        int32_t old_status=0;uint32_t old_width=99,old_height=99;
        std::thread rendering([&]{std::vector<uint8_t> out(64*64*4);old_status=pixaura_editor_render(&high.context,&old,64,64,out.data(),out.size(),&old_width,&old_height);});
        checkpoint.wait();
        high.pressure(1,3);VERIFY(high.strategy.tier==0&&high.current(old)==13&&high.document()==before_pressure);
        const auto newest=high.request();(void)high.render(newest);VERIFY(high.current(newest)==0&&high.current(old)==13);
        checkpoint.release();rendering.join();VERIFY(old_status==13&&old_width==99&&old_height==99);
        VERIFY(pixaura_test_editor_observe(&high.context,nullptr,nullptr)==0);
        VERIFY(low.document()==untouched&&low.strategy.epoch==independent.epoch);
        high.action(1);VERIFY(high.count()==2);pressure_doc=high.document();
        VERIFY(high.model()->operations().size()==1&&std::get<document::Exposure>(high.model()->operations()[0].parameters).milli_ev==1000);
        for(unsigned i=0;i<3;++i){high.pressure(1,1);VERIFY(high.strategy.tier==(i<2?0u:2u));}
        VERIFY(high.document()==pressure_doc);pressure_reference=reference(high,1102);matrix.record("HIGH PRESSURE RECOVERY",high,"PASS",pressure_reference);
    }
    {Editor low(pressure,profile(0,platform),1103);VERIFY(low.document()==pressure_doc);verify_source(low);}
    {
        storage::Repository repository(pressure);const auto before=repository.read();auto facts=profile(0,platform);auto observed=observation(facts);observed.memory_pressure=4;
        pixaura_editor_context rejected{};pixaura_execution_strategy sentinel{};sentinel.version=99;const auto id=identity(1104);
        VERIFY(pixaura_editor_open_policy(1,&rejected,bytes(pressure),pressure.size(),bytes(id),32,nullptr,nullptr,0,&facts,&observed,&sentinel)==8);
        const auto after=repository.read();VERIFY(rejected.api_version==0&&sentinel.version==99&&before.epoch==after.epoch);
        VERIFY(before.snapshot->session().id==after.snapshot->session().id&&before.snapshot->session().generation==after.snapshot->session().generation);
        VERIFY(canonical(pressure)==pressure_doc);
        const auto source=after.snapshot->source();storage::AssetStore assets(pressure);assets.verify({source.sha256.text(),source.byte_length});
        facts.hardware.logical_cpu_count=1025;observed=observation(facts);
        VERIFY(pixaura_editor_open_policy(1,&rejected,bytes(pressure),pressure.size(),bytes(id),32,nullptr,nullptr,0,&facts,&observed,&sentinel)==1);
        VERIFY(repository.read().epoch==before.epoch&&canonical(pressure)==pressure_doc);
        bool stale=false;try{(void)repository.open_session(identity(1105),before.epoch+1);}catch(const document::Failure& failure){stale=failure.code==9;}
        VERIFY(stale&&repository.read().epoch==before.epoch);
    }
    matrix.rejected(pressure,profile(0,platform));
    {Editor recovered(pressure,profile(0,platform),1106);VERIFY(recovered.document()==pressure_doc&&recovered.strategy.epoch==1&&reference(recovered,1107)==pressure_reference);
     matrix.record("FRESH LOW AFTER REJECTION",recovered,"PASS",pressure_reference);}

    const auto stress=suite.project("terminal-races");
    {Editor owner(stress,profile(2,platform),1150,true);
     for(unsigned i=0;i<16;++i){const auto prior=owner.count();owner.select(4,{1000});std::atomic<unsigned> failures{0};
        std::thread updating([&]{try{owner.pressure(1,3);}catch(...){++failures;}});
        std::thread terminal([&]{try{owner.action(i%2==0?1u:0u);}catch(...){++failures;}});
        updating.join();terminal.join();VERIFY(failures==0&&owner.count()==prior+(i%2==0?1u:0u));verify_source(owner);
     }
     matrix.record("PRESSURE COMMIT CANCEL RACES",owner,"PASS",reference(owner,1151));}
    {
        Editor closing(stress,profile(0,platform),1152);const auto request=closing.request();Checkpoint checkpoint;
        VERIFY(pixaura_test_editor_observe(&closing.context,Checkpoint::pause,&checkpoint)==0);
        int32_t status=0;std::thread rendering([&]{std::vector<uint8_t> out(64*64*4);uint32_t w=0,h=0;status=pixaura_editor_render(&closing.context,&request,64,64,out.data(),out.size(),&w,&h);});
        checkpoint.wait();VERIFY(pixaura_editor_stop(&closing.context)==0);checkpoint.release();rendering.join();VERIFY(status==13);
        VERIFY(pixaura_editor_destroy(&closing.context)==0);uint64_t count=99;
        VERIFY(pixaura_test_editor_document(&closing.context,nullptr,0,&count)==3&&count==99);
    }

    // Optional backend preference differs, but every manual/reference result
    // remains CPU-authoritative. These are synthetic trust-state representations.
    const std::array<std::array<uint32_t,3>,4> backends={{{1,2,1},{1,2,0},{1,1,0},{2,2,0}}};
    for(std::size_t i=0;i<backends.size();++i){const auto b=backends[i];Editor owner(reverse,profile(2,b[0]==1?3u:4u,b[0],b[1],b[2]),1200+i);
        VERIFY(owner.document()==reverse_document&&reference(owner,1300+i)==reverse_reference);
        if(b[2]==0)VERIFY(owner.strategy.forced_cpu==1);
        if(b[2]==1){const auto old=owner.request();(void)owner.render(old);owner.pressure(1,1,1);VERIFY(owner.strategy.forced_cpu==1&&owner.current(old)==13);}
        auto bad=owner.readings;bad.gpu_validation=2;++bad.sequence;const auto prior=owner.document();
        VERIFY(pixaura_editor_policy_update(1,&owner.context,&bad,&owner.strategy)==1&&owner.document()==prior);
        matrix.record(i==3?"METAL UNCERTIFIED":"VULKAN SYNTHETIC SCOPE",owner,"BACKEND_REOPEN_PASS",reverse_reference);
    }
    for(unsigned iteration=0;iteration<12;++iteration){Editor owner(reverse,profile(iteration%3,platform),1400+iteration);
        VERIFY(owner.strategy.epoch==1&&owner.document()==reverse_document&&reference(owner,1500+iteration)==reverse_reference);}

    const auto legacy=suite.project("schema-compatibility");
    {Input original(cross_tier_small,sizeof(cross_tier_small));storage::AssetStore assets(legacy);(void)assets.ingest(original,sizeof(cross_tier_small));
     auto initial=document::deserialize(cross_tier_golden_root,identity(1550));VERIFY(initial.code==0);
     storage::Repository repository(legacy);repository.create(*initial.value);}
    for(uint32_t version:{1u,2u}){
        const auto before=[&]{storage::Repository repository(legacy);VERIFY(repository.version()==version);return repository.read();}();
        auto facts=profile(0,platform);auto observed=observation(facts);pixaura_editor_context rejected{};pixaura_execution_strategy output{};
        const auto id=identity(1550+version);
        VERIFY(pixaura_editor_open_policy(1,&rejected,bytes(legacy),legacy.size(),bytes(id),32,nullptr,nullptr,0,&facts,&observed,&output)==4);
        storage::Repository repository(legacy);
        VERIFY(repository.version()==version&&repository.read().epoch==before.epoch&&canonical(legacy)==cross_tier_golden_root);
        repository.migrate(version,version+1);VERIFY(canonical(legacy)==cross_tier_golden_root);
    }
    {Editor upgraded(legacy,profile(0,platform),1553);VERIFY(upgraded.document()==cross_tier_golden_root);verify_source(upgraded);
     matrix.record("EXPLICIT SCHEMA 1 2 3",upgraded,"PASS",reference(upgraded,1554));}

    const auto large=suite.project("fit-exact");
    std::string large_reference;
    {Editor high(large,profile(2,platform),1600,true,true);const auto image=high.preview(1024);VERIFY(image.width==1024&&image.height==512);verify_source(high,true);large_reference=reference(high,1602);}
    {Editor low(large,profile(0,platform),1601);const auto image=low.preview(1024);VERIFY(image.width==720&&image.height==360);
     VERIFY(reference(low,1603)==large_reference);
     low.pressure(1,4);const auto smaller=low.preview(1024);VERIFY(smaller.width==360&&smaller.height==180);verify_source(low,true);}
    write(parent/"cross-tier-matrix.tsv",matrix.text);
    std::printf("cross-tier canonical=%s source=%s reference=%s rows=%u platform=%u SYNTHETIC POLICY EVIDENCE PASS\n",digest(redo_state).c_str(),cross_tier_small_sha,reference_hash.c_str(),matrix.rows,platform);
}
}
extern "C" int32_t pixaura_test_cross_tier_replay(const char* parent,uint32_t platform) {
    try { VERIFY(parent&&(platform==0||platform==3||platform==4));const fs::path root=fs::canonical(parent);
        VERIFY(root.is_absolute()&&root.generic_string().size()<=800);certify(root,platform);return 0;
    } catch(const Failed& failure){return failure.line;}catch(const document::Failure& failure){std::fprintf(stderr,"cross-tier core status %d\n",failure.code);return 1;}
    catch(...){std::fputs("cross-tier unexpected failure\n",stderr);return 1;}
}
