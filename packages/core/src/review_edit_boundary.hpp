// Metadata-only edit intent construction. No raster, approval or publication.
namespace {
static_assert(sizeof(pixaura_review_edit_ticket)==136,"edit ticket ABI");
static_assert(sizeof(pixaura_review_edit_request)==776,"edit request ABI");
static_assert(sizeof(pixaura_review_edit_result)==480,"edit result ABI");
void edit_current(Service& s,const pixaura_review_edit_ticket& t){
    s.ready();auto& e=s.review_edit;
    require(t.api_version==1,2);require(t.struct_size==sizeof(t)&&t.kind==PIXAURA_REVIEW_EDIT_KIND&&!t.reserved,3);
    require(e.active&&std::memcmp(&t,&e.result.ticket,sizeof(t))==0,9);
    require(t.source_serial==s.live.serial&&t.repository_epoch==s.loaded.epoch&&
        s.repository->read().epoch==t.repository_epoch&&e.auto_sequence==s.auto_sequence,9);
    require(!s.selected&&!s.proposal.serial,9);
}
String edit_candidate(Service& s,const pixaura_review_inspection& source){
    // Construct from trusted native records, never parse provider/UI prose.
    String raw="{\"actions\":[";
    for(uint32_t i=0;i<source.action_count;++i){const auto& a=source.actions[i];if(i)raw+=',';
        raw+="{\"action_index\":";raw+=std::to_string(i);
        raw+=",\"operation_version\":";raw+=std::to_string(a.operation_version);
        raw+=",\"parameter_version\":";raw+=std::to_string(a.parameter_version);
        raw+=",\"parameters\":{";
        for(uint32_t p=0;p<a.parameter_count;++p){if(p)raw+=',';raw+='"';raw+=a.parameters[p].name;raw+="\":";raw+=std::to_string(a.parameters[p].raw_value);}
        raw+="},\"tool_id\":\"";raw+=a.tool_id;raw+="\"}";
    }
    raw+="],\"binding\":";auto binding=good(document::agent_schema_binding(*s.loaded.snapshot,s.document_identity));
    require(!binding.empty()&&binding.back()=='\n',14);binding.pop_back();raw+=binding;raw+=",\"plan_version\":1}\n";
    return raw;
}
}
extern "C" int32_t pixaura_review_edit_begin(uint32_t v,pixaura_editor_context* c,
 const pixaura_review_inspection* inspection,pixaura_review_edit_result* out){return attempt([&]{
    require(v==1,2);execution_ranges({{c,sizeof(*c),alignof(pixaura_editor_context)},
        {inspection,sizeof(*inspection),alignof(pixaura_review_inspection)},{out,sizeof(*out),alignof(pixaura_review_edit_result)}});
    auto& s=owner(c);std::unique_lock<std::mutex> lock(s.gate,std::try_to_lock);require(lock.owns_lock(),8);
    inspection_current(s,*inspection);auto& e=s.review_edit;
    require(!s.agent_active&&!s.auto_active&&!s.assist_active&&!s.review_preview.active,8);
    require(!e.active||e.auto_sequence!=s.auto_sequence,8);require(e.sequence!=UINT64_MAX,8);
    String original(s.auto_plan);pixaura_review_edit_result r{};r.api_version=1;r.struct_size=sizeof(r);r.state=PIXAURA_REVIEW_ORIGINAL;r.review=inspection->review;
    auto& t=r.ticket;t.api_version=1;t.struct_size=sizeof(t);t.kind=PIXAURA_REVIEW_EDIT_KIND;t.serial=e.sequence+1;
    t.source_serial=s.live.serial;t.repository_epoch=s.loaded.epoch;std::memcpy(t.context_id,s.seed.data(),32);std::memcpy(t.plan_sha256,inspection->plan_sha256,64);
    e.original.swap(original);e.result=r;e.sequence=t.serial;e.auto_sequence=s.auto_sequence;e.active=true;*out=r;
});}
extern "C" int32_t pixaura_review_edit_current(uint32_t v,pixaura_editor_context* c,
 const pixaura_review_edit_ticket* t,pixaura_review_edit_result* out){return attempt([&]{
    require(v==1,2);execution_ranges({{c,sizeof(*c),alignof(pixaura_editor_context)},
        {t,sizeof(*t),alignof(pixaura_review_edit_ticket)},{out,sizeof(*out),alignof(pixaura_review_edit_result)}});
    auto& s=owner(c);std::unique_lock<std::mutex> lock(s.gate,std::try_to_lock);require(lock.owns_lock(),8);edit_current(s,*t);*out=s.review_edit.result;
});}
extern "C" int32_t pixaura_review_edit(uint32_t v,pixaura_editor_context* c,
 const pixaura_review_edit_request* request,pixaura_review_edit_result* out){return attempt([&]{
    require(v==1,2);execution_ranges({{c,sizeof(*c),alignof(pixaura_editor_context)},
        {request,sizeof(*request),alignof(pixaura_review_edit_request)},{out,sizeof(*out),alignof(pixaura_review_edit_result)}});
    require(request->api_version==1,2);require(request->struct_size==sizeof(*request)&&!request->reserved,3);
    require(request->operation>=1&&request->operation<=4,5);
    auto& s=owner(c);std::unique_lock<std::mutex> lock(s.gate,std::try_to_lock);require(lock.owns_lock(),8);
    edit_current(s,request->ticket);auto& e=s.review_edit;
    require(!s.agent_active&&!s.assist_active&&!s.auto_active&&!s.review_preview.active,8);
    require(s.auto_sequence!=UINT64_MAX&&e.sequence!=UINT64_MAX,8);
    require(request->operation==PIXAURA_REVIEW_RESTORE_ORIGINAL||e.result.state<=PIXAURA_REVIEW_EDITED,9);
    pixaura_review_inspection description{};
    if(request->operation!=PIXAURA_REVIEW_RESTORE_ORIGINAL)description=inspection_description(s,e.result.review,s.auto_plan);
    const auto provenance=e.result.review;
    // Admission irreversibly retires old eligibility, including on allocation or
    // validation failure. Existing approval revocation state, no new authority.
    s.review_preview.invalidate(s.raster);s.auto_ready=false;s.auto_plan.clear();s.auto_cancelled=true;
    ++s.auto_sequence;e.auto_sequence=s.auto_sequence;e.result.state=PIXAURA_REVIEW_EDIT_FAILED;
    for(auto& entry:s.approvals)if(entry.second.state==0)entry.second.state=1;
    execution_checkpoint(s,42); // existing certification rendezvous; release no-op
    String candidate;
    if(request->operation==PIXAURA_REVIEW_RESTORE_ORIGINAL){
        require(!request->action_index&&!request->parameter_index&&!request->destination&&!request->raw_value,3);
        const char zero[32]{};require(std::memcmp(request->tool_id,zero,32)==0&&std::memcmp(request->parameter_name,zero,32)==0,3);
        const pixaura_review_action empty{};require(std::memcmp(&request->expected_action,&empty,sizeof(empty))==0,3);
        candidate=e.original;
    }else{
        require(request->action_index<description.action_count,7);auto& a=description.actions[request->action_index];
        require(std::memcmp(&request->expected_action,&a,sizeof(a))==0,3);
        require(std::memcmp(request->tool_id,a.tool_id,32)==0,3);
        if(request->operation==PIXAURA_REVIEW_CHANGE_VALUE){
            require(!request->destination&&request->parameter_index<a.parameter_count,7);auto& p=a.parameters[request->parameter_index];
            require(std::memcmp(request->parameter_name,p.name,32)==0,3);
            require(request->raw_value>=p.minimum&&request->raw_value<=p.maximum,7);p.raw_value=request->raw_value;
        }else{
            const char zero[32]{};require(!request->parameter_index&&!request->raw_value&&std::memcmp(request->parameter_name,zero,32)==0,3);
            if(request->operation==PIXAURA_REVIEW_REMOVE_ACTION){require(!request->destination,3);
                for(uint32_t i=request->action_index;i+1<description.action_count;++i)description.actions[i]=description.actions[i+1];
                --description.action_count;
            }else{require(request->destination<description.action_count,7);const auto moved=a;
                if(request->action_index<request->destination)for(uint32_t i=request->action_index;i<request->destination;++i)description.actions[i]=description.actions[i+1];
                else for(uint32_t i=request->action_index;i>request->destination;--i)description.actions[i]=description.actions[i-1];
                description.actions[request->destination]=moved;
            }
        }
        if(description.action_count)candidate=edit_candidate(s,description);
    }
    auto result=e.result;result.ticket.serial=e.sequence+1;
    if(candidate.empty()){
        require(s.repository->read().epoch==result.ticket.repository_epoch,9);
        result.state=PIXAURA_REVIEW_NO_ACTIONS;std::memset(result.ticket.plan_sha256,0,64);
        e.result=result;e.sequence=result.ticket.serial;*out=result;return;
    }
    // Existing complete parser, Registry guard and full-stack geometry/resource
    // validation; this helper creates no approval/candidate and renders nothing.
    candidate=good(document::canonical_agent_schema(candidate,2,*s.loaded.snapshot,s.document_identity));
    (void)execution_plan(s,byte_pointer(candidate),candidate.size());
    const assistance::Validated validated{String{},String(candidate),assistance::Kind::PlanCandidate};
    auto proposal=assist_value(assist_experience::auto_presentation(validated,manual::registry().data(),manual::registry().size()));
    proposal.request_serial=s.auto_sequence;proposal.source_serial=s.live.serial;
    std::memcpy(proposal.context_id,provenance.context_id,32);std::memcpy(proposal.provider_id,provenance.provider_id,sizeof(proposal.provider_id));
    const auto id=assemble({"review.edit.",std::to_string(result.ticket.serial)});require(id.size()<sizeof(proposal.proposal_id),8);std::memcpy(proposal.proposal_id,id.data(),id.size());
    const auto digest=execution_digest(byte_pointer(candidate),candidate.size());std::memcpy(result.ticket.plan_sha256,digest.data(),64);
    result.state=candidate==e.original?PIXAURA_REVIEW_ORIGINAL:PIXAURA_REVIEW_EDITED;result.review=proposal;
    require(s.repository->read().epoch==result.ticket.repository_epoch,9);
    // Non-allocating install after all construction/validation succeeds.
    s.auto_plan.swap(candidate);s.auto_proposal=proposal;s.auto_repository_epoch=s.loaded.epoch;s.auto_cancelled=false;s.auto_ready=true;
    e.result=result;e.sequence=result.ticket.serial;*out=result;
});}
