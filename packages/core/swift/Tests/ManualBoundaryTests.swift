import XCTest
import Foundation
import CPixAuraCore

final class ManualBoundaryTests: XCTestCase {
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
        XCTAssertEqual(tools.compactMap { $0["tool_id"] as? String }, ["pixaura.crop", "pixaura.exposure", "pixaura.rotate"])
        let parameters = try XCTUnwrap(tools[1]["parameters"] as? [[String: Any]])
        XCTAssertEqual(parameters[0]["minimum"] as? Int, -5000)
        XCTAssertEqual(parameters[0]["maximum"] as? Int, 5000)
        XCTAssertEqual(parameters[0]["default"] as? Int, 0)
    }
}
