#include "context_v2.hpp"
#include "../../storage_files.hpp"
#include "../../sha256.hpp"
#include "sodium.h"
#include <algorithm>
#include <map>
#include <cstring>
namespace pixaura::editor {
namespace {
void need(bool ok,int code=7){if(!ok)throw document::Failure{code};}
template<class T>T good(document::Result<T> r){if(r.code)throw document::Failure{r.code,r.index};return std::move(r.value);}
document::String command(const document::ImageDocument& doc,std::string_view kind){
    document::String out="{\"command_version\":1,\"kind\":\"";out.append(kind);out.append("\",\"expected_revision_id\":\"");out.append(doc.current().text());out.append("\",\"expected_session_id\":\"");out.append(doc.session().id.text());out.append("\",\"expected_generation\":");out.append(std::to_string(doc.session().generation));out+='}';return out;
}
}
EditorContextV2::Work::Work(EditorContextV2& value):owner(value){std::lock_guard<std::mutex> lock(owner.work_gate_);owner.ready();need(owner.active_work_!=UINT64_MAX,8);++owner.active_work_;}
EditorContextV2::Work::~Work(){std::lock_guard<std::mutex> lock(owner.work_gate_);if(!--owner.active_work_)owner.work_idle_.notify_all();}
EditorContextV2::AdvisoryWork::AdvisoryWork(EditorContextV2& value):owner(value){need(!owner.advisory_work_,8);owner.advisory_work_=true;}
EditorContextV2::AdvisoryWork::~AdvisoryWork(){owner.advisory_work_=false;}
uint64_t EditorContextV2::advance_generation()noexcept{
    auto value=generation_.load(std::memory_order_acquire);
    for(;;){if(value==UINT64_MAX){stopped_=true;revoked_=true;return value;}
        if(generation_.compare_exchange_weak(value,value+1,std::memory_order_acq_rel))return value+1;}
}
uint64_t EditorContextV2::advance_expected(uint64_t expected){
    need(expected!=UINT64_MAX,8);const auto next=expected+1;need(generation_.compare_exchange_strong(expected,next,std::memory_order_acq_rel),13);return next;
}
document::String EditorContextV2::fresh(){need(serial_!=UINT64_MAX,8);++serial_;document::String seed=identity_;seed+=':';seed.append(std::to_string(serial_));storage::Sha256 hash;hash.update(reinterpret_cast<const uint8_t*>(seed.data()),seed.size());return hash.finish().substr(0,32);}
void EditorContextV2::ready(){need(!stopped_.load(std::memory_order_acquire),13);}
void EditorContextV2::stale(){need(repository_->epoch_v2()==loaded_.epoch,9);}
void EditorContextV2::reload(){
    auto lease=std::unique_ptr<tiled::Budget::Lease>(new tiled::Budget::Lease(budget_->reserve(repository_->metadata_reservation_v2())));
    auto next=repository_->read();loaded_=std::move(next);metadata_lease_=std::move(lease);
}
void EditorContextV2::install_review(Review&& value,bool replacing){
    std::lock_guard<std::mutex> lock(control_gate_);ready();need(value.view.id.size()==32&&value.cancellation,14);
    if(replacing)need(advisory_control_&&!advisory_control_->requested&&generation_==value.generation,13);
    if(advisory_control_)advisory_control_->requested=true;
    review_=std::move(value);std::memcpy(control_id_.data(),review_->view.id.data(),32);advisory_control_=review_->cancellation;
}
void EditorContextV2::cancel_advisory(const std::string& id){
    need(id.size()==32,1);{std::lock_guard<std::mutex> lock(control_gate_);need(advisory_control_&&std::memcmp(control_id_.data(),id.data(),32)==0,9);advisory_control_->requested=true;}
    std::lock_guard<std::mutex> lock(gate_);if(review_&&review_->view.id==id)clear();
}
void EditorContextV2::clear(){{std::lock_guard<std::mutex> lock(control_gate_);for(auto& task:preview_work_)task.second->cancellation->requested=true;preview_work_.clear();if(advisory_control_)advisory_control_->requested=true;advisory_control_.reset();control_id_.fill(0);}if(review_){try{commercial_->dismiss_delivery(review_->view.delivery);}catch(...){commercial_unavailable_=true;}review_.reset();}provider_.cancel();if(gesture_)gesture_->cancel();gesture_.reset();candidate_.reset();candidate_lease_.reset();selected_=nullptr;gesture_operation_id_.clear();values_.clear();}
void EditorContextV2::admit_source(){
    need(commercial_&&!native_receipts_unverified_,3003);
    const auto admitted=commercial_->reserve(fresh(),source_->digest(),false);
    if(admitted.decision!=commerce::Admission::allowed)throw document::Failure{admitted.decision==commerce::Admission::photo_limit?3001:3003};
    try{need(commercial_->finish(admitted.token,true,{}),12);}catch(...){(void)commercial_->finish(admitted.token,false,{});throw;}
}
EditorContextV2::EditorContextV2(const std::string& root,storage::Reader* input,uint64_t import_bytes,bool migrate,std::shared_ptr<tiled::Budget> budget,commerce::CommercialCoordinatorV2& commercial,storage::ReceiptAuthorityV2& key,std::function<std::shared_ptr<void>()> admission):EditorContextV2(root,input,import_bytes,migrate,std::move(budget),&commercial,key,std::move(admission)){}
EditorContextV2::EditorContextV2(const std::string& root,storage::Reader* input,uint64_t import_bytes,bool migrate,std::shared_ptr<tiled::Budget> budget,commerce::CommercialCoordinatorV2* commercial,storage::ReceiptAuthorityV2& key,std::function<std::shared_ptr<void>()> admission):budget_(std::move(budget)),commercial_(commercial),key_(key),resource_admission_(std::move(admission)){
    auto resource_work=admit_resources();
    need(budget_!=nullptr,1);control_=std::unique_ptr<tiled::Budget::Lease>(new tiled::Budget::Lease(budget_->reserve(8ULL<<20)));
    need(sodium_init()>=0,12);unsigned char random[16];randombytes_buf(random,sizeof random);static constexpr char hex[]="0123456789abcdef";for(auto b:random){identity_+=hex[b>>4];identity_+=hex[b&15];}
    repository_=std::make_unique<storage::Repository>(root,4);storage::Asset asset;
    if(import_bytes){need(repository_->version()==1,4);need(input&&import_bytes<=decode::defaults().encoded_bytes,8);storage::AssetStore assets(root);asset=assets.ingest(*input,import_bytes);}
    else{
        need(!input,1);reload();asset={loaded_.snapshot->source().sha256.text(),loaded_.snapshot->source().byte_length};
        need(repository_->version()==4||(repository_->version()==3&&migrate),4);
    }
    {
        storage::Files files(root);source_=integration::OriginalReaderV2::from_verified(files,asset,*budget_);need(source_->digest()==asset.digest,19);
    }
    if(import_bytes){
        need(repository_->version()==1,4);repository_->migrate(1,2);repository_->migrate(2,3);
        const auto project=fresh(),doc=fresh(),revision=fresh(),session=fresh();const auto& m=source_->metadata().value;
        document::String json="{\"schema_version\":1,\"project_id\":\"";json.append(project);json.append("\",\"document_id\":\"");json.append(doc);json.append("\",\"current_revision_id\":\"");json.append(revision);json.append("\",\"redo\":[],\"operations\":[],\"revisions\":[{\"id\":\"");json.append(revision);json.append("\",\"parent_id\":null,\"actor\":\"import\",\"plan_id\":null,\"stack\":[]}],\"source\":{\"sha256\":\"");json.append(asset.digest);json.append("\",\"byte_length\":");json.append(std::to_string(asset.bytes));json.append(",\"metadata\":{\"width\":");json.append(std::to_string(m.width));json.append(",\"height\":");json.append(std::to_string(m.height));json.append(",\"orientation\":");json.append(std::to_string(m.orientation));json.append(",\"codec\":\"");json.append(m.format==1?"jpeg":"png");json.append("\",\"has_alpha\":");json.append(m.has_alpha?"true":"false");json.append(",\"icc_sha256\":null}}}");
        auto metadata=budget_->reserve(1048576);auto initial=good(document::create_document(json,session));
        repository_->create(*initial);repository_->migrate(3,4);reload();
    }else{
        if(repository_->version()==3)repository_->migrate(3,4);
        auto transient=budget_->reserve(repository_->metadata_reservation_v2());loaded_=repository_->open_session(fresh(),loaded_.epoch);reload();
    }
    integration::RevisionProjectionV2 binding(loaded_.snapshot,*source_,*budget_);(void)binding;
    // Receipt authenticity gates NEW editing, while recovery remains available.
    try{auto validation=budget_->reserve(repository_->metadata_reservation_v2()*2+1048576);(void)repository_->receipts_v2(key_);}catch(...){native_receipts_unverified_=true;}
    if(commercial_&&!native_receipts_unverified_){try{auto validation=budget_->reserve(repository_->metadata_reservation_v2()*2+1048576);(void)commercial_->reconcile(*repository_);}catch(...){commercial_unavailable_=true;}}else commercial_unavailable_=true;

}
EditorContextV2::~EditorContextV2(){stop();join();}
void EditorContextV2::activate_import(){std::lock_guard<std::mutex> lock(gate_);ready();stale();auto resource_work=admit_resources();auto workspace=budget_->reserve(repository_->metadata_reservation_v2()*2+1048576);repository_->fence_v2(loaded_.epoch,[&]{admit_source();});}
void EditorContextV2::select(uint32_t index){
    const auto requested=advance_generation();std::lock_guard<std::mutex> lock(gate_);ready();stale();need(index<manual::registry().size());clear();
    publication_phase_=1;struct ResetSelect{std::atomic<unsigned>& phase;~ResetSelect(){phase=0;}} reset{publication_phase_};
    auto resource_work=admit_resources();const auto& descriptor=manual::registry()[index];auto gesture=good(manual::Gesture::begin(loaded_.snapshot,fresh(),descriptor.tool_id));
    auto operation=fresh();std::vector<int32_t> values;values.reserve(descriptor.parameter_count);
    for(std::size_t i=0;i<descriptor.parameter_count;++i)values.push_back(descriptor.parameters[i].neutral);
    // No fallible gesture allocation follows commercial success. A denied or
    // resource-failed begin never consumes a new distinct-source slot.
    auto workspace=budget_->reserve(repository_->metadata_reservation_v2()*2+1048576);
    repository_->fence_v2(loaded_.epoch,[&]{ready();need(generation_==requested,13);unsigned evaluating=1;need(publication_phase_.compare_exchange_strong(evaluating,4),13);admit_source();gesture_=std::move(gesture);selected_=&descriptor;gesture_operation_id_=std::move(operation);values_=std::move(values);advance_generation();});
}
void EditorContextV2::update(const int32_t* values,uint32_t count){
    advance_generation();std::lock_guard<std::mutex> lock(gate_);ready();stale();need(selected_&&gesture_&&!candidate_,13);need(values&&count==selected_->parameter_count);
    document::String parameters="{";for(unsigned i=0;i<count;++i){if(i)parameters+=',';parameters+='"';parameters.append(selected_->parameters[i].name);parameters.append("\":");parameters.append(std::to_string(values[i]));}parameters+='}';
    document::String op="{\"operations\":[{\"id\":\"";op.append(gesture_operation_id_);op.append("\",\"type\":\"");op.append(selected_->type);op.append("\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":");op.append(parameters);op.append("}]}");
    std::vector<int32_t> next_values(values,values+count);(void)good(gesture_->update(op));values_.swap(next_values);advance_generation();
}
bool EditorContextV2::human_apply(){
    std::lock_guard<std::mutex> lock(gate_);ready();stale();need(gesture_&&selected_,13);auto resource_work=admit_resources();publication_phase_=1;
    struct Reset{std::atomic<unsigned>& phase;~Reset(){phase=0;}} reset{publication_phase_};
    if(!candidate_){candidate_lease_=std::unique_ptr<tiled::Budget::Lease>(new tiled::Budget::Lease(budget_->reserve(repository_->metadata_reservation_v2()+1048576)));candidate_=good(gesture_->commit(loaded_.snapshot,fresh()));}
    if(candidate_==loaded_.snapshot){clear();advance_generation();return false;}
    const auto generation=generation_.load();evaluation::Cancellation cancellation;cancellation.generation=&generation_;cancellation.expected=generation;cancellation.revoked=&revoked_;
    integration::RevisionProjectionV2 projection(candidate_,*source_,*budget_);const auto extent=projection.plan().extent();
    for(uint32_t y=0;y<extent.height;y+=128)for(uint32_t x=0;x<extent.width;x+=128){auto tile=projection.plan().render({x,y,std::min(x+128,extent.width),std::min(y+128,extent.height)},*budget_,&cancellation);(void)tile;}
    auto workspace=budget_->reserve(repository_->metadata_reservation_v2()*3+1048576);
    const auto serialized=good(document::serialize_commit(*loaded_.snapshot,*candidate_));
    ready();evaluation::checkpoint(&cancellation,evaluation::Checkpoint::publication);unsigned evaluating=1;need(publication_phase_.compare_exchange_strong(evaluating,2),13);
    const auto receipt=repository_->publish_v2(serialized,loaded_.epoch,*commercial_,key_);need(receipt.revision()==candidate_->current().text(),19);
    // Commit already won. A late stop/commerce failure cannot erase publication
    // or turn it into a fabricated zero-revision failure.
    loaded_={candidate_,receipt.epoch()};metadata_lease_=std::move(candidate_lease_);clear();advance_generation();
    try{(void)commercial_->reconcile(*repository_);commercial_unavailable_=false;}catch(...){commercial_unavailable_=true;}
    return true;
}
void EditorContextV2::cancel(){{std::lock_guard<std::mutex> lock(control_gate_);if(advisory_control_)advisory_control_->requested=true;unsigned phase=1;(void)publication_phase_.compare_exchange_strong(phase,3);if(publication_phase_!=2&&publication_phase_!=4)advance_generation();}std::lock_guard<std::mutex> lock(gate_);clear();advance_generation();}
void EditorContextV2::history(bool redo){
    advance_generation();std::lock_guard<std::mutex> lock(gate_);ready();stale();need(!selected_,13);auto resource_work=admit_resources();
    auto next_lease=std::unique_ptr<tiled::Budget::Lease>(new tiled::Budget::Lease(budget_->reserve(repository_->metadata_reservation_v2())));
    auto workspace=budget_->reserve(repository_->metadata_reservation_v2()*3);
    const auto navigation=command(*loaded_.snapshot,redo?"redo":"undo");auto next=good(document::transition(*loaded_.snapshot,navigation));
    // Invalid navigation/resource preparation cannot consume a source slot.
    // Source eligibility is separate from ordinary native history authority.
    admit_source();const auto saved=repository_->apply(navigation,loaded_.epoch);
    loaded_={std::move(next),saved.epoch};metadata_lease_=std::move(next_lease);advance_generation();
}
StateV2 EditorContextV2::state(){std::lock_guard<std::mutex> lock(gate_);ready();stale();StateV2 state;const auto& doc=*loaded_.snapshot;state.revision=doc.current().text();state.revisions=static_cast<uint32_t>(doc.revisions().size());state.epoch=loaded_.epoch;state.generation=generation_;state.pending=bool(candidate_);state.commerce_unavailable=commercial_unavailable_;state.recovery_only=!commercial_||native_receipts_unverified_;state.values=values_;if(selected_)state.selected=static_cast<uint32_t>(selected_-manual::registry().data());
    if(!selected_){for(const auto& revision:doc.revisions())if(revision.id==doc.current())state.can_undo=revision.parent.has_value();state.can_redo=!doc.redo().empty();}return state;
}
document::String EditorContextV2::gesture_identity(){std::lock_guard<std::mutex> lock(gate_);ready();stale();return gesture_operation_id_;}
FrameV2 EditorContextV2::preview(uint32_t width,uint32_t height,bool original){
    std::lock_guard<std::mutex> lock(gate_);ready();stale();need(width&&height&&width<=1024&&height<=1024,7);const auto generation=generation_.load();evaluation::Cancellation cancellation;cancellation.generation=&generation_;cancellation.expected=generation;cancellation.revoked=&revoked_;
    auto resource_work=admit_resources();evaluation::Stack operations;
    if(!original){if(candidate_)operations=good(document::replay(*candidate_,candidate_->current()));else if(gesture_)operations=evaluation::parse(good(gesture_->preview(loaded_.snapshot)));else operations=good(document::replay(*loaded_.snapshot,loaded_.snapshot->current()));}
    integration::StagePlanV2 plan(*source_,operations,*budget_);auto pixels=integration::fit_preview_v2(plan,width,height,*budget_,&cancellation);return {budget_,std::move(pixels),identity_,generation,loaded_.epoch};
}
document::String EditorContextV2::history_json(){
    std::lock_guard<std::mutex> lock(gate_);ready();stale();auto resource_work=admit_resources();auto transport=budget_->reserve(2097152);const auto& doc=*loaded_.snapshot;
    document::String json="{\"current\":\"";json.append(doc.current().text());json.append("\",\"document_id\":\"");json.append(doc.identity().document.text());json.append("\",\"epoch\":");json.append(std::to_string(loaded_.epoch));json.append(",\"redo\":[");
    bool first=true;for(const auto& id:doc.redo()){if(!first)json+=',';first=false;json+='"';json.append(id.text());json+='"';}
    json.append("],\"revisions\":[");first=true;
    for(const auto& revision:doc.revisions()){
        if(!first)json+=',';
        first=false;json.append("{\"actor\":\"");json.append(revision.actor);json.append("\",\"id\":\"");json.append(revision.id.text());json.append("\",\"parent_id\":");
        if(revision.parent){json+='"';json.append(revision.parent->text());json+='"';}else json.append("null");
        json.append(",\"plan_id\":");if(revision.plan){json+='"';json.append(revision.plan->text());json+='"';}else json.append("null");
        // Read-only geometry identity follows the accepted ordinary-history
        // comparison: ordered geometry types/parameters, never operation IDs.
        auto workspace=budget_->reserve(repository_->metadata_reservation_v2());
        const auto stack=good(document::replay(doc,revision.id));storage::Sha256 geometry;
        for(const auto& op:stack){
            document::String value;
            if(const auto* crop=std::get_if<document::Crop>(&op.parameters)){
                value=op.type;value+=' ';value.append(std::to_string(crop->x_ppm));value+=' ';value.append(std::to_string(crop->y_ppm));value+=' ';value.append(std::to_string(crop->width_ppm));value+=' ';value.append(std::to_string(crop->height_ppm));value+='\n';
            }else if(const auto* rotate=std::get_if<document::Rotate>(&op.parameters)){
                value=op.type;value+=' ';value.append(std::to_string(rotate->quarter_turns));value+='\n';
            }
            geometry.update(reinterpret_cast<const uint8_t*>(value.data()),value.size());
        }
        json.append(",\"geometry_sha256\":\"");json.append(geometry.finish());json.append("\"}");need(json.size()<=1048320,8);
    }
    json.append("],\"source_sha256\":\"");json.append(doc.source().sha256.text());json.append("\",\"version\":2}\n");need(json.size()<=1048576,8);return json;
}
FrameV2 EditorContextV2::preview_revision(const std::string& revision,uint32_t width,uint32_t height){
    std::lock_guard<std::mutex> lock(gate_);ready();stale();need(width&&height&&width<=1024&&height<=1024,7);
    auto resource_work=admit_resources();const auto id=document::Id::parse(revision);bool approved=false;for(const auto& r:loaded_.snapshot->revisions())if(r.id==id)approved=true;need(approved,9);
    auto operations=good(document::replay(*loaded_.snapshot,id));const auto generation=generation_.load();evaluation::Cancellation cancellation;cancellation.generation=&generation_;cancellation.expected=generation;cancellation.revoked=&revoked_;
    integration::StagePlanV2 plan(*source_,operations,*budget_);auto pixels=integration::fit_preview_v2(plan,width,height,*budget_,&cancellation);return {budget_,std::move(pixels),identity_,generation,loaded_.epoch};
}
bool EditorContextV2::current(const FrameV2& frame,bool wait){return current_status(frame,wait)==0;}
int32_t EditorContextV2::current_status(const FrameV2& frame,bool wait){
    std::unique_lock<std::mutex> lock(gate_,std::defer_lock);if(wait)lock.lock();else if(!lock.try_lock())return 8;
    return !stopped_&&(!frame.control||!frame.control->requested)&&(!frame.review_control||!frame.review_control->requested)&&frame.context_id==identity_&&frame.budget_owner==budget_&&frame.generation==generation_&&frame.epoch==loaded_.epoch&&repository_->epoch_v2()==loaded_.epoch?0:9;
}
uint64_t EditorContextV2::export_saved(uint32_t format,int quality,integration::StagedSinkV2& sink){return export_observed(format,quality,sink).bytes;}
ExportObservationV2 EditorContextV2::export_observed(uint32_t format,int quality,integration::StagedSinkV2& sink){
    uint64_t ticket=0;
    try{ticket=begin_export(format,quality);auto result=run_export(ticket,sink);release_export(ticket);return result;}
    catch(...){if(ticket)release_export(ticket);sink.abort();throw;}
}
uint64_t EditorContextV2::begin_export(uint32_t format,int quality){
    std::lock_guard<std::mutex> lock(gate_);ready();stale();need((format==1||format==2)&&quality>=(format==2?0:1)&&quality<=100,7);
    auto resource_work=admit_resources();auto task=std::make_shared<ExportWork>();task->metadata.reset(new tiled::Budget::Lease(budget_->reserve(repository_->metadata_reservation_v2())));
    task->snapshot=loaded_.snapshot;task->epoch=loaded_.epoch;task->format=format;task->quality=quality;
    // An already authorized saved-revision export is independent of later
    // editing/quota changes. Only its own cancellation or owner stop retires it.
    task->cancellation.generation=&task->generation;task->cancellation.expected=1;task->cancellation.revoked=&stopped_;
    std::lock_guard<std::mutex> control(control_gate_);need(export_work_.size()<2&&serial_!=UINT64_MAX,8);const auto ticket=++serial_;export_work_.emplace(ticket,std::move(task));return ticket;
}
void EditorContextV2::cancel_export(uint64_t ticket){
    need(ticket,1);std::lock_guard<std::mutex> lock(control_gate_);const auto found=export_work_.find(ticket);need(found!=export_work_.end(),9);found->second->cancellation.requested=true;
}
void EditorContextV2::release_export(uint64_t ticket){
    need(ticket,1);std::lock_guard<std::mutex> lock(control_gate_);const auto found=export_work_.find(ticket);need(found!=export_work_.end(),9);found->second->cancellation.requested=true;export_work_.erase(found);
}
ExportObservationV2 EditorContextV2::run_export(uint64_t ticket,integration::StagedSinkV2& sink){
    try{
    need(ticket,1);std::lock_guard<std::mutex> lock(gate_);ready();std::shared_ptr<ExportWork> task;
    {std::lock_guard<std::mutex> control(control_gate_);const auto found=export_work_.find(ticket);need(found!=export_work_.end(),9);task=found->second;need(!task->started,15);task->started=true;}
    evaluation::checkpoint(&task->cancellation,evaluation::Checkpoint::admission);
    auto resource_work=admit_resources();integration::RevisionProjectionV2 projection(task->snapshot,*source_,*budget_);
    ExportObservationV2 result;result.epoch=task->epoch;result.width=projection.plan().extent().width;result.height=projection.plan().extent().height;result.format=task->format;
    result.source=task->snapshot->source().sha256.text();result.document=task->snapshot->identity().document.text();result.revision=task->snapshot->current().text();
    result.bytes=integration::stream_export_v2(projection.plan(),task->format,task->quality,*budget_,sink,&task->cancellation);return result;
    }catch(...){sink.abort();throw;}
}

namespace {
template<class T>T provider_value(assistance::Outcome<T> r){if(r.code)throw document::Failure{r.code};need(r.value.has_value(),14);return std::move(*r.value);}
std::string advisory_capsule(const document::ImageDocument& doc,uint32_t choice){return "PXA_ADVISORY_V2\n"+doc.source().sha256.text()+"\n"+doc.identity().project.text()+"\n"+doc.identity().document.text()+"\n"+doc.current().text()+"\n"+std::to_string(choice);}
}
#ifdef PIXAURA_CERTIFICATION_TESTING
int32_t EditorContextV2::certification_control(uint32_t family,uint32_t command,uint32_t value){
    need(family>=1&&family<=4,1);
    auto& test=certification_[family-1];
    const auto result=test.control(command,value,family==1?1:family==3?3:2,family==1?5:family==2?12:family==4?3:0);
    if(!result&&(command==0||command==3)&&family<=2)advisory_test_family_=family;
    return result;
}
document::String EditorContextV2::test_provider(assistance::Request input){
    const auto family=advisory_test_family_.load();auto& test=certification_[family-1];
    test.checkpoint(0);const auto fault=test.fault.load();
    if(fault==1)provider_.stop();
    if(fault==4)throw document::Failure{8};
    if(fault==5)throw document::Failure{14};
    if(fault==3)input.payload="{}\n";
    if(fault==6)input.registry_version=2;
    if(fault==7)input.payload="{}\n";
    if(fault==8)input.payload.assign(8193,'x');
    if(fault==11)input.version=2;
    if(fault==12){constexpr std::string_view marker="\"structured_intent_version\":1";const auto at=input.payload.find(marker);if(at!=document::String::npos)input.payload[at+marker.size()-1]='2';}
    auto envelope=provider_value(provider_.request(input));
    if(fault==2)envelope+='x';
    if(fault==9)envelope.assign(12289,'x');
    if(fault==10){const auto at=envelope.find(assistance::reference_id);need(at!=document::String::npos,14);envelope.replace(at,assistance::reference_id.size(),"unknown.provider.v1");}
    test.checkpoint(1);return envelope;
}
#endif
document::String EditorContextV2::provider_envelope(assistance::Request input,std::unique_lock<std::mutex>& lock){
    const auto generation=generation_.load();
    // Provider owns only immutable untrusted request data. Manual operations,
    // cancellation and source changes must remain available during this call.
    lock.unlock();document::String envelope;
    try{
#ifdef PIXAURA_CERTIFICATION_TESTING
        envelope=test_provider(std::move(input));
#else
        envelope=provider_value(provider_.request(input));
#endif
    }catch(...){lock.lock();throw;}
    lock.lock();ready();stale();
    {std::lock_guard<std::mutex> control(control_gate_);if(generation_!=generation){need(request_ticket_!=0,13);throw document::Failure{9};}}
    return envelope;
}
EditorContextV2::Review EditorContextV2::plan_advisory(uint32_t choice,const std::string& delivery,std::unique_lock<std::mutex>& lock){
    need(choice<structured::goals.size(),7);const auto selected=assist_experience::choice(choice);const auto& descriptor=manual::registry()[selected.tool_index];
    need(serial_!=UINT64_MAX,8);
    auto input=provider_value(assist_experience::request(*loaded_.snapshot,identity_,choice,descriptor,selected,++serial_));
    // The goal helper emits a structured intent, not an already-authored Plan.
    // The admitted deterministic planner validates it and derives the Plan.
    input.kind=assistance::Kind::StructuredIntent;
    const auto envelope=provider_envelope(std::move(input),lock);
    auto decoded=provider_value(assistance::decode(envelope,*loaded_.snapshot,identity_,assistance::reference_id));
    std::vector<int32_t> values;for(uint32_t v=0;v<selected.count;++v)values.push_back(selected.values[v]);
    return decoded_review(std::move(decoded),delivery,choice,selected.tool_index,values);
}
EditorContextV2::Review EditorContextV2::decoded_review(assistance::Decoded&& decoded,const std::string& delivery,uint32_t choice,uint32_t tool,const std::vector<int32_t>& values){
    need(!decoded.operations.empty()&&decoded.operations.size()<=8,7);
    auto stack=good(document::replay(*loaded_.snapshot,loaded_.snapshot->current()));for(auto& op:decoded.operations){const auto* registered=manual::find(op.type,op.operation_version,op.parameter_version);need(registered&&manual::agent_exposable(*registered),5);
        // Planner identities are temporary validator positions. Native ownership
        // supplies fresh operation identities before review or publication.
        op.id=document::Id::parse(fresh());stack.push_back(op);}
    integration::StagePlanV2 admitted(*source_,stack,*budget_);const auto extent=admitted.extent();const auto generation=generation_.load();evaluation::Cancellation cancellation;cancellation.generation=&generation_;cancellation.expected=generation;cancellation.revoked=&revoked_;
    for(uint32_t y=0;y<extent.height;y+=128)for(uint32_t x=0;x<extent.width;x+=128){auto tile=admitted.render({x,y,std::min(x+128,extent.width),std::min(y+128,extent.height)},*budget_,&cancellation);(void)tile;}
    Review result;result.view={fresh(),delivery,decoded.value.plan,good(document::serialize_evaluation(decoded.operations)),loaded_.epoch,choice,tool,values};result.operations=std::move(decoded.operations);result.original=result.operations;result.cancellation=std::make_shared<evaluation::Cancellation>();result.generation=generation_;return result;
}
document::String EditorContextV2::agent_bytes(uint32_t mode,std::string_view input){
    std::lock_guard<std::mutex> lock(gate_);ready();stale();
    if(mode==1){need(input.empty(),1);return good(document::agent_schema_binding(*loaded_.snapshot,identity_));}
    need(mode==2||mode==3,1);
    if(mode==3)return good(document::plan_structured_intent(input,*loaded_.snapshot,identity_)).plan;
    return good(document::canonical_agent_schema(input,2,*loaded_.snapshot,identity_));
}
EditorContextV2::Review EditorContextV2::plan_candidate(std::string_view plan,const std::string& delivery,std::unique_lock<std::mutex>& lock){
    const auto& context=identity_;need(serial_!=UINT64_MAX,8);
    assistance::Request input;input.kind=assistance::Kind::PlanCandidate;input.proposal_id="candidate.";input.proposal_id+=std::to_string(++serial_);
    input.binding=good(document::agent_schema_binding(*loaded_.snapshot,context));
    input.payload=good(document::canonical_agent_schema(plan,2,*loaded_.snapshot,context));
    const auto envelope=provider_envelope(std::move(input),lock);
    auto decoded=provider_value(assistance::decode(envelope,*loaded_.snapshot,context,assistance::reference_id));
    need(!decoded.operations.empty(),7);const auto& first=decoded.operations.front();const auto* tool=manual::find(first.type,first.operation_version,first.parameter_version);need(tool&&manual::agent_exposable(*tool),5);
    auto result=decoded_review(std::move(decoded),delivery,UINT32_MAX,static_cast<uint32_t>(tool-manual::registry().data()),{});
    result.view.plan=canonical_review_plan(result.operations);return result;
}
uint64_t EditorContextV2::begin_plan_candidate(std::string_view plan){
    need(!plan.empty()&&plan.size()<=8192,8);
    std::lock_guard<std::mutex> lock(gate_);ready();stale();need(!advisory_work_&&!gesture_&&!candidate_,8);
    {std::lock_guard<std::mutex> control(control_gate_);need(request_ticket_==0,8);}
    const auto requested=advance_generation();clear();
    auto canonical=good(document::canonical_agent_schema(plan,2,*loaded_.snapshot,identity_));
    std::lock_guard<std::mutex> control(control_gate_);need(generation_==requested,13);request_plan_=std::move(canonical);request_ticket_=requested;request_choice_=UINT32_MAX;return requested;
}
uint64_t EditorContextV2::begin_advisory(uint32_t choice){
    need(choice<structured::goals.size(),7);
    std::lock_guard<std::mutex> lock(gate_);ready();stale();need(!advisory_work_&&!gesture_&&!candidate_,8);
    {std::lock_guard<std::mutex> control(control_gate_);need(request_ticket_==0,8);}
    const auto requested=advance_generation();clear();
    std::lock_guard<std::mutex> control(control_gate_);
    need(generation_==requested,13);request_plan_.clear();request_ticket_=requested;request_choice_=choice;return requested;
}
void EditorContextV2::cancel_advisory_request(uint64_t ticket){
    need(ticket!=0,1);std::lock_guard<std::mutex> control(control_gate_);
    need(request_ticket_==ticket,9);
    // Once native delivery wins its fence, a recoverable usable proposal exists.
    // The ticket cannot undo that delivery or cancel later unrelated editing.
    if(publication_phase_==4)return;
    uint64_t expected=ticket;
    if(ticket!=UINT64_MAX)(void)generation_.compare_exchange_strong(expected,ticket+1);
    request_ticket_=0;
}
AdvisoryViewV2 EditorContextV2::request_advisory(uint32_t choice){return run_advisory(begin_advisory(choice));}
AdvisoryViewV2 EditorContextV2::run_advisory(uint64_t ticket){
    Work active(*this);need(ticket!=0,1);std::unique_lock<std::mutex> lock(gate_);ready();stale();need(!gesture_&&!candidate_,8);
    uint32_t choice=0;{std::lock_guard<std::mutex> control(control_gate_);need(request_ticket_==ticket&&generation_==ticket,13);choice=request_choice_;need(publication_phase_==0,8);}
    AdvisoryWork provider_lane(*this);
    // A consumed worker ticket retires on EVERY exit, including quota denial or
    // commerce/cache failure before a reservation exists. It must not wedge the
    // native provider lane after manual editing legitimately starts the trial.
    struct RetireRequest {std::mutex& gate;uint64_t& pending;uint64_t ticket;~RetireRequest(){std::lock_guard<std::mutex> control(gate);if(pending==ticket)pending=0;}} retired{control_gate_,request_ticket_,ticket};
    const auto generation=ticket;
    need(commercial_&&!native_receipts_unverified_,3003);const auto admitted=commercial_->reserve(fresh(),source_->digest(),true);if(admitted.decision!=commerce::Admission::allowed)throw document::Failure{admitted.decision==commerce::Admission::ai_limit?3002:admitted.decision==commerce::Admission::photo_limit?3001:3003};
    publication_phase_=1;struct ResetDelivery{std::atomic<unsigned>& phase;~ResetDelivery(){phase=0;}} reset{publication_phase_};
    try{
#ifdef PIXAURA_CERTIFICATION_TESTING
        CertificationV2::Run test_run(certification_[advisory_test_family_.load()-1]);
#endif
        auto resource_work=admit_resources();auto result=choice==UINT32_MAX?plan_candidate(request_plan_,admitted.token,lock):plan_advisory(choice,admitted.token,lock);ready();stale();need(generation_==generation,13);
#ifdef PIXAURA_CERTIFICATION_TESTING
        if(advisory_test_family_==2){
            lock.unlock();try{certification_[1].checkpoint(2);}catch(...){lock.lock();throw;}
            lock.lock();ready();stale();
            if(generation_!=generation){std::lock_guard<std::mutex> control(control_gate_);need(request_ticket_!=0,13);throw document::Failure{9};}
        }
        ready();need(generation_==generation,13);
#endif
        const auto capsule=choice==UINT32_MAX?std::string("PXA_ADVISORY_V3\n")+loaded_.snapshot->source().sha256.text()+"\n"+loaded_.snapshot->identity().project.text()+"\n"+loaded_.snapshot->identity().document.text()+"\n"+loaded_.snapshot->current().text()+"\n"+std::string(result.view.operations):advisory_capsule(*loaded_.snapshot,choice);need(capsule.size()<=3300,8);
        auto workspace=budget_->reserve(repository_->metadata_reservation_v2()*2+1048576);
        unsigned evaluating=1;need(publication_phase_.compare_exchange_strong(evaluating,4),13);
        repository_->fence_v2(loaded_.epoch,[&]{
            storage::NativeAdvisoryDeliveryV2 receipt;receipt.event_=fresh();receipt.source_=source_->digest();
            receipt.project_=loaded_.snapshot->identity().project.text();receipt.document_=loaded_.snapshot->identity().document.text();
            receipt.revision_=loaded_.snapshot->current().text();receipt.session_=loaded_.snapshot->session().id.text();receipt.context_=identity_;
            receipt.epoch_=loaded_.epoch;receipt.generation_=generation;receipt.token_=admitted.token;receipt.capsule_=capsule;
            storage::Sha256 digest;digest.update(reinterpret_cast<const uint8_t*>(result.view.operations.data()),result.view.operations.size());receipt.operations_digest_=digest.finish();
            receipt.validated_at_fence_=true;receipt.seal_=key_.authenticate(receipt.canonical());
            need(commercial_->deliver(receipt),12);install_review(std::move(result));
        });
        {std::lock_guard<std::mutex> control(control_gate_);if(request_ticket_==ticket)request_ticket_=0;}
        return review_->view;
    }catch(...){
        {std::lock_guard<std::mutex> control(control_gate_);if(request_ticket_==ticket)request_ticket_=0;}
        (void)commercial_->finish(admitted.token,false,{});throw;
    }
}
std::vector<std::string> EditorContextV2::pending_advisories(){
    std::lock_guard<std::mutex> lock(gate_);ready();stale();need(commercial_&&!native_receipts_unverified_,3003);
    std::vector<std::string> result;result.reserve(64);
    const auto prefix=std::string("PXA_ADVISORY_V2\n")+loaded_.snapshot->source().sha256.text()+"\n"+loaded_.snapshot->identity().project.text()+"\n"+loaded_.snapshot->identity().document.text()+"\n";
    for(const auto& token:commercial_->deliveries()){
        const auto capsule=commercial_->recover_delivery(token);
        const auto generic_prefix=std::string("PXA_ADVISORY_V3\n")+loaded_.snapshot->source().sha256.text()+"\n"+loaded_.snapshot->identity().project.text()+"\n"+loaded_.snapshot->identity().document.text()+"\n";
        if(capsule.compare(0,generic_prefix.size(),generic_prefix)==0){
            bool valid=false;
            try{(void)restored_candidate_plan(capsule);valid=true;}catch(const document::Failure& e){if(e.code==8)throw;}
            if(valid)result.push_back(token);else commercial_->dismiss_delivery(token);continue;
        }
        if(capsule.compare(0,prefix.size(),prefix)!=0)continue;
        bool current=false;for(uint32_t c=0;c<structured::goals.size();++c)if(capsule==advisory_capsule(*loaded_.snapshot,c))current=true;
        if(current)result.push_back(token);else commercial_->dismiss_delivery(token);
    }return result;
}
document::String EditorContextV2::restored_candidate_plan(std::string_view capsule){
    const auto prefix=std::string("PXA_ADVISORY_V3\n")+loaded_.snapshot->source().sha256.text()+"\n"+loaded_.snapshot->identity().project.text()+"\n"+loaded_.snapshot->identity().document.text()+"\n"+loaded_.snapshot->current().text()+"\n";
    need(capsule.size()<=3300&&capsule.size()>prefix.size()&&capsule.substr(0,prefix.size())==prefix,9);
    // Only untrusted operation data is recovered. Current native session/context
    // and fresh IDs are supplied anew; no old binding or approval is revived.
    const auto operations=evaluation::parse(capsule.substr(prefix.size()));need(!operations.empty()&&operations.size()<=8,7);
    return canonical_review_plan(operations);
}
AdvisoryViewV2 EditorContextV2::restore_advisory(const std::string& delivery){
    Work active(*this);
    advance_generation();std::unique_lock<std::mutex> lock(gate_);AdvisoryWork provider_lane(*this);
    {std::lock_guard<std::mutex> control(control_gate_);need(request_ticket_==0,8);}ready();stale();need(!gesture_&&!candidate_,8);
    need(commercial_&&!native_receipts_unverified_,3003);const auto capsule=commercial_->recover_delivery(delivery);need(capsule.size()<=4096,8);uint32_t choice=UINT32_MAX;
    for(uint32_t c=0;c<structured::goals.size();++c)if(capsule==advisory_capsule(*loaded_.snapshot,c))choice=c;
    const auto generic_prefix=std::string("PXA_ADVISORY_V3\n")+loaded_.snapshot->source().sha256.text()+"\n"+loaded_.snapshot->identity().project.text()+"\n"+loaded_.snapshot->identity().document.text()+"\n";
    const bool generic=capsule.compare(0,generic_prefix.size(),generic_prefix)==0;
    need(choice!=UINT32_MAX||generic,9);
    document::String plan;
    if(generic)plan=restored_candidate_plan(capsule);
    review_.reset();
    const auto admitted=commercial_->reserve(fresh(),source_->digest(),false);
    if(admitted.decision!=commerce::Admission::allowed)throw document::Failure{admitted.decision==commerce::Admission::photo_limit?3001:3003};
    publication_phase_=1;struct ResetRestore{std::atomic<unsigned>& phase;~ResetRestore(){phase=0;}} reset{publication_phase_};
    try{
        auto resource_work=admit_resources();const auto generation=generation_.load();auto result=generic?plan_candidate(plan,delivery,lock):plan_advisory(choice,delivery,lock);ready();stale();need(generation_==generation,13);
        auto workspace=budget_->reserve(repository_->metadata_reservation_v2()*2+1048576);unsigned evaluating=1;need(publication_phase_.compare_exchange_strong(evaluating,4),13);
        repository_->fence_v2(loaded_.epoch,[&]{need(commercial_->finish(admitted.token,true,{}),12);install_review(std::move(result));});
        return review_->view;
    }catch(...){(void)commercial_->finish(admitted.token,false,{});throw;}
}
void EditorContextV2::review_current(std::string_view id,bool allow_failed){ready();stale();need(review_&&review_->view.id==id&&review_->view.epoch==loaded_.epoch&&review_->generation==generation_,9);need(review_->cancellation&&!review_->cancellation->requested&&(allow_failed||!review_->edit_failed),13);}
document::String EditorContextV2::canonical_review_plan(const document::Vector<document::EditOperation>& operations){
    if(operations.empty())return {};
    document::String plan="{\"actions\":[";
    for(size_t i=0;i<operations.size();++i){const auto& op=operations[i];const auto* tool=manual::find(op.type,op.operation_version,op.parameter_version);need(tool&&manual::agent_exposable(*tool),5);
        std::map<std::string,int64_t> values;
        if(const auto* crop=std::get_if<document::Crop>(&op.parameters))values={{"x_ppm",crop->x_ppm},{"y_ppm",crop->y_ppm},{"width_ppm",crop->width_ppm},{"height_ppm",crop->height_ppm}};
        else{need(tool->parameter_count==1,7);int64_t value=0;
            if(const auto* exposure=std::get_if<document::Exposure>(&op.parameters))value=exposure->milli_ev;
            else if(const auto* rotate=std::get_if<document::Rotate>(&op.parameters))value=rotate->quarter_turns;
            else if(const auto* tone=std::get_if<document::Tone>(&op.parameters))value=tone->value;
            else if(const auto* detail=std::get_if<document::Detail>(&op.parameters))value=detail->value;
            else throw document::Failure{7};
            values.emplace(std::string(tool->parameters[0].name),value);
        }
        if(i)plan+=',';
        plan.append("{\"action_index\":");plan.append(std::to_string(i));plan.append(",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{");
        bool first=true;for(const auto& entry:values){if(!first)plan+=',';first=false;plan+='"';plan.append(entry.first);plan.append("\":");plan.append(std::to_string(entry.second));}
        plan.append("},\"tool_id\":\"");plan.append(tool->tool_id);plan.append("\"}");
    }
    auto binding=good(document::agent_schema_binding(*loaded_.snapshot,identity_));need(!binding.empty()&&binding.back()=='\n',6);binding.pop_back();
    plan.append("],\"binding\":");plan.append(binding);plan.append(",\"plan_version\":1}\n");
    const auto canonical=good(document::canonical_agent_schema(plan,2,*loaded_.snapshot,identity_));need(canonical==plan,6);return canonical;
}
document::String EditorContextV2::review_digest()const{need(review_.has_value(),9);const auto bytes=good(document::serialize_evaluation(review_->operations));storage::Sha256 hash;hash.update(reinterpret_cast<const uint8_t*>(bytes.data()),bytes.size());return hash.finish();}
pixaura_v2_review_inspection EditorContextV2::inspect_review(const std::string& id){
    std::lock_guard<std::mutex> lock(gate_);review_current(id);need(review_->operations.size()<=8,8);
    pixaura_v2_review_inspection out{};out.version=2;out.struct_size=sizeof(out);out.epoch=loaded_.epoch;out.action_count=static_cast<uint32_t>(review_->operations.size());out.warnings=out.action_count>1?1:0;
    const auto text=[](char* to,size_t capacity,std::string_view value){need(value.size()<capacity,14);std::memcpy(to,value.data(),value.size());};
    text(out.review_id,sizeof(out.review_id),id);text(out.provider_id,sizeof(out.provider_id),"reference.assistance.v1");
    for(uint32_t index=0;index<out.action_count;++index){
        const auto& op=review_->operations[index];const auto* tool=manual::find(op.type,op.operation_version,op.parameter_version);need(tool&&manual::agent_exposable(*tool),5);
        auto& action=out.actions[index];action.index=index;action.tool_index=static_cast<uint32_t>(tool-manual::registry().data());action.operation_version=op.operation_version;action.parameter_version=op.parameter_version;action.parameter_count=static_cast<uint32_t>(tool->parameter_count);need(action.parameter_count<=4,14);
        text(action.tool_id,sizeof(action.tool_id),tool->tool_id);text(action.label,sizeof(action.label),tool->name);std::array<int32_t,4> values{};
        if(const auto* crop=std::get_if<document::Crop>(&op.parameters)){values={static_cast<int32_t>(crop->height_ppm),static_cast<int32_t>(crop->width_ppm),static_cast<int32_t>(crop->x_ppm),static_cast<int32_t>(crop->y_ppm)};action.geometry=1;}
        else if(const auto* rotate=std::get_if<document::Rotate>(&op.parameters)){values[0]=static_cast<int32_t>(rotate->quarter_turns);action.geometry=1;}
        else if(const auto* exposure=std::get_if<document::Exposure>(&op.parameters))values[0]=exposure->milli_ev;
        else if(const auto* tone=std::get_if<document::Tone>(&op.parameters))values[0]=tone->value;
        else if(const auto* detail=std::get_if<document::Detail>(&op.parameters))values[0]=detail->value;
        else throw document::Failure{7};
        if(action.geometry)out.warnings|=2;
        for(uint32_t p=0;p<action.parameter_count;++p){const auto& descriptor=tool->parameters[p];auto& value=action.parameters[p];text(value.name,sizeof(value.name),descriptor.name);text(value.unit,sizeof(value.unit),descriptor.unit);value.raw=values[p];value.minimum=descriptor.minimum;value.maximum=descriptor.maximum;value.neutral=descriptor.neutral;}
    }
    return out;
}
AdvisoryViewV2 EditorContextV2::edit_review_command(const std::string& id,uint32_t operation,uint32_t action,uint32_t parameter,int32_t value,uint32_t destination){
    document::String bytes;
    {std::lock_guard<std::mutex> lock(gate_);review_current(id,true);review_->edit_failed=true;review_->approval.clear();review_->approved_digest.clear();review_->generation=advance_expected(review_->generation);
        auto operations=review_->operations;need(operation>=1&&operation<=4,7);
        if(operation==4)operations=review_->original;
        else{
            need(action<operations.size(),7);
            if(operation==2)operations.erase(operations.begin()+action);
            else if(operation==3){need(destination<operations.size(),7);auto moved=operations[action];operations.erase(operations.begin()+action);operations.insert(operations.begin()+destination,std::move(moved));}
            else{
                auto& op=operations[action];const auto* tool=manual::find(op.type,op.operation_version,op.parameter_version);need(tool&&parameter<tool->parameter_count,7);
                const auto& bound=tool->parameters[parameter];need(value>=bound.minimum&&value<=bound.maximum,7);
                if(auto* crop=std::get_if<document::Crop>(&op.parameters)){const auto raw=static_cast<uint32_t>(value);switch(parameter){case 0:crop->x_ppm=raw;break;case 1:crop->y_ppm=raw;break;case 2:crop->width_ppm=raw;break;case 3:crop->height_ppm=raw;break;default:throw document::Failure{7};}}
                else if(auto* rotate=std::get_if<document::Rotate>(&op.parameters))rotate->quarter_turns=static_cast<uint32_t>(value);
                else if(auto* exposure=std::get_if<document::Exposure>(&op.parameters))exposure->milli_ev=value;
                else if(auto* tone=std::get_if<document::Tone>(&op.parameters))tone->value=value;
                else if(auto* detail=std::get_if<document::Detail>(&op.parameters))detail->value=value;
                else throw document::Failure{7};
            }
        }
        bytes=good(document::serialize_evaluation(operations));
    }
    // Revalidate the exact source/review after releasing the short command gate.
    // Native code alone constructs data; this still creates no Apply authority.
    return edit_advisory(id,bytes);
}
AdvisoryViewV2 EditorContextV2::edit_advisory(const std::string& id,std::string_view json){
    std::lock_guard<std::mutex> lock(gate_);review_current(id,true);
    review_->edit_failed=true;review_->approval.clear();review_->approved_digest.clear();
    const auto expected_generation=advance_expected(review_->generation);review_->generation=expected_generation; // Even a failed edit revokes prior intent.
    auto resource_work=admit_resources();need(json.size()<=8192,8);auto operations=evaluation::parse(json);need(operations.size()<=8,8);
    for(auto& op:operations){const auto* tool=manual::find(op.type,op.operation_version,op.parameter_version);need(tool&&manual::agent_exposable(*tool),5);op.id=document::Id::parse(fresh());}
    auto stack=good(document::replay(*loaded_.snapshot,loaded_.snapshot->current()));for(const auto& op:operations)stack.push_back(op);integration::StagePlanV2 check(*source_,stack,*budget_);(void)check;
    auto next=*review_;next.operations=std::move(operations);next.view.id=fresh();next.view.plan=canonical_review_plan(next.operations);next.view.operations=good(document::serialize_evaluation(next.operations));next.consumed=false;next.edit_failed=false;next.cancellation=std::make_shared<evaluation::Cancellation>();
    ready();stale();next.generation=advance_expected(expected_generation);install_review(std::move(next),true);return review_->view;
}
AdvisoryViewV2 EditorContextV2::restore_review(const std::string& id){
    std::lock_guard<std::mutex> lock(gate_);review_current(id,true);review_->edit_failed=true;review_->approval.clear();review_->approved_digest.clear();const auto expected_generation=advance_expected(review_->generation);review_->generation=expected_generation;
    auto resource_work=admit_resources();auto next=*review_;next.operations=next.original;next.view.id=fresh();next.view.plan=canonical_review_plan(next.operations);next.view.operations=good(document::serialize_evaluation(next.operations));next.consumed=false;next.edit_failed=false;next.cancellation=std::make_shared<evaluation::Cancellation>();
    ready();stale();next.generation=advance_expected(expected_generation);install_review(std::move(next),true);return review_->view;
}
std::string EditorContextV2::approve_from_human(const std::string& id){std::lock_guard<std::mutex> lock(gate_);review_current(id);need(!review_->operations.empty(),7);review_->approval=fresh();review_->approved_digest=review_digest();review_->consumed=false;return review_->approval;}
document::Snapshot EditorContextV2::review_candidate(){
    need(review_.has_value(),9);auto json=good(document::serialize_evaluation(review_->operations));const auto begin=json.find('['),end=json.rfind(']');need(begin!=document::String::npos&&end>begin,17);
    document::String tail=",\"revision_id\":\"";tail.append(fresh());tail.append("\",\"actor\":\"ai\",\"plan_id\":\"");tail.append(review_->view.id);tail.append("\",\"operations\":");tail.append(json.substr(begin,end-begin+1));tail.append(",\"stack\":[");
    const auto prior=good(document::replay(*loaded_.snapshot,loaded_.snapshot->current()));bool first=true;for(const auto& stack:{prior,review_->operations})for(const auto& op:stack){if(!first)tail+=',';first=false;tail+='"';tail.append(op.id.text());tail+='"';}tail+=']';
    auto base=command(*loaded_.snapshot,"commit");base.pop_back();base.append(tail);base+='}';return good(document::transition(*loaded_.snapshot,base));
}
bool EditorContextV2::execute_approved(const std::string& id,const std::string& approval){return execute_approved_receipt(id,approval).kind()=="approved_ai_apply";}
storage::NativeCommitReceiptV2 EditorContextV2::execute_approved_receipt(const std::string& id,const std::string& approval){
    Work active(*this);std::unique_lock<std::mutex> lock(gate_);review_current(id);need(!approval.empty()&&review_->approval==approval&&!review_->consumed&&review_->approved_digest==review_digest(),15);
    review_->consumed=true;review_->approval.clear();publication_phase_=1;struct Reset{std::atomic<unsigned>& p;~Reset(){p=0;}} reset{publication_phase_};
#ifdef PIXAURA_CERTIFICATION_TESTING
    CertificationV2::Run test_run(certification_[2]);
#endif
    auto resource_work=admit_resources();auto metadata=std::unique_ptr<tiled::Budget::Lease>(new tiled::Budget::Lease(budget_->reserve(repository_->metadata_reservation_v2()+1048576)));auto candidate=review_candidate();
    const auto generation=generation_.load();auto cancellation=review_->cancellation;cancellation->generation=&generation_;cancellation->expected=generation;cancellation->revoked=&revoked_;
    integration::RevisionProjectionV2 projection(candidate,*source_,*budget_);const auto size=projection.plan().extent();
    lock.unlock();try{
#ifdef PIXAURA_CERTIFICATION_TESTING
    certification_[2].checkpoint(1,30);
#endif
    for(uint32_t y=0;y<size.height;y+=128)for(uint32_t x=0;x<size.width;x+=128){auto tile=projection.plan().render({x,y,std::min(x+128,size.width),std::min(y+128,size.height)},*budget_,cancellation.get());(void)tile;}
#ifdef PIXAURA_CERTIFICATION_TESTING
    certification_[2].checkpoint(2,30);certification_[2].checkpoint(3,30);
#endif
    }catch(...){lock.lock();throw;}
    lock.lock();ready();stale();evaluation::checkpoint(cancellation.get(),evaluation::Checkpoint::publication);
    auto workspace=budget_->reserve(repository_->metadata_reservation_v2()*3+1048576);const auto serialized=good(document::serialize_commit(*loaded_.snapshot,*candidate));ready();evaluation::checkpoint(cancellation.get(),evaluation::Checkpoint::publication);unsigned evaluating=1;need(publication_phase_.compare_exchange_strong(evaluating,2),13);
    const auto receipt=repository_->publish_v2(serialized,loaded_.epoch,*commercial_,key_);loaded_={std::move(candidate),receipt.epoch()};metadata_lease_=std::move(metadata);clear();advance_generation();try{(void)commercial_->reconcile(*repository_);commercial_unavailable_=false;}catch(...){commercial_unavailable_=true;}return receipt;
}
uint64_t EditorContextV2::begin_review_preview(const std::string& id,uint32_t width,uint32_t height){
    std::lock_guard<std::mutex> lock(gate_);review_current(id);need(width&&height&&width<=1024&&height<=1024,7);
    auto task=std::make_shared<PreviewWork>();task->review=id;task->generation=generation_;task->epoch=loaded_.epoch;task->width=width;task->height=height;
    task->cancellation=std::make_shared<evaluation::Cancellation>();task->review_control=review_->cancellation;
    task->cancellation->generation=&generation_;task->cancellation->expected=task->generation;task->cancellation->revoked=&task->review_control->requested;
    std::lock_guard<std::mutex> control(control_gate_);need(preview_work_.size()<4&&serial_!=UINT64_MAX,8);const auto token=++serial_;preview_work_.emplace(token,std::move(task));return token;
}
void EditorContextV2::cancel_review_preview(uint64_t ticket){
    need(ticket,1);std::lock_guard<std::mutex> control(control_gate_);const auto found=preview_work_.find(ticket);need(found!=preview_work_.end(),9);found->second->cancellation->requested=true;
}
void EditorContextV2::release_review_preview(uint64_t ticket){
    need(ticket,1);std::lock_guard<std::mutex> control(control_gate_);const auto found=preview_work_.find(ticket);need(found!=preview_work_.end(),9);found->second->cancellation->requested=true;preview_work_.erase(found);
}
std::array<FrameV2,2> EditorContextV2::render_review_preview(uint64_t ticket){
    Work active(*this);need(ticket,1);std::unique_lock<std::mutex> lock(gate_);std::shared_ptr<PreviewWork> task;
    {std::lock_guard<std::mutex> control(control_gate_);const auto found=preview_work_.find(ticket);need(found!=preview_work_.end(),9);task=found->second;need(!task->started,15);task->started=true;}
    review_current(task->review);need(task->generation==generation_&&task->epoch==loaded_.epoch,9);
    evaluation::checkpoint(task->cancellation.get(),evaluation::Checkpoint::admission);
    auto resource_work=admit_resources();auto stack=good(document::replay(*loaded_.snapshot,loaded_.snapshot->current()));
    integration::StagePlanV2 before(*source_,stack,*budget_);
    for(const auto& op:review_->operations)stack.push_back(op);
    integration::StagePlanV2 after(*source_,stack,*budget_);
    lock.unlock();
    // Immutable plans/source plus scoped cancellation cross the unlocked stage.
    // Work keeps stop/join authoritative; cancellation never waits for pixels.
    try{
#ifdef PIXAURA_CERTIFICATION_TESTING
        CertificationV2::Run test_run(certification_[3]);certification_[3].checkpoint(0);
        const auto fault=certification_[3].fault.load();if(fault==1)throw std::bad_alloc();if(fault==2)throw document::Failure{8};if(fault==3)throw document::Failure{14};
#endif
        auto original=integration::fit_preview_v2(before,task->width,task->height,*budget_,task->cancellation.get());
        auto edited=integration::fit_preview_v2(after,task->width,task->height,*budget_,task->cancellation.get());
#ifdef PIXAURA_CERTIFICATION_TESTING
        certification_[3].checkpoint(1);certification_[3].checkpoint(2);
#endif
        evaluation::checkpoint(task->cancellation.get(),evaluation::Checkpoint::publication);
        lock.lock();review_current(task->review);need(task->generation==generation_&&task->epoch==loaded_.epoch,9);
        return {{{budget_,std::move(original),identity_,task->generation,task->epoch,task->cancellation,task->review_control},
                 {budget_,std::move(edited),identity_,task->generation,task->epoch,task->cancellation,task->review_control}}};
    }catch(...){if(!lock.owns_lock())lock.lock();throw;}
}

FrameV2 EditorContextV2::preview_advisory(const std::string& id,uint32_t width,uint32_t height){std::lock_guard<std::mutex> lock(gate_);review_current(id);need(width&&height&&width<=1024&&height<=1024,7);auto resource_work=admit_resources();auto stack=good(document::replay(*loaded_.snapshot,loaded_.snapshot->current()));for(const auto& op:review_->operations)stack.push_back(op);const auto generation=generation_.load();evaluation::Cancellation cancellation;cancellation.generation=&generation_;cancellation.expected=generation;cancellation.revoked=&review_->cancellation->requested;integration::StagePlanV2 plan(*source_,stack,*budget_);auto pixels=integration::fit_preview_v2(plan,width,height,*budget_,&cancellation);return {budget_,std::move(pixels),identity_,generation,loaded_.epoch,review_->cancellation};}
document::String EditorContextV2::advisory_content(const std::string& id,bool operations){std::lock_guard<std::mutex> lock(gate_);review_current(id);return operations?review_->view.operations:review_->view.plan;}
void EditorContextV2::join(){
    // Unlocked provider/evaluation stages retain immutable inputs and participate
    // in stop/join. No context destruction can outrun those workers.
    {std::unique_lock<std::mutex> lock(work_gate_);work_idle_.wait(lock,[&]{return active_work_==0;});}
    std::lock_guard<std::mutex> lock(gate_);
}
void EditorContextV2::stop()noexcept{stopped_=true;
#ifdef PIXAURA_CERTIFICATION_TESTING
    for(auto& test:certification_)test.resume=1;
#endif
provider_.stop();unsigned phase=1;(void)publication_phase_.compare_exchange_strong(phase,3);if(publication_phase_!=2&&publication_phase_!=4){revoked_=true;advance_generation();}}
}
