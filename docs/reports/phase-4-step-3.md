# Phase 4 Step 3 — Dynamic Memory/Thermal Downgrade

Status: IMPLEMENTED / VALIDATION IN PROGRESS. No unobserved gate counts as PASS.
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
Final exact-SHA Foundation and Native application shells remain pending.

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
