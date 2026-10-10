#include "../src/document.hpp"
#include "pixaura/document.h"
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <thread>

using namespace pixaura::document;
#define CHECK(e) do { if (!(e)) { std::fprintf(stderr, "document test line %d: %s\n", __LINE__, #e); std::abort(); } } while (false)
static std::string identity(const std::string& suffix) { return std::string(32 - suffix.size(), '0') + suffix; }
static const std::string session = identity("500");
static std::string replace(std::string value, const std::string& from, const std::string& to) {
    const auto p = value.find(from); CHECK(p != std::string::npos); value.replace(p, from.size(), to); return value;
}
static std::string command(const ImageDocument& doc, const std::string& kind, const std::string& extra = "") {
    return "{\"command_version\":1,\"kind\":\"" + kind + "\",\"expected_revision_id\":\"" + doc.current().text() +
        "\",\"expected_session_id\":\"" + doc.session().id.text() + "\",\"expected_generation\":" + std::to_string(doc.session().generation) + extra + "}";
}
static Snapshot next(const Snapshot& doc, const std::string& kind, const std::string& extra = "") {
    auto result = transition(*doc, command(*doc, kind, extra)); CHECK(result.code == 0); return result.value;
}
static std::string bytes(const Snapshot& doc) { auto result = serialize(*doc); CHECK(result.code == 0); return result.value; }
static const uint8_t* input_bytes(const std::string& text) { return reinterpret_cast<const uint8_t*>(text.data()); }
static void invalid(const std::string& input, int32_t code = -1) { const auto result = deserialize(input, session); CHECK(result.code != 0); if (code != -1) CHECK(result.code == code); }
static std::string repeated(const std::string& entry, std::size_t count) {
    std::string result = "["; for (std::size_t i = 0; i < count; ++i) { if (i) result += ','; result += entry; } return result + ']';
}

int main(int argc, char** argv) {
    CHECK(argc == 2); std::ifstream file(argv[1], std::ios::binary);
    const std::string golden((std::istreambuf_iterator<char>(file)), {}); CHECK(!golden.empty());
    const auto parsed = deserialize(golden, session); CHECK(parsed.code == 0); const auto doc = parsed.value;
    CHECK(bytes(doc) == golden); CHECK(bytes(deserialize(" \t\n" + golden + "\r", session).value) == golden);
    CHECK(bytes(deserialize(replace(golden, "\"schema_version\"", "\"schema_\\u0076ersion\""), session).value) == golden);
    auto replayed = replay(*doc, doc->current()); CHECK(replayed.code == 0 && replayed.value.size() == 2);
    CHECK(replayed.value[0].type == "pixaura.exposure" && replayed.value[1].type == "pixaura.rotate");

    const std::string replacement = ",\"revision_id\":\"" + identity("103") + "\",\"actor\":\"manual\",\"plan_id\":null,\"operations\":[{\"id\":\"" + identity("13") +
        "\",\"type\":\"pixaura.exposure\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":-1000}}],\"stack\":[\"" + identity("13") + "\",\"" + identity("12") + "\"]";
    auto replaced = next(doc, "commit", replacement);
    CHECK(replaced->revisions().size() == 4 && replaced->operations().size() == 3);
    replayed = replay(*replaced, replaced->current()); CHECK(std::get<Exposure>(replayed.value[0].parameters).milli_ev == -1000);
    CHECK(std::get<Exposure>(replay(*replaced, doc->current()).value[0].parameters).milli_ev == 1250);
    CHECK(bytes(doc) == golden && replaced->source().sha256 == doc->source().sha256);
    const auto reorder_payload = ",\"revision_id\":\"" + identity("104") + "\",\"actor\":\"manual\",\"plan_id\":null,\"operations\":[],\"stack\":[\"" + identity("12") + "\",\"" + identity("11") + "\"]";
    const auto reordered = next(doc, "commit", reorder_payload);
    CHECK(replay(*reordered, reordered->current()).value[0].type == "pixaura.rotate");
    CHECK(replay(*reordered, doc->current()).value[0].type == "pixaura.exposure");
    const auto removed = next(doc, "commit", replace(reorder_payload, "\"" + identity("12") + "\",", ""));
    CHECK(replay(*removed, removed->current()).value.size() == 1 && removed->operations().size() == 2);
    auto undone = next(doc, "undo"); CHECK(undone->current().text() == identity("101") && undone->redo().size() == 1);
    CHECK(deserialize(bytes(undone), session).value->redo()[0].text() == identity("102"));
    auto root = next(undone, "undo"); CHECK(root->current().text() == identity("100"));
    CHECK(transition(*root, command(*root, "undo")).code == PIXAURA_DOCUMENT_NO_HISTORY);
    const auto root_manifest = bytes(root);
    // Initial creation requires no retained history, rather than discarding it.
    CHECK(create_document(root_manifest, session).code == PIXAURA_DOCUMENT_INVALID_PROJECT);
    std::string initial = golden;
    initial = replace(initial, "\"current_revision_id\":\"" + identity("102") + "\"", "\"current_revision_id\":\"" + identity("100") + "\"");
    const auto ops_start = initial.find("\"operations\":["); const auto ops_end = initial.find("],\"project_id\"", ops_start);
    initial.replace(ops_start, ops_end + 1 - ops_start, "\"operations\":[]");
    const auto rev_start = initial.find("\"revisions\":["); const auto rev_end = initial.find("],\"schema_version\"", rev_start);
    initial.replace(rev_start, rev_end + 1 - rev_start, "\"revisions\":[{\"actor\":\"import\",\"id\":\"" + identity("100") + "\",\"parent_id\":null,\"plan_id\":null,\"stack\":[]}]");
    auto created = create_document(initial, session); CHECK(created.code == 0 && bytes(created.value) == initial);
    const auto append_payload = replace(replacement, ",\"" + identity("12") + "\"]", "]");
    auto appended = next(created.value, "commit", append_payload); CHECK(appended->revisions().size() == 2);
    auto redone = next(undone, "redo"); CHECK(bytes(redone) == golden && redone->session().generation == 2);
    CHECK(transition(*redone, command(*doc, "undo")).code == PIXAURA_DOCUMENT_STALE_BASE);
    CHECK(transition(*doc, replace(command(*doc, "undo"), session, identity("501"))).code == PIXAURA_DOCUMENT_STALE_BASE);
    CHECK(transition(*doc, replace(command(*doc, "undo"), identity("102"), identity("101"))).code == PIXAURA_DOCUMENT_STALE_BASE);
    auto branched = next(undone, "commit", replacement); CHECK(branched->redo().empty() && branched->revisions().size() == 4);
    CHECK(replay(*branched, Id::parse(identity("102"))).value.size() == 2);
    CHECK(transition(*branched, command(*branched, "redo")).code == PIXAURA_DOCUMENT_NO_HISTORY);
    auto checked_out = next(branched, "checkout", ",\"revision_id\":\"" + identity("102") + "\"");
    CHECK(checked_out->current() == doc->current() && checked_out->redo().empty());
    CHECK(transition(*doc, replace(command(*doc, "undo"), "\"command_version\":1", "\"command_version\":2")).code == PIXAURA_UNSUPPORTED_ABI);
    CHECK(transition(*doc, command(*doc, "migrate")).code == PIXAURA_INVALID_ARGUMENT);
    auto candidate = DetachedCandidate{doc->identity(), doc->current(), doc->session(), replaced->revisions().back().stack,
        {replaced->operations().back()}, Id::parse(identity("103")), "manual", std::nullopt};
    CHECK(transition(*doc, candidate).code == 0);
    CHECK(transition(*undone, candidate).code == PIXAURA_DOCUMENT_STALE_BASE);
    CHECK(transition(*redone, candidate).code == PIXAURA_DOCUMENT_STALE_BASE);
    CHECK(transition(*deserialize(golden, identity("501")).value, candidate).code == PIXAURA_DOCUMENT_STALE_BASE);
    candidate.identity.document = Id::parse(identity("9")); CHECK(transition(*doc, candidate).code == PIXAURA_DOCUMENT_STALE_BASE);
    CHECK(transition(*doc, replace(command(*doc, "commit", replacement), "-1000", "5001")).code == PIXAURA_DOCUMENT_INVALID_PARAMETERS);
    CHECK(transition(*doc, replace(command(*doc, "commit", replacement), identity("13"), identity("11"))).code == PIXAURA_DOCUMENT_INVALID_PROJECT);
    CHECK(transition(*doc, command(*doc, "commit", replace(replacement, "\"manual\"", "\"ai\""))).code == PIXAURA_DOCUMENT_INVALID_PROJECT);
    auto ai = next(doc, "commit", replace(replace(replacement, "\"manual\"", "\"ai\""), "\"plan_id\":null", "\"plan_id\":\"" + identity("777") + "\""));
    CHECK(ai->revisions().back().plan.has_value());

    invalid(replace(golden, "\"schema_version\":1", "\"schema_version\":2"), PIXAURA_DOCUMENT_UNSUPPORTED_SCHEMA);
    for (const auto& token : {"1.0", "1e0", "01", "-0", "NaN", "Infinity", "9007199254740992", "99999999999999999999999999999", "true", "null"})
        invalid(replace(golden, "\"schema_version\":1", std::string("\"schema_version\":") + token));
    invalid(replace(golden, "pixaura.exposure", "pixaura.denoise"), PIXAURA_DOCUMENT_UNSUPPORTED_OPERATION);
    invalid(replace(golden, "\"operation_version\":1", "\"operation_version\":2"), PIXAURA_DOCUMENT_UNSUPPORTED_OPERATION);
    invalid(replace(golden, "\"parameter_version\":1", "\"parameter_version\":2"), PIXAURA_DOCUMENT_UNSUPPORTED_OPERATION);
    invalid(replace(golden, "1250", "5001"), PIXAURA_DOCUMENT_INVALID_PARAMETERS);
    invalid(replace(golden, "\"quarter_turns\":1", "\"quarter_turns\":4"), PIXAURA_DOCUMENT_INVALID_PARAMETERS);
    invalid(replace(golden, "\"milli_ev\":1250", "\"quarter_turns\":1"), PIXAURA_DOCUMENT_INVALID_PARAMETERS);
    invalid(replace(golden, "\"milli_ev\":1250", "\"milli_ev\":\"1250\""), PIXAURA_DOCUMENT_INVALID_PARAMETERS);
    invalid(replace(golden, "\"milli_ev\":1250", "\"milli_ev\":1250,\"width_ppm\":1"), PIXAURA_DOCUMENT_INVALID_PARAMETERS);
    const auto crop = replace(replace(golden, "pixaura.exposure", "pixaura.crop"), "\"milli_ev\":1250", "\"height_ppm\":1000000,\"width_ppm\":1000000,\"x_ppm\":0,\"y_ppm\":0");
    CHECK(deserialize(crop, session).code == 0);
    invalid(replace(crop, "\"x_ppm\":0", "\"x_ppm\":1"), PIXAURA_DOCUMENT_INVALID_PARAMETERS);
    invalid(replace(golden, "\"width\":3", "\"width\":0"));
    invalid(replace(replace(golden, "\"width\":3", "\"width\":65535"), "\"height\":2", "\"height\":65535"), PIXAURA_DOCUMENT_RESOURCE_LIMIT);
    invalid(replace(golden, "\"byte_length\":128", "\"byte_length\":8589934593"));
    invalid(replace(golden, "\"orientation\":1", "\"orientation\":9"));
    invalid(replace(golden, "\"png\"", "\"jpeg\""));
    invalid(replace(golden, "\"source\":{", "\"source\":null,\"unused\":{") );
    invalid(replace(golden, "\"schema_version\":1", "\"schema_version\":1,\"schema_\\u0076ersion\":1"));
    invalid(replace(golden, "\"current_revision_id\":\"" + identity("102") + "\"", "\"current_revision_id\":\"" + identity("999") + "\""));
    invalid(replace(golden, "\"parent_id\":\"" + identity("100") + "\"", "\"parent_id\":\"" + identity("102") + "\""));
    invalid(replace(golden, "\"parent_id\":\"" + identity("100") + "\"", "\"parent_id\":null"));
    invalid(replace(golden, "\"id\":\"" + identity("101") + "\"", "\"id\":\"" + identity("100") + "\""));
    invalid(replace(golden, "\"id\":\"" + identity("12") + "\"", "\"id\":\"" + identity("11") + "\""));
    invalid(replace(golden, "\"stack\":[\"" + identity("11") + "\"]", "\"stack\":[\"" + identity("999") + "\"]"));
    invalid(replace(golden, "\"stack\":[\"" + identity("11") + "\"]", "\"stack\":[\"" + identity("11") + "\",\"" + identity("11") + "\"]"));
    invalid(replace(golden, "\"redo\":[]", "\"redo\":[\"" + identity("100") + "\"]"));
    invalid(replace(golden, "\"id\":\"" + identity("12") + "\"", "\"id\":\"" + identity("99") + "\""));
    invalid(golden + "null"); invalid(std::string("\xef\xbb\xbf") + golden);
    for (const auto& bad : {std::string("\xc0\xaf"), std::string("\xed\xa0\x80"), std::string("\xf4\x90\x80\x80"), std::string("\x80"), std::string("\\uD800"), std::string("\\uDC00"), std::string("\\uD800\\u0041"), std::string("\n")})
        invalid(replace(golden, "pixaura.exposure", bad));
    // Decoding string escapes precedes duplicate/domain validation.
    invalid(replace(golden, "pixaura.exposure", "\\uD83D\\uDE00"));
    invalid(replace(golden, "pixaura.exposure", std::string(257, 'a')), PIXAURA_DOCUMENT_RESOURCE_LIMIT);
    invalid(replace(golden, "\"redo\":[]", "\"redo\":" + repeated("\"" + identity("102") + "\"", 4096)), PIXAURA_DOCUMENT_RESOURCE_LIMIT);
    invalid(replace(golden, "\"stack\":[]", "\"stack\":" + repeated("\"" + identity("11") + "\"", 257)), PIXAURA_DOCUMENT_RESOURCE_LIMIT);
    invalid(replace(golden, "\"operations\":[", "\"operations\":" + repeated("null", 4097) + ",\"revisions\":["), PIXAURA_DOCUMENT_RESOURCE_LIMIT);
    invalid(replace(golden, "\"revisions\":[", "\"revisions\":" + repeated("null", 4097) + ",\"operations\":["), PIXAURA_DOCUMENT_RESOURCE_LIMIT);
    const auto operation_entry = [](const std::string& op_id) {
        return "{\"id\":\"" + op_id + "\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":0},\"type\":\"pixaura.exposure\"}";
    };
    invalid(replace(golden, "],\"project_id\"", "," + operation_entry(identity("13")) + "],\"project_id\"")); // valid but unreferenced operation
    const std::string root_record = "{\"actor\":\"import\",\"id\":\"" + identity("100") + "\",\"parent_id\":null,\"plan_id\":null,\"stack\":[]}";
    std::string many_revisions = "[" + root_record; std::string parent = identity("100");
    for (int i = 0; i < 4095; ++i) {
        const auto rev_id = identity(std::to_string(10000 + i));
        many_revisions += ",{\"actor\":\"manual\",\"id\":\"" + rev_id + "\",\"parent_id\":\"" + parent + "\",\"plan_id\":null,\"stack\":[]}"; parent = rev_id;
    }
    many_revisions += ']';
    const auto revision_limit_fixture = replace(initial, "[" + root_record + "]", many_revisions);
    const auto max_revisions = deserialize(revision_limit_fixture, session); CHECK(max_revisions.code == 0 && max_revisions.value->revisions().size() == 4096);
    CHECK(transition(*max_revisions.value, command(*max_revisions.value, "commit", ",\"revision_id\":\"" + identity("99999") + "\",\"operations\":[],\"stack\":[],\"actor\":\"manual\",\"plan_id\":null")).code == PIXAURA_DOCUMENT_RESOURCE_LIMIT);
    std::string all_operations = "[", chunk_revisions = "[" + root_record;
    parent = identity("100");
    for (int chunk = 0; chunk < 16; ++chunk) {
        std::string ordered = "[";
        for (int offset = 0; offset < 256; ++offset) {
            const auto op_id = identity(std::to_string(10000 + chunk * 256 + offset));
            if (chunk || offset) all_operations += ',';
            all_operations += operation_entry(op_id);
            if (offset) ordered += ',';
            ordered += "\"" + op_id + "\"";
        }
        ordered += ']'; const auto rev_id = identity(std::to_string(20000 + chunk));
        chunk_revisions += ",{\"actor\":\"manual\",\"id\":\"" + rev_id + "\",\"parent_id\":\"" + parent + "\",\"plan_id\":null,\"stack\":" + ordered + "}"; parent = rev_id;
    }
    all_operations += ']'; chunk_revisions += ']';
    auto count_fixture = replace(replace(initial, "\"operations\":[]", "\"operations\":" + all_operations), "[" + root_record + "]", chunk_revisions);
    count_fixture = replace(count_fixture, "\"current_revision_id\":\"" + identity("100") + "\"", "\"current_revision_id\":\"" + parent + "\"");
    auto max_operations = deserialize(count_fixture, session); CHECK(max_operations.code == 0 && max_operations.value->operations().size() == 4096);
    CHECK(replay(*max_operations.value, max_operations.value->current()).value.size() == 256);
    CHECK(bytes(max_operations.value) == count_fixture);
    invalid(std::string(manifest_limit + 1, ' '), PIXAURA_DOCUMENT_RESOURCE_LIMIT);
    CHECK(deserialize(golden + std::string(manifest_limit - golden.size(), ' '), session).code == 0);
    for (std::size_t n = 0; n < golden.size() - 1; ++n) invalid(golden.substr(0, n));
    for (std::size_t depth : {2u, 16u, 17u, 100000u}) invalid(std::string(depth, '[') + "0" + std::string(depth, ']'));
    for (std::size_t p = 0; p < golden.size(); p += 13) {
        for (char byte : {'\0', '{', ']', '"', '\\', '0', static_cast<char>(0xff)}) {
            auto mutated = golden; mutated[p] = byte; const auto result = deserialize(mutated, session);
            if (result.code == 0) { const auto canonical = bytes(result.value); CHECK(bytes(deserialize(canonical, session).value) == canonical); }
        }
    }
    CHECK(bytes(doc) == golden);

    pixaura_document_context a{}, b{}, uninitialized{};
    CHECK(pixaura_document_context_destroy(nullptr) == PIXAURA_INVALID_ARGUMENT);
    CHECK(pixaura_document_context_destroy(&uninitialized) == PIXAURA_DOCUMENT_INVALID_HANDLE);
    const auto context_a = identity("a"), context_b = identity("b"), context_c = identity("c");
    CHECK(pixaura_document_context_init(2, &a, sizeof(a), input_bytes(context_a), 32, nullptr) == PIXAURA_UNSUPPORTED_ABI);
    CHECK(pixaura_document_context_init(1, nullptr, sizeof(a), input_bytes(context_a), 32, nullptr) == PIXAURA_INVALID_ARGUMENT);
    CHECK(pixaura_document_context_init(1, &a, sizeof(a) - 1, input_bytes(context_a), 32, nullptr) == PIXAURA_INVALID_ARGUMENT);
    CHECK(pixaura_document_context_init(1, &a, sizeof(a), nullptr, 32, nullptr) == PIXAURA_INVALID_ARGUMENT);
    CHECK(pixaura_document_context_init(1, &a, sizeof(a), input_bytes(context_a), UINT64_MAX, nullptr) == PIXAURA_INVALID_ARGUMENT);
    CHECK(pixaura_document_context_init(1, &a, sizeof(a), input_bytes(context_a), 32, nullptr) == 0);
    CHECK(pixaura_document_context_init(1, &b, sizeof(b), input_bytes(context_b), 32, nullptr) == 0);
    CHECK(pixaura_document_context_init(1, &a, sizeof(a), input_bytes(context_a), 32, nullptr) == PIXAURA_INVALID_ARGUMENT);
    pixaura_document_handle h{}; CHECK(pixaura_document_open(1, &a, input_bytes(golden), golden.size(), input_bytes(session), 32, &h, nullptr) == 0);
    uint64_t required = 987; uint8_t small[2] = {17, 18};
    CHECK(pixaura_document_serialize(&b, &h, small, 2, &required, nullptr) == PIXAURA_DOCUMENT_INVALID_HANDLE && required == 987 && small[0] == 17);
    CHECK(pixaura_document_release(&b, &h) == PIXAURA_DOCUMENT_INVALID_HANDLE);
    CHECK(pixaura_document_serialize(&a, &h, nullptr, 0, &required, nullptr) == 0 && required == golden.size());
    CHECK(pixaura_document_serialize(&a, &h, small, 2, &required, nullptr) == PIXAURA_DOCUMENT_BUFFER_TOO_SMALL && small[0] == 17 && small[1] == 18);
    std::string serialized(required, '\0'); CHECK(pixaura_document_serialize(&a, &h, reinterpret_cast<uint8_t*>(serialized.data()), serialized.size(), &required, nullptr) == 0 && serialized == golden);
    const auto sentinel = h; pixaura_document_error error{}; error.api_version = 1; error.struct_size = sizeof(error) - 1;
    CHECK(pixaura_document_open(1, &a, input_bytes(golden), golden.size(), input_bytes(session), 32, &h, &error) == PIXAURA_INVALID_ARGUMENT && std::memcmp(&h, &sentinel, sizeof(h)) == 0);
    error.struct_size = sizeof(error); error.reserved = 1;
    CHECK(pixaura_document_serialize(&a, &h, small, 2, &required, &error) == PIXAURA_INVALID_ARGUMENT);
    error.reserved = 0;
    CHECK(pixaura_document_open(2, &a, input_bytes(golden), golden.size(), input_bytes(session), 32, &h, &error) == PIXAURA_UNSUPPORTED_ABI && error.code == PIXAURA_UNSUPPORTED_ABI && error.message_bytes > 0);
    CHECK(std::memcmp(&h, &sentinel, sizeof(h)) == 0);
    CHECK(pixaura_document_open(1, &a, nullptr, 10, input_bytes(session), 32, &h, nullptr) == PIXAURA_INVALID_ARGUMENT);
    CHECK(pixaura_document_open(1, &a, input_bytes(golden), UINT64_MAX, input_bytes(session), 32, &h, nullptr) == PIXAURA_DOCUMENT_RESOURCE_LIMIT);
    CHECK(pixaura_document_open(1, &a, input_bytes(golden), golden.size(), nullptr, 32, &h, nullptr) == PIXAURA_INVALID_ARGUMENT);
    CHECK(pixaura_document_open(1, nullptr, input_bytes(golden), golden.size(), input_bytes(session), 32, &h, nullptr) == PIXAURA_INVALID_ARGUMENT);
    CHECK(pixaura_document_open(1, &a, input_bytes(golden), golden.size(), input_bytes(session), 32, nullptr, nullptr) == PIXAURA_INVALID_ARGUMENT);
    CHECK(pixaura_document_open(1, &a, input_bytes(golden), 0, input_bytes(session), 32, &h, nullptr) == PIXAURA_INVALID_ARGUMENT);
    CHECK(pixaura_document_create(1, &a, nullptr, 0, input_bytes(session), 32, &h, nullptr) == PIXAURA_INVALID_ARGUMENT);
    CHECK(pixaura_document_create(1, &a, input_bytes(golden), golden.size(), input_bytes(session), 32, &h, nullptr) == PIXAURA_DOCUMENT_INVALID_PROJECT);
    CHECK(pixaura_document_serialize(&a, nullptr, small, 2, &required, nullptr) == PIXAURA_INVALID_ARGUMENT);
    CHECK(pixaura_document_serialize(&a, &h, nullptr, 1, &required, nullptr) == PIXAURA_INVALID_ARGUMENT);
    CHECK(pixaura_document_serialize(&a, &h, small, 2, nullptr, nullptr) == PIXAURA_INVALID_ARGUMENT);
    auto forged = h; forged.serial = 9999; required = 123;
    CHECK(pixaura_document_serialize(&a, &forged, small, 2, &required, nullptr) == PIXAURA_DOCUMENT_INVALID_HANDLE && required == 123);
    forged = h; forged.reserved = 1; CHECK(pixaura_document_release(&a, &forged) == PIXAURA_DOCUMENT_INVALID_HANDLE);
    forged = h; forged.serial = 0; CHECK(pixaura_document_release(&a, &forged) == PIXAURA_DOCUMENT_INVALID_HANDLE);
    forged = h; forged.api_version = 2; CHECK(pixaura_document_release(&a, &forged) == PIXAURA_DOCUMENT_INVALID_HANDLE);
    forged = h; forged.struct_size = sizeof(h) - 1; CHECK(pixaura_document_release(&a, &forged) == PIXAURA_DOCUMENT_INVALID_HANDLE);
    CHECK(pixaura_document_release(&a, nullptr) == PIXAURA_INVALID_ARGUMENT);
    CHECK(pixaura_document_release(&a, &h) == 0); CHECK(serialized == golden);
    CHECK(pixaura_document_release(&a, &h) == PIXAURA_DOCUMENT_INVALID_HANDLE);
    pixaura_document_handle newer{};
    CHECK(pixaura_document_open(1, &a, input_bytes(golden), golden.size(), input_bytes(session), 32, &newer, nullptr) == 0 && newer.serial != h.serial);
    CHECK(pixaura_document_serialize(&a, &h, nullptr, 0, &required, nullptr) == PIXAURA_DOCUMENT_INVALID_HANDLE);
    const auto undo_command = command(*doc, "undo"); pixaura_document_handle proposal{};
    CHECK(pixaura_document_apply(1, &a, nullptr, input_bytes(undo_command), undo_command.size(), &proposal, nullptr) == PIXAURA_INVALID_ARGUMENT);
    CHECK(pixaura_document_apply(1, &a, &newer, nullptr, undo_command.size(), &proposal, nullptr) == PIXAURA_INVALID_ARGUMENT);
    CHECK(pixaura_document_apply(1, &a, &newer, input_bytes(undo_command), undo_command.size(), nullptr, nullptr) == PIXAURA_INVALID_ARGUMENT);
    CHECK(pixaura_document_apply(2, &a, &newer, input_bytes(undo_command), undo_command.size(), &proposal, nullptr) == PIXAURA_UNSUPPORTED_ABI);
    CHECK(pixaura_document_apply(1, &a, &newer, input_bytes(undo_command), undo_command.size(), &proposal, nullptr) == 0);
    CHECK(pixaura_document_serialize(&a, &newer, nullptr, 0, &required, nullptr) == 0 && required == golden.size());
    auto stale = command(*doc, "redo"); auto failed_output = sentinel;
    CHECK(pixaura_document_apply(1, &a, &proposal, input_bytes(stale), stale.size(), &failed_output, nullptr) == PIXAURA_DOCUMENT_STALE_BASE && std::memcmp(&failed_output, &sentinel, sizeof(sentinel)) == 0);
    CHECK(pixaura_document_apply(1, &a, &newer, input_bytes(undo_command), UINT64_MAX, &failed_output, nullptr) == PIXAURA_DOCUMENT_RESOURCE_LIMIT);
    CHECK(pixaura_document_release(&a, &proposal) == 0);
    // A racing serializer either pins the immutable snapshot or rejects release.
    std::atomic<bool> start{false};
    std::thread reader([&] {
        while (!start.load()) std::this_thread::yield();
        for (int i = 0; i < 100; ++i) {
            uint64_t size = 0; const auto status = pixaura_document_serialize(&a, &newer, nullptr, 0, &size, nullptr);
            CHECK(status == 0 || status == PIXAURA_DOCUMENT_INVALID_HANDLE); if (status == 0) CHECK(size == golden.size());
        }
    });
    start.store(true); CHECK(pixaura_document_release(&a, &newer) == 0); reader.join();
    CHECK(pixaura_document_create(1, &a, input_bytes(initial), initial.size(), input_bytes(session), 32, &newer, nullptr) == 0);
    CHECK(pixaura_document_context_destroy(&a) == 0);
    CHECK(pixaura_document_serialize(&a, &newer, nullptr, 0, &required, nullptr) == PIXAURA_DOCUMENT_INVALID_HANDLE);
    CHECK(pixaura_document_context_destroy(&a) == PIXAURA_DOCUMENT_INVALID_HANDLE);
    CHECK(pixaura_document_context_init(1, &a, sizeof(a), input_bytes(context_a), 32, nullptr) == PIXAURA_INVALID_ARGUMENT);
    CHECK(pixaura_document_context_init(1, &a, sizeof(a), input_bytes(context_c), 32, nullptr) == 0);
    CHECK(pixaura_document_release(&a, &newer) == PIXAURA_DOCUMENT_INVALID_HANDLE);
    std::vector<pixaura_document_handle> handles;
    for (int i = 0; i < 64; ++i) { pixaura_document_handle live_handle{}; CHECK(pixaura_document_open(1, &a, input_bytes(initial), initial.size(), input_bytes(session), 32, &live_handle, nullptr) == 0); handles.push_back(live_handle); }
    failed_output = sentinel;
    CHECK(pixaura_document_open(1, &a, input_bytes(initial), initial.size(), input_bytes(session), 32, &failed_output, nullptr) == PIXAURA_DOCUMENT_RESOURCE_LIMIT && std::memcmp(&failed_output, &sentinel, sizeof(sentinel)) == 0);
    CHECK(pixaura_document_context_destroy(&a) == 0 && pixaura_document_context_destroy(&b) == 0);
    CHECK(serialized == golden);
    std::puts("Native document: canonical fixture, history, adversarial inputs, context ownership and racing release PASS");
    return 0;
}
