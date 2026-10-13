# Apple capability integration v1 — Phase 4 Step 5

Status: COMPLETE / FULL PASS. Authorized baseline `40a0c9bef05df328a2b3e89497316c368b81aa24`.
Steps 1–4 COMPLETE / FULL PASS. Frozen seven-step roadmap unchanged;
Steps 6–7 NOT STARTED. [Evidence](../reports/phase-4-step-5.md).

## Authority and ownership

The application composition owns one AppleCapabilityOwner, passed through shell
and editor construction. macOS consumers use the same caller-owned service.
Static profile is copied once. Swift only collects/maps observations. Shared
Capability v1, BudgetAdmission and dynamic policy v1 exclusively derive tier,
cap, edge, concurrency, backend, reason, epoch and recovery. No device-name table,
new dependency, network, telemetry, identifier, consumer UI or policy engine.

ManualEditor policy-aware open configures the existing native editor facade
BEFORE decode/working allocations. Apply/query use its existing lifecycle/display
gate; render runs outside it. Native current check and actual CGImage replacement
remain under that SAME gate. No second publication owner or Swift generation.
Existing stop → join → destroy remains mandatory.

One thermal observer, one memory-pressure dispatch source and platform lifecycle
notifications serve at most eight subscribers. First subscription registers;
final unsubscribe/close invalidates token identity before unregister/cancel.
Weak callbacks cannot retain a view, scene, owner or native context. Reconstruction
uses fresh tokens; stale callbacks cannot deliver to new subscriptions. Callback
delivery is synchronous/bounded; subscribers only hand off work. The existing
command lane holds one pending observation and one drain notification. Coalescing
preserves strongest pressure, constrained OR, smallest budget and unavailable/
unknown backend. Unknown cannot become recovery evidence by coalescing.

## Static and dynamic facts

Static: native compiled architecture/platform, bounded activeProcessorCount
(1…1024, otherwise unknown zero), ProcessInfo.physicalMemory, OS major version,
MTLCreateSystemDefaultDevice availability. Validation is ALWAYS NOT_VALIDATED.
Simulator provenance is VIRTUAL; other API measurements use UNOBSERVED provenance
for certification. A hosted Mac is not target iPhone/iPad physical evidence.

iOS os_proc_available_memory reports advisory REMAINING allocatable bytes, not
a fixed process limit. Read afresh before work/when signals arrive; conservatively
bound by observed RAM. Shared policy still subtracts its frozen reserve. Zero is
unknown; macOS process budget is unknown zero because no equivalent fixed limit
is asserted. Neither observation authorizes allocation or raises a hard ceiling.
The static NORMAL/unconstrained profile is only a recovery envelope; actual
initial observations stay separate and are never fabricated normal readings.

Dynamic: current ProcessInfo thermalState, observed pressure, inactive/background
state, fresh remaining-budget observation and fresh Metal availability.

## Frozen adapter mappings

| Source | Shared observation |
| --- | --- |
| thermal nominal | NORMAL=1 |
| thermal fair | ELEVATED=2 |
| thermal serious | SEVERE=3 |
| thermal critical | CRITICAL=4 |
| unsupported/new ambiguous thermal | UNKNOWN=0 |
| dispatch memory normal | NORMAL=1 |
| dispatch memory warning | SEVERE=3, conservative warning mapping |
| dispatch memory critical | CRITICAL=4 |
| empty/ambiguous memory event; before first event | UNKNOWN=0 |
| iOS application memory warning | SEVERE=3; notification has no finer severity |
| scene/application active | constrained=false |
| inactive/background | constrained=true |
| Metal device absent/present | UNAVAILABLE/AVAILABLE; validation remains zero |

Both platforms use dispatch pressure signals; iOS additionally observes UIKit
warnings. A warning notification does not invent a clear/recovery event. Only a
legitimate normal source event clears pressure. Thermal is read before observer
registration as required by Apple. No sleeps, polling timer, fabricated stable
readings or model heuristics. Lifecycle reconstruction begins conservatively;
current scene observations refresh constrained state. Existing interruption
cancels without auto-commit. Closing editor removes its subscription.

## Shared invariants and trust

Strongest signal wins: CRITICAL > SEVERE > constrained > ELEVATED > baseline.
Immediate reductions; THREE distinct stable normal observations for recovery.
Material shared strategy changes alone increment nonwrapping epoch. Sequence
exhaustion fails closed; identical strategy does not churn epoch. Core rejects
malformed/versioned records and validation escalation. Native structs have
explicit sizes/versions and bounded checked conversions; no Swift object retained.

EXACT remains exact-or-resource-failure. FIT may shrink via shared policy only
(critical bound 360). Full reference/source semantics, operation parameters,
history and saved document remain authoritative. Old higher-tier candidates
cannot install after epoch revokes their PRV1 ticket. Failed newest requests
retain prior valid display. Active gesture stays active with accepted values;
changed commit exactly one revision, neutral/cancel zero. Pressure adds none.

Dynamic budgets only lower Step 2 maxima (T0 128 MiB/720, T1 192 MiB/1280,
T2 256 MiB/2048). Severe ≤64 MiB/720, critical ≤32 MiB/360, concurrency one,
CPU fallback where admitted. Recovery never exceeds configured baseline.
LOW/MID/HIGH fixtures are synthetic only, normal caps 96/192/256 MiB.
AVAILABLE != VALIDATED/CERTIFIED. Metal is optional; no trust grant or waiver.
DH-APPLE-METAL-01 remains DEFERRED, unchanged. No physical Apple hardware is
available; simulator and hosted macOS evidence cannot satisfy that gate.

Schema 3 unchanged. No capability, tier, pressure, epoch or profile persistence.
Fresh process/reopen reprobes current facts and starts a fresh native policy.
Initial CRITICAL reopen may reject status 8: frozen 32 MiB scratch plus metadata/
control exceeds the critical 32 MiB cap. This is correct admission behavior.
After legitimate normal observations/fresh owner, admitted reopen preserves
committed history/source and starts epoch one; no budget or semantic weakening.
No Phase 5/AI/model work. Step 6 NOT STARTED.

## Validation

Require observed source/docs, workflow lint, Windows/Linux/Apple native,
ASan/UBSan, Swift macOS/iOS Simulator, iOS debug/release/app/UI, retained Android
ABI/JVM/instrumentation, retained Step 1–4 and Phase 0–3 gates, diff check,
and both complete hosted workflows on the SAME final accepted SHA. Tests cover
real probe wiring without certification, mappings, unavailable/unknown facts,
synthetic profiles, pressure/recovery, backend loss, constrained state, malformed
records/trust escalation, coalescing, bounded subscriptions, churn, concurrent
callbacks/close, commit/cancel races, stale installation and fresh reopen.

API references: [thermal notification](https://developer.apple.com/documentation/foundation/processinfo/thermalstatedidchangenotification),
[available memory](https://developer.apple.com/documentation/os/os_proc_available_memory),
[memory pressure](https://developer.apple.com/documentation/dispatch/dispatchsourcememorypressure).
