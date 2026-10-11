#ifndef PIXAURA_CAPABILITY_BOUNDARY_H
#define PIXAURA_CAPABILITY_BOUNDARY_H
#include "capability_fixtures.h"
#include "pixaura/decode.h"
#include <string.h>
static int capability_boundary_check(void) {
    pixaura_capability_profile p, saved, output;
    pixaura_decode_limits before, after;
    uint8_t bytes[PIXAURA_CAPABILITY_WIRE_BYTES + 1], second[PIXAURA_CAPABILITY_WIRE_BYTES];
    uint64_t required;
#define CAP_CHECK(condition) do { if (!(condition)) return __LINE__; } while (0)
    CAP_CHECK(pixaura_decode_default_limits(1, &before) == 0);
    CAP_CHECK(pixaura_capability_baseline(1, &p) == 0);
    CAP_CHECK(pixaura_capability_validate(1, &p) == 0);
    CAP_CHECK(p.hardware.logical_cpu_count == 0 && p.hardware.physical_memory_bytes == 0);
    CAP_CHECK(p.hardware.gpu_validation == PIXAURA_GPU_NOT_VALIDATED && p.runtime.constrained == 1);
    CAP_CHECK(p.runtime.thermal == PIXAURA_THERMAL_UNKNOWN && p.runtime.process_budget_bytes == 0);
    for (unsigned i = 0; i < 3; ++i) {
        p.hardware.provenance = PIXAURA_PROVENANCE_SYNTHETIC;
        p.hardware.platform = PIXAURA_PLATFORM_UNKNOWN; p.hardware.architecture = PIXAURA_ARCH_UNKNOWN;
        p.hardware.logical_cpu_count = capability_fixtures[i].cpus;
        p.hardware.physical_memory_bytes = capability_fixtures[i].memory;
        p.runtime.process_budget_bytes = capability_fixtures[i].budget;
        p.runtime.constrained = 0; p.runtime.thermal = PIXAURA_THERMAL_NORMAL; p.runtime.memory_pressure = PIXAURA_PRESSURE_NORMAL;
        p.runtime.sequence = UINT64_C(0x0102030405060708);
        CAP_CHECK(pixaura_capability_normalize(1, &p.hardware, &p.runtime, &output) == 0);
        CAP_CHECK(output.runtime.process_budget_bytes == capability_fixtures[i].budget);
        memset(bytes, 0xa5, sizeof(bytes)); required = 99;
        CAP_CHECK(pixaura_capability_write(1, &p, bytes, PIXAURA_CAPABILITY_WIRE_BYTES - 1, &required) == 4);
        CAP_CHECK(required == PIXAURA_CAPABILITY_WIRE_BYTES && bytes[0] == 0xa5);
        CAP_CHECK(pixaura_capability_write(1, &p, bytes, sizeof(bytes), &required) == 0);
        CAP_CHECK(bytes[0] == 0 && bytes[3] == 1 && bytes[15] == capability_fixtures[i].cpus);
        CAP_CHECK(bytes[68] == 1 && bytes[75] == 8 && bytes[PIXAURA_CAPABILITY_WIRE_BYTES] == 0xa5);
        if (i == 0) {
            const uint8_t golden[76] = {
                0,0,0,1, 0,0,0,0, 0,0,0,0, 0,0,0,2, 0,0,0,0,
                0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,1,
                0,0,0,0,128,0,0,0,
                0,0,0,1, 0,0,0,1, 0,0,0,0, 0,0,0,0,
                0,0,0,0,8,0,0,0, 1,2,3,4,5,6,7,8
            };
            CAP_CHECK(memcmp(bytes, golden, sizeof(golden)) == 0);
        }
        CAP_CHECK(pixaura_capability_write(1, &output, second, sizeof(second), &required) == 0);
        CAP_CHECK(memcmp(bytes, second, sizeof(second)) == 0);
    }
    saved = p; output = p;
    p.runtime.memory_pressure = 99;
    CAP_CHECK(pixaura_capability_normalize(1, &p.hardware, &p.runtime, &output) == 1);
    CAP_CHECK(memcmp(&output, &saved, sizeof(saved)) == 0);
    required = 99; memset(bytes, 0xa5, sizeof(bytes));
    CAP_CHECK(pixaura_capability_write(1, &p, bytes, sizeof(bytes), &required) == 1);
    CAP_CHECK(required == 99 && bytes[0] == 0xa5);
    p = saved; p.hardware.logical_cpu_count = PIXAURA_CAPABILITY_MAX_CPUS + 1;
    CAP_CHECK(pixaura_capability_validate(1, &p) == 1);
    p = saved; p.hardware.version = 2;
    CAP_CHECK(pixaura_capability_validate(1, &p) == 2);
    p = saved; p.runtime.struct_size = 0;
    CAP_CHECK(pixaura_capability_validate(1, &p) == 2);
    p = saved; p.runtime.process_budget_bytes = UINT64_MAX;
    CAP_CHECK(pixaura_capability_validate(1, &p) == 1);
    p = saved; p.hardware.gpu_backend = PIXAURA_CAPABILITY_GPU_VULKAN;
    p.hardware.gpu_availability = p.runtime.gpu_availability = PIXAURA_CAPABILITY_AVAILABLE;
    CAP_CHECK(pixaura_capability_validate(1, &p) == 0); /* available != validated */
    p.hardware.gpu_validation = PIXAURA_GPU_PHYSICAL_CERTIFIED;
    CAP_CHECK(pixaura_capability_validate(1, &p) == 1); /* synthetic cannot certify */
    p.hardware.gpu_validation = PIXAURA_GPU_RUNTIME_VALIDATED;
    CAP_CHECK(pixaura_capability_validate(1, &p) == 0);
    p.hardware.provenance = PIXAURA_PROVENANCE_VIRTUAL; p.hardware.gpu_validation = PIXAURA_GPU_PHYSICAL_CERTIFIED;
    CAP_CHECK(pixaura_capability_validate(1, &p) == 1);
    p = saved; p.runtime.thermal = PIXAURA_THERMAL_SEVERE; p.runtime.memory_pressure = PIXAURA_PRESSURE_SEVERE;
    CAP_CHECK(pixaura_capability_validate(1, &p) == 0); /* observation, not a downgrade service */
    CAP_CHECK(pixaura_capability_baseline(2, &output) == 2);
    CAP_CHECK(pixaura_capability_baseline(1, NULL) == 1 && pixaura_capability_validate(1, NULL) == 1);
    CAP_CHECK(pixaura_decode_default_limits(1, &after) == 0);
    CAP_CHECK(memcmp(&before, &after, sizeof(before)) == 0);
#undef CAP_CHECK
    return 0;
}
#endif
