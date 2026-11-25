#ifndef PIXAURA_EDITOR_CONTEXT_V2_HPP
#define PIXAURA_EDITOR_CONTEXT_V2_HPP
#include "../../manual_tools.hpp"
#include "certification_v2.hpp"
#include "pixaura/editor_v2.h"
#include "../../storage.hpp"
#include "../integration/preview_v2.hpp"
#include "../integration/revision_projection_v2.hpp"
#include "../integration/stream_writer_v2.hpp"
#include "../commerce/coordinator_v2.hpp"
#include "../../assist_experience.hpp"
#include <optional>
#include <array>
#include <atomic>
#include <mutex>
#include <map>
#include <condition_variable>
namespace pixaura::editor {
struct AdvisoryViewV2 {std::string id,delivery,plan,operations;uint64_t epoch=0;uint32_t choice=0,tool=0;std::vector<int32_t> values;};
struct StateV2 {std::string revision;uint64_t epoch=0,generation=0;uint32_t revisions=0,selected=UINT32_MAX;bool pending=false,can_undo=false,can_redo=false,commerce_unavailable=false,recovery_only=false;std::vector<int32_t> values;};
struct ExportObservationV2 {uint64_t bytes=0,epoch=0;uint32_t width=0,height=0,format=0;std::string source,document,revision;};
struct FrameV2 {
    std::shared_ptr<tiled::Budget> budget_owner;
    integration::PreviewV2 pixels;
    document::String context_id;
    uint64_t generation=0,epoch=0;
    std::shared_ptr<evaluation::Cancellation> control{},review_control{};
};
// Sole native editing/repository owner for an application context. All IO/render
// calls run on platform workers. stop is lock-free; destruction follows join.
class EditorContextV2 final {
    friend struct EditorContextV2TestAccess; // native lock-contention regression only
#ifdef PIXAURA_CERTIFICATION_TESTING
    std::array<CertificationV2,4> certification_;
    std::atomic<uint32_t> advisory_test_family_{1};
    document::String test_provider(assistance::Request);
#endif
    std::shared_ptr<tiled::Budget> budget_;
    std::unique_ptr<tiled::Budget::Lease> control_,metadata_lease_,candidate_lease_;
    std::unique_ptr<storage::Repository> repository_;
    std::unique_ptr<integration::OriginalReaderV2> source_;
    storage::Loaded loaded_{};document::Snapshot candidate_;
    std::unique_ptr<manual::Gesture> gesture_;
    commerce::CommercialCoordinatorV2* commercial_;storage::ReceiptAuthorityV2& key_;
    document::String identity_,gesture_operation_id_;uint64_t serial_=0;
    std::function<std::shared_ptr<void>()> resource_admission_;
    std::shared_ptr<void> admit_resources(){return resource_admission_?resource_admission_():nullptr;}
    const manual::Descriptor* selected_=nullptr;std::vector<int32_t> values_;
    std::mutex gate_,control_gate_,work_gate_;
    std::condition_variable work_idle_;uint64_t active_work_=0;
    struct Work {EditorContextV2& owner;explicit Work(EditorContextV2&);~Work();Work(const Work&)=delete;};
    std::array<char,32> control_id_{};std::shared_ptr<evaluation::Cancellation> advisory_control_;
    std::atomic<bool> stopped_{false},revoked_{false};std::atomic<uint64_t> generation_{1};
    std::atomic<unsigned> publication_phase_{0}; //0idle,1evaluating,2publishing,3cancelled
    struct Review {
        AdvisoryViewV2 view;document::Vector<document::EditOperation> operations,original;
        std::shared_ptr<evaluation::Cancellation> cancellation;
        uint64_t generation=0;document::String approval,approved_digest;bool consumed=false,edit_failed=false;
    };
    struct PreviewWork {
        document::String review;uint64_t generation=0,epoch=0;uint32_t width=0,height=0;bool started=false;
        std::shared_ptr<evaluation::Cancellation> cancellation,review_control;
    };
    std::map<uint64_t,std::shared_ptr<PreviewWork>> preview_work_;
    struct ExportWork {
        document::Snapshot snapshot;uint64_t epoch=0;uint32_t format=0;int quality=0;bool started=false;
        std::atomic<uint64_t> generation{1};evaluation::Cancellation cancellation;
        std::unique_ptr<tiled::Budget::Lease> metadata;
    };
    std::map<uint64_t,std::shared_ptr<ExportWork>> export_work_;

    bool advisory_work_=false; // gate_: a live provider lane cannot be displaced
    struct AdvisoryWork {EditorContextV2& owner;explicit AdvisoryWork(EditorContextV2&);~AdvisoryWork();AdvisoryWork(const AdvisoryWork&)=delete;};
    uint64_t request_ticket_=0;uint32_t request_choice_=0;document::String request_plan_;
    std::optional<Review> review_;assistance::ReferenceProvider provider_;
    bool commercial_unavailable_=false,native_receipts_unverified_=false;
    uint64_t advance_generation()noexcept;
    uint64_t advance_expected(uint64_t);
    document::String fresh();void ready();void stale();void reload();void clear();
    void admit_source();
    void install_review(Review&&,bool replacing=false);
    Review plan_advisory(uint32_t,const std::string&,std::unique_lock<std::mutex>&);
    Review plan_candidate(std::string_view,const std::string&,std::unique_lock<std::mutex>&);
    document::String provider_envelope(assistance::Request,std::unique_lock<std::mutex>&);
    document::String restored_candidate_plan(std::string_view);
    Review decoded_review(assistance::Decoded&&,const std::string&,uint32_t,uint32_t,const std::vector<int32_t>&);
    document::String review_digest()const;
    document::String canonical_review_plan(const document::Vector<document::EditOperation>&);
    document::Snapshot review_candidate();
    void review_current(std::string_view,bool allow_failed=false);
public:
#ifdef PIXAURA_CERTIFICATION_TESTING
    int32_t certification_control(uint32_t,uint32_t,uint32_t);
#endif
    EditorContextV2(const std::string& canonical_private_root,storage::Reader*,uint64_t import_bytes,
        bool explicit_schema4_migration,std::shared_ptr<tiled::Budget>,
        commerce::CommercialCoordinatorV2&,storage::ReceiptAuthorityV2&,std::function<std::shared_ptr<void>()> ={});
    // A missing/corrupt commercial cache must not block recovery inspection/export.
    EditorContextV2(const std::string&,storage::Reader*,uint64_t,bool,
        std::shared_ptr<tiled::Budget>,commerce::CommercialCoordinatorV2*,storage::ReceiptAuthorityV2&,std::function<std::shared_ptr<void>()> ={});
    ~EditorContextV2();
    EditorContextV2(const EditorContextV2&)=delete;
    // Native bridge calls this only after all fallible handle allocations and
    // cancellation checks, immediately before publishing a usable import owner.
    void activate_import();
    void select(uint32_t);
    void update(const int32_t*,uint32_t);
    bool human_apply();void cancel();void history(bool redo);
    StateV2 state();
    document::String gesture_identity();
    document::String history_json();
    FrameV2 preview_revision(const std::string&,uint32_t,uint32_t);
    FrameV2 preview(uint32_t width,uint32_t height,bool original=false);
    bool current(const FrameV2&,bool wait=false);
    int32_t current_status(const FrameV2&,bool wait=false);
    uint64_t export_saved(uint32_t format,int quality,integration::StagedSinkV2&);
    ExportObservationV2 export_observed(uint32_t,int,integration::StagedSinkV2&);
    uint64_t begin_export(uint32_t,int);
    ExportObservationV2 run_export(uint64_t,integration::StagedSinkV2&);
    void cancel_export(uint64_t);
    void release_export(uint64_t);

    // Begin on the command lane before dispatching work. Tickets only identify
    // cancellable work; they confer neither delivery nor publication authority.
    document::String agent_bytes(uint32_t,std::string_view);
    uint64_t begin_advisory(uint32_t choice);
    uint64_t begin_plan_candidate(std::string_view);
    void cancel_advisory_request(uint64_t ticket);
    AdvisoryViewV2 run_advisory(uint64_t ticket);
    AdvisoryViewV2 request_advisory(uint32_t choice);
    AdvisoryViewV2 restore_advisory(const std::string& delivery);
    std::vector<std::string> pending_advisories();
    pixaura_v2_review_inspection inspect_review(const std::string&);
    AdvisoryViewV2 edit_review_command(const std::string&,uint32_t,uint32_t,uint32_t,int32_t,uint32_t);
    AdvisoryViewV2 edit_advisory(const std::string& id,std::string_view operations_json);
    AdvisoryViewV2 restore_review(const std::string& id);
    std::string approve_from_human(const std::string& id);
    bool execute_approved(const std::string& id,const std::string& approval);
    storage::NativeCommitReceiptV2 execute_approved_receipt(const std::string&,const std::string&);
    uint64_t begin_review_preview(const std::string&,uint32_t,uint32_t);
    std::array<FrameV2,2> render_review_preview(uint64_t);
    void cancel_review_preview(uint64_t);
    void release_review_preview(uint64_t);
    FrameV2 preview_advisory(const std::string& id,uint32_t width,uint32_t height);
    document::String advisory_content(const std::string& id,bool operations);
    void join();
    void cancel_advisory(const std::string&);
    void stop()noexcept;
    const document::String& identity()const{return identity_;}
};
}
#endif
