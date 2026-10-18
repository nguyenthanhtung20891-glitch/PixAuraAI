# Phase 6 Step 4 — structured intent and deterministic reference planner

Baseline `fa6462e2bc1c0a88b30a46e69a9d982f8d843fe4`, clean at audit. Step 3
COMPLETE / FULL PASS: Foundation 39131908247 SUCCESS 7/7; Native application shells
39131908300 SUCCESS 3/3. Step 4 AUTHORIZED / IN PROGRESS through exact-SHA gates.
Step 5 NOT STARTED. Phase 7 NOT STARTED. No Step 9+.

## Architecture decision and audit

The required stop gate correctly found no structured tool/amount semantics in the
frozen free-form Intent v1. Product Owner approved Option B:
"Phase 6 Step 4 may introduce a separately versioned structured-intent contract
without modifying free-form intent v1."
The explicit decision is recorded in DECISIONS.md. Audit covered product,
architecture, quality/roadmap/decisions, ADR 0004/0009, Steps 1–3 contracts/code/
tests, manual descriptors/parameter validators, shared JSON, proposal ownership,
snapshot binding, combined handle ceiling, cancellation slot/epoch and native/
platform boundaries. No further architecture conflict or ADR is required.

[Structured Intent v1](../contracts/editing-structured-intent-v1.md) freezes a
separate exact four-field machine-semantic envelope: binding, goal_id, parameters,
structured_intent_version. Eleven explicit goals map one-to-one to current tool
version tuples 1/1. No structured release recipe composition, direction inference,
text interpretation, NLP or model. Free-form Intent v1, Plan v1 and Registry v1
remain unchanged, with retained compatibility digests/tests.

[Reference planner v1](../contracts/editing-reference-planner-v1.md), identity
reference.deterministic.v1, validates canonical structured input against native
snapshot authority, builds one ordinary action through the existing Writer and
Step 2 validator, then invokes the SAME Step 3 tool binding guard. Mapping stores
only goal/tool identities; no duplicated parameter ranges/defaults or guessed
values. No plan mutation, implicit migration, clamp or tool substitution.
Binding is preserved exactly. Input bound 2048 bytes; plan bound 8192 bytes and
max eight actions retained, actual policy always one action. No new handles.

The existing one-parse/context slot, nonwrapping schema cancellation epoch and
final liveness/publication fence are shared. Work is private, bounded and outside
the registry lock. Before-planning, in-flight and pre-publication cancellation,
busy rejection, recovery, independent contexts and release fencing are tested.
No candidate, revision, pixel work, GPU/inference or execution allocation occurs.
Valid generated actions staged only by a test fixture remain NOT_APPROVED=15;
no production plan-to-stage/approval/execution bridge is introduced.

## Evidence requirements and scope

Repo-owned golden fixture: 39 positive cases covering every goal's neutral value,
every parameter minimum/maximum and valid crop boundaries; independent canonical
structured/plan bytes and SHA-256 digests. Forty-two negative rows cover outside
bounds, wrong/missing/extra parameters, unknown version/goal, ambiguity, overflow,
float, duplicates, crop extent, stale/wrong context and free-form non-mapping.
Additional native malformed ABI/UTF-8/strict canonical/size/overlap and lifecycle
cases fail closed. Host corpus repeats each positive 50 times; shared Android
boundary repeats three times and binds through public Registry v1. Independent
contexts plan concurrently without mutable global planner state. Complete returned
arrays compare against goldens; failure paths retain full byte/length sentinels.

The exact free-form phrase "make this photo brighter" remains valid under the old
API and is unsupported by the new planner, never silently mapped to brightness.
No test-only recipe/tool is installed. Existing multi/eight-action and nine-action
rejection corpora remain retained in Steps 2–3; no multi-tool release semantics
are invented to force new planner cases. More than one generated action and output
size overflow are unreachable by fixed construction and bounds; explicit final
checks remain. Unknown registry/tool tuples and denied exposure fail through
shared authority, not fallback or semantic substitution.

PENDING_EXACT_SHA until BOTH full final-SHA workflows succeed, including all
Windows/Linux/Apple/Android and retained Phase 0–5/Steps 1–3/manual gates,
active ASan/UBSan, source/canonical/security tests, production-object isolation,
workflow lint and git diff --check. Observed final SHA/run results and counts are
recorded in the bounded engineering receipt and final delivery after execution.
No unexecuted check is PASS. Apple hosted/simulator software evidence is not
physical certification.

A: current structured parser, deterministic reference mapping, security and retained
certification. B: Step 5 approval/execution, Step 6 lifecycle, Step 7 integration,
Step 8 closure and future production planner architecture. C: optional AI/UX/
research/tuning. Fix A only. Approved original gap resolved; no further blocker.
Schema 3, manual editing and Phase 5 semantics unchanged. Production model UNRESOLVED.
No external runtime, weights, NLP/model, cloud/network dependency or AI UX.
DH-APPLE-METAL-01 unchanged: DEFERRED, NOT WAIVED, NOT SATISFIED.
Step 5 NOT STARTED. Phase 7 NOT STARTED. STOP AFTER STEP 4.

## Local pre-push validation

Existing Windows compiler fallback: six affected planner/agent/C ABI/production
isolation tests PASS; finalized planner corpus 921836 assertions, zero failures.
Windows-applicable source suite 110/110 PASS. Complete Linux source suite 152/152
PASS, zero skips; full retained Linux native suite 44/44 PASS. Retained Steps 1–3,
manual and Phase 0–5 gates in those suites PASS. Final planner corpus under active
ASan/UBSan: 921836 assertions, zero failures. Actual instrumentation confirmed by
both negative probes (ASan exit 86; UBSan exit 87); no suppression. Full retained
local sanitizer suite is still running and remains pending until observed.
Strict Clang diagnostics with explicit misleading-indentation warnings PASS;
GCC/MSVC and Apple/Android full certification remain final-SHA CI evidence.
Normal production-object scans, workflow lint and git diff --check PASS.
No validation was removed or loosened. Observed full final-SHA workflow results,
platform counts and final Step 4 identity are recorded after execution in the
engineering receipt and delivery; this commit is a certification candidate.
