import XCTest
import CPixAuraCore
@testable import PixAuraCore

final class CoreProbeTests: XCTestCase {
    func testSwiftCallsSharedCore() {
        XCTAssertEqual(CoreProbe.abiVersion(), 1)
    }

    func testCLayoutAndFeatureContractFromSwift() {
        XCTAssertEqual(MemoryLayout<pixaura_core_info>.size, 16)
        XCTAssertEqual(MemoryLayout<pixaura_core_info>.alignment, 4)
        var info = pixaura_core_info()
        XCTAssertEqual(pixaura_get_core_info(1, &info, 16), PIXAURA_OK)
        XCTAssertEqual(info.abi_version, 1)
        XCTAssertEqual(info.struct_size, 16)
        XCTAssertEqual(info.implemented_features, 0)
        XCTAssertEqual(info.reserved, 0)
    }

    func testInvalidCallsPreserveCallerBuffer() {
        var info = pixaura_core_info(abi_version: 99, struct_size: 98,
                                     implemented_features: 97, reserved: 96)
        XCTAssertEqual(pixaura_get_core_info(1, nil, 16), PIXAURA_INVALID_ARGUMENT)
        XCTAssertEqual(pixaura_get_core_info(1, &info, 15), PIXAURA_INVALID_ARGUMENT)
        XCTAssertEqual(pixaura_get_core_info(2, &info, 16), PIXAURA_UNSUPPORTED_ABI)
        XCTAssertEqual(info.abi_version, 99)
        XCTAssertEqual(info.struct_size, 98)
        XCTAssertEqual(info.implemented_features, 97)
        XCTAssertEqual(info.reserved, 96)
    }
}
