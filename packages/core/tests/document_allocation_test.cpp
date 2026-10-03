// Test-only replacement allocator. This executable compiles the production
// sources independently; no failure hook or replacement allocator ships.
#include "pixaura/document.h"
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <new>
#include <string>
#include <thread>
#ifdef _MSC_VER
#include <crtdbg.h>
#endif

static thread_local int64_t fail_after = -1;
static thread_local bool pause_allocation = false;
static std::atomic<bool> serializer_pinned{false}, continue_serialization{false}, serializer_done{false};
static std::atomic<const char*> active_phase{"startup"};
static std::atomic<int> active_path{-1};
static std::atomic<int64_t> active_case{-1};
static std::atomic<uint64_t> heartbeat{0};

// Diagnostics use C stdio, never a C++ container or the injected allocator.
static void progress(const char* phase, int path = -1, int64_t index = -1) {
    active_path.store(path); active_case.store(index); active_phase.store(phase);
    heartbeat.fetch_add(1);
    std::fprintf(stderr, "allocation phase=%s path=%d fail_after=%lld\n", phase, path, static_cast<long long>(index));
    std::fflush(stderr);
}
[[noreturn]] static void failure(const char* message, int line) {
    fail_after = -1;
    std::fprintf(stderr, "allocation FAIL phase=%s path=%d fail_after=%lld line=%d: %s\n",
        active_phase.load(), active_path.load(), static_cast<long long>(active_case.load()), line, message);
    std::fflush(stderr);
    // Do not invoke MSVC abort/report dialogs or wait for teardown on failure.
    std::_Exit(EXIT_FAILURE);
}
#define CHECK(e) do { if (!(e)) failure(#e, __LINE__); } while (false)

static void wait_for(const std::atomic<bool>& flag, const char* message) {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!flag.load()) {
        if (std::chrono::steady_clock::now() > deadline) failure(message, __LINE__);
        std::this_thread::yield();
    }
}

void* operator new(std::size_t size) {
    if (pause_allocation) {
        pause_allocation = false;
        serializer_pinned.store(true);
        wait_for(continue_serialization, "paused serializer was not resumed within 5 seconds");
    }
    if (fail_after == 0) {
        std::fprintf(stderr, "allocation injecting bad_alloc phase=%s path=%d fail_after=%lld bytes=%zu\n",
            active_phase.load(), active_path.load(), static_cast<long long>(active_case.load()), size);
        std::fflush(stderr);
        throw std::bad_alloc();
    }
    if (fail_after > 0) --fail_after;
    if (void* p = std::malloc(size == 0 ? 1 : size)) return p;
    throw std::bad_alloc();
}
void* operator new[](std::size_t size) { return ::operator new(size); }
void operator delete(void* pointer) noexcept { std::free(pointer); }
void operator delete[](void* pointer) noexcept { std::free(pointer); }
void operator delete(void* pointer, std::size_t) noexcept { std::free(pointer); }
void operator delete[](void* pointer, std::size_t) noexcept { std::free(pointer); }
static const uint8_t context_id[] = "000000000000000000000000000000a2";
static const uint8_t session[] = "00000000000000000000000000000500";
static const uint8_t undo[] = "{\"command_version\":1,\"kind\":\"undo\",\"expected_session_id\":\"00000000000000000000000000000500\",\"expected_generation\":0,\"expected_revision_id\":\"00000000000000000000000000000102\"}";

int main(int argc, char** argv) {
#ifdef _MSC_VER
    _set_error_mode(_OUT_TO_STDERR);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#ifdef _DEBUG
    for (const int kind : {_CRT_WARN, _CRT_ERROR, _CRT_ASSERT}) {
        _CrtSetReportMode(kind, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(kind, _CRTDBG_FILE_STDERR);
    }
#endif
#endif
    std::set_terminate([] { failure("std::terminate: unhandled exception or exception escaped noexcept", __LINE__); });
    std::atomic<bool> finished{false};
    std::thread watchdog([&] {
        const auto started = std::chrono::steady_clock::now();
        auto last_progress = started;
        auto observed = heartbeat.load();
        while (!finished.load()) {
            const auto now = std::chrono::steady_clock::now();
            const auto current = heartbeat.load();
            if (current != observed) { observed = current; last_progress = now; }
            if (now - last_progress > std::chrono::seconds(15)) failure("no scenario progress for 15 seconds", __LINE__);
            if (now - started > std::chrono::seconds(90)) failure("allocation test exceeded internal 90-second bound", __LINE__);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    progress("fixture.read");
    CHECK(argc == 2); std::ifstream file(argv[1], std::ios::binary);
    const std::string golden((std::istreambuf_iterator<char>(file)), {}); CHECK(!golden.empty());
    const auto* input = reinterpret_cast<const uint8_t*>(golden.data());
    pixaura_document_context context{};
    progress("context.init.injected", -1, 0);
    fail_after = 0;
    CHECK(pixaura_document_context_init(1, &context, sizeof(context), context_id, 32, nullptr) == PIXAURA_DOCUMENT_RESOURCE_LIMIT);
    fail_after = -1;
    pixaura_document_context zero{}; CHECK(std::memcmp(&context, &zero, sizeof(context)) == 0);
    progress("context.init.normal");
    CHECK(pixaura_document_context_init(1, &context, sizeof(context), context_id, 32, nullptr) == 0);
    pixaura_document_handle stable{};
    // Warm immutable registry constants before measuring each allocation site.
    progress("fixture.open.warm");
    CHECK(pixaura_document_open(1, &context, input, golden.size(), session, 32, &stable, nullptr) == 0);
    pixaura_document_handle sentinel{}; sentinel.serial = 999; sentinel.api_version = 87;
    const auto sweep = [&](int path) {
        int failures = 0;
        for (int64_t n = 0; n < 10000; ++n) {
            auto output = sentinel; uint64_t required = 777;
            uint8_t buffer[8192]; std::memset(buffer, 0x5a, sizeof(buffer));
            progress("sweep.call", path, n);
            fail_after = n; int32_t status;
            if (path == 0) status = pixaura_document_open(1, &context, input, golden.size(), session, 32, &output, nullptr);
            else if (path == 1) status = pixaura_document_serialize(&context, &stable, buffer, sizeof(buffer), &required, nullptr);
            else status = pixaura_document_apply(1, &context, &stable, undo, sizeof(undo) - 1, &output, nullptr);
            fail_after = -1;
            std::fprintf(stderr, "allocation returned path=%d fail_after=%lld status=%d\n", path, static_cast<long long>(n), status);
            std::fflush(stderr);
            progress("sweep.verify", path, n);
            if (status == 0) {
                progress(path == 1 ? "sweep.canonical" : "sweep.release", path, n);
                if (path != 1) CHECK(pixaura_document_release(&context, &output) == 0);
                else CHECK(required == golden.size() && std::memcmp(buffer, golden.data(), golden.size()) == 0);
                CHECK(failures > 20); std::printf("Allocation path %d: %d injected failures before success\n", path, failures); return;
            }
            CHECK(status == PIXAURA_DOCUMENT_RESOURCE_LIMIT);
            CHECK(std::memcmp(&output, &sentinel, sizeof(output)) == 0 && required == 777);
            for (const auto byte : buffer) CHECK(byte == 0x5a);
            ++failures;
            progress("sweep.prior_snapshot", path, n);
            uint64_t size = 0; CHECK(pixaura_document_serialize(&context, &stable, nullptr, 0, &size, nullptr) == 0 && size == golden.size());
        }
        failure("allocation sweep exhausted all 10000 indices without success", __LINE__);
    };
    sweep(0); sweep(1); sweep(2);
    // The first serialization allocation occurs after acquire pins the snapshot.
    // Hold that call while another thread releases its registry ownership.
    progress("concurrency.start");
    std::thread serializer([&] {
        uint8_t buffer[8192]; uint64_t required = 0;
        pause_allocation = true;
        CHECK(pixaura_document_serialize(&context, &stable, buffer, sizeof(buffer), &required, nullptr) == 0);
        CHECK(required == golden.size() && std::memcmp(buffer, golden.data(), golden.size()) == 0);
        serializer_done.store(true);
    });
    progress("concurrency.wait_pinned");
    wait_for(serializer_pinned, "serializer did not pin snapshot within 5 seconds");
    progress("concurrency.release");
    CHECK(pixaura_document_release(&context, &stable) == 0);
    progress("concurrency.resume");
    continue_serialization.store(true);
    wait_for(serializer_done, "serializer did not complete within 5 seconds after release");
    progress("concurrency.join");
    serializer.join();
    progress("ownership.stale_release");
    CHECK(pixaura_document_release(&context, &stable) == PIXAURA_DOCUMENT_INVALID_HANDLE);
    progress("context.destroy");
    CHECK(pixaura_document_context_destroy(&context) == 0);
    progress("ownership.destroyed_release");
    CHECK(pixaura_document_release(&context, &stable) == PIXAURA_DOCUMENT_INVALID_HANDLE);
    CHECK(pixaura_document_context_destroy(&context) == PIXAURA_DOCUMENT_INVALID_HANDLE);
    progress("watchdog.join");
    finished.store(true); watchdog.join();
    std::puts("Allocation failures preserve handles, prior snapshots and caller buffers PASS");
    return 0;
}
