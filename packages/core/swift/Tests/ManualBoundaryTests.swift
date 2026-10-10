import XCTest
import Foundation
import CPixAuraCore

final class ManualBoundaryTests: XCTestCase {
    func testToneToolsConsumeSharedDescriptorsAndGestureHistory() throws {
        let url = try XCTUnwrap(Bundle.module.url(forResource: "image-document-v1", withExtension: "json", subdirectory: "Fixtures"))
        let fixture = Array(try Data(contentsOf: url))
        func identity() -> [UInt8] { Array(UUID().uuidString.replacingOccurrences(of: "-", with: "").lowercased().utf8) }
        var required: UInt64 = 0
        XCTAssertEqual(pixaura_manual_registry_json(1, nil, 0, &required), 11)
        var registry = [UInt8](repeating: 0, count: Int(required))
        XCTAssertEqual(registry.withUnsafeMutableBufferPointer { pixaura_manual_registry_json(1, $0.baseAddress, UInt64($0.count), &required) }, 0)
        let object = try XCTUnwrap(JSONSerialization.jsonObject(with: Data(registry)) as? [String: Any])
        let tools = try XCTUnwrap(object["tools"] as? [[String: Any]])
        var context = pixaura_document_context()
        var base = pixaura_document_handle()
        let contextID = identity(), session = identity()
        XCTAssertEqual(contextID.withUnsafeBufferPointer { pixaura_document_context_init(1, &context, 64, $0.baseAddress, 32, nil) }, 0)
        defer { XCTAssertEqual(pixaura_document_context_destroy(&context), 0) }
        XCTAssertEqual(fixture.withUnsafeBufferPointer { m in session.withUnsafeBufferPointer { s in
            pixaura_document_open(1, &context, m.baseAddress, UInt64(m.count), s.baseAddress, 32, &base, nil)
        } }, 0)
        for descriptor in tools where ["tone_color", "detail"].contains(descriptor["category"] as? String ?? "") {
            let tool = try XCTUnwrap(descriptor["tool_id"] as? String)
            let parameters = try XCTUnwrap(descriptor["parameters"] as? [[String: Any]])
            let name = try XCTUnwrap(parameters[0]["name"] as? String)
            let maximum = try XCTUnwrap(parameters[0]["maximum"] as? Int)
            let neutral = try XCTUnwrap(parameters[0]["default"] as? Int)
            let operationID = String(decoding: identity(), as: UTF8.self)
            func operation(_ value: Int) -> [UInt8] { Array("{\"operations\":[{\"id\":\"\(operationID)\",\"type\":\"\(tool)\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"\(name)\":\(value)}}]}".utf8) }
            let toolBytes = Array(tool.utf8)
            for value in [maximum, neutral] {
                var gesture = pixaura_manual_gesture(), proposal = pixaura_document_handle()
                let gestureID = identity(), revisionID = identity()
                XCTAssertEqual(gestureID.withUnsafeBufferPointer { g in toolBytes.withUnsafeBufferPointer { t in
                    pixaura_manual_begin(1, &context, &base, g.baseAddress, 32, t.baseAddress, UInt64(t.count), nil, 0, &gesture)
                } }, 0)
                var sequence: UInt64 = 999
                let request = operation(value)
                for update in 1...1001 {
                    XCTAssertEqual(request.withUnsafeBufferPointer { pixaura_manual_update(&context, &gesture, &base, $0.baseAddress, UInt64($0.count), &sequence) }, 0)
                    XCTAssertEqual(sequence, UInt64(update))
                }
                let invalid = operation(maximum + 1)
                XCTAssertEqual(invalid.withUnsafeBufferPointer { pixaura_manual_update(&context, &gesture, &base, $0.baseAddress, UInt64($0.count), &sequence) }, 7)
                XCTAssertEqual(sequence, 1001)
                XCTAssertEqual(pixaura_manual_current(&context, &gesture, &base, 1000), 13)
                var changed: UInt32 = 999
                XCTAssertEqual(revisionID.withUnsafeBufferPointer { pixaura_manual_commit(&context, &gesture, &base, $0.baseAddress, 32, &proposal, &changed) }, 0)
                XCTAssertEqual(changed, value == neutral ? 0 : 1)
                if changed == 0 { XCTAssertEqual(proposal.serial, base.serial) }
                else { XCTAssertEqual(pixaura_document_release(&context, &proposal), 0) }
                XCTAssertEqual(pixaura_manual_release(&context, &gesture), 0)
            }
        }
    }
    func testGeometryGestureCoalescingAndProposal() throws {
        let url = try XCTUnwrap(Bundle.module.url(forResource: "image-document-v1", withExtension: "json", subdirectory: "Fixtures"))
        let fixture = Array(try Data(contentsOf: url))
        let identity = Array(UUID().uuidString.replacingOccurrences(of: "-", with: "").lowercased().utf8)
        let session = Array(UUID().uuidString.replacingOccurrences(of: "-", with: "").lowercased().utf8)
        let gestureID = Array(UUID().uuidString.replacingOccurrences(of: "-", with: "").lowercased().utf8)
        let revisionID = Array(UUID().uuidString.replacingOccurrences(of: "-", with: "").lowercased().utf8)
        let tool = Array("pixaura.rotate".utf8)
        let operation = Array("{\"operations\":[{\"id\":\"00000000000000000000000000000705\",\"type\":\"pixaura.rotate\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"quarter_turns\":1}}]}".utf8)
        var context = pixaura_document_context()
        var base = pixaura_document_handle(), proposal = pixaura_document_handle()
        var gesture = pixaura_manual_gesture()
        XCTAssertEqual(MemoryLayout<pixaura_manual_gesture>.size, 56)
        XCTAssertEqual(identity.withUnsafeBufferPointer { pixaura_document_context_init(1, &context, 64, $0.baseAddress, 32, nil) }, 0)
        defer { XCTAssertEqual(pixaura_document_context_destroy(&context), 0) }
        XCTAssertEqual(fixture.withUnsafeBufferPointer { m in session.withUnsafeBufferPointer { s in
            pixaura_document_open(1, &context, m.baseAddress, UInt64(m.count), s.baseAddress, 32, &base, nil)
        } }, 0)
        XCTAssertEqual(gestureID.withUnsafeBufferPointer { g in tool.withUnsafeBufferPointer { t in
            pixaura_manual_geometry_begin(1, &context, &base, g.baseAddress, 32, t.baseAddress, UInt64(t.count), nil, 0, &gesture)
        } }, 0)
        var sequence: UInt64 = 999
        for i in 1...1001 {
            XCTAssertEqual(operation.withUnsafeBufferPointer {
                pixaura_manual_geometry_update(&context, &gesture, &base, $0.baseAddress, UInt64($0.count), &sequence)
            }, 0)
            XCTAssertEqual(sequence, UInt64(i))
        }
        XCTAssertEqual(pixaura_manual_geometry_current(&context, &gesture, &base, 1000), PIXAURA_DOCUMENT_CANCELLED)
        XCTAssertEqual(pixaura_manual_geometry_current(&context, &gesture, &base, 1001), 0)
        var changed: UInt32 = 999
        XCTAssertEqual(revisionID.withUnsafeBufferPointer {
            pixaura_manual_geometry_commit(&context, &gesture, &base, $0.baseAddress, 32, &proposal, &changed)
        }, 0)
        XCTAssertEqual(changed, 1)
        XCTAssertNotEqual(base.serial, proposal.serial)
        XCTAssertEqual(pixaura_manual_geometry_current(&context, &gesture, &base, 1001), PIXAURA_DOCUMENT_CANCELLED)
        XCTAssertEqual(pixaura_manual_geometry_release(&context, &gesture), 0)
        XCTAssertEqual(pixaura_manual_geometry_release(&context, &gesture), PIXAURA_DOCUMENT_INVALID_HANDLE)
        let cropTool = Array("pixaura.crop".utf8)
        let cropID = Array(UUID().uuidString.replacingOccurrences(of: "-", with: "").lowercased().utf8)
        let cropRevision = Array(UUID().uuidString.replacingOccurrences(of: "-", with: "").lowercased().utf8)
        let cropOperation = Array("{\"operations\":[{\"id\":\"00000000000000000000000000000706\",\"type\":\"pixaura.crop\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"x_ppm\":999999,\"y_ppm\":999999,\"width_ppm\":1,\"height_ppm\":1}}]}".utf8)
        var cropped = pixaura_document_handle()
        XCTAssertEqual(cropID.withUnsafeBufferPointer { g in cropTool.withUnsafeBufferPointer { t in
            pixaura_manual_geometry_begin(1, &context, &proposal, g.baseAddress, 32, t.baseAddress, UInt64(t.count), nil, 0, &gesture)
        } }, 0)
        XCTAssertEqual(cropOperation.withUnsafeBufferPointer {
            pixaura_manual_geometry_update(&context, &gesture, &proposal, $0.baseAddress, UInt64($0.count), &sequence)
        }, 0)
        XCTAssertEqual(sequence, 1)
        XCTAssertEqual(cropRevision.withUnsafeBufferPointer {
            pixaura_manual_geometry_commit(&context, &gesture, &proposal, $0.baseAddress, 32, &cropped, &changed)
        }, 0)
        XCTAssertEqual(changed, 1)
        XCTAssertEqual(pixaura_manual_geometry_release(&context, &gesture), 0)
        XCTAssertEqual(pixaura_document_release(&context, &cropped), 0)
        XCTAssertEqual(pixaura_document_release(&context, &proposal), 0)
        XCTAssertEqual(pixaura_document_release(&context, &base), 0)
    }
    func testSharedRegistryAndRejectedVersion() throws {
        var required: UInt64 = 999
        XCTAssertEqual(pixaura_manual_registry_json(2, nil, 0, &required), PIXAURA_UNSUPPORTED_ABI)
        XCTAssertEqual(required, 999)
        XCTAssertEqual(pixaura_manual_registry_json(1, nil, 0, &required), PIXAURA_DOCUMENT_BUFFER_TOO_SMALL)
        XCTAssertLessThanOrEqual(required, 32768)
        var bytes = [UInt8](repeating: 0, count: Int(required))
        XCTAssertEqual(bytes.withUnsafeMutableBufferPointer {
            pixaura_manual_registry_json(1, $0.baseAddress, UInt64($0.count), &required)
        }, 0)
        XCTAssertEqual(bytes.last, 10)
        let object = try XCTUnwrap(JSONSerialization.jsonObject(with: Data(bytes)) as? [String: Any])
        let tools = try XCTUnwrap(object["tools"] as? [[String: Any]])
        XCTAssertEqual(tools.compactMap { $0["tool_id"] as? String }, ["pixaura.blur", "pixaura.brightness", "pixaura.contrast", "pixaura.crop", "pixaura.exposure", "pixaura.highlights", "pixaura.rotate", "pixaura.saturation", "pixaura.shadows", "pixaura.sharpen", "pixaura.temperature"])
        let parameters = try XCTUnwrap(tools[3]["parameters"] as? [[String: Any]])
        XCTAssertEqual(parameters[0]["minimum"] as? Int, -5000)
        XCTAssertEqual(parameters[0]["maximum"] as? Int, 5000)
        XCTAssertEqual(parameters[0]["default"] as? Int, 0)
    }
}
