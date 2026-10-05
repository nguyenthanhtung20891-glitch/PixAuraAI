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
                var workingLimits = pixaura_working_limits()
                XCTAssertEqual(pixaura_working_default_limits(1, &workingLimits), 0)
                var working = pixaura_decode_handle()
                XCTAssertEqual(pixaura_working_normalize(&context, &image, &workingLimits, &working), 0)
                var workingMetadata = pixaura_working_metadata()
                XCTAssertEqual(pixaura_working_query(&context, &working, &workingMetadata), 0)
                let cropRequest = Array("{\"operations\":[{\"id\":\"00000000000000000000000000000009\",\"type\":\"pixaura.crop\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"x_ppm\":999999,\"y_ppm\":0,\"width_ppm\":1,\"height_ppm\":1000000}}]}".utf8)
                var plan = pixaura_geometry_plan()
                XCTAssertEqual(cropRequest.withUnsafeBufferPointer {
                    pixaura_geometry_preflight(1, 2, 3, $0.baseAddress, UInt64($0.count), &workingLimits, &plan)
                }, 0)
                XCTAssertEqual(plan.width, 1)
                XCTAssertEqual(plan.height, 3)
                XCTAssertEqual(plan.executable, 0)
                var tile = pixaura_geometry_tile()
                XCTAssertEqual(pixaura_geometry_tile_at(1, 129, 129, 3, &tile), 0)
                XCTAssertEqual(tile.x0, 128)
                XCTAssertEqual(tile.x1, 129)
                var cancellation = pixaura_decode_handle()
                XCTAssertEqual(pixaura_cancel_create(&context, 1, &cancellation), 0)
                var cancelledOutput = pixaura_decode_handle()
                XCTAssertEqual(cropRequest.withUnsafeBufferPointer {
                    pixaura_working_evaluate_cancel(&context, &working, 1, $0.baseAddress, UInt64($0.count), &workingLimits, &cancellation, &cancelledOutput)
                }, 5)
                XCTAssertEqual(cancelledOutput.serial, 0)
                XCTAssertEqual(pixaura_cancel_signal(&context, &cancellation), 0)
                XCTAssertEqual(pixaura_cancel_signal(&context, &cancellation), 0)
                let identityRequest = Array("{\"operations\":[]}".utf8)
                XCTAssertEqual(identityRequest.withUnsafeBufferPointer {
                    pixaura_working_evaluate_cancel(&context, &working, 1, $0.baseAddress, UInt64($0.count), &workingLimits, &cancellation, &cancelledOutput)
                }, 13)
                XCTAssertEqual(cancelledOutput.serial, 0)
                XCTAssertEqual(pixaura_cancel_release(&context, &cancellation), 0)
                XCTAssertEqual(pixaura_cancel_signal(&context, &cancellation), 3)
                XCTAssertEqual(pixaura_cancel_release(&context, &cancellation), 3)
                XCTAssertEqual(workingMetadata.width, 2)
                XCTAssertEqual(workingMetadata.height, 3)
                XCTAssertEqual(workingMetadata.orientation, 1)
                XCTAssertEqual(workingMetadata.row_stride, 32)
                var evaluated = pixaura_decode_handle()
                XCTAssertEqual(pixaura_working_identity(&context, &working, &workingLimits, &evaluated), 0)
                let request = Array("{\"operations\":[{\"id\":\"00000000000000000000000000000001\",\"type\":\"pixaura.exposure\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":1000}}]}".utf8)
                var edited = pixaura_decode_handle()
                XCTAssertEqual(request.withUnsafeBufferPointer {
                    pixaura_evaluation_validate(1, $0.baseAddress, UInt64($0.count))
                }, 0)
                XCTAssertEqual(request.withUnsafeBufferPointer {
                    pixaura_working_evaluate(&context, &working, 1, $0.baseAddress, UInt64($0.count), &workingLimits, &edited)
                }, 0)
                XCTAssertEqual(pixaura_working_query(&context, &edited, &workingMetadata), 0)
                XCTAssertEqual(workingMetadata.width, 2)
                let invalidOperation = Array("{}".utf8)
                XCTAssertNotEqual(invalidOperation.withUnsafeBufferPointer {
                    pixaura_evaluation_validate(1, $0.baseAddress, UInt64($0.count))
                }, 0)
                XCTAssertEqual(pixaura_decode_release(&context, &edited), 0)
                XCTAssertEqual(pixaura_decode_release(&context, &edited), 3)
                XCTAssertEqual(pixaura_decode_release(&context, &working), 0)
                var pixels = [Float](repeating: -1, count: 24)
                XCTAssertEqual(pixels.withUnsafeMutableBufferPointer {
                    pixaura_working_copy(&context, &evaluated, 0, $0.baseAddress, UInt64($0.count))
                }, 0)
                XCTAssertGreaterThanOrEqual(pixels[0], 0)
                XCTAssertLessThanOrEqual(pixels[0], pixels[3])
                XCTAssertEqual(pixaura_decode_release(&context, &evaluated), 0)
                XCTAssertEqual(pixaura_working_query(&context, &working, &workingMetadata), 3)
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
