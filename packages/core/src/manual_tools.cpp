#include "manual_tools.hpp"
#include "tone.hpp"
#include "detail.hpp"
#include "pixaura/preset.h"
#include <algorithm>
#include <cstring>
#include <new>
#include <charconv>

namespace pixaura::manual {
using namespace document;
namespace {
constexpr Parameter crop[] = {{"height_ppm", "millionths_current_extent", 1, 1000000, 1000000, 1},
    {"width_ppm", "millionths_current_extent", 1, 1000000, 1000000, 1},
    {"x_ppm", "millionths_current_extent", 0, 999999, 0, 1},
    {"y_ppm", "millionths_current_extent", 0, 999999, 0, 1}};
constexpr Parameter exposure[] = {{"milli_ev", "thousandths_exposure_stop", -5000, 5000, 0, 1}};
constexpr Parameter rotate[] = {{"quarter_turns", "clockwise_quarter_turns", 0, 3, 0, 1}};
constexpr Parameter blur[]={{"milli_strength","thousandths_fixed_kernel_blend",0,1000,0,1}};
constexpr Parameter sharpen[]={{"milli_amount","thousandths_fixed_kernel_unsharp",0,1000,0,1}};
constexpr Parameter brightness[]={{"milli_linear","thousandths_linear_white",-1000,1000,0,1}};
constexpr Parameter contrast[]={{"milli_stops","thousandths_log2_contrast_slope",-2000,2000,0,1}};
constexpr Parameter highlights[]={{"milli_ev","thousandths_highlight_exposure_stop",-2000,2000,0,1}};
constexpr Parameter saturation[]={{"milli_ratio","thousandths_chroma_multiplier",0,2000,1000,1}};
constexpr Parameter shadows[]={{"milli_ev","thousandths_shadow_exposure_stop",-2000,2000,0,1}};
constexpr Parameter temperature[]={{"kelvin","daylight_white_kelvin",4000,25000,6504,1}};
constexpr std::array<Descriptor, 11> descriptors{{
    {"pixaura.blur","pixaura.blur","detail","Blur",1,1,blur,1},
    {"pixaura.brightness","pixaura.brightness","tone_color","Brightness",1,1,brightness,1},
    {"pixaura.contrast","pixaura.contrast","tone_color","Contrast",1,1,contrast,1},
    {"pixaura.crop", "pixaura.crop", "geometry", "Crop", 1, 1, crop, 4},
    {"pixaura.exposure", "pixaura.exposure", "tone_color", "Exposure", 1, 1, exposure, 1},
    {"pixaura.highlights","pixaura.highlights","tone_color","Highlights",1,1,highlights,1},
    {"pixaura.rotate", "pixaura.rotate", "geometry", "Rotate", 1, 1, rotate, 1},
    {"pixaura.saturation","pixaura.saturation","tone_color","Saturation",1,1,saturation,1},
    {"pixaura.shadows","pixaura.shadows","tone_color","Shadows",1,1,shadows,1},
    {"pixaura.sharpen","pixaura.sharpen","detail","Sharpen",1,1,sharpen,1},
    {"pixaura.temperature","pixaura.temperature","tone_color","Temperature",1,1,temperature,1}}};
template<class T, class F> Result<T> attempt(F&& f) {
    try { return {0, f()}; }
    catch (const Failure& e) { return {e.code, {}, e.index}; }
    catch (const std::bad_alloc&) { return {PIXAURA_DOCUMENT_RESOURCE_LIMIT, {}}; }
    catch (...) { return {PIXAURA_DOCUMENT_INTERNAL_ERROR, {}}; }
}
void require(bool condition, int32_t code) { if (!condition) throw Failure{code}; }
bool text(std::string_view value, bool spaces = false) {
    return !value.empty() && value.size() <= 64 && std::all_of(value.begin(), value.end(), [spaces](char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '.' || c == '_' || (spaces && c == ' ');
    });
}
void number(String& output, int64_t value) {
    char buffer[32];
    const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value);
    require(result.ec == std::errc{}, PIXAURA_DOCUMENT_INTERNAL_ERROR);
    output.append(buffer, static_cast<std::size_t>(result.ptr - buffer));
}
String parameters(const EditOperation& op) {
    String output;
    if (const auto* e = std::get_if<Exposure>(&op.parameters)) {
        output += "{\"milli_ev\":"; number(output, e->milli_ev);
    } else if (const auto* c = std::get_if<Crop>(&op.parameters)) {
        output += "{\"height_ppm\":"; number(output, c->height_ppm);
        output += ",\"width_ppm\":"; number(output, c->width_ppm);
        output += ",\"x_ppm\":"; number(output, c->x_ppm);
        output += ",\"y_ppm\":"; number(output, c->y_ppm);
    } else if(const auto* value=std::get_if<Tone>(&op.parameters)) {
        const auto* spec=tone::find(op.type);require(spec!=nullptr,PIXAURA_DOCUMENT_UNSUPPORTED_OPERATION);
        output+="{\"";output+=spec->parameter;output+="\":";number(output,value->value);
    } else if(const auto* detail_value=std::get_if<Detail>(&op.parameters)){const auto* spec=detail::find(op.type);require(spec!=nullptr,PIXAURA_DOCUMENT_UNSUPPORTED_OPERATION);output+="{\"";output+=spec->parameter;output+="\":";number(output,detail_value->value);
    } else {
        output += "{\"quarter_turns\":"; number(output, std::get<Rotate>(op.parameters).quarter_turns);
    }
    output += '}'; return output;
}
bool neutral(const EditOperation& op) {
    if (const auto* e = std::get_if<Exposure>(&op.parameters)) return e->milli_ev == 0;
    if (const auto* c = std::get_if<Crop>(&op.parameters)) return c->x_ppm == 0 && c->y_ppm == 0 && c->width_ppm == 1000000 && c->height_ppm == 1000000;
    if(const auto* value=std::get_if<Tone>(&op.parameters)){const auto* spec=tone::find(op.type);return spec&&value->value==spec->neutral;}
    if(const auto* value=std::get_if<Detail>(&op.parameters))return value->value==0;
    return std::get<Rotate>(op.parameters).quarter_turns == 0;
}
String registry_json() {
    require(validate_descriptors(descriptors.data(), descriptors.size()) == 0, PIXAURA_DOCUMENT_INTERNAL_ERROR);
    String output = "{\"descriptor_version\":1,\"tools\":[";
    bool first = true;
    for (const auto& d : descriptors) {
        if (!first) output += ',';
        first = false;
        output += "{\"capability\":\"offline_cpu\",\"category\":\""; output += d.category;
        output += "\",\"commit\":\"one_immutable_revision_per_changed_gesture\",\"invalid\":\"reject\",\"name\":\""; output += d.name;
        output += "\",\"operation_version\":"; number(output, d.operation_version);
        output += ",\"parameter_version\":"; number(output, d.parameter_version);
        output += ",\"parameters\":[";
        for (std::size_t i = 0; i < d.parameter_count; ++i) {
            if (i) output += ',';
            const auto& p = d.parameters[i];
            output += "{\"default\":"; number(output, p.neutral);
            output += ",\"maximum\":"; number(output, p.maximum);
            output += ",\"minimum\":"; number(output, p.minimum);
            output += ",\"name\":\""; output += p.name;
            output += "\",\"step\":"; number(output, p.step);
            output += ",\"type\":\"integer\",\"unit\":\""; output += p.unit; output += "\"}";
        }
        output += "],\"preview\":\"detached_latest_generation\",\"replay\":\"ordered_phase2_versions\",\"serialization\":\"schema1_canonical_integer_json\",\"tool_id\":\"";
        output += d.tool_id; output += "\",\"type\":\""; output += d.type; output += "\"}";
    }
    output += "]}\n";
    require(output.size() <= PIXAURA_MANUAL_MAX_REGISTRY_BYTES, PIXAURA_DOCUMENT_RESOURCE_LIMIT);
    return output;
}
}
bool is_neutral(const EditOperation& op) { return neutral(op); }
bool preset_operation(const EditOperation& op) { const auto* descriptor=find(op.type,op.operation_version,op.parameter_version); return descriptor&&(descriptor->category=="tone_color"||descriptor->category=="detail"); }
const std::array<Descriptor, 11>& registry() { return descriptors; }
int32_t validate_descriptors(const Descriptor* table, std::size_t count) {
    if (!table || count == 0 || count > PIXAURA_MANUAL_MAX_DESCRIPTORS) return PIXAURA_DOCUMENT_RESOURCE_LIMIT;
    for (std::size_t i = 0; i < count; ++i) {
        const auto& d = table[i];
        if (!text(d.tool_id) || !text(d.type) || !text(d.name, true) ||
            (d.category != "geometry" && d.category != "tone_color" && d.category != "detail" && d.category != "filters_presets") ||
            d.operation_version == 0 || d.parameter_version == 0 || !d.parameters ||
            d.parameter_count == 0 || d.parameter_count > PIXAURA_MANUAL_MAX_PARAMETERS) return PIXAURA_DOCUMENT_INVALID_PARAMETERS;
        if (i && !(table[i - 1].tool_id < d.tool_id)) return PIXAURA_INVALID_ARGUMENT;
        for (std::size_t j = 0; j < i; ++j) {
            if (table[j].type == d.type && table[j].operation_version == d.operation_version &&
                table[j].parameter_version == d.parameter_version) return PIXAURA_INVALID_ARGUMENT;
        }
        for (std::size_t j = 0; j < d.parameter_count; ++j) {
            const auto& p = d.parameters[j];
            if (!text(p.name) || !text(p.unit) || p.minimum > p.maximum || p.neutral < p.minimum ||
                p.neutral > p.maximum || p.step != 1 || (j && !(d.parameters[j - 1].name < p.name))) return PIXAURA_DOCUMENT_INVALID_PARAMETERS;
        }
    }
    return 0;
}
const Descriptor* find(std::string_view tool, uint32_t ov, uint32_t pv) {
    for (const auto& d : descriptors) if (d.tool_id == tool && d.operation_version == ov && d.parameter_version == pv) return &d;
    return nullptr;
}
String canonical_operation(const EditOperation& op) {
    String output = "{\"id\":\""; output += op.id.text();
    output += "\",\"operation_version\":"; number(output, op.operation_version);
    output += ",\"parameter_version\":"; number(output, op.parameter_version);
    output += ",\"parameters\":"; output += parameters(op);
    output += ",\"type\":\""; output += op.type; output += "\"}";
    return output;
}
EditOperation single_operation(std::string_view request) {
    const auto parsed = parse_evaluation(request);
    if (parsed.code) throw Failure{parsed.code, parsed.index};
    require(parsed.value.size() == 1, PIXAURA_DOCUMENT_INVALID_PARAMETERS);
    const auto& op = parsed.value[0];
    require(find(op.type, op.operation_version, op.parameter_version) != nullptr, PIXAURA_DOCUMENT_UNSUPPORTED_OPERATION);
    return op;
}
Gesture::Gesture(Snapshot base, Vector<EditOperation> stack, Id gesture, const Descriptor* descriptor,
    std::optional<std::size_t> replacement) : base_(std::move(base)), stack_(std::move(stack)),
    gesture_id_(std::move(gesture)), descriptor_(descriptor), replacement_(replacement) {}
Result<std::unique_ptr<Gesture>> Gesture::begin(Snapshot base, std::string_view gesture, std::string_view tool,
    std::optional<std::string_view> replace) {
    return attempt<std::unique_ptr<Gesture>>([&] {
        require(base != nullptr, PIXAURA_INVALID_ARGUMENT);
        const auto* descriptor = find(tool);
        require(descriptor != nullptr, PIXAURA_DOCUMENT_UNSUPPORTED_OPERATION);
        auto replayed = replay(*base, base->current());
        require(replayed.code == 0, replayed.code);
        std::optional<std::size_t> target;
        if (replace) {
            const auto id = Id::parse(*replace);
            for (std::size_t i = 0; i < replayed.value.size(); ++i) if (replayed.value[i].id == id) target = i;
            require(target.has_value() && replayed.value[*target].type == descriptor->type, PIXAURA_DOCUMENT_INVALID_PARAMETERS);
        }
        return std::unique_ptr<Gesture>(new Gesture(base, std::move(replayed.value), Id::parse(gesture), descriptor, target));
    });
}
bool Gesture::current(const Snapshot& live) const {
    return live && live->identity().project == base_->identity().project && live->identity().document == base_->identity().document &&
        live->source().sha256 == base_->source().sha256 && live->current() == base_->current() &&
        live->session().id == base_->session().id && live->session().generation == base_->session().generation;
}
Result<uint64_t> Gesture::update(std::string_view request) {
    return attempt<uint64_t>([&] {
        require(!closed_, PIXAURA_DOCUMENT_CANCELLED);
        require(sequence_ != UINT64_MAX, PIXAURA_DOCUMENT_RESOURCE_LIMIT);
        auto op = single_operation(request);
        require(op.type == descriptor_->type, PIXAURA_DOCUMENT_INVALID_PARAMETERS);
        require(replacement_.has_value() || neutral(op) || stack_.size() < 256, PIXAURA_DOCUMENT_RESOURCE_LIMIT);
        for (const auto& old : base_->operations()) require(old.id != op.id, PIXAURA_DOCUMENT_INVALID_PROJECT);
        auto next = std::make_shared<const EditOperation>(std::move(op));
        pending_ = std::move(next);
        return ++sequence_;
    });
}
bool Gesture::eligible(std::string_view gesture, uint64_t sequence, const Snapshot& live) const {
    return !closed_ && pending_ && current(live) && gesture == gesture_id_.text() && sequence != 0 && sequence == sequence_;
}
Result<String> Gesture::preview(const Snapshot& live) const {
    return attempt<String>([&] {
        require(!closed_ && current(live), PIXAURA_DOCUMENT_CANCELLED);
        String output = "{\"operations\":[";
        bool first = true;
        for (std::size_t i = 0; i <= stack_.size(); ++i) {
            const EditOperation* op = i < stack_.size() ? &stack_[i] : nullptr;
            if (pending_ && ((replacement_ && i == *replacement_) || (!replacement_ && i == stack_.size()))) {
                op = neutral(*pending_) ? nullptr : pending_.get();
            }
            if (!op) continue;
            if (!first) output += ',';
            first = false;
            output += canonical_operation(*op);
        }
        output += "]}\n";
        auto checked = parse_evaluation(output);
        require(checked.code == 0, checked.code);
        return output;
    });
}
Result<Snapshot> Gesture::commit(const Snapshot& live, std::string_view revision) {
    return attempt<Snapshot>([&] {
        require(!closed_, PIXAURA_DOCUMENT_CANCELLED);
        if (!current(live)) { cancel(); throw Failure{PIXAURA_DOCUMENT_STALE_BASE}; }
        const auto new_revision = Id::parse(revision);
        for (const auto& old : base_->revisions()) require(old.id != new_revision, PIXAURA_DOCUMENT_INVALID_PROJECT);
        if (!pending_ || (!replacement_ && neutral(*pending_)) ||
            (replacement_ && parameters(*pending_) == parameters(stack_[*replacement_]))) {
            closed_ = true;
            pending_.reset();
            return live;
        }
        Vector<Id> stack;
        Vector<EditOperation> operations;
        for (std::size_t i = 0; i < stack_.size(); ++i) {
            if (replacement_ && i == *replacement_) {
                if (!neutral(*pending_)) stack.push_back(pending_->id);
            } else stack.push_back(stack_[i].id);
        }
        if (!neutral(*pending_)) {
            if (!replacement_) stack.push_back(pending_->id);
            operations.push_back(*pending_);
        }
        DetachedCandidate candidate{live->identity(), base_->current(), base_->session(), std::move(stack),
            std::move(operations), new_revision, "manual", std::nullopt};
        auto next = transition(*live, candidate);
        require(next.code == 0, next.code);
        closed_ = true;
        pending_.reset();
        return next.value;
    });
}
void Gesture::cancel() { closed_ = true; pending_.reset(); }
}

namespace {
int32_t copy_json(const pixaura::document::String& json, uint8_t* output, uint64_t capacity, uint64_t* required) {
    if (capacity < json.size() || !output) { *required = json.size(); return PIXAURA_DOCUMENT_BUFFER_TOO_SMALL; }
    std::memcpy(output, json.data(), json.size());
    *required = json.size();
    return 0;
}
template<class F> int32_t boundary(uint32_t version, uint8_t* output, uint64_t capacity, uint64_t* required, F&& work) {
    if (version != PIXAURA_MANUAL_API_VERSION) return PIXAURA_UNSUPPORTED_ABI;
    if (!required || (!output && capacity != 0)) return PIXAURA_INVALID_ARGUMENT;
    try { return copy_json(work(), output, capacity, required); }
    catch (const pixaura::document::Failure& e) { return e.code; }
    catch (const std::bad_alloc&) { return PIXAURA_DOCUMENT_RESOURCE_LIMIT; }
    catch (...) { return PIXAURA_DOCUMENT_INTERNAL_ERROR; }
}
}
extern "C" int32_t pixaura_manual_registry_json(uint32_t version, uint8_t* output, uint64_t capacity, uint64_t* required) {
    return boundary(version, output, capacity, required, [] { return pixaura::manual::registry_json(); });
}
extern "C" int32_t pixaura_manual_canonical_operation(uint32_t version, const uint8_t* request, uint64_t bytes,
    uint8_t* output, uint64_t capacity, uint64_t* required) {
    return boundary(version, output, capacity, required, [&] {
        if (!request || bytes == 0) throw pixaura::document::Failure{PIXAURA_INVALID_ARGUMENT};
        if (bytes > pixaura::document::command_limit) throw pixaura::document::Failure{PIXAURA_DOCUMENT_RESOURCE_LIMIT};
        const auto op = pixaura::manual::single_operation(std::string_view(reinterpret_cast<const char*>(request), static_cast<std::size_t>(bytes)));
        pixaura::document::String output = "{\"operations\":[";
        output += pixaura::manual::canonical_operation(op); output += "]}\n";
        return output;
    });
}

extern "C" int32_t pixaura_preset_canonical(uint32_t version,const uint8_t* recipe,uint64_t bytes,uint8_t* output,uint64_t capacity,uint64_t* required) {
    return boundary(version,output,capacity,required,[&]{if(!recipe||!bytes)throw pixaura::document::Failure{1};if(bytes>PIXAURA_PRESET_MAX_BYTES)throw pixaura::document::Failure{8};
        auto parsed=pixaura::document::parse_preset({reinterpret_cast<const char*>(recipe),static_cast<std::size_t>(bytes)});if(parsed.code)throw pixaura::document::Failure{parsed.code};
        for(const auto& op:parsed.value.operations)if(!pixaura::manual::preset_operation(op))throw pixaura::document::Failure{5};
        auto canonical=pixaura::document::serialize_preset(parsed.value);if(canonical.code)throw pixaura::document::Failure{canonical.code};return canonical.value;});
}
extern "C" int32_t pixaura_preset_catalog(uint32_t version,uint8_t* output,uint64_t capacity,uint64_t* required) {
    return boundary(version,output,capacity,required,[]{return pixaura::document::String("{\"presets\":[]}\n");});
}
