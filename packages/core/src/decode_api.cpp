#include "decode.hpp"
#include "working.hpp"
#include "evaluation.hpp"
#include "geometry.hpp"
#include "preview.hpp"
#include "storage_files.hpp"
#include "sha256.hpp"
#include <map>
#include <mutex>
#include <cstring>
namespace {
using namespace pixaura;
void need(bool ok,int32_t code=1){if(!ok)throw document::Failure{code};}
template<class F> int32_t boundary(F&& fn)noexcept{try{fn();return 0;}catch(const document::Failure& e){return e.code;}catch(const std::bad_alloc&){return 8;}catch(...){return 14;}}
constexpr uint64_t live_tag=0x5049584445433031ULL,dead_tag=0x5049584445433030ULL;
struct Entry { std::shared_ptr<const decode::Source> source;std::unique_ptr<decode::Vector<uint8_t>> pixels;bool image=false;working::Image working{};preview::Image preview{};uint64_t preview_generation=0;bool original=false; };
struct Registry { std::mutex mutex, cancellation_mutex;pixaura_decode_limits limits;uint64_t next=1;std::map<uint64_t,Entry> entries;
    std::map<uint64_t,std::shared_ptr<evaluation::Cancellation>> cancellations;
    std::atomic<uint64_t> preview_generation{0};std::atomic<bool> preview_revoked{false};
    bool preview_stopped=false,preview_started=false;
#ifdef PIXAURA_PREVIEW_TESTING
    evaluation::Observer observer=nullptr;void* observer_state=nullptr;
#endif
};
static_assert(sizeof(pixaura_preview_ticket)==56,"ticket ABI v1");
static_assert(sizeof(std::atomic<uint64_t>)+sizeof(std::atomic<bool>)+2*sizeof(bool)<=32,"bounded interactive fields");
struct Header { uint64_t tag;Registry* registry;uint8_t identity[32]; };
static_assert(sizeof(Header)<=sizeof(pixaura_decode_context::opaque),"context layout");
Header read(pixaura_decode_context* c){need(c!=nullptr);Header h{};std::memcpy(&h,c->opaque,sizeof(h));return h;}
Header live(pixaura_decode_context* c){auto h=read(c);need(c->api_version==1&&c->struct_size==sizeof(*c)&&c->reserved==0&&h.tag==live_tag&&h.registry!=nullptr,3);return h;}
void write(pixaura_decode_context* c,const Header& h){std::memset(c,0,sizeof(*c));c->api_version=1;c->struct_size=sizeof(*c);std::memcpy(c->opaque,&h,sizeof(h));}
const Entry& entry(const Header& h,const pixaura_decode_handle* handle){need(handle!=nullptr);need(handle->api_version==1&&handle->struct_size==sizeof(*handle)&&handle->reserved==0&&std::memcmp(handle->context_id,h.identity,32)==0,3);auto it=h.registry->entries.find(handle->serial);need(it!=h.registry->entries.end(),3);return it->second;}
void room(Registry& r,uint64_t extra){std::lock_guard<std::mutex> guard(r.cancellation_mutex);need(r.entries.size()+r.cancellations.size()<64&&r.next!=UINT64_MAX,8);uint64_t used=0;const decode::Source* seen[64]{};std::size_t count=0;
    for(const auto& pair:r.entries){const auto& e=pair.second;used+=e.pixels?e.pixels->capacity():0;used+=e.preview.pixels?e.preview.pixels->capacity():0;used+=e.working.pixels?decode::multiply(e.working.pixels->capacity(),sizeof(float)):0;bool duplicate=false;for(std::size_t i=0;i<count;++i)if(seen[i]==e.source.get())duplicate=true;
        if(e.source&&!duplicate){seen[count++]=e.source.get();used+=e.source->encoded().capacity()+e.source->metadata().profile.capacity();}}
    need(used<=r.limits.context_bytes&&extra<=r.limits.context_bytes-used,8);
}
pixaura_decode_handle handle(const Header& h,uint64_t serial){pixaura_decode_handle out{};out.api_version=1;out.struct_size=sizeof(out);out.serial=serial;std::memcpy(out.context_id,h.identity,32);return out;}
pixaura_decode_handle insert_locked(const Header& h,Entry e){auto& r=*h.registry;const auto serial=r.next;r.entries.emplace(serial,std::move(e));++r.next;return handle(h,serial);}
pixaura_decode_handle insert(const Header& h,Entry e){std::lock_guard<std::mutex> guard(h.registry->cancellation_mutex);return insert_locked(h,std::move(e));}
std::shared_ptr<evaluation::Cancellation> cancellation_locked(const Header& h,const pixaura_decode_handle* token){
    need(token&&token->api_version==1&&token->struct_size==sizeof(*token)&&token->reserved==0&&std::memcmp(token->context_id,h.identity,32)==0,3);
    const auto it=h.registry->cancellations.find(token->serial);need(it!=h.registry->cancellations.end(),3);return it->second;
}
void ticket_valid(const Header& h,const pixaura_preview_ticket* t){
    need(t);need(t->version==1,2);
    need(t->struct_size==sizeof(*t)&&t->kind==PIXAURA_PREVIEW_TICKET_KIND&&t->reserved==0&&
        std::memcmp(t->context_id,h.identity,32)==0&&t->generation!=0&&
        t->generation<=h.registry->preview_generation.load(std::memory_order_acquire),3);
}
void eligible(const Header& h,const pixaura_preview_ticket* t){
    ticket_valid(h,t);need(!h.registry->preview_stopped&&!h.registry->preview_revoked.load(std::memory_order_acquire)&&
        t->generation==h.registry->preview_generation.load(std::memory_order_acquire),13);
}
std::string_view text(const uint8_t* p,uint64_t n,uint64_t limit){need(p&&n&&n<=limit&&n<=SIZE_MAX);const std::string_view v(reinterpret_cast<const char*>(p),static_cast<std::size_t>(n));need(v.find('\0')==std::string_view::npos);return v;}
int32_t copy(pixaura_decode_context* c,const pixaura_decode_handle* handle,uint64_t offset,uint8_t* output,uint64_t bytes,bool pixels){return boundary([&]{const auto h=live(c);std::lock_guard<std::mutex> lock(h.registry->mutex);const auto& e=entry(h,handle);need(!pixels||e.image,3);need(e.source!=nullptr,3);const auto& v=pixels?*e.pixels:e.source->metadata().profile;need(offset<=v.size()&&bytes<=v.size()-offset&&(output||bytes==0));if(bytes)std::memcpy(output,v.data()+static_cast<std::size_t>(offset),static_cast<std::size_t>(bytes));});}
}
int32_t pixaura_decode_default_limits(uint32_t version,pixaura_decode_limits* output){return boundary([&]{need(version==1,2);need(output!=nullptr);*output=decode::defaults();});}
int32_t pixaura_decode_context_init(uint32_t version,pixaura_decode_context* c,uint32_t bytes,const uint8_t* id,uint64_t n,const pixaura_decode_limits* l){return boundary([&]{
    need(version==1,2);need(c&&bytes>=sizeof(*c)&&l&&id&&n==32);decode::validate_limits(*l);for(std::size_t i=0;i<32;++i)need((id[i]>='0'&&id[i]<='9')||(id[i]>='a'&&id[i]<='f'));
    const auto previous=read(c);need(previous.tag==0||previous.tag==dead_tag);if(previous.tag==0){const pixaura_decode_context zero{};need(std::memcmp(c,&zero,sizeof(zero))==0);}else need(c->api_version==1&&c->struct_size==sizeof(*c)&&c->reserved==0&&std::memcmp(previous.identity,id,32)!=0);
    auto r=std::make_unique<Registry>();r->limits=*l;Header h{};h.tag=live_tag;h.registry=r.get();std::memcpy(h.identity,id,32);write(c,h);r.release();
});}
int32_t pixaura_decode_context_destroy(pixaura_decode_context* c){return boundary([&]{auto h=live(c);delete h.registry;h.registry=nullptr;h.tag=dead_tag;write(c,h);});}
int32_t pixaura_decode_open(pixaura_decode_context* c,const uint8_t* root,uint64_t root_n,const uint8_t* digest,uint64_t digest_n,uint64_t bytes,pixaura_decode_handle* output){return boundary([&]{
    need(output!=nullptr);const auto path=text(root,root_n,1024),hash=text(digest,digest_n,64);need(hash.size()==64);const auto h=live(c);std::lock_guard<std::mutex> lock(h.registry->mutex);const auto& l=h.registry->limits;
    need(bytes>0&&bytes<=l.encoded_bytes,bytes?8:18);room(*h.registry,bytes+3*l.profile_bytes+l.scratch_bytes);
    storage::Files files(path);auto encoded=files.read_verified({document::String(hash),bytes},l.encoded_bytes);auto metadata=decode::admit(encoded->data(),encoded->size(),l);
    auto source=std::make_shared<const decode::Source>(std::move(encoded),std::move(metadata));const auto result=insert(h,{source,{},false});*output=result;
});}
int32_t pixaura_decode_query(pixaura_decode_context* c,const pixaura_decode_handle* handle,pixaura_decode_metadata* output){return boundary([&]{need(output!=nullptr);const auto h=live(c);std::lock_guard<std::mutex> lock(h.registry->mutex);const auto& e=entry(h,handle);need(e.source!=nullptr,3);*output=e.source->metadata().value;});}
int32_t pixaura_decode_image(pixaura_decode_context* c,const pixaura_decode_handle* handle,pixaura_decode_handle* output){return boundary([&]{need(output!=nullptr);const auto h=live(c);std::lock_guard<std::mutex> lock(h.registry->mutex);const auto& e=entry(h,handle);need(!e.image&&e.source!=nullptr,3);const auto& l=h.registry->limits;
    need(!e.working.pixels,3);room(*h.registry,e.source->metadata().value.decoded_bytes+l.scratch_bytes);auto pixels=decode::execute(*e.source,l);const auto result=insert(h,{e.source,std::move(pixels),true});*output=result;
});}
int32_t pixaura_decode_copy_pixels(pixaura_decode_context* c,const pixaura_decode_handle* h,uint64_t offset,uint8_t* output,uint64_t bytes){return copy(c,h,offset,output,bytes,true);}
int32_t pixaura_decode_copy_profile(pixaura_decode_context* c,const pixaura_decode_handle* h,uint64_t offset,uint8_t* output,uint64_t bytes){return copy(c,h,offset,output,bytes,false);}
int32_t pixaura_decode_release(pixaura_decode_context* c,const pixaura_decode_handle* handle){return boundary([&]{const auto h=live(c);std::lock_guard<std::mutex> lock(h.registry->mutex);std::lock_guard<std::mutex> publication(h.registry->cancellation_mutex);entry(h,handle);h.registry->entries.erase(handle->serial);});}
int32_t pixaura_working_default_limits(uint32_t version,pixaura_working_limits* out){return boundary([&]{need(version==1,2);need(out!=nullptr);*out=working::defaults();});}
int32_t pixaura_working_normalize(pixaura_decode_context* c,const pixaura_decode_handle* handle,const pixaura_working_limits* l,pixaura_decode_handle* out){return boundary([&]{
    need(l&&out);const auto h=live(c);std::lock_guard<std::mutex> lock(h.registry->mutex);const auto& e=entry(h,handle);need(e.image&&e.pixels,3);
    const auto m=working::layout(e.source->metadata(),*l);room(*h.registry,m.image_bytes);
    Entry result;result.source=e.source;result.working=working::normalize(e.source->metadata(),e.pixels->data(),e.pixels->size(),*l);result.original=true;
    const auto token=insert(h,std::move(result));*out=token;
});}
int32_t pixaura_working_identity(pixaura_decode_context* c,const pixaura_decode_handle* handle,const pixaura_working_limits* l,pixaura_decode_handle* out){return boundary([&]{
    need(l&&out);const auto h=live(c);std::lock_guard<std::mutex> lock(h.registry->mutex);const auto& e=entry(h,handle);need(e.working.pixels!=nullptr,3);working::validate(*l);
    room(*h.registry,e.working.metadata.image_bytes);Entry result;result.source=e.source;result.working=working::identity(e.working,*l);const auto token=insert(h,std::move(result));*out=token;
});}
int32_t pixaura_working_query(pixaura_decode_context* c,const pixaura_decode_handle* handle,pixaura_working_metadata* out){return boundary([&]{need(out!=nullptr);const auto h=live(c);std::lock_guard<std::mutex> lock(h.registry->mutex);const auto& e=entry(h,handle);need(e.working.pixels!=nullptr,3);*out=e.working.metadata;});}
int32_t pixaura_working_validate_original(pixaura_decode_context* c,const pixaura_decode_handle* token,const uint8_t* digest,uint64_t bytes,uint64_t encoded_bytes){return boundary([&]{
    const auto hash=text(digest,bytes,64);need(hash.size()==64);const auto h=live(c);std::lock_guard<std::mutex> lock(h.registry->mutex);
    const auto& e=entry(h,token);need(e.original&&e.working.pixels&&e.source,3);
    need(e.source->encoded().size()==encoded_bytes,9);storage::Sha256 sha;sha.update(e.source->encoded().data(),e.source->encoded().size());need(sha.finish()==hash,9);
});}
int32_t pixaura_working_original_current(pixaura_decode_context* c,const pixaura_decode_handle* token){return boundary([&]{
    const auto h=live(c);std::lock_guard<std::mutex> publication(h.registry->cancellation_mutex);
    const auto& e=entry(h,token);need(e.original&&e.working.pixels&&e.source,3);
});}
int32_t pixaura_working_copy(pixaura_decode_context* c,const pixaura_decode_handle* handle,uint64_t offset,float* out,uint64_t count){return boundary([&]{const auto h=live(c);std::lock_guard<std::mutex> lock(h.registry->mutex);const auto& e=entry(h,handle);need(e.working.pixels!=nullptr,3);const auto& v=*e.working.pixels;need(offset<=v.size()&&count<=v.size()-offset&&(out||count==0));if(count)std::memcpy(out,v.data()+static_cast<std::size_t>(offset),static_cast<std::size_t>(count)*sizeof(float));});}
int32_t pixaura_evaluation_validate(uint32_t version,const uint8_t* request,uint64_t bytes){return boundary([&]{
    need(version==1,2);need(bytes<=PIXAURA_EVALUATION_MAX_REQUEST_BYTES,8);
    evaluation::parse(text(request,bytes,PIXAURA_EVALUATION_MAX_REQUEST_BYTES));
});}
int32_t pixaura_working_evaluate(pixaura_decode_context* c,const pixaura_decode_handle* handle,uint32_t version,
    const uint8_t* request,uint64_t bytes,const pixaura_working_limits* l,pixaura_decode_handle* out){return boundary([&]{
    need(version==1,2);need(l&&out);need(bytes<=PIXAURA_EVALUATION_MAX_REQUEST_BYTES,8);
    const auto h=live(c);std::lock_guard<std::mutex> lock(h.registry->mutex);
    const auto& e=entry(h,handle);need(e.working.pixels!=nullptr,3);working::validate(*l);
    // Reserve bounded parser/stack payload before parsing (no heap kernel scratch).
    // Conservative fixed 1 MiB covers at most 256 bounded integer envelopes.
    room(*h.registry,e.working.metadata.image_bytes+1048576);
    const auto stack=evaluation::parse(text(request,bytes,PIXAURA_EVALUATION_MAX_REQUEST_BYTES));
    room(*h.registry,evaluation::reservation(e.working.metadata.width,e.working.metadata.height,stack,*l)+1048576);
    Entry result;result.source=e.source;result.working=evaluation::evaluate(e.working,stack,*l);
    const auto token=insert(h,std::move(result));*out=token;
});}
int32_t pixaura_geometry_preflight(uint32_t version,uint32_t width,uint32_t height,const uint8_t* request,uint64_t bytes,
    const pixaura_working_limits* l,pixaura_geometry_plan* out){return boundary([&]{
    need(version==1,2);need(l&&out);need(bytes<=65536,8);
    geometry::admit({width,height},*l);
    const auto parsed=document::parse_evaluation(text(request,bytes,65536));need(parsed.code==0,parsed.code);
    const auto plan=geometry::plan({width,height},parsed.value,*l);*out=plan.summary;
});}
int32_t pixaura_geometry_tile_at(uint32_t version,uint32_t width,uint32_t height,uint32_t index,pixaura_geometry_tile* out){return boundary([&]{
    need(version==1,2);need(out);const auto r=geometry::tile({width,height},index);
    *out={1,sizeof(pixaura_geometry_tile),index,r.x0,r.y0,r.x1,r.y1};
});}
int32_t pixaura_cancel_create(pixaura_decode_context* c,uint32_t version,pixaura_decode_handle* out){return boundary([&]{
    need(version==1,2);need(out);const auto h=live(c);std::lock_guard<std::mutex> lock(h.registry->mutex);room(*h.registry,0);
    auto token=std::make_shared<evaluation::Cancellation>();std::lock_guard<std::mutex> guard(h.registry->cancellation_mutex);
    const auto serial=h.registry->next;h.registry->cancellations.emplace(serial,std::move(token));++h.registry->next;*out=handle(h,serial);
});}
int32_t pixaura_cancel_signal(pixaura_decode_context* c,const pixaura_decode_handle* token){return boundary([&]{
    const auto h=live(c);std::lock_guard<std::mutex> guard(h.registry->cancellation_mutex);
    cancellation_locked(h,token)->requested.store(true,std::memory_order_release);
});}
int32_t pixaura_cancel_release(pixaura_decode_context* c,const pixaura_decode_handle* token){return boundary([&]{
    const auto h=live(c);std::lock_guard<std::mutex> guard(h.registry->cancellation_mutex);cancellation_locked(h,token);h.registry->cancellations.erase(token->serial);
});}
int32_t pixaura_working_evaluate_cancel(pixaura_decode_context* c,const pixaura_decode_handle* source,uint32_t version,
    const uint8_t* request,uint64_t bytes,const pixaura_working_limits* l,const pixaura_decode_handle* token,pixaura_decode_handle* out){return boundary([&]{
    need(version==1,2);need(l&&out);need(bytes<=65536,8);const auto h=live(c);
    std::shared_ptr<evaluation::Cancellation> cancel;
    {std::lock_guard<std::mutex> guard(h.registry->cancellation_mutex);cancel=cancellation_locked(h,token);}
    evaluation::checkpoint(cancel.get(),evaluation::Checkpoint::admission);
    std::lock_guard<std::mutex> lock(h.registry->mutex);
    evaluation::checkpoint(cancel.get(),evaluation::Checkpoint::admission);
    const auto& e=entry(h,source);need(e.working.pixels!=nullptr,3);working::validate(*l);
    room(*h.registry,e.working.metadata.image_bytes+1048576);
    const auto stack=evaluation::parse(text(request,bytes,65536));
    room(*h.registry,evaluation::reservation(e.working.metadata.width,e.working.metadata.height,stack,*l)+1048576);
    evaluation::checkpoint(cancel.get(),evaluation::Checkpoint::allocation);
    Entry result;result.source=e.source;result.working=evaluation::evaluate(e.working,stack,*l,cancel.get());
    // Publication and signal share this lock: whichever commits first wins.
    std::lock_guard<std::mutex> guard(h.registry->cancellation_mutex);
    evaluation::checkpoint(cancel.get(),evaluation::Checkpoint::publication);
    const auto published=insert_locked(h,std::move(result));*out=published;
});}

int32_t pixaura_preview_begin(pixaura_decode_context* c,uint32_t version,pixaura_preview_ticket* out){return boundary([&]{
    need(version==1,2);need(out);const auto h=live(c);std::lock_guard<std::mutex> guard(h.registry->cancellation_mutex);
    auto& r=*h.registry;need(!r.preview_stopped,13);const auto previous=r.preview_generation.load(std::memory_order_relaxed);need(previous!=UINT64_MAX,8);
    pixaura_preview_ticket t{};t.version=1;t.struct_size=sizeof(t);t.kind=PIXAURA_PREVIEW_TICKET_KIND;
    std::memcpy(t.context_id,h.identity,32);t.generation=previous+1;
    r.preview_generation.store(t.generation,std::memory_order_release);r.preview_revoked.store(false,std::memory_order_release);r.preview_started=false;*out=t;
});}
int32_t pixaura_preview_cancel(pixaura_decode_context* c,const pixaura_preview_ticket* t){return boundary([&]{
    const auto h=live(c);std::lock_guard<std::mutex> guard(h.registry->cancellation_mutex);ticket_valid(h,t);
    need(t->generation==h.registry->preview_generation.load(std::memory_order_acquire),13);
    h.registry->preview_revoked.store(true,std::memory_order_release);
});}
int32_t pixaura_preview_stop(pixaura_decode_context* c){return boundary([&]{
    const auto h=live(c);std::lock_guard<std::mutex> guard(h.registry->cancellation_mutex);
    h.registry->preview_stopped=true;h.registry->preview_revoked.store(true,std::memory_order_release);
});}
int32_t pixaura_preview_current(pixaura_decode_context* c,const pixaura_preview_ticket* t,const pixaura_decode_handle* result){return boundary([&]{
    need(t);const auto h=live(c);ticket_valid(h,t);
    if(result){std::lock_guard<std::mutex> lock(h.registry->mutex);const auto& e=entry(h,result);need(e.preview.pixels&&e.preview_generation==t->generation,3);
        std::lock_guard<std::mutex> guard(h.registry->cancellation_mutex);eligible(h,t);
    }else{std::lock_guard<std::mutex> guard(h.registry->cancellation_mutex);eligible(h,t);}
});}
int32_t pixaura_preview_render_interactive(pixaura_decode_context* c,const pixaura_decode_handle* source,
    const pixaura_preview_ticket* t,const pixaura_preview_request* request,pixaura_decode_handle* out){return boundary([&]{
    need(t&&request&&out);const auto h=live(c);evaluation::Cancellation cancel;
    {std::lock_guard<std::mutex> guard(h.registry->cancellation_mutex);eligible(h,t);need(!h.registry->preview_started,13);
        h.registry->preview_started=true;cancel.generation=&h.registry->preview_generation;cancel.revoked=&h.registry->preview_revoked;cancel.expected=t->generation;}
    #ifdef PIXAURA_PREVIEW_TESTING
    const auto observe=h.registry->observer;auto* state=h.registry->observer_state;
#else
    const evaluation::Observer observe=nullptr;void* state=nullptr;
#endif
    evaluation::checkpoint(&cancel,evaluation::Checkpoint::admission,observe,state);
    std::lock_guard<std::mutex> lock(h.registry->mutex);
    evaluation::checkpoint(&cancel,evaluation::Checkpoint::admission);
    const auto& e=entry(h,source);need(e.working.pixels!=nullptr,3);
    const auto metadata=preview::layout(e.working,*request);room(*h.registry,metadata.image_bytes);
    Entry result;result.preview=preview::render(e.working,*request,&cancel,observe,state);result.preview_generation=t->generation;
    std::lock_guard<std::mutex> guard(h.registry->cancellation_mutex);eligible(h,t);
    evaluation::checkpoint(&cancel,evaluation::Checkpoint::publication);
    const auto published=insert_locked(h,std::move(result));*out=published;
});}
int32_t pixaura_preview_create(pixaura_decode_context* c,const pixaura_decode_handle* source,uint32_t version,
    const pixaura_decode_handle* token,pixaura_decode_handle* out){return boundary([&]{
    need(version==1,2);need(out);const auto h=live(c);std::shared_ptr<evaluation::Cancellation> cancel;
    if(token){std::lock_guard<std::mutex> guard(h.registry->cancellation_mutex);cancel=cancellation_locked(h,token);}
    evaluation::checkpoint(cancel.get(),evaluation::Checkpoint::admission);
    std::lock_guard<std::mutex> lock(h.registry->mutex);
    const auto& e=entry(h,source);need(e.working.pixels!=nullptr,3);
    const auto metadata=preview::layout(e.working);room(*h.registry,metadata.image_bytes);
    Entry result;result.preview=preview::render(e.working,cancel.get());
    std::lock_guard<std::mutex> guard(h.registry->cancellation_mutex);
    evaluation::checkpoint(cancel.get(),evaluation::Checkpoint::publication);
    const auto published=insert_locked(h,std::move(result));*out=published;
});}
int32_t pixaura_preview_render(pixaura_decode_context* c,const pixaura_decode_handle* source,const pixaura_preview_request* request,
    const pixaura_decode_handle* token,pixaura_decode_handle* out){return boundary([&]{
    need(request&&out);const auto h=live(c);std::shared_ptr<evaluation::Cancellation> cancel;
    if(token){std::lock_guard<std::mutex> guard(h.registry->cancellation_mutex);cancel=cancellation_locked(h,token);}
    evaluation::checkpoint(cancel.get(),evaluation::Checkpoint::admission);
    std::lock_guard<std::mutex> lock(h.registry->mutex);
    const auto& e=entry(h,source);need(e.working.pixels!=nullptr,3);
    const auto metadata=preview::layout(e.working,*request);room(*h.registry,metadata.image_bytes);
    Entry result;result.preview=preview::render(e.working,*request,cancel.get());
    std::lock_guard<std::mutex> guard(h.registry->cancellation_mutex);
    evaluation::checkpoint(cancel.get(),evaluation::Checkpoint::publication);
    const auto published=insert_locked(h,std::move(result));*out=published;
});}
int32_t pixaura_preview_query(pixaura_decode_context* c,const pixaura_decode_handle* handle,pixaura_preview_metadata* out){return boundary([&]{
    need(out);const auto h=live(c);std::lock_guard<std::mutex> lock(h.registry->mutex);const auto& e=entry(h,handle);need(e.preview.pixels!=nullptr,3);*out=e.preview.metadata;
});}
int32_t pixaura_preview_copy(pixaura_decode_context* c,const pixaura_decode_handle* handle,uint64_t offset,uint8_t* out,uint64_t bytes){return boundary([&]{
    const auto h=live(c);std::lock_guard<std::mutex> lock(h.registry->mutex);const auto& e=entry(h,handle);need(e.preview.pixels!=nullptr,3);
    const auto& v=*e.preview.pixels;need(offset<=v.size()&&bytes<=v.size()-offset&&(out||bytes==0));
    if(bytes)std::memcpy(out,v.data()+static_cast<std::size_t>(offset),static_cast<std::size_t>(bytes));
});}
int32_t pixaura_preview_release(pixaura_decode_context* c,const pixaura_decode_handle* handle){return boundary([&]{
    const auto h=live(c);std::lock_guard<std::mutex> lock(h.registry->mutex);std::lock_guard<std::mutex> publication(h.registry->cancellation_mutex);const auto& e=entry(h,handle);need(e.preview.pixels!=nullptr,3);h.registry->entries.erase(handle->serial);
});}

#ifdef PIXAURA_PREVIEW_TESTING
namespace pixaura::preview {
pixaura_decode_handle test_working(pixaura_decode_context* c,working::Image image){const auto h=live(c);std::lock_guard<std::mutex> lock(h.registry->mutex);room(*h.registry,image.metadata.image_bytes);Entry e;e.working=std::move(image);return insert(h,std::move(e));}
void test_observer(pixaura_decode_context* c,evaluation::Observer observer,void* state){const auto h=live(c);h.registry->observer=observer;h.registry->observer_state=state;}
void test_generation(pixaura_decode_context* c,uint64_t value){const auto h=live(c);h.registry->preview_generation.store(value);}
void test_context_budget(pixaura_decode_context* c,uint64_t bytes){const auto h=live(c);need(bytes>0&&bytes<=decode::defaults().context_bytes);h.registry->limits.context_bytes=bytes;}
}
#endif
