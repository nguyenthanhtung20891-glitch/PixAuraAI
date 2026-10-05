#ifndef PIXAURA_DOCUMENT_HPP
#define PIXAURA_DOCUMENT_HPP

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include "document_containers.hpp"

namespace pixaura::document {
constexpr std::size_t manifest_limit = 8u * 1024u * 1024u;
constexpr std::size_t command_limit = 64u * 1024u;
constexpr uint64_t max_generation = 9007199254740991ULL;

struct Failure { int32_t code; uint32_t index = UINT32_MAX; };
template<class T> struct Result { int32_t code; T value{}; uint32_t index = UINT32_MAX; };

template<std::size_t N> class HexIdentity {
    String text_;
    explicit HexIdentity(String text) : text_(std::move(text)) {}
public:
    static HexIdentity parse(std::string_view text);
    const String& text() const { return text_; }
    bool operator==(const HexIdentity& other) const { return text_ == other.text_; }
    bool operator!=(const HexIdentity& other) const { return !(*this == other); }
};
using Id = HexIdentity<32>;
using Digest = HexIdentity<64>;
struct DocumentIdentity { Id project; Id document; };
struct SourceMetadata {
    uint32_t width, height, orientation;
    String codec;
    bool has_alpha;
    std::optional<Digest> icc;
};
struct SourceDescriptor { Digest sha256; uint64_t byte_length; SourceMetadata metadata; };
struct Exposure { int32_t milli_ev; };
struct Crop { uint32_t x_ppm, y_ppm, width_ppm, height_ppm; };
struct Rotate { uint32_t quarter_turns; };
using Parameters = std::variant<Exposure, Crop, Rotate>;
struct EditOperation { Id id; String type; uint32_t operation_version, parameter_version; Parameters parameters; };
struct Revision { Id id; std::optional<Id> parent; Vector<Id> stack; String actor; std::optional<Id> plan; };
struct SessionIdentity { Id id; uint64_t generation; };
struct DetachedCandidate {
    DocumentIdentity identity;
    Id base;
    SessionIdentity session;
    Vector<Id> stack;
    Vector<EditOperation> operations;
    Id revision;
    String actor;
    std::optional<Id> plan;
};

class ImageDocument;
using Snapshot = std::shared_ptr<const ImageDocument>;
class ImageDocument {
    DocumentIdentity identity_;
    SourceDescriptor source_;
    Vector<EditOperation> operations_;
    Vector<Revision> revisions_;
    Id current_;
    Vector<Id> redo_;
    SessionIdentity session_;
    ImageDocument(DocumentIdentity identity, SourceDescriptor source,
        Vector<EditOperation> operations, Vector<Revision> revisions,
        Id current, Vector<Id> redo, SessionIdentity session);
    friend struct Engine;
public:
    const DocumentIdentity& identity() const { return identity_; }
    const SourceDescriptor& source() const { return source_; }
    const Vector<EditOperation>& operations() const { return operations_; }
    const Vector<Revision>& revisions() const { return revisions_; }
    const Id& current() const { return current_; }
    const Vector<Id>& redo() const { return redo_; }
    const SessionIdentity& session() const { return session_; }
};
Result<Snapshot> deserialize(std::string_view manifest, std::string_view session_id);
// Storage-only envelope restoration; canonical schema 1 remains session-free.
Result<Snapshot> restore_generation(Snapshot snapshot, uint64_t generation);
Result<Snapshot> create_document(std::string_view root_manifest, std::string_view session_id);
Result<String> serialize(const ImageDocument& document);
Result<Snapshot> transition(const ImageDocument& document, std::string_view command);
Result<Snapshot> transition(const ImageDocument& document, const DetachedCandidate& candidate);
Result<Vector<EditOperation>> replay(const ImageDocument& document, const Id& revision);
// Bounded evaluation envelope: {"operations":[schema-1 operation records]}.
Result<Vector<EditOperation>> parse_evaluation(std::string_view request);
}
#endif
