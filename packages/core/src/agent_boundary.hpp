// Included in document_api.cpp: reuse its registry, mutex and exception boundary.
#include <initializer_list>
namespace {
constexpr uint32_t agent_kind = 0x41475031;
static_assert(sizeof(pixaura_agent_proposal) == 56 && offsetof(pixaura_agent_proposal, serial) == 48, "agent token ABI");
struct AgentRange { const void* pointer; uint64_t bytes; std::size_t alignment = 1; };
void agent_ranges(std::initializer_list<AgentRange> ranges) {
    for (const auto& a : ranges) {
        require(a.pointer || a.bytes == 0);
        const auto start = reinterpret_cast<uintptr_t>(a.pointer);
        require(start % a.alignment == 0 && a.bytes <= UINTPTR_MAX - start);
        for (const auto& b : ranges) {
            if (&a == &b || !a.bytes || !b.bytes) continue;
            const auto other = reinterpret_cast<uintptr_t>(b.pointer);
            require(b.bytes <= UINTPTR_MAX - other);
            require(start >= other + b.bytes || other >= start + a.bytes);
        }
    }
}
void agent_version(uint32_t version) { require(version == 1, PIXAURA_UNSUPPORTED_ABI); }
AgentEntry& agent_entry(const Header& h, const pixaura_agent_proposal* proposal) {
    require(proposal->api_version == 1 && proposal->struct_size == sizeof(*proposal) &&
        proposal->kind == agent_kind && proposal->reserved == 0 && proposal->serial &&
        std::memcmp(proposal->context_id, h.identity, 32) == 0, PIXAURA_DOCUMENT_INVALID_HANDLE);
    const auto found = h.registry->agents.find(proposal->serial);
    require(found != h.registry->agents.end(), PIXAURA_DOCUMENT_INVALID_HANDLE);
    return found->second;
}
AgentEntry& agent_current(const Header& h, const pixaura_agent_proposal* proposal,
    const pixaura_document_handle* base) {
    auto& entry = agent_entry(h, proposal);
    require(entry.terminal == 0, entry.terminal);
    handle_valid(h, base);
    if (base->serial != entry.base_serial || !h.registry->snapshots.count(entry.base_serial)) {
        entry.terminal = PIXAURA_DOCUMENT_STALE_BASE;
        throw Failure{entry.terminal};
    }
    return entry;
}
template<class F> int32_t agent_read(uint32_t version, pixaura_document_context* context,
    const pixaura_agent_proposal* proposal, const pixaura_document_handle* base, F&& action) {
    return boundary(nullptr, [&] {
        agent_version(version);
        agent_ranges({{context,sizeof(*context),alignof(pixaura_document_context)},{proposal,sizeof(*proposal),alignof(pixaura_agent_proposal)},{base,sizeof(*base),alignof(pixaura_document_handle)}});
        const auto h = live(context); std::lock_guard<std::mutex> guard(h.registry->mutex);
        return action(agent_current(h, proposal, base));
    });
}
}
int32_t pixaura_agent_stage(uint32_t version, pixaura_document_context* context,
    const pixaura_document_handle* base, const uint8_t* input, uint64_t bytes,
    pixaura_agent_proposal* output) {
    return boundary(nullptr, [&] {
        agent_version(version);
        require(bytes && bytes <= PIXAURA_AGENT_MAX_BYTES, PIXAURA_DOCUMENT_RESOURCE_LIMIT);
        agent_ranges({{context,sizeof(*context),alignof(pixaura_document_context)},{base,sizeof(*base),alignof(pixaura_document_handle)},{input,bytes},{output,sizeof(*output),alignof(pixaura_agent_proposal)}});
        const auto h = live(context); std::lock_guard<std::mutex> guard(h.registry->mutex);
        (void)snapshot_locked(h,base);
        require(h.registry->agents.size() < PIXAURA_AGENT_MAX_PROPOSALS &&
            h.registry->snapshots.size()+h.registry->gestures.size()+h.registry->agents.size() < max_handles &&
            h.registry->next != UINT64_MAX, PIXAURA_DOCUMENT_RESOURCE_LIMIT);
        auto parsed = parse_evaluation({reinterpret_cast<const char*>(input),static_cast<std::size_t>(bytes)});
        if (parsed.code) throw Failure{parsed.code,parsed.index};
        require(!parsed.value.empty());
        require(parsed.value.size() <= PIXAURA_AGENT_MAX_ACTIONS, PIXAURA_DOCUMENT_RESOURCE_LIMIT);
        for (const auto& op : parsed.value)
            require(pixaura::manual::find(op.type,op.operation_version,op.parameter_version) != nullptr,
                PIXAURA_DOCUMENT_UNSUPPORTED_OPERATION);
        auto canonical = serialize_evaluation(parsed.value);
        if (canonical.code) throw Failure{canonical.code,canonical.index};
        require(canonical.value.size() <= PIXAURA_AGENT_MAX_BYTES, PIXAURA_DOCUMENT_RESOURCE_LIMIT);
        const auto serial = h.registry->next;
        h.registry->agents.emplace(serial,AgentEntry{base->serial,std::move(canonical.value),0});
        ++h.registry->next;
        pixaura_agent_proposal result{};
        result.api_version=1; result.struct_size=sizeof(result); result.kind=agent_kind; result.serial=serial;
        std::memcpy(result.context_id,h.identity,32); *output=result;
        return PIXAURA_OK;
    });
}
int32_t pixaura_agent_validate(uint32_t version, pixaura_document_context* context,
    const pixaura_agent_proposal* proposal, const pixaura_document_handle* base) {
    return agent_read(version,context,proposal,base,[](const AgentEntry&){return PIXAURA_OK;});
}
int32_t pixaura_agent_execution_authority(uint32_t version, pixaura_document_context* context,
    const pixaura_agent_proposal* proposal, const pixaura_document_handle* base) {
    return agent_read(version,context,proposal,base,[](const AgentEntry&){return PIXAURA_AGENT_NOT_APPROVED;});
}
int32_t pixaura_agent_inspect(uint32_t version, pixaura_document_context* context,
    const pixaura_agent_proposal* proposal, const pixaura_document_handle* base,
    uint8_t* output, uint64_t capacity, uint64_t* required) {
    return boundary(nullptr,[&] {
        agent_version(version);
        agent_ranges({{context,sizeof(*context),alignof(pixaura_document_context)},{proposal,sizeof(*proposal),alignof(pixaura_agent_proposal)},{base,sizeof(*base),alignof(pixaura_document_handle)},
            {output,capacity},{required,sizeof(*required),alignof(uint64_t)}});
        const auto h=live(context); std::lock_guard<std::mutex> guard(h.registry->mutex);
        const auto& entry=agent_current(h,proposal,base);
        const auto bytes=entry.canonical.size();
        if (!output || capacity < bytes) { *required=bytes; return PIXAURA_DOCUMENT_BUFFER_TOO_SMALL; }
        std::memcpy(output,entry.canonical.data(),bytes); *required=bytes; return PIXAURA_OK;
    });
}
static int32_t agent_terminal(uint32_t version, pixaura_document_context* context,
    const pixaura_agent_proposal* proposal, bool release) {
    return boundary(nullptr,[&] {
        agent_version(version); agent_ranges({{context,sizeof(*context),alignof(pixaura_document_context)},{proposal,sizeof(*proposal),alignof(pixaura_agent_proposal)}});
        const auto h=live(context); std::lock_guard<std::mutex> guard(h.registry->mutex);
        auto& entry=agent_entry(h,proposal);
        if (release) h.registry->agents.erase(proposal->serial);
        else if (!entry.terminal) { entry.terminal=PIXAURA_DOCUMENT_CANCELLED; entry.canonical.clear(); }
        return PIXAURA_OK;
    });
}
int32_t pixaura_agent_cancel(uint32_t version, pixaura_document_context* context, const pixaura_agent_proposal* proposal) {
    return agent_terminal(version,context,proposal,false);
}
int32_t pixaura_agent_release(uint32_t version, pixaura_document_context* context, const pixaura_agent_proposal* proposal) {
    return agent_terminal(version,context,proposal,true);
}
