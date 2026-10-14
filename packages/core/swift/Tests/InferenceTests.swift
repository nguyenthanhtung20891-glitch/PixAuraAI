import XCTest
import CPixAuraCore
import CPixAuraCertification

final class InferenceTests: XCTestCase {
    func testSharedVerifiedOnnxReferenceCorpus() async {
        let result = await Task.detached { pixaura_inference_test_boundary() }.value
        XCTAssertEqual(result, 0)
    }
    func testUnknownProviderPreservesDescriptor() {
        var descriptor = pixaura_inference_descriptor()
        descriptor.version = 99
        let status = "unknown".withCString {
            pixaura_inference_descriptor_query(1, $0, 7, &descriptor, UInt32(MemoryLayout<pixaura_inference_descriptor>.size))
        }
        XCTAssertEqual(status, 4)
        XCTAssertEqual(descriptor.version, 99)
    }
}
