import XCTest
import Foundation
import CPixAuraCore

final class StorageBoundaryTests: XCTestCase {
    func testNativeSchemaCreationAndReopen() async throws {
        let root = FileManager.default.temporaryDirectory.appendingPathComponent("storage-test-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: root, withIntermediateDirectories: false)
        defer { try? FileManager.default.removeItem(at: root) }
        // Darwin temporary roots have symlink aliases; pass the actual private directory.
        let path = Array(root.resolvingSymlinksInPath().path.utf8)
        XCTAssertEqual(MemoryLayout<pixaura_storage_info>.size, 16)
        for _ in 0..<2 {
            let result = await Task.detached {
                var info = pixaura_storage_info()
                let status = path.withUnsafeBufferPointer {
                    pixaura_storage_check(1, $0.baseAddress, UInt64($0.count), &info)
                }
                return (status, info.storage_version, info.sqlite_version)
            }.value
            XCTAssertEqual(result.0, 0)
            XCTAssertEqual(result.1, 1)
            XCTAssertEqual(result.2, 3053004)
        }
        XCTAssertTrue(FileManager.default.fileExists(atPath: root.appendingPathComponent("catalog.sqlite").path))
        let invalid = Array("../escape".utf8)
        let status = await Task.detached {
            var info = pixaura_storage_info()
            return invalid.withUnsafeBufferPointer {
                pixaura_storage_check(1, $0.baseAddress, UInt64($0.count), &info)
            }
        }.value
        XCTAssertEqual(status, 1)
    }
}
