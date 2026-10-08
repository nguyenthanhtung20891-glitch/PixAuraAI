#include "../src/manual_tools.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <cstdlib>
#include <new>
#include <algorithm>
#include <cstdio>
#include <exception>
#ifdef _MSC_VER
#include <crtdbg.h>
#endif
// Test-only one-shot allocation failure; no allocator hook ships.
static thread_local int64_t fail_after = -1;
static thread_local bool injected = false;
static int active_path = -1;
static int64_t active_index = -1;
void* operator new(std::size_t size) {
    if (fail_after == 0) { fail_after = -1; injected = true; throw std::bad_alloc(); }
    if (fail_after > 0) --fail_after;
    if (void* pointer = std::malloc(size ? size : 1)) return pointer;
    throw std::bad_alloc();
}
void* operator new[](std::size_t size) { return ::operator new(size); }
void operator delete(void* pointer) noexcept { std::free(pointer); }
void operator delete[](void* pointer) noexcept { std::free(pointer); }
void operator delete(void* pointer, std::size_t) noexcept { std::free(pointer); }
void operator delete[](void* pointer, std::size_t) noexcept { std::free(pointer); }
using namespace pixaura::document;
using namespace pixaura::manual;
namespace {
unsigned checks = 0;
#define CHECK(x) do { ++checks; if (!(x)) { std::cerr << "manual check " << __LINE__ << " failed: " << #x << '\n'; std::exit(1); } } while (false)
std::string id(const std::string& suffix) { return std::string(32 - suffix.size(), '0') + suffix; }
std::string request(const std::string& type, const std::string& params, const std::string& suffix = "91", unsigned ov = 1, unsigned pv = 1) {
    return "{\"operations\":[{\"type\":\"pixaura." + type + "\",\"parameters\":" + params +
        ",\"parameter_version\":" + std::to_string(pv) + ",\"operation_version\":" + std::to_string(ov) + ",\"id\":\"" + id(suffix) + "\"}]}";
}
std::string load(const std::string& file) { std::ifstream in(file, std::ios::binary); CHECK(in.good()); return {std::istreambuf_iterator<char>(in), {}}; }
std::unique_ptr<Gesture> gesture(Snapshot base, const char* tool = "pixaura.exposure", std::optional<std::string_view> replace = std::nullopt) {
    auto g = Gesture::begin(base, id("a1"), tool, replace); CHECK(g.code == 0); return std::move(g.value);
}
std::string bytes(Snapshot s) { auto r = serialize(*s); CHECK(r.code == 0); return r.value; }
int32_t validate(const std::string& json) {
    uint64_t size = 42;
    const auto code = pixaura_manual_canonical_operation(1, reinterpret_cast<const uint8_t*>(json.data()), json.size(), nullptr, 0, &size);
    if (code == PIXAURA_DOCUMENT_BUFFER_TOO_SMALL) { CHECK(size > 0); return 0; }
    CHECK(size == 42); return code;
}
}
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
    std::set_terminate([] {
        std::fprintf(stderr, "manual terminate path=%d allocation=%lld\n", active_path, static_cast<long long>(active_index));
        std::fflush(stderr); std::_Exit(1);
    });
    CHECK(argc == 3);
    CHECK(registry().size() == 3 && validate_descriptors(registry().data(), registry().size()) == 0);
    CHECK(find("pixaura.resize") == nullptr && find("pixaura.exposure", 2) == nullptr);
    auto table = registry(); table[1] = table[0]; CHECK(validate_descriptors(table.data(), table.size()) != 0);
    CHECK(validate_descriptors(registry().data(), 17) == PIXAURA_DOCUMENT_RESOURCE_LIMIT);
    table = registry(); table[0].parameter_count = 9; CHECK(validate_descriptors(table.data(), table.size()) != 0);
    uint64_t length = 0;
    CHECK(pixaura_manual_registry_json(1, nullptr, 0, &length) == PIXAURA_DOCUMENT_BUFFER_TOO_SMALL);
    std::string descriptors(static_cast<std::size_t>(length), 'x');
    CHECK(pixaura_manual_registry_json(1, reinterpret_cast<uint8_t*>(descriptors.data()), length, &length) == 0);
    CHECK(descriptors.back() == '\n' && descriptors.size() < 32768);
    std::ofstream(std::string(argv[2]) + "/manual-registry.json", std::ios::binary) << descriptors;
    for (int e : {-5000, 0, 5000}) CHECK(validate(request("exposure", "{\"milli_ev\":" + std::to_string(e) + "}")) == 0);
    for (const auto* bad : {"-5001", "5001", "0.1", "1e0", "-0", "\"1\"", "null", "true"}) {
        CHECK(validate(request("exposure", std::string("{\"milli_ev\":") + bad + "}")) != 0);
    }
    CHECK(validate(request("exposure", "{\"milli_ev\":0,\"milli_ev\":1}")) != 0);
    CHECK(validate(request("exposure", "{\"milli_ev\":0,\"extra\":1}")) != 0);
    CHECK(validate(request("exposure", "{\"milli_ev\":0}", "91", 2)) == PIXAURA_DOCUMENT_UNSUPPORTED_OPERATION);
    CHECK(validate(request("exposure", "{\"milli_ev\":0}", "91", 1, 2)) == PIXAURA_DOCUMENT_UNSUPPORTED_OPERATION);
    CHECK(validate(request("brightness", "{\"milli_ev\":0}")) == PIXAURA_DOCUMENT_UNSUPPORTED_OPERATION);
    for (unsigned turns = 0; turns <= 3; ++turns) CHECK(validate(request("rotate", "{\"quarter_turns\":" + std::to_string(turns) + "}")) == 0);
    CHECK(validate(request("rotate", "{\"quarter_turns\":4}")) != 0);
    CHECK(validate(request("crop", "{\"x_ppm\":999999,\"y_ppm\":999999,\"width_ppm\":1,\"height_ppm\":1}")) == 0);
    CHECK(validate(request("crop", "{\"x_ppm\":1,\"y_ppm\":0,\"width_ppm\":1000000,\"height_ppm\":1000000}")) != 0);
    CHECK(validate(request("crop", "{\"x_ppm\":0,\"y_ppm\":0,\"width_ppm\":0,\"height_ppm\":1}")) != 0);
    auto opened = deserialize(load(argv[1]), id("55")); CHECK(opened.code == 0);
    auto base = opened.value; const auto original = bytes(base);
    auto g = gesture(base);
    for (unsigned i = 0; i < 1000; ++i) CHECK(g->update(request("exposure", "{\"milli_ev\":" + std::to_string(i) + "}")).value == i + 1);
    CHECK(g->eligible(id("a1"), 1000, base) && !g->eligible(id("a1"), 999, base) && !g->eligible(id("a2"), 1000, base));
    CHECK(g->update(request("exposure", "{\"milli_ev\":5001}")).code == PIXAURA_DOCUMENT_INVALID_PARAMETERS);
    CHECK(g->eligible(id("a1"), 1000, base));
    CHECK(bytes(base) == original && g->preview(base).code == 0);
    auto committed = g->commit(base, id("201")); CHECK(committed.code == 0);
    CHECK(committed.value->revisions().size() == base->revisions().size() + 1);
    CHECK(committed.value->operations().size() == base->operations().size() + 1);
    CHECK(g->commit(base, id("202")).code == PIXAURA_DOCUMENT_CANCELLED);
    CHECK(!g->eligible(id("a1"), 1000, base));
    auto replayed = replay(*committed.value, committed.value->current()); CHECK(replayed.code == 0);
    CHECK(replayed.value[0].type == "pixaura.exposure" && replayed.value[1].type == "pixaura.rotate");
    CHECK(std::get<Exposure>(replayed.value.back().parameters).milli_ev == 999);
    CHECK(deserialize(bytes(committed.value), id("56")).code == 0);
    CHECK(bytes(deserialize(bytes(committed.value), id("56")).value) == bytes(committed.value));
    CHECK(bytes(base) == original && committed.value->source().sha256 == base->source().sha256);
    for (const auto* tool : {"pixaura.exposure", "pixaura.rotate", "pixaura.crop"}) {
        auto n = gesture(base, tool);
        std::string json = tool == std::string("pixaura.exposure") ? request("exposure", "{\"milli_ev\":0}") :
            tool == std::string("pixaura.rotate") ? request("rotate", "{\"quarter_turns\":0}") :
            request("crop", "{\"height_ppm\":1000000,\"width_ppm\":1000000,\"x_ppm\":0,\"y_ppm\":0}");
        CHECK(n->update(json).code == 0); CHECK(n->commit(base, id("203")).value == base);
    }
    auto cancelled = gesture(base); CHECK(cancelled->update(request("exposure", "{\"milli_ev\":2000}")).code == 0);
    cancelled->cancel(); CHECK(cancelled->commit(base, id("204")).code == PIXAURA_DOCUMENT_CANCELLED); CHECK(bytes(base) == original);
    auto stale = gesture(base); CHECK(stale->update(request("exposure", "{\"milli_ev\":2000}")).code == 0);
    CHECK(!stale->eligible(id("a1"), 1, committed.value)); CHECK(stale->commit(committed.value, id("205")).code == PIXAURA_DOCUMENT_STALE_BASE);
    CHECK(stale->update(request("exposure", "{\"milli_ev\":2000}")).code == PIXAURA_DOCUMENT_CANCELLED);
    const auto replace_id = id("11");
    auto replaced = gesture(base, "pixaura.exposure", replace_id);
    CHECK(replaced->update(request("exposure", "{\"milli_ev\":-1000}")).code == 0);
    auto next = replaced->commit(base, id("206")); CHECK(next.code == 0);
    auto ordered = replay(*next.value, next.value->current()); CHECK(std::get<Exposure>(ordered.value[0].parameters).milli_ev == -1000);
    CHECK(ordered.value[1].type == "pixaura.rotate"); CHECK(bytes(base) == original);
    auto removed = gesture(base, "pixaura.exposure", replace_id); CHECK(removed->update(request("exposure", "{\"milli_ev\":0}")).code == 0);
    auto without = removed->commit(base, id("207")); CHECK(without.code == 0);
    CHECK(replay(*without.value, without.value->current()).value.size() == 1 && without.value->operations().size() == base->operations().size());
    auto unchanged = gesture(base, "pixaura.exposure", replace_id); CHECK(unchanged->update(request("exposure", "{\"milli_ev\":1250}")).code == 0);
    CHECK(unchanged->commit(base, id("208")).value == base);
    CHECK(Gesture::begin(base, id("a3"), "pixaura.exposure", id("12")).code != 0);
    CHECK(Gesture::begin(base, id("a3"), "pixaura.blur").code == PIXAURA_DOCUMENT_UNSUPPORTED_OPERATION);
    CHECK(validate("{\"operations\":[]}") != 0);
    CHECK(validate(request("rotate", "{\"quarter_turns\":-1}")) != 0);
    CHECK(validate(request("exposure", "{}")) != 0);
    CHECK(validate(request("exposure", "{\"milli_ev\":2147483648}")) != 0);
    CHECK(validate(request("crop", "{\"x_ppm\":-1,\"y_ppm\":0,\"width_ppm\":1,\"height_ppm\":1}")) != 0);
    CHECK(validate(request("crop", "{\"x_ppm\":0,\"y_ppm\":999999,\"width_ppm\":1,\"height_ppm\":2}")) != 0);
    auto empty = gesture(base); CHECK(empty->commit(base, id("209")).value == base);
    auto duplicate = gesture(base);
    CHECK(duplicate->commit(base, base->current().text()).code == PIXAURA_DOCUMENT_INVALID_PROJECT);
    CHECK(duplicate->update(request("exposure", "{\"milli_ev\":1}", "11")).code == PIXAURA_DOCUMENT_INVALID_PROJECT);
    CHECK(duplicate->update(request("rotate", "{\"quarter_turns\":1}")).code == PIXAURA_DOCUMENT_INVALID_PARAMETERS);
    auto session_changed = deserialize(original, id("57")); CHECK(session_changed.code == 0);
    auto old_session = gesture(base); CHECK(old_session->update(request("exposure", "{\"milli_ev\":1}")).code == 0);
    CHECK(!old_session->eligible(id("a1"), 1, session_changed.value));
    CHECK(old_session->commit(session_changed.value, id("211")).code == PIXAURA_DOCUMENT_STALE_BASE);
    auto source_text = original;
    const auto digest_at = source_text.find(std::string(64, 'a')); CHECK(digest_at != std::string::npos);
    source_text.replace(digest_at, 64, std::string(64, 'b'));
    auto source_changed = deserialize(source_text, id("55")); CHECK(source_changed.code == 0);
    auto old_source = gesture(base); CHECK(old_source->update(request("exposure", "{\"milli_ev\":1}")).code == 0);
    CHECK(!old_source->eligible(id("a1"), 1, source_changed.value));
    CHECK(old_source->commit(source_changed.value, id("212")).code == PIXAURA_DOCUMENT_STALE_BASE);
    Vector<Id> full_stack; Vector<EditOperation> additions;
    for (const auto& op : replay(*base, base->current()).value) full_stack.push_back(op.id);
    for (unsigned i = 0; i < 254; ++i) {
        auto parsed = parse_evaluation(request("exposure", "{\"milli_ev\":1}", "9" + std::to_string(i)));
        CHECK(parsed.code == 0); full_stack.push_back(parsed.value[0].id); additions.push_back(parsed.value[0]);
    }
    DetachedCandidate filled{base->identity(), base->current(), base->session(), std::move(full_stack), std::move(additions), Id::parse(id("213")), "manual", std::nullopt};
    auto full = transition(*base, filled); CHECK(full.code == 0);
    auto at_limit = gesture(full.value);
    CHECK(at_limit->update(request("exposure", "{\"milli_ev\":1}", "8000")).code == PIXAURA_DOCUMENT_RESOURCE_LIMIT);
    CHECK(at_limit->update(request("exposure", "{\"milli_ev\":0}", "8000")).code == 0);
    CHECK(at_limit->commit(full.value, id("214")).value == full.value);
    unsigned allocation_failures = 0;
    const auto update = request("exposure", "{\"milli_ev\":2400}");
    const auto previous = request("exposure", "{\"milli_ev\":1200}");
    const auto revision = id("210"), gesture_id = id("a1");
    for (int path = 0; path < 4; ++path) {
        bool completed = false;
        for (int64_t index = 0; index < 2000; ++index) {
            active_path = path; active_index = index;
            std::fprintf(stderr, "manual fault path=%d allocation=%lld\n", path, static_cast<long long>(index));
            std::fflush(stderr);
            auto fault = gesture(base); CHECK(fault->update(previous).code == 0);
            uint64_t required = 777;
            uint8_t sentinel[32768]; std::fill(std::begin(sentinel), std::end(sentinel), uint8_t{0x5a});
            injected = false; fail_after = index;
            int32_t code = 0;
            if (path == 0) code = pixaura_manual_registry_json(1, sentinel, sizeof(sentinel), &required);
            else if (path == 1) code = fault->update(update).code;
            else if (path == 2) code = fault->commit(base, revision).code;
            else code = pixaura_manual_canonical_operation(1, reinterpret_cast<const uint8_t*>(update.data()), update.size(), sentinel, sizeof(sentinel), &required);
            fail_after = -1;
            if (!injected) { CHECK(code == 0); completed = true; break; }
            ++allocation_failures; CHECK(code == PIXAURA_DOCUMENT_RESOURCE_LIMIT);
            CHECK(bytes(base) == original);
            if (path == 0 || path == 3) {
                CHECK(required == 777 && std::all_of(std::begin(sentinel), std::end(sentinel), [](uint8_t c) { return c == 0x5a; }));
            } else {
                CHECK(fault->eligible(gesture_id, 1, base));
                if (path == 1) CHECK(fault->update(update).value == 2);
                CHECK(fault->commit(base, revision).code == 0);
            }
        }
        CHECK(completed);
    }
    CHECK(allocation_failures > 0);
    std::cout << "manual allocation failures recovered=" << allocation_failures << '\n';
    std::cout << "manual native checks=" << checks << "; coalesced updates=1000; retained base unchanged\n";
}
