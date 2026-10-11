# Capability profile foundation v1

Authority: authorized Phase 4 Step 1, baseline
`56cd60560b6271b7d27bf5e03fd1d720a3aad2fa`.
The [seven-step roadmap](../plans/phase-4-roadmap.md) is now FROZEN by Product Owner
at Step 2 authorization; Step 1 is accepted at `82fca1f6fe3a565a6704afe18cb5bc80cb9a9823`.
The additive [budget v1 contract](budget-admission-v1.md) consumes these unchanged
records. Only Step 2 is authorized; Steps 3–7 remain NOT STARTED.
This additive C ABI family establishes data/probe seams only. ABI 1 feature bits,
document/storage schemas, accepted manual tools and resource ceilings are unchanged.
No BudgetAdmission, device classifier, dynamic strategy or editor integration is
implemented in Step 1. Those require roadmap freeze and later authorization.

## Normalized snapshots

`pixaura_capability_profile` has version/struct_size and two versioned records:

| Record | Fields | Meaning |
| --- | --- | --- |
| Static hardware/runtime facts | platform, architecture, logical_cpu_count, runtime_version, physical_memory_bytes, gpu_backend, gpu_availability, gpu_validation, provenance | Facts for one probe lifetime/runtime. Platform and process ABI architecture are compile-target observations, not physical SoC identification; CPU/memory/API/backend observations require later real probes. No brand/model marketing classification. |
| Dynamic state | thermal, memory_pressure, constrained, gpu_availability, process_budget_bytes, sequence | Snapshot of live conditions. Budget is a safe platform estimate when actually available, not a promise of allocation or an admitted core limit. Sequence is diagnostic observation identity, not PRV1 generation or an approval token. |

CPU count 0, memory/budget 0 and runtime version 0 mean UNKNOWN, not absent
hardware or zero permitted editing. CPU count is bounded to 1024 for normalized
metadata sanity, not a thread allocation count. Integer memory values are bytes;
logical CPUs/API levels/enums are unsigned fixed-width integers. An observed
process budget greater than known physical memory is inconsistent and rejects.
Unknown physical memory permits an independently known process estimate.

Platform: UNKNOWN/WINDOWS/LINUX/ANDROID/APPLE. Architecture:
UNKNOWN/ARM32/ARM64/X86/X86_64. Availability: UNKNOWN/UNAVAILABLE/AVAILABLE.
GPU backend: NONE/VULKAN/METAL. Validation: NOT_VALIDATED/RUNTIME_VALIDATED/
PHYSICAL_CERTIFIED. Provenance: UNOBSERVED/SYNTHETIC/VIRTUAL/PHYSICAL.
Thermal and memory pressure: UNKNOWN/NORMAL/ELEVATED/SEVERE. `constrained` is 0/1;
the conservative baseline sets 1. Unknown enums, versions and sizes reject.

Availability is not validation. A backend may be available but unvalidated;
loss of current availability does not erase historical validation facts. NONE
cannot be available/validated; validation requires static availability. Synthetic
and virtual profiles cannot claim PHYSICAL_CERTIFIED. Structural validation is
**not authentication of hardware/certification claims**. Only internal trusted
adapters may eventually attach reviewed, scope-matched evidence. A supplied enum
never bypasses existing GPU checks, parity or physical certification gates.
No external profile loader or production synthetic-profile injection is exposed.

Static facts, dynamic facts and derived execution policy remain separate.
Step 2 freezes T0/T1/T2 multi-input thresholds and centralized
ADMIT/ADMIT_WITH_DOWNGRADE/REJECT_RESOURCE_LIMIT behavior in budget v1. Step 3 will freeze
policy epochs/recovery hysteresis and connect changes to existing PRV1 generation
ownership. Step 1 intentionally selects no tier or runtime budget.

## C ownership, normalization and diagnostics

`capability.h` uses fixed-width fields, caller-owned accessible storage, checked
family version 1 and exact local struct sizes; no STL/exception crosses C.
`baseline` reports compile-target platform/architecture and unknown probe facts,
NOT_VALIDATED GPU, constrained runtime, unknown thermal/pressure and no established
process budget. It performs no OS calls or hardware certification.
`normalize` validates separate records and atomically replaces the caller output
only on success. It makes no class/admission decision. `validate` checks shape and
internal consistency. All are allocation-free and have no mutable global state,
retained callbacks, handles, context ownership or background jobs.

Callers own immutable snapshots and synchronize any writes to their storage.
Distinct calls/snapshots are independent; simultaneous mutation of the same C
storage is invalid. No native context is created, so context-destruction/render
races belong to the later integration step, not an implied Step 1 implementation.
There is no allocation-failure path in this pure core foundation. Platform copy
failure returns failure; it cannot mutate a document or create a profile allowance.

`write` produces exactly **76 bytes**, unsigned big-endian, no strings, padding,
native pointers or native struct-size fields. Field order:

1. u32 profile version, platform, architecture, CPU count, runtime version,
   GPU backend, static availability, validation, provenance;
2. u64 physical memory bytes;
3. u32 thermal, pressure, constrained, runtime GPU availability;
4. u64 process budget bytes, sequence.

Canonical bytes are diagnostic/test data only. No deserializer, disk cache,
project serialization field, durable recovery authority or new schema is added.
Every call uses fixed stack storage, with one 76-byte temporary in `write`.
Status 0 = OK, 1 = invalid, 2 = unsupported family/version/size, 4 = insufficient
diagnostic buffer. Failure preserves outputs/required length except status 4
reports required=76 while preserving the byte buffer. Input/output storage and
length pointers must be truthful/accessibly sized; length output must not overlap
other storage. Snapshot inputs must not be concurrently modified.

## Platform seams and privacy

Android `CapabilityProbe` / `ConservativeCapabilityProbe` calls the shared C
baseline/write through JNI and owns defensive diagnostic copies. Swift's protocol
and value snapshot do the same through CPixAuraCore. These are explicit stubs,
not ActivityManager/thermal/Vulkan/ProcessInfo/Metal probes. No UI consumes them
yet and no platform defines thresholds/admission. Later adapters must use safe
internal APIs and normalize through this same C contract. Test seams cannot grant
production resource or certification authority.

Only justified numeric metadata exists. No model/brand/name, serial, tracking ID,
UUID, GPU name, user/image content, fingerprint identifier or sensor location is
collected. No network call, upload, telemetry, analytics, remote database, model
pack, accelerator provider or dependency is introduced. Data stays ephemeral/local.

## Synthetic fixtures and retained authorities

The test-only [fixture header](../../packages/core/tests/capability_fixtures.h)
provides SYNTHETIC POLICY PROFILE scenarios, **not tier thresholds**:

| Label | Logical CPUs | Physical RAM | Process estimate |
| --- | --- | --- | --- |
| LOW | 2 | 2 GiB | 128 MiB |
| MID | 4 | 4 GiB | 256 MiB |
| HIGH | 8 | 8 GiB | 1 GiB |

The HIGH estimate deliberately exceeds the existing 256-MiB context ceiling;
normalizing an observation cannot raise that ceiling. No fixture is a physical
device profile, device benchmark, selected execution tier or T0 pressure proof.
The shared C fixture corpus executes on native hosts and Android JNI diagnostics.
Swift verifies baseline/unknown/uncertified/malformed and detached boundaries.

T0 means the safest CPU/manual baseline, independent of GPU/NPU/AI/network.
The already accepted import/edit/preview/commit/cancel/history/reopen authority
remains intact; no new admission decision or preview rescaling occurs here.
Frozen 128-MiB raster/8388608 pixels/256-MiB context/64 handles, operation/history,
scratch, platform preview and preset/gesture bounds remain authoritative.
Phase 4 budgets may only lower them. Unknown facts never grant extra resources.

Accepted Android Z Fold7 Vulkan certification remains separate, scope-limited
historical evidence. The baseline does not infer availability or a HIGH physical
profile from that device name. Apple simulator/Metal availability is not physical
certification. **DH-APPLE-METAL-01 remains unchanged and DEFERRED**, mandatory
before Phase 11 hardening acceptance, Phase 12 Beta readiness completion,
production release or any physical Apple GPU certification claim.
