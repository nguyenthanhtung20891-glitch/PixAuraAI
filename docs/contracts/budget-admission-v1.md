# Budget admission v1 — Phase 4 Step 2

Authority: Product Owner freezes the seven-step Phase 4 roadmap and accepts
Step 1 at `82fca1f6fe3a565a6704afe18cb5bc80cb9a9823`; only Step 2 is authorized.
This contract freezes shared policy family 1. No dynamic transition orchestration,
platform probe collection, editor UI, inference or new resource ownership.

## One pure authority

`pixaura_budget_admit` consumes a validated CapabilityProfile v1 and versioned,
caller-owned `pixaura_budget_request`. It returns a detached fixed-size result.
No allocations, mutable globals, locks, callbacks, context pointers, reservations,
queues, OS queries or persistent state. Identical normalized inputs yield identical
integer results regardless of host platform/ABI. Querying concurrently cannot
reserve memory or mutate an editor, revision, generation or profile.

This is an additional preflight, never permission to bypass native resource,
decode, operation, scratch, context, handle, cancellation or backend checks.
Internal owners supply truthful peak capacities and serialize occupancy snapshot,
preflight and actual allocation under their existing authority; a result is not a
reservation and can become stale. Context destruction still requires stop/join/
quiescence. Step 2 does not install a second counter or scheduler. Later integration
must recheck owners, not assume concurrent ADMIT queries reserve independent room.
No UI chooses tiers or admission. No external diagnostic loader or JNI argument
accepts synthetic profiles. Profiles cannot exceed frozen native hard ceilings.

## Frozen integer tier thresholds

Selection uses ALL three measured inputs, plus NORMAL thermal and pressure and
`constrained=0`. A failed higher-tier predicate falls to the next satisfied tier.

| Execution tier | CPU minimum | Physical RAM minimum | Process observation minimum | Policy context cap | Preview edge bound | Concurrent work |
| --- | --- | --- | --- | --- | --- | --- |
| T0 | No required observation | No required observation | No required observation | 128 MiB | 720 | 1 |
| T1 | 4 logical CPUs | 4 GiB | 256 MiB | 192 MiB | 1280 | 1 |
| T2 | 8 logical CPUs | 8 GiB | 512 MiB | 256 MiB | 2048 | 1 |

MiB=1048576, GiB=1073741824; every threshold is integer. CPU+RAM+process minima
prevent a single large metric from upgrading an otherwise constrained device.
T1 reserves one quarter of the 256-MiB hard context maximum; T2 can reach that
maximum but never exceed it. Preview bounds reuse historical targets, not new
pixel semantics or UI controls. Concurrency remains one: no expansion of existing
serialized execution/publication. These are conservative policy constants, not
benchmarks or claims about physical devices. Architecture/model names are not
selection criteria. Backend availability/trust affects optional backend requests,
not whether CPU-capable T2 exists.

Effective aggregate cap is the minimum of:

1. selected tier cap;
2. known physical RAM / 16 (integer floor), a conservative process working-set slice;
3. known process budget minus 32 MiB, saturating at zero, reserving UI/runtime headroom;
4. caller's positive existing context limit, never above native 256 MiB;
5. 64 MiB if thermal or memory pressure is SEVERE.

Unknown CPU/RAM/process/thermal/pressure facts cannot promote T1/T2. Unknown RAM
or process budget contributes no invented measured allowance; T0's 128-MiB policy
ceiling remains below native authority, with ordinary allocation failure/rejection
still mandatory. Unknown does not mean zero editing allowance. ELEVATED/SEVERE or
constrained snapshots select T0; severe additionally caps 64 MiB. This is a static
snapshot decision only: no hysteresis, callback, cancellation or policy epoch is
introduced here. Dynamic downgrade/recovery remains Step 3.

## Requests, accounting and absolute maxima

Request v1 includes work class, full source width/height, exact bytes per pixel,
new peak `raster_count`, encoded bytes, scratch, temporary buffers, retained and
in-flight byte capacities, retained/in-flight/requested handles, already admitted
work count, actual context limit, optional preview edge and GPU/fallback intent.
Capacities represent peak simultaneously live allocations, including source,
intermediates, output, metadata and platform/native work where relevant. Do not
count one allocation twice; do not omit retained source or superseded in-flight
work. Full working rasters are not converted into display-sized working buffers.

| Work | BPP | Existing absolute raster authority | Additional behavior |
| --- | --- | --- | --- |
| IMPORT | 4 | Decode defaults: 16384×16384, 33554432 pixels, 65536 row bytes, 128 MiB raster; encoded ≤64 MiB | Encoded bytes must be positive and are charged; no preview/GPU intent. |
| PREVIEW | 16 | Working defaults: 16384×16384, 8388608 pixels, 262144 row bytes, 128 MiB raster | Charge all full-size working rasters, then one bounded RGBA8 display. Positive requested preview edge. |
| COMMIT/reference render | 16 | Same working defaults | No dimension reduction, parameter change or optional GPU dispatch. |
| GPU_DIAGNOSTIC | 16 | Working defaults AND existing 16384-pixel tile ceiling | At least two rasters (input/output), zero scratch; optional GPU with explicit CPU fallback. No manual GPU pipeline expansion. |

Native decode/working getters supply maxima; they are not modified. Per-operation
stricter scratch limits remain native authority. Generic scratch cannot exceed
decode's existing 32 MiB. Positive context limit cannot exceed 256 MiB; combined
retained+in-flight+requested handles cannot exceed 64. Any already admitted work
(`admitted_work_count >= 1`) rejects OCCUPANCY rather than creating an update queue.
Requested handle count must be positive. Raster count must be positive. Invalid
enums/flags, zero dimensions/limits, wrong BPP or irrelevant fields reject MALFORMED.

Checked unsigned 64-bit multiply/add computes pixels, row, individual raster,
all requested rasters, retained+in-flight occupancy and peak. Every overflow
rejects OVERFLOW before comparisons, without wraparound. Raster sizes also fit
native size_t. Peak = occupancy + full rasters + scratch + temporary + encoded
bytes + 1 MiB conservative metadata/control reserve (+ preview display if any).
Oversized encoded/raster/scratch/dimensions/context/handles/base peak reject hard
ceilings even on HIGH. Runtime cap can only tighten those limits. No operation,
preset, preview generation, history, storage or decode ceiling is changed.

## Static choices and GPU trust

Manual/import/commit execution remains CPU/reference. For preview, start with
min(requested edge, tier edge); fit aspect ratio using integer floor with a minimum
one-pixel minor dimension, never upscale. If output cannot fit remaining aggregate
room, halve the edge (floor, minimum 1) and retry, at most 12 fixed attempts. Only
RGBA8 display allocation shrinks. If full-size working buffers cannot fit, reject;
never silently shrink source fidelity or committed semantics. Results include
exact output dimensions, policy preview bound, effective aggregate cap, available
room, full raster bytes and peak bytes. The tier is a resource policy, not a device label.

GPU selection is permitted ONLY for GPU_DIAGNOSTIC with prefer_gpu=1, static and
runtime availability AVAILABLE, validation at least RUNTIME_VALIDATED, NORMAL
thermal/pressure and unconstrained state. Otherwise return CPU DOWNGRADE when
explicit fallback is allowed, or REJECT with the backend/pressure reason.
Physical certification remains separate; runtime-validated virtual Metal does
not become physically certified. Existing adapters must still enforce backend
capability/parity and reviewed evidence scope. A supplied validation enum is not
authentication. No physical certification or runtime probe is performed by policy.
**DH-APPLE-METAL-01 is unchanged/deferred/mandatory at its existing later gates.**

## Versioned results and precedence

Decision 0=ADMIT, 1=ADMIT_WITH_DOWNGRADE, 2=REJECT_RESOURCE_LIMIT. Backend 0=CPU,
1=VULKAN, 2=METAL; tier 0/1/2=T0/T1/T2. All reason values are stable in family 1:

| Code | Reason |
| --- | --- |
| 0 | BASELINE |
| 1 | HIGHER_TIER |
| 2 | PREVIEW_BOUND |
| 3 | PREVIEW_MEMORY |
| 4 | BACKEND_UNAVAILABLE |
| 5 | BACKEND_UNVALIDATED |
| 6 | PRESSURE |
| 7 | UNKNOWN_FACTS |
| 8 | HARD_CEILING |
| 9 | PROCESS_BUDGET (effective policy cap, including RAM/context constraints) |
| 10 | OVERFLOW |
| 11 | MALFORMED |
| 12 | OCCUPANCY |

ABI/null/profile errors return existing status 1/2 and preserve result. Unsupported
request version/size likewise preserves result. A versioned request with malformed
fields returns status OK plus REJECT/MALFORMED. Resource rejection is a successful
query with REJECT, never a successful edit. Rejected result fields grant no authority.
Precedence: ABI/profile → request shape → arithmetic overflow → hard limits →
occupied work slot → effective cap → GPU availability/trust/pressure → preview
fit. For admitted work, preview-memory then preview-bound describes its concrete
strategy; otherwise backend downgrade then pressure/unknown/tier describes admission.
No free-form string controls semantics. No serialized policy, new durable schema,
model/inference work class, device identifier, network, telemetry or dependency.

## Synthetic and platform evidence

Step 1 labels remain SYNTHETIC POLICY PROFILE fixtures, not measured devices:
LOW (2 CPUs/2 GiB/128 MiB) → T0/96 MiB; MID (4/4 GiB/256 MiB) → T1/192 MiB;
HIGH (8/8 GiB/1 GiB) → T2/256 MiB. A small 64×64 two-raster CPU request admits
all three with exact peak 1179648 bytes. Oversized HIGH requests reject.
Native C/C++ and Android argument-free JNI diagnostics execute the SAME corpus:
each threshold ±1, unknown/contradictory/versioned facts, overflow, retained/in-flight
occupancy, preview fit/exhaustion, CPU/GPU trust and unchanged default limits.
Swift macOS/iOS tests use independently declared expected integer results.
Concurrent calls compare every result with identical queries; no memory reservation
or context mutation occurs. T0's existing manual flows are regression-tested;
cross-tier persisted replay and dynamic race orchestration remain Steps 3/6.
