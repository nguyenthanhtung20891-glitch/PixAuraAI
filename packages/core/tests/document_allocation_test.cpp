// Test-only replacement allocator. This executable compiles the production
// sources independently; no failure hook or replacement allocator ships.
#include "pixaura/document.h"
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <new>
#include <string>
#include <thread>

static thread_local int64_t fail_after = -1;
static thread_local bool pause_allocation = false;
static std::atomic<bool> serializer_pinned{false}, continue_serialization{false};
void* operator new(std::size_t size) {
    if (pause_allocation) {
        pause_allocation = false;
        serializer_pinned.store(true);
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (!continue_serialization.load()) {
            if (std::chrono::steady_clock::now() > deadline) std::abort();
            std::this_thread::yield();
        }
    }
    if (fail_after == 0) throw std::bad_alloc();
    if (fail_after > 0) --fail_after;
    if (void* p = std::malloc(size == 0 ? 1 : size)) return p;
    throw std::bad_alloc();
}
void* operator new[](std::size_t size) { return ::operator new(size); }
void operator delete(void* pointer) noexcept { std::free(pointer); }
void operator delete[](void* pointer) noexcept { std::free(pointer); }
void operator delete(void* pointer, std::size_t) noexcept { std::free(pointer); }
void operator delete[](void* pointer, std::size_t) noexcept { std::free(pointer); }
#define CHECK(e) do { if (!(e)) { std::fprintf(stderr, "allocation line %d: %s\n", __LINE__, #e); std::abort(); } } while (false)
static const uint8_t context_id[] = "000000000000000000000000000000a2";
static const uint8_t session[] = "00000000000000000000000000000500";
static const uint8_t undo[] = "{\"command_version\":1,\"kind\":\"undo\",\"expected_session_id\":\"00000000000000000000000000000500\",\"expected_generation\":0,\"expected_revision_id\":\"00000000000000000000000000000102\"}";

int main(int argc, char** argv) {
    CHECK(argc == 2); std::ifstream file(argv[1], std::ios::binary);
    const std::string golden((std::istreambuf_iterator<char>(file)), {}); CHECK(!golden.empty());
    const auto* input = reinterpret_cast<const uint8_t*>(golden.data());
    pixaura_document_context context{};
    fail_after = 0;
    CHECK(pixaura_document_context_init(1, &context, sizeof(context), context_id, 32, nullptr) == PIXAURA_DOCUMENT_RESOURCE_LIMIT);
    fail_after = -1;
    pixaura_document_context zero{}; CHECK(std::memcmp(&context, &zero, sizeof(context)) == 0);
    CHECK(pixaura_document_context_init(1, &context, sizeof(context), context_id, 32, nullptr) == 0);
    pixaura_document_handle stable{};
    // Warm immutable registry constants before measuring each allocation site.
    CHECK(pixaura_document_open(1, &context, input, golden.size(), session, 32, &stable, nullptr) == 0);
    pixaura_document_handle sentinel{}; sentinel.serial = 999; sentinel.api_version = 87;
    const auto sweep = [&](int path) {
        int failures = 0;
        for (int64_t n = 0; n < 10000; ++n) {
            auto output = sentinel; uint64_t required = 777;
            uint8_t buffer[8192]; std::memset(buffer, 0x5a, sizeof(buffer));
            fail_after = n; int32_t status;
            if (path == 0) status = pixaura_document_open(1, &context, input, golden.size(), session, 32, &output, nullptr);
            else if (path == 1) status = pixaura_document_serialize(&context, &stable, buffer, sizeof(buffer), &required, nullptr);
            else status = pixaura_document_apply(1, &context, &stable, undo, sizeof(undo) - 1, &output, nullptr);
            fail_after = -1;
            if (status == 0) {
                if (path != 1) CHECK(pixaura_document_release(&context, &output) == 0);
                else CHECK(required == golden.size() && std::memcmp(buffer, golden.data(), golden.size()) == 0);
                CHECK(failures > 20); std::printf("Allocation path %d: %d injected failures before success\n", path, failures); return;
            }
            CHECK(status == PIXAURA_DOCUMENT_RESOURCE_LIMIT);
            CHECK(std::memcmp(&output, &sentinel, sizeof(output)) == 0 && required == 777);
            for (const auto byte : buffer) CHECK(byte == 0x5a);
            ++failures;
            uint64_t size = 0; CHECK(pixaura_document_serialize(&context, &stable, nullptr, 0, &size, nullptr) == 0 && size == golden.size());
        }
        CHECK(false);
    };
    sweep(0); sweep(1); sweep(2);
    // The first serialization allocation occurs after acquire pins the snapshot.
    // Hold that call while another thread releases its registry ownership.
    std::thread serializer([&] {
        uint8_t buffer[8192]; uint64_t required = 0;
        pause_allocation = true;
        CHECK(pixaura_document_serialize(&context, &stable, buffer, sizeof(buffer), &required, nullptr) == 0);
        CHECK(required == golden.size() && std::memcmp(buffer, golden.data(), golden.size()) == 0);
    });
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!serializer_pinned.load()) { CHECK(std::chrono::steady_clock::now() <= deadline); std::this_thread::yield(); }
    CHECK(pixaura_document_release(&context, &stable) == 0);
    continue_serialization.store(true); serializer.join();
    CHECK(pixaura_document_release(&context, &stable) == PIXAURA_DOCUMENT_INVALID_HANDLE);
    CHECK(pixaura_document_context_destroy(&context) == 0);
    std::puts("Allocation failures preserve handles, prior snapshots and caller buffers PASS");
    return 0;
}
