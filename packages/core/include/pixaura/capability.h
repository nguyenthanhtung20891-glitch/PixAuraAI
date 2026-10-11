#ifndef PIXAURA_CAPABILITY_H
#define PIXAURA_CAPABILITY_H
#include "core.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PIXAURA_CAPABILITY_VERSION 1u
#define PIXAURA_CAPABILITY_WIRE_BYTES 76u
#define PIXAURA_CAPABILITY_MAX_CPUS 1024u
/* Unknown is explicit, never an assertion of availability or safe admission. */
enum pixaura_capability_platform { PIXAURA_PLATFORM_UNKNOWN, PIXAURA_PLATFORM_WINDOWS,
    PIXAURA_PLATFORM_LINUX, PIXAURA_PLATFORM_ANDROID, PIXAURA_PLATFORM_APPLE };
enum pixaura_capability_architecture { PIXAURA_ARCH_UNKNOWN, PIXAURA_ARCH_ARM32,
    PIXAURA_ARCH_ARM64, PIXAURA_ARCH_X86, PIXAURA_ARCH_X86_64 };
enum pixaura_capability_availability { PIXAURA_CAPABILITY_UNKNOWN,
    PIXAURA_CAPABILITY_UNAVAILABLE, PIXAURA_CAPABILITY_AVAILABLE };
enum pixaura_capability_gpu { PIXAURA_CAPABILITY_GPU_NONE,
    PIXAURA_CAPABILITY_GPU_VULKAN, PIXAURA_CAPABILITY_GPU_METAL };
enum pixaura_capability_validation { PIXAURA_GPU_NOT_VALIDATED,
    PIXAURA_GPU_RUNTIME_VALIDATED, PIXAURA_GPU_PHYSICAL_CERTIFIED };
enum pixaura_capability_provenance { PIXAURA_PROVENANCE_UNOBSERVED,
    PIXAURA_PROVENANCE_SYNTHETIC, PIXAURA_PROVENANCE_VIRTUAL,
    PIXAURA_PROVENANCE_PHYSICAL };
enum pixaura_capability_thermal { PIXAURA_THERMAL_UNKNOWN, PIXAURA_THERMAL_NORMAL,
    PIXAURA_THERMAL_ELEVATED, PIXAURA_THERMAL_SEVERE };
enum pixaura_capability_pressure { PIXAURA_PRESSURE_UNKNOWN, PIXAURA_PRESSURE_NORMAL,
    PIXAURA_PRESSURE_ELEVATED, PIXAURA_PRESSURE_SEVERE };
typedef struct pixaura_capability_static {
    uint32_t version, struct_size, platform, architecture, logical_cpu_count;
    uint32_t runtime_version, gpu_backend, gpu_availability, gpu_validation, provenance;
    uint64_t physical_memory_bytes;
} pixaura_capability_static;
typedef struct pixaura_capability_dynamic {
    uint32_t version, struct_size, thermal, memory_pressure, constrained, gpu_availability;
    uint64_t process_budget_bytes, sequence;
} pixaura_capability_dynamic;
typedef struct pixaura_capability_profile {
    uint32_t version, struct_size;
    pixaura_capability_static hardware;
    pixaura_capability_dynamic runtime;
} pixaura_capability_profile;
/* Pure, allocation-free metadata foundation. Caller owns immutable snapshots;
 * no state, admission, certification authority, refresh or render is created.
 * Accessible C pointers and synchronization of caller-owned storage are required.
 * All failures preserve outputs (including required_bytes except sizing result).
 * Status: 1 invalid, 2 unsupported version/size, 4 insufficient diagnostic buffer.
 */
PIXAURA_API int32_t pixaura_capability_baseline(uint32_t version, pixaura_capability_profile* output);
PIXAURA_API int32_t pixaura_capability_validate(uint32_t version, const pixaura_capability_profile* profile);
PIXAURA_API int32_t pixaura_capability_normalize(uint32_t version,
    const pixaura_capability_static* hardware, const pixaura_capability_dynamic* runtime,
    pixaura_capability_profile* output);
/* Canonical big-endian diagnostic bytes, never native struct/padding bytes.
 * No deserializer or document persistence. A profile cannot grant resource/GPU
 * authority. Do not route external diagnostic input into production adapters.
 */
PIXAURA_API int32_t pixaura_capability_write(uint32_t version,
    const pixaura_capability_profile* profile, uint8_t* output,
    uint64_t capacity, uint64_t* required_bytes);
#ifdef __cplusplus
}
#endif
#endif
