#include "native_owner_v1.hpp"
#include <algorithm>
#include <array>
#include "../core/src/storage_files.hpp"
#include "../core/vendor/sqlite/sqlite3.h"
namespace pixaura::commerce {
#ifdef PIXAURA_V18_TESTING
void (*NativeCommercialOwner::checkpoint_)(unsigned)=nullptr;
#endif
namespace {
void need(bool ok,int code=7){if(!ok){throw document::Failure{code};}}
// Hold the native project's writer reservation WITHOUT changing schema/data.
// WAL readers/recovery remain available; all SQLite publishers must wait/fail BUSY.
// The actual ledger COMMIT occurs while the native revision is fenced.
class PublicationFenceV1 final {
    storage::Files files_;sqlite3* database_=nullptr;
public:
    explicit PublicationFenceV1(const std::string& root):files_(root){
        const auto path=files_.database_path();
        const auto opened=sqlite3_open_v2(path.c_str(),&database_,SQLITE_OPEN_READWRITE|SQLITE_OPEN_FULLMUTEX|SQLITE_OPEN_NOFOLLOW,nullptr);
        if(opened!=SQLITE_OK){if(database_)sqlite3_close_v2(database_);database_=nullptr;throw document::Failure{12};}
        const auto rc=sqlite3_exec(database_,"BEGIN IMMEDIATE",nullptr,nullptr,nullptr);
        if(rc!=SQLITE_OK){sqlite3_close_v2(database_);database_=nullptr;throw document::Failure{(rc==SQLITE_BUSY||rc==SQLITE_LOCKED)?15:12};}
    }
    ~PublicationFenceV1(){if(database_){sqlite3_exec(database_,"ROLLBACK",nullptr,nullptr,nullptr);sqlite3_close_v2(database_);}}
    PublicationFenceV1(const PublicationFenceV1&)=delete;
};
std::string text(const document::String& value){return {value.data(),value.size()};}
std::string capsule(const document::ImageDocument& doc,const std::string& goal,const std::string& parameters){
    return "PXA-ADVISORY-1\n"+text(doc.source().sha256.text())+"\n"+std::to_string(doc.source().byte_length)+"\n"+
        text(doc.identity().project.text())+"\n"+text(doc.identity().document.text())+"\n"+text(doc.current().text())+"\n"+goal+"\n"+parameters;
}
std::array<std::string,8> fields(const std::string& payload){
    need(payload.size()<=4096,17);std::array<std::string,8> values;std::size_t start=0;
    for(std::size_t i=0;i<7;++i){const auto end=payload.find('\n',start);need(end!=std::string::npos,17);values[i]=payload.substr(start,end-start);start=end+1;}
    values[7]=payload.substr(start);return values;
}
}
NativeCommercialOwner::NativeCommercialOwner(const std::string& ledger_path,const std::string& project_root,
    const integration::OriginalReaderV2& source,tiled::Budget& budget,NativeClockSourceV1& clock,std::string native_context):
    ledger_(ledger_path),repository_(project_root),source_(source),budget_(budget),clock_(clock),context_(std::move(native_context)),project_root_(project_root){
    (void)document::Id::parse(context_);(void)current();
}
storage::Loaded NativeCommercialOwner::current(){
    auto loaded=repository_.read();
    // Real native stored snapshot + immutable decoded-source equality, not UI state.
    integration::RevisionProjectionV2 verified(loaded.snapshot,source_,budget_);
    return loaded;
}
NativeSourceReceiptV1 NativeCommercialOwner::receipt(const storage::Loaded& loaded){
    const auto& doc=*loaded.snapshot;
    return {text(doc.source().sha256.text()),text(doc.identity().document.text()),text(doc.current().text()),doc.source().byte_length,loaded.epoch};
}
document::ReferencePlan NativeCommercialOwner::plan(const storage::Loaded& loaded,const std::string& goal,const std::string& parameters,const evaluation::Cancellation* cancel){
    evaluation::checkpoint(cancel,evaluation::Checkpoint::admission);
    need(!goal.empty()&&goal.size()<=64&&parameters.size()<=1024);
    need(std::all_of(goal.begin(),goal.end(),[](char c){return (c>='a'&&c<='z')||c=='.'||c=='-'||c=='_';}));
    // The existing v1 approval/execution geometry boundary does not admit >8MP.
    // Do not charge for a proposal that its production Human Apply cannot execute.
    need(uint64_t(source_.width())*source_.height()<=working::defaults().pixel_count,8);
    auto binding=document::agent_schema_binding(*loaded.snapshot,context_);if(binding.code)throw document::Failure{binding.code};
    const auto input="{\"binding\":"+text(binding.value)+",\"goal_id\":\""+goal+"\",\"parameters\":"+parameters+",\"structured_intent_version\":1}";
    auto result=document::plan_structured_intent(input,*loaded.snapshot,context_);if(result.code)throw document::Failure{result.code};
    // Usability is stricter than parsing: reserve the frozen legacy original,
    // candidate/scratch and worst-case display payload. Never charge a result
    // that only the experimental tiled executor could render.
    const auto pixels=uint64_t(source_.width())*source_.height();
    const auto candidate=evaluation::reservation(source_.width(),source_.height(),result.value.operations,working::defaults());
    auto legacy_execution=budget_.reserve(pixels*20+candidate);
    integration::StagePlanV2 checked(source_,result.value.operations,budget_);
    auto proof=checked.render({0,0,std::min(1u,checked.extent().width),std::min(1u,checked.extent().height)},budget_,cancel);
    (void)proof;return std::move(result.value);
}
Admission NativeCommercialOwner::admit_editable_source(const std::string& request){
    std::lock_guard<std::mutex> lock(gate_);PublicationFenceV1 fence(project_root_);const auto loaded=current();const auto clock=clock_.sample();
    const auto lease=ledger_.reserve(request,text(loaded.snapshot->source().sha256.text()),false,clock);
    if(lease.decision!=Admission::allowed)return lease;
    try{
        integration::RevisionProjectionV2 projection(loaded.snapshot,source_,budget_);
        auto proof=projection.plan().render({0,0,1,1},budget_);(void)proof;
        need(current().epoch==loaded.epoch,9);
        need(ledger_.complete(lease.token,true,false,"",clock_.sample()),12);return lease;
    }catch(...){ledger_.complete(lease.token,false,false,"",clock);throw;}
}
AdvisoryV1 NativeCommercialOwner::restore_locked(const std::string& token,const storage::Loaded& loaded){
    const auto payload=ledger_.recover_delivery(token);need(!payload.empty(),9);const auto values=fields(payload);const auto& doc=*loaded.snapshot;
    need(values[0]=="PXA-ADVISORY-1"&&values[1]==text(doc.source().sha256.text())&&values[2]==std::to_string(doc.source().byte_length)&&
        values[3]==text(doc.identity().project.text())&&values[4]==text(doc.identity().document.text())&&values[5]==text(doc.current().text()),9);
    auto validated=plan(loaded,values[6],values[7]);need(current().epoch==loaded.epoch,9);
    // Rebuild only untrusted advisory bytes under fresh native session/context.
    // No approval, candidate, execution permission or additional quota charge.
    return {token,text(validated.plan),receipt(loaded)};
}
void NativeCommercialOwner::begin_request(const std::string& id){std::lock_guard<std::mutex> lock(completion_gate_);active_request_=id;cancellation_.requested=false;running_=true;}
void NativeCommercialOwner::retire_request(){std::lock_guard<std::mutex> lock(completion_gate_);active_request_.clear();running_=false;}
void NativeCommercialOwner::completed_request(const std::string& id){last_delivered_=id;running_=false;}
NativeCancelV1 NativeCommercialOwner::cancel_advisory(const std::string& id){
    std::lock_guard<std::mutex> lock(completion_gate_);
    if(running_&&active_request_==id){cancellation_.requested=true;return NativeCancelV1::requested;}
    return last_delivered_==id?NativeCancelV1::already_delivered:NativeCancelV1::not_active;
}
bool NativeCommercialOwner::terminal_request(const std::string& token){
    std::lock_guard<std::mutex> lock(ledger_.mutex_);sqlite3_stmt* query=nullptr;
    need(sqlite3_prepare_v2(ledger_.db_,"SELECT done FROM request WHERE token=?",-1,&query,nullptr)==SQLITE_OK,12);
    struct Finalize{sqlite3_stmt* query;~Finalize(){sqlite3_finalize(query);}} cleanup{query};
    need(sqlite3_bind_text(query,1,token.c_str(),static_cast<int>(token.size()),SQLITE_TRANSIENT)==SQLITE_OK,12);
    const auto rc=sqlite3_step(query);need(rc==SQLITE_ROW||rc==SQLITE_DONE,12);
    if(rc==SQLITE_DONE)return false;
    need(sqlite3_column_type(query,0)==SQLITE_INTEGER,17);const auto done=sqlite3_column_int(query,0);need(done==0||done==1,17);return done==1;
}
AdvisoryV1 NativeCommercialOwner::deliver_advisory(const std::string& request,const std::string& goal,const std::string& parameters){
    std::lock_guard<std::mutex> lock(gate_);PublicationFenceV1 fence(project_root_);const auto loaded=current();const auto clock=clock_.sample();
    const auto lease=ledger_.reserve(request,text(loaded.snapshot->source().sha256.text()),true,clock);
    if(lease.decision!=Admission::allowed)throw CommercialDenialV1{lease.decision};
    begin_request(request);
    auto cleanup=[this](NativeCommercialOwner*){retire_request();};
    std::unique_ptr<NativeCommercialOwner,decltype(cleanup)> retirement(this,cleanup);
    checkpoint(1);
    if(!ledger_.recover_delivery(lease.token).empty()){
        auto restored=restore_locked(lease.token,loaded);
        std::lock_guard<std::mutex> completion(completion_gate_);evaluation::checkpoint(&cancellation_,evaluation::Checkpoint::publication);completed_request(request);return restored;
    }
    need(!terminal_request(lease.token),9); // Dismissed request cannot be recycled into free advice.
    try{
        auto validated=plan(loaded,goal,parameters,&cancellation_);checkpoint(2);evaluation::checkpoint(&cancellation_,evaluation::Checkpoint::publication);need(current().epoch==loaded.epoch,9);
        const auto payload=capsule(*loaded.snapshot,goal,parameters);need(payload.size()<=4096,8);
        checkpoint(3);
        {std::lock_guard<std::mutex> completion(completion_gate_);
            evaluation::checkpoint(&cancellation_,evaluation::Checkpoint::publication);
            need(ledger_.complete(lease.token,true,false,payload,clock_.sample()),12);completed_request(request);
        }
        checkpoint(4);
        return {lease.token,text(validated.plan),receipt(loaded)};
    }catch(...){ledger_.complete(lease.token,false,false,"",clock);throw;}
}
AdvisoryV1 NativeCommercialOwner::restore_advisory(const std::string& token){std::lock_guard<std::mutex> lock(gate_);PublicationFenceV1 fence(project_root_);return restore_locked(token,current());}
void NativeCommercialOwner::dismiss_advisory(const std::string& token){std::lock_guard<std::mutex> lock(gate_);ledger_.acknowledge_delivery(token);}
State NativeCommercialOwner::state(){std::lock_guard<std::mutex> lock(gate_);return ledger_.state(clock_.sample());}
void NativeCommercialOwner::verified_store_fact(const std::string& product,const Entitlement& entitlement){std::lock_guard<std::mutex> lock(gate_);ledger_.verified_store_fact(product,entitlement);}
}
