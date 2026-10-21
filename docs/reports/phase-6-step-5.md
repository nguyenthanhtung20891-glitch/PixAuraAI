# Phase 6 Step 5 — plan validation, human approval and execution bridge

> Historical Step 5 engineering report. Accepted final closure evidence and current
> integrated Phase 6 status are recorded in [Phase 6 closure](phase-6-closure.md).
> Pending/future-step language below describes the original report time only.


Step 5 COMPLETE / FULL PASS.
Implementation certification SHA: `266a1b6f9a9e2ca50c86b11387bedafeab236d87`.
Foundation 39200707896 SUCCESS 7/7; Native application shells 39200707963 SUCCESS 3/3.
The closure documentation commit has its own final SHA and requires both complete
workflows independently; its observed receipt and final delivery record that evidence.
Baseline `7fbde1668be48da347db084037f3ff12e5624445`, clean at audit.
Step 4 closure Foundation 39194577029 SUCCESS 7/7; Native shells 39194577014
SUCCESS 3/3. Step 6 NOT STARTED. Phase 7 NOT STARTED.

## Audit and implementation

Audited product/architecture/quality/roadmap/decisions, ADRs 0004/0009,
Steps 1–4 contracts/code/tests, immutable document/revision/session envelopes,
existing application-service persistence/publication, manual registry/parameter
validation, evaluator/cancellation, resource accounting, native ABI, Android/Swift
owner locks, stop/join/destroy and Phase 4 policy integration. The existing
DetachedCandidate transition already supports a complete stack in one revision;
no atomic-history redesign or schema change is necessary.

[Approval/execution v1](../contracts/editing-approval-execution-v1.md) adds an
explicit trusted human application approval API. Native approval is an authority
boundary against untrusted planner data, not a sandbox against dishonest native
application code. The planner receives no native approval capability.
Validation, approval and execution are separate. Canonical Plan v1 plus actual
service live state passes existing Step 2 parsing and public Step 3 binding before
approval and again before execution. Inspection exposes complete canonical bytes.
Existing evaluator reservations and Phase 4 policy admission establish full-stack
geometry/resource eligibility before approval and consumption. Agent execution
requires a configured trusted policy; legacy manual-only operation is unchanged.
No JSON field, actor, timer or model confidence grants approval.

SHA-256 over exact canonical bytes including LF identifies the approved plan.
Full bytes, native live snapshot and owning context must match; digest alone never
validates. Core-minted typed tokens reserve existing proposal leases, bounded to
eight within the unchanged combined 64-handle ceiling. Revoke/release/stop/destroy
are explicit. No persistent approval, new status, new context owner or scheduler.

Consumption is single-use: prevalidation/pre-cancel/stale/admission cancellation
does not consume. Once execution starts, approval is permanently consumed even
if allocation, evaluator, cancellation or persistence later fails. No retry,
fallback recipe, implicit reapproval or modified-plan execution.
Review tightened admission: capture the cancellation epoch before validation and
reserve consumption under the same cancellation fence. Cancellation during
validation cannot be missed, and cancellation versus consumption has one winner.
A deterministic pre-validation checkpoint verifies rejection without consumption.
The first implementation workflow exposed GCC test indentation diagnostics,
MSVC default-C static-assert syntax and missing C++ UBSan runtime linkage for the
independent C consumer. Explicit braces, portable compile-time layout checks and
the existing C-consumer C++ linker pattern fix those issues without suppressions.

The existing editor Service owns live state and serializes mutation. Private
full-stack evaluation uses the same cancellable geometry/tone/detail engine,
immutable original and bounded raster/scratch accounting. One DetachedCandidate
and existing repository epoch-CAS/accept create one ordinary revision. Earlier
private actions never become intermediate history. Existing `ai`/plan attribution
fields are reused without Schema 3 changes. Undo/redo and branch-after-undo use
the existing path. Cancellation and durable publication have an explicit winner;
advanced asynchronous candidate orchestration remains Step 6.

## Evidence and certification

Focused Linux approval/execution tests and actual release-object scans PASS.
Observed local retained gates: Windows native 46/46 PASS, Linux native 47/47
PASS, Windows source 113/113 PASS and Linux source 155/155 PASS with zero skips.
Final affected approval/execution, independent C ABI and normal-object isolation
tests: 3/3 PASS on Windows, Linux and active ASan/UBSan. Full sanitizer suite
49/49 PASS; negative probes confirm actual ASan/UBSan instrumentation (exits
86/87), with halt-on-error and no suppressions. Exact-SHA implementation workflows both completed SUCCESS as recorded above.
An initial full sanitizer run had a loader failure (shared library briefly too
short during a concurrent local relink), not a sanitizer finding. The complete
suite was rebuilt and passed 49/49 without concurrent relinking; that initial
attempt does not count as PASS.
Both complete implementation workflows above PASS: Windows/Linux/Apple native,
Android arm64-v8a and armeabi-v7a, Android debug/release/JVM/lint/instrumentation,
Swift macOS/iOS Simulator, iOS app/UI, all retained Phase 0–5/Steps 1–4/manual
gates, active ASan/UBSan, release isolation, source consistency and workflow lint.
Apple results are hosted/simulator software evidence only, not physical certification.
Local git diff --check PASS. No gate was skipped, loosened or substituted.

Observed implementation counts: Windows native 46/46, Linux native 47/47,
Apple native 47/47, ASan/UBSan 49/49, complete source 155/155. New execution
corpus: 1039 assertions and zero failures on Windows/Linux/ASan; Apple native
execution test PASS. Swift macOS and iOS Simulator each 32/32, Android emulator
31/31, iOS app 7/7 and UI 2/2 PASS. Both Android ARM ABI jobs PASS.

Native certification covers repo-owned single/crop-rotate/tone/detail/eight-action
plans, exact digest/bytes, unchanged source, one revision, order, undo/redo/branch,
no approval, consumed/revoked/released/wrong-type/random/cross-context authority,
changed parameters, stale source, size/version/reserved/overlap defenses,
cancellation checkpoints, later-action rollback and execution/revocation/release
races. Independent C compilation asserts token/request/receipt layouts. Test
injection is certification-only, with actual normal-object scans.

A: approval authority, exact validation/binding, atomic execution, regression and
certification evidence. B: advanced Step 6 lifecycle, Step 7 platform certification
and Step 8 closure. C: optional UI/NLP/model/research. Fix A only. No architecture
blocker identified; no frozen contract reinterpretation or ADR supersession.

Intent v1 unchanged. Structured Intent v1 unchanged. Plan v1 unchanged.
Registry v1 unchanged. Schema 3 and Phase 5 unchanged. Manual editing unchanged.
Production model UNRESOLVED. No NLP/model, cloud/network, external runtime, weights
or AI UX. DH-APPLE-METAL-01 DEFERRED, NOT WAIVED, NOT SATISFIED.
Step 6 NOT STARTED. Phase 7 NOT STARTED. STOP AFTER STEP 5.

## Closure authority

Current authority and the eight-step roadmap agree: Steps 1–5 COMPLETE / FULL PASS;
Steps 6–8 NOT STARTED; Phase 7 NOT STARTED. Source consistency checks enforce
that state and require observed implementation SHA/workflow evidence on closure.
Checks scope the current Phase 6 Step 5 section and reject conflicting status
lines; unrelated historical Phase 5 Step 5 authorization cannot mask closure.
This closure-only documentation commit independently runs both complete workflows
on its own SHA; no implementation result substitutes for its certification.
No runtime/product semantics change in the closure commit. STOP AFTER STEP 5.
