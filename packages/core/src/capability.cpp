#include "pixaura/capability.h"
#include <cstring>

extern "C" int32_t pixaura_capability_validate(uint32_t version, const pixaura_capability_profile* p) {
    if (version != PIXAURA_CAPABILITY_VERSION) return PIXAURA_UNSUPPORTED_ABI;
    if (!p) return PIXAURA_INVALID_ARGUMENT;
    const auto& h = p->hardware;
    const auto& r = p->runtime;
    if (p->version != version || p->struct_size != sizeof(*p) || h.version != version ||
        h.struct_size != sizeof(h) || r.version != version || r.struct_size != sizeof(r)) return PIXAURA_UNSUPPORTED_ABI;
    if (h.platform > PIXAURA_PLATFORM_APPLE || h.architecture > PIXAURA_ARCH_X86_64 ||
        h.logical_cpu_count > PIXAURA_CAPABILITY_MAX_CPUS || h.gpu_backend > PIXAURA_CAPABILITY_GPU_METAL ||
        h.gpu_availability > PIXAURA_CAPABILITY_AVAILABLE || h.gpu_validation > PIXAURA_GPU_PHYSICAL_CERTIFIED ||
        h.provenance > PIXAURA_PROVENANCE_PHYSICAL || r.thermal > PIXAURA_THERMAL_SEVERE ||
        r.memory_pressure > PIXAURA_PRESSURE_SEVERE || r.constrained > 1 ||
        r.gpu_availability > PIXAURA_CAPABILITY_AVAILABLE) return PIXAURA_INVALID_ARGUMENT;
    if (h.gpu_backend == PIXAURA_CAPABILITY_GPU_NONE &&
        (h.gpu_availability == PIXAURA_CAPABILITY_AVAILABLE || r.gpu_availability == PIXAURA_CAPABILITY_AVAILABLE ||
         h.gpu_validation != PIXAURA_GPU_NOT_VALIDATED)) return PIXAURA_INVALID_ARGUMENT;
    if (h.gpu_availability != PIXAURA_CAPABILITY_AVAILABLE && h.gpu_validation != PIXAURA_GPU_NOT_VALIDATED) return PIXAURA_INVALID_ARGUMENT;
    if (h.gpu_availability == PIXAURA_CAPABILITY_UNAVAILABLE && r.gpu_availability == PIXAURA_CAPABILITY_AVAILABLE) return PIXAURA_INVALID_ARGUMENT;
    if (h.gpu_validation == PIXAURA_GPU_PHYSICAL_CERTIFIED && h.provenance != PIXAURA_PROVENANCE_PHYSICAL) return PIXAURA_INVALID_ARGUMENT;
    if (h.physical_memory_bytes != 0 && r.process_budget_bytes > h.physical_memory_bytes) return PIXAURA_INVALID_ARGUMENT;
    return PIXAURA_OK;
}

extern "C" int32_t pixaura_capability_normalize(uint32_t version,
    const pixaura_capability_static* h, const pixaura_capability_dynamic* r, pixaura_capability_profile* output) {
    if (version != PIXAURA_CAPABILITY_VERSION) return PIXAURA_UNSUPPORTED_ABI;
    if (!h || !r || !output) return PIXAURA_INVALID_ARGUMENT;
    const pixaura_capability_profile p{version, sizeof(pixaura_capability_profile), *h, *r};
    const auto status = pixaura_capability_validate(version, &p);
    if (status != PIXAURA_OK) return status;
    *output = p;
    return PIXAURA_OK;
}

extern "C" int32_t pixaura_capability_baseline(uint32_t version, pixaura_capability_profile* output) {
    if (version != PIXAURA_CAPABILITY_VERSION) return PIXAURA_UNSUPPORTED_ABI;
    if (!output) return PIXAURA_INVALID_ARGUMENT;
    pixaura_capability_profile p{};
    p.version = p.hardware.version = p.runtime.version = version;
    p.struct_size = sizeof(p); p.hardware.struct_size = sizeof(p.hardware); p.runtime.struct_size = sizeof(p.runtime);
#if defined(__ANDROID__)
    p.hardware.platform = PIXAURA_PLATFORM_ANDROID;
#elif defined(__APPLE__)
    p.hardware.platform = PIXAURA_PLATFORM_APPLE;
#elif defined(_WIN32)
    p.hardware.platform = PIXAURA_PLATFORM_WINDOWS;
#elif defined(__linux__)
    p.hardware.platform = PIXAURA_PLATFORM_LINUX;
#endif
#if defined(__aarch64__) || defined(_M_ARM64)
    p.hardware.architecture = PIXAURA_ARCH_ARM64;
#elif defined(__arm__) || defined(_M_ARM)
    p.hardware.architecture = PIXAURA_ARCH_ARM32;
#elif defined(__x86_64__) || defined(_M_X64)
    p.hardware.architecture = PIXAURA_ARCH_X86_64;
#elif defined(__i386__) || defined(_M_IX86)
    p.hardware.architecture = PIXAURA_ARCH_X86;
#endif
    p.runtime.constrained = 1;
    *output = p;
    return PIXAURA_OK;
}

extern "C" int32_t pixaura_capability_write(uint32_t version, const pixaura_capability_profile* p,
    uint8_t* output, uint64_t capacity, uint64_t* required_bytes) {
    if (!required_bytes) return PIXAURA_INVALID_ARGUMENT;
    const auto status = pixaura_capability_validate(version, p);
    if (status != PIXAURA_OK) return status;
    if (!output && capacity != 0) return PIXAURA_INVALID_ARGUMENT;
    if (!output || capacity < PIXAURA_CAPABILITY_WIRE_BYTES) { *required_bytes = PIXAURA_CAPABILITY_WIRE_BYTES; return 4; }
    uint8_t bytes[PIXAURA_CAPABILITY_WIRE_BYTES]{};
    unsigned offset = 0;
    const auto word = [&](uint64_t value, unsigned count) {
        for (unsigned i = 0; i < count; ++i) bytes[offset++] = static_cast<uint8_t>(value >> ((count - 1 - i) * 8));
    };
    const auto& h = p->hardware; const auto& r = p->runtime;
    word(version, 4); word(h.platform, 4); word(h.architecture, 4); word(h.logical_cpu_count, 4);
    word(h.runtime_version, 4); word(h.gpu_backend, 4); word(h.gpu_availability, 4); word(h.gpu_validation, 4);
    word(h.provenance, 4); word(h.physical_memory_bytes, 8); word(r.thermal, 4); word(r.memory_pressure, 4);
    word(r.constrained, 4); word(r.gpu_availability, 4); word(r.process_budget_bytes, 8); word(r.sequence, 8);
    std::memcpy(output, bytes, sizeof(bytes));
    *required_bytes = sizeof(bytes);
    return PIXAURA_OK;
}
