// Proposal-only delegates. Never call approval, candidate, editing or history APIs.
namespace {
static_assert(sizeof(pixaura_auto_request)==16,"auto request ABI");
static_assert(sizeof(pixaura_auto_action)==24,"auto action ABI");
static_assert(sizeof(pixaura_auto_proposal)==328,"auto proposal ABI");
void auto_current(Service& s,const pixaura_auto_proposal& value){
    require(value.api_version==1,2);require(value.struct_size==sizeof(value)&&value.reserved==0);
    s.ready();require(s.auto_ready&&!s.selected&&!s.proposal.serial,9);
    require(std::memcmp(&value,&s.auto_proposal,sizeof(value))==0,9);
    require(value.source_serial==s.live.serial&&s.auto_repository_epoch==s.loaded.epoch,9);
}
#ifdef PIXAURA_CERTIFICATION_TESTING
void auto_checkpoint(Service& s,uint32_t point){
    if(s.auto_test_point.load()!=point)return;
    s.auto_test_reached=1;const auto until=std::chrono::steady_clock::now()+std::chrono::seconds(15);
    while(!s.auto_test_resume.load()){require(std::chrono::steady_clock::now()<until,14);std::this_thread::sleep_for(std::chrono::milliseconds(1));}
}
#endif
}
extern "C" int32_t pixaura_editor_auto_request(uint32_t version,pixaura_editor_context* context,
 const pixaura_auto_request* request,const uint8_t* input,uint64_t bytes,pixaura_auto_proposal* out){return attempt([&]{
    require(version==1,2);require(bytes<=8192,8);
    execution_ranges({{context,sizeof(*context),alignof(pixaura_editor_context)},{request,sizeof(*request),alignof(pixaura_auto_request)},
        {input,bytes},{out,sizeof(*out),alignof(pixaura_auto_proposal)}});
    require(request->api_version==1,2);require(request->struct_size==sizeof(*request));
    require(request->kind==2||request->kind==3,1);
    require(request->kind==2?(!input&&!bytes):(input&&bytes&&request->choice_index==0),1);
    const auto index=request->kind==2?request->choice_index:0;
    const auto choice=assist_experience::choice(index);
    auto& s=owner(context);document::Snapshot snapshot;String context_id;uint64_t serial=0,source=0,epoch=0;
#ifdef PIXAURA_CERTIFICATION_TESTING
    uint32_t fault=0;
#endif
    {std::lock_guard<std::mutex> lock(s.gate);s.ready();require(!s.assist_unavailable,13);
     require(!s.auto_active&&!s.assist_active&&!s.selected&&!s.proposal.serial,8);require(s.auto_sequence!=UINT64_MAX,8);
     if(!s.assist_provider)s.assist_provider=std::make_unique<assistance::ReferenceProvider>();
     check(assistance::check_identity(s.assist_provider->identity()));
     snapshot=s.loaded.snapshot;context_id=s.document_identity;source=s.live.serial;epoch=s.loaded.epoch;
     serial=++s.auto_sequence;s.auto_cancelled=false;s.auto_ready=false;s.auto_plan.clear();s.auto_active=true;
#ifdef PIXAURA_CERTIFICATION_TESTING
     fault=s.auto_test_fault;
#endif
    }
    struct Lease{Service& s;~Lease(){std::lock_guard<std::mutex> lock(s.gate);s.auto_active=false;}} lease{s};
#ifdef PIXAURA_CERTIFICATION_TESTING
    auto_checkpoint(s,0);
    if(fault==1)s.assist_provider->stop();
    if(fault==4)throw Failure{8};
    if(fault==5)throw Failure{14};
#endif
    auto data=assist_value(assist_experience::request(*snapshot,context_id,index,manual::registry()[choice.tool_index],choice,serial));
    data.kind=static_cast<assistance::Kind>(request->kind);data.proposal_id="auto.";data.proposal_id+=std::to_string(serial);
    if(request->kind==3)data.payload.assign(reinterpret_cast<const char*>(input),static_cast<std::size_t>(bytes));
#ifdef PIXAURA_CERTIFICATION_TESTING
    if(fault==3){const auto at=data.payload.find("adjust.");if(at!=String::npos)data.payload.replace(at,6,"unknown");else data.payload="{}\n";}
    if(fault==6)data.registry_version=2;
    if(fault==7)data.payload="{}\n";
    if(fault==8)data.payload.assign(8193,'x');
    if(fault==11)data.version=2;
    if(fault==12){constexpr std::string_view marker="\"structured_intent_version\":1";const auto at=data.payload.find(marker);if(at!=String::npos)data.payload[at+marker.size()-1]='2';}
#endif
    auto envelope=assist_value(s.assist_provider->request(data));
#ifdef PIXAURA_CERTIFICATION_TESTING
    if(fault==2)envelope+='x';
    if(fault==9)envelope.assign(12289,'x');
    if(fault==10){const auto at=envelope.find(assistance::reference_id);require(at!=String::npos,14);envelope.replace(at,assistance::reference_id.size(),"unknown.provider.v1");}
    auto_checkpoint(s,1); // untrusted proposal received, before validation
#endif
    const auto validated=assist_value(assistance::validate(envelope,*snapshot,context_id,assistance::reference_id));
    auto result=assist_value(assist_experience::auto_presentation(validated,manual::registry().data(),manual::registry().size()));
    require(!validated.plan.empty()&&validated.plan.size()<=8192,8);String plan=validated.plan;
    result.request_serial=serial;result.source_serial=source;require(context_id.size()==32,14);std::memcpy(result.context_id,context_id.data(),32);
    require(data.proposal_id.size()<sizeof(result.proposal_id)&&assistance::reference_id.size()<sizeof(result.provider_id),14);
    std::memcpy(result.proposal_id,data.proposal_id.data(),data.proposal_id.size());
    std::memcpy(result.provider_id,assistance::reference_id.data(),assistance::reference_id.size());
#ifdef PIXAURA_CERTIFICATION_TESTING
    auto_checkpoint(s,2); // validated, before review-ready publication
#endif
    std::lock_guard<std::mutex> lock(s.gate);s.ready();require(!s.assist_unavailable,13);
    require(s.live.serial==source&&s.loaded.epoch==epoch&&!s.selected&&!s.proposal.serial,9);
    require(!s.auto_cancelled&&s.auto_sequence==serial,13);s.auto_plan=std::move(plan);s.auto_proposal=result;s.auto_repository_epoch=epoch;s.auto_ready=true;*out=result;
});}
extern "C" int32_t pixaura_editor_auto_current(uint32_t version,pixaura_editor_context* context,const pixaura_auto_proposal* value){return attempt([&]{
    require(version==1,2);execution_ranges({{context,sizeof(*context),alignof(pixaura_editor_context)},{value,sizeof(*value),alignof(pixaura_auto_proposal)}});
    auto& s=owner(context);std::lock_guard<std::mutex> lock(s.gate);auto_current(s,*value);
});}
extern "C" int32_t pixaura_editor_auto_plan(uint32_t version,pixaura_editor_context* context,const pixaura_auto_proposal* value,
 uint8_t* output,uint64_t capacity,uint64_t* required){return attempt([&]{
    require(version==1,2);require(capacity<=8192,8);
    execution_ranges({{context,sizeof(*context),alignof(pixaura_editor_context)},{value,sizeof(*value),alignof(pixaura_auto_proposal)},
        {output,capacity},{required,sizeof(*required),alignof(uint64_t)}});
    auto& s=owner(context);std::lock_guard<std::mutex> lock(s.gate);auto_current(s,*value);
    *required=s.auto_plan.size();require(capacity>=*required,11);std::memcpy(output,s.auto_plan.data(),s.auto_plan.size());
});}
extern "C" int32_t pixaura_editor_auto_cancel(uint32_t version,pixaura_editor_context* context){return attempt([&]{
    require(version==1,2);execution_ranges({{context,sizeof(*context),alignof(pixaura_editor_context)}});
    auto& s=owner(context);std::lock_guard<std::mutex> lock(s.gate);s.ready();s.invalidate_auto();
});}
#ifdef PIXAURA_CERTIFICATION_TESTING
extern "C" PIXAURA_API int32_t pixaura_test_auto_control(pixaura_editor_context* context,uint32_t command,uint32_t value){
    try{auto& s=owner(context);
        if(command==1)return static_cast<int32_t>(s.auto_test_reached.load());
        if(command==2){s.auto_test_resume=1;return 0;}
        std::lock_guard<std::mutex> lock(s.gate);s.ready();require(!s.auto_active,8);
        if(command==0){require(value<=2||value==UINT32_MAX);s.auto_test_point=value;s.auto_test_reached=0;s.auto_test_resume=0;}
        else if(command==3){require(value<=12);s.auto_test_fault=value;}
        else if(command==4){require(value<=1);s.auto_sequence=UINT64_MAX-value;}
        else if(command==5)return static_cast<int32_t>(s.approvals.size());
        else return 1;
        return 0;
    }catch(const Failure& e){return e.code;}catch(...){return 14;}
}
#endif
