# Android capability integration v1 — Phase 4 Step 4

Status: COMPLETE / FULL PASS. Authorized baseline
`6389750c5ccab3b40d3ee76cb29bbb88b0fb5596`; Steps 1–3 COMPLETE / FULL PASS.
The frozen seven-step roadmap is unchanged; Steps 5–7 NOT STARTED.
See [Step 4 report](../reports/phase-4-step-4.md). No unobserved gate is PASS.

## Ownership and observations

One `PixAuraApplication` owns `AndroidCapabilityOwner`, retaining Application only.
One thermal listener and one ComponentCallbacks2 registration are shared by at
most eight subscribers; first subscribe registers, final unsubscribe unregisters.
Closing the owner invalidates callback tokens before unregistering. Old callbacks
cannot affect a recreated subscription. No Activity, raw Java object, listener,
timer or callback is retained by native core. Native storage remains stable through
stop/join/quiesce/destroy. The application object itself lives for the process.

The model subscribes on its existing command worker and disposes on clearing.
Activity recreation retains the model/service subscription; screen disposal uses
the existing interruption/close semantics. New editor contexts configure fresh
policy with current observations. Process restart reprobes facts and derives a
fresh epoch. Neither snapshots nor epochs enter project storage or preferences.
Application ActivityLifecycleCallbacks count started activities without retaining
Activity references and supply foreground observations. Configuration replacement
does not falsely classify a process background transition. Actual backgrounding
is constrained; existing Step 6 interruption still cancels without auto-commit.

Facts: bounded Runtime.availableProcessors (invalid => unknown zero); native
compiled architecture, API SDK integer, ActivityManager.MemoryInfo.totalMem,
ordinary ActivityManager.memoryClass and Runtime.maxMemory, plus native Vulkan
availability. Total RAM is kernel-accessible RAM observation, not free process
memory. The process observation is the conservative minimum of ordinary memory
class MiB, managed heap maximum and known total RAM, computed in signed 64-bit
before checked unsigned JNI conversion. It is a conservative proxy, not measured
native free memory or permission to allocate. This app does not request largeHeap,
so largeMemoryClass is not an admission input. Missing facts remain unknown.

Static facts are copied once per owner. The profile NORMAL/unconstrained envelope defines only
the static recovery ceiling; actual initial pressure is supplied separately and
is never replaced by fabricated normal readings. Dynamic thermal, memory, constrained,
backend and process-budget observations remain separate. Shared Capability v1,
BudgetAdmission and dynamic policy v1 retain every frozen rule. No Android tier,
preview-edge, cap, recovery, operation or history table is added.

## Exact normalized mappings

Thermal API: PowerManager.currentThermalStatus and OnThermalStatusChangedListener,
API 29+. API 26–28, unavailable service or failed reading => UNKNOWN.

| Android thermal value | Shared state |
| --- | --- |
| NONE (0) | NORMAL (1) |
| LIGHT (1), MODERATE (2) | ELEVATED (2) |
| SEVERE (3) | SEVERE (3) |
| CRITICAL (4), EMERGENCY (5), SHUTDOWN (6) | CRITICAL (4) |
| Other/malformed/unsupported | UNKNOWN (0) |

| Android trim signal | Shared memory state |
| --- | --- |
| RUNNING_MODERATE (5) | ELEVATED (2) |
| RUNNING_LOW (10) | SEVERE (3) |
| RUNNING_CRITICAL (15), BACKGROUND (40), MODERATE (60), COMPLETE (80), onLowMemory | CRITICAL (4) |
| UI_HIDDEN (20) | Preserve memory observation; constrained background |
| Other/ambiguous | UNKNOWN (0) |

These historical callbacks remain handled for older supported OS versions.
Android API 34+ can omit non-UI trim notifications; absence is never normal-state
evidence. Fresh MemoryInfo sampling is an independent observation: lowMemory=true
or availMem<threshold => SEVERE; valid nonlow reading at/above positive threshold
=> NORMAL; unavailable, negative availability or nonpositive threshold => UNKNOWN.
New samples at preview-request opportunities refresh thermal/memory/budget/backend.
They provide actual distinct observations for the shared three-observation recovery;
there is no timer, fabricated stable count or Android recovery logic.

References: [PowerManager](https://developer.android.com/reference/android/os/PowerManager),
[ComponentCallbacks2](https://developer.android.com/reference/android/content/ComponentCallbacks2),
[MemoryInfo](https://developer.android.com/reference/android/app/ActivityManager.MemoryInfo).

## Vulkan and trust

Bounded native probing creates an instance, enumerates at most 32 physical-device
handles and 64 queue families per device, checks a nonempty compute queue, then
destroys the instance. No pipeline/dispatch, GPU name, raw identity or device
resource cache is created. Success means AVAILABLE only. Failure means UNAVAILABLE.
Sampling can observe backend loss/restoration, fed through shared normalized state.
Production Java input has NO validation/certification/provenance setter. JNI always
sets NOT_VALIDATED and UNOBSERVED; compile-time architecture comes from native.
CPU fallback remains authoritative where admission permits. Software/emulator
Vulkan availability cannot certify physical hardware.

Historical Z Fold7/Adreno 830 identity/exposure tile certification remains accepted
within its recorded scope. No current runtime identity matching has been observed,
so this integration does not elevate trust or generalize that evidence. No model
whitelist, chipset tier, fingerprint, serial, account identifier, analytics, remote
profile, telemetry, network or extra permission is collected.

## JNI, execution and publication

Internal JNI records use exact-length signed long arrays: facts six words and
observations seven words, starting with version 1. Negative values, wrong length,
version, enum, flag, CPU/API bound or zero sequence reject. Bytes remain 64-bit;
conversion occurs only after validation. Native builds the existing checked
CapabilityProfile/48-byte observation; strategy results are fixed eleven words.
These arrays are internal adapter arguments, never a deserializer/exported component.
Synthetic tests cannot set backend trust or raise native hard ceilings.

The additive policy-aware open configures admission BEFORE decode/working allocation;
insufficient initial budget rejects without publishing context/strategy or creating
a document database. The legacy open API remains unchanged. The platform-neutral
editor facade delegates configure/update/query to the
private raster policy without exposing a raw raster pointer. No document schema,
request ABI or shared policy constant changes. Platform lifecycle/display lock
covers updates and native-current plus display installation; rendering stays
outside that lock. Material shared changes revoke PRV1, and the same original
native-current check rejects late candidates. No Android publication generation
authority is introduced. Active parameters/history remain native-owned.

One pending snapshot and one drain runnable coalesce callbacks on the existing
command lane. Coalescing preserves the strongest pending pressure, constrained
state, unknown/lost backend and smallest budget; it cannot drop a downgrade while
the lane is busy. Fresh later samples may recover only through core hysteresis.
Repeated nonmaterial observations do not trigger redundant preview work. Signals
create no revision, auto-commit, operation rewrite or source resampling.

The existing editor display request is explicit FIT; core may shrink it, including
critical 360-edge. EXACT API semantics remain exact-or-resource-failure; Android
does not convert EXACT to FIT. Committed/reference CPU semantics stay unchanged.
Lower-budget rejection preserves prior valid detached display. Runtime caps only
lower Step 2/native raster/context/scratch/handle/storage limits; no device can
raise them. Fixed listener/callback state introduces no new render cache.

## Test/evidence scope

Deterministic internal fake signal sources are reachable only from compiled tests,
not exported components, input files or network. They supply normalized observations
through the same owner and JNI, never certification. JVM tests cover mapping,
overflow, unknowns, subscription lifetime, stale callbacks and 10000-callback churn.
Instrumentation covers real OS probe wiring, synthetic transitions/hysteresis,
commit/cancel/reopen, malformed JNI, ceilings, backend loss and delayed completion
publication. Shared native checkpoint races remain the late-kernel-completion proof;
Android checkpoints delayed candidate delivery before install through its owner.
Emulator evidence is wiring/lifecycle/synthetic evidence, never physical thermal,
memory or GPU certification. Actual physical measurements are recorded only when
a connected device is observed. Schema 3 and DH-APPLE-METAL-01 unchanged.
No Step 5 Apple integration, Phase 5/AI/model work or product capability UI.
