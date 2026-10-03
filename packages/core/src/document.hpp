#ifndef PIXAURA_DOCUMENT_HPP
#define PIXAURA_DOCUMENT_HPP

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace pixaura::document {
constexpr std::size_t manifest_limit = 8u * 1024u * 1024u;
constexpr std::size_t command_limit = 64u * 1024u;
constexpr uint64_t max_generation = 9007199254740991ULL;

struct Failure { int32_t code; uint32_t index = UINT32_MAX; };
template<class T> struct Result { int32_t code; T value{}; uint32_t index = UINT32_MAX; };

template<std::size_t N> class HexIdentity {
    std::string text_;
    explicit HexIdentity(std::string text) : text_(std::move(text)) {}
public:
    static HexIdentity parse(std::string_view text);
    const std::string& text() const { return text_; }
    bool operator==(const HexIdentity& other) const { return text_ == other.text_; }
    bool operator!=(const HexIdentity& other) const { return !(*this == other); }
};
using Id = HexIdentity<32>;
using Digest = HexIdentity<64>;
struct DocumentIdentity { Id project; Id document; };
struct SourceMetadata {
    uint32_t width, height, orientation;
    std::string codec;
    bool has_alpha;
    std::optional<Digest> icc;
};
struct SourceDescriptor { Digest sha256; uint64_t byte_length; SourceMetadata metadata; };
struct Exposure { int32_t milli_ev; };
struct Crop { uint32_t x_ppm, y_ppm, width_ppm, height_ppm; };
struct Rotate { uint32_t quarter_turns; };
using Parameters = std::variant<Exposure, Crop, Rotate>;
struct EditOperation { Id id; std::string type; uint32_t operation_version, parameter_version; Parameters parameters; };
struct Revision { Id id; std::optional<Id> parent; std::vector<Id> stack; std::string actor; std::optional<Id> plan; };
struct SessionIdentity { Id id; uint64_t generation; };
struct DetachedCandidate {
    DocumentIdentity identity;
    Id base;
    SessionIdentity session;
    std::vector<Id> stack;
    std::vector<EditOperation> operations;
    Id revision;
    std::string actor;
    std::optional<Id> plan;
};

class ImageDocument;
using Snapshot = std::shared_ptr<const ImageDocument>;
class ImageDocument {
    DocumentIdentity identity_;
    SourceDescriptor source_;
    std::vector<EditOperation> operations_;
    std::vector<Revision> revisions_;
    Id current_;
    std::vector<Id> redo_;
    SessionIdentity session_;
    ImageDocument(DocumentIdentity identity, SourceDescriptor source,
        std::vector<EditOperation> operations, std::vector<Revision> revisions,
        Id current, std::vector<Id> redo, SessionIdentity session);
    friend struct Engine;
public:
    const DocumentIdentity& identity() const { return identity_; }
    const SourceDescriptor& source() const { return source_; }
    const std::vector<EditOperation>& operations() const { return operations_; }
    const std::vector<Revision>& revisions() const { return revisions_; }
    const Id& current() const { return current_; }
    const std::vector<Id>& redo() const { return redo_; }
    const SessionIdentity& session() const { return session_; }
};
Result<Snapshot> deserialize(std::string_view manifest, std::string_view session_id);
Result<Snapshot> create_document(std::string_view root_manifest, std::string_view session_id);
Result<std::string> serialize(const ImageDocument& document);
Result<Snapshot> transition(const ImageDocument& document, std::string_view command);
Result<Snapshot> transition(const ImageDocument& document, const DetachedCandidate& candidate);
Result<std::vector<EditOperation>> replay(const ImageDocument& document, const Id& revision);
}
#endif
