// Read-only projection. Included after Auto/Preview for existing binding checks.
namespace {
static_assert(sizeof(pixaura_review_parameter)==112,"inspection parameter ABI");
static_assert(sizeof(pixaura_review_action)==544,"inspection action ABI");
static_assert(sizeof(pixaura_review_inspection)==4808,"inspection ABI");
template<std::size_t N> void inspection_text(char (&out)[N],std::string_view value){
    require(!value.empty()&&value.size()<N,14);std::memcpy(out,value.data(),value.size());
}
pixaura_review_inspection inspection_description(Service& s,const pixaura_auto_proposal& p,std::string_view plan){
    auto_current(s,p);require(plan==s.auto_plan,9);
    require(s.repository->read().epoch==s.loaded.epoch,9);
    require(good(document::canonical_agent_schema(plan,2,*s.loaded.snapshot,s.document_identity))==plan,6);
    // Existing native Registry validator; no second Plan parser or tool rules.
    const pixaura_agent_schema_request request{1,sizeof(request),2,1,0,{0,0,0}};uint64_t required=0;
    const auto status=pixaura_agent_tools_bind(1,1,&s.document,&s.live,&request,
        reinterpret_cast<const uint8_t*>(plan.data()),plan.size(),nullptr,0,&required);
    require(status==11,status?status:14);
    require(p.action_count>=1&&p.action_count<=8,7);
    // Reuse the frozen native action projection on the exact validated bytes,
    // and verify the cached proposal cannot hide, reorder or substitute actions.
    const assistance::Validated validated{String{},String(plan),static_cast<assistance::Kind>(p.kind)};
    const auto projection=assist_value(assist_experience::auto_presentation(validated,manual::registry().data(),manual::registry().size()));
    require(projection.action_count==p.action_count&&projection.registry_version==p.registry_version&&
        std::memcmp(projection.actions,p.actions,sizeof(p.actions))==0,14);
    pixaura_review_inspection result{};result.api_version=1;result.struct_size=sizeof(result);
    result.kind=PIXAURA_REVIEW_INSPECTION_KIND;result.action_count=p.action_count;
    result.registry_version=p.registry_version;result.repository_epoch=s.loaded.epoch;result.review=p;
    storage::Sha256 hash;hash.update(reinterpret_cast<const uint8_t*>(plan.data()),plan.size());
    const auto digest=hash.finish();require(digest.size()==64,14);std::memcpy(result.plan_sha256,digest.data(),64);
    const auto revision=s.loaded.snapshot->current().text();require(revision.size()==32,14);std::memcpy(result.source_revision,revision.data(),32);
    if(p.action_count>1)result.warnings|=PIXAURA_REVIEW_WARNING_MULTIPLE;
    for(uint32_t i=0;i<p.action_count;++i){const auto& a=projection.actions[i];require(a.tool_index<manual::registry().size(),7);
        const auto& d=manual::registry()[a.tool_index];require(a.parameter_count==d.parameter_count&&d.parameter_count<=4,7);
        auto& out=result.actions[i];out.index=i;out.tool_index=a.tool_index;out.operation_version=d.operation_version;
        out.parameter_version=d.parameter_version;out.parameter_count=a.parameter_count;
        inspection_text(out.tool_id,d.tool_id);inspection_text(out.label,d.name);
        out.geometry_changed=d.category=="geometry"?1u:0u;
        if(out.geometry_changed){out.warnings=PIXAURA_REVIEW_WARNING_GEOMETRY;result.warnings|=out.warnings;}
        for(uint32_t j=0;j<a.parameter_count;++j){const auto& parameter=d.parameters[j];auto& v=out.parameters[j];
            inspection_text(v.name,parameter.name);inspection_text(v.unit,parameter.unit);
            v.raw_value=a.values[j];v.minimum=parameter.minimum;v.maximum=parameter.maximum;v.neutral=parameter.neutral;
            require(v.raw_value>=v.minimum&&v.raw_value<=v.maximum,7);
        }
    }
    return result;
}
void inspection_current(Service& s,const pixaura_review_inspection& value){
    require(value.api_version==1,2);require(value.struct_size==sizeof(value)&&value.kind==PIXAURA_REVIEW_INSPECTION_KIND,3);
    const auto expected=inspection_description(s,value.review,s.auto_plan);
    require(std::memcmp(&value,&expected,sizeof(value))==0,3);
}
}
extern "C" int32_t pixaura_review_inspect(uint32_t version,pixaura_editor_context* c,
 const pixaura_auto_proposal* p,const uint8_t* input,uint64_t bytes,pixaura_review_inspection* out){return attempt([&]{
    require(version==1,2);require(bytes&&bytes<=8192,8);
    execution_ranges({{c,sizeof(*c),alignof(pixaura_editor_context)},{p,sizeof(*p),alignof(pixaura_auto_proposal)},
        {input,bytes},{out,sizeof(*out),alignof(pixaura_review_inspection)}});
    auto& s=owner(c);std::lock_guard<std::mutex> lock(s.gate);
    const auto result=inspection_description(s,*p,std::string_view(reinterpret_cast<const char*>(input),static_cast<std::size_t>(bytes)));*out=result;
});}
extern "C" int32_t pixaura_review_inspection_current(uint32_t version,pixaura_editor_context* c,const pixaura_review_inspection* value){return attempt([&]{
    require(version==1,2);execution_ranges({{c,sizeof(*c),alignof(pixaura_editor_context)},{value,sizeof(*value),alignof(pixaura_review_inspection)}});
    auto& s=owner(c);std::lock_guard<std::mutex> lock(s.gate);inspection_current(s,*value);
});}
extern "C" int32_t pixaura_review_inspection_preview(uint32_t version,pixaura_editor_context* c,
 const pixaura_review_inspection* value,const pixaura_review_preview_pair* pair){return attempt([&]{
    require(version==1,2);execution_ranges({{c,sizeof(*c),alignof(pixaura_editor_context)},{value,sizeof(*value),alignof(pixaura_review_inspection)},
        {pair,sizeof(*pair),alignof(pixaura_review_preview_pair)}});
    auto& s=owner(c);std::lock_guard<std::mutex> lock(s.gate);inspection_current(s,*value);review_pair(s,*pair);
    require(std::memcmp(&value->review,&s.review_preview.proposal,sizeof(value->review))==0,9);
});}
