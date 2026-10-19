# Phase 6 Step 6 - candidate lifecycle, cancellation and stale protection

## Phase 6 Step 6 - current authority

Step 5 COMPLETE / FULL PASS at `80d9fb05b7ceb1f6dbb464fe371070e236a33bad`:
Foundation 39202326671 SUCCESS 7/7; Native application shells 39202326788 SUCCESS 3/3.
Step 6 AUTHORIZED / IN PROGRESS. Exact-SHA certification pending.
Candidate lifecycle v1 governs one private execution per service, source-generation
recheck, cancellation/publication fencing and stop -> join -> destroy.
Approval authority and consumption remain frozen Step 5 semantics.
Full retained Phase 0-5 and Steps 1-5, manual, lifecycle/race/failure, native hosts,
Android both ABIs/apps, Apple hosted/simulator apps, active ASan/UBSan,
release/test isolation, workflow lint and diff checks required.
Intent v1, Structured Intent v1, Plan v1, Registry v1, Schema 3 and Phase 5 unchanged.
Production model UNRESOLVED. No NLP/model, external runtime, weights or AI UX.
DH-APPLE-METAL-01 DEFERRED, NOT WAIVED, NOT SATISFIED.
Step 7 NOT STARTED. Phase 7 NOT STARTED. No Step 9+. STOP after Step 6.
Historical authority follows.

## Evidence

Implementation and certification in progress. No unobserved gate is PASS.
## Audit and implementation

Audited Step 5 bridge, native Service ownership, DetachedCandidate transition,
repository transaction/epoch CAS, immutable original, preview/gesture generation
contracts, ADR 0009, Phase 4 admission and existing cooperative raster cancellation.
Platform ownership remains stop/join/destroy; no wrapper or app semantic change.
The one active candidate is internal and ephemeral: no new public handle or ABI.
CREATED -> RUNNING -> READY -> PUBLISHED, with terminal CANCELLED/STALE/FAILED.
Private work releases the editor owner lock. Publication reacquires owner authority,
checks nonreused source live serial (including undo/redo ABA), checks no new manual
candidate, then uses existing repository CAS and cancellation publication fence.
No approval-map references survive the unlocked stage. Consumed approval release
cannot invalidate private copied identity. BUSY never transfers approval or queues.

## Observed local evidence (certification still pending)

Full Windows native 46/46 PASS. Focused final native execution/C ABI/normal release
object scan 3/3 PASS; execution corpus 1471 assertions, zero failures.
Full retained source suite 156/156 PASS, zero skips.
Deterministic interleavings cover manual commit, undo/redo ABA, redo, stop, cancel,
manual gesture interruption, consumed approval release, dual execution BUSY,
external repository session acquisition, and late cancellation after publication.
Separate repository reads prove no unpublished durable revision, including stop
and SQL failure after updates/before COMMIT. Existing multi/eight-action, repeated
operations, undo/redo/branch, allocation, later-action rollback and 72-failure
resource reclamation tests retained. New hooks exist only in certification builds;
normal editor AND storage objects are scanned for hook absence.
Linux/sanitizer and exact-SHA platform evidence are recorded only when observed.

## Scope classification

A: candidate runtime, cancellation/stale/publication correctness and retained
certification. B: Step 7 integration/security certification and Step 8 closure,
not started. C: UI/model/NLP/research, not undertaken. No architecture blocker.
No new tools, approval authority, persistence, inference/profile/trust semantics,
cloud or runtime. No implicit retry/replan/rebase. No crash claim beyond existing
repository transactions. Apple hosted/simulator evidence is not physical evidence.

Workflow lint PASS. git diff --check PASS. Nonwrapping cancellation epoch
exhaustion rejects execute/cancel without mutation. Local launcher diagnostics:
CMake required its existing explicit SDK path/process-launch permission; Linux
launcher line endings were corrected before native test execution. No test or
security control was disabled.

Full retained ASan/UBSan 49/49 PASS. Negative probes confirmed instrumentation
(active ASan exit 86, UBSan exit 87); no suppressions. Windows-applicable source
gates 104/104 PASS. Final expanded lifecycle/ABI/release-isolation/instrumentation rerun 5/5 PASS.

## Resolved test-boundary finding and local filesystem limitation

The expanded durable-read test initially linked a second static SQLite runtime
into the same process as the shared core. Linux process-scoped file locks cannot
coordinate separate SQLite lock tables; a synthetic durable read saw old state.
Removed that test linkage and moved bounded inspection/external-session injection
inside the existing core certification boundary, sharing the authoritative SQLite
runtime. Production implementation was unaffected by the test-linkage defect.
Final Windows 3/3 and sanitizer 5/5 focused gates PASS, 1471 assertions each.

The local regular Linux run on Windows-mounted storage initially had three
failures: the resolved test-linkage defect plus retained storage permission and
Android-publication filesystem cases under repository-local TMPDIR on DrvFS.
Those filesystem checks are not disabled or weakened. Full active ASan/UBSan
49/49 passed using the retained sanitizer environment; native Linux CI on its
native filesystem must independently PASS all 47 retained tests before closure.

Final focused Linux execution/C ABI/production-object scan 3/3 PASS,
1471 assertions and zero failures. Final focused source checks 7/7 PASS.

## Exact-SHA CI remediation

Initial implementation b4baf2fa0ad6618b75267ccc03352e36e7431df4 is not certification.
Foundation 39250793603 Windows compilation rejected a shadowed test local under
MSVC C4456 /WX. Renamed only that local. Native shells 39250793480 Android debug
linked the core's test-only storage hook without the storage target's matching
certification define (Android host tests are OFF). Added the same define inside
the existing Android debug-only certification guard; release remains OFF.
A source regression check freezes that shared guard. No warning suppression,
release hook, operation or lifecycle semantic change. New SHA needs both complete
workflows; no old-SHA results substitute.

Remediation verification: local Android arm64 debug full native build/link PASS;
Windows focused native 3/3 and source checks 8/8 PASS. Release flag remains OFF.
