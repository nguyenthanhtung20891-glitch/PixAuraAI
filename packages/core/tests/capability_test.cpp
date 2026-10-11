#include "capability_boundary.h"
#include <atomic>
#include <cstdio>
#include <thread>
#include <vector>
int main() {
    if (capability_boundary_check() != 0) return 1;
    pixaura_capability_profile p{};
    if (pixaura_capability_baseline(1, &p) != 0) return 1;
    uint32_t* fields[] = {&p.hardware.platform, &p.hardware.architecture,
        &p.hardware.logical_cpu_count, &p.hardware.gpu_backend, &p.hardware.gpu_availability,
        &p.hardware.gpu_validation, &p.hardware.provenance, &p.runtime.thermal,
        &p.runtime.memory_pressure, &p.runtime.constrained, &p.runtime.gpu_availability};
    for (auto* field : fields) {
        const auto prior = *field; *field = UINT32_MAX;
        if (pixaura_capability_validate(1, &p) != PIXAURA_INVALID_ARGUMENT) return 1;
        *field = prior;
    }
    std::atomic<unsigned> failures{0};
    std::vector<std::thread> workers;
    for (unsigned lane = 0; lane < 8; ++lane) workers.emplace_back([&] {
        for (unsigned i = 0; i < 10000; ++i) {
            pixaura_capability_profile p{}, result{};
            uint8_t bytes[PIXAURA_CAPABILITY_WIRE_BYTES]{};
            uint64_t required = 0;
            if (pixaura_capability_baseline(1, &p) != 0) { ++failures; continue; }
            p.runtime.sequence = i;
            if (pixaura_capability_normalize(1, &p.hardware, &p.runtime, &result) != 0 ||
                result.runtime.sequence != i || pixaura_capability_write(1, &result, bytes, sizeof(bytes), &required) != 0 ||
                required != sizeof(bytes) || bytes[75] != static_cast<uint8_t>(i)) ++failures;
        }
    });
    for (auto& worker : workers) worker.join();
    if (failures != 0) return 1;
    puts("Capability immutable snapshots: 80000 parallel normalize/serialize calls PASS; no shared mutable state");
    return 0;
}
