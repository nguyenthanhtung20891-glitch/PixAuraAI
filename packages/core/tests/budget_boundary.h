#ifndef PIXAURA_BUDGET_BOUNDARY_H
#define PIXAURA_BUDGET_BOUNDARY_H
#include "capability_fixtures.h"
#include "pixaura/budget.h"
#include "pixaura/decode.h"
#include "pixaura/working.h"
#include <string.h>
static pixaura_capability_profile budget_profile(unsigned i) {
    pixaura_capability_profile p;
    pixaura_capability_baseline(1, &p);
    p.hardware.platform = PIXAURA_PLATFORM_UNKNOWN; p.hardware.architecture = PIXAURA_ARCH_UNKNOWN;
    p.hardware.provenance = PIXAURA_PROVENANCE_SYNTHETIC;
    p.hardware.logical_cpu_count = capability_fixtures[i].cpus;
    p.hardware.physical_memory_bytes = capability_fixtures[i].memory;
    p.runtime.process_budget_bytes = capability_fixtures[i].budget;
    p.runtime.constrained = 0; p.runtime.thermal = PIXAURA_THERMAL_NORMAL;
    p.runtime.memory_pressure = PIXAURA_PRESSURE_NORMAL;
    return p;
}
static pixaura_budget_request budget_request(void) {
    pixaura_budget_request q;
    memset(&q, 0, sizeof(q)); q.version = 1; q.struct_size = sizeof(q);
    q.work_class = PIXAURA_BUDGET_COMMIT; q.bytes_per_pixel = 16; q.raster_count = 2;
    q.width = 64; q.height = 64; q.requested_handles = 1;
    q.context_limit_bytes = UINT64_C(268435456);
    return q;
}
static int budget_boundary_check(void) {
#define BUDGET_CHECK(c) do { if (!(c)) return __LINE__; } while (0)
    const uint64_t mib = UINT64_C(1048576), gib = UINT64_C(1073741824);
    pixaura_budget_request q = budget_request(), saved_q;
    pixaura_capability_profile p, saved_p;
    pixaura_budget_result out, prior, again;
    pixaura_decode_limits before, after; pixaura_working_limits working;
    BUDGET_CHECK(pixaura_decode_default_limits(1, &before) == 0);
    BUDGET_CHECK(pixaura_working_default_limits(1, &working) == 0);
    for (unsigned i = 0; i < 3; ++i) {
        p = budget_profile(i);
        BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0);
        BUDGET_CHECK(out.decision == PIXAURA_BUDGET_ADMIT && out.tier == i && out.backend == PIXAURA_BUDGET_CPU);
        BUDGET_CHECK(out.runtime_cap_bytes == (i == 0 ? 96*mib : (i == 1 ? 192*mib : 256*mib)));
        BUDGET_CHECK(out.peak_bytes == UINT64_C(1179648) && out.raster_bytes == 65536 && out.max_concurrent_work == 1);
        BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &again) == 0 && memcmp(&again, &out, sizeof(out)) == 0);
    }
    /* Independent threshold-minus-one/exact/plus-one expectations on EACH input. */
    for (unsigned tier = 1; tier <= 2; ++tier) {
        for (unsigned field = 0; field < 3; ++field) {
            for (int delta = -1; delta <= 1; ++delta) {
                p = budget_profile(tier);
                if (field == 0) p.hardware.logical_cpu_count = (tier == 1 ? 4u : 8u) + delta;
                if (field == 1) p.hardware.physical_memory_bytes = (tier == 1 ? 4*gib : 8*gib) + delta;
                if (field == 2) p.runtime.process_budget_bytes = (tier == 1 ? 256*mib : 512*mib) + delta;
                BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0);
                BUDGET_CHECK(out.tier == (delta < 0 ? tier - 1 : tier));
            }
        }
    }
    for (unsigned field = 0; field < 5; ++field) {
        p = budget_profile(2);
        if (field == 0) p.hardware.logical_cpu_count = 0;
        if (field == 1) p.hardware.physical_memory_bytes = 0;
        if (field == 2) p.runtime.process_budget_bytes = 0;
        if (field == 3) p.runtime.thermal = PIXAURA_THERMAL_UNKNOWN;
        if (field == 4) p.runtime.memory_pressure = PIXAURA_PRESSURE_UNKNOWN;
        BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.tier == PIXAURA_T0 && out.runtime_cap_bytes <= 128*mib);
    }
    p = budget_profile(2); saved_p = p; saved_q = q;
    for (unsigned field = 0; field < 9; ++field) {
        q = saved_q;
        if (field == 0) q.height = 0;
        if (field == 1) q.bytes_per_pixel = 4;
        if (field == 2) q.raster_count = 0;
        if (field == 3) q.work_class = UINT32_MAX;
        if (field == 4) q.prefer_gpu = 2;
        if (field == 5) q.allow_cpu_fallback = 2;
        if (field == 6) q.requested_handles = 0;
        if (field == 7) q.context_limit_bytes = 0;
        if (field == 8) q.preview_edge = 1;
        BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.decision == PIXAURA_BUDGET_REJECT && out.reason == PIXAURA_BUDGET_MALFORMED);
    }
    q = saved_q;
    q.width = 0; BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.reason == PIXAURA_BUDGET_MALFORMED);
    q = saved_q; q.width = UINT64_MAX; q.height = 2;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.reason == PIXAURA_BUDGET_OVERFLOW);
    q = saved_q; q.temporary_bytes = UINT64_MAX;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.reason == PIXAURA_BUDGET_OVERFLOW);
    q = saved_q; q.width = UINT64_MAX; q.height = 1;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.reason == PIXAURA_BUDGET_OVERFLOW);
    q = saved_q; q.width = UINT64_C(4294967296); q.height = UINT64_C(16777216); q.raster_count = UINT32_MAX;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.reason == PIXAURA_BUDGET_OVERFLOW);
    q = saved_q; q.retained_bytes = UINT64_MAX; q.inflight_bytes = 1;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.reason == PIXAURA_BUDGET_OVERFLOW);
    q = saved_q; q.width = 4096; q.height = 4096;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.reason == PIXAURA_BUDGET_HARD_CEILING);
    q = saved_q; q.context_limit_bytes = before.context_bytes + 1;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.reason == PIXAURA_BUDGET_HARD_CEILING);
    q = saved_q; q.scratch_bytes = before.scratch_bytes + 1;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.reason == PIXAURA_BUDGET_HARD_CEILING);
    q = saved_q; q.width = working.width + 1;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.reason == PIXAURA_BUDGET_HARD_CEILING);
    q = saved_q; q.context_limit_bytes = 2*mib;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.runtime_cap_bytes == 2*mib);
    q.temporary_bytes = mib;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.reason == PIXAURA_BUDGET_HARD_CEILING);
    q = saved_q; q.retained_handles = 63;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.decision == PIXAURA_BUDGET_ADMIT);
    q.inflight_handles = 1;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.reason == PIXAURA_BUDGET_HARD_CEILING);
    q = saved_q; q.admitted_work_count = 1;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.reason == PIXAURA_BUDGET_OCCUPANCY);
    q = saved_q; p = budget_profile(0); q.retained_bytes = 96*mib - UINT64_C(1179648);
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.peak_bytes == 96*mib && out.decision == PIXAURA_BUDGET_ADMIT);
    q.inflight_bytes = 1;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.reason == PIXAURA_BUDGET_PROCESS_BUDGET);
    q = saved_q; p.runtime.process_budget_bytes = 32*mib;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.decision == PIXAURA_BUDGET_REJECT && out.runtime_cap_bytes == 0);
    p = saved_p; q.work_class = PIXAURA_BUDGET_PREVIEW; q.preview_edge = 4096; q.width = 1024; q.height = 1024;
    p = budget_profile(0);
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.decision == PIXAURA_BUDGET_DOWNGRADE);
    BUDGET_CHECK(out.preview_width == 720 && out.preview_height == 720 && out.peak_bytes == 33*mib + UINT64_C(2073600));
    q.retained_bytes = 62*mib;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.reason == PIXAURA_BUDGET_PREVIEW_MEMORY && out.preview_width == 360);
    q.retained_bytes = 64*mib; /* Smaller output cannot rescue full-size working rasters. */
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.decision == PIXAURA_BUDGET_REJECT);
    q.retained_bytes = 0; q.width = 1000; q.height = 333;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.preview_width == 720 && out.preview_height == 239);
    q.width = 333; q.height = 1000;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.preview_width == 239 && out.preview_height == 720);
    q.width = 1; q.height = 1000; q.preview_edge = 1;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.preview_width == 1 && out.preview_height == 1);
    p = saved_p; q = saved_q; q.work_class = PIXAURA_BUDGET_IMPORT; q.bytes_per_pixel = 4; q.raster_count = 1; q.encoded_bytes = 100;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.peak_bytes == mib + 16384 + 100);
    q.encoded_bytes = before.encoded_bytes + 1;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.reason == PIXAURA_BUDGET_HARD_CEILING);
    q = saved_q; q.work_class = PIXAURA_BUDGET_GPU_DIAGNOSTIC; q.prefer_gpu = q.allow_cpu_fallback = 1;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.reason == PIXAURA_BUDGET_BACKEND_UNAVAILABLE && out.backend == PIXAURA_BUDGET_CPU);
    p.hardware.gpu_backend = PIXAURA_CAPABILITY_GPU_VULKAN;
    p.hardware.gpu_availability = p.runtime.gpu_availability = PIXAURA_CAPABILITY_AVAILABLE;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.reason == PIXAURA_BUDGET_BACKEND_UNVALIDATED);
    p.hardware.platform = PIXAURA_PLATFORM_ANDROID; p.hardware.provenance = PIXAURA_PROVENANCE_PHYSICAL;
    p.hardware.gpu_validation = PIXAURA_GPU_PHYSICAL_CERTIFIED;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.backend == PIXAURA_BUDGET_VULKAN && out.decision == PIXAURA_BUDGET_ADMIT);
    p.runtime.gpu_availability = PIXAURA_CAPABILITY_UNAVAILABLE;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.reason == PIXAURA_BUDGET_BACKEND_UNAVAILABLE && out.backend == PIXAURA_BUDGET_CPU);
    p.runtime.gpu_availability = PIXAURA_CAPABILITY_AVAILABLE;
    p.runtime.thermal = PIXAURA_THERMAL_UNKNOWN;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.reason == PIXAURA_BUDGET_UNKNOWN_FACTS && out.backend == PIXAURA_BUDGET_CPU);
    p.runtime.thermal = PIXAURA_THERMAL_NORMAL;
    q.width = 129; q.height = 128;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.reason == PIXAURA_BUDGET_HARD_CEILING);
    q = saved_q; q.work_class = PIXAURA_BUDGET_GPU_DIAGNOSTIC; q.prefer_gpu = q.allow_cpu_fallback = 1;
    p.hardware.platform = PIXAURA_PLATFORM_APPLE; p.hardware.gpu_backend = PIXAURA_CAPABILITY_GPU_METAL;
    p.hardware.provenance = PIXAURA_PROVENANCE_VIRTUAL; p.hardware.gpu_validation = PIXAURA_GPU_NOT_VALIDATED;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.backend == PIXAURA_BUDGET_CPU && out.reason == PIXAURA_BUDGET_BACKEND_UNVALIDATED);
    p.hardware.gpu_validation = PIXAURA_GPU_RUNTIME_VALIDATED;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.backend == PIXAURA_BUDGET_METAL);
    BUDGET_CHECK(p.hardware.provenance == PIXAURA_PROVENANCE_VIRTUAL && p.hardware.gpu_validation == PIXAURA_GPU_RUNTIME_VALIDATED);
    p.hardware.gpu_validation = PIXAURA_GPU_NOT_VALIDATED;
    q.allow_cpu_fallback = 0;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.decision == PIXAURA_BUDGET_REJECT);
    p = saved_p; q = saved_q; p.runtime.thermal = PIXAURA_THERMAL_SEVERE;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.tier == PIXAURA_T0 && out.runtime_cap_bytes == 64*mib && out.decision == PIXAURA_BUDGET_DOWNGRADE);
    p = saved_p; p.runtime.memory_pressure = PIXAURA_PRESSURE_ELEVATED;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 0 && out.tier == PIXAURA_T0 && out.reason == PIXAURA_BUDGET_PRESSURE);
    memset(&out, 0xa5, sizeof(out)); prior = out;
    p = saved_p; p.version = 2;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 2 && memcmp(&out, &prior, sizeof(out)) == 0);
    p = saved_p; q.version = 2;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 2 && memcmp(&out, &prior, sizeof(out)) == 0);
    q = saved_q; q.struct_size = 0;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 2 && memcmp(&out, &prior, sizeof(out)) == 0);
    BUDGET_CHECK(pixaura_budget_admit(2, &p, &q, &out) == 2);
    BUDGET_CHECK(pixaura_budget_admit(1, NULL, &q, &out) == 1 && pixaura_budget_admit(1, &p, NULL, &out) == 1);
    q = saved_q; p.runtime.process_budget_bytes = p.hardware.physical_memory_bytes + 1;
    BUDGET_CHECK(pixaura_budget_admit(1, &p, &q, &out) == 1 && memcmp(&out, &prior, sizeof(out)) == 0);
    BUDGET_CHECK(pixaura_decode_default_limits(1, &after) == 0 && memcmp(&before, &after, sizeof(before)) == 0);
    BUDGET_CHECK(working.image_bytes == 128*mib && working.pixel_count == 8388608 && before.context_bytes == 256*mib);
#undef BUDGET_CHECK
    return 0;
}
#endif
