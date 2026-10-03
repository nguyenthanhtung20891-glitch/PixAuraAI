import XCTest
import Foundation
import CPixAuraCore

final class DocumentBoundaryTests: XCTestCase {
    func testCanonicalFixtureAndContextOwnership() throws {
        let url = try XCTUnwrap(Bundle.module.url(forResource: "image-document-v1", withExtension: "json", subdirectory: "Fixtures"))
        let fixture = Array(try Data(contentsOf: url))
        let contextID = Array(UUID().uuidString.replacingOccurrences(of: "-", with: "").lowercased().utf8)
        let otherID = Array(UUID().uuidString.replacingOccurrences(of: "-", with: "").lowercased().utf8)
        let sessionID = Array(UUID().uuidString.replacingOccurrences(of: "-", with: "").lowercased().utf8)
        var context = pixaura_document_context()
        var other = pixaura_document_context()
        var handle = pixaura_document_handle()
        var error = pixaura_document_error()
        error.api_version = 1
        error.struct_size = 184
        XCTAssertEqual(MemoryLayout<pixaura_document_error>.size, 184)
        XCTAssertEqual(MemoryLayout<pixaura_document_error>.alignment, 4)
        XCTAssertEqual(MemoryLayout<pixaura_document_handle>.size, 56)
        XCTAssertEqual(MemoryLayout<pixaura_document_context>.size, 64)
        XCTAssertEqual(contextID.withUnsafeBufferPointer {
            pixaura_document_context_init(1, &context, 64, $0.baseAddress, 32, &error)
        }, 0)
        XCTAssertEqual(otherID.withUnsafeBufferPointer {
            pixaura_document_context_init(1, &other, 64, $0.baseAddress, 32, nil)
        }, 0)
        XCTAssertEqual(fixture.withUnsafeBufferPointer { input in
            sessionID.withUnsafeBufferPointer { session in
                pixaura_document_open(1, &context, input.baseAddress, UInt64(input.count), session.baseAddress, 32, &handle, &error)
            }
        }, 0)
        var required: UInt64 = 999
        XCTAssertEqual(pixaura_document_serialize(&other, &handle, nil, 0, &required, &error), PIXAURA_DOCUMENT_INVALID_HANDLE)
        XCTAssertEqual(required, 999)
        XCTAssertEqual(pixaura_document_serialize(&context, &handle, nil, 0, &required, &error), 0)
        XCTAssertEqual(required, UInt64(fixture.count))
        var output = [UInt8](repeating: 0x5a, count: fixture.count + 1)
        XCTAssertEqual(output.withUnsafeMutableBufferPointer {
            pixaura_document_serialize(&context, &handle, $0.baseAddress, 1, &required, &error)
        }, PIXAURA_DOCUMENT_BUFFER_TOO_SMALL)
        XCTAssertTrue(output.allSatisfy { $0 == 0x5a })
        XCTAssertEqual(output.withUnsafeMutableBufferPointer {
            pixaura_document_serialize(&context, &handle, $0.baseAddress, UInt64($0.count), &required, &error)
        }, 0)
        XCTAssertEqual(Array(output.dropLast()), fixture)
        XCTAssertEqual(output.last, 0x5a)
        XCTAssertEqual(pixaura_document_release(&context, &handle), 0)
        XCTAssertEqual(pixaura_document_release(&context, &handle), PIXAURA_DOCUMENT_INVALID_HANDLE)
        XCTAssertEqual(Array(output.dropLast()), fixture)
        XCTAssertEqual(pixaura_document_context_destroy(&context), 0)
        XCTAssertEqual(pixaura_document_context_destroy(&context), PIXAURA_DOCUMENT_INVALID_HANDLE)
        XCTAssertEqual(pixaura_document_serialize(&context, &handle, nil, 0, &required, &error), PIXAURA_DOCUMENT_INVALID_HANDLE)
        XCTAssertEqual(pixaura_document_context_destroy(&other), 0)
    }
}
