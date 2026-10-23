// Included once after document.cpp's existing canonical Parser/Writer.
namespace pixaura::assist_experience {
using namespace document;
assistance::Outcome<assistance::Request> request(const ImageDocument& doc,std::string_view context,
    uint32_t index,const manual::Descriptor& tool,const Choice& selected,uint64_t serial) {
    try {
        require(index<structured::goals.size() && selected.count==tool.parameter_count && selected.count<=4);
        Json::Object parameters;
        for(std::size_t i=0;i<selected.count;++i)parameters.emplace(String(tool.parameters[i].name),Json(static_cast<int64_t>(selected.values[i])));
        const auto binding=agent_binding_json(doc,context);
        assistance::Request result;result.kind=assistance::Kind::Advice;
        result.proposal_id="assist.";result.proposal_id+=std::to_string(serial);
        result.binding=Writer().write(binding);
        result.payload=Writer().write(Json(Json::Object{{"binding",binding},
            {"goal_id",Json(String(structured::goals[index].id))},{"parameters",Json(std::move(parameters))},
            {"structured_intent_version",Json(int64_t{1})}}));
        return {0,std::move(result)};
    } catch(const Failure& error){return {error.code,std::nullopt};}
      catch(const std::bad_alloc&){return {8,std::nullopt};}catch(...){return {14,std::nullopt};}
}
assistance::Outcome<Choice> presentation(const assistance::Validated& validated,const manual::Descriptor* tools,std::size_t count) {
    try {
        require(validated.kind==assistance::Kind::Advice && validated.plan.empty());
        const auto envelope=Parser(validated.envelope,assistance::envelope_limit).parse(Shape::Assistance);
        const auto& payload=field(envelope,"payload");const auto& goal=text(field(payload,"goal_id"));
        std::string_view id;
        for(const auto& entry:structured::goals)if(entry.id==goal)id=entry.tool;
        require(!id.empty(),5);
        const auto& parameters=object(field(payload,"parameters"));
        for(std::size_t index=0;index<count;++index)if(tools[index].tool_id==id){
            const auto& tool=tools[index];require(tool.parameter_count<=4 && parameters.size()==tool.parameter_count,7);
            Choice result;result.tool_index=static_cast<uint32_t>(index);result.count=static_cast<uint32_t>(tool.parameter_count);
            for(std::size_t i=0;i<tool.parameter_count;++i)
                result.values[i]=static_cast<int32_t>(integer(field(field(payload,"parameters"),tool.parameters[i].name.data()),INT32_MIN,INT32_MAX,7));
            return {0,result};
        }
        fail(5);
    } catch(const Failure& error){return {error.code,std::nullopt};}
      catch(const std::bad_alloc&){return {8,std::nullopt};}catch(...){return {14,std::nullopt};}
}
}
