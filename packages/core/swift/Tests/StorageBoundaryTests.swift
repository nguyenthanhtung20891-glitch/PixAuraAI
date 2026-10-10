import XCTest
import Foundation
import Darwin
import CPixAuraCore

final class StorageBoundaryTests: XCTestCase {
    private struct ProbeResult: Sendable {
        let status: Int32
        let apiVersion: UInt32
        let structSize: UInt32
        let storageVersion: UInt32
        let sqliteVersion: UInt32
        let osError: Int32
    }

    private static func canonicalBytes(_ root: URL) throws -> [UInt8] {
        // Foundation resolvingSymlinksInPath may strip /private on Darwin and
        // reintroduce the /var or /tmp alias. The native contract needs realpath.
        let resolved: UnsafeMutablePointer<CChar>? = root.withUnsafeFileSystemRepresentation {
            guard let input = $0 else { return nil }
            return realpath(input, nil)
        }
        let osError = errno
        let pointer = try XCTUnwrap(resolved, "realpath failed errno=\(osError)")
        defer { free(pointer) }
        return Array(String(cString: pointer).utf8)
    }

    private static func probe(_ path: [UInt8]) async -> ProbeResult {
        await Task.detached {
            var info = pixaura_storage_info(api_version: 87, struct_size: 99, storage_version: 55, sqlite_version: 44)
            errno = 0
            let status = path.withUnsafeBufferPointer {
                pixaura_storage_check(1, $0.baseAddress, UInt64($0.count), &info)
            }
            let osError = errno
            return ProbeResult(status: status, apiVersion: info.api_version, structSize: info.struct_size,
                               storageVersion: info.storage_version, sqliteVersion: info.sqlite_version, osError: osError)
        }.value
    }

    private func expectFailure(_ path: [UInt8], status: Int32) async {
        let result = await Self.probe(path)
        XCTAssertEqual(result.status, status, "storage status=\(result.status) errno=\(result.osError)")
        XCTAssertEqual(result.apiVersion, 87)
        XCTAssertEqual(result.structSize, 99)
        XCTAssertEqual(result.storageVersion, 55)
        XCTAssertEqual(result.sqliteVersion, 44)
    }

    func testNativeSchemaCreationAndReopen() async throws {
        let root = FileManager.default.temporaryDirectory.appendingPathComponent("storage-test-\(UUID().uuidString) space-%#-é")
        try FileManager.default.createDirectory(at: root, withIntermediateDirectories: false)
        defer { try? FileManager.default.removeItem(at: root) }
        let path = try await Task.detached { try Self.canonicalBytes(root) }.value
        let foundationText = root.resolvingSymlinksInPath().path
        let foundationPath = Array(foundationText.utf8)
        if String(decoding: path, as: UTF8.self) == "/private" + foundationText {
            // Capture the original Foundation alias rejection before schema creation.
            let rejection = await Self.probe(foundationPath)
            XCTAssertEqual(rejection.status, PIXAURA_DOCUMENT_INVALID_PROJECT)
            await expectFailure(foundationPath, status: 6)
            print("storage Foundation alias status=\(rejection.status) errno=\(rejection.osError)")
        }
        XCTAssertEqual(MemoryLayout<pixaura_storage_info>.size, 16)
        for _ in 0..<2 {
            let result = await Self.probe(path)
            XCTAssertEqual(result.status, 0, "storage status=\(result.status) errno=\(result.osError)")
            XCTAssertEqual(result.apiVersion, 1)
            XCTAssertEqual(result.structSize, 16)
            XCTAssertEqual(result.storageVersion, 1)
            XCTAssertEqual(result.sqliteVersion, 3053004)
        }
        XCTAssertTrue(FileManager.default.fileExists(atPath: root.appendingPathComponent("catalog.sqlite").path))
        let migrated = await Task.detached {
            var info = pixaura_storage_info()
            return path.withUnsafeBufferPointer { pixaura_storage_migrate(1, $0.baseAddress, UInt64($0.count), 1, 2, &info) }
        }.value
        XCTAssertEqual(migrated, 0)
        let reopened = await Self.probe(path)
        XCTAssertEqual(reopened.status, 0)
        XCTAssertEqual(reopened.storageVersion, 2)
        let repeated = await Task.detached {
            var info = pixaura_storage_info(api_version: 87, struct_size: 99, storage_version: 55, sqlite_version: 44)
            let status = path.withUnsafeBufferPointer { pixaura_storage_migrate(1, $0.baseAddress, UInt64($0.count), 1, 2, &info) }
            return (status, info.storage_version)
        }.value
        XCTAssertEqual(repeated.0, 4)
        XCTAssertEqual(repeated.1, 55)
        let detailMigration = await Task.detached {
            var info = pixaura_storage_info()
            return path.withUnsafeBufferPointer { pixaura_storage_migrate(1, $0.baseAddress, UInt64($0.count), 2, 3, &info) }
        }.value
        XCTAssertEqual(detailMigration, 0)
        let detailReopened = await Self.probe(path)
        XCTAssertEqual(detailReopened.storageVersion, 3)
        let detailRepeated = await Task.detached {
            var info = pixaura_storage_info(api_version: 87, struct_size: 99, storage_version: 55, sqlite_version: 44)
            let status = path.withUnsafeBufferPointer { pixaura_storage_migrate(1, $0.baseAddress, UInt64($0.count), 2, 3, &info) }
            return (status, info.storage_version)
        }.value
        XCTAssertEqual(detailRepeated.0, 4)
        XCTAssertEqual(detailRepeated.1, 55)
        await expectFailure(Array("../escape".utf8), status: 1)
    }

    func testRejectedRootsPreserveOutputs() async throws {
        let root = FileManager.default.temporaryDirectory.appendingPathComponent("storage-errors-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: root, withIntermediateDirectories: false)
        defer { try? FileManager.default.removeItem(at: root) }
        let path = try await Task.detached { try Self.canonicalBytes(root) }.value
        let text = String(decoding: path, as: UTF8.self)
        await expectFailure(Array((text + "/nonexistent").utf8), status: 6)
        await expectFailure(Array((text + "/../escape").utf8), status: 1)
        let file = root.appendingPathComponent("regular-file")
        try Data([1]).write(to: file)
        await expectFailure(Array((text + "/regular-file").utf8), status: 6)

        // Controlled alias reproduces the /var -> /private/var path shape.
        let alias = root.appendingPathComponent("alias")
        try FileManager.default.createSymbolicLink(atPath: alias.path, withDestinationPath: text)
        let aliasPath = text + "/alias"
        let nativeOpen = await Task.detached {
            errno = 0
            let fd = open(aliasPath, O_RDONLY | O_DIRECTORY | O_NOFOLLOW_ANY | O_CLOEXEC)
            let osError = errno
            if fd >= 0 { close(fd) }
            return (fd, osError)
        }.value
        XCTAssertEqual(nativeOpen.0, -1)
        XCTAssertEqual(nativeOpen.1, ELOOP, "Darwin root open errno=\(nativeOpen.1)")
        await expectFailure(Array(aliasPath.utf8), status: 6)
        let rejection = await Self.probe(Array(aliasPath.utf8))
        print("storage controlled alias open-errno=\(nativeOpen.1) native-status=\(rejection.status) native-errno=\(rejection.osError)")
        await expectFailure(Array((aliasPath + "/nonexistent").utf8), status: 6)

        let denied = root.appendingPathComponent("unwritable")
        try FileManager.default.createDirectory(at: denied, withIntermediateDirectories: false)
        try FileManager.default.setAttributes([.posixPermissions: 0o500], ofItemAtPath: denied.path)
        defer { try? FileManager.default.setAttributes([.posixPermissions: 0o700], ofItemAtPath: denied.path) }
        await expectFailure(Array((text + "/unwritable").utf8), status: 12)
        XCTAssertFalse(FileManager.default.fileExists(atPath: denied.appendingPathComponent("catalog.sqlite").path))
    }
}
