# Phase 4 Step 3 — Dynamic Memory/Thermal Downgrade

Status: COMPLETE / FULL PASS (certified implementation). No unobserved gate counts as PASS.
Authorized baseline: `d118cd26f9384c2f1735a2fbc57682c24a5990e7`.
Steps 1/2 COMPLETE / FULL PASS; seven-step Phase 4 roadmap FROZEN. Steps 4–7
NOT STARTED. Phase 0–3 remain closed. Stop after Step 3.

## Implementation and measurable acceptance

[Dynamic policy v1](../contracts/dynamic-capability-policy-v1.md) freezes separate
UNKNOWN/NORMAL/ELEVATED/SEVERE/CRITICAL thermal/memory states, strongest-signal
precedence, immediate componentwise reduction and three distinct stable recovery
observations. ELEVATED/constrained select T0/≤128 MiB/720; SEVERE T0/≤64 MiB/720;
CRITICAL T0/≤32 MiB/360. Every value is bounded by Step 2's configured baseline,
known RAM/process budget and the actual context limit. LOW/MID/HIGH remain
synthetic normal T0/96 MiB, T1/192 MiB, T2/256 MiB. No physical profiling claim.

Fixed native context state uses existing raster cancellation/publication mutex.
Execution changes increment a nonwrapping epoch and revoke PRV1. Existing platform
check/display locks remain mandatory; no second publisher/scheduler/queue. Active
gestures refresh tickets without pending-parameter/sequence/history mutation.
Interactive EXACT remains exact or rejects with resource failure; FIT alone may shrink.
Full working source and committed CPU
semantics remain. Current lower cap is enforced at allocation and registration.
Unknown facts prevent upgrade; backend availability/validation remain separate,
and dynamic validation cannot exceed static evidence. Stop/join/quiesce precedes
destruction. Epoch exhaustion returns 8 and stops future work fail-closed.

Acceptance: all normalized transitions, multi-signal precedence, budget/backend
loss, constrained/unknown states, recovery/idempotence/churn, unchanged ceilings,
late-preview race, active/commit/cancel correctness, stop/join/destroy, no retained
resource growth, cross-platform C/JNI/Swift outputs and all ten same-SHA hosted
jobs observed SUCCESS. Recovery metadata alone must not churn epoch or allocate.

## Tests and observed evidence

Shared C/C++/argument-free JNI corpus covers LOW/MID/HIGH, thermal-only/memory-only
transitions, three-observation recovery, duplicate/stale/conflicting sequences,
malformed/versioned records, budget rejection/overflow/ceilings, backend loss and
trust reduction, static T1 recovery ceiling and 30000 churn updates.
Swift declares independent expected integer transitions. Native concurrent stress
adds 80000 updates and 80000 admissions under one shared context plus stop/join.
Checkpoint-controlled gesture tests finish old high-policy preview AFTER a new
low-policy preview, require stale rejection, preserve prior display and pending
1234 milli-EV, and commit exactly one revision. Terminal races and allocation-free
churn run under fallible STL; repeated complete fixture runs compare live payload.
Display tests require 1024×512 → 360×180 while source and prior display dimensions
remain 1024×512; insufficient later budget rejects without destroying display.

Final local Windows native regression: 30/30 PASS. Full Linux native regression:
31/31 PASS; full ASan/UBSan: 33/33 PASS including negative controls. After final
EXACT/FIT and cap/occupancy changes, affected capability/gesture/preview cases
were rebuilt and passed 5/5 independently on Linux and ASan/UBSan. Logs are
`build/step3-*-acceptance*.log`; unchanged retained regression evidence is in
`build/step3-linux-all.log` and `build/step3-sanitize-tests.log`.
Android strict debug/release builds, three ABI boundaries, JVM and lint completed
successfully (151 tasks). Instrumentation runtime remains hosted-only.
Final Windows-applicable source/documentation tests: 87/87 PASS, zero skips.
Workflow actionlint and git diff --check PASS.
Initial unused test-helper wiring and WSL argument quoting were corrected as
validation harness issues; neither failed invocation counts as PASS. Rejected-cap
reporting, tiny-context clamping and native occupancy enforcement were tightened
before final local acceptance. No assertions or resource limits were weakened.
No hosted Apple/Android runtime execution is inferred from local build checks.
Initial hosted implementation SHA `747472b3025bf0f2bdf6a8d6ed9b4741e319c480`:
Foundation 38265332585 exposed a GCC enum/unsigned conditional (A portability
defect) and MSVC signed/unsigned test literals (B harness portability defect).
Explicit unsigned types preserve all behavior, assertions and strict warnings.
Local WSL lacks GCC; hosted GCC is the authoritative compiler-specific recheck.
Rebuilt local capability/gesture tests passed 4/4; Windows passed 30/30.
The next GCC run found misleading indentation at the stop/join test seam (B);
putting unconditional join on its own line preserves stop-before-join semantics.

## Exact-SHA certification

Certified implementation: `f466a54ada85dcc9068b4daddfa690b2f00e06a7`.
Foundation boundaries [38266530730](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/38266530730): 7/7 SUCCESS.
Native application shells [38266531215](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/38266531215): 3/3 SUCCESS.
Both workflows completed successfully on that SAME SHA, verified from run JSON.
Any later documentation-only closure SHA must also pass both complete workflows;
its final run IDs are delivered separately to avoid a self-referencing commit.

| Required job | Observed result |
| --- | --- |
| Foundation portable Windows | SUCCESS; native 30/30, source 81/81, zero skips |
| Foundation portable Linux | SUCCESS; native 31/31, source 81/81, zero skips |
| Foundation sanitizer | SUCCESS; 33/33, including active ASan/UBSan negative controls |
| Foundation Apple boundary | SUCCESS; native 30/30, Swift macOS 19/19, iOS simulator 19/19 |
| Foundation Android arm64-v8a | SUCCESS; checked C/C++ ABI build |
| Foundation Android armeabi-v7a | SUCCESS; checked C/C++ ABI build |
| Foundation workflow lint | SUCCESS; pinned actionlint |
| Shells source | SUCCESS; 123/123, zero skips, independent fixtures and shell syntax |
| Shells Android app | SUCCESS; strict debug/release/JVM/lint builds; emulator instrumentation 19/19 |
| Shells iOS app | SUCCESS; package/C ABI gates, debug/release app builds, app tests 6/6, UI tests 2/2 |

All affected and retained Phase 0–3 / Step 1/2 gates executed. Hosted evidence logs
are retained locally under `build/step3-hosted-*.log`; build artifacts are not committed.
No physical Apple certification is claimed; AVAILABLE != VALIDATED/CERTIFIED.

## Transition and correctness outcomes

| Input | Result (also bounded by initial static policy and occupancy) |
| --- | --- |
| NORMAL stable | LOW T0/96 MiB/720; MID T1/192 MiB/1280; HIGH T2/256 MiB/2048 |
| ELEVATED thermal or memory; constrained | Immediate T0, at most 128 MiB/720, CPU, concurrency 1 |
| SEVERE thermal or memory | Immediate T0, at most 64 MiB/720, CPU, concurrency 1 |
| CRITICAL thermal or memory | Immediate T0, at most 32 MiB/360, CPU, concurrency 1 |
| Competing signals | Maximum severity, never averaged; critical > severe > constrained > elevated > baseline |
| Backend loss | Immediate CPU fallback; revoke old policy/PRV1 eligibility; document unchanged |
| Recovery | Three distinct increasing known-NORMAL observations with identical candidate; no static-cap/tier exceedance |
| Unknown or duplicate | No upgrade; identical retry earns no recovery credit or epoch churn |

Material strategy changes alone increment the nonwrapping epoch. Exhaustion fails
closed. Reasons remain bounded enum values: baseline, thermal, memory, constrained,
backend, budget, unknown facts. EXACT rejects with resource failure when its exact
output cannot fit; FIT may shrink disposable display dimensions only. Full-size
reference output stays exact where admitted. Active gesture retains accepted
parameters; changed commit creates exactly one revision; neutral/cancel zero.
Old high-tier preview finishing last is rejected by policy/PRV1 fences. Failed
newest preview preserves prior display. Dynamic changes create no revision.

Observed race/stress coverage includes 80000 updates plus 80000 admissions on one
context, competing pressure signals, stop/join/quiesce/destruction, terminal races,
30000 corpus churn updates and 10000 allocation-free gesture churn updates.
Repeated full fixtures retain equal live payload after warmup; no unbounded policy
snapshots or resource accumulation. Original Step 2 parallel determinism remains.
Identical normalized C/C++/JNI corpus and independent Swift expected transitions
pass across Windows/Linux/Apple/Android. Hard-limit, tiny-context, occupancy,
malformed/version/sequence, epoch-boundary and overflow tests pass without raising
any ceiling or weakening sanitizers/assertions. No architecture conflict/new ADR.
Failure classification: A compiler portability corrected minimally; B test harness
portability/wiring corrected without semantic changes; C no final runner retry
required. Sandbox/process invocation failures were not counted as test passes.

## Scope and classification

Category A: shared dynamic policy, existing raster/gesture integration, bounded
fault/race/churn tests, C/JNI/Swift boundaries and validation/documentation.
No architecture conflict/new ADR. Category B: frozen Steps 4–7 only after explicit
authorization; real Android/Apple observation collection remains Steps 4/5.
Category C: optional pressure dashboards, latency telemetry and product polish.
No schema/storage/history-policy persistence, source-fidelity/operation change,
production dependency, product UI, network/telemetry, GPU kernel expansion,
AI/model/inference/Phase 5 work. Schema 3 unchanged. DH-APPLE-METAL-01 unchanged.
Step 4 NOT STARTED. Stop after Step 3.
