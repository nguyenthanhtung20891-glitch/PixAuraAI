import XCTest
import CPixAuraCore
@testable import PixAuraCore

final class CapabilityFoundationTests: XCTestCase {
    func testConservativeAppleBoundaryIsUnprobedAndDetached() throws {
        let snapshot = try XCTUnwrap(ConservativeCapabilityProbe().snapshot())
        XCTAssertEqual(snapshot.diagnosticBytes.count, 76)
        XCTAssertEqual(Array(snapshot.diagnosticBytes[0..<8]), [0, 0, 0, 1, 0, 0, 0, 4])
        XCTAssertEqual(Array(snapshot.diagnosticBytes[12..<16]), [0, 0, 0, 0])
        XCTAssertEqual(Array(snapshot.diagnosticBytes[28..<44]), [UInt8](repeating: 0, count: 16))
        XCTAssertEqual(Array(snapshot.diagnosticBytes[52..<56]), [0, 0, 0, 1])
        var copy = snapshot.diagnosticBytes
        copy[0] = 255
        XCTAssertEqual(snapshot.diagnosticBytes[0], 0)
    }
    func testProfileValidationPreservesLastSnapshotAndSeparatesCertification() {
        var p = pixaura_capability_profile()
        XCTAssertEqual(pixaura_capability_baseline(1, &p), 0)
        p.hardware.provenance = UInt32(PIXAURA_PROVENANCE_VIRTUAL.rawValue)
        p.hardware.gpu_backend = UInt32(PIXAURA_CAPABILITY_GPU_METAL.rawValue)
        p.hardware.gpu_availability = UInt32(PIXAURA_CAPABILITY_AVAILABLE.rawValue)
        XCTAssertEqual(pixaura_capability_validate(1, &p), 0)
        XCTAssertEqual(p.hardware.gpu_validation, 0)
        p.hardware.gpu_validation = UInt32(PIXAURA_GPU_PHYSICAL_CERTIFIED.rawValue)
        XCTAssertEqual(pixaura_capability_validate(1, &p), 1)
        p.hardware.gpu_validation = 0
        p.runtime.version = 2
        XCTAssertEqual(pixaura_capability_validate(1, &p), 2)
        p.runtime.version = 1
        p.runtime.constrained = 2
        var output = pixaura_capability_profile()
        XCTAssertEqual(pixaura_capability_baseline(1, &output), 0)
        let prior = output.runtime.constrained
        XCTAssertEqual(pixaura_capability_normalize(1, &p.hardware, &p.runtime, &output), 1)
        XCTAssertEqual(output.runtime.constrained, prior)
    }
}
