# Phase 4 Step 7 — Phase 4 closure

Phase 4 — CLOSED / COMPLETE / FULL PASS. Steps 1–7 — COMPLETE / FULL PASS.
Authorized baseline: `3d5db65b82e16bb18cca7b906b5108b70fc95956`, matching HEAD and
origin/main with clean working tree at inspection. Phase 0–3 remain CLOSED.
Phase 5 NOT STARTED. Seven frozen steps only; no Step 8 or Phase 4.5.

## Evidence and final-SHA promotion

This closure is a documentation/status audit and an updated finite-roadmap source
guard, not feature development. No production implementation, schema, threshold,
resource ceiling, tool, preset, GPU kernel, UI or dependency changes.

Observed baseline certification: Foundation boundaries
[38419049842](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/38419049842)
7/7 SUCCESS and Native application shells
[38419049786](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/38419049786)
3/3 SUCCESS on the SAME baseline SHA. All retained Step 1–6 exit criteria have
observed evidence below. Baseline results are historical evidence, never a
substitute for final closure certification.

Closure promotion is effective ONLY when BOTH complete workflows on the commit
containing this report succeed: seven Foundation jobs and three application-shell
jobs, all completed SUCCESS, no skipped required gate. The final delivery records
that exact closure SHA, workflow IDs and counts after observation. This avoids
self-referential SHA edits or a later uncertified status commit. Until those jobs
pass, closure is a candidate and Phase 4 remains open for delivery purposes.

| Retained gate | Observed baseline result; required again on closure SHA |
| --- | --- |
| Source/documentation | 126/126, zero skips; Windows-applicable local set 90/90 |
| Windows / Linux / Apple native | 31/31 / 32/32 / 31/31 |
| ASan/UBSan | 34/34 including active negative controls |
| Android ABI | arm64-v8a and armeabi-v7a PASS |
| Android debug/release/lint | PASS; JVM 7/7 each variant |
| Android instrumentation | 27/27, zero failures/errors/skips |
| Swift macOS / iOS Simulator | 25/25 each |
| iOS debug/release | unsigned builds PASS |
| iOS app / UI | 7/7 / 2/2 |
| Workflow lint / shell syntax / diff check | PASS |

The workflows execute persistence/reopen, shared cross-tier certification,
capability/admission/dynamic-policy and retained Phase 0–3 regressions. Evidence
logs/artifacts stay ignored under repository build directories. Never label an
unobserved closure job PASS; inspect exact failing logs and preserve assertions.

## Consolidated Step 1–7 certification

| Step | Status | Authority and certification |
| --- | --- | --- |
| 1 Capability Contract & Probe Foundation | COMPLETE / FULL PASS | [Contract](../contracts/capability-profile-v1.md): checked family-1 fixed records, 76-byte canonical diagnostics, explicit unknowns, separate static/dynamic facts, allocation-free normalization; synthetic fixtures only. |
| 2 Budget Admission & Execution Tiers | COMPLETE / FULL PASS | [Contract](../contracts/budget-admission-v1.md): pure deterministic request-dependent shared admission, checked arithmetic, occupancy and hard ceilings; no reservation, global mutable policy or persistence. |
| 3 Dynamic Memory/Thermal Downgrade | COMPLETE / FULL PASS | [Contract](../contracts/dynamic-capability-policy-v1.md): fixed context-owned strategy, strongest-signal immediate downgrade, deterministic recovery and PRV1 fencing. |
| 4 Android Capability Integration | COMPLETE / FULL PASS | [Contract](../contracts/android-capability-integration-v1.md): one Application-owned observation service; checked JNI, bounded callbacks/lifetime and lifecycle integration. |
| 5 Apple Capability Integration | COMPLETE / FULL PASS | [Contract](../contracts/apple-capability-integration-v1.md): one application-composed observation service; checked Swift/C, bounded observers/coalescing and lifecycle integration; Metal uncertified. |
| 6 Cross-tier Replay & Certification | COMPLETE / FULL PASS | [Contract](../contracts/cross-tier-replay-certification-v1.md), [matrix/report](phase-4-step-6.md): common bounded canonical corpus runs native/Android/Apple, preserving source/history/reference. |
| 7 Phase 4 Closure | COMPLETE / FULL PASS | This audit plus all ten complete same-final-SHA hosted jobs required for effective closure. |

Historical reports retain their original authorization/stop points. This closure
supersedes historical NOT STARTED text without rewriting accepted contracts/ADRs.

## Architecture and frozen limits

Platform adapters → normalized observations → shared CapabilityProfile → shared
BudgetAdmission → shared DynamicPolicy → existing PRV1/generation publication
authority → existing document/history/persistence authority. Android/Swift collect
facts; neither selects tiers, caps, preview bounds, recovery or history semantics.
No mutable global policy authority, duplicate publication owner or device tier table.

| Tier | Required normal, unconstrained measured minima | Policy cap | Preview edge | Concurrency |
| --- | --- | --- | --- | --- |
| T0 | No required CPU/RAM/process observation | 128 MiB | 720 | 1 |
| T1 | 4 CPUs, 4 GiB RAM, 256 MiB process observation | 192 MiB | 1280 | 1 |
| T2 | 8 CPUs, 8 GiB RAM, 512 MiB process observation | 256 MiB | 2048 | 1 |

Effective cap further minimizes known RAM/16, saturating process budget minus
32 MiB, and existing context limit. Synthetic LOW/MID/HIGH facts are CPU 2/4/8,
RAM 2/4/8 GiB, process 128/256/1024 MiB: normal outcomes T0/96 MiB, T1/192 MiB,
T2/256 MiB. Unknown cannot promote or escalate backend trust.

Frozen decode maxima remain 16384 edges, 33554432 pixels, 65536 row bytes,
128 MiB raster, 64 MiB encoded and 32 MiB generic scratch. Working maxima remain
16384 edges, 8388608 pixels, 262144 row bytes, 128 MiB raster. Context 256 MiB,
64 combined handles; GPU diagnostic tile 16384 pixels. Stricter per-operation
scratch, operation/history/preset/storage limits remain unchanged and exercised
by retained boundary corpora. HIGH cannot raise any maximum; policy can only lower.
Checked add/multiply, signed conversions, malformed records/version/trust and
overflow tests reject conservatively without document mutation.

## Dynamic and platform observations

Dynamic family-1 thermal AND memory enums: UNKNOWN=0, NORMAL=1, ELEVATED=2,
SEVERE=3, CRITICAL=4. Capability v1's historical four-value snapshot/76-byte wire
remains unchanged; critical uses the additive dynamic record, not a wire rewrite.
Precedence: CRITICAL > SEVERE > constrained > ELEVATED > baseline; strongest
thermal/memory signal wins, memory wins equal-severity diagnostic ties.

| Observation | Immediate future strategy |
| --- | --- |
| ELEVATED or constrained | T0/CPU, ≤128 MiB, ≤720 edge |
| SEVERE | T0/CPU, ≤64 MiB, ≤720 edge |
| CRITICAL | T0/CPU, ≤32 MiB, ≤360 edge |
| Backend lost/unvalidated | CPU, no other component increase |
| Budget shrinks | Lower cap immediately; Step 2 may lower tier/edge |
| Unknown | At most T0/CPU; no cap/edge increase or recovery evidence |

All rows further minimize configured static/current authority. Recovery requires
THREE distinct increasing sequences deriving the same candidate, known NORMAL
thermal/memory, unconstrained, positive known budget and known availability.
Worsening/unknown resets recovery; same sequence identical retry does not count.
Epoch starts one; only material strategy changes increment it. UINT64_MAX next
material change returns resource failure and stops PRV1 fail-closed, never wraps.
Old render may finish but cannot publish; owner lock covers eligibility/install.
Cancellation saves work; fences establish correctness. No wall-clock/sleep logic.

Android static facts: bounded CPU/compiled ABI/API, MemoryInfo total RAM, minimum
ordinary memoryClass/heap/known RAM process proxy, bounded native Vulkan probe.
Thermal API 29+: NONE→NORMAL, LIGHT/MODERATE→ELEVATED, SEVERE→SEVERE,
CRITICAL/EMERGENCY/SHUTDOWN→CRITICAL; unsupported→UNKNOWN.
Trim RUNNING_MODERATE→ELEVATED, RUNNING_LOW→SEVERE, RUNNING_CRITICAL and
BACKGROUND/MODERATE/COMPLETE/onLowMemory→CRITICAL; UI_HIDDEN preserves pressure
and constrains. Fresh valid MemoryInfo supplies normal/severe; missing callbacks
are not normal evidence. One thermal/ComponentCallbacks registration, ≤8
subscribers, last unsubscribe invalidates tokens/unregisters. Activity recreation
retains model subscription; fresh context/process derives fresh policy. Background
uses existing interruption, never auto-commit. Exact-length/versioned signed JNI
arrays reject negative/malformed/overflow inputs; production cannot set GPU trust.

Apple static facts: bounded active CPU/compiled architecture, physical RAM/OS,
Metal availability, iOS advisory remaining os_proc_available_memory bounded by RAM;
macOS process observation unknown. Nominal/fair/serious/critical thermal maps to
NORMAL/ELEVATED/SEVERE/CRITICAL; unsupported→UNKNOWN. Dispatch memory
normal/warning/critical maps NORMAL/SEVERE/CRITICAL, ambiguous/pre-event→UNKNOWN;
iOS warning→SEVERE, never invented recovery. Inactive/background constrains.
One thermal observer/pressure source plus lifecycle observers, ≤8 subscribers;
weak token-checked callbacks, final cleanup, one bounded pending coalesced signal.
Coalescing preserves strongest pressure/minimum budget and UNKNOWN backend.
Reconstruction/restart reprobes conservatively; strict Swift concurrency retained.
Swift/C uses checked bounded records; no Swift object retained by native core.

AVAILABLE != VALIDATED/CERTIFIED. Production Android probes remain NOT_VALIDATED;
Apple Metal remains NOT_VALIDATED. Test-only normalized trust fixtures do not
certify hardware or broaden historical Z Fold7 scope. CPU T0 editing remains
available wherever safely admitted; insufficient memory can reject even T0.

## Replay, history, storage and races

HIGH→MID→LOW preserves exact eight-revision graph, current/redo, IDs, versions,
parameters and ordered crop/rotate/exposure/contrast/saturation/blur/sharpen stacks.
LOW→HIGH preserves four revisions and complete CPU T0 import→preview→edit→commit
→cancel→undo→redo→close→reopen. Geometry/tone/detail reverse-order pairs retain
order; undo branches retain graph and clear old redo. Android/Apple produce/consume
the same canonical goldens. Full CPU reference matches across normalized tiers.

Source SHA: `410e778774cb0ef8e168dfc8aa10c6ad4e0683eede5cd2f4be2aa7590508f3c5`.
Full canonical SHA: `0711b34c325fc5ad10029e8cbda326e303758281d15587160928690fbf1fee05`.
Reference SHA: `779ce293329f4dcb461b800e06af18bc85901034d03416618fee2f0ada5e233f`.
Redo checkpoint and bounded 21-row certification matrix are in Step 6 evidence.

EXACT = exact requested output or deterministic resource rejection. FIT = disposable
display allowed to shrink (1024×512 HIGH, 720×360 LOW, 360×180 critical case).
Full/source/committed semantics remain authoritative; preview never becomes source.
Changed commit one revision; neutral/cancel zero; pressure/tier/recovery zero.

Durable: source/assets/revisions/operation stack/current/redo/schema. Runtime-only:
profile/tier/admission/thermal/pressure/policy epoch/preview bound/backend state.
Canonical serializer and schemas contain no capability state. Schema 1/2 historical
fingerprints/compatibility remain; explicit 1→2 and 2→3, no auto migration on ordinary
open, Schema 3 current authority unchanged, no Schema 4. Session/storage envelope
is durable ownership, distinct from nonpersisted policy epoch/document identity.

Step 6 rejected-reopen fix remains: prepare decode/working/document/PRV1 before
durable session acquisition, then expected-epoch CAS rejects intervening changes.
CRITICAL 32 MiB can reject status 8 because exact decode scratch plus control cannot
fit. Tests preserve canonical/source/history/current/redo AND durable epoch/session/
generation, with no partial mutation; recovered fresh policy reopens successfully.

Retained capability parallel tests, allocation-free bounded policy churn, Android
10000-callback churn, Apple observer/strict-concurrency races, pressure vs preview/
commit/cancel, backend loss, stale listener/context calls and stop→join→destroy
execute. Cross-tier actual checkpoint-paused high render finishes AFTER eligible
low render and returns revoked status 13 preserving output. Sixteen terminal races,
12 fresh-policy tier reopen cycles and two-context isolation pass; no UAF/deadlock/
double commit/stale publication observed. Temporary corpus projects are reclaimed;
fixed policy state and bounded diagnostics do not accumulate snapshots/tasks.
Retained path/hash/storage/no-overwrite/security/fault/crash tests and active sanitizer
controls remain enabled. Debug/test seams absent from production release controls.

## Evidence classification and remaining work

SYNTHETIC POLICY EVIDENCE: LOW/MID/HIGH and injected pressure/backend fixtures.
EMULATOR/SIMULATOR INTEGRATION EVIDENCE: Android runtime, iOS Simulator wiring,
lifecycle/JNI/Swift/C/editor/canonical parity. HOSTED RUNTIME EVIDENCE: Windows,
Linux, macOS compiler/native/Swift/build execution. PHYSICAL DEVICE EVIDENCE:
no fresh Phase 4 Android measurement; no physical Apple hardware/evidence.
Historical physical Z Fold7 Phase 2 certification remains narrow and separate.

No unresolved Category A. Step 6's durable rejection defect remains fixed/certified;
Step 7 introduces no product fix. Source guard updates reflect authorized closure
without dropping frozen-row/status assertions. No unresolved architecture conflict,
new ADR or expanded feature scope.

B: DH-APPLE-METAL-01 = DEFERRED, unchanged, mandatory before Phase 11 hardening
acceptance, Phase 12 Beta readiness completion, production release or any physical
Apple GPU certification claim. Later physical coverage stays at its approved gates.
C: optional capability dashboards/scores/broad diagnostics/performance polish remain
backlog only. Neither category blocks this authorized Phase 4 closure.

No physical Apple certification claim. No Step 8, Phase 4.5, Phase 5/AI/model/NPU,
Editing Agent, telemetry/network/cloud or new product UI. STOP after Phase 4 closure.
