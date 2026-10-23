// Shared-native ADVICE-only adapter. No approval/execution/candidate API calls.
namespace {
static_assert(sizeof(pixaura_assist_request)==16,"assist request ABI");
static_assert(sizeof(pixaura_assist_advice)==88,"assist data ABI");
template<class T>T assist_value(assistance::Outcome<T> result){check(result.code);require(result.value.has_value(),14);return std::move(*result.value);}
pixaura_assist_advice assist_catalog(uint32_t index){
    const auto choice=assist_experience::choice(index);
    pixaura_assist_advice value{};value.api_version=1;value.struct_size=sizeof(value);
    value.choice_index=index;value.tool_index=choice.tool_index;value.parameter_count=choice.count;
    std::copy(choice.values.begin(),choice.values.end(),value.values);return value;
}
#ifdef PIXAURA_CERTIFICATION_TESTING
void assist_checkpoint(Service& s,uint32_t point){
    if(s.assist_test_point.load()!=point)return;
    s.assist_test_reached=1;
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(15);
    while(!s.assist_test_resume.load()){
        require(std::chrono::steady_clock::now()<deadline,14);std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
#endif
}
extern "C" int32_t pixaura_assist_choice(uint32_t version,uint32_t index,pixaura_assist_advice* out){return attempt([&]{
    require(version==1,2);execution_ranges({{out,sizeof(*out),alignof(pixaura_assist_advice)}});
    *out=assist_catalog(index);
});}
extern "C" int32_t pixaura_editor_assist_request(uint32_t version,pixaura_editor_context* context,
    const pixaura_assist_request* request,pixaura_assist_advice* out){return attempt([&]{
    require(version==1,2);
    execution_ranges({{context,sizeof(*context),alignof(pixaura_editor_context)},
        {request,sizeof(*request),alignof(pixaura_assist_request)},{out,sizeof(*out),alignof(pixaura_assist_advice)}});
    require(request->api_version==1,2);require(request->struct_size==sizeof(*request)&&request->reserved==0);
    const auto selected=assist_experience::choice(request->choice_index);
    auto& s=owner(context);document::Snapshot snapshot;String context_id;uint64_t serial=0,source=0,epoch=0;
#ifdef PIXAURA_CERTIFICATION_TESTING
    uint32_t fault=0;
#endif
    {
        std::lock_guard<std::mutex> lock(s.gate);s.ready();require(!s.assist_unavailable,13);
        require(!s.assist_active&&!s.selected&&!s.proposal.serial,8);require(s.assist_sequence!=UINT64_MAX,8);
        if(!s.assist_provider)s.assist_provider=std::make_unique<assistance::ReferenceProvider>();
        check(assistance::check_identity(s.assist_provider->identity()));
        snapshot=s.loaded.snapshot;context_id=s.document_identity;source=s.live.serial;epoch=s.loaded.epoch;
        serial=++s.assist_sequence;s.assist_ready=false;s.assist_active=true;
#ifdef PIXAURA_CERTIFICATION_TESTING
        fault=s.assist_test_fault;
#endif
    }
    struct Lease{Service& s;~Lease(){std::lock_guard<std::mutex> lock(s.gate);s.assist_active=false;}} lease{s};
#ifdef PIXAURA_CERTIFICATION_TESTING
    assist_checkpoint(s,0);
    if(fault==1)s.assist_provider->stop();
    if(fault==4)throw Failure{8};
    if(fault==5)throw Failure{14};
#endif
    auto data=assist_value(assist_experience::request(*snapshot,context_id,request->choice_index,manual::registry()[selected.tool_index],selected,serial));
#ifdef PIXAURA_CERTIFICATION_TESTING
    if(fault==3){const auto at=data.payload.find(structured::goals[request->choice_index].id);require(at!=String::npos,14);
        data.payload.replace(at,structured::goals[request->choice_index].id.size(),"adjust.unlisted");}
#endif
    auto envelope=assist_value(s.assist_provider->request(data));
#ifdef PIXAURA_CERTIFICATION_TESTING
    if(fault==2)envelope+='x';
#endif
    const auto validated=assist_value(assistance::validate(envelope,*snapshot,context_id,assistance::reference_id));
    const auto values=assist_value(assist_experience::presentation(validated,manual::registry().data(),manual::registry().size()));
    require(values.tool_index==selected.tool_index&&values.count==selected.count&&values.values==selected.values,6);
    auto result=assist_catalog(request->choice_index);result.request_serial=serial;result.source_serial=source;
    require(context_id.size()==32,14);std::memcpy(result.context_id,context_id.data(),32);
#ifdef PIXAURA_CERTIFICATION_TESTING
    assist_checkpoint(s,1);
#endif
    std::lock_guard<std::mutex> lock(s.gate);s.ready();require(!s.assist_unavailable,13);
    require(s.live.serial==source&&s.loaded.epoch==epoch&&!s.selected&&!s.proposal.serial,9);
    require(s.assist_sequence==serial,13);
    s.assist_advice=result;s.assist_repository_epoch=epoch;s.assist_ready=true;*out=result;
});}
extern "C" int32_t pixaura_editor_assist_current(uint32_t version,pixaura_editor_context* context,const pixaura_assist_advice* advice){return attempt([&]{
    require(version==1,2);execution_ranges({{context,sizeof(*context),alignof(pixaura_editor_context)},{advice,sizeof(*advice),alignof(pixaura_assist_advice)}});
    require(advice->api_version==1,2);require(advice->struct_size==sizeof(*advice)&&advice->reserved==0);
    auto& s=owner(context);std::lock_guard<std::mutex> lock(s.gate);s.ready();
    require(s.assist_ready&&s.assist_advice.request_serial!=0&&!s.selected&&!s.proposal.serial,9);
    require(std::memcmp(advice,&s.assist_advice,sizeof(*advice))==0,9);
    require(advice->source_serial==s.live.serial&&s.assist_repository_epoch==s.loaded.epoch,9);
});}
extern "C" int32_t pixaura_editor_assist_cancel(uint32_t version,pixaura_editor_context* context){return attempt([&]{
    require(version==1,2);execution_ranges({{context,sizeof(*context),alignof(pixaura_editor_context)}});
    auto& s=owner(context);std::lock_guard<std::mutex> lock(s.gate);s.ready();s.invalidate_advice();
});}
#ifdef PIXAURA_CERTIFICATION_TESTING
extern "C" PIXAURA_API int32_t pixaura_test_assist_control(pixaura_editor_context* context,uint32_t command,uint32_t value){
    try {
        auto& s=owner(context);
        if(command==1)return static_cast<int32_t>(s.assist_test_reached.load());
        if(command==2){s.assist_test_resume=1;return 0;}
        std::lock_guard<std::mutex> lock(s.gate);s.ready();require(!s.assist_active,8);
        if(command==0){require(value<=1||value==UINT32_MAX);s.assist_test_point=value;s.assist_test_reached=0;s.assist_test_resume=0;}
        else if(command==3){require(value<=5);s.assist_test_fault=value;}
        else if(command==4)s.assist_sequence=UINT64_MAX;
        else return 1;
        return 0;
    } catch(const Failure& e){return e.code;}catch(...){return 14;}
}
#endif
