#include "capability_boundary.h"
#include "budget_boundary.h"
#include "dynamic_policy_boundary.h"
#include <atomic>
#include <cstdio>
#include <cstring>
#include <thread>
#include <vector>
int main() {
    const auto dynamic_status=dynamic_policy_boundary_check();
    if(dynamic_status){std::fprintf(stderr,"Dynamic policy failed at line %d\n",dynamic_status);return 1;}
    if (capability_boundary_check() != 0) return 1;
    const int budget_status = budget_boundary_check();
    if (budget_status != 0) { std::fprintf(stderr, "Budget failed at line %d\n", budget_status); return 1; }
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
            auto profile = budget_profile(i % 3);
            auto request = budget_request();
            pixaura_budget_result admitted{}, repeated{};
            if (pixaura_budget_admit(1, &profile, &request, &admitted) != 0 || admitted.tier != i % 3 ||
                pixaura_budget_admit(1, &profile, &request, &repeated) != 0 ||
                std::memcmp(&admitted, &repeated, sizeof(admitted)) != 0) ++failures;
        }
    });
    for (auto& worker : workers) worker.join();
    if (failures != 0) return 1;
    puts("Capability immutable snapshots: 80000 parallel normalize/serialize calls PASS; no shared mutable state");
    puts("Budget deterministic snapshots: 160000 parallel admission queries PASS; no reservations or mutable state");
    pixaura_decode_context context{};pixaura_decode_limits limits{};uint8_t identity[32];std::memset(identity,'f',32);identity[31]='0';
    auto profile=budget_profile(2);profile.hardware.gpu_backend=PIXAURA_CAPABILITY_GPU_VULKAN;
    profile.hardware.gpu_availability=2;profile.hardware.gpu_validation=1;profile.runtime.gpu_availability=2;
    auto observation=policy_observation(&profile);observation.gpu_availability=2;pixaura_execution_strategy strategy{};
    if(pixaura_decode_default_limits(1,&limits)||pixaura_decode_context_init(1,&context,sizeof(context),identity,32,&limits)||
        pixaura_policy_configure(1,&context,&profile,&observation,&strategy))return 1;
    std::atomic<uint64_t> sequence{1};workers.clear();
    for(unsigned lane=0;lane<8;++lane)workers.emplace_back([&,lane]{
        for(unsigned i=0;i<10000;++i){auto o=observation;o.sequence=sequence.fetch_add(1)+1;
            if(lane%2)o.thermal=(i+lane)%4?PIXAURA_POLICY_NORMAL:PIXAURA_POLICY_SEVERE;
            else o.memory_pressure=(i+lane)%4?PIXAURA_POLICY_NORMAL:PIXAURA_POLICY_CRITICAL;
            o.gpu_availability=i%7?2:1;o.gpu_validation=i%13?1:0;
            pixaura_execution_strategy local{};const auto code=pixaura_policy_update(1,&context,&o,&local);
            if(code!=0&&code!=9)++failures;
            if(code==0&&(local.tier>2||local.runtime_cap_bytes>268435456||local.concurrency!=1))++failures;
            auto q=budget_request();pixaura_budget_result admitted{};uint64_t epoch=0;
            if(pixaura_policy_admit(1,&context,&q,&admitted,&epoch)||!epoch||admitted.runtime_cap_bytes>268435456)++failures;
        }
    });
    for(auto& worker:workers)worker.join();
    // Backend loss/stop races updates; only quiescent destruction is authorized.
    std::thread updater([&]{for(unsigned i=0;i<1000;++i){auto o=observation;o.sequence=sequence.fetch_add(1)+1;
        o.gpu_availability=1;
        pixaura_execution_strategy local{};const auto code=pixaura_policy_update(1,&context,&o,&local);if(code!=0&&code!=13)++failures;}});
    if(pixaura_preview_stop(&context))return 1;
    updater.join();
    if(pixaura_decode_context_destroy(&context)||pixaura_policy_query(1,&context,&strategy)!=3||failures)return 1;
    puts("Dynamic policy: 80000 same-context updates + 80000 admissions, stale observations, stop/join/destroy PASS");
    return 0;
}
