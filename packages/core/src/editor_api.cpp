#include "pixaura/editor.h"
#include "storage.hpp"
#include "storage_files.hpp"
#include "sha256.hpp"
#include "manual_tools.hpp"
#include <array>
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <mutex>
#include <new>

namespace {
using namespace pixaura;
using document::String;
using document::Failure;
constexpr uint64_t tag=0x5049584544495431ULL;
// MSVC iterator-debug string moves allocate inside noexcept. Chained base
// string operator+ can select those moves even when operands are our throwing
// String wrapper. Append and return String so every allocation can propagate
// to the checked C boundary without terminating the process.
String assemble(std::initializer_list<std::string_view> parts){
    String result;for(const auto part:parts)result.append(part.data(),part.size());return result;
}
const uint8_t* byte_pointer(std::string_view s){return reinterpret_cast<const uint8_t*>(s.data());}
void check(int32_t code){if(code)throw Failure{code};}
void require(bool ok,int32_t code=1){if(!ok)throw Failure{code};}
template<class T>T good(document::Result<T> r){check(r.code);return std::move(r.value);}
template<class F>int32_t attempt(F&& f) noexcept {
    try{f();return 0;}catch(const Failure& e){return e.code;}
    catch(const std::bad_alloc&){return 8;}catch(...){return 14;}
}
struct Reader:storage::Reader {
    pixaura_editor_reader callback;void* state;
    Reader(pixaura_editor_reader c,void* s):callback(c),state(s){}
    std::size_t read(uint8_t* p,std::size_t n)override{
        const auto result=callback(state,p,n);require(result>=0&&static_cast<uint64_t>(result)<=n,12);
        return static_cast<std::size_t>(result);
    }
};
struct Service {
    std::mutex gate;
    String seed,root;uint64_t serial=0;
    pixaura_document_context document{};pixaura_decode_context raster{};
    pixaura_document_handle live{},proposal{};pixaura_decode_handle original{};
    pixaura_working_limits limits{};pixaura_manual_gesture gesture{};
    pixaura_preview_ticket approved_ticket{};
    std::unique_ptr<storage::Repository> repository;
    storage::Loaded loaded{};
    document::Snapshot pending_snapshot;
    String pending_command;
    const manual::Descriptor* selected=nullptr;
    std::array<int32_t,8> values{};
    bool stopped=false;
    Service(std::string_view r,std::string_view id):seed(id),root(r){}
    String fresh(){require(serial!=UINT64_MAX,8);auto s=assemble({seed,":",std::to_string(++serial)});
        storage::Sha256 hash;hash.update(byte_pointer(s),s.size());return hash.finish().substr(0,32);}
    ~Service(){if(document.api_version)(void)pixaura_document_context_destroy(&document);
        if(raster.api_version){(void)pixaura_preview_stop(&raster);(void)pixaura_decode_context_destroy(&raster);}}
    void ready(){require(!stopped,13);}
    void clear(){check(pixaura_manual_interrupt(&document));gesture={};selected=nullptr;
        if(proposal.serial){check(pixaura_document_release(&document,&proposal));proposal={};}
        pending_snapshot.reset();pending_command.clear();}
    void approved_preview(){check(pixaura_preview_begin(&raster,1,&approved_ticket));}
    String command(const char* kind){return assemble({"{\"command_version\":1,\"kind\":\"",kind,
        "\",\"expected_session_id\":\"",loaded.snapshot->session().id.text(),"\",\"expected_generation\":",
        std::to_string(loaded.snapshot->session().generation),",\"expected_revision_id\":\"",loaded.snapshot->current().text(),"\"}"});}
    document::Snapshot snapshot(const pixaura_document_handle& h){uint64_t n=0;
        check(pixaura_document_serialize(&document,&h,nullptr,0,&n,nullptr));String s(static_cast<std::size_t>(n),' ');
        check(pixaura_document_serialize(&document,&h,reinterpret_cast<uint8_t*>(s.data()),n,&n,nullptr));
        return good(document::deserialize(s,loaded.snapshot->session().id.text()));}
    void accept(){
        // All candidate/command/next-snapshot allocation precedes the durable write.
        // Saved reports checkpoint failure separately; a durable write is success.
        const auto saved=repository->apply(pending_command,loaded.epoch);
        const auto prior=live;live=proposal;proposal={};loaded.snapshot=std::move(pending_snapshot);loaded.epoch=saved.epoch;
        (void)pixaura_document_release(&document,&prior);
        pending_command.clear();selected=nullptr;
        if(gesture.serial){(void)pixaura_manual_release(&document,&gesture);}
        gesture={};
        // Durability already succeeded. Preview failure must not masquerade as
        // failed approval; request_preview issues the next generation separately.
        (void)pixaura_preview_begin(&raster,1,&approved_ticket);
    }
};
struct Header{uint64_t tag_value;Service* service;};
Service& owner(pixaura_editor_context* c){require(c&&c->api_version==1&&c->struct_size==sizeof(*c),3);
    Header h{};std::memcpy(&h,c->opaque,sizeof(h));require(h.tag_value==tag&&h.service,3);return *h.service;}
}
extern "C" int32_t pixaura_editor_open(uint32_t version,pixaura_editor_context* context,
    const uint8_t* root,uint64_t root_bytes,const uint8_t* identity,uint64_t identity_bytes,
    pixaura_editor_reader reader,void* reader_state,uint64_t import_bytes){return attempt([&]{
    require(version==1,2);require(context&&context->api_version==0&&root&&root_bytes&&root_bytes<=1024&&identity&&identity_bytes==32);
    require(!reader||import_bytes!=0,17);
    const std::string_view path(reinterpret_cast<const char*>(root),static_cast<std::size_t>(root_bytes));
    require(path.find('\0')==std::string_view::npos);const std::string_view seed(reinterpret_cast<const char*>(identity),32);
    (void)document::Id::parse(seed);auto s=std::make_unique<Service>(path,seed);
    pixaura_decode_limits dl{};check(pixaura_decode_default_limits(1,&dl));check(pixaura_working_default_limits(1,&s->limits));
    auto id=s->fresh();check(pixaura_decode_context_init(1,&s->raster,sizeof(s->raster),byte_pointer(id),32,&dl));
    id=s->fresh();check(pixaura_document_context_init(1,&s->document,sizeof(s->document),byte_pointer(id),32,nullptr));
    const auto session=s->fresh();storage::Asset asset;storage::AssetStore assets(path);
    if(import_bytes){
        require(reader&&import_bytes<=dl.encoded_bytes,8);
        require(!std::filesystem::exists(std::filesystem::u8path(path)/"catalog.sqlite"),9);
        Reader input(reader,reader_state);asset=assets.ingest(input,import_bytes);
    }else{
        s->repository=std::make_unique<storage::Repository>(path);require(s->repository->version()==3,4);
        s->loaded=s->repository->open_session(session);asset={s->loaded.snapshot->source().sha256.text(),s->loaded.snapshot->source().byte_length};
    }
    pixaura_decode_handle source{},decoded{};pixaura_decode_metadata metadata{};
    check(pixaura_decode_open(&s->raster,root,root_bytes,byte_pointer(asset.digest),64,asset.bytes,&source));
    check(pixaura_decode_query(&s->raster,&source,&metadata));check(pixaura_decode_image(&s->raster,&source,&decoded));
    require(metadata.format==PIXAURA_DECODE_PNG||metadata.format==PIXAURA_DECODE_JPEG,16);
    check(pixaura_working_normalize(&s->raster,&decoded,&s->limits,&s->original));
    check(pixaura_decode_release(&s->raster,&decoded));check(pixaura_decode_release(&s->raster,&source));
    if(import_bytes){const auto project=s->fresh(),document=s->fresh(),revision=s->fresh();
        const String manifest=assemble({"{\"schema_version\":1,\"project_id\":\"",project,"\",\"document_id\":\"",document,
            "\",\"current_revision_id\":\"",revision,"\",\"redo\":[],\"operations\":[],\"revisions\":[{\"id\":\"",revision,
            "\",\"parent_id\":null,\"actor\":\"import\",\"plan_id\":null,\"stack\":[]}],\"source\":{\"sha256\":\"",asset.digest,
            "\",\"byte_length\":",std::to_string(asset.bytes),",\"metadata\":{\"width\":",std::to_string(metadata.width),
            ",\"height\":",std::to_string(metadata.height),",\"orientation\":",std::to_string(metadata.orientation),
            ",\"codec\":\"",(metadata.format==PIXAURA_DECODE_PNG?"png":"jpeg"),"\",\"has_alpha\":",
            (metadata.has_alpha?"true":"false"),",\"icc_sha256\":null}}}"});
        auto initial=good(document::create_document(manifest,session));
        s->repository=std::make_unique<storage::Repository>(path);
        s->repository->migrate(1,2);s->repository->migrate(2,3);
        const auto saved=s->repository->create(*initial);s->loaded={std::move(initial),saved.epoch};
    }
    const auto manifest=good(document::serialize(*s->loaded.snapshot));
    check(pixaura_document_open(1,&s->document,byte_pointer(manifest),manifest.size(),byte_pointer(session),32,&s->live,nullptr));
    s->approved_preview();Header h{tag,s.release()};pixaura_editor_context out{};out.api_version=1;out.struct_size=sizeof(out);
    std::memcpy(out.opaque,&h,sizeof(h));*context=out;
});}
extern "C" int32_t pixaura_editor_select(pixaura_editor_context* c,uint32_t index){return attempt([&]{
    auto& s=owner(c);std::lock_guard<std::mutex> lock(s.gate);s.ready();require(index<manual::registry().size(),5);
    s.clear();const auto& d=manual::registry()[index];const auto id=s.fresh();
    check(pixaura_manual_edit_begin(1,&s.document,&s.live,byte_pointer(id),32,byte_pointer(d.tool_id),d.tool_id.size(),nullptr,0,&s.raster,&s.original,&s.gesture));
    s.selected=&d;for(std::size_t i=0;i<d.parameter_count;++i)s.values[i]=d.parameters[i].neutral;
});}
extern "C" int32_t pixaura_editor_update(pixaura_editor_context* c,const int32_t* values,uint32_t count){return attempt([&]{
    auto& s=owner(c);std::lock_guard<std::mutex> lock(s.gate);s.ready();require(s.selected&&!s.proposal.serial,13);
    const auto& d=*s.selected;require(values&&count==d.parameter_count,7);
    // Construct exact integer envelope; only the shared parser validates values.
    String parameters="{";for(uint32_t i=0;i<count;++i){if(i)parameters+=',';parameters+='"';parameters+=d.parameters[i].name;
        parameters+="\":";parameters+=std::to_string(values[i]);}parameters+='}';
    // One operation identity per gesture, never per update.
    storage::Sha256 hash;hash.update(s.gesture.context_id,32);hash.update(reinterpret_cast<const uint8_t*>(&s.gesture.serial),sizeof(s.gesture.serial));
    const auto id=hash.finish().substr(0,32);
    const String op=assemble({"{\"operations\":[{\"id\":\"",id,"\",\"type\":\"",d.type,"\",\"operation_version\":",
        std::to_string(d.operation_version),",\"parameter_version\":",std::to_string(d.parameter_version),",\"parameters\":",parameters,"}]}"});
    uint64_t sequence=0;check(pixaura_manual_update(&s.document,&s.gesture,&s.live,byte_pointer(op),op.size(),&sequence));
    std::copy_n(values,count,s.values.begin());
});}
extern "C" int32_t pixaura_editor_action(pixaura_editor_context* c,uint32_t action){return attempt([&]{
    auto& s=owner(c);std::lock_guard<std::mutex> lock(s.gate);s.ready();require(action<=4);
    if(action==0||action==4){s.clear();s.approved_preview();return;}
    if(action==1){require(s.selected,13);
        if(!s.proposal.serial){const auto revision=s.fresh();uint32_t changed=0;
            check(pixaura_manual_commit(&s.document,&s.gesture,&s.live,byte_pointer(revision),32,&s.proposal,&changed));
            if(!changed){s.proposal={};s.clear();s.approved_preview();return;}
        }
        // A storage retry reuses this exact detached proposal.
        auto proposed=s.snapshot(s.proposal);s.pending_command=good(document::serialize_commit(*s.loaded.snapshot,*proposed));
        s.pending_snapshot=good(document::transition(*s.loaded.snapshot,s.pending_command));
        try{s.accept();}catch(const Failure& e){if(e.code==9)s.clear();throw;}
    }else{require(!s.selected,13);const auto command=s.command(action==2?"undo":"redo");
        auto next=good(document::transition(*s.loaded.snapshot,command));String prepared_command=command;pixaura_document_handle proposal{};
        check(pixaura_document_apply(1,&s.document,&s.live,byte_pointer(command),command.size(),&proposal,nullptr));
        s.pending_command.swap(prepared_command);s.pending_snapshot=std::move(next);s.proposal=proposal;
        try{s.accept();}catch(...){(void)pixaura_document_release(&s.document,&s.proposal);s.proposal={};s.pending_snapshot.reset();s.pending_command.clear();throw;}
    }
});}
extern "C" int32_t pixaura_editor_state(pixaura_editor_context* c,uint8_t* output,uint64_t capacity,uint64_t* required){return attempt([&]{
    auto& s=owner(c);std::lock_guard<std::mutex> lock(s.gate);require(required&&(!capacity||output));
    const auto& doc=*s.loaded.snapshot;bool undo=false;for(const auto& r:doc.revisions())if(r.id==doc.current())undo=r.parent.has_value();
    uint32_t gesture_state=0;uint64_t sequence=0;if(s.gesture.serial)check(pixaura_manual_state(&s.document,&s.gesture,&gesture_state,&sequence));
    String json=assemble({"{\"active\":",(s.selected?"true":"false"),",\"approval_pending\":",(s.proposal.serial?"true":"false"),
        ",\"can_undo\":",(!s.selected&&undo?"true":"false"),",\"can_redo\":",(!s.selected&&!doc.redo().empty()?"true":"false"),
        ",\"revision_count\":",std::to_string(doc.revisions().size()),",\"revision_id\":\"",doc.current().text(),"\",\"selected\":"});
    if(s.selected)json+=std::to_string(s.selected-manual::registry().data());else json+="-1";
    json+=assemble({",\"gesture_state\":",std::to_string(gesture_state),",\"edit_id\":\"",std::to_string(s.gesture.serial),"\",\"values\":["});if(s.selected)for(std::size_t i=0;i<s.selected->parameter_count;++i){if(i)json+=',';json+=std::to_string(s.values[i]);}json+="]}";
    *required=json.size();if(!output)return;require(capacity>=json.size(),11);std::memcpy(output,json.data(),json.size());
});}
extern "C" int32_t pixaura_editor_request_preview(pixaura_editor_context* c,pixaura_editor_request* out){return attempt([&]{
    auto& s=owner(c);std::lock_guard<std::mutex> lock(s.gate);s.ready();require(out);pixaura_editor_request request{};request.live=s.live;
    if(s.selected&&!s.proposal.serial){request.gesture=s.gesture;check(pixaura_manual_preview_ticket(&s.document,&s.gesture,&s.live,&request.ticket,&request.sequence));require(request.sequence!=0,13);}
    else {check(pixaura_preview_begin(&s.raster,1,&s.approved_ticket));request.ticket=s.approved_ticket;}*out=request;
});}
extern "C" int32_t pixaura_editor_current(pixaura_editor_context* c,const pixaura_editor_request* r){return attempt([&]{
    auto& s=owner(c);std::lock_guard<std::mutex> lock(s.gate);s.ready();require(r);require(std::memcmp(&r->live,&s.live,sizeof(s.live))==0,13);
    if(r->gesture.serial)require(r->gesture.serial==s.gesture.serial,13);
    check(r->gesture.serial?pixaura_manual_preview_current(&s.document,&r->gesture,&s.live,r->sequence,&r->ticket,nullptr):pixaura_preview_current(&s.raster,&r->ticket,nullptr));
});}
extern "C" int32_t pixaura_editor_render(pixaura_editor_context* c,const pixaura_editor_request* r,
    uint32_t width,uint32_t height,uint8_t* output,uint64_t capacity,uint32_t* out_width,uint32_t* out_height){return attempt([&]{
    auto& s=owner(c);require(r&&output&&out_width&&out_height&&width&&height&&width<=1024&&height<=1024);
    require(capacity>=static_cast<uint64_t>(width)*height*4,11);
    check(pixaura_editor_current(c,r));
    String projection;bool empty_stack=false;{std::lock_guard<std::mutex> lock(s.gate);s.ready();require(r->live.serial==s.live.serial,13);
        if(!r->gesture.serial){const auto operations=good(document::replay(*s.loaded.snapshot,s.loaded.snapshot->current()));
            empty_stack=operations.empty();projection=good(document::serialize_evaluation(operations));}}
    pixaura_decode_handle preview{},evaluated{};
    struct Cleanup{Service& s;pixaura_decode_handle& p; pixaura_decode_handle& e;~Cleanup(){if(p.serial)(void)pixaura_preview_release(&s.raster,&p);if(e.serial)(void)pixaura_decode_release(&s.raster,&e);}} cleanup{s,preview,evaluated};
    const pixaura_preview_request fit{1,sizeof(pixaura_preview_request),1,width,height,0};
    if(r->gesture.serial){uint64_t sequence=0;check(pixaura_manual_render(&s.document,&r->gesture,&r->live,&s.raster,&s.original,&s.limits,nullptr,&r->ticket,&fit,&preview,&sequence));require(sequence==r->sequence,13);}
    else if(empty_stack)check(pixaura_preview_render_interactive(&s.raster,&s.original,&r->ticket,&fit,&preview));
    else{check(pixaura_working_evaluate(&s.raster,&s.original,1,byte_pointer(projection),projection.size(),&s.limits,&evaluated));
        check(pixaura_preview_render_interactive(&s.raster,&evaluated,&r->ticket,&fit,&preview));}
    pixaura_preview_metadata metadata{};check(pixaura_preview_query(&s.raster,&preview,&metadata));
    check(pixaura_editor_current(c,r));check(pixaura_preview_copy(&s.raster,&preview,0,output,metadata.image_bytes));
    *out_width=metadata.width;*out_height=metadata.height;
});}
extern "C" int32_t pixaura_editor_stop(pixaura_editor_context* c){return attempt([&]{auto& s=owner(c);std::lock_guard<std::mutex> lock(s.gate);
    if(!s.stopped){s.clear();s.stopped=true;check(pixaura_preview_stop(&s.raster));}});}
extern "C" int32_t pixaura_editor_destroy(pixaura_editor_context* c){return attempt([&]{auto& s=owner(c);require(s.stopped,13);delete &s;
    *c={};c->api_version=1;c->struct_size=sizeof(*c);});}
