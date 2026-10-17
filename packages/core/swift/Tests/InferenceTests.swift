import XCTest
import CPixAuraCore
import CPixAuraCertification

final class InferenceTests: XCTestCase {
    func testAdmissionPolicyRequiresTrustedConfiguration() {
        var context = pixaura_inference_context()
        let identity = Array(repeating: UInt8(54), count: 32)
        XCTAssertEqual(identity.withUnsafeBufferPointer {
            pixaura_inference_context_init(1, &context, UInt32(MemoryLayout<pixaura_inference_context>.size), $0.baseAddress, 32, 67108864)
        }, 0)
        defer { XCTAssertEqual(pixaura_inference_context_destroy(&context), 0) }
        var strategy = pixaura_execution_strategy()
        strategy.version = 99
        XCTAssertEqual(pixaura_inference_policy_query(1, &context, &strategy), 1)
        XCTAssertEqual(strategy.version, 99)
        XCTAssertEqual(pixaura_inference_policy_query(2, &context, &strategy), 2)
        XCTAssertEqual(strategy.version, 99)
    }
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
