import Foundation
import Metal
import CPixAuraCore
import Darwin

// Hardware ownership only. Gain, validation, corpus, tolerance and fallback are
// supplied by shared C++; this service cannot install a preview or edit history.
public enum MetalValidation {
    private final class Backend {
        private var owner = pixaura_metal_exposure_context()
        var dispatched = false
        init(device: MTLDevice) throws {
            _ = device // Evidence probe; native worker owns device/queue/buffers.
            let status = pixaura_metal_exposure_init(1, &owner, UInt32(MemoryLayout<pixaura_metal_exposure_context>.size))
            guard status == 0 else { throw Failure(code: status) }
        }
        deinit { _ = pixaura_metal_exposure_destroy(&owner) }
        func dispatch(source: UnsafePointer<Float>, destination: UnsafeMutablePointer<Float>,
                      count: UInt32, gain: Float, identity: UInt32) -> Int32 {
            let status = pixaura_metal_exposure_dispatch(&owner, source, destination, count, gain, identity)
            if status == 0 { dispatched = true }
            return status
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
