#include "pixaura/budget.h"
#include "pixaura/decode.h"
#include "pixaura/working.h"
#include "pixaura/gpu.h"
#include <algorithm>
#include <limits>

namespace {
constexpr uint64_t mib = UINT64_C(1048576), gib = UINT64_C(1073741824);
bool add(uint64_t a, uint64_t b, uint64_t& out) {
    if (b > UINT64_MAX - a) return false;
    out = a + b; return true;
}
bool mul(uint64_t a, uint64_t b, uint64_t& out) {
    if (b && a > UINT64_MAX / b) return false;
    out = a * b; return true;
}
}
extern "C" int32_t pixaura_budget_admit(uint32_t version,
    const pixaura_capability_profile* p, const pixaura_budget_request* q,
    pixaura_budget_result* output) {
    if (version != PIXAURA_BUDGET_VERSION) return PIXAURA_UNSUPPORTED_ABI;
    if (!q || !output) return PIXAURA_INVALID_ARGUMENT;
    const auto status = pixaura_capability_validate(PIXAURA_CAPABILITY_VERSION, p);
    if (status != PIXAURA_OK) return status;
    if (q->version != version || q->struct_size != sizeof(*q)) return PIXAURA_UNSUPPORTED_ABI;
    pixaura_budget_result out{};
    out.version = version; out.struct_size = sizeof(out);
    out.decision = PIXAURA_BUDGET_REJECT; out.max_concurrent_work = 1;
    const auto finish = [&](uint32_t reason) { out.reason = reason; *output = out; return PIXAURA_OK; };
    const auto& h = p->hardware; const auto& r = p->runtime;
    const bool normal = r.thermal == PIXAURA_THERMAL_NORMAL &&
        r.memory_pressure == PIXAURA_PRESSURE_NORMAL && !r.constrained;
    if (normal && h.logical_cpu_count >= 4 && h.physical_memory_bytes >= 4*gib && r.process_budget_bytes >= 256*mib)
        out.tier = PIXAURA_T1;
    if (normal && h.logical_cpu_count >= 8 && h.physical_memory_bytes >= 8*gib && r.process_budget_bytes >= 512*mib)
        out.tier = PIXAURA_T2;
    const uint64_t caps[] = {128*mib, 192*mib, 256*mib};
    const uint32_t edges[] = {720, 1280, 2048};
    out.preview_edge = edges[out.tier];
    out.runtime_cap_bytes = caps[out.tier];
    if (h.physical_memory_bytes) out.runtime_cap_bytes = std::min(out.runtime_cap_bytes, h.physical_memory_bytes / 16);
    if (r.process_budget_bytes) out.runtime_cap_bytes = std::min(out.runtime_cap_bytes,
        r.process_budget_bytes > 32*mib ? r.process_budget_bytes - 32*mib : uint64_t(0));
    if (r.thermal == PIXAURA_THERMAL_SEVERE || r.memory_pressure == PIXAURA_PRESSURE_SEVERE)
        out.runtime_cap_bytes = std::min(out.runtime_cap_bytes, 64*mib);
    pixaura_decode_limits decode{}; pixaura_working_limits working{};
    pixaura_decode_default_limits(1, &decode); pixaura_working_default_limits(1, &working);
    const bool imported = q->work_class == PIXAURA_BUDGET_IMPORT;
    const bool preview = q->work_class == PIXAURA_BUDGET_PREVIEW;
    const bool gpu = q->work_class == PIXAURA_BUDGET_GPU_DIAGNOSTIC;
    if (q->work_class > PIXAURA_BUDGET_GPU_DIAGNOSTIC || !q->width || !q->height ||
        q->bytes_per_pixel != (imported ? 4u : 16u) || !q->raster_count || q->prefer_gpu > 1 ||
        q->allow_cpu_fallback > 1 || (!gpu && (q->prefer_gpu || q->allow_cpu_fallback)) ||
        (gpu && (q->raster_count < 2 || q->scratch_bytes)) ||
        (preview ? !q->preview_edge : q->preview_edge != 0) ||
        (imported ? !q->encoded_bytes : q->encoded_bytes != 0) ||
        !q->requested_handles || !q->context_limit_bytes)
        return finish(PIXAURA_BUDGET_MALFORMED);
    uint64_t pixels = 0, row = 0, rasters = 0, used = 0, fixed = 0, peak = 0;
    if (!mul(q->width, q->height, pixels) || !mul(q->width, q->bytes_per_pixel, row) ||
        !mul(pixels, q->bytes_per_pixel, out.raster_bytes) || !mul(out.raster_bytes, q->raster_count, rasters) ||
        !add(q->retained_bytes, q->inflight_bytes, used) || !add(rasters, q->scratch_bytes, fixed) ||
        !add(fixed, q->temporary_bytes, fixed) || !add(fixed, q->encoded_bytes, fixed) ||
        !add(fixed, mib, fixed) || !add(used, fixed, peak)) return finish(PIXAURA_BUDGET_OVERFLOW);
    const uint64_t handles = uint64_t(q->retained_handles) + q->inflight_handles + q->requested_handles;
    if (q->context_limit_bytes > decode.context_bytes || q->width > (imported ? decode.width : working.width) ||
        q->height > (imported ? decode.height : working.height) ||
        pixels > (imported ? decode.pixel_count : working.pixel_count) ||
        row > (imported ? decode.row_stride : working.row_stride) ||
        out.raster_bytes > (imported ? decode.decoded_bytes : working.image_bytes) ||
        out.raster_bytes > std::numeric_limits<std::size_t>::max() || q->scratch_bytes > decode.scratch_bytes ||
        q->encoded_bytes > decode.encoded_bytes || (gpu && pixels > PIXAURA_GPU_MAX_PIXELS) ||
        handles > 64 || peak > q->context_limit_bytes)
        return finish(PIXAURA_BUDGET_HARD_CEILING);
    out.runtime_cap_bytes = std::min(out.runtime_cap_bytes, q->context_limit_bytes);
    out.available_bytes = used < out.runtime_cap_bytes ? out.runtime_cap_bytes - used : 0;
    if (q->admitted_work_count >= out.max_concurrent_work) return finish(PIXAURA_BUDGET_OCCUPANCY);
    if (peak > out.runtime_cap_bytes) return finish(PIXAURA_BUDGET_PROCESS_BUDGET);
    out.peak_bytes = peak;
    uint32_t reason = out.tier ? PIXAURA_BUDGET_HIGHER_TIER : PIXAURA_BUDGET_BASELINE;
    bool downgraded = false;
    if (!h.logical_cpu_count || !h.physical_memory_bytes || !r.process_budget_bytes ||
        r.thermal == PIXAURA_THERMAL_UNKNOWN || r.memory_pressure == PIXAURA_PRESSURE_UNKNOWN)
        reason = PIXAURA_BUDGET_UNKNOWN_FACTS;
    if (r.constrained || r.thermal >= PIXAURA_THERMAL_ELEVATED || r.memory_pressure >= PIXAURA_PRESSURE_ELEVATED) {
        reason = PIXAURA_BUDGET_PRESSURE;
        downgraded = true;
    }
    if (gpu && q->prefer_gpu) {
        const bool available = h.gpu_availability == PIXAURA_CAPABILITY_AVAILABLE &&
            r.gpu_availability == PIXAURA_CAPABILITY_AVAILABLE;
        const bool validated = h.gpu_validation >= PIXAURA_GPU_RUNTIME_VALIDATED;
        if (normal && available && validated) out.backend = h.gpu_backend;
        else {
            reason = !available ? PIXAURA_BUDGET_BACKEND_UNAVAILABLE :
                (!validated ? PIXAURA_BUDGET_BACKEND_UNVALIDATED :
                 (r.constrained || r.thermal >= PIXAURA_THERMAL_ELEVATED || r.memory_pressure >= PIXAURA_PRESSURE_ELEVATED ?
                  PIXAURA_BUDGET_PRESSURE : PIXAURA_BUDGET_UNKNOWN_FACTS));
            if (!q->allow_cpu_fallback) return finish(reason);
            downgraded = true;
        }
    }
    if (preview) {
        uint32_t edge = std::min(q->preview_edge, out.preview_edge);
        // Only the RGBA8 display output shrinks. Full-size working rasters above
        // remain charged: existing preview evaluates the full source first.
        for (unsigned attempt = 0; attempt < 12; ++attempt) {
            uint64_t w = q->width, height = q->height;
            if (w > edge || height > edge) {
                if (w >= height) { height = std::max(UINT64_C(1), height * edge / w); w = edge; }
                else { w = std::max(UINT64_C(1), w * edge / height); height = edge; }
            }
            uint64_t display = w * height * 4, candidate = 0;
            if (!add(peak, display, candidate)) return finish(PIXAURA_BUDGET_OVERFLOW);
            if (candidate <= out.runtime_cap_bytes && candidate <= q->context_limit_bytes) {
                out.preview_width = static_cast<uint32_t>(w); out.preview_height = static_cast<uint32_t>(height);
                out.peak_bytes = candidate;
                if (edge < std::min(q->preview_edge, out.preview_edge)) { reason = PIXAURA_BUDGET_PREVIEW_MEMORY; downgraded = true; }
                else if (w < q->width || height < q->height) { reason = PIXAURA_BUDGET_PREVIEW_BOUND; downgraded = true; }
                break;
            }
            if (edge == 1) return finish(PIXAURA_BUDGET_PROCESS_BUDGET);
            edge = std::max(1u, edge / 2);
        }
        if (!out.preview_width) return finish(PIXAURA_BUDGET_PROCESS_BUDGET);
    }
    out.decision = downgraded ? PIXAURA_BUDGET_DOWNGRADE : PIXAURA_BUDGET_ADMIT;
    return finish(reason);
}
