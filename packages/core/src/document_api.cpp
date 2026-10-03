#include "pixaura/document.h"
#include "document.hpp"

#include <cstring>
#include <limits>
#include <map>
#include <mutex>
#include <new>
#include <utility>

namespace {
using namespace pixaura::document;
constexpr uint64_t live_tag = 0x504958444f434331ULL;
constexpr uint64_t dead_tag = 0x504958444f434430ULL;
constexpr std::size_t max_handles = 64;
struct Registry {
    std::mutex mutex;
    std::map<uint64_t, Snapshot> snapshots;
    uint64_t next = 1;
};
struct Header { uint64_t tag; Registry* registry; uint8_t identity[32]; };
static_assert(sizeof(Header) <= sizeof(pixaura_document_context::opaque), "context storage");
static_assert(sizeof(pixaura_document_error) == 184 && offsetof(pixaura_document_error, message) == 24, "error layout");
static_assert(alignof(pixaura_document_error) == 4, "error alignment");
static_assert(sizeof(pixaura_document_handle) == 56 && offsetof(pixaura_document_handle, serial) == 48, "handle layout");
static_assert(sizeof(pixaura_document_context) == 64, "context layout");

bool valid_error(const pixaura_document_error* error) noexcept {
    return error == nullptr || (error->api_version == 1 && error->struct_size == sizeof(*error) && error->reserved == 0);
}
int32_t report(pixaura_document_error* error, int32_t code, uint32_t index = UINT32_MAX) noexcept {
    if (error) {
        pixaura_document_error result{}; result.api_version = 1; result.struct_size = sizeof(result); result.code = code; result.field_index = index;
        if (code != 0) {
            static const char* const names[] = {"", "INVALID_ARGUMENT", "UNSUPPORTED_ABI", "INVALID_HANDLE", "UNSUPPORTED_SCHEMA", "UNSUPPORTED_OPERATION", "INVALID_PROJECT", "INVALID_PARAMETERS", "RESOURCE_LIMIT", "STALE_BASE", "NO_HISTORY", "BUFFER_TOO_SMALL", "IO_ERROR", "CANCELLED", "INTERNAL_ERROR"};
            const char* name = code >= 0 && code <= 14 ? names[code] : "INTERNAL_ERROR";
            result.message_bytes = static_cast<uint32_t>(std::strlen(name)); std::memcpy(result.message, name, result.message_bytes);
        }
        *error = result;
    }
    return code;
}
template<class F> int32_t boundary(pixaura_document_error* error, F&& function) noexcept {
    if (!valid_error(error)) return PIXAURA_INVALID_ARGUMENT;
    try { return report(error, function()); }
    catch (const Failure& failure) { return report(error, failure.code, failure.index); }
    catch (const std::bad_alloc&) { return report(error, PIXAURA_DOCUMENT_RESOURCE_LIMIT); }
    catch (...) { return report(error, PIXAURA_DOCUMENT_INTERNAL_ERROR); }
}
void require(bool value, int32_t code = PIXAURA_INVALID_ARGUMENT) { if (!value) throw Failure{code}; }
Header read(pixaura_document_context* context) {
    require(context != nullptr); Header result{}; std::memcpy(&result, context->opaque, sizeof(result)); return result;
}
Header live(pixaura_document_context* context) {
    auto header = read(context);
    require(context->api_version == 1 && context->struct_size == sizeof(*context) && context->reserved == 0 &&
        header.tag == live_tag && header.registry != nullptr, PIXAURA_DOCUMENT_INVALID_HANDLE); return header;
}
void write(pixaura_document_context* context, const Header& header) {
    std::memset(context, 0, sizeof(*context)); context->api_version = 1; context->struct_size = sizeof(*context);
    std::memcpy(context->opaque, &header, sizeof(header));
}
void identity(const uint8_t* input, uint64_t bytes) {
    require(input != nullptr && bytes == 32);
    for (std::size_t i = 0; i < 32; ++i) require((input[i] >= '0' && input[i] <= '9') || (input[i] >= 'a' && input[i] <= 'f'));
}
std::string_view input_view(const uint8_t* input, uint64_t bytes, std::size_t limit) {
    require(bytes <= limit && bytes <= SIZE_MAX, PIXAURA_DOCUMENT_RESOURCE_LIMIT);
    require(input != nullptr && bytes != 0); return {reinterpret_cast<const char*>(input), static_cast<std::size_t>(bytes)};
}
void handle_valid(const Header& header, const pixaura_document_handle* handle) {
    require(handle != nullptr);
    require(handle->api_version == 1 && handle->struct_size == sizeof(*handle) && handle->reserved == 0 && handle->serial != 0 &&
        std::memcmp(header.identity, handle->context_id, 32) == 0, PIXAURA_DOCUMENT_INVALID_HANDLE);
}
Snapshot acquire(const Header& header, const pixaura_document_handle* handle) {
    handle_valid(header, handle); std::lock_guard<std::mutex> guard(header.registry->mutex);
    const auto it = header.registry->snapshots.find(handle->serial);
    require(it != header.registry->snapshots.end(), PIXAURA_DOCUMENT_INVALID_HANDLE); return it->second;
}
pixaura_document_handle insert(const Header& header, Snapshot snapshot) {
    std::lock_guard<std::mutex> guard(header.registry->mutex);
    auto& registry = *header.registry;
    require(registry.snapshots.size() < max_handles && registry.next != UINT64_MAX, PIXAURA_DOCUMENT_RESOURCE_LIMIT);
    const auto serial = registry.next;
    registry.snapshots.emplace(serial, std::move(snapshot)); ++registry.next;
    pixaura_document_handle result{}; result.api_version = 1; result.struct_size = sizeof(result); result.serial = serial;
    std::memcpy(result.context_id, header.identity, 32); return result;
}
Snapshot unwrap(Result<Snapshot> result) {
    if (result.code != 0) throw Failure{result.code, result.index};
    return std::move(result.value);
}
int32_t open_impl(bool create, uint32_t version, pixaura_document_context* context,
    const uint8_t* manifest, uint64_t bytes, const uint8_t* session, uint64_t session_bytes,
    pixaura_document_handle* output, pixaura_document_error* error) noexcept {
    return boundary(error, [&] {
        require(version == 1, PIXAURA_UNSUPPORTED_ABI); require(output != nullptr); identity(session, session_bytes);
        const auto header = live(context); const auto input = input_view(manifest, bytes, manifest_limit);
        const std::string_view session_view(reinterpret_cast<const char*>(session), 32);
        auto snapshot = unwrap(create ? create_document(input, session_view) : deserialize(input, session_view));
        const auto handle = insert(header, std::move(snapshot)); *output = handle; return PIXAURA_OK;
    });
}
}

int32_t pixaura_document_context_init(uint32_t version, pixaura_document_context* context,
    uint32_t context_bytes, const uint8_t* context_id, uint64_t identity_bytes, pixaura_document_error* error) {
    return boundary(error, [&] {
        require(version == 1, PIXAURA_UNSUPPORTED_ABI);
        require(context != nullptr && context_bytes >= sizeof(*context)); identity(context_id, identity_bytes);
        const auto previous = read(context);
        require(previous.tag == 0 || previous.tag == dead_tag);
        require(context->reserved == 0 && ((previous.tag == 0 && context->api_version == 0 && context->struct_size == 0) ||
            (previous.tag == dead_tag && context->api_version == 1 && context->struct_size == sizeof(*context))));
        if (previous.tag == 0) { const pixaura_document_context zero{}; require(std::memcmp(context, &zero, sizeof(zero)) == 0); }
        if (previous.tag == dead_tag) require(std::memcmp(previous.identity, context_id, 32) != 0);
        Header header{}; header.tag = live_tag; std::memcpy(header.identity, context_id, 32);
        auto registry = std::make_unique<Registry>(); header.registry = registry.get(); write(context, header); registry.release(); return PIXAURA_OK;
    });
}
int32_t pixaura_document_context_destroy(pixaura_document_context* context) {
    return boundary(nullptr, [&] {
        auto header = live(context); delete header.registry; header.registry = nullptr; header.tag = dead_tag; write(context, header); return PIXAURA_OK;
    });
}
int32_t pixaura_document_open(uint32_t version, pixaura_document_context* context,
    const uint8_t* manifest, uint64_t bytes, const uint8_t* session, uint64_t session_bytes,
    pixaura_document_handle* output, pixaura_document_error* error) {
    return open_impl(false, version, context, manifest, bytes, session, session_bytes, output, error);
}
int32_t pixaura_document_create(uint32_t version, pixaura_document_context* context,
    const uint8_t* manifest, uint64_t bytes, const uint8_t* session, uint64_t session_bytes,
    pixaura_document_handle* output, pixaura_document_error* error) {
    return open_impl(true, version, context, manifest, bytes, session, session_bytes, output, error);
}
int32_t pixaura_document_apply(uint32_t version, pixaura_document_context* context,
    const pixaura_document_handle* handle, const uint8_t* command, uint64_t bytes,
    pixaura_document_handle* output, pixaura_document_error* error) {
    return boundary(error, [&] {
        require(version == 1, PIXAURA_UNSUPPORTED_ABI); require(output != nullptr);
        const auto header = live(context); const auto snapshot = acquire(header, handle);
        auto next = unwrap(transition(*snapshot, input_view(command, bytes, command_limit)));
        *output = insert(header, std::move(next)); return PIXAURA_OK;
    });
}
int32_t pixaura_document_serialize(pixaura_document_context* context, const pixaura_document_handle* handle,
    uint8_t* output, uint64_t capacity, uint64_t* required, pixaura_document_error* error) {
    return boundary(error, [&] {
        require(required != nullptr && (output != nullptr || capacity == 0) && capacity <= SIZE_MAX);
        const auto snapshot = acquire(live(context), handle); auto result = serialize(*snapshot);
        if (result.code != 0) throw Failure{result.code, result.index};
        const auto size = static_cast<uint64_t>(result.value.size());
        if (output == nullptr) { *required = size; return PIXAURA_OK; }
        if (capacity < size) { *required = size; return PIXAURA_DOCUMENT_BUFFER_TOO_SMALL; }
        std::memcpy(output, result.value.data(), result.value.size()); *required = size; return PIXAURA_OK;
    });
}
int32_t pixaura_document_release(pixaura_document_context* context, const pixaura_document_handle* handle) {
    return boundary(nullptr, [&] {
        const auto header = live(context); handle_valid(header, handle);
        std::lock_guard<std::mutex> guard(header.registry->mutex);
        require(header.registry->snapshots.erase(handle->serial) == 1, PIXAURA_DOCUMENT_INVALID_HANDLE); return PIXAURA_OK;
    });
}
