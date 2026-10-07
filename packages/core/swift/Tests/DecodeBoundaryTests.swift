import XCTest
import Foundation
import Darwin
import CryptoKit
import CPixAuraCore
import PixAuraCore
import CoreGraphics

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
                XCTAssertEqual(plan.executable, 1)
                var tile = pixaura_geometry_tile()
                XCTAssertEqual(pixaura_geometry_tile_at(1, 129, 129, 3, &tile), 0)
                XCTAssertEqual(tile.x0, 128)
                XCTAssertEqual(tile.x1, 129)
                var cancellation = pixaura_decode_handle()
                XCTAssertEqual(pixaura_cancel_create(&context, 1, &cancellation), 0)
                var cancelledOutput = pixaura_decode_handle()
                XCTAssertEqual(cropRequest.withUnsafeBufferPointer {
                    pixaura_working_evaluate_cancel(&context, &working, 1, $0.baseAddress, UInt64($0.count), &workingLimits, &cancellation, &cancelledOutput)
                }, 0)
                var selectedMetadata = pixaura_working_metadata()
                XCTAssertEqual(pixaura_working_query(&context, &cancelledOutput, &selectedMetadata), 0)
                XCTAssertEqual(selectedMetadata.width, 1)
                XCTAssertEqual(selectedMetadata.height, 3)
                var sourcePixels = [Float](repeating: -1, count: 24)
                var selectedPixels = [Float](repeating: -1, count: 12)
                XCTAssertEqual(sourcePixels.withUnsafeMutableBufferPointer {
                    pixaura_working_copy(&context, &working, 0, $0.baseAddress, 24)
                }, 0)
                XCTAssertEqual(selectedPixels.withUnsafeMutableBufferPointer {
                    pixaura_working_copy(&context, &cancelledOutput, 0, $0.baseAddress, 12)
                }, 0)
                XCTAssertEqual(selectedPixels.map(\.bitPattern), (Array(sourcePixels[4..<8]) + Array(sourcePixels[12..<16]) + Array(sourcePixels[20..<24])).map(\.bitPattern))
                XCTAssertEqual(pixaura_decode_release(&context, &cancelledOutput), 0)
                let mixedRequest = Array("{\"operations\":[{\"id\":\"00000000000000000000000000000001\",\"type\":\"pixaura.rotate\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"quarter_turns\":1}},{\"id\":\"00000000000000000000000000000002\",\"type\":\"pixaura.crop\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"x_ppm\":0,\"y_ppm\":0,\"width_ppm\":1,\"height_ppm\":1}},{\"id\":\"00000000000000000000000000000003\",\"type\":\"pixaura.exposure\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":0}}]}".utf8)
                XCTAssertEqual(mixedRequest.withUnsafeBufferPointer {
                    pixaura_working_evaluate_cancel(&context, &working, 1, $0.baseAddress, UInt64($0.count), &workingLimits, &cancellation, &cancelledOutput)
                }, 0)
                XCTAssertEqual(pixaura_working_query(&context, &cancelledOutput, &selectedMetadata), 0)
                XCTAssertEqual(selectedMetadata.width, 1)
                XCTAssertEqual(selectedMetadata.height, 1)
                var mixedPixel = [Float](repeating: -1, count: 4)
                XCTAssertEqual(mixedPixel.withUnsafeMutableBufferPointer {
                    pixaura_working_copy(&context, &cancelledOutput, 0, $0.baseAddress, 4)
                }, 0)
                XCTAssertEqual(mixedPixel.map(\.bitPattern), Array(sourcePixels[16..<20]).map(\.bitPattern))
                XCTAssertEqual(pixaura_decode_release(&context, &cancelledOutput), 0)
                cancelledOutput = pixaura_decode_handle()
                let invalidGeometry = Array(String(decoding: mixedRequest, as: UTF8.self).replacingOccurrences(of: "quarter_turns\":1", with: "quarter_turns\":4").utf8)
                XCTAssertEqual(invalidGeometry.withUnsafeBufferPointer {
                    pixaura_working_evaluate_cancel(&context, &working, 1, $0.baseAddress, UInt64($0.count), &workingLimits, &cancellation, &cancelledOutput)
                }, 7)
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
                var preview = pixaura_decode_handle()
                var previewMetadata = pixaura_preview_metadata()
                XCTAssertEqual(pixaura_preview_create(&context, &evaluated, 1, nil, &preview), 0)
                XCTAssertEqual(pixaura_preview_query(&context, &preview, &previewMetadata), 0)
                XCTAssertEqual(previewMetadata.width, 2)
                XCTAssertEqual(previewMetadata.height, 3)
                XCTAssertEqual(previewMetadata.row_stride, 8)
                XCTAssertEqual(previewMetadata.image_bytes, 24)
                XCTAssertEqual(previewMetadata.pixel_format, 1)
                var previewBytes = [UInt8](repeating: 0, count: 24)
                XCTAssertEqual(previewBytes.withUnsafeMutableBufferPointer {
                    pixaura_preview_copy(&context, &preview, 0, $0.baseAddress, 24)
                }, 0)
                XCTAssertEqual(previewBytes[3], 255)
                XCTAssertEqual(previewBytes[23], name == "png" ? 128 : 255)
                let platformExact = try XCTUnwrap(ReferencePreview.copyImage(context: &context, preview: &preview))
                XCTAssertEqual(platformExact.width, 2)
                XCTAssertEqual(platformExact.alphaInfo, .last)
                XCTAssertEqual(platformExact.colorSpace?.name, CGColorSpace.sRGB)
                XCTAssertEqual(platformExact.dataProvider?.data as Data?, Data(previewBytes))
                if name == "png" {
                    let red = try XCTUnwrap(platformExact.cropping(to: CGRect(x: 0, y: 0, width: 1, height: 1)))
                    var rendered = [UInt8](repeating: 0, count: 4)
                    let drew = rendered.withUnsafeMutableBytes { storage -> Bool in
                        guard let space = CGColorSpace(name: CGColorSpace.sRGB),
                              let display = CGContext(data: storage.baseAddress, width: 1, height: 1,
                                                      bitsPerComponent: 8, bytesPerRow: 4, space: space,
                                                      bitmapInfo: CGBitmapInfo.byteOrder32Big.rawValue | CGImageAlphaInfo.premultipliedLast.rawValue) else { return false }
                        display.draw(red, in: CGRect(x: 0, y: 0, width: 1, height: 1))
                        return true
                    }
                    XCTAssertTrue(drew)
                    XCTAssertEqual(rendered, [255, 0, 0, 255])
                }
                var fitRequest = pixaura_preview_request(version: 1, struct_size: UInt32(MemoryLayout<pixaura_preview_request>.size), mode: 1, max_width: 1, max_height: 2, reserved: 0)
                var fitted = pixaura_decode_handle()
                XCTAssertEqual(pixaura_preview_render(&context, &evaluated, &fitRequest, nil, &fitted), 0)
                let platformFit = try XCTUnwrap(ReferencePreview.copyImage(context: &context, preview: &fitted))
                XCTAssertEqual(platformFit.width, 1)
                XCTAssertEqual(platformFit.height, 1)
                if name == "png" { XCTAssertEqual(platformFit.dataProvider?.data as Data?, Data([188, 188, 255, 255])) }
                XCTAssertEqual(pixaura_preview_release(&context, &fitted), 0)
                // Provider data remains owned after native release.
                XCTAssertEqual((platformFit.dataProvider?.data as Data?)?.count, 4)
                let interactive = try XCTUnwrap(InteractivePreview(identity: Array(UUID().uuidString.replacingOccurrences(of: "-", with: "").lowercased().utf8)))
                let ownedWorking: pixaura_decode_handle = interactive.prepare { owner in
                    var original = pixaura_decode_handle(), decoded = pixaura_decode_handle(), canonical = pixaura_decode_handle()
                    XCTAssertEqual(path.withUnsafeBufferPointer { p in digest.withUnsafeBufferPointer { d in
                        pixaura_decode_open(owner, p.baseAddress, UInt64(p.count), d.baseAddress, 64, UInt64(values.count), &original)
                    } }, 0)
                    XCTAssertEqual(pixaura_decode_image(owner, &original, &decoded), 0)
                    XCTAssertEqual(pixaura_working_normalize(owner, &decoded, &workingLimits, &canonical), 0)
                    XCTAssertEqual(pixaura_decode_release(owner, &original), 0)
                    XCTAssertEqual(pixaura_decode_release(owner, &decoded), 0)
                    return canonical
                }
                let first = try XCTUnwrap(interactive.begin())
                let old = try XCTUnwrap(interactive.render(first, working: ownedWorking, request: fitRequest))
                let latest = try XCTUnwrap(interactive.begin())
                XCTAssertFalse(interactive.install(old))
                XCTAssertNil(interactive.render(first, working: ownedWorking, request: fitRequest))
                let current = try XCTUnwrap(interactive.render(latest, working: ownedWorking, request: fitRequest))
                XCTAssertTrue(interactive.install(current))
                XCTAssertFalse(interactive.install(old))
                let cancelled = try XCTUnwrap(interactive.begin())
                XCTAssertTrue(interactive.cancel(cancelled))
                XCTAssertTrue(interactive.cancel(cancelled))
                XCTAssertNil(interactive.render(cancelled, working: ownedWorking, request: fitRequest))
                let failed = try XCTUnwrap(interactive.begin())
                var invalidFit = fitRequest; invalidFit.max_width = 0
                XCTAssertNil(interactive.render(failed, working: ownedWorking, request: invalidFit))
                XCTAssertEqual(interactive.requestedGeneration, failed.generation)
                XCTAssertEqual(interactive.displayedGeneration, latest.generation)
                interactive.close()
                XCTAssertFalse(interactive.install(current))
                XCTAssertNil(interactive.begin())
                XCTAssertEqual((current.image.dataProvider?.data as Data?)?.count, 4)
                if name == "png" { XCTAssertEqual(current.image.dataProvider?.data as Data?, Data([188,188,255,255])) }
                var previewCancel = pixaura_decode_handle()
                var rejectedPreview = pixaura_decode_handle()
                XCTAssertEqual(pixaura_cancel_create(&context, 1, &previewCancel), 0)
                XCTAssertEqual(pixaura_cancel_signal(&context, &previewCancel), 0)
                XCTAssertEqual(pixaura_preview_create(&context, &evaluated, 1, &previewCancel, &rejectedPreview), 13)
                XCTAssertEqual(rejectedPreview.serial, 0)
                XCTAssertEqual(pixaura_cancel_release(&context, &previewCancel), 0)
                XCTAssertEqual(pixaura_preview_release(&context, &preview), 0)
                XCTAssertEqual(pixaura_preview_release(&context, &preview), 3)
                XCTAssertEqual(pixaura_preview_query(&context, &preview, &previewMetadata), 3)
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
