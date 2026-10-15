import XCTest
import CPixAuraCore
import CPixAuraCertification

final class InferenceTests: XCTestCase {
    func testExposureHardwareOwnerRejectsMalformedLifecycle() {
        var owner = pixaura_metal_exposure_context()
        XCTAssertEqual(pixaura_metal_exposure_init(2, &owner, UInt32(MemoryLayout<pixaura_metal_exposure_context>.size)), 1)
        XCTAssertEqual(pixaura_metal_exposure_destroy(&owner), 1)
    }
    func testSharedVerifiedOnnxMetalCorpus() async {
        let result = await Task.detached { pixaura_inference_apple_test_boundary() }.value
        XCTAssertEqual(result, 0)
    }
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
