import XCTest
import CPixAuraCore
import CPixAuraCertification

final class ModelPackageTests: XCTestCase {
    func testSharedSignedPackageAndIndependentEd25519Corpus() async {
        let result = await Task.detached { pixaura_model_test_boundary() }.value
        XCTAssertEqual(result, 0)
    }
    func testInvalidAbiPreservesPackageOutput() {
        var diagnostic = pixaura_model_diagnostic()
        var package: OpaquePointer?
        XCTAssertEqual(pixaura_model_package_verify(99, nil, 0, 65536, &package, &diagnostic), 8)
        XCTAssertNil(package)
        XCTAssertEqual(diagnostic.reason, 8)
    }
}
