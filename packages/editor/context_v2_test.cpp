#include "context_v2.hpp"
#include "../commerce/receipt_key_v2.hpp"
#include "../core/src/decode.hpp"
#include "../core/vendor/sqlite/sqlite3.h"
#include <type_traits>
#include <future>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <stdexcept>
namespace pixaura::editor {
struct EditorContextV2TestAccess {
    static std::unique_lock<std::mutex> hold(EditorContextV2& owner){return std::unique_lock<std::mutex>(owner.gate_);}
};
}
namespace pixaura::commerce {
// Synthetic corruption injection in the native test executable only.
struct LedgerTestAccess {static sqlite3* db(Ledger& owner){return owner.db_;}};
struct CommercialCoordinatorV2TestAccess {
    static std::string payload(CommercialCoordinatorV2& owner,const std::string& token){sqlite3_stmt* q=nullptr;auto* db=LedgerTestAccess::db(owner.ledger_);if(sqlite3_prepare_v2(db,"SELECT payload FROM request WHERE token=?",-1,&q,nullptr)!=SQLITE_OK)throw std::runtime_error("prepare payload");sqlite3_bind_text(q,1,token.c_str(),-1,SQLITE_TRANSIENT);if(sqlite3_step(q)!=SQLITE_ROW){sqlite3_finalize(q);throw std::runtime_error("missing payload");}std::string value(reinterpret_cast<const char*>(sqlite3_column_text(q,0)));sqlite3_finalize(q);return value;}
    static void replace(CommercialCoordinatorV2& owner,const std::string& token,const std::string& payload){sqlite3_stmt* q=nullptr;auto* db=LedgerTestAccess::db(owner.ledger_);if(sqlite3_prepare_v2(db,"UPDATE request SET payload=? WHERE token=?",-1,&q,nullptr)!=SQLITE_OK)throw std::runtime_error("prepare corruption");sqlite3_bind_text(q,1,payload.c_str(),-1,SQLITE_TRANSIENT);sqlite3_bind_text(q,2,token.c_str(),-1,SQLITE_TRANSIENT);const auto code=sqlite3_step(q);sqlite3_finalize(q);if(code!=SQLITE_DONE)throw std::runtime_error("inject corruption");}
};
}
static_assert(!std::is_default_constructible_v<pixaura::storage::NativeAdvisoryDeliveryV2>,"UI cannot construct delivery proof");
static_assert(!std::is_default_constructible_v<pixaura::storage::NativeCommitReceiptV2>,"UI cannot construct edit proof");
using namespace pixaura;
namespace fs=std::filesystem;
#define CHECK(x) do{if(!(x))throw std::runtime_error("editorv2 test line "+std::to_string(__LINE__));}while(false)
struct OSClock:commerce::CommercialClockV2 {commerce::Clock sample_os()override{return {20000LL*86400,100,std::string(32,'a'),false};}};
struct Input:storage::Reader {
    std::ifstream file;
    explicit Input(const char* path):file(path,std::ios::binary){CHECK(file);}
    std::size_t read(uint8_t* p,std::size_t n)override{file.read(reinterpret_cast<char*>(p),static_cast<std::streamsize>(n));if(file.bad())throw document::Failure{12};return static_cast<std::size_t>(file.gcount());}
};
struct FileSink:integration::StagedSinkV2 {
    std::ofstream out;bool sealed=false,aborted=false;
    explicit FileSink(const std::string& path):out(path,std::ios::binary){CHECK(out);}
    void write(const uint8_t* p,std::size_t n)override{out.write(reinterpret_cast<const char*>(p),static_cast<std::streamsize>(n));if(!out)throw document::Failure{12};}
    void seal()override{out.close();if(!out)throw document::Failure{12};sealed=true;}
    void abort()noexcept override{out.close();aborted=true;}
};
template<class F>void fails(int code,F f){bool failed=false;try{f();}catch(const document::Failure& e){CHECK(e.code==code);failed=true;}CHECK(failed);}
int main(int argc,char** argv){try{
    CHECK(argc>=2);const bool prepare=std::string(argv[1])=="--prepare";CHECK((prepare&&argc==3)||(!prepare&&argc==2&&std::string(argv[1])=="--recover"));
    std::string root,committed,delivery,old_approval,old_proposal;
    if(prepare){root=(fs::current_path()/("editor-v2-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()))).string();CHECK(fs::create_directory(root));}
    else{std::ifstream marker("editor-v2-recovery-state");CHECK(marker);for(auto* value:{&root,&committed,&delivery,&old_approval,&old_proposal}){CHECK(bool(std::getline(marker,*value))&&value->size()<=4095);}CHECK(!root.empty()&&committed.size()==32&&old_approval.size()==32&&old_proposal.size()==32);}
    std::array<uint8_t,32> synthetic{};for(unsigned i=0;i<32;++i)synthetic[i]=static_cast<uint8_t>(i+1);commerce::ReceiptKeyV2 key(synthetic.data(),synthetic.size());OSClock clock;commerce::CommercialCoordinatorV2 commercial(root+"/commercial.sqlite",clock,key);
    auto budget=std::make_shared<tiled::Budget>(134217728,0);
    if(prepare){
    {
        std::fprintf(stderr,"stage import\n");Input file(argv[2]);editor::EditorContextV2 owner(root,&file,fs::file_size(argv[2]),true,budget,commercial,key);owner.activate_import();CHECK(owner.state().revisions==1&&commercial.state().trial_start==-1);
        auto before=owner.preview(64,48,true);
        // A nonblocking UI probe may report false for a VALID frame solely
        // because metadata owns the gate. Worker delivery must use the waiting
        // binding check, never interpret that UI probe as stale-result proof.
        std::promise<void> locked,release;auto unlocked=release.get_future();
        auto holder=std::async(std::launch::async,[&]{auto gate=editor::EditorContextV2TestAccess::hold(owner);locked.set_value();unlocked.wait();});
        locked.get_future().wait();const bool busy_probe=owner.current(before);const auto classified=owner.current_status(before);
        auto waiting=std::async(std::launch::async,[&]{return owner.current(before,true);});
        const auto waiting_status=waiting.wait_for(std::chrono::milliseconds(20));
        release.set_value();holder.get();const bool binding_valid=waiting.get();
        CHECK(!busy_probe&&classified==8&&waiting_status==std::future_status::timeout&&binding_valid&&owner.current_status(before)==0);
        CHECK(owner.state().revisions==1);
        std::puts("Native valid-frame contention: nonblocking false, waiting binding true, history unchanged PASS");
CHECK(before.pixels.source.width==4000&&before.pixels.source.height==3000&&owner.current(before));
        uint32_t exposure=UINT32_MAX,blur=UINT32_MAX;for(uint32_t i=0;i<manual::registry().size();++i){if(manual::registry()[i].tool_id=="pixaura.exposure")exposure=i;if(manual::registry()[i].tool_id=="pixaura.blur")blur=i;}CHECK(exposure!=UINT32_MAX&&blur!=UINT32_MAX);
        const auto cancelled_request=owner.begin_advisory(2);
        owner.cancel_advisory_request(cancelled_request);
        fails(13,[&]{owner.run_advisory(cancelled_request);});
        CHECK(commercial.state().ai==0&&owner.state().revisions==1);
        owner.select(exposure);const int32_t early_value=1000;owner.update(&early_value,1);
        fails(9,[&]{owner.cancel_advisory_request(cancelled_request);});
        CHECK(owner.state().selected==exposure&&owner.state().values==std::vector<int32_t>{early_value});owner.cancel();
        std::fprintf(stderr,"stage free AI\n");for(unsigned request=0;request<3;++request){auto advice=owner.request_advisory(2);CHECK(owner.state().revisions==1&&commercial.state().trial_start==-1&&commercial.state().ai==int(request+1));if(request==0){auto comparison=owner.preview_advisory(advice.id,64,48);CHECK(owner.current(comparison));owner.cancel_advisory(advice.id);CHECK(!owner.current(comparison));}else owner.cancel();}
        fails(3002,[&]{owner.request_advisory(2);});CHECK(commercial.state().ai==3);
        // Exhausting AI leaves the ordinary manual native path available.
        std::fprintf(stderr,"stage manual\n");owner.select(exposure);const int32_t value=1000;owner.update(&value,1);CHECK(owner.state().revisions==1&&!owner.current(before));auto preview=owner.preview(64,48);CHECK(preview.pixels.pixels!=before.pixels.pixels);CHECK(owner.human_apply());CHECK(owner.state().revisions==2&&commercial.state().trial_start==20000LL*86400);committed=owner.state().revision;
        CHECK(!owner.current(preview));owner.select(blur);const int32_t amount=500;owner.update(&amount,1);owner.cancel();CHECK(owner.state().revisions==2&&owner.state().revision==committed);
        owner.history(false);CHECK(owner.state().revision!=committed&&owner.state().revisions==2);owner.history(true);CHECK(owner.state().revision==committed);
        std::fprintf(stderr,"stage trial AI\n");auto proposal=owner.request_advisory(2);CHECK(owner.state().revisions==2&&commercial.state().ai==3);
        fails(15,[&]{owner.execute_approved(proposal.id,"");});
        const auto cancelled_preview=owner.begin_review_preview(proposal.id,64,48);
        owner.cancel_review_preview(cancelled_preview);fails(13,[&]{owner.render_review_preview(cancelled_preview);});
        CHECK(!owner.advisory_content(proposal.id,false).empty());owner.release_review_preview(cancelled_preview);
        const auto preview_ticket=owner.begin_review_preview(proposal.id,64,48);
        auto comparison=owner.render_review_preview(preview_ticket);CHECK(owner.current(comparison[0])&&owner.current(comparison[1]));
        owner.cancel_review_preview(preview_ticket);CHECK(!owner.current(comparison[0])&&!owner.current(comparison[1]));
        CHECK(!owner.advisory_content(proposal.id,false).empty());owner.release_review_preview(preview_ticket);
        const auto retired_approval=owner.approve_from_human(proposal.id);
        bool edit_rejected=false;try{(void)owner.edit_advisory(proposal.id,"{");}catch(const document::Failure&){edit_rejected=true;}CHECK(edit_rejected);
        fails(13,[&]{owner.approve_from_human(proposal.id);});
        fails(13,[&]{owner.execute_approved(proposal.id,retired_approval);});
        proposal=owner.restore_review(proposal.id);
        const auto approval=owner.approve_from_human(proposal.id);
        auto empty=owner.edit_advisory(proposal.id,"{\"operations\":[]}");
        fails(9,[&]{owner.approve_from_human(proposal.id);});fails(7,[&]{owner.approve_from_human(empty.id);});
        auto restored=owner.restore_review(empty.id);fails(9,[&]{owner.execute_approved(proposal.id,approval);});
        fails(15,[&]{owner.execute_approved(restored.id,approval);});
        const auto fresh=owner.approve_from_human(restored.id);CHECK(owner.execute_approved(restored.id,fresh));CHECK(owner.state().revisions==3);committed=owner.state().revision;
        fails(9,[&]{owner.execute_approved(restored.id,fresh);});
        owner.select(exposure);owner.update(&value,1);fails(9,[&]{owner.cancel_advisory(restored.id);});CHECK(owner.state().selected==exposure&&owner.state().values==std::vector<int32_t>{value});owner.cancel();
        const auto binding=owner.agent_bytes(1,{});
        const std::string actions="{\"action_index\":0,\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":101},\"tool_id\":\"pixaura.exposure\"},{\"action_index\":1,\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_linear\":-123},\"tool_id\":\"pixaura.brightness\"},{\"action_index\":2,\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"quarter_turns\":1},\"tool_id\":\"pixaura.rotate\"}";
        const auto canonical=owner.agent_bytes(2,"{\"actions\":["+actions+"],\"binding\":"+std::string(binding)+",\"plan_version\":1}");
        const auto cancelled_plan=owner.begin_plan_candidate(canonical);owner.cancel_advisory_request(cancelled_plan);fails(13,[&]{owner.run_advisory(cancelled_plan);});
        std::fprintf(stderr,"stage next advisory\n");auto recoverable=owner.run_advisory(owner.begin_plan_candidate(canonical));CHECK(owner.inspect_review(recoverable.id).action_count==3);delivery=recoverable.delivery;old_proposal=recoverable.id;old_approval=owner.approve_from_human(recoverable.id);
        owner.stop();CHECK(!owner.current(before));
    }
    CHECK(budget->inflight()==0);
    // Test-only replay attack material, never a product advisory record. The
    // next PROCESS must reject this abandoned approval and restore no capability.
    std::ofstream marker("editor-v2-recovery-state",std::ios::trunc);CHECK(marker);
    for(const auto* value:{&root,&committed,&delivery,&old_approval,&old_proposal})marker<<*value<<'\n';marker.close();CHECK(marker);
    std::puts("Native real12MP publication, quotas, trial, single-use review authority and lease release PASS");return 0;
    }
    {
        editor::EditorContextV2 reopened(root,nullptr,0,false,budget,commercial,key);CHECK(reopened.state().revision==committed&&reopened.state().revisions==3);CHECK(commercial.state().trial_start==20000LL*86400);
        using Access=commerce::CommercialCoordinatorV2TestAccess;
        const auto payload=Access::payload(commercial,delivery);auto corrupt=payload;corrupt.back()=corrupt.back()=='2'?'3':'2';Access::replace(commercial,delivery,corrupt);
        fails(6,[&]{reopened.pending_advisories();});fails(6,[&]{reopened.restore_advisory(delivery);});CHECK(commercial.state().ai==3);
        reopened.select(0);reopened.cancel();CHECK(reopened.state().revisions==3); // manual survives untrusted inbox corruption
        Access::replace(commercial,delivery,payload);
        CHECK(reopened.pending_advisories()==std::vector<std::string>{delivery});
        auto restored=reopened.restore_advisory(delivery);CHECK(restored.id!=old_proposal&&commercial.state().ai==3);CHECK(reopened.inspect_review(restored.id).action_count==3);
        fails(9,[&]{reopened.execute_approved(old_proposal,old_approval);});fails(15,[&]{reopened.execute_approved(restored.id,old_approval);});
        const auto cancelled_export=reopened.begin_export(2,93);reopened.cancel_export(cancelled_export);
        {FileSink sink(root+"/cancelled-export.png");fails(13,[&]{reopened.run_export(cancelled_export,sink);});CHECK(sink.aborted&&!sink.sealed);}
        reopened.release_export(cancelled_export);CHECK(!reopened.advisory_content(restored.id,false).empty());
        const auto original_saved=reopened.state().revision;const auto authorized_export=reopened.begin_export(2,93);
        reopened.history(false);CHECK(reopened.state().revision!=original_saved);
        {FileSink sink(root+"/authorized-snapshot.png");const auto exported=reopened.run_export(authorized_export,sink);CHECK(exported.revision==original_saved&&exported.bytes>0&&exported.width==4000&&sink.sealed);}
        reopened.release_export(authorized_export);reopened.history(true);
        reopened.cancel();auto image=reopened.preview(64,48);CHECK(image.pixels.source.width==4000&&image.pixels.source.height==3000);
        for(unsigned format:{1u,2u}){FileSink sink(root+(format==1?"/full.jpg":"/full.png"));CHECK(reopened.export_saved(format,93,sink)>0&&sink.sealed&&!sink.aborted);}

    }
    CHECK(budget->inflight()==0);
    {
        editor::EditorContextV2 recovery(root,nullptr,0,false,budget,static_cast<commerce::CommercialCoordinatorV2*>(nullptr),key);
        CHECK(recovery.state().recovery_only&&recovery.state().commerce_unavailable);
        auto image=recovery.preview(64,48);CHECK(image.pixels.source.width==4000);
        fails(3003,[&]{recovery.select(0);});fails(3003,[&]{recovery.request_advisory(2);});
        FileSink sink(root+"/cache-loss-recovery.png");CHECK(recovery.export_saved(2,93,sink)>0&&sink.sealed);
    }
    CHECK(budget->inflight()==0);
    {
        synthetic[0]^=0xff;commerce::ReceiptKeyV2 wrong(synthetic.data(),synthetic.size());
        editor::EditorContextV2 recovery(root,nullptr,0,false,budget,commercial,wrong);
        CHECK(recovery.state().recovery_only);fails(3003,[&]{recovery.select(0);});
        CHECK(recovery.preview(64,48).pixels.source.width==4000);
    }
    CHECK(budget->inflight()==0);storage::Repository inspect(root,4);CHECK(inspect.receipts_v2(key).size()==2);
    for(const auto* file:{"/full.jpg","/full.png"}){std::ifstream stream(root+file,std::ios::binary|std::ios::ate);const auto n=stream.tellg();CHECK(n>0);stream.seekg(0);std::vector<uint8_t> bytes(static_cast<size_t>(n));CHECK(stream.read(reinterpret_cast<char*>(bytes.data()),n));auto m=decode::admit(bytes.data(),bytes.size(),decode::defaults());CHECK(m.value.width==4000&&m.value.height==3000);}
    std::printf("Native editor-context-v2 real4000x3000 import/manualBeforeAfter/HUMANApply/Cancel/UndoRedo/reopen/fullJPEGPNG +sameSQLreceipt/trial +leasesrelease PASS root%s\n",root.c_str());return 0;
}catch(const document::Failure& e){std::fprintf(stderr,"native failure%d\n",e.code);return 1;}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}}
