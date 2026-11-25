#include "pixaura/editor_v2.h"
#include "context_v2.hpp"
#include "resource_admission_v2.hpp"
#include "../commerce/receipt_key_v2.hpp"
#include "../../storage_files.hpp"
#include "../../sha256.hpp"
#include "../../dynamic_policy.hpp"
#include "sodium.h"
#include <chrono>
#include <cstring>
#include <map>
#include <set>
#include <fstream>
#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define fd_dup _dup
#define fd_close _close
#define fd_read _read
#define fd_write _write
#define fd_stat _fstat64
#define stat_type struct _stat64
#define fd_seek _lseeki64
#else
#include <unistd.h>
#include <time.h>
#define fd_dup dup
#define fd_close close
#define fd_read read
#define fd_write write
#define fd_stat fstat
#define stat_type struct stat
#define fd_seek lseek
#endif
#ifdef __APPLE__
#include <sys/sysctl.h>
#include <mach/mach_time.h>
#endif
namespace {
using namespace pixaura;
void need(bool ok,int code=1){if(!ok)throw document::Failure{code};}
template<class F>int32_t call(uint32_t version,F&& f)noexcept{try{need(version==2,2);f();return 0;}catch(const document::Failure& e){return e.code;}catch(const std::bad_alloc&){return 8;}catch(...){return 12;}}
template<class T>void pointer(T* p){need(p&&reinterpret_cast<uintptr_t>(p)%alignof(T)==0);}
std::string text(const char* p,uint64_t n,uint64_t maximum){need(p&&n&&n<=maximum);need(!std::memchr(p,0,static_cast<size_t>(n)));return {p,static_cast<size_t>(n)};}
template<size_t N>void copy(char (&out)[N],const std::string& in){need(in.size()<N,8);std::memcpy(out,in.data(),in.size());}
struct NativeClock final:commerce::CommercialClockV2 {
    std::string boot;
    NativeClock(){
        std::string raw;
#ifdef __APPLE__
        char value[128]{};size_t size=sizeof(value);if(sysctlbyname("kern.bootsessionuuid",value,&size,nullptr,0)==0&&size>1&&size<=sizeof(value))raw.assign(value,strnlen(value,sizeof(value)));
#elif defined(__linux__)
        std::ifstream file("/proc/sys/kernel/random/boot_id");std::getline(file,raw);
#endif
        if(raw.empty()||raw.size()>128){static const std::string process_boot=[](){unsigned char random[16];randombytes_buf(random,sizeof random);return std::string(reinterpret_cast<char*>(random),sizeof random); }();raw=process_boot;}
        storage::Sha256 hash;hash.update(reinterpret_cast<const uint8_t*>(raw.data()),raw.size());boot=hash.finish().substr(0,32);
    }
    commerce::Clock sample_os()override{
        int64_t uptime=0;
#ifdef __APPLE__
        mach_timebase_info_data_t scale{};need(mach_timebase_info(&scale)==KERN_SUCCESS&&scale.denom,12);
        const long double seconds=static_cast<long double>(mach_continuous_time())*scale.numer/scale.denom/1000000000.L;need(seconds>=0&&seconds<static_cast<long double>(INT64_MAX),12);uptime=static_cast<int64_t>(seconds);
#elif defined(_WIN32)
        uptime=static_cast<int64_t>(GetTickCount64()/1000);
#else
        struct timespec now{};
#ifdef CLOCK_BOOTTIME
        need(clock_gettime(CLOCK_BOOTTIME,&now)==0,12);
#else
        need(clock_gettime(CLOCK_MONOTONIC,&now)==0,12);
#endif
        uptime=static_cast<int64_t>(now.tv_sec);
#endif
        return {std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count(),uptime,boot,false};
    }
};
struct RuntimeAuthority final:storage::ReceiptAuthorityV2 {
    std::unique_ptr<commerce::ReceiptKeyV2> secret;
    explicit RuntimeAuthority(const uint8_t* key){if(key)secret=std::make_unique<commerce::ReceiptKeyV2>(key,32);}
    document::String authenticate(std::string_view bytes)override{need(secret!=nullptr,3003);return secret->authenticate(bytes);}
};
struct Runtime {
    std::string root;RuntimeAuthority key;NativeClock clock;
    std::mutex publication_gate;policy::State policy{};
    const uint64_t cap;std::shared_ptr<tiled::Budget> budget;
    std::unique_ptr<tiled::Budget::Lease> control,store_control;
    std::unique_ptr<commerce::CommercialCoordinatorV2> commercial;
    std::shared_ptr<editor::ResourceAdmissionV2> admission;
    Runtime(std::string r,const uint8_t* k,const policy::State* observed=nullptr):root(std::move(r)),key(k),cap(134217728),budget(std::make_shared<tiled::Budget>(cap,0)){if(observed)policy=*observed;storage::Files private_root(root);(void)private_root;control.reset(new tiled::Budget::Lease(budget->reserve(1048576)));const auto effective=observed?observed->strategy.runtime_cap_bytes:cap;need(effective>=1048576,8);admission=std::make_shared<editor::ResourceAdmissionV2>(budget,cap,effective);if(k)try{commercial=std::make_unique<commerce::CommercialCoordinatorV2>(root+"/commercial-v2.sqlite",clock,key,3);}catch(...){/* recovery remains available */}}
};
struct WorkBinding;
thread_local WorkBinding* active_work=nullptr;
struct WorkBinding {
    std::shared_ptr<editor::ResourceAdmissionV2> owner;
    editor::ResourceAdmissionV2::Work work;
    WorkBinding* previous;WorkBinding* root;
    explicit WorkBinding(std::shared_ptr<editor::ResourceAdmissionV2> value):owner(std::move(value)),work(owner),previous(active_work),root(previous&&previous->owner==owner?previous->root:this){active_work=this;}
    ~WorkBinding(){active_work=previous;}
    WorkBinding(const WorkBinding&)=delete;WorkBinding& operator=(const WorkBinding&)=delete;
};
std::shared_ptr<void> admit_resources(const std::weak_ptr<editor::ResourceAdmissionV2>& weak){
    const auto owner=weak.lock();need(owner&&active_work&&active_work->owner==owner,14);
    auto* root=active_work->root;root->work.admit();return std::shared_ptr<void>(owner,root);
}
struct Entry {std::string root;std::shared_ptr<Runtime> runtime;std::unique_ptr<editor::EditorContextV2> editor;};
struct Frame {std::weak_ptr<Entry> entry;std::shared_ptr<Runtime> runtime;editor::FrameV2 pixels;std::unique_ptr<tiled::Budget::Lease> consumer;std::atomic<bool> copied{false};};
std::mutex registry_gate;
std::map<uint64_t,std::shared_ptr<Runtime>> runtimes;
std::map<uint64_t,std::shared_ptr<Entry>> editors;
std::map<uint64_t,std::shared_ptr<Frame>> frames;
std::set<std::string> open_roots;
struct OpenRoot {
    std::string root;bool installed=false;
    explicit OpenRoot(std::string value):root(std::move(value)){std::lock_guard<std::mutex> lock(registry_gate);need(open_roots.size()<8&&open_roots.insert(root).second,15);}
    ~OpenRoot(){if(!installed){std::lock_guard<std::mutex> lock(registry_gate);open_roots.erase(root);}}
};
uint64_t sequence=0;
uint64_t identity(){need(sequence!=UINT64_MAX,8);return ++sequence;}
template<class T>std::shared_ptr<T> lookup(const std::map<uint64_t,std::shared_ptr<T>>& map,uint64_t h){std::lock_guard<std::mutex> lock(registry_gate);const auto at=map.find(h);need(at!=map.end(),9);return at->second;}
struct EntryAccess {
    std::shared_ptr<Entry> data;WorkBinding binding;
    explicit EntryAccess(std::shared_ptr<Entry> value):data(std::move(value)),binding(data->runtime->admission){}
    Entry* operator->()const{return data.get();}
    operator std::shared_ptr<Entry>()const{return data;}
    friend bool operator==(const std::shared_ptr<Entry>& value,const EntryAccess& access){return value==access.data;}
};
EntryAccess lookup(const std::map<uint64_t,std::shared_ptr<Entry>>& source,uint64_t h){
    std::shared_ptr<Entry> entry;{std::lock_guard<std::mutex> lock(registry_gate);const auto at=source.find(h);need(at!=source.end(),9);entry=at->second;}return EntryAccess(std::move(entry));
}
void preview_bounds(const std::shared_ptr<Runtime>& owner,uint32_t width,uint32_t height){std::lock_guard<std::mutex> lock(owner->publication_gate);const auto edge=owner->policy.enabled?owner->policy.strategy.preview_edge:720;need(width<=edge&&height<=edge,8);}
struct Descriptor:storage::Reader {
    int fd=-1;
    explicit Descriptor(int original){need(original>=0);fd=fd_dup(original);need(fd>=0,12);}
    ~Descriptor(){if(fd>=0)fd_close(fd);}
    std::size_t read(uint8_t* out,std::size_t size)override{const auto n=::fd_read(fd,out,static_cast<unsigned>(std::min<std::size_t>(size,65536)));need(n>=0,12);return static_cast<size_t>(n);}
};
struct Sink:integration::StagedSinkV2 {
    Descriptor output;
    explicit Sink(int fd):output(fd){stat_type st{};need(fd_stat(output.fd,&st)==0,12);need((st.st_mode&S_IFMT)==S_IFREG&&st.st_size==0&&st.st_nlink==1);need(fd_seek(output.fd,0,SEEK_SET)==0,12);}
    void write(const uint8_t* p,size_t n)override{while(n){const auto written=::fd_write(output.fd,p,static_cast<unsigned>(std::min<size_t>(n,65536)));need(written>0,12);p+=written;n-=static_cast<size_t>(written);}}
    void seal()override{
#ifdef _WIN32
        need(_commit(output.fd)==0,12);
#else
        need(fsync(output.fd)==0,12);
#endif
    }
    void abort()noexcept override{
#ifdef _WIN32
        (void)_chsize_s(output.fd,0);
#else
        (void)ftruncate(output.fd,0);
#endif
    }
};
pixaura_v2_export_result exported(const editor::ExportObservationV2& result){pixaura_v2_export_result observed{};observed.version=2;observed.struct_size=sizeof(observed);observed.width=result.width;observed.height=result.height;observed.format=result.format;observed.bytes=result.bytes;observed.epoch=result.epoch;copy(observed.source,result.source);copy(observed.document,result.document);copy(observed.revision,result.revision);return observed;}
pixaura_v2_advisory advisory(const editor::AdvisoryViewV2& v){pixaura_v2_advisory out{};out.version=2;out.struct_size=sizeof(out);out.choice=v.choice;out.tool=v.tool;out.epoch=v.epoch;copy(out.id,v.id);copy(out.delivery,v.delivery);return out;}
pixaura_v2_preview frame(std::shared_ptr<Entry> entry,editor::FrameV2 value){auto owned=std::make_shared<Frame>();owned->entry=entry;owned->runtime=entry->runtime;owned->pixels=std::move(value);owned->consumer.reset(new tiled::Budget::Lease(owned->runtime->budget->reserve(owned->pixels.pixels.pixels.size()*4)));pixaura_v2_preview out{};out.version=2;out.struct_size=sizeof(out);out.width=owned->pixels.pixels.width;out.height=owned->pixels.pixels.height;out.source_width=owned->pixels.pixels.source.width;out.source_height=owned->pixels.pixels.source.height;out.bytes=owned->pixels.pixels.pixels.size();std::lock_guard<std::mutex> lock(registry_gate);need(frames.size()<64,8);out.frame=identity();frames.emplace(out.frame,std::move(owned));return out;}
}
extern "C" {
int32_t pixaura_v2_runtime_create(uint32_t v,const char* root,uint64_t bytes,const uint8_t* key,uint64_t key_bytes,uint64_t* out){return call(v,[&]{pointer(out);need(key&&key_bytes==32);const auto r=text(root,bytes,4095);need(sodium_init()>=0,12);std::lock_guard<std::mutex> lock(registry_gate);need(runtimes.size()<1,8);auto owner=std::make_shared<Runtime>(r,key);const auto h=identity();runtimes.emplace(h,std::move(owner));*out=h;});}
int32_t pixaura_v2_runtime_recovery_create(uint32_t v,const char* root,uint64_t bytes,const pixaura_capability_profile* profile,const pixaura_policy_observation* observation,pixaura_execution_strategy* strategy,uint64_t* out){return call(v,[&]{
    pointer(out);need((profile==nullptr)==(observation==nullptr));const auto r=text(root,bytes,4095);need(sodium_init()>=0,12);policy::State state;
    if(profile){pointer(strategy);const auto status=policy::configure(state,profile,observation,134217728);need(status==0,status);state.strategy.backend=0;state.strategy.forced_cpu=1;}
    std::lock_guard<std::mutex> lock(registry_gate);need(runtimes.empty(),8);auto owner=std::make_shared<Runtime>(r,nullptr,profile?&state:nullptr);const auto h=identity();runtimes.emplace(h,std::move(owner));if(profile)*strategy=state.strategy;*out=h;
});}
int32_t pixaura_v2_runtime_create_policy(uint32_t v,const char* root,uint64_t bytes,const uint8_t* key,uint64_t key_bytes,const pixaura_capability_profile* profile,const pixaura_policy_observation* observation,pixaura_execution_strategy* strategy,uint64_t* out){return call(v,[&]{
    pointer(out);pointer(strategy);pointer(profile);pointer(observation);need(key&&key_bytes==32);const auto r=text(root,bytes,4095);need(sodium_init()>=0,12);
    policy::State state;const auto status=policy::configure(state,profile,observation,134217728);need(status==0,status);state.strategy.backend=0;state.strategy.forced_cpu=1;
    std::lock_guard<std::mutex> lock(registry_gate);need(runtimes.empty(),8);auto owner=std::make_shared<Runtime>(r,key,&state);const auto h=identity();runtimes.emplace(h,std::move(owner));*strategy=state.strategy;*out=h;
});}
int32_t pixaura_v2_runtime_strategy(uint32_t v,uint64_t h,pixaura_execution_strategy* out){return call(v,[&]{pointer(out);auto owner=lookup(runtimes,h);std::lock_guard<std::mutex> lock(owner->publication_gate);need(owner->policy.enabled,1);*out=owner->policy.strategy;});}
int32_t pixaura_v2_runtime_update(uint32_t v,uint64_t h,const pixaura_policy_observation* observation,pixaura_execution_strategy* out){return call(v,[&]{
    pointer(observation);pointer(out);auto retained=lookup(runtimes,h);auto& owner=*retained;std::lock_guard<std::mutex> publication(owner.publication_gate);auto next=owner.policy;
    const auto status=policy::update(next,observation,134217728);need(status==0,status);next.strategy.backend=0;next.strategy.forced_cpu=1;
    // Capacity withholding enforces the approved cap without mutating the
    // frozen Budget or destroying a live manual gesture. Existing resources
    // remain accounted; new allocating work fails closed until it can fit.
    owner.admission->update(next.strategy.runtime_cap_bytes);
    owner.policy=next;*out=next.strategy;
});}
int32_t pixaura_v2_runtime_commercial_state(uint32_t v,uint64_t h,pixaura_v2_commercial_state* out){return call(v,[&]{pointer(out);auto owner=lookup(runtimes,h);need(owner->commercial!=nullptr,3003);const auto state=owner->commercial->state();pixaura_v2_commercial_state value{};value.version=2;value.struct_size=sizeof(value);value.access=static_cast<uint32_t>(state.access);value.clock_uncertain=state.clock_uncertain;value.photos=static_cast<uint32_t>(state.photos);value.ai=static_cast<uint32_t>(state.ai);value.pending=static_cast<uint32_t>(state.pending);value.day=state.day;value.logical_utc=state.logical_utc;value.trial_start=state.trial_start;value.trial_end=state.trial_end;*out=value;});}
int32_t pixaura_v2_runtime_close(uint32_t v,uint64_t h){return call(v,[&]{std::lock_guard<std::mutex> lock(registry_gate);auto at=runtimes.find(h);need(at!=runtimes.end(),9);need(at->second.use_count()==1,15);at->second->admission->close();runtimes.erase(at);});}
int32_t pixaura_v2_editor_open(uint32_t v,uint64_t runtime,const char* root,uint64_t bytes,int32_t input,uint64_t input_bytes,uint32_t migration,uint64_t* out){return call(v,[&]{
    pointer(out);need(migration<=1&&((input>=0)==(input_bytes>0)));auto owner=lookup(runtimes,runtime);WorkBinding resources(owner->admission);if(input_bytes)need(owner->commercial!=nullptr,3003);
    const auto path=text(root,bytes,4095);OpenRoot opening(path);auto entry=std::make_shared<Entry>();entry->root=path;entry->runtime=owner;
    std::map<uint64_t,std::shared_ptr<Entry>> prepared;uint64_t h=0;{std::lock_guard<std::mutex> lock(registry_gate);h=identity();}prepared.emplace(h,entry);auto node=prepared.extract(h);
    std::unique_ptr<Descriptor> reader;if(input>=0){reader=std::make_unique<Descriptor>(input);stat_type st{};need(fd_stat(reader->fd,&st)==0,12);need((st.st_mode&S_IFMT)==S_IFREG&&static_cast<uint64_t>(st.st_size)==input_bytes);need(fd_seek(reader->fd,0,SEEK_SET)==0,12);}
    entry->editor=std::make_unique<editor::EditorContextV2>(path,reader.get(),input_bytes,migration!=0,owner->budget,owner->commercial.get(),owner->key,[weak=std::weak_ptr<editor::ResourceAdmissionV2>(owner->admission)]{return admit_resources(weak);});
    // All allocating handle work precedes the success/charge fence. A pressure
    // stop before this fence rejects without consuming a source slot.
    std::lock_guard<std::mutex> publication(owner->publication_gate);
    if(input_bytes)entry->editor->activate_import();
    {std::lock_guard<std::mutex> lock(registry_gate);editors.insert(std::move(node));opening.installed=true;*out=h;}
});}

int32_t pixaura_v2_editor_stop(uint32_t v,uint64_t h){return call(v,[&]{lookup(editors,h)->editor->stop();});}
int32_t pixaura_v2_editor_close(uint32_t v,uint64_t h){return call(v,[&]{std::shared_ptr<Entry> entry;{std::lock_guard<std::mutex> lock(registry_gate);const auto at=editors.find(h);need(at!=editors.end(),9);entry=at->second;editors.erase(at);}entry->editor->stop();entry->editor->join();{std::lock_guard<std::mutex> lock(registry_gate);open_roots.erase(entry->root);}});}
int32_t pixaura_v2_editor_state(uint32_t v,uint64_t h,pixaura_v2_state* output){return call(v,[&]{pointer(output);const auto state=lookup(editors,h)->editor->state();pixaura_v2_state out{};out.version=2;out.struct_size=sizeof(out);out.revisions=state.revisions;out.selected=state.selected;out.epoch=state.epoch;out.generation=state.generation;out.pending=state.pending;out.can_undo=state.can_undo;out.can_redo=state.can_redo;out.recovery_only=state.recovery_only;out.commerce_unavailable=state.commerce_unavailable;need(state.values.size()<=4,14);out.value_count=static_cast<uint32_t>(state.values.size());std::copy(state.values.begin(),state.values.end(),out.values);copy(out.revision,state.revision);*output=out;});}
int32_t pixaura_v2_editor_edit_identity(uint32_t v,uint64_t h,char* out,uint64_t capacity){return call(v,[&]{need(out&&capacity>=33);const auto id=lookup(editors,h)->editor->gesture_identity();need(id.empty()||id.size()==32,14);char value[33]{};std::memcpy(value,id.data(),id.size());std::memcpy(out,value,33);});}
int32_t pixaura_v2_editor_history_json(uint32_t v,uint64_t h,uint8_t* out,uint64_t capacity,uint64_t* required){return call(v,[&]{pointer(required);need(capacity<=1048576&&(!capacity||out));const auto json=lookup(editors,h)->editor->history_json();*required=json.size();need(capacity>=json.size(),11);std::memcpy(out,json.data(),json.size());});}
int32_t pixaura_v2_editor_revision_preview(uint32_t v,uint64_t h,const char* revision,uint64_t n,uint32_t w,uint32_t height,pixaura_v2_preview* out){return call(v,[&]{pointer(out);auto entry=lookup(editors,h);preview_bounds(entry->runtime,w,height);*out=frame(entry,entry->editor->preview_revision(text(revision,n,32),w,height));});}
int32_t pixaura_v2_editor_select(uint32_t v,uint64_t h,uint32_t tool){return call(v,[&]{lookup(editors,h)->editor->select(tool);});}
int32_t pixaura_v2_editor_update(uint32_t v,uint64_t h,const int32_t* values,uint32_t n){return call(v,[&]{pointer(values);need(n&&n<=4);lookup(editors,h)->editor->update(values,n);});}
int32_t pixaura_v2_editor_apply(uint32_t v,uint64_t h,uint32_t* out){return call(v,[&]{pointer(out);*out=lookup(editors,h)->editor->human_apply()?1:0;});}
int32_t pixaura_v2_editor_cancel(uint32_t v,uint64_t h){return call(v,[&]{lookup(editors,h)->editor->cancel();});}
int32_t pixaura_v2_editor_history(uint32_t v,uint64_t h,uint32_t redo){return call(v,[&]{need(redo<=1);lookup(editors,h)->editor->history(redo!=0);});}
int32_t pixaura_v2_editor_preview(uint32_t v,uint64_t h,uint32_t w,uint32_t height,uint32_t original,pixaura_v2_preview* out){return call(v,[&]{pointer(out);need(original<=1);auto entry=lookup(editors,h);preview_bounds(entry->runtime,w,height);*out=frame(entry,entry->editor->preview(w,height,original!=0));});}
int32_t pixaura_v2_frame_copy(uint32_t v,uint64_t h,uint64_t f,uint8_t* out,uint64_t n){return call(v,[&]{need(out);auto entry=lookup(editors,h);auto pixels=lookup(frames,f);need(pixels->entry.lock()==entry&&entry->editor->current(pixels->pixels,true),9);preview_bounds(entry->runtime,pixels->pixels.pixels.width,pixels->pixels.pixels.height);need(n>=pixels->pixels.pixels.pixels.size(),11);bool unused=false;need(pixels->copied.compare_exchange_strong(unused,true),8);std::memcpy(out,pixels->pixels.pixels.pixels.data(),pixels->pixels.pixels.pixels.size());});}
int32_t pixaura_v2_frame_current(uint32_t v,uint64_t h,uint64_t f){return call(v,[&]{auto entry=lookup(editors,h);auto pixels=lookup(frames,f);need(pixels->entry.lock()==entry&&entry->editor->current(pixels->pixels),9);preview_bounds(entry->runtime,pixels->pixels.pixels.width,pixels->pixels.pixels.height);});}
int32_t pixaura_v2_frame_probe(uint32_t v,uint64_t h,uint64_t f,uint32_t wait,uint32_t* out){return call(v,[&]{
    pointer(out);need(wait<=1);std::shared_ptr<Entry> entry;std::shared_ptr<Frame> pixels;
    {std::unique_lock<std::mutex> registry(registry_gate,std::defer_lock);if(wait)registry.lock();else if(!registry.try_lock()){*out=1;return;}
     const auto e=editors.find(h);const auto p=frames.find(f);need(e!=editors.end()&&p!=frames.end(),9);entry=e->second;pixels=p->second;}
    need(pixels->entry.lock()==entry,9);const auto status=entry->editor->current_status(pixels->pixels,wait!=0);
    if(status==8){*out=1;return;}if(status==9){*out=2;return;}need(status==0,status);
    // Read an existing, already-reserved frame. Never allocate or admit work.
    // Keep policy checks, but never wait on the UI for a policy update either.
    std::unique_lock<std::mutex> policy(entry->runtime->publication_gate,std::defer_lock);if(wait)policy.lock();else if(!policy.try_lock()){*out=1;return;}
    const auto edge=entry->runtime->policy.enabled?entry->runtime->policy.strategy.preview_edge:720;
    need(pixels->pixels.pixels.width<=edge&&pixels->pixels.pixels.height<=edge,8);*out=0;
});}
int32_t pixaura_v2_frame_release(uint32_t v,uint64_t f){return call(v,[&]{std::lock_guard<std::mutex> lock(registry_gate);need(frames.erase(f)==1,9);});}
int32_t pixaura_v2_export_begin(uint32_t v,uint64_t h,uint32_t format,int32_t quality,uint64_t* out){return call(v,[&]{pointer(out);*out=lookup(editors,h)->editor->begin_export(format,quality);});}
int32_t pixaura_v2_export_cancel(uint32_t v,uint64_t h,uint64_t ticket){return call(v,[&]{lookup(editors,h)->editor->cancel_export(ticket);});}
int32_t pixaura_v2_export_release(uint32_t v,uint64_t h,uint64_t ticket){return call(v,[&]{lookup(editors,h)->editor->release_export(ticket);});}
int32_t pixaura_v2_export_run(uint32_t v,uint64_t h,uint64_t ticket,int32_t fd,pixaura_v2_export_result* out){return call(v,[&]{pointer(out);Sink sink(fd);*out=exported(lookup(editors,h)->editor->run_export(ticket,sink));});}
int32_t pixaura_v2_editor_export_observed(uint32_t v,uint64_t h,uint32_t format,int32_t quality,int32_t fd,pixaura_v2_export_result* out){return call(v,[&]{pointer(out);Sink sink(fd);*out=exported(lookup(editors,h)->editor->export_observed(format,quality,sink));});}
int32_t pixaura_v2_editor_export(uint32_t v,uint64_t h,uint32_t format,int32_t quality,int32_t fd,uint64_t* out){return call(v,[&]{pointer(out);Sink sink(fd);*out=lookup(editors,h)->editor->export_saved(format,quality,sink);});}
int32_t pixaura_v2_advisory_cancel(uint32_t v,uint64_t h,const char* id,uint64_t n){return call(v,[&]{lookup(editors,h)->editor->cancel_advisory(text(id,n,32));});}
int32_t pixaura_v2_advisory_begin(uint32_t v,uint64_t h,uint32_t choice,uint64_t* out){return call(v,[&]{pointer(out);*out=lookup(editors,h)->editor->begin_advisory(choice);});}
int32_t pixaura_v2_advisory_run(uint32_t v,uint64_t h,uint64_t ticket,pixaura_v2_advisory* out){return call(v,[&]{pointer(out);*out=advisory(lookup(editors,h)->editor->run_advisory(ticket));});}
int32_t pixaura_v2_advisory_request_cancel(uint32_t v,uint64_t h,uint64_t ticket){return call(v,[&]{lookup(editors,h)->editor->cancel_advisory_request(ticket);});}
int32_t pixaura_v2_advisory_request(uint32_t v,uint64_t h,uint32_t choice,pixaura_v2_advisory* out){return call(v,[&]{pointer(out);*out=advisory(lookup(editors,h)->editor->request_advisory(choice));});}
int32_t pixaura_v2_advisory_pending(uint32_t v,uint64_t h,uint8_t* out,uint64_t capacity,uint64_t* required){return call(v,[&]{pointer(required);need(capacity<=8192&&(!capacity||out));const auto tokens=lookup(editors,h)->editor->pending_advisories();std::string json="[";bool first=true;for(const auto& token:tokens){if(!first)json+=',';first=false;json+='"';json+=token;json+='"';}json+=']';need(json.size()<=8192,8);*required=json.size();need(capacity>=json.size(),11);std::memcpy(out,json.data(),json.size());});}
int32_t pixaura_v2_advisory_restore(uint32_t v,uint64_t h,const char* token,uint64_t n,pixaura_v2_advisory* out){return call(v,[&]{pointer(out);*out=advisory(lookup(editors,h)->editor->restore_advisory(text(token,n,64)));});}
int32_t pixaura_v2_editor_agent_bytes(uint32_t v,uint64_t h,uint32_t mode,const uint8_t* input,uint64_t n,uint8_t* out,uint64_t capacity,uint64_t* required){return call(v,[&]{pointer(required);need(n<=8192&&(!n||input)&&capacity<=8192&&(!capacity||out));const auto bytes=lookup(editors,h)->editor->agent_bytes(mode,{n?reinterpret_cast<const char*>(input):"",static_cast<size_t>(n)});*required=bytes.size();need(capacity>=bytes.size(),11);if(!bytes.empty())std::memcpy(out,bytes.data(),bytes.size());});}
int32_t pixaura_v2_advisory_plan_begin(uint32_t v,uint64_t h,const uint8_t* input,uint64_t n,uint64_t* out){return call(v,[&]{pointer(out);need(input&&n&&n<=8192);*out=lookup(editors,h)->editor->begin_plan_candidate({reinterpret_cast<const char*>(input),static_cast<size_t>(n)});});}
int32_t pixaura_v2_advisory_content(uint32_t v,uint64_t h,const char* id,uint64_t n,uint32_t kind,uint8_t* out,uint64_t capacity,uint64_t* required){return call(v,[&]{pointer(required);need(kind<=1&&capacity<=8192&&(!capacity||out));const auto value=lookup(editors,h)->editor->advisory_content(text(id,n,32),kind!=0);*required=value.size();need(capacity>=value.size(),11);if(!value.empty())std::memcpy(out,value.data(),value.size());});}
int32_t pixaura_v2_advisory_inspect(uint32_t v,uint64_t h,const char* id,uint64_t n,pixaura_v2_review_inspection* out){return call(v,[&]{pointer(out);*out=lookup(editors,h)->editor->inspect_review(text(id,n,32));});}
int32_t pixaura_v2_advisory_edit_command(uint32_t v,uint64_t h,const char* id,uint64_t n,uint32_t command,uint32_t action,uint32_t parameter,int32_t value,uint32_t destination,pixaura_v2_advisory* out){return call(v,[&]{pointer(out);*out=advisory(lookup(editors,h)->editor->edit_review_command(text(id,n,32),command,action,parameter,value,destination));});}
int32_t pixaura_v2_advisory_edit(uint32_t v,uint64_t h,const char* id,uint64_t n,const uint8_t* bytes,uint64_t size,pixaura_v2_advisory* out){return call(v,[&]{pointer(out);need(bytes&&size&&size<=8192);*out=advisory(lookup(editors,h)->editor->edit_advisory(text(id,n,32),{reinterpret_cast<const char*>(bytes),static_cast<size_t>(size)}));});}
int32_t pixaura_v2_advisory_reset(uint32_t v,uint64_t h,const char* id,uint64_t n,pixaura_v2_advisory* out){return call(v,[&]{pointer(out);*out=advisory(lookup(editors,h)->editor->restore_review(text(id,n,32)));});}
int32_t pixaura_v2_advisory_approve(uint32_t v,uint64_t h,const char* id,uint64_t n,char* out,uint64_t capacity){return call(v,[&]{need(out&&capacity>=33);const auto approval=lookup(editors,h)->editor->approve_from_human(text(id,n,32));need(approval.size()==32,14);std::memcpy(out,approval.c_str(),33);});}
int32_t pixaura_v2_advisory_apply(uint32_t v,uint64_t h,const char* id,uint64_t n,const char* approval,uint64_t size,uint32_t* out){return call(v,[&]{pointer(out);*out=lookup(editors,h)->editor->execute_approved(text(id,n,32),text(approval,size,32))?1:0;});}
int32_t pixaura_v2_advisory_apply_receipt(uint32_t v,uint64_t h,const char* id,uint64_t n,const char* approval,uint64_t size,pixaura_v2_commit_result* out){return call(v,[&]{
    pointer(out);const auto receipt=lookup(editors,h)->editor->execute_approved_receipt(text(id,n,32),text(approval,size,32));
    pixaura_v2_commit_result value{};value.version=2;value.struct_size=sizeof(value);value.kind=2;value.epoch=receipt.epoch();value.logical_utc=receipt.time().logical_utc;value.clock_uncertain=receipt.time().uncertain;value.utc_authenticated=receipt.time().utc_authenticated;
    copy(value.event,receipt.event());copy(value.source,receipt.source());copy(value.project,receipt.project());copy(value.document,receipt.document());copy(value.parent,receipt.parent());copy(value.revision,receipt.revision());copy(value.revision_digest,receipt.revision_digest());*out=value;
});}
int32_t pixaura_v2_review_preview_begin(uint32_t v,uint64_t h,const char* id,uint64_t n,uint32_t w,uint32_t height,uint64_t* out){return call(v,[&]{pointer(out);auto entry=lookup(editors,h);preview_bounds(entry->runtime,w,height);*out=entry->editor->begin_review_preview(text(id,n,32),w,height);});}
int32_t pixaura_v2_review_preview_cancel(uint32_t v,uint64_t h,uint64_t ticket){return call(v,[&]{lookup(editors,h)->editor->cancel_review_preview(ticket);});}
int32_t pixaura_v2_review_preview_release(uint32_t v,uint64_t h,uint64_t ticket){return call(v,[&]{lookup(editors,h)->editor->release_review_preview(ticket);});}
int32_t pixaura_v2_review_preview_render(uint32_t v,uint64_t h,uint64_t ticket,pixaura_v2_preview* before,pixaura_v2_preview* after){return call(v,[&]{
    pointer(before);pointer(after);const auto a=reinterpret_cast<uintptr_t>(before),b=reinterpret_cast<uintptr_t>(after);
    need((a>b?a-b:b-a)>=sizeof(*before));auto entry=lookup(editors,h);auto pair=entry->editor->render_review_preview(ticket);
    const auto first=frame(entry,std::move(pair[0]));
    try{const auto second=frame(entry,std::move(pair[1]));*before=first;*after=second;}
    catch(...){std::lock_guard<std::mutex> lock(registry_gate);frames.erase(first.frame);throw;}
});}
int32_t pixaura_v2_advisory_preview(uint32_t v,uint64_t h,const char* id,uint64_t n,uint32_t w,uint32_t height,pixaura_v2_preview* out){return call(v,[&]{pointer(out);auto entry=lookup(editors,h);preview_bounds(entry->runtime,w,height);*out=frame(entry,entry->editor->preview_advisory(text(id,n,32),w,height));});}
}

#ifdef PIXAURA_CERTIFICATION_TESTING
// Private debug symbol, intentionally absent from the public v2 C header and
// release binaries. It can corrupt/pause work, never manufacture a success.
extern "C" PIXAURA_API int32_t pixaura_test_editor_v2_control(uint64_t h,uint32_t family,uint32_t command,uint32_t value){
    try{
        auto entry=lookup(editors,h);
        if(family==4&&command==5){int32_t retained=0;std::lock_guard<std::mutex> lock(registry_gate);for(const auto& row:frames)if(row.second->entry.lock()==entry&&row.second->pixels.review_control)++retained;return retained;}
        return entry->editor->certification_control(family,command,value);
    }catch(const pixaura::document::Failure& e){return e.code;}catch(...){return 14;}
}
#endif

#include "store_ingress_v3.inc"
