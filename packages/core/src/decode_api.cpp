#include "decode.hpp"
#include "storage_files.hpp"
#include <map>
#include <mutex>
#include <cstring>
namespace {
using namespace pixaura;
void need(bool ok,int32_t code=1){if(!ok)throw document::Failure{code};}
template<class F> int32_t boundary(F&& fn)noexcept{try{fn();return 0;}catch(const document::Failure& e){return e.code;}catch(const std::bad_alloc&){return 8;}catch(...){return 14;}}
constexpr uint64_t live_tag=0x5049584445433031ULL,dead_tag=0x5049584445433030ULL;
struct Entry { std::shared_ptr<const decode::Source> source;std::unique_ptr<decode::Vector<uint8_t>> pixels;bool image=false; };
struct Registry { std::mutex mutex;pixaura_decode_limits limits;uint64_t next=1;std::map<uint64_t,Entry> entries; };
struct Header { uint64_t tag;Registry* registry;uint8_t identity[32]; };
static_assert(sizeof(Header)<=sizeof(pixaura_decode_context::opaque),"context layout");
Header read(pixaura_decode_context* c){need(c!=nullptr);Header h{};std::memcpy(&h,c->opaque,sizeof(h));return h;}
Header live(pixaura_decode_context* c){auto h=read(c);need(c->api_version==1&&c->struct_size==sizeof(*c)&&c->reserved==0&&h.tag==live_tag&&h.registry!=nullptr,3);return h;}
void write(pixaura_decode_context* c,const Header& h){std::memset(c,0,sizeof(*c));c->api_version=1;c->struct_size=sizeof(*c);std::memcpy(c->opaque,&h,sizeof(h));}
const Entry& entry(const Header& h,const pixaura_decode_handle* handle){need(handle!=nullptr);need(handle->api_version==1&&handle->struct_size==sizeof(*handle)&&handle->reserved==0&&std::memcmp(handle->context_id,h.identity,32)==0,3);auto it=h.registry->entries.find(handle->serial);need(it!=h.registry->entries.end(),3);return it->second;}
void room(const Registry& r,uint64_t extra){need(r.entries.size()<64&&r.next!=UINT64_MAX,8);uint64_t used=0;const decode::Source* seen[64]{};std::size_t count=0;
    for(const auto& pair:r.entries){const auto& e=pair.second;used+=e.pixels?e.pixels->capacity():0;bool duplicate=false;for(std::size_t i=0;i<count;++i)if(seen[i]==e.source.get())duplicate=true;
        if(!duplicate){seen[count++]=e.source.get();used+=e.source->encoded().capacity()+e.source->metadata().profile.capacity();}}
    need(used<=r.limits.context_bytes&&extra<=r.limits.context_bytes-used,8);
}
pixaura_decode_handle insert(const Header& h,Entry e){auto& r=*h.registry;const auto serial=r.next;r.entries.emplace(serial,std::move(e));++r.next;pixaura_decode_handle out{};out.api_version=1;out.struct_size=sizeof(out);out.serial=serial;std::memcpy(out.context_id,h.identity,32);return out;}
std::string_view text(const uint8_t* p,uint64_t n,uint64_t limit){need(p&&n&&n<=limit&&n<=SIZE_MAX);const std::string_view v(reinterpret_cast<const char*>(p),static_cast<std::size_t>(n));need(v.find('\0')==std::string_view::npos);return v;}
int32_t copy(pixaura_decode_context* c,const pixaura_decode_handle* handle,uint64_t offset,uint8_t* output,uint64_t bytes,bool pixels){return boundary([&]{const auto h=live(c);std::lock_guard<std::mutex> lock(h.registry->mutex);const auto& e=entry(h,handle);need(!pixels||e.image,3);const auto& v=pixels?*e.pixels:e.source->metadata().profile;need(offset<=v.size()&&bytes<=v.size()-offset&&(output||bytes==0));if(bytes)std::memcpy(output,v.data()+static_cast<std::size_t>(offset),static_cast<std::size_t>(bytes));});}
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
int32_t pixaura_decode_query(pixaura_decode_context* c,const pixaura_decode_handle* handle,pixaura_decode_metadata* output){return boundary([&]{need(output!=nullptr);const auto h=live(c);std::lock_guard<std::mutex> lock(h.registry->mutex);*output=entry(h,handle).source->metadata().value;});}
int32_t pixaura_decode_image(pixaura_decode_context* c,const pixaura_decode_handle* handle,pixaura_decode_handle* output){return boundary([&]{need(output!=nullptr);const auto h=live(c);std::lock_guard<std::mutex> lock(h.registry->mutex);const auto& e=entry(h,handle);need(!e.image,3);const auto& l=h.registry->limits;
    room(*h.registry,e.source->metadata().value.decoded_bytes+l.scratch_bytes);auto pixels=decode::execute(*e.source,l);const auto result=insert(h,{e.source,std::move(pixels),true});*output=result;
});}
int32_t pixaura_decode_copy_pixels(pixaura_decode_context* c,const pixaura_decode_handle* h,uint64_t offset,uint8_t* output,uint64_t bytes){return copy(c,h,offset,output,bytes,true);}
int32_t pixaura_decode_copy_profile(pixaura_decode_context* c,const pixaura_decode_handle* h,uint64_t offset,uint8_t* output,uint64_t bytes){return copy(c,h,offset,output,bytes,false);}
int32_t pixaura_decode_release(pixaura_decode_context* c,const pixaura_decode_handle* handle){return boundary([&]{const auto h=live(c);std::lock_guard<std::mutex> lock(h.registry->mutex);entry(h,handle);h.registry->entries.erase(handle->serial);});}
