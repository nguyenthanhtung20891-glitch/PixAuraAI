import XCTest
import Foundation
import Darwin
import CryptoKit
import CPixAuraCore

final class DecodeBoundaryTests: XCTestCase {
    func testSharedAdmissionDecodeAndOwnership() async throws {
        try await Task.detached {
            let root = FileManager.default.temporaryDirectory.appendingPathComponent("decode-\(UUID().uuidString)")
            let directory = root.appendingPathComponent("assets/sha256")
            try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
            defer { try? FileManager.default.removeItem(at: root) }
            let raw: UnsafeMutablePointer<CChar>? = root.withUnsafeFileSystemRepresentation {
                guard let p = $0 else { return nil }
                return realpath(p, nil)
            }
            let resolved = try XCTUnwrap(raw)
            defer { free(resolved) }
            let path = Array(String(cString: resolved).utf8)
            let url = try XCTUnwrap(Bundle.module.url(forResource: "decode", withExtension: "json", subdirectory: "Fixtures"))
            let fixtures = try XCTUnwrap(JSONSerialization.jsonObject(with: Data(contentsOf: url)) as? [String: [String: Any]])
            for name in ["jpeg", "png"] {
                let fixture = try XCTUnwrap(fixtures[name])
                let values = try XCTUnwrap(fixture["bytes"] as? [UInt8])
                let hash = try XCTUnwrap(fixture["sha256"] as? String)
                try Data(values).write(to: directory.appendingPathComponent(hash))
                let digest = Array(hash.utf8)
                let identity = Array(UUID().uuidString.replacingOccurrences(of: "-", with: "").lowercased().utf8)
                var limits = pixaura_decode_limits()
                XCTAssertEqual(pixaura_decode_default_limits(1, &limits), 0)
                var context = pixaura_decode_context()
                XCTAssertEqual(identity.withUnsafeBufferPointer {
                    pixaura_decode_context_init(1, &context, UInt32(MemoryLayout<pixaura_decode_context>.size), $0.baseAddress, 32, &limits)
                }, 0)
                defer { XCTAssertEqual(pixaura_decode_context_destroy(&context), 0) }
                var source = pixaura_decode_handle()
                let status = path.withUnsafeBufferPointer { p in digest.withUnsafeBufferPointer { d in
                    pixaura_decode_open(&context, p.baseAddress, UInt64(p.count), d.baseAddress, 64, UInt64(values.count), &source)
                } }
                XCTAssertEqual(status, 0)
                var metadata = pixaura_decode_metadata()
                XCTAssertEqual(pixaura_decode_query(&context, &source, &metadata), 0)
                XCTAssertEqual(metadata.width, 2)
                XCTAssertEqual(metadata.height, 3)
                XCTAssertEqual(metadata.row_stride, 8)
                var image = pixaura_decode_handle()
                XCTAssertEqual(pixaura_decode_image(&context, &source, &image), 0)
                XCTAssertEqual(pixaura_decode_release(&context, &source), 0)
                XCTAssertEqual(pixaura_decode_query(&context, &image, &metadata), 0)
                XCTAssertEqual(pixaura_decode_release(&context, &image), 0)
                XCTAssertEqual(pixaura_decode_release(&context, &image), 3)
                XCTAssertEqual(pixaura_decode_query(&context, &source, &metadata), 3)
                let invalid = Data([1, 2, 3])
                let badHash = SHA256.hash(data: invalid).map { String(format: "%02x", $0) }.joined()
                try invalid.write(to: directory.appendingPathComponent(badHash))
                let badDigest = Array(badHash.utf8)
                let oldSerial = image.serial
                let rejected = path.withUnsafeBufferPointer { p in badDigest.withUnsafeBufferPointer { d in
                    pixaura_decode_open(&context, p.baseAddress, UInt64(p.count), d.baseAddress, 64, 3, &image)
                } }
                XCTAssertEqual(rejected, 17)
                XCTAssertEqual(image.serial, oldSerial)
                let recovered = path.withUnsafeBufferPointer { p in digest.withUnsafeBufferPointer { d in
                    pixaura_decode_open(&context, p.baseAddress, UInt64(p.count), d.baseAddress, 64, UInt64(values.count), &source)
                } }
                XCTAssertEqual(recovered, 0)
                XCTAssertEqual(pixaura_decode_image(&context, &source, &image), 0)
            }
        }.value
    }
}
