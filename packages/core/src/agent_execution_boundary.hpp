// Included by editor_api.cpp. The existing application service is the only
// live-session, persistence and publication owner. No alternate history owner.
namespace {
constexpr uint32_t approval_kind=0x41505631;
static_assert(sizeof(pixaura_agent_approval)==56,"approval ABI");
static_assert(sizeof(pixaura_agent_execution_request)==32,"execution request ABI");
static_assert(sizeof(pixaura_agent_execution_receipt)==144,"receipt ABI");
struct ExecutionRange{const void* p;uint64_t n;std::size_t alignment=1;};
void execution_ranges(std::initializer_list<ExecutionRange> ranges){
    for(const auto& a:ranges){
        const auto x=reinterpret_cast<uintptr_t>(a.p);
        require((a.p||!a.n)&&x%a.alignment==0&&a.n<=UINTPTR_MAX-x);
        for(const auto& b:ranges){if(&a==&b||!a.n||!b.n)continue;
            const auto y=reinterpret_cast<uintptr_t>(b.p);require(b.n<=UINTPTR_MAX-y);
            require(x>=y+b.n||y>=x+a.n);}
    }
}
void execution_request(uint32_t version,const pixaura_agent_execution_request* r){
    require(version==1&&r->api_version==1&&r->registry_version==1,2);
    require(r->struct_size==sizeof(*r)&&r->pre_cancel<=1);
    for(const auto v:r->reserved)require(v==0);
    require(!r->pre_cancel,13);
}
document::Vector<document::EditOperation> execution_plan(Service& s,const uint8_t* input,uint64_t bytes){
    require(input&&bytes&&bytes<=8192,8);
    require(!s.selected&&!s.proposal.serial,8);
    document::Vector<document::EditOperation> operations;
    const std::string_view view(reinterpret_cast<const char*>(input),static_cast<std::size_t>(bytes));
    const auto canonical=good(document::canonical_agent_schema(view,2,*s.loaded.snapshot,s.document_identity,&operations));
    require(canonical==view,6);
    // Use the same public Step 3 guard and its exact trusted native live handle.
    const pixaura_agent_schema_request request{1,sizeof(pixaura_agent_schema_request),2,1,0,{0,0,0}};
    uint64_t required=0;
    const auto code=pixaura_agent_tools_bind(1,1,&s.document,&s.live,&request,input,bytes,nullptr,0,&required);
    require(code==11,code?code:14);
    // Approval eligibility uses existing full-stack geometry/resource authority,
    // not an agent copy of policy or operation bounds. No raster work starts.
    auto stack=good(document::replay(*s.loaded.snapshot,s.loaded.snapshot->current()));
    uint32_t candidate_serial=1;
    for(auto op:operations){
        for(;;){
            require(candidate_serial<=264,8);String id(32,'0');const auto tail=std::to_string(candidate_serial++);
            id.replace(32-tail.size(),tail.size(),tail);bool used=false;
            for(const auto& prior:stack)if(prior.id.text()==id)used=true;
            if(!used){op.id=document::Id::parse(id);break;}
        }
        stack.push_back(op);
    }
    pixaura_working_metadata metadata{};check(pixaura_working_query(&s.raster,&s.original,&metadata));
    const auto reservation=evaluation::reservation(metadata.width,metadata.height,stack,s.limits);
    require(reservation>=metadata.image_bytes,8);
    pixaura_execution_strategy strategy{};check(pixaura_policy_query(1,&s.raster,&strategy));
    pixaura_decode_limits limits{};check(pixaura_decode_default_limits(1,&limits));
    pixaura_budget_request budget{};budget.version=1;budget.struct_size=sizeof(budget);
    budget.work_class=PIXAURA_BUDGET_COMMIT;budget.bytes_per_pixel=16;budget.raster_count=1;
    budget.requested_handles=2;budget.width=metadata.width;budget.height=metadata.height;
    budget.scratch_bytes=reservation-metadata.image_bytes;budget.context_limit_bytes=limits.context_bytes;
    pixaura_budget_result result{};uint64_t policy_epoch=0;
    check(pixaura_policy_admit(1,&s.raster,&budget,&result,&policy_epoch));
    require(result.decision==PIXAURA_BUDGET_ADMIT&&policy_epoch==strategy.epoch,8);
    return operations;
}
String execution_digest(const uint8_t* input,uint64_t bytes){
    storage::Sha256 hash;hash.update(input,static_cast<std::size_t>(bytes));return hash.finish();
}
Service::Approval& execution_approval(Service& s,const pixaura_agent_approval* a){
    require(a->api_version==1&&a->struct_size==sizeof(*a)&&a->kind==approval_kind&&a->reserved==0&&a->serial&&
        std::memcmp(a->context_id,s.seed.data(),32)==0,3);
    const auto found=s.approvals.find(a->serial);require(found!=s.approvals.end(),3);return found->second;
}
void execution_copy(const String& value,uint8_t* output,uint64_t capacity,uint64_t* required){
    if(!output||capacity<value.size()){*required=value.size();throw Failure{11};}
    std::memcpy(output,value.data(),value.size());*required=value.size();
}
void execution_checkpoint(Service& s,uint32_t point){
#ifdef PIXAURA_CERTIFICATION_TESTING
    if(s.agent_checkpoint)s.agent_checkpoint(point,s.agent_checkpoint_state);
#else
    (void)s;(void)point;
#endif
}
}
extern "C" int32_t pixaura_editor_agent_binding(uint32_t version,pixaura_editor_context* c,uint8_t* out,uint64_t capacity,uint64_t* required){return attempt([&]{
    require(version==1,2);
    execution_ranges({{c,sizeof(*c),alignof(pixaura_editor_context)},{out,capacity},{required,sizeof(*required),alignof(uint64_t)}});
    auto& s=owner(c);std::lock_guard<std::mutex> lock(s.gate);s.ready();
    execution_copy(good(document::agent_schema_binding(*s.loaded.snapshot,s.document_identity)),out,capacity,required);
});}
extern "C" int32_t pixaura_editor_agent_validate(uint32_t version,pixaura_editor_context* c,const pixaura_agent_execution_request* r,
    const uint8_t* input,uint64_t bytes,uint8_t* out,uint64_t capacity,uint64_t* required){return attempt([&]{
    execution_ranges({{c,sizeof(*c),alignof(pixaura_editor_context)},{r,sizeof(*r),alignof(pixaura_agent_execution_request)},
        {input,bytes},{out,capacity},{required,sizeof(*required),alignof(uint64_t)}});
    execution_request(version,r);auto& s=owner(c);std::lock_guard<std::mutex> lock(s.gate);s.ready();
    (void)execution_plan(s,input,bytes);
    execution_copy(String(reinterpret_cast<const char*>(input),static_cast<std::size_t>(bytes)),out,capacity,required);
});}
extern "C" int32_t pixaura_editor_agent_approve(uint32_t version,pixaura_editor_context* c,const pixaura_agent_execution_request* r,
    const uint8_t* input,uint64_t bytes,pixaura_agent_approval* out){return attempt([&]{
    execution_ranges({{c,sizeof(*c),alignof(pixaura_editor_context)},{r,sizeof(*r),alignof(pixaura_agent_execution_request)},
        {input,bytes},{out,sizeof(*out),alignof(pixaura_agent_approval)}});
    execution_request(version,r);auto& s=owner(c);std::lock_guard<std::mutex> lock(s.gate);s.ready();
    const auto operations=execution_plan(s,input,bytes);
    require(s.approvals.size()<8,8);
    Service::Approval value;value.canonical=String(reinterpret_cast<const char*>(input),static_cast<std::size_t>(bytes));
    value.digest=execution_digest(input,bytes);value.base_serial=s.live.serial;
    const auto envelope=good(document::serialize_evaluation(operations));
    check(pixaura_agent_stage(1,&s.document,&s.live,byte_pointer(envelope),envelope.size(),&value.lease));
    const auto lease=value.lease;
    try{s.approvals.emplace(lease.serial,std::move(value));}
    catch(...){(void)pixaura_agent_release(1,&s.document,&lease);throw;}
    pixaura_agent_approval result{};result.api_version=1;result.struct_size=sizeof(result);result.kind=approval_kind;
    std::memcpy(result.context_id,s.seed.data(),32);result.serial=lease.serial;*out=result;
});}
static int32_t execution_terminal(uint32_t version,pixaura_editor_context* c,const pixaura_agent_approval* a,bool release){return attempt([&]{
    require(version==1,2);execution_ranges({{c,sizeof(*c),alignof(pixaura_editor_context)},{a,sizeof(*a),alignof(pixaura_agent_approval)}});
    auto& s=owner(c);std::lock_guard<std::mutex> lock(s.gate);s.ready();auto& value=execution_approval(s,a);
    if(release){check(pixaura_agent_release(1,&s.document,&value.lease));s.approvals.erase(a->serial);}
    else if(value.state==0)value.state=1;
});}
extern "C" int32_t pixaura_editor_agent_revoke(uint32_t v,pixaura_editor_context* c,const pixaura_agent_approval* a){return execution_terminal(v,c,a,false);}
extern "C" int32_t pixaura_editor_agent_release(uint32_t v,pixaura_editor_context* c,const pixaura_agent_approval* a){return execution_terminal(v,c,a,true);}
extern "C" int32_t pixaura_editor_agent_cancel(uint32_t version,pixaura_editor_context* c){return attempt([&]{
    require(version==1,2);execution_ranges({{c,sizeof(*c),alignof(pixaura_editor_context)}});auto& s=owner(c);
    std::lock_guard<std::mutex> lock(s.agent_publication);require(s.agent_epoch!=UINT64_MAX,8);++s.agent_epoch;
    if(s.agent_cancel.serial)check(pixaura_cancel_signal(&s.raster,&s.agent_cancel));
});}
extern "C" int32_t pixaura_editor_agent_execute(uint32_t version,pixaura_editor_context* c,const pixaura_agent_execution_request* r,
    const pixaura_agent_approval* a,const uint8_t* input,uint64_t bytes,pixaura_agent_execution_receipt* out){return attempt([&]{
    execution_ranges({{c,sizeof(*c),alignof(pixaura_editor_context)},{r,sizeof(*r),alignof(pixaura_agent_execution_request)},
        {a,a?sizeof(*a):0,alignof(pixaura_agent_approval)},{input,bytes},{out,sizeof(*out),alignof(pixaura_agent_execution_receipt)}});
    execution_request(version,r);auto& s=owner(c);
    std::unique_lock<std::mutex> lock(s.gate,std::try_to_lock);require(lock.owns_lock(),8);s.ready();
    uint64_t epoch=0;
    {std::lock_guard<std::mutex> fence(s.agent_publication);require(s.agent_epoch!=UINT64_MAX,8);epoch=s.agent_epoch;}
    require(a!=nullptr,15);auto& approval=execution_approval(s,a);require(approval.state==0,15);
    require(approval.base_serial==s.live.serial,9);
    execution_checkpoint(s,4);
    auto operations=execution_plan(s,input,bytes);
    require(approval.canonical==std::string_view(reinterpret_cast<const char*>(input),static_cast<std::size_t>(bytes)),15);
    execution_checkpoint(s,0);
    // Linearized single-use reservation precedes all deterministic execution.
    {std::lock_guard<std::mutex> fence(s.agent_publication);require(s.agent_epoch==epoch,13);approval.state=2;}
    auto stack=good(document::replay(*s.loaded.snapshot,s.loaded.snapshot->current()));
    document::Vector<document::Id> ids;
    for(const auto& op:stack)ids.push_back(op.id);
    for(auto& op:operations){op.id=document::Id::parse(s.fresh());ids.push_back(op.id);stack.push_back(op);}
    const auto revision=document::Id::parse(s.fresh());
    document::DetachedCandidate candidate{s.loaded.snapshot->identity(),s.loaded.snapshot->current(),s.loaded.snapshot->session(),
        std::move(ids),std::move(operations),revision,"ai",document::Id::parse(approval.digest.substr(0,32))};
    const auto projection=good(document::serialize_evaluation(stack));
    pixaura_decode_handle evaluated{};
    struct Cleanup{Service& s;pixaura_decode_handle& evaluated;~Cleanup(){
        std::lock_guard<std::mutex> fence(s.agent_publication);
        if(s.agent_cancel.serial)(void)pixaura_cancel_release(&s.raster,&s.agent_cancel);
        s.agent_cancel={};if(evaluated.serial)(void)pixaura_decode_release(&s.raster,&evaluated);
    }}cleanup{s,evaluated};
    {std::lock_guard<std::mutex> fence(s.agent_publication);require(epoch==s.agent_epoch,13);
        check(pixaura_cancel_create(&s.raster,1,&s.agent_cancel));}
    execution_checkpoint(s,1);
    check(pixaura_working_evaluate_cancel(&s.raster,&s.original,1,byte_pointer(projection),projection.size(),&s.limits,&s.agent_cancel,&evaluated));
    execution_checkpoint(s,2);
    auto next=good(document::transition(*s.loaded.snapshot,candidate));
    auto command=good(document::serialize_commit(*s.loaded.snapshot,*next));
    pixaura_document_handle proposal{};
    check(pixaura_document_apply(1,&s.document,&s.live,byte_pointer(command),command.size(),&proposal,nullptr));
    struct ProposalCleanup{Service& s;pixaura_document_handle& h;~ProposalCleanup(){if(h.serial)(void)pixaura_document_release(&s.document,&h);}}proposal_cleanup{s,proposal};
    pixaura_agent_execution_receipt result{};result.api_version=1;result.struct_size=sizeof(result);result.approval_consumed=1;
    std::memcpy(result.plan_sha256,approval.digest.data(),64);
    std::memcpy(result.source_revision,s.loaded.snapshot->current().text().data(),32);
    std::memcpy(result.resulting_revision,revision.text().data(),32);
    execution_checkpoint(s,3);
    // Cancellation and durable publication share this lock: one authority wins.
    std::lock_guard<std::mutex> fence(s.agent_publication);require(epoch==s.agent_epoch,13);
    s.pending_command.swap(command);s.pending_snapshot=std::move(next);s.proposal=proposal;proposal={};
    try{s.accept();}catch(...){if(s.proposal.serial)(void)pixaura_document_release(&s.document,&s.proposal);
        s.proposal={};s.pending_snapshot.reset();s.pending_command.clear();throw;}
    *out=result;
});}
#ifdef PIXAURA_CERTIFICATION_TESTING
extern "C" PIXAURA_API int32_t pixaura_test_agent_execution_checkpoint(pixaura_editor_context* c,void (*callback)(uint32_t,void*),void* state){return attempt([&]{
    auto& s=owner(c);std::lock_guard<std::mutex> lock(s.gate);s.agent_checkpoint=callback;s.agent_checkpoint_state=state;
});}
#endif
