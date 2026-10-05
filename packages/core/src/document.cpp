#include "document.hpp"
#include "pixaura/document.h"

#include <algorithm>
#include <map>
#include <set>
#include <limits>
#include <new>
#include <utility>

namespace pixaura::document {
namespace {
[[noreturn]] void fail(int32_t code = PIXAURA_DOCUMENT_INVALID_PROJECT) { throw Failure{code}; }
void require(bool condition, int32_t code = PIXAURA_DOCUMENT_INVALID_PROJECT) { if (!condition) fail(code); }
template<class T, class F> Result<T> attempt(F&& function) {
    try { return {PIXAURA_OK, function()}; }
    catch (const Failure& error) { return {error.code, {}, error.index}; }
    catch (const std::bad_alloc&) { return {PIXAURA_DOCUMENT_RESOURCE_LIMIT, {}}; }
    catch (...) { return {PIXAURA_DOCUMENT_INTERNAL_ERROR, {}}; }
}

struct Json {
    using Object = std::map<String, Json>;
    using Array = Vector<Json>;
    std::variant<std::nullptr_t, bool, int64_t, String, Object, Array> data;
    Json() : data(nullptr) {}
    explicit Json(bool value) : data(value) {}
    explicit Json(int64_t value) : data(value) {}
    explicit Json(String value) : data(std::move(value)) {}
    explicit Json(Object value) : data(std::move(value)) {}
    explicit Json(Array value) : data(std::move(value)) {}
};
enum class Shape { Manifest, Source, Metadata, Operation, Parameters, Revision, Command, Evaluation, Scalar };
const std::set<String>& allowed(Shape shape) {
    // Immutable registry data only; no global mutable state.
    static const std::set<String> manifest{"schema_version", "project_id", "document_id", "source", "operations", "revisions", "current_revision_id", "redo"};
    static const std::set<String> source{"sha256", "byte_length", "metadata"};
    static const std::set<String> metadata{"width", "height", "orientation", "codec", "has_alpha", "icc_sha256"};
    static const std::set<String> operation{"id", "type", "operation_version", "parameter_version", "parameters"};
    static const std::set<String> parameters{"milli_ev", "quarter_turns", "x_ppm", "y_ppm", "width_ppm", "height_ppm"};
    static const std::set<String> revision{"id", "parent_id", "stack", "actor", "plan_id"};
    static const std::set<String> command{"command_version", "kind", "expected_revision_id", "expected_session_id", "expected_generation", "revision_id", "operations", "stack", "actor", "plan_id"};
    static const std::set<String> evaluation{"operations"};
    switch (shape) {
        case Shape::Manifest: return manifest;
        case Shape::Source: return source;
        case Shape::Metadata: return metadata;
        case Shape::Operation: return operation;
        case Shape::Parameters: return parameters;
        case Shape::Revision: return revision;
        case Shape::Command: return command;
        case Shape::Evaluation: return evaluation;
        default: fail();
    }
}

class Parser {
    std::string_view input_;
    bool evaluation_ = false;
    std::size_t position_ = 0;
    void whitespace() {
        while (position_ < input_.size() && (input_[position_] == ' ' || input_[position_] == '\n' || input_[position_] == '\r' || input_[position_] == '\t')) ++position_;
    }
    char peek() const { return position_ < input_.size() ? input_[position_] : '\0'; }
    void take(char value) { require(peek() == value); ++position_; }
    uint32_t hex4() {
        uint32_t result = 0;
        for (int i = 0; i < 4; ++i) {
            const char c = peek(); ++position_;
            uint32_t digit = 0;
            if (c >= '0' && c <= '9') digit = static_cast<uint32_t>(c - '0');
            else if (c >= 'a' && c <= 'f') digit = static_cast<uint32_t>(c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') digit = static_cast<uint32_t>(c - 'A' + 10);
            else fail();
            result = result * 16u + digit;
        }
        return result;
    }
    static void append_codepoint(String& output, uint32_t value) {
        if (value < 0x80) output.push_back(static_cast<char>(value));
        else if (value < 0x800) {
            output.push_back(static_cast<char>(0xc0u | (value >> 6)));
            output.push_back(static_cast<char>(0x80u | (value & 63u)));
        } else if (value < 0x10000) {
            output.push_back(static_cast<char>(0xe0u | (value >> 12)));
            output.push_back(static_cast<char>(0x80u | ((value >> 6) & 63u)));
            output.push_back(static_cast<char>(0x80u | (value & 63u)));
        } else {
            output.push_back(static_cast<char>(0xf0u | (value >> 18)));
            output.push_back(static_cast<char>(0x80u | ((value >> 12) & 63u)));
            output.push_back(static_cast<char>(0x80u | ((value >> 6) & 63u)));
            output.push_back(static_cast<char>(0x80u | (value & 63u)));
        }
    }
    String string() {
        take('"');
        String result;
        while (position_ < input_.size()) {
            const auto c = static_cast<uint8_t>(input_[position_++]);
            if (c == '"') return result;
            require(c >= 32);
            if (c == '\\') {
                const char escape = peek(); ++position_;
                switch (escape) {
                    case '"': result.push_back('"'); break;
                    case '\\': result.push_back('\\'); break;
                    case '/': result.push_back('/'); break;
                    case 'b': result.push_back('\b'); break;
                    case 'f': result.push_back('\f'); break;
                    case 'n': result.push_back('\n'); break;
                    case 'r': result.push_back('\r'); break;
                    case 't': result.push_back('\t'); break;
                    case 'u': {
                        uint32_t cp = hex4();
                        if (cp >= 0xd800 && cp <= 0xdbff) {
                            take('\\'); take('u'); const auto low = hex4();
                            require(low >= 0xdc00 && low <= 0xdfff);
                            cp = 0x10000u + (cp - 0xd800u) * 1024u + low - 0xdc00u;
                        } else require(cp < 0xdc00 || cp > 0xdfff);
                        append_codepoint(result, cp); break;
                    }
                    default: fail();
                }
            } else if (c < 0x80) result.push_back(static_cast<char>(c));
            else {
                uint32_t cp; uint32_t remaining; uint32_t minimum;
                if (c >= 0xc2 && c <= 0xdf) { cp = c & 31u; remaining = 1; minimum = 0x80; }
                else if (c >= 0xe0 && c <= 0xef) { cp = c & 15u; remaining = 2; minimum = 0x800; }
                else if (c >= 0xf0 && c <= 0xf4) { cp = c & 7u; remaining = 3; minimum = 0x10000; }
                else fail();
                for (uint32_t i = 0; i < remaining; ++i) {
                    require(position_ < input_.size());
                    const auto next = static_cast<uint8_t>(input_[position_++]);
                    require(next >= 0x80 && next <= 0xbf); cp = (cp << 6) | (next & 63u);
                }
                require(cp >= minimum && cp <= 0x10ffff && (cp < 0xd800 || cp > 0xdfff));
                append_codepoint(result, cp);
            }
            require(result.size() <= 256, PIXAURA_DOCUMENT_RESOURCE_LIMIT);
        }
        fail();
    }
    Json value(uint32_t depth, Shape shape = Shape::Scalar, std::size_t array_limit = 0, Shape element = Shape::Scalar) {
        whitespace();
        if (peek() == '{' || peek() == '[') require(depth <= 16, PIXAURA_DOCUMENT_RESOURCE_LIMIT);
        if (peek() == '{') {
            require(shape != Shape::Scalar);
            ++position_; whitespace(); Json::Object result;
            if (peek() == '}') { ++position_; return Json(std::move(result)); }
            for (;;) {
                auto key = string();
                require(allowed(shape).count(key) == 1, shape == Shape::Parameters ? PIXAURA_DOCUMENT_INVALID_PARAMETERS : PIXAURA_DOCUMENT_INVALID_PROJECT);
                require(result.count(key) == 0);
                whitespace(); take(':');
                Shape child = Shape::Scalar; Shape item = Shape::Scalar; std::size_t bound = 0;
                if (key == "source") child = Shape::Source;
                else if (key == "metadata") child = Shape::Metadata;
                else if (key == "parameters") child = Shape::Parameters;
                else if (key == "operations") { bound = evaluation_ ? 256 : 4096; item = Shape::Operation; }
                else if (key == "revisions") { bound = 4096; item = Shape::Revision; }
                else if (key == "stack") bound = 256;
                else if (key == "redo") bound = 4095;
                result.emplace(std::move(key), value(depth + 1, child, bound, item));
                whitespace();
                if (peek() == '}') { ++position_; break; }
                take(','); whitespace();
            }
            return Json(std::move(result));
        }
        if (peek() == '[') {
            require(array_limit != 0); ++position_; whitespace(); Json::Array result;
            if (peek() == ']') { ++position_; return Json(std::move(result)); }
            for (;;) {
                require(result.size() < array_limit, PIXAURA_DOCUMENT_RESOURCE_LIMIT);
                whitespace();
                const auto start = position_;
                result.push_back(value(depth + 1, element));
                if (evaluation_ && element == Shape::Operation) require(position_ - start <= 1024, PIXAURA_DOCUMENT_RESOURCE_LIMIT);
                whitespace();
                if (peek() == ']') { ++position_; break; }
                take(',');
            }
            return Json(std::move(result));
        }
        if (peek() == '"') return Json(string());
        for (const auto& token : {std::string_view("true"), std::string_view("false"), std::string_view("null")}) {
            if (input_.substr(position_, token.size()) == token) {
                position_ += token.size();
                return token == "null" ? Json() : Json(token == "true");
            }
        }
        const bool negative = peek() == '-'; if (negative) ++position_;
        require(peek() >= '0' && peek() <= '9');
        const bool zero = peek() == '0'; uint64_t number = 0;
        do {
            const auto digit = static_cast<uint64_t>(peek() - '0');
            require(number <= (max_generation - digit) / 10);
            number = number * 10 + digit; ++position_;
            if (zero) break;
        } while (peek() >= '0' && peek() <= '9');
        require(!(zero && peek() >= '0' && peek() <= '9') && !(negative && number == 0));
        require(peek() != '.' && peek() != 'e' && peek() != 'E');
        return Json(negative ? -static_cast<int64_t>(number) : static_cast<int64_t>(number));
    }
public:
    Parser(std::string_view input, std::size_t bound, bool evaluation = false) : input_(input), evaluation_(evaluation) {
        require(input.size() <= bound, PIXAURA_DOCUMENT_RESOURCE_LIMIT);
    }
    Json parse(Shape shape) { auto result = value(1, shape); whitespace(); require(position_ == input_.size()); return result; }
};

const Json::Object& object(const Json& value) { const auto* p = std::get_if<Json::Object>(&value.data); require(p != nullptr); return *p; }
const Json::Array& array(const Json& value) { const auto* p = std::get_if<Json::Array>(&value.data); require(p != nullptr); return *p; }
const String& text(const Json& value) { const auto* p = std::get_if<String>(&value.data); require(p != nullptr); return *p; }
const Json& field(const Json& value, const char* key) { const auto& o = object(value); const auto it = o.find(key); require(it != o.end()); return it->second; }
void keys(const Json& value, std::initializer_list<const char*> expected, int32_t code = PIXAURA_DOCUMENT_INVALID_PROJECT) {
    const auto* o = std::get_if<Json::Object>(&value.data); require(o != nullptr, code); require(o->size() == expected.size(), code);
    for (const auto* name : expected) require(o->count(name) == 1, code);
}
int64_t integer(const Json& value, int64_t low, int64_t high, int32_t code = PIXAURA_DOCUMENT_INVALID_PROJECT) {
    const auto* n = std::get_if<int64_t>(&value.data); require(n != nullptr, code); require(*n >= low && *n <= high, code); return *n;
}
bool boolean(const Json& value) { const auto* b = std::get_if<bool>(&value.data); require(b != nullptr); return *b; }
bool null(const Json& value) { return std::holds_alternative<std::nullptr_t>(value.data); }
Id id(const Json& value) { return Id::parse(text(value)); }
std::optional<Id> optional_id(const Json& value) { return null(value) ? std::nullopt : std::optional<Id>(id(value)); }
Vector<Id> stack(const Json& value) {
    Vector<Id> result; std::set<String> seen;
    for (const auto& entry : array(value)) { auto identity = id(entry); require(seen.insert(identity.text()).second); result.push_back(std::move(identity)); }
    return result;
}
EditOperation operation(const Json& value) {
    keys(value, {"id", "type", "operation_version", "parameter_version", "parameters"});
    const auto& type = text(field(value, "type"));
    require(!type.empty() && type.size() <= 64 && type[0] >= 'a' && type[0] <= 'z');
    require(std::all_of(type.begin(), type.end(), [](char c) { return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '.'; }));
    const auto ov = integer(field(value, "operation_version"), 1, UINT32_MAX);
    const auto pv = integer(field(value, "parameter_version"), 1, UINT32_MAX);
    require(ov == 1 && pv == 1, PIXAURA_DOCUMENT_UNSUPPORTED_OPERATION);
    const auto& p = field(value, "parameters"); Parameters params;
    const auto number = [&](const char* name, int64_t low, int64_t high) { return integer(field(p, name), low, high, PIXAURA_DOCUMENT_INVALID_PARAMETERS); };
    if (type == "pixaura.exposure") {
        keys(p, {"milli_ev"}, PIXAURA_DOCUMENT_INVALID_PARAMETERS);
        params = Exposure{static_cast<int32_t>(number("milli_ev", -5000, 5000))};
    } else if (type == "pixaura.rotate") {
        keys(p, {"quarter_turns"}, PIXAURA_DOCUMENT_INVALID_PARAMETERS);
        params = Rotate{static_cast<uint32_t>(number("quarter_turns", 0, 3))};
    } else if (type == "pixaura.crop") {
        keys(p, {"x_ppm", "y_ppm", "width_ppm", "height_ppm"}, PIXAURA_DOCUMENT_INVALID_PARAMETERS);
        Crop c{static_cast<uint32_t>(number("x_ppm", 0, 999999)), static_cast<uint32_t>(number("y_ppm", 0, 999999)),
            static_cast<uint32_t>(number("width_ppm", 1, 1000000)), static_cast<uint32_t>(number("height_ppm", 1, 1000000))};
        require(c.x_ppm + c.width_ppm <= 1000000 && c.y_ppm + c.height_ppm <= 1000000, PIXAURA_DOCUMENT_INVALID_PARAMETERS); params = c;
    } else fail(PIXAURA_DOCUMENT_UNSUPPORTED_OPERATION);
    return {id(field(value, "id")), type, static_cast<uint32_t>(ov), static_cast<uint32_t>(pv), params};
}
SourceDescriptor source(const Json& value) {
    keys(value, {"sha256", "byte_length", "metadata"});
    const auto& m = field(value, "metadata"); keys(m, {"width", "height", "orientation", "codec", "has_alpha", "icc_sha256"});
    const auto width = integer(field(m, "width"), 1, 65535); const auto height = integer(field(m, "height"), 1, 65535);
    require(width * height <= 268435456, PIXAURA_DOCUMENT_RESOURCE_LIMIT);
    const auto& codec = text(field(m, "codec")); const bool alpha = boolean(field(m, "has_alpha"));
    require((codec == "jpeg" || codec == "png") && (codec != "jpeg" || !alpha));
    const auto& icc = field(m, "icc_sha256");
    return {Digest::parse(text(field(value, "sha256"))), static_cast<uint64_t>(integer(field(value, "byte_length"), 1, 8589934592LL)),
        {static_cast<uint32_t>(width), static_cast<uint32_t>(height), static_cast<uint32_t>(integer(field(m, "orientation"), 1, 8)), codec, alpha,
            null(icc) ? std::nullopt : std::optional<Digest>(Digest::parse(text(icc)))}};
}
Json ids_json(const Vector<Id>& ids) { Json::Array result; for (const auto& i : ids) result.emplace_back(i.text()); return Json(std::move(result)); }
Json nullable(const std::optional<Id>& value) { return value ? Json(value->text()) : Json(); }
Json operation_json(const EditOperation& op) {
    Json::Object parameters;
    if (const auto* p = std::get_if<Exposure>(&op.parameters)) parameters.emplace("milli_ev", Json(static_cast<int64_t>(p->milli_ev)));
    else if (const auto* c = std::get_if<Crop>(&op.parameters)) {
        parameters = {{"x_ppm", Json(static_cast<int64_t>(c->x_ppm))}, {"y_ppm", Json(static_cast<int64_t>(c->y_ppm))},
            {"width_ppm", Json(static_cast<int64_t>(c->width_ppm))}, {"height_ppm", Json(static_cast<int64_t>(c->height_ppm))}};
    } else parameters.emplace("quarter_turns", Json(static_cast<int64_t>(std::get<Rotate>(op.parameters).quarter_turns)));
    return Json(Json::Object{{"id", Json(op.id.text())}, {"type", Json(op.type)}, {"operation_version", Json(static_cast<int64_t>(op.operation_version))},
        {"parameter_version", Json(static_cast<int64_t>(op.parameter_version))}, {"parameters", Json(std::move(parameters))}});
}
class Writer {
    String output_;
    void append(std::string_view value) { require(value.size() <= manifest_limit - output_.size(), PIXAURA_DOCUMENT_RESOURCE_LIMIT); output_.append(value); }
    void string(const String& value) {
        append("\"");
        for (const unsigned char c : value) {
            switch (c) {
                case '"': append("\\\""); break;
                case '\\': append("\\\\"); break;
                case '\b': append("\\b"); break;
                case '\f': append("\\f"); break;
                case '\n': append("\\n"); break;
                case '\r': append("\\r"); break;
                case '\t': append("\\t"); break;
                default:
                    if (c < 32) { const char digits[] = "0123456789abcdef"; char escaped[] = {'\\', 'u', '0', '0', digits[c >> 4], digits[c & 15]}; append(std::string_view(escaped, 6)); }
                    else { const char byte = static_cast<char>(c); append(std::string_view(&byte, 1)); }
            }
        }
        append("\"");
    }
    void value(const Json& json, uint32_t depth) {
        require(depth <= 16, PIXAURA_DOCUMENT_RESOURCE_LIMIT);
        if (std::holds_alternative<std::nullptr_t>(json.data)) append("null");
        else if (const auto* b = std::get_if<bool>(&json.data)) append(*b ? "true" : "false");
        else if (const auto* n = std::get_if<int64_t>(&json.data)) append(std::to_string(*n));
        else if (const auto* s = std::get_if<String>(&json.data)) string(*s);
        else if (const auto* o = std::get_if<Json::Object>(&json.data)) {
            append("{"); bool first = true;
            for (const auto& entry : *o) { if (!first) append(","); first = false; string(entry.first); append(":"); value(entry.second, depth + 1); }
            append("}");
        } else {
            append("["); bool first = true;
            for (const auto& entry : std::get<Json::Array>(json.data)) { if (!first) append(","); first = false; value(entry, depth + 1); }
            append("]");
        }
    }
public:
    String write(const Json& json) { value(json, 1); append("\n"); return std::move(output_); }
};
}

template<std::size_t N> HexIdentity<N> HexIdentity<N>::parse(std::string_view text) {
    require(text.size() == N && std::all_of(text.begin(), text.end(), [](char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); }));
    return HexIdentity(String(text));
}
template class HexIdentity<32>;
template class HexIdentity<64>;

ImageDocument::ImageDocument(DocumentIdentity identity, SourceDescriptor source_value,
    Vector<EditOperation> operations, Vector<Revision> revisions, Id current,
    Vector<Id> redo, SessionIdentity session) : identity_(std::move(identity)), source_(std::move(source_value)),
    operations_(std::move(operations)), revisions_(std::move(revisions)), current_(std::move(current)), redo_(std::move(redo)), session_(std::move(session)) {}

struct Engine {
    static const Revision& revision(const ImageDocument& doc, const Id& id_value) {
        const auto it = std::find_if(doc.revisions_.begin(), doc.revisions_.end(), [&](const Revision& r) { return r.id == id_value; });
        require(it != doc.revisions_.end()); return *it;
    }
    static void validate(const ImageDocument& doc) {
        require(doc.operations_.size() <= 4096 && doc.revisions_.size() <= 4096 && doc.redo_.size() <= 4095, PIXAURA_DOCUMENT_RESOURCE_LIMIT);
        require(!doc.revisions_.empty());
        std::set<String> operations, referenced; std::map<String, std::optional<Id>> revisions;
        for (const auto& op : doc.operations_) { operation(operation_json(op)); require(operations.insert(op.id.text()).second); }
        for (std::size_t i = 0; i < doc.revisions_.size(); ++i) {
            const auto& r = doc.revisions_[i];
            try {
                require(r.stack.size() <= 256, PIXAURA_DOCUMENT_RESOURCE_LIMIT);
                if (i == 0) require(!r.parent && r.actor == "import" && !r.plan && r.stack.empty());
                else {
                    require(r.parent && revisions.count(r.parent->text()) == 1);
                    require((r.actor == "manual" && !r.plan) || (r.actor == "ai" && r.plan));
                }
                std::set<String> unique;
                for (const auto& op : r.stack) { require(operations.count(op.text()) == 1 && unique.insert(op.text()).second); referenced.insert(op.text()); }
                require(revisions.emplace(r.id.text(), r.parent).second);
            } catch (Failure& error) { error.index = static_cast<uint32_t>(i); throw; }
        }
        require(referenced.size() == operations.size() && revisions.count(doc.current_.text()) == 1);
        std::set<String> seen; Id previous = doc.current_;
        for (const auto& r : doc.redo_) {
            const auto found = revisions.find(r.text()); require(found != revisions.end() && found->second && *found->second == previous && seen.insert(r.text()).second); previous = r;
        }
        require(doc.session_.generation <= max_generation, PIXAURA_DOCUMENT_RESOURCE_LIMIT);
    }
    static Snapshot decode(const Json& json, std::string_view session) {
        keys(json, {"schema_version", "project_id", "document_id", "source", "operations", "revisions", "current_revision_id", "redo"});
        require(integer(field(json, "schema_version"), 0, static_cast<int64_t>(max_generation)) == 1, PIXAURA_DOCUMENT_UNSUPPORTED_SCHEMA);
        Vector<EditOperation> operations;
        for (const auto& op : array(field(json, "operations"))) {
            try { operations.push_back(operation(op)); }
            catch (Failure& error) { error.index = static_cast<uint32_t>(operations.size()); throw; }
        }
        Vector<Revision> revisions;
        for (const auto& r : array(field(json, "revisions"))) {
            keys(r, {"id", "parent_id", "stack", "actor", "plan_id"});
            revisions.push_back({id(field(r, "id")), optional_id(field(r, "parent_id")), stack(field(r, "stack")), text(field(r, "actor")), optional_id(field(r, "plan_id"))});
        }
        auto doc = std::shared_ptr<ImageDocument>(new ImageDocument({id(field(json, "project_id")), id(field(json, "document_id"))}, source(field(json, "source")),
            std::move(operations), std::move(revisions), id(field(json, "current_revision_id")), stack(field(json, "redo")), {Id::parse(session), 0}));
        validate(*doc); return doc;
    }
    static Json encode(const ImageDocument& doc) {
        Json::Array operations, revisions;
        for (const auto& op : doc.operations_) operations.push_back(operation_json(op));
        for (const auto& r : doc.revisions_) revisions.emplace_back(Json::Object{{"id", Json(r.id.text())}, {"parent_id", nullable(r.parent)},
            {"stack", ids_json(r.stack)}, {"actor", Json(r.actor)}, {"plan_id", nullable(r.plan)}});
        const auto& m = doc.source_.metadata;
        Json metadata(Json::Object{{"width", Json(static_cast<int64_t>(m.width))}, {"height", Json(static_cast<int64_t>(m.height))},
            {"orientation", Json(static_cast<int64_t>(m.orientation))}, {"codec", Json(m.codec)}, {"has_alpha", Json(m.has_alpha)}, {"icc_sha256", m.icc ? Json(m.icc->text()) : Json()}});
        Json source_json(Json::Object{{"sha256", Json(doc.source_.sha256.text())}, {"byte_length", Json(static_cast<int64_t>(doc.source_.byte_length))}, {"metadata", std::move(metadata)}});
        return Json(Json::Object{{"schema_version", Json(int64_t{1})}, {"project_id", Json(doc.identity_.project.text())}, {"document_id", Json(doc.identity_.document.text())},
            {"source", std::move(source_json)}, {"operations", Json(std::move(operations))}, {"revisions", Json(std::move(revisions))}, {"current_revision_id", Json(doc.current_.text())}, {"redo", ids_json(doc.redo_)}});
    }
    static void check_base(const ImageDocument& doc, const Id& revision_id, const SessionIdentity& session) {
        require(doc.current_ == revision_id && doc.session_.id == session.id && doc.session_.generation == session.generation, PIXAURA_DOCUMENT_STALE_BASE);
        require(doc.session_.generation < max_generation, PIXAURA_DOCUMENT_RESOURCE_LIMIT);
    }
    static Snapshot finish(std::shared_ptr<ImageDocument> next) {
        ++next->session_.generation; validate(*next); Writer().write(encode(*next)); return next;
    }
    static Snapshot restore(Snapshot snapshot, uint64_t generation) {
        require(snapshot != nullptr && generation <= max_generation, PIXAURA_DOCUMENT_RESOURCE_LIMIT);
        auto next = std::make_shared<ImageDocument>(*snapshot);
        next->session_.generation = generation; validate(*next); return next;
    }
    static Snapshot commit(const ImageDocument& doc, const DetachedCandidate& candidate) {
        require(candidate.identity.project == doc.identity_.project && candidate.identity.document == doc.identity_.document, PIXAURA_DOCUMENT_STALE_BASE);
        check_base(doc, candidate.base, candidate.session);
        require(candidate.operations.size() <= 4096 - doc.operations_.size() && doc.revisions_.size() < 4096 && candidate.stack.size() <= 256, PIXAURA_DOCUMENT_RESOURCE_LIMIT);
        std::set<String> existing;
        for (const auto& op : doc.operations_) existing.insert(op.id.text());
        for (const auto& op : candidate.operations) {
            require(existing.insert(op.id.text()).second);
            require(std::find(candidate.stack.begin(), candidate.stack.end(), op.id) != candidate.stack.end());
        }
        auto next = std::shared_ptr<ImageDocument>(new ImageDocument(doc));
        next->operations_.insert(next->operations_.end(), candidate.operations.begin(), candidate.operations.end());
        next->revisions_.push_back({candidate.revision, doc.current_, candidate.stack, candidate.actor, candidate.plan});
        next->current_ = candidate.revision; next->redo_.clear(); return finish(std::move(next));
    }
    static Snapshot apply(const ImageDocument& doc, const Json& command) {
        const auto version = integer(field(command, "command_version"), 0, UINT32_MAX);
        require(version == 1, PIXAURA_UNSUPPORTED_ABI);
        const auto& kind = text(field(command, "kind"));
        const auto base = id(field(command, "expected_revision_id"));
        SessionIdentity session{id(field(command, "expected_session_id")), static_cast<uint64_t>(integer(field(command, "expected_generation"), 0, static_cast<int64_t>(max_generation)))};
        check_base(doc, base, session);
        if (kind == "commit") {
            keys(command, {"command_version", "kind", "expected_revision_id", "expected_session_id", "expected_generation", "revision_id", "operations", "stack", "actor", "plan_id"});
            Vector<EditOperation> operations;
            for (const auto& op : array(field(command, "operations"))) operations.push_back(operation(op));
            return commit(doc, {doc.identity_, base, session, stack(field(command, "stack")), std::move(operations), id(field(command, "revision_id")), text(field(command, "actor")), optional_id(field(command, "plan_id"))});
        }
        auto next = std::shared_ptr<ImageDocument>(new ImageDocument(doc));
        if (kind == "checkout") {
            keys(command, {"command_version", "kind", "expected_revision_id", "expected_session_id", "expected_generation", "revision_id"});
            next->current_ = id(field(command, "revision_id")); revision(doc, next->current_); next->redo_.clear();
        } else if (kind == "undo" || kind == "redo") {
            keys(command, {"command_version", "kind", "expected_revision_id", "expected_session_id", "expected_generation"});
            if (kind == "undo") {
                const auto& current = revision(doc, doc.current_); require(current.parent.has_value(), PIXAURA_DOCUMENT_NO_HISTORY);
                require(next->redo_.size() < 4095, PIXAURA_DOCUMENT_RESOURCE_LIMIT);
                next->redo_.insert(next->redo_.begin(), current.id); next->current_ = *current.parent;
            } else {
                require(!next->redo_.empty(), PIXAURA_DOCUMENT_NO_HISTORY); next->current_ = next->redo_.front(); next->redo_.erase(next->redo_.begin());
            }
        } else fail(PIXAURA_INVALID_ARGUMENT);
        return finish(std::move(next));
    }
};

Result<Snapshot> deserialize(std::string_view manifest, std::string_view session_id) {
    return attempt<Snapshot>([&] { return Engine::decode(Parser(manifest, manifest_limit).parse(Shape::Manifest), session_id); });
}
Result<Snapshot> restore_generation(Snapshot snapshot, uint64_t generation) {
    return attempt<Snapshot>([&] { return Engine::restore(std::move(snapshot), generation); });
}
Result<Snapshot> create_document(std::string_view manifest, std::string_view session_id) {
    return attempt<Snapshot>([&] {
        auto doc = Engine::decode(Parser(manifest, manifest_limit).parse(Shape::Manifest), session_id);
        require(doc->operations().empty() && doc->revisions().size() == 1 && doc->redo().empty()); return doc;
    });
}
Result<String> serialize(const ImageDocument& doc) {
    return attempt<String>([&] { Engine::validate(doc); return Writer().write(Engine::encode(doc)); });
}
Result<Snapshot> transition(const ImageDocument& doc, std::string_view command) {
    return attempt<Snapshot>([&] { return Engine::apply(doc, Parser(command, command_limit).parse(Shape::Command)); });
}
Result<Snapshot> transition(const ImageDocument& doc, const DetachedCandidate& candidate) {
    return attempt<Snapshot>([&] { return Engine::commit(doc, candidate); });
}
Result<Vector<EditOperation>> replay(const ImageDocument& doc, const Id& revision_id) {
    return attempt<Vector<EditOperation>>([&] {
        Vector<EditOperation> ordered;
        for (const auto& op_id : Engine::revision(doc, revision_id).stack) {
            const auto found = std::find_if(doc.operations().begin(), doc.operations().end(), [&](const EditOperation& op) { return op.id == op_id; });
            require(found != doc.operations().end()); ordered.push_back(*found);
        }
        return ordered;
    });
}
Result<Vector<EditOperation>> parse_evaluation(std::string_view request) {
    return attempt<Vector<EditOperation>>([&] {
        const auto json = Parser(request, command_limit, true).parse(Shape::Evaluation);
        keys(json, {"operations"});
        Vector<EditOperation> ordered;
        std::set<String> seen;
        for (const auto& value : array(field(json, "operations"))) {
            auto op = operation(value);
            require(seen.insert(op.id.text()).second);
            ordered.push_back(std::move(op));
        }
        return ordered;
    });
}
}
