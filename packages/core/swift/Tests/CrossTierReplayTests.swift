import XCTest
import Foundation
import CPixAuraCertification

final class CrossTierReplayTests: XCTestCase {
    func testSharedCanonicalReplayAndReferenceCertification() async throws {
        try await Task.detached {
            let root = FileManager.default.temporaryDirectory.appendingPathComponent("cross-tier-\(UUID())")
            try FileManager.default.createDirectory(at: root, withIntermediateDirectories: true)
            defer { try? FileManager.default.removeItem(at: root) }
            XCTAssertEqual(root.path.withCString { pixaura_test_cross_tier_replay($0, 4) }, 0)
            let matrix = try String(contentsOf: root.appendingPathComponent("cross-tier-matrix.tsv"), encoding: .utf8)
            XCTAssertLessThanOrEqual(matrix.utf8.count, 32768)
            XCTAssertTrue(matrix.contains("SYNTHETIC POLICY EVIDENCE"))
            XCTAssertTrue(matrix.contains("PEER CANONICAL LOW"))
            XCTAssertTrue(matrix.contains("T0_MANUAL_FLOW_PASS"))
            XCTAssertTrue(matrix.contains("LOW_HIGH_PASS"))
            print("APPLE SIMULATOR/HOSTED INTEGRATION, NOT PHYSICAL CERTIFICATION\n\(matrix)")
        }.value
    }
}
