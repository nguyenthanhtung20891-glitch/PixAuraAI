import XCTest
import Foundation
import CPixAuraCore
import PixAuraCore

final class GestureLifecycleTests: XCTestCase {
    func testIntegratedGestureGenerationHistoryAndInterruptions() async throws {
        try await Task.detached {
            let root = FileManager.default.temporaryDirectory.appendingPathComponent("gesture-\(UUID().uuidString)")
            let directory = root.appendingPathComponent("assets/sha256")
            try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
            defer { try? FileManager.default.removeItem(at: root) }
            let url = try XCTUnwrap(Bundle.module.url(forResource: "decode", withExtension: "json", subdirectory: "Fixtures"))
            let fixtures = try XCTUnwrap(JSONSerialization.jsonObject(with: Data(contentsOf: url)) as? [String: [String: Any]])
            let fixture = try XCTUnwrap(fixtures["png"])
            let values = try XCTUnwrap(fixture["bytes"] as? [UInt8])
            let hash = try XCTUnwrap(fixture["sha256"] as? String)
            try Data(values).write(to: directory.appendingPathComponent(hash))
            func identity() -> [UInt8] { Array(UUID().uuidString.replacingOccurrences(of: "-", with: "").lowercased().utf8) }
            let contextID = identity(), rasterID = identity(), session = identity()
            let revision = String(decoding: identity(), as: UTF8.self)
            let manifest = Array("{\"current_revision_id\":\"\(revision)\",\"document_id\":\"00000000000000000000000000006005\",\"operations\":[],\"project_id\":\"00000000000000000000000000006006\",\"redo\":[],\"revisions\":[{\"actor\":\"import\",\"id\":\"\(revision)\",\"parent_id\":null,\"plan_id\":null,\"stack\":[]}],\"schema_version\":1,\"source\":{\"byte_length\":\(values.count),\"metadata\":{\"codec\":\"png\",\"has_alpha\":true,\"height\":3,\"icc_sha256\":null,\"orientation\":1,\"width\":2},\"sha256\":\"\(hash)\"}}".utf8)
            var document = pixaura_document_context(), raster = pixaura_decode_context()
            var base = pixaura_document_handle(), encoded = pixaura_decode_handle(), decoded = pixaura_decode_handle(), original = pixaura_decode_handle()
            var dl = pixaura_decode_limits(), wl = pixaura_working_limits()
            XCTAssertEqual(pixaura_decode_default_limits(1, &dl), 0)
            XCTAssertEqual(pixaura_working_default_limits(1, &wl), 0)
            XCTAssertEqual(contextID.withUnsafeBufferPointer { pixaura_document_context_init(1, &document, 64, $0.baseAddress, 32, nil) }, 0)
            XCTAssertEqual(rasterID.withUnsafeBufferPointer { pixaura_decode_context_init(1, &raster, 64, $0.baseAddress, 32, &dl) }, 0)
            defer {
                XCTAssertEqual(pixaura_manual_interrupt(&document), 0)
                XCTAssertEqual(pixaura_document_context_destroy(&document), 0)
                XCTAssertEqual(pixaura_decode_context_destroy(&raster), 0)
            }
            XCTAssertEqual(manifest.withUnsafeBufferPointer { m in session.withUnsafeBufferPointer { s in pixaura_document_create(1, &document, m.baseAddress, UInt64(m.count), s.baseAddress, 32, &base, nil) } }, 0)
            let path = Array(root.resolvingSymlinksInPath().path.utf8), digest = Array(hash.utf8)
            XCTAssertEqual(path.withUnsafeBufferPointer { p in digest.withUnsafeBufferPointer { d in pixaura_decode_open(&raster, p.baseAddress, UInt64(p.count), d.baseAddress, 64, UInt64(values.count), &encoded) } }, 0)
            XCTAssertEqual(pixaura_decode_image(&raster, &encoded, &decoded), 0)
            XCTAssertEqual(pixaura_working_normalize(&raster, &decoded, &wl, &original), 0)
            XCTAssertEqual(pixaura_decode_release(&raster, &encoded), 0)
            XCTAssertEqual(pixaura_decode_release(&raster, &decoded), 0)
            for (tool, parameter) in [("rotate", "quarter_turns"), ("exposure", "milli_ev"), ("sharpen", "milli_amount")] {
                var gesture = pixaura_manual_gesture(), ticket = pixaura_preview_ticket(), latest = pixaura_preview_ticket()
                var sequence: UInt64 = 99, state: UInt32 = 99
                let toolBytes = Array("pixaura.\(tool)".utf8), operationID = String(decoding: identity(), as: UTF8.self)
                func begin(_ live: inout pixaura_document_handle) -> Int32 {
                    let gid = identity()
                    return gid.withUnsafeBufferPointer { g in toolBytes.withUnsafeBufferPointer { t in pixaura_manual_edit_begin(1, &document, &live, g.baseAddress, 32, t.baseAddress, UInt64(t.count), nil, 0, &raster, &original, &gesture) } }
                }
                let operation = Array("{\"operations\":[{\"id\":\"\(operationID)\",\"type\":\"pixaura.\(tool)\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"\(parameter)\":1}}]}".utf8)
                XCTAssertEqual(begin(&base), 0)
                XCTAssertEqual(pixaura_manual_state(&document, &gesture, &state, &sequence), 0)
                XCTAssertEqual(state, 1)
                XCTAssertEqual(sequence, 0)
                XCTAssertEqual(operation.withUnsafeBufferPointer { pixaura_manual_update(&document, &gesture, &base, $0.baseAddress, UInt64($0.count), &sequence) }, 0)
                XCTAssertEqual(pixaura_manual_preview_ticket(&document, &gesture, &base, &ticket, &sequence), 0)
                var preview = pixaura_decode_handle()
                var request = pixaura_preview_request(version: 1, struct_size: 24, mode: 1, max_width: 2, max_height: 3, reserved: 0)
                XCTAssertEqual(pixaura_manual_render(&document, &gesture, &base, &raster, &original, &wl, nil, &ticket, &request, &preview, &sequence), 0)
                let copy = try XCTUnwrap(ReferencePreview.copyImage(context: &raster, preview: &preview))
                for update in 2...10001 {
                    XCTAssertEqual(operation.withUnsafeBufferPointer { pixaura_manual_update(&document, &gesture, &base, $0.baseAddress, UInt64($0.count), &sequence) }, 0)
                    XCTAssertEqual(sequence, UInt64(update))
                }
                XCTAssertEqual(pixaura_manual_preview_ticket(&document, &gesture, &base, &latest, &sequence), 0)
                XCTAssertEqual(latest.generation, ticket.generation + 10000)
                XCTAssertEqual(pixaura_manual_preview_current(&document, &gesture, &base, 1, &ticket, &preview), 13)
                XCTAssertEqual(pixaura_preview_release(&raster, &preview), 0)
                XCTAssertNotNil(copy.dataProvider?.data)
                var proposal = pixaura_document_handle(), changed: UInt32 = 99
                let revisionID = identity()
                XCTAssertEqual(revisionID.withUnsafeBufferPointer { pixaura_manual_commit(&document, &gesture, &base, $0.baseAddress, 32, &proposal, &changed) }, 0)
                XCTAssertEqual(changed, 1)
                XCTAssertEqual(pixaura_manual_state(&document, &gesture, &state, &sequence), 0)
                XCTAssertEqual(state, 3)
                XCTAssertEqual(pixaura_preview_current(&raster, &latest, nil), 13)
                XCTAssertEqual(revisionID.withUnsafeBufferPointer { pixaura_manual_commit(&document, &gesture, &base, $0.baseAddress, 32, &proposal, &changed) }, 13)
                XCTAssertEqual(pixaura_manual_release(&document, &gesture), 0)
                XCTAssertEqual(begin(&proposal), 0)
                XCTAssertEqual(pixaura_manual_interrupt(&document), 0)
                XCTAssertEqual(pixaura_manual_interrupt(&document), 0)
                XCTAssertEqual(pixaura_manual_state(&document, &gesture, &state, &sequence), 3)
                XCTAssertEqual(pixaura_document_release(&document, &proposal), 0)
                XCTAssertNotNil(copy.dataProvider?.data)
            }
        }.value
    }
}
