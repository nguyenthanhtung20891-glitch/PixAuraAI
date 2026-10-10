#ifndef PIXAURA_MANUAL_TOOLS_HPP
#define PIXAURA_MANUAL_TOOLS_HPP
#include "document.hpp"
#include "pixaura/manual.h"
#include <array>

namespace pixaura::manual {
struct Parameter {
    std::string_view name, unit;
    int32_t minimum, maximum, neutral, step;
};
struct Descriptor {
    std::string_view tool_id, type, category, name;
    uint32_t operation_version, parameter_version;
    const Parameter* parameters;
    std::size_t parameter_count;
};
const std::array<Descriptor, 11>& registry();
// Validates controlled startup tables; never installs or changes the registry.
int32_t validate_descriptors(const Descriptor* descriptors, std::size_t count);
const Descriptor* find(std::string_view tool, uint32_t operation_version = 1,
    uint32_t parameter_version = 1);

// Contract reference controller, serialized by its application owner, no UI,
// render, approval, persistence, scheduler or cross-thread publication authority.
class Gesture {
    document::Snapshot base_;
    document::Vector<document::EditOperation> stack_;
    document::Id gesture_id_;
    const Descriptor* descriptor_;
    std::optional<std::size_t> replacement_;
    std::shared_ptr<const document::EditOperation> pending_;
    uint64_t sequence_ = 0;
    bool closed_ = false;
    Gesture(document::Snapshot base, document::Vector<document::EditOperation> stack,
        document::Id gesture, const Descriptor* descriptor, std::optional<std::size_t> replacement);
    bool current(const document::Snapshot& live) const;
public:
    Gesture(const Gesture&) = default;
    bool active() const { return !closed_; }
    bool bound_to(const document::Snapshot& live) const { return current(live); }
    bool same_owner(const document::Snapshot& live) const {
        return live && live->identity().project == base_->identity().project &&
            live->identity().document == base_->identity().document;
    }
    uint64_t sequence() const { return sequence_; }
    static document::Result<std::unique_ptr<Gesture>> begin(document::Snapshot base,
        std::string_view fresh_gesture_id, std::string_view tool,
        std::optional<std::string_view> replace_operation = std::nullopt);
    document::Result<uint64_t> update(std::string_view single_operation_request);
    bool eligible(std::string_view gesture, uint64_t sequence, const document::Snapshot& live) const;
    document::Result<document::String> preview(const document::Snapshot& live) const;
    document::Result<document::Snapshot> commit(const document::Snapshot& live,
        std::string_view fresh_revision_id);
    void cancel();
};
}
#endif
