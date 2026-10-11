# Phase 4 Step 2 — Budget Admission & Execution Tiers

Status: **IMPLEMENTED / VALIDATION IN PROGRESS**.
Baseline: `82fca1f6fe3a565a6704afe18cb5bc80cb9a9823`.
Product Owner freezes the seven-step Phase 4 roadmap and accepts Step 1 FULL PASS.
Only Step 2 is authorized; Steps 3–7 remain NOT STARTED. Phase 0–3 stay closed.

## Delivery

The [budget v1 contract](../contracts/budget-admission-v1.md) freezes shared
allocation-free, immutable-snapshot preflight with versioned request/result and
stable reason codes. No reservation, context access, duplicated counters,
publication owner, worker queue, policy persistence or real platform probe.
Existing context owners remain responsible for truthful occupancy, serialized
allocation and ALL operation-specific checks. No architecture conflict/new ADR.

T0: CPU/manual baseline, 128-MiB policy cap/720 preview edge. T1: at least
4 CPUs, 4 GiB physical RAM and 256 MiB process observation, NORMAL thermal and
pressure, unconstrained; 192 MiB/1280 edge. T2: 8 CPUs, 8 GiB, 512 MiB;
256 MiB/2048 edge. Concurrency stays one. Effective cap additionally minimizes
known RAM/16, process observation minus 32-MiB reserve, current context limit,
and 64 MiB for severe pressure/thermal. Unknown selection inputs cannot promote.
These constants are conservative policy, not physical-device measurements.

Synthetic fixtures LOW/MID/HIGH produce T0/96 MiB, T1/192 MiB, T2/256 MiB.
The same 64×64 two-raster CPU request admits all, peak exactly 1179648 bytes.
HIGH does not admit work over existing limits. Caps and operation-specific native
checks remain absolute authorities; no storage/decode/operation/preset/gesture/
scratch/context/handle/generation ceiling increase.

Work classes: IMPORT/decode, PREVIEW, COMMIT/reference render, optional bounded
GPU_DIAGNOSTIC. Checked 64-bit pixels/BPP/count/scratch/temporary/occupancy/encoded
accounting includes retained and in-flight capacities and 1-MiB control reserve.
Overflow rejects. Preview may fit/halve only RGBA8 display dimensions, with all
full-size working buffers still charged; no source/commit/parameter downgrade.
GPU is considered only for the diagnostic class; available AND runtime-validated
or reviewed physical evidence AND normal state, otherwise explicit CPU fallback
or rejection. Metal availability/runtime validation never claims physical
certification. No validated enum bypasses existing backend/parity checks.

## Tests and evidence

Shared C/C++/JNI corpus exercises fixture outcomes, every CPU/RAM/process tier
threshold minus-one/exact/plus-one, unknowns, contradictory/versioned profiles,
malformed request fields/version/size, multiplication/addition overflow, explicit
hard maxima, reduced context limit, byte/handle/work occupancy, exact-fit/one-byte
exhaustion, aspect ratios/one-pixel fit, preview memory fallback and full-source
failure, GPU-unavailable/unvalidated/Vulkan evidence representation/virtual Metal.
Tests use physical-provenance enum values solely to verify evidence representation;
they are NOT physical measurements or certification of the executing device.
Native stress checks 160000 parallel queries across eight independent callers,
comparing identical complete result records. No mutable state/reservation.
Swift macOS/iOS simulator tests assert independent expected integer outputs.
Android instrumentation invokes the argument-free SAME shared corpus through JNI.
No untrusted-input production injection, new UI or actual OS probe collection.

Local affected C/C++ tests were rebuilt and observed 2/2 PASS under ASan/UBSan.
An initial missing `<cstring>` test include caused compilation failure; it was
corrected and rebuilt before counting tests. No stale binary PASS is accepted.
Local Windows native regression passed 30/30; final affected admission tests
passed 2/2 with 160000 parallel queries. Local source/docs passed 86/86 (zero
skips), workflow lint/diff checks passed. Android strict debug/release/JVM/lint/
instrumentation-APK builds passed all requested tasks, including the final reason
correction rebuild. Real runtime instrumentation and Apple gates require hosted
execution and are not inferred from local builds.

Full local WSL ASan/UBSan passed 31/33, with active negative controls. The two
unchanged storage cases failed on `/mnt/g`'s 9p/DrvFS mount without POSIX metadata:
unwritable-directory rejection in the unprivileged child and the simulated
Android publication branch. No storage/security code or assertion is changed;
these failures are not counted PASS. Final rebuilt admission corpus passed 2/2
under ASan/UBSan. Hosted Linux must pass both unchanged filesystem cases and
all 33 sanitizer tests on its native filesystem before delivery.
Remaining exact-SHA hosted gates are pending and not counted PASS.
Implementation SHA `0606c4ee934ec89243dabfc1e96ed1fe6ed8073e` passed
Foundation 38162938140 (7/7), source 122/122 and Android runtime 18/18. Shells
38162938172 failed only its existing iOS UI rotation readiness predicate;
debug/release builds, six app tests and Swift 18/18 per runtime passed. The
captured hierarchy shows an enabled on-screen button with the horizontal
scrollbar accessibility region overlapping its default hit point. The coordinate
tap helper now checks enabled/existing plus its actual upper target on screen,
rather than gating that different coordinate on XCTest's default `isHittable`.
Actual tap, mandatory core-backed selection and every full editor/history/reopen
assertion remain. No production UI/domain code changes; no failed run is PASS.
Final delivery requires all seven Foundation and three Native application shell
jobs SUCCESS on the same final SHA, including Android runtime and iOS app/UI.
The delivery report supplies final SHA, workflow/job IDs and observed totals.

## Scope and review boundary

Files: budget.h/budget.cpp; shared budget boundary corpus; existing C/C++ test
consumers/concurrency; JNI diagnostic/CoreProbe and instrumentation; Swift budget
tests and Package source wiring; CMake; capability architecture checks; budget/
capability contracts, frozen roadmap and current ROADMAP/QUALITY_GATES/ARCHITECTURE/
DECISIONS/report entries. Historical Phase 3 closure and ADR 0021 remain unchanged.

A: finish Step 2 technical gates and exact-SHA closure; no known architecture conflict.
B: frozen Steps 3–7 after explicit authorization; DH-APPLE-METAL-01 at its existing
mandatory later hardening/Beta/production/physical-claim gates.
C: optional dashboards, device scores, benchmarks and product polish.
No schema change, Phase 5/inference/AI/model, network/telemetry, new manual tool,
preset catalog, numerical/UI feature or GPU-kernel work. DH-APPLE-METAL-01 remains
unchanged/deferred. STOP after Step 2; do not begin Step 3 automatically.
