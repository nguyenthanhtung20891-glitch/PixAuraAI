# Dynamic capability policy v1 — Phase 4 Step 3

Authority: Product Owner authorizes Step 3 at
`d118cd26f9384c2f1735a2fbc57682c24a5990e7`. Steps 1/2 are COMPLETE / FULL PASS.
The seven-step roadmap remains FROZEN; Steps 4–7 NOT STARTED. No Step 8+.
Status: COMPLETE / FULL PASS; exact-SHA executed evidence belongs in the
[Step 3 report](../reports/phase-4-step-3.md). DH-APPLE-METAL-01 unchanged.

## Authority and bounded state

`dynamic_policy.h` is an additive checked C ABI family 1. Trusted internal future
adapters configure one immutable CapabilityProfile copy per live raster context.
The shared core owns derivation; platforms provide normalized observations only.
No user UI/diagnostic loader accepts profile or observation injection. Synthetic
LOW/MID/HIGH are test fixtures, never physical observations or certification.
No real Android/Apple pressure probes land here; those remain Steps 4/5.

The existing raster registry owns one fixed State: immutable profile, last
observation, current strategy and one recovery candidate. No allocation in
configure/update/query/admit/current, no global policy instance, request map,
retained snapshot list, timer, cache, thread, queue or second scheduler. Context
creation retains its existing allocation. Policy memory is fixed native control
overhead, not raster payload or a peak RSS claim. No transient cache is introduced
to evict; failed/superseded private render resources use existing reclamation.
Explicitly owned source/committed/display handles are never evicted by pressure.

## Normalization and precedence

Observation family 1 freezes thermal AND memory pressure as unsigned enums:
UNKNOWN=0, NORMAL=1, ELEVATED=2, SEVERE=3, CRITICAL=4. Wrong enum, reserved field,
flag, version or record size rejects. Capability v1's accepted four-value enums,
76-byte diagnostics and all Step 2 constants remain unchanged. For Step 2
evaluation only, CRITICAL maps to its SEVERE value, with stricter dynamic caps
below. Other normalized values map exactly. No free-form semantic strings.

Effective severity is max(thermal,memory), never an average. CRITICAL dominates
SEVERE, then constrained, then ELEVATED, then baseline. Memory wins an equal
thermal/memory severity tie for diagnostic reason. Unknown in either input
prevents upgrade/backend enablement; it selects at most T0 and cannot increase an
already lower cap. Constrained forces T0/CPU. Static hardware facts are never
mutated; a local Step 2 evaluation copy represents current observations.

Runtime backend validation may decrease accepted static trust, never raise it.
An observation claiming validation above the configured static evidence rejects.
Availability UNKNOWN/UNAVAILABLE cannot enable a backend. AVAILABLE remains
distinct from RUNTIME_VALIDATED and PHYSICAL_CERTIFIED. Virtual/runtime-validated
Metal can never establish physical certification or change DH-APPLE-METAL-01.

## Frozen transition matrix

| Current observation change | Immediate future execution strategy |
| --- | --- |
| NORMAL → ELEVATED, either signal | T0; cap ≤128 MiB; preview ≤720; CPU |
| NORMAL → SEVERE, either signal | T0; cap ≤64 MiB; preview ≤720; CPU |
| NORMAL → CRITICAL, either signal | T0; cap ≤32 MiB; preview ≤360; CPU |
| constrained false → true | T0; cap ≤128 MiB; preview ≤720; CPU |
| thermal NORMAL + memory SEVERE (or inverse) | SEVERE rule; stronger signal wins |
| simultaneous CRITICAL/constrained/backend loss | CRITICAL cap/edge; CPU |
| backend available → unavailable/unvalidated | Immediate CPU; keep other limits at most current values |
| known process budget shrinks | Immediately lower effective cap; lower tier/edge when Step 2 requires |
| any observation becomes UNKNOWN | At most T0/CPU, never an increased cap/edge |
| ELEVATED/SEVERE/CRITICAL → NORMAL | Hold lower strategy until recovery criterion below |
| backend restored / process budget grows | Same recovery criterion; never exceed configured Step 2 ceiling |

All caps are further minimized by Step 2's configured baseline tier/cap/edge,
current known RAM/16, process budget minus 32 MiB saturating at zero and the
actual existing context limit. LOW/MID/HIGH normal outcomes remain T0/96 MiB,
T1/192 MiB, T2/256 MiB. Concurrent native work allowance remains one. GPU
preference applies only to the existing optional GPU_DIAGNOSTIC class; manual
and committed/reference rendering remain CPU semantic authority.

Each worsening component lowers immediately, preserving minima in other
components. A simultaneous backend improvement cannot raise another component
during downgrade. Reducing budgets may reject even T0 work; supported manual
editing remains available where safely admitted, not an allocation guarantee.

## Recovery and sequence

Recovery requires THREE distinct monotonically increasing observation sequences
that derive the same candidate execution fields, with NORMAL thermal AND memory,
unconstrained state, known positive process budget and known backend availability.
No wall-clock or sleeps. A new candidate resets count to one. Worsening or unknown
observations reset recovery. Same sequence + identical observation is an idempotent
retry and contributes no stability evidence; same sequence + different payload,
or older sequence, rejects STALE_BASE(9). Sequence zero is malformed. Adapters
must not wrap sequence numbers; a fresh context requires a fresh identity.
Repeated identical strategies at new sequences do not change execution epoch.

Recovery restores at most the configured Step 2 profile's tier/cap/edge, even if
later process observations become larger. Native raster/context/operation/scratch/
storage/handle ceilings cannot be raised. Unknown static CPU/RAM/process values
cannot grant a higher baseline. Queries remain preflight, never reservations.

## Epoch and PRV1 ownership

Epoch starts at one and increments only when execution tier, cap, edge,
concurrency, backend or forced-CPU flag changes. Recovery count/pending and
diagnostic reason changes alone do not churn epoch or trigger work. UINT64_MAX
may be reached; a subsequent material transition returns RESOURCE_LIMIT(8),
preserves caller output, stops PRV1 and fails closed for new allocations/work.
No wrapping, identity reuse or silent stale upgrade.

Policy updates use the existing raster cancellation/publication mutex, never
the long-running raster mutex. A material transition revokes the current PRV1
ticket through its existing atomic revocation flag. Old work may finish; it
cannot register/install after revocation. Recovery also revokes old eligibility.
Future begin gets an ordinary new PRV1 generation. `policy_current` checks epoch,
ticket and optional native result under the SAME existing publication fence.
It does not install anything or create another publication authority.

The platform owner lock MUST cover policy changes, begin/cancel/live changes,
native eligibility check and display replacement. Rendering/copying stays outside
that lock. Raw policy updates must not bypass this established owner. Checked
epoch alone is insufficient for installation; PRV1 and the display lock remain
mandatory. Context stop → join/quiesce → destroy and document-before-raster
destruction remain unchanged. Calls racing actual freed context storage are
unauthorized; tests race update against stop, join, destroy, then stale calls.

`policy_admit` reads existing native retained capacities and combined handles
under the same publication mutex and floors caller totals at that actual
occupancy. This is a bounded registry walk, not a second counter. Callers must
still supply truthful TOTAL native + external retained occupancy, temporary/
in-flight capacities and their owner work count; results are not reservations.
The ordinary raster mutex and final registration remain execution authority.

## Gesture and raster integration

For configured contexts, `manual_preview_ticket` refreshes the active gesture's
ticket on a changed epoch without replacing pending parameters, advancing its
logical sequence, committing/cancelling the gesture or adding history. Accepted
parameter state remains ACTIVE. Neutral/cancel remains zero revisions; changed
commit remains exactly one detached proposal under the ordinary approval owner.

Interactive preview validates the original request before clamping disposable
display dimensions to current strategy bounds. Conversion additionally halves
display bounds (floor, minimum one) for at most twelve attempts against actual
remaining payload room, following Step 2's static display-only downgrade rule.
Full working source stays intact; if those retained buffers already exceed room
or even a one-pixel display cannot fit, reject instead of reducing source fidelity.
Only explicit FIT requests adapt; EXACT requests retain exact pixels/dimensions
and reject when the dynamic edge or available room cannot admit that exact output.
Noninteractive reference output remains full-size and exact where budget admits.
All ordinary raster allocations and final registrations check the current lower
aggregate cap using real retained registry capacities, keeping Step 2's 1-MiB
control reserve outside usable payload room. Even a legal context smaller than
that reserve reports a cap no higher than its actual limit and rejects new work.
Pressure updates can occur
during a render; final registration and PRV1 checks decide eligibility regardless
of cancellation latency. Noninteractive reference pixels keep their exact
semantics and may reject on budget exhaustion; no approximation of committed
operations, reduced original resolution or parameter rewrite is permitted.
Prior valid display handles/copies remain readable after revocation or rejection.

## Failures, diagnostics and persistence

ABI/null/malformed/stale-context/observation errors preserve state/output. Epoch
exhaustion alone adds the described fail-closed stop. No allocation-failure site
exists in policy updates; fallible-allocation tests prove this while real render
allocation faults retain existing handling. Reason codes: BASELINE=0, THERMAL=1,
MEMORY=2, CONSTRAINED=3, BACKEND=4, BUDGET=5, UNKNOWN_FACTS=6. Admission retains
Step 2's versioned decisions/reasons, including unavailable vs unvalidated.

Policy/thermal/pressure/tier/epoch are runtime-only, never serialized into
documents/history/SQLite. Schema 3, immutable original, canonical operation/replay
semantics and all retained Phase 0–3 gates remain. Reopen derives a fresh policy.
No new production dependency, network/telemetry, AI/model/inference/Phase 5 scope,
GPU kernel expansion or product UI. Stop after Step 3; Step 4 NOT STARTED.
