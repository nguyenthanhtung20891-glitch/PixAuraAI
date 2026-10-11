import XCTest
import CPixAuraCore

final class BudgetAdmissionTests: XCTestCase {
    private func request() -> pixaura_budget_request {
        var q = pixaura_budget_request()
        q.version = 1; q.struct_size = UInt32(MemoryLayout<pixaura_budget_request>.size)
        q.work_class = UInt32(PIXAURA_BUDGET_COMMIT.rawValue)
        q.bytes_per_pixel = 16; q.raster_count = 2; q.width = 64; q.height = 64
        q.requested_handles = 1; q.context_limit_bytes = 268435456
        return q
    }
    private func profile(_ tier: UInt32) -> pixaura_capability_profile {
        var p = pixaura_capability_profile()
        XCTAssertEqual(pixaura_capability_baseline(1, &p), 0)
        p.hardware.logical_cpu_count = [2, 4, 8][Int(tier)]
        p.hardware.physical_memory_bytes = [2147483648, 4294967296, 8589934592][Int(tier)]
        p.hardware.provenance = UInt32(PIXAURA_PROVENANCE_SYNTHETIC.rawValue)
        p.runtime.process_budget_bytes = [134217728, 268435456, 1073741824][Int(tier)]
        p.runtime.constrained = 0; p.runtime.thermal = UInt32(PIXAURA_THERMAL_NORMAL.rawValue)
        p.runtime.memory_pressure = UInt32(PIXAURA_PRESSURE_NORMAL.rawValue)
        return p
    }
    func testSharedIntegerTierAndCapGolden() {
        for tier: UInt32 in 0...2 {
            var p = profile(tier), q = request(), out = pixaura_budget_result()
            XCTAssertEqual(pixaura_budget_admit(1, &p, &q, &out), 0)
            XCTAssertEqual(out.tier, tier); XCTAssertEqual(out.decision, UInt32(PIXAURA_BUDGET_ADMIT.rawValue))
            XCTAssertEqual(out.runtime_cap_bytes, [100663296, 201326592, 268435456][Int(tier)])
            XCTAssertEqual(out.peak_bytes, 1179648); XCTAssertEqual(out.raster_bytes, 65536)
            XCTAssertEqual(out.backend, UInt32(PIXAURA_BUDGET_CPU.rawValue))
            XCTAssertEqual(out.max_concurrent_work, 1)
        }
    }
    func testPreviewDowngradeCannotShrinkWorkingSourceOrRaiseCeilings() {
        var p = profile(0), q = request(), out = pixaura_budget_result()
        q.work_class = UInt32(PIXAURA_BUDGET_PREVIEW.rawValue)
        q.width = 1024; q.height = 1024; q.preview_edge = 4096
        XCTAssertEqual(pixaura_budget_admit(1, &p, &q, &out), 0)
        XCTAssertEqual(out.preview_width, 720); XCTAssertEqual(out.peak_bytes, 36676608)
        q.retained_bytes = 65011712
        XCTAssertEqual(pixaura_budget_admit(1, &p, &q, &out), 0)
        XCTAssertEqual(out.preview_width, 360); XCTAssertEqual(out.reason, UInt32(PIXAURA_BUDGET_PREVIEW_MEMORY.rawValue))
        q.retained_bytes = 67108864
        XCTAssertEqual(pixaura_budget_admit(1, &p, &q, &out), 0)
        XCTAssertEqual(out.decision, UInt32(PIXAURA_BUDGET_REJECT.rawValue))
        p = profile(2); q = request(); q.context_limit_bytes = 268435457
        XCTAssertEqual(pixaura_budget_admit(1, &p, &q, &out), 0)
        XCTAssertEqual(out.reason, UInt32(PIXAURA_BUDGET_HARD_CEILING.rawValue))
        q = request(); q.width = UInt64.max; q.height = 2
        XCTAssertEqual(pixaura_budget_admit(1, &p, &q, &out), 0)
        XCTAssertEqual(out.reason, UInt32(PIXAURA_BUDGET_OVERFLOW.rawValue))
        let prior = out.reason
        q.version = 2
        XCTAssertEqual(pixaura_budget_admit(1, &p, &q, &out), 2)
        XCTAssertEqual(out.reason, prior)
    }
}
