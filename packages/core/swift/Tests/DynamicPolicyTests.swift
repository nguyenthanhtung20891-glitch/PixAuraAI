import XCTest
import CPixAuraCore

final class DynamicPolicyTests: XCTestCase {
    func testIndependentNormalizedTransitionGolden() {
        for tier in 0...2 {
            var p = pixaura_capability_profile()
            XCTAssertEqual(pixaura_capability_baseline(1, &p), 0)
            p.hardware.logical_cpu_count = [2, 4, 8][tier]
            p.hardware.physical_memory_bytes = [2147483648, 4294967296, 8589934592][tier]
            p.hardware.provenance = UInt32(PIXAURA_PROVENANCE_SYNTHETIC.rawValue)
            p.runtime.process_budget_bytes = [134217728, 268435456, 1073741824][tier]
            p.runtime.thermal = 1; p.runtime.memory_pressure = 1; p.runtime.constrained = 0
            var c = pixaura_decode_context(), limits = pixaura_decode_limits()
            XCTAssertEqual(pixaura_decode_default_limits(1, &limits), 0)
            let identity = Array(repeating: UInt8(97 + tier), count: 32)
            XCTAssertEqual(identity.withUnsafeBufferPointer {
                pixaura_decode_context_init(1, &c, UInt32(MemoryLayout<pixaura_decode_context>.size), $0.baseAddress, 32, &limits)
            }, 0)
            var o = pixaura_policy_observation()
            o.version = 1; o.struct_size = UInt32(MemoryLayout<pixaura_policy_observation>.size)
            o.thermal = 1; o.memory_pressure = 1; o.gpu_availability = 1
            o.process_budget_bytes = p.runtime.process_budget_bytes; o.sequence = 1
            var s = pixaura_execution_strategy()
            XCTAssertEqual(pixaura_policy_configure(1, &c, &p, &o, &s), 0)
            XCTAssertEqual(s.tier, UInt32(tier)); XCTAssertEqual(s.epoch, 1)
            XCTAssertEqual(s.runtime_cap_bytes, [100663296, 201326592, 268435456][tier])
            for severity: UInt32 in 2...4 {
                o.sequence += 1; o.memory_pressure = severity
                XCTAssertEqual(pixaura_policy_update(1, &c, &o, &s), 0)
                XCTAssertEqual(s.tier, 0); XCTAssertEqual(s.backend, 0)
                XCTAssertEqual(s.preview_edge, severity == 4 ? 360 : 720)
                XCTAssertLessThanOrEqual(s.runtime_cap_bytes, severity == 4 ? 33554432 : (severity == 3 ? 67108864 : 134217728))
                let epoch = s.epoch, cap = s.runtime_cap_bytes
                o.memory_pressure = 1
                for stable in 1...3 {
                    o.sequence += 1
                    XCTAssertEqual(pixaura_policy_update(1, &c, &o, &s), 0)
                    if stable < 3 { XCTAssertEqual(s.epoch, epoch); XCTAssertEqual(s.runtime_cap_bytes, cap) }
                }
                XCTAssertEqual(s.tier, UInt32(tier))
            }
            let priorEpoch = s.epoch
            XCTAssertEqual(pixaura_policy_update(1, &c, &o, &s), 0)
            XCTAssertEqual(s.epoch, priorEpoch)
            o.sequence += 1; o.thermal = 0
            XCTAssertEqual(pixaura_policy_update(1, &c, &o, &s), 0); XCTAssertEqual(s.tier, 0)
            o.sequence += 1; o.thermal = 99
            XCTAssertEqual(pixaura_policy_update(1, &c, &o, &s), 1)
            XCTAssertEqual(pixaura_preview_stop(&c), 0)
            XCTAssertEqual(pixaura_decode_context_destroy(&c), 0)
            XCTAssertEqual(pixaura_policy_query(1, &c, &s), 3)
        }
    }
}
