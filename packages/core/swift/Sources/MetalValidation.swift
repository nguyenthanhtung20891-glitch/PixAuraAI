import Foundation
import Metal
import CPixAuraCore
import Darwin

// Hardware ownership only. Gain, validation, corpus, tolerance and fallback are
// supplied by shared C++; this service cannot install a preview or edit history.
public enum MetalValidation {
    private final class Backend {
        let device: MTLDevice
        let queue: MTLCommandQueue
        let pipeline: MTLComputePipelineState
        let input: MTLBuffer
        let output: MTLBuffer
        var dispatched = false

        init(device: MTLDevice) throws {
            self.device = device
            let bytes = Int(PIXAURA_GPU_MAX_PIXELS) * 16
            guard device.maxBufferLength >= bytes,
                  let queue = device.makeCommandQueue(),
                  let input = device.makeBuffer(length: bytes, options: .storageModeShared),
                  let output = device.makeBuffer(length: bytes, options: .storageModeShared),
                  let url = Bundle.module.url(forResource: "exposure", withExtension: "metal", subdirectory: "Shaders")
            else { throw Failure(code: 8) }
            self.queue = queue
            self.input = input
            self.output = output
            let options = MTLCompileOptions()
            if #available(iOS 18.0, macOS 15.0, *) {
                options.mathMode = .safe
            } else {
                options.fastMathEnabled = false
            }
            options.languageVersion = .version2_4
            let library = try device.makeLibrary(source: String(contentsOf: url, encoding: .utf8), options: options)
            guard let function = library.makeFunction(name: "pixaura_exposure") else { throw Failure(code: 14) }
            pipeline = try device.makeComputePipelineState(function: function)
            guard pipeline.maxTotalThreadsPerThreadgroup >= 64 else { throw Failure(code: 5) }
        }

        func dispatch(source: UnsafePointer<Float>, destination: UnsafeMutablePointer<Float>,
                      count: UInt32, gain: Float, identity: UInt32) -> Int32 {
            guard count > 0 && count <= PIXAURA_GPU_MAX_PIXELS else { return 8 }
            let bytes = Int(count) * 16
            memcpy(input.contents(), source, bytes)
            memset(output.contents(), 0xcd, bytes)
            guard let command = queue.makeCommandBuffer(),
                  let encoder = command.makeComputeCommandEncoder() else { return 14 }
            encoder.setComputePipelineState(pipeline)
            encoder.setBuffer(input, offset: 0, index: 0)
            encoder.setBuffer(output, offset: 0, index: 1)
            // Fixed 12-byte layout shared with the shader, with explicit bit words.
            var parameters = [count, gain.bitPattern, identity]
            parameters.withUnsafeMutableBytes { encoder.setBytes($0.baseAddress!, length: 12, index: 2) }
            encoder.dispatchThreadgroups(MTLSize(width: (Int(count) + 63) / 64, height: 1, depth: 1),
                                         threadsPerThreadgroup: MTLSize(width: 64, height: 1, depth: 1))
            encoder.endEncoding()
            command.commit()
            // Shared memory becomes readable only after GPU completion. ARC keeps
            // command/pipeline/buffers alive even if candidate cancellation wins.
            command.waitUntilCompleted()
            guard command.status == .completed && command.error == nil else { return 14 }
            dispatched = true
            memcpy(destination, output.contents(), bytes)
            return 0
        }
    }

    private struct Failure: Error { let code: Int32 }

    public static func evidence() -> [String: Any] {
        guard !Thread.isMainThread else {
            return ["platform": "Apple", "backend": "Metal compute", "status": "FAIL",
                    "reason": "Synchronous GPU validation requires a worker thread", "test_count": 0]
        }
        var model = utsname()
        uname(&model)
        let modelCapacity = MemoryLayout.size(ofValue: model.machine)
        let modelName = withUnsafePointer(to: &model.machine) {
            $0.withMemoryRebound(to: CChar.self, capacity: modelCapacity) { String(cString: $0) }
        }
        var evidence: [String: Any] = ["platform": "Apple", "backend": "Metal compute",
            "device_model": modelName, "os_version": ProcessInfo.processInfo.operatingSystemVersionString,
            "api_version": "Metal language 2.4; OS Metal runtime", "hardware": false,
            "pipeline": false, "dispatch": false, "parity_passed": 0,
            "test_count": 0, "expected_test_count": pixaura_gpu_case_count(), "status": "UNSUPPORTED"]
        guard let device = MTLCreateSystemDefaultDevice() else { return evidence }
        evidence["gpu"] = device.name
        let apple = device.supportsFamily(.apple1)
        let mac = device.supportsFamily(.mac2)
        evidence["apple_family_1"] = apple
        evidence["mac_family_2"] = mac
        evidence["unified_memory"] = device.hasUnifiedMemory
        #if targetEnvironment(simulator)
        evidence["reason"] = "Physical Apple hardware required; simulator is not certification"
        return evidence
        #else
        let name = device.name.lowercased()
        guard (apple || mac) && !name.contains("software") && !name.contains("virtual") else { return evidence }
        evidence["hardware"] = true
        do {
            let backend = try Backend(device: device)
            evidence["pipeline"] = true
            let state = Unmanaged.passUnretained(backend).toOpaque()
            var input = [Float](repeating: 0, count: Int(PIXAURA_GPU_CASE_PIXELS) * 4)
            var output = input
            var passed: UInt32 = 0
            for index in 0..<pixaura_gpu_case_count() {
                var ev: Int32 = 0
                let setup = input.withUnsafeMutableBufferPointer { pixaura_gpu_case(index, $0.baseAddress, PIXAURA_GPU_CASE_PIXELS, &ev) }
                guard setup == 0 else { throw Failure(code: setup) }
                var result = pixaura_gpu_result()
                evidence["test_count"] = index + 1
                let status = input.withUnsafeBufferPointer { source in
                    output.withUnsafeMutableBufferPointer { destination in
                        pixaura_gpu_tile(1, source.baseAddress, PIXAURA_GPU_CASE_PIXELS, ev, { state, source, destination, count, gain, identity in
                            guard let state = state, let source = source, let destination = destination else { return 1 }
                            return Unmanaged<Backend>.fromOpaque(state).takeUnretainedValue().dispatch(
                                source: source, destination: destination, count: count, gain: gain, identity: identity)
                        }, state, nil, nil, 0, destination.baseAddress, &result)
                    }
                }
                evidence["dispatch"] = backend.dispatched
                guard status == 0 && result.used_gpu == 1 else { throw Failure(code: status == 0 ? 14 : status) }
                passed += 1
                evidence["parity_passed"] = passed
            }
            evidence["status"] = "PASS"
        } catch {
            let code = (error as? Failure)?.code ?? 14
            evidence["status_code"] = code
            evidence["status"] = code == 5 ? "UNSUPPORTED" : "FAIL"
        }
        return evidence
        #endif
    }

    public static func printEvidence(_ evidence: [String: Any]) -> String {
        let data = (try? JSONSerialization.data(withJSONObject: evidence, options: [.sortedKeys])) ?? Data("{}".utf8)
        let json = String(decoding: data, as: UTF8.self)
        print("PIXAURA_GPU_EVIDENCE \(json)")
        print("Metal hardware validation: \(evidence["status"] ?? "FAIL") \(evidence["parity_passed"] ?? 0)/\(evidence["expected_test_count"] ?? 0) expected parity cases; GPU=\(evidence["gpu"] ?? "unavailable")")
        return json
    }
}
