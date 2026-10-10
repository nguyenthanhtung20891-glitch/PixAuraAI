#ifndef PIXAURA_PRESET_HPP
#define PIXAURA_PRESET_HPP
#include "manual_tools.hpp"
#include <set>
#include <algorithm>
namespace pixaura::preset {
using namespace document;
class Catalog {
    Vector<PresetRecipe> recipes_;
public:
    explicit Catalog(const Vector<PresetRecipe>& recipes) {
        if(recipes.size()>16)throw Failure{8};
        std::set<String> ids;std::size_t bytes=0;
        for(const auto& recipe:recipes){auto canonical=serialize_preset(recipe);if(canonical.code)throw Failure{canonical.code};
            for(const auto& op:recipe.operations)if(!manual::preset_operation(op))throw Failure{5};
            if(!ids.insert(recipe.preset_id).second)throw Failure{6};bytes+=canonical.value.size();if(bytes>262144)throw Failure{8};}
        recipes_=recipes;
        std::sort(recipes_.begin(),recipes_.end(),[](const PresetRecipe& a,const PresetRecipe& b){return a.preset_id<b.preset_id;});
    }
    Catalog(const Catalog&) = default;
    Catalog& operator=(const Catalog&) = delete;
    const Vector<PresetRecipe>& recipes() const { return recipes_; }
};
inline Result<Snapshot> propose(const Snapshot& base,const Snapshot& live,std::string_view recipe_text,std::string_view bindings_text,std::string_view revision_text) {
    try {
        if(!base||!live)throw Failure{1};
        if(base->identity().project!=live->identity().project||base->identity().document!=live->identity().document||base->source().sha256!=live->source().sha256||base->current()!=live->current()||base->session().id!=live->session().id||base->session().generation!=live->session().generation)throw Failure{9};
        auto parsed=parse_preset(recipe_text);if(parsed.code)throw Failure{parsed.code};
        auto bound=parse_preset_bindings(bindings_text);if(bound.code)throw Failure{bound.code};
        if(bound.value.size()!=parsed.value.operations.size())throw Failure{6};
        const auto revision=Id::parse(revision_text);for(const auto& old:base->revisions())if(old.id==revision)throw Failure{6};
        auto replayed=replay(*base,base->current());if(replayed.code)throw Failure{replayed.code};
        Vector<Id> stack;for(const auto& op:replayed.value)stack.push_back(op.id);
        bool changed=false;
        for(std::size_t i=0;i<parsed.value.operations.size();++i){auto& op=parsed.value.operations[i];
            if(!manual::preset_operation(op))throw Failure{5};
            for(const auto& old:base->operations())if(old.id==bound.value[i])throw Failure{6};
            op.id=bound.value[i];changed=changed||!manual::is_neutral(op);stack.push_back(op.id);}
        if(!changed)return {0,live};
        if(stack.size()>256)throw Failure{8};
        DetachedCandidate candidate{base->identity(),base->current(),base->session(),std::move(stack),std::move(parsed.value.operations),revision,"manual",std::nullopt};
        return transition(*live,candidate);
    }catch(const Failure& e){return {e.code,{}};}catch(const std::bad_alloc&){return {8,{}};}catch(...){return {14,{}};}
}
}
#endif
