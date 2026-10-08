import XCTest
import PixAuraCore

final class GpuHardwareTests: XCTestCase {
    func testPhysicalMetalCertification() {
        let completed = expectation(description: "Metal hardware dispatch and readback")
        DispatchQueue.global(qos: .userInitiated).async {
            let evidence = MetalValidation.evidence()
            let json = MetalValidation.printEvidence(evidence)
            let attachment = XCTAttachment(string: json)
            attachment.name = "pixaura-metal-hardware.json"
            attachment.lifetime = .keepAlways
            self.add(attachment)
            #if targetEnvironment(simulator)
            XCTAssertEqual(evidence["status"] as? String, "UNSUPPORTED", json)
            #else
            XCTAssertEqual(evidence["status"] as? String, "PASS", json)
            XCTAssertEqual(evidence["hardware"] as? Bool, true, json)
            XCTAssertEqual(evidence["pipeline"] as? Bool, true, json)
            XCTAssertEqual(evidence["dispatch"] as? Bool, true, json)
            XCTAssertEqual(evidence["test_count"] as? UInt32, evidence["expected_test_count"] as? UInt32, json)
            XCTAssertEqual(evidence["parity_passed"] as? UInt32, evidence["expected_test_count"] as? UInt32, json)
            #endif
            completed.fulfill()
        }
        wait(for: [completed], timeout: 120)
    }
}
