// Uses the existing document Parser/Writer; included only by document.cpp.
namespace pixaura::assistance {
using namespace document;
namespace {
template<class T,class F> Outcome<T> assistance_attempt(F&& function) {
    try { return {0,function()}; }
    catch(const Failure& error) { return {error.code,std::nullopt}; }
    catch(const std::bad_alloc&) { return {8,std::nullopt}; }
    catch(...) { return {14,std::nullopt}; }
}
void identifier(std::string_view value) {
    require(!value.empty() && value.size()<=identifier_limit,PIXAURA_INVALID_ARGUMENT);
    bool segment=false;
    for(char c:value) {
        if(c=='.'){require(segment,PIXAURA_INVALID_ARGUMENT);segment=false;}
        else {require((c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='_'||c=='-',PIXAURA_INVALID_ARGUMENT);segment=true;}
    }
    require(segment,PIXAURA_INVALID_ARGUMENT);
}
Shape payload_shape(Kind kind) {
    require(kind==Kind::Advice || kind==Kind::StructuredIntent || kind==Kind::PlanCandidate,PIXAURA_INVALID_ARGUMENT);
    return kind==Kind::PlanCandidate?Shape::AgentPlan:Shape::StructuredIntent;
}
}
Outcome<String> encode(const Request& request,std::string_view provider) {
    return assistance_attempt<String>([&]{
        require(request.version==1 && request.registry_version==1,PIXAURA_UNSUPPORTED_ABI);
        require(!request.pre_cancel,PIXAURA_DOCUMENT_CANCELLED);
        identifier(provider);require(provider==reference_id,PIXAURA_INVALID_ARGUMENT);identifier(request.proposal_id);
        require(!request.payload.empty() && request.payload.size()<=payload_limit && request.binding.size()<=512,PIXAURA_DOCUMENT_RESOURCE_LIMIT);
        const auto payload=Parser(request.payload,request.kind==Kind::PlanCandidate?8192:2048).parse(payload_shape(request.kind));
        require(Writer().write(payload)==request.payload);
        const auto binding=Parser(request.binding,512).parse(Shape::AgentBinding);
        require(Writer().write(binding)==request.binding);
        require(Writer().write(field(payload,"binding"))==request.binding,PIXAURA_DOCUMENT_STALE_BASE);
        auto result=Writer().write(Json(Json::Object{
            {"binding",binding},{"envelope_version",Json(int64_t{1})},
            {"kind",Json(static_cast<int64_t>(request.kind))},{"payload",payload},
            {"proposal_id",Json(request.proposal_id)},{"provider_id",Json(String(provider))},
            {"provider_version",Json(int64_t{1})},{"registry_version",Json(int64_t{1})}}));
        require(result.size()<=envelope_limit,PIXAURA_DOCUMENT_RESOURCE_LIMIT);return result;
    });
}
Outcome<Decoded> decode(std::string_view input,const ImageDocument& doc,
    std::string_view context,std::string_view selected_provider,uint32_t registry_version) {
    return assistance_attempt<Decoded>([&]{
        require(registry_version==1,PIXAURA_UNSUPPORTED_ABI);
        identifier(selected_provider);require(selected_provider==reference_id,PIXAURA_INVALID_ARGUMENT);
        require(!input.empty(),PIXAURA_INVALID_ARGUMENT);
        const auto value=Parser(input,envelope_limit).parse(Shape::Assistance);
        keys(value,{"binding","envelope_version","kind","payload","proposal_id","provider_id","provider_version","registry_version"});
        for(const char* version:{"envelope_version","provider_version","registry_version"})
            require(integer(field(value,version),0,UINT32_MAX)==1,PIXAURA_UNSUPPORTED_ABI);
        const auto& provider=text(field(value,"provider_id"));identifier(provider);
        require(provider==selected_provider,PIXAURA_INVALID_ARGUMENT);
        identifier(text(field(value,"proposal_id")));
        const auto kind=static_cast<Kind>(integer(field(value,"kind"),1,3,PIXAURA_INVALID_ARGUMENT));
        const auto binding=agent_binding_json(doc,context);
        check_agent_binding(field(value,"binding"),binding);
        const auto& payload=field(value,"payload");
        check_agent_binding(field(payload,"binding"),binding);
        auto bytes=Writer().write(payload);
        Decoded result;result.value.kind=kind;
        if(kind==Kind::PlanCandidate) {
            auto plan=canonical_agent_schema(bytes,2,doc,context,&result.operations);
            if(plan.code)fail(plan.code);
            result.value.plan=std::move(plan.value);
        } else {
            auto plan=plan_structured_intent(bytes,doc,context);
            if(plan.code)fail(plan.code);
            result.operations=std::move(plan.value.operations);
            // Advice validates the exact tool suggestion, but grants no plan output.
            if(kind==Kind::StructuredIntent)result.value.plan=std::move(plan.value.plan);
        }
        result.value.envelope=Writer().write(value);
        require(result.value.envelope==input); // canonical input only, no hidden repair
        return result;
    });
}
}
