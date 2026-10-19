# Architectural decision index

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


## Phase 6 Step 5 — current authority

Step 4 COMPLETE / FULL PASS at closure SHA
`7fbde1668be48da347db084037f3ff12e5624445`:
Foundation 39194577029 SUCCESS 7/7; Native application shells 39194577014 SUCCESS 3/3.
Step 5 COMPLETE / FULL PASS.
Implementation certification SHA: `266a1b6f9a9e2ca50c86b11387bedafeab236d87`.
Foundation 39200707896 SUCCESS 7/7; Native application shells 39200707963 SUCCESS 3/3.
The closure documentation commit has its own final SHA and requires both complete
workflows independently; its observed receipt and final delivery record that evidence.
[Approval/execution v1](docs/contracts/editing-approval-execution-v1.md) and
[Step 5 report](docs/reports/phase-6-step-5.md) govern trusted human application
approval, exact canonical Plan v1 binding, single-use ephemeral approvals and the
existing editor service's atomic deterministic execution/durable history path.
No planner/model approval, JSON authority, silent retry or parallel history.
Intent v1, Structured Intent v1, Plan v1, Registry v1, Schema 3 and Phase 5 unchanged.
Full retained Phase 0–5/Steps 1–4/manual, malformed/forgery/lifecycle/race/atomicity,
Windows/Linux/Apple/Android, active ASan/UBSan, release isolation, source consistency,
workflow lint and diff checks required. Both complete exact-SHA workflows required.
Production model UNRESOLVED. No NLP/model, external runtime, weights or AI UX.
DH-APPLE-METAL-01 DEFERRED, NOT WAIVED, NOT SATISFIED. Step 6 NOT STARTED.
Phase 7 NOT STARTED. No Step 9+. STOP after Step 5. Historical authority follows.

## Phase 6 Step 4 — current authority

Step 3 COMPLETE / FULL PASS at `fa6462e2bc1c0a88b30a46e69a9d982f8d843fe4`:
Foundation 39131908247 SUCCESS 7/7; Native application shells 39131908300 SUCCESS 3/3.
Step 4 COMPLETE / FULL PASS. Certified implementation SHA:
`7c00b9193c649fb2f7addf2dc6cd6a558a84dee5`.
Foundation 39136505109 SUCCESS 7/7; Native application shells 39136505094 SUCCESS 3/3.
This closure-only documentation correction preserves that implementation evidence;
its new commit requires independent certification by both complete workflows.
Approved Option B: separate [Structured Intent v1](docs/contracts/editing-structured-intent-v1.md)
and [deterministic reference planner v1](docs/contracts/editing-reference-planner-v1.md).
Eleven explicit one-to-one goal/tool mappings; exact existing parameter validation,
canonical Plan v1 and Registry v1 binding. Free-form Intent v1 unchanged and never
interpreted. No multi-tool release recipe, NLP, model, approval, execution or AI UX.
[Step 4 report](docs/reports/phase-6-step-4.md) records bounded parsing/planning,
cancellation, canonical-byte goldens and retained evidence. Both complete final-SHA
workflows, Windows/Linux/Apple/Android, active ASan/UBSan, full retained Phase 0–5
and Steps 1–3/manual gates, release isolation, workflow lint and diff check required.
Production model UNRESOLVED. Schema 3, Plan v1, Registry v1 and Phase 5 unchanged.
No external runtime or weights. DH-APPLE-METAL-01 unchanged: DEFERRED, NOT WAIVED,
NOT SATISFIED. Step 5 NOT STARTED. Phase 7 NOT STARTED. No Step 9+. STOP after Step 4.
Historical authorization below is retained.

## Approved structured-intent architecture decision — Phase 6 Step 4

Product Owner decision at baseline `fa6462e2bc1c0a88b30a46e69a9d982f8d843fe4`:

Phase 6 Step 4 may introduce a separately versioned structured-intent contract
without modifying free-form intent v1.

The initial stop gate was triggered because free-form intent v1 has no structured
operation/amount semantics. Approved Option B resolves that gap by adding a separate
explicit machine-semantic envelope. Free-form Intent v1 -> future NLP/model input
boundary; Structured Intent v1 -> deterministic reference planner -> unchanged
Plan v1 -> later human approval -> existing deterministic execution. Only the
structured-input/reference-planner layers are implemented in Step 4. No NLP,
heuristic, model, release multi-tool recipe, approval or execution authority.
Eleven explicit goal IDs select exactly one accepted tool tuple 1/1; all numeric
semantics remain authoritative in existing manual/document validators. Unknown,
missing, extra, ambiguous and invalid data reject. Intent, plan and execution remain
distinct. Schema 3, Phase 5, manual editing and DH-APPLE-METAL-01 unchanged.
Step 5 NOT STARTED. Phase 7 NOT STARTED. This decision supersedes no historical ADR.


## Phase 6 Step 3 — current authority

Step 2 COMPLETE / FULL PASS at `578958979f2fc4b5fea4b53dfc3ecfee7aafe34f`:
Foundation 39106599021 SUCCESS 7/7; Native application shells 39106599002 SUCCESS 3/3.
Step 3 AUTHORIZED / IN PROGRESS; exact-SHA certification pending.
[Editing tool registry v1](docs/contracts/editing-tool-registry-v1.md) and
[Step 3 report](docs/reports/phase-6-step-3.md) govern immutable manual-derived
editing metadata and canonical plan binding. Eleven explicitly allowed tools,
exact existing integer/version/parameter semantics, deny by default. No plan
mutation, approval, execution, planner, dynamic registration, persistence or AI UX.
Both complete final-SHA workflows, registry/table/binding/security/side-effect
and release-isolation tests, retained Phase 0–5 and Steps 1–2/manual gates,
Windows/Linux/Apple/Android, active ASan/UBSan, workflow lint and diff check required.
Production model UNRESOLVED. Schema 3 and Phase 5 unchanged. No runtime or weights.
DH-APPLE-METAL-01 unchanged: DEFERRED, NOT WAIVED, NOT SATISFIED.
Step 4 NOT STARTED. Phase 7 NOT STARTED. No Step 9+. STOP after Step 3.
Historical authorization below is retained.


## Phase 6 Step 2 — current authority

Step 1 COMPLETE / FULL PASS at `f07016da62556b0634fde8ed081b1361b8c9eb57`:
Foundation 39104299393 SUCCESS 7/7; Native application shells 39104299376 SUCCESS 3/3.
Step 2 AUTHORIZED / IN PROGRESS; exact-SHA certification pending.
[Intent/plan v1](docs/contracts/editing-intent-plan-v1.md) and
[Step 2 report](docs/reports/phase-6-step-2.md) freeze separate bounded canonical
objects with native document/revision/session/context binding. Shared JSON and
operation semantics retained. Max eight actions and 8192 bytes. Unknown fields,
versions and malformed data fail closed. No new handles, persistence, planner,
capability registry, approval issuer or execution bridge. No AI UX.
Both complete final-SHA workflows, goldens/security/canonical-byte/context and
cancellation tests, retained Step 1 and Phase 0–5/manual gates, active ASan/UBSan,
Windows/Linux/Apple/Android, workflow lint and diff check are required.
Production model UNRESOLVED; Schema 3, manual and Phase 5 semantics unchanged.
No external runtime or weights. DH-APPLE-METAL-01 unchanged and DEFERRED,
NOT WAIVED, NOT SATISFIED. Step 3 NOT STARTED. Phase 7 NOT STARTED.
No Step 9+. STOP after Step 2. Historical authorization below is retained.


## Phase 6 Step 1 — current authority

Phase 5 CLOSED / COMPLETE / FULL PASS at
`a66bbb0a303b707d48590df37cbd369148fba6a8`: Foundation 39087829531 SUCCESS
7/7 and Native application shells 39087829537 SUCCESS 3/3.
Phase 6 Step 1 AUTHORIZED / IN PROGRESS; exact-SHA certification pending.
[Editing Agent core v1](docs/contracts/editing-agent-core-v1.md),
[Step 1 report](docs/reports/phase-6-step-1.md) and
[frozen eight-step roadmap](docs/plans/phase-6-roadmap.md) govern current work.
Human approval remains authoritative. Proposals never mutate document/history;
reuse native document/manual semantics. Intent, plan and execution stay distinct.
Step 1 stages bounded operations only; no plan schema, planner, approval issuer,
execution bridge, AI UX or new runtime. Both complete final-SHA workflows,
agent security/side-effect/approval-bypass tests, retained Phase 0–5/manual gates,
Windows/Linux/Apple/Android ARM ABIs, active ASan/UBSan, workflow lint and diff
check are required. A: current boundary/evidence; B: later authorized steps;
C: optional research. No architecture conflict or new ADR.
Production model UNRESOLVED; Schema 3/profile unchanged; no weights/runtime.
CPU release VALIDATED; Vulkan/Metal UNKNOWN. DH-APPLE-METAL-01 DEFERRED,
NOT WAIVED, NOT SATISFIED. Step 2 NOT STARTED. Phase 7 NOT STARTED.
No Step 9+. STOP after Phase 6 Step 1. Historical authority below is retained.


## Phase 5 Step 8 — closure authority

Committed candidate status: PENDING_EXACT_SHA. Steps 1–7 COMPLETE / FULL PASS;
Step 8 authorized for closure/certification only at
`b46b30c26af0790d0715bc48d24093b83290b8fe`.
Phase 5 becomes CLOSED / COMPLETE / FULL PASS only after BOTH complete workflows
succeed on the final Step 8 SHA. The observed engineering receipt and final
delivery resolve status and Step 8 identity; no baseline or pending job suffices.
[Closure report](docs/reports/phase-5-closure.md),
[Step 8 report](docs/reports/phase-5-step-8.md), and
[candidate manifest](docs/reports/phase-5-closure-v1.json) record the finite scope.
CPU release trust VALIDATED; Vulkan and Metal release trust UNKNOWN. All three
accepted software suites PASS, retained again on the closure SHA. No provider is
CERTIFIED. Production model UNRESOLVED. DH-APPLE-METAL-01 DEFERRED, NOT WAIVED,
NOT SATISFIED; no physical Apple inference certification. Schema 3/profile unchanged;
no weights, external runtime, AI UX or Editing Agent. Phase 6 NOT STARTED.
STOP after Phase 5 closure; no Step 9+. This supersedes historical scope below.

<!-- historical-authority -->

## Phase 5 Step 7 current authorization

Step 6 COMPLETE / FULL PASS at `5592cd839c2607379c6cb8c02a4b42a44e80311d`,
accepted by Product Owner. Step 7 AUTHORIZED / IN PROGRESS under approved Option A:
software parity/security evidence is separate from release trust. CPU remains
VALIDATED; Vulkan and Metal remain UNKNOWN. No hosted-evidence promotion policy.
[Certification contract](docs/contracts/inference-provider-certification-v1.md)
and [Step 7 report](docs/reports/phase-5-step-7.md) govern the complete retained
suite and both complete same-final-SHA workflows. Physical evidence stays separate;
DH-APPLE-METAL-01 unchanged and DEFERRED. Production model UNRESOLVED, profile and
Schema 3 unchanged. No weights, external runtime or AI UX. Step 8 and Phase 6
NOT STARTED. Stop after Step 7. Supersedes historical authorization text below.


## Phase 5 Step 6 current authorization

Step 5 COMPLETE / FULL PASS at `59d846967fffef7e735597f6b65301841659336f`:
Foundation 38999701451 (7/7), Native application shells 38999701458 (3/3).
Step 6 AUTHORIZED / IN PROGRESS: deterministic capability-aware inference
admission and PRE-EXECUTION fallback only. Reuse Phase 4 policy authority and
existing provider/package semantics. Accelerated UNKNOWN trust remains ineligible
for automatic selection. Production model selection UNRESOLVED. Require retained
Steps 1-5, malformed/resource/epoch/concurrency tests, sanitizers and both complete
same-final-SHA workflows. [Admission contract](docs/contracts/inference-admission-v1.md),
[Step 6 report](docs/reports/phase-5-step-6.md). Schema 3, provider profiles and
DH-APPLE-METAL-01 unchanged. No AI UX, runtime/model expansion, Step 7 or Phase 6.
Stop after Step 6. This authority supersedes historical authorization text below.

## Phase 5 Step 5 current authorization

Step 4 COMPLETE / FULL PASS at `a3ad7e7a61030992d933b3ca68572f83604dd5a9`:
Foundation 38730904780 (7/7), Native application shells 38730904823 (3/3).
Step 5 AUTHORIZED / IN PROGRESS under approved Option A: bounded synthetic
segmentation evaluation only. [Evaluation contract](docs/contracts/segmentation-evaluation-v1.md),
[Step 5 report](docs/reports/phase-5-step-5.md). Production selection UNRESOLVED;
U2NETP audited/unsupported, no weights/runtime/profile expansion. Require metrics,
independent goldens, corpus, retained Steps 1-4/sanitizers and both complete
same-final-SHA workflows; no unobserved PASS. Schema 3 and DH-APPLE-METAL-01
unchanged. No AI UX, Step 6+ or Phase 6. Stop after Step 5. This supersedes
historical authorization text below without rewriting prior closure.

## Phase 5 Step 4 current authorization

Step 3 COMPLETE / FULL PASS at `bf31eb33d5b5027236d71cf56da4c648d6b56255`:
Foundation 38685139496 (7/7), Native application shells 38685139454 (3/3).
Step 4 AUTHORIZED / IN PROGRESS. Apple-specific Option A explicitly permits
`apple.metal.v1` to consume the authenticated portable CPU ONNX record ONLY
for the frozen reference profile. Mandatory CPU compatibility, separate Metal
trust, explicit selection, no package-schema extension or Core ML execution.
[Apple contract](docs/contracts/apple-inference-provider-v1.md),
[Step 4 report](docs/reports/phase-5-step-4.md). Both complete same-final-SHA
workflows govern closure. Schema 3 and DH-APPLE-METAL-01 unchanged; no physical
Apple claim. Steps 5+ and Phase 6 NOT STARTED. Stop after Step 4.
This supersedes historical Step 3 stop text below without reopening closure.

## Phase 5 Step 3 current authorization

Step 2 COMPLETE / FULL PASS at `58202caf1fd86215abe4650b208b2134cdca9b58`:
Foundation 38605609607 (7/7), Native application shells 38605609516 (3/3).
Step 3 AUTHORIZED / IN PROGRESS. Approved Option A permits `android.vulkan.v1`
to consume the authenticated portable CPU ONNX compatibility record ONLY for the
exact frozen reference profile. No package schema extension or new dependency.
[Android contract](docs/contracts/android-inference-provider-v1.md),
[Step 3 report](docs/reports/phase-5-step-3.md). CPU reference remains authority;
Vulkan availability is separate from validation/certification. Explicit selection;
Step 6 owns admission/fallback. Both complete same-final-SHA workflows required.
Schema 3 and DH-APPLE-METAL-01 unchanged. Steps 4+ and Phase 6 NOT STARTED.
This supersedes historical Step 2 stop text below. Stop after Step 3.

## Phase 5 Step 2 current authorization

Step 1 COMPLETE / FULL PASS at `8d5a30338c97a05e987ecf1324b86a2afb795356`: Foundation
38491704459 (7/7), Native application shells 38491704500 (3/3).
Step 2 AUTHORIZED / IN PROGRESS. Product Owner approves dependency-free
`cpu.reference.v1` with a narrowly bounded genuine ONNX reference profile.
[Inference contract](docs/contracts/inference-provider-v1.md),
[CPU profile](docs/contracts/cpu-reference-provider-v1.md),
[Step 2 report](docs/reports/phase-5-step-2.md). No general ONNX execution or
external runtime. Complete same-final-SHA Foundation/application-shell workflows
are required for closure. Schema 3 and DH-APPLE-METAL-01 unchanged.
Steps 3+ and Phase 6 NOT STARTED; stop after Step 2. This current authority
supersedes historical Step 1-only stop text below, without rewriting closure evidence.

## Phase 5 Step 1 signature architecture

Product Owner approves pure Ed25519 with one shared portable native verification
authority. [ADR 0022](docs/adr/0022-model-package-ed25519-authority.md) records the
decision and accepts libsodium 1.0.22 (ISC), approved archive SHA-256
`adbdd8f16149e81ac6078a03aca6fc03b592b89ef7b5ed83841c086191be3349`.
[Candidate audit](docs/reviews/phase-5-ed25519-dependency-audit.md) recommends
libsodium 1.0.22; Monocypher NOT selected. Exact source hashes/static portable
configuration retained. Step 1 IN PROGRESS through implementation/certification.
Eight-step roadmap FROZEN, Steps 2+ and Phase 6 NOT STARTED; Schema 3 and
DH-APPLE-METAL-01 unchanged. This supersedes historical Phase 5 stop points only.

## Phase 4 final closure

**Phase 4 — CLOSED / COMPLETE / FULL PASS.** All Steps 1–7 COMPLETE / FULL PASS.
Authorized closure baseline `3d5db65b82e16bb18cca7b906b5108b70fc95956` passed
Foundation 38419049842 (7/7) and Native application shells 38419049786 (3/3).
[Consolidated Step 7 report](docs/reports/phase-4-step-7.md) freezes retained
contracts, limits, evidence classification and exit certification. Effective
closure requires BOTH complete workflows passing on the final commit containing
this status; delivery records its exact SHA/run IDs. Prior-SHA green is historical.
Adapters provide observations; shared capability/admission/dynamic policy and
existing PRV1/document/storage owners remain authoritative. Schema 3 and hard
ceilings unchanged. No unresolved Category A or architecture conflict.
DH-APPLE-METAL-01 remains DEFERRED at its existing hardening/Beta/release/physical
claim boundaries. No physical Apple certification claim. No Step 8 or Phase 4.5.
Phase 5 NOT STARTED; no AI/model/NPU work. STOP after Phase 4 closure.
This current status supersedes historical authorization/stop points below.


## Phase 4 Step 6 authorization

At `ce8d513764d6167aa6756b22ef5b0f81698c11c9` Product Owner authorizes bounded
cross-tier replay certification; Steps 1–5 FULL PASS, Step 7 NOT STARTED.
Step 6 COMPLETE / FULL PASS at `417f127d6018448910886d1760e1f8122bcecc11`:
Foundation 38414896874 (7/7), Native application shells 38414897742 (3/3), both
complete SUCCESS on that implementation SHA.
The [contract](docs/contracts/cross-tier-replay-certification-v1.md) composes
existing canonical storage, shared policy, CPU renderer and PRV1 owners.
Critical admission failure preserving durable ownership uses preparation-before-
acquisition and existing-epoch CAS, not a new architecture decision. No new ADR,
schema, limits, physical Apple certification or Phase 5 authorization.
This supersedes historical step authorization below. Stop after Step 6.

## Phase 4 Step 5 acceptance

Apple integration COMPLETE / FULL PASS at `944517952252cf1c9e6970bb3dfb2c62fe7f908a`.
Foundation 38355260289 (7/7) and Native application shells 38355259361 (3/3)
are both complete SUCCESS. [Contract](docs/contracts/apple-capability-integration-v1.md)
and [report](docs/reports/phase-4-step-5.md) compose accepted contracts without
a new architectural decision or ADR. Static/dynamic facts remain separate;
shared core owns policy, existing PRV1/display lock owns publication. Schema 3,
hard ceilings and deferred DH-APPLE-METAL-01 unchanged. No physical Apple claim,
Phase 5/AI/model work or Step 6 authorization. Frozen seven-step roadmap unchanged.
This supersedes historical authorization sections below. Stop after Step 5.

## Phase 4 Step 4 current authorization — Android capability integration

Product Owner authorizes Step 4 at `6389750c5ccab3b40d3ee76cb29bbb88b0fb5596`.
Steps 1–3 COMPLETE / FULL PASS; seven-step roadmap remains FROZEN.
[Android contract](docs/contracts/android-capability-integration-v1.md) and
[Step 4 report](docs/reports/phase-4-step-4.md) govern trusted local Android
observations into shared capability/admission/dynamic policy and existing
editor/PRV1 ownership. Step 4 COMPLETE / FULL PASS at `16483ff498f8a2ac67152f6a5b8fdac0e6d822fe`:
Foundation 38281943443 (7/7), Native application shells 38281943346 (3/3),
all SUCCESS on the same implementation SHA.
All retained source/native/sanitizer/Android/Apple gates and both complete
same-final-SHA hosted workflows must pass; unobserved gates are not PASS.
No Android tier table, schema change, UI, telemetry, new dependency or trust
expansion. Schema 3, hard ceilings and DH-APPLE-METAL-01 unchanged.
Steps 5–7 NOT STARTED; no Step 8+ or Phase 5/AI/model authorization.
Stop after Step 4. This supersedes historical Step 3-only authorization below.

## Phase 4 Step 3 authorization

Product Owner authorizes Dynamic Memory/Thermal Downgrade at
`d118cd26f9384c2f1735a2fbc57682c24a5990e7`; Steps 1/2 COMPLETE / FULL PASS.
[Dynamic policy v1](docs/contracts/dynamic-capability-policy-v1.md) freezes a
separate normalized enum family including CRITICAL, immediate componentwise
reductions, three distinct stable recovery observations and nonwrapping epochs.
Existing PRV1/raster/gesture owners remain authoritative; no frozen architecture
conflict or superseding ADR is required. Schema 3 and hard ceilings unchanged.
Frozen roadmap remains seven steps; Steps 4–7 NOT STARTED; no Step 8+ or Phase 5.
DH-APPLE-METAL-01 unchanged. Supersedes historical Step 2-only authorization.


## Phase 4 roadmap freeze and Step 2 authorization

Product Owner accepts Step 1 COMPLETE / FULL PASS at
`82fca1f6fe3a565a6704afe18cb5bc80cb9a9823` and freezes exactly seven roadmap steps.
Only Step 2 is authorized: [shared budget v1](docs/contracts/budget-admission-v1.md)
freezes T0/T1/T2 multi-input integer thresholds, lower runtime caps, checked peak
accounting and static optional preview/CPU fallback decisions. No architectural
supersession or new ownership is needed. All existing ceilings and certification
scope remain intact. Steps 3–7 NOT STARTED; no Step 8+ or Phase 5 authorization.
This supersedes historical proposed-roadmap/Step-1-review-pending wording below.
DH-APPLE-METAL-01 and ADR 0021 remain unchanged.


## Phase 4 Step 1 authorization and review boundary

The Product Owner authorizes audit, a finite proposed roadmap and minimum
capability data/probe foundation at `56cd60560b6271b7d27bf5e03fd1d720a3aad2fa`.
[Capability v1](docs/contracts/capability-profile-v1.md) follows the accepted
shared-core/adapter separation with no superseding architectural decision:
normalized metadata, explicit unknown facts and availability/validation distinction,
test-only synthetic scenarios, local bounded diagnostics and conservative stubs.
The [seven-step proposal](docs/plans/phase-4-roadmap.md) is NOT FROZEN. Step 1
review/acceptance and explicit roadmap freeze are required before Steps 2+.
Existing tier budget targets do not override frozen resource ceilings. No physical
Phase 4 tier is inferred from device names or historical GPU evidence. Phase 0–3
closure remains accepted; DH-APPLE-METAL-01 is unchanged. No Phase 5 authorization.

## Phase 3 Step 8 certification and closure

**Phase 3 — CLOSED / COMPLETE / FULL PASS.** Steps 1–8 and all frozen exit criteria are certified at implementation SHA `e73605501019afb1aca2a12c521539b68176618e`: Foundation 38144341158 (7/7) and Native application shells 38144341145 (3/3), all SUCCESS. Android teardown and iOS UI input-targeting remediation are validated. This closure supersedes historical not-started wording below. The documentation-only closure commit must also pass both complete exact-SHA workflows before delivery. No Step 9 is created and Phase 4 has NOT started. DH-APPLE-METAL-01 remains DEFERRED and mandatory at its existing later hardening/Beta/release boundaries. No new tool, numerical, AI, GPU, network, telemetry or storage-schema scope was added.

Evidence: [Step 8 report](docs/reports/phase-3-step-8.md) and [certification matrix](docs/contracts/phase-3-certification-v1.md).

ADR [0021](docs/adr/0021-android-no-replace-asset-publication.md) is Accepted by
the Product Owner / Architect on 2026-10-11. It supersedes ONLY ADR 0010's Android
asset publication syscall with atomic renameat2(RENAME_NOREPLACE), retaining
every immutable/no-overwrite/durability invariant. Other platforms are unchanged.

Phase 3 Step 7 is COMPLETE / FULL PASS (technical gates), authorized at baseline `116c4efc65f55025f717241a01bc6056ed27d042`.
[Manual editor integration](docs/contracts/manual-editor-integration-v1.md) composes existing gesture/PRV1/document/schema-3 authority; no new numerical semantics or storage schema.
Native, Android controller/Compose, Swift/macOS/iOS app/UI, persistence/reopen, parity and exact-SHA hosted gates are observed PASS. Step 8 has NOT started; DH-APPLE-METAL-01 unchanged.
Evidence: [Step 7 report](docs/reports/phase-3-step-7.md). Historical statuses below do not override this authorization.


Phase 3 Step 6 is authorized at baseline `311392faf9d5600598a5e541ba531dd5f722e996`.
[Gesture lifecycle integration v1](docs/contracts/gesture-lifecycle-integration-v1.md)
uses the existing controller, document-context mutex and PRV1 publication owner;
one active gesture per editing context, terminal revocation, retryable failed
commit and reject-active undo/redo/preset policy refine the accepted Step 1 rules.
Source binding checks the normalized original's verified bytes. Gesture state is
ephemeral, with no Schema 4 or new persistence authority. No frozen architecture
conflict or superseding ADR. Step 7/8 remain NOT STARTED.

Phase 3 Step 5: accepted operation batches already support one preset application as one detached manual revision proposal. [Preset v1](docs/contracts/presets-v1.md) freezes bounded canonical tone/detail recipes, excludes geometry, and ships an empty preset catalog. Normal operations/revisions retain inspectability and fit Schema 3; no Schema 4 or migration change is required. No parallel engine or AI behavior. Step 6 is not started.

Phase 3 Step 4 authorization freezes only scalar Sharpen/Blur under the [detail v1 contract](docs/contracts/manual-detail-v1.md). Schema 1/2 remain immutable; independent schema 3 and explicit transactional 2->3 are authorized Category A. Bounded three-row scratch preserves existing ceilings and two-raster ownership. [ADR 0020](docs/adr/0020-bounded-detail-reference.md) records this narrow extension; no automatic migration, direct 1->3, GPU expansion or Step 5 work.

Phase 3 Step 3 Architect decision, 2026-10-10: schema 1 remains immutable;
introduce independent storage schema 2 and explicit transactional 1->2 migration,
without automatic migration. Preserve every accepted source/history/envelope
value and canonical replay. [ADR 0019](docs/adr/0019-explicit-storage-schema-evolution.md)
records the narrowly approved SQLite transaction protocol and compatibility.
This resolves the Step 3 storage blocker; migration is Category A, not a new step.
The [tone/color contract](docs/contracts/manual-tone-color-v1.md) freezes the
initial six numerical recipes under Step 3 authorization; exposure 1/1 is unchanged.

## Phase 3 numbered roadmap freeze (2026-10-08)

Authority: explicit Product Owner / Architect decision. The official Phase 3
sequence is frozen as [Steps 1–8](ROADMAP.md#frozen-phase-3-numbered-roadmap--manual-tools).
Step 1 is COMPLETE / FULL PASS at `b29123037effd76d984780f59889f1fcbb3c8c76`;
Steps 2–8 are NOT STARTED. This supersedes the earlier absence of a numbered
roadmap, not the accepted Step 1 operation/parameter contracts. Approved future
tool scopes still require their specified units/ranges/versioned semantics or
separate recipe freezes before implementation/shipping as applicable.

Apply [A/B/C scope discipline](QUALITY_GATES.md#phase-3-scope-discipline) to every
new proposal. No additional steps, shipping tools, Phase 3.5, AI behavior or
Phase 4 work may be introduced without explicit Product Owner approval. Close
Phase 3 at Step 8 only after all exit criteria pass; no automatic Step 9.
This task records the plan only and does not authorize Step 2 implementation.
DH-APPLE-METAL-01 and its existing decision/requirements remain unchanged.

## Phase 3 Step 1 authorization (2026-10-08)

The Product Owner authorizes only Manual Tool Contract & Registry Foundation after
Phase 2 closure at 67454642c3f918ef67618d78ace41cef150c1c03. Reuse accepted Phase 2
document/history/storage/preview semantics; descriptors freeze only accepted
operation tuples. Other required MVP tools remain PROPOSED until separately
reviewed parameter/behavior freezes. No numbered Phase 3 roadmap is committed;
later steps must not be assigned numbers or scope. This refines ADR 0008/0009
without superseding architecture. See [contract](docs/contracts/manual-tools-v1.md)
and [report](docs/reports/phase-3-step-1.md).

Phase 2 is CLOSED by the explicit Product Owner/Architect decision below. Steps 1-11 are accepted FULL PASS; the original first GPU pipeline is complete, hosted compile/regression is green, and physical Android Vulkan certification is accepted PASS 14/14. Apple physical Metal certification is DEFERRED, not waived, under [DH-APPLE-METAL-01](QUALITY_GATES.md#dh-apple-metal-01-deferred-physical-apple-metal-certification). Existing ceilings, CPU authority and GPU semantics remain unchanged. Phase 3 has not started; no additional Phase 2 work is authorized.

## Phase 2 closure decision (2026-10-08)

Authority: explicit Product Owner/Architect instruction, accepted Android evidence publication `bd10239c38f069a2d683b0f513d5c77a02995598`.

**Apple physical Metal certification deferred due unavailable hardware; mandatory before Beta/production; Phase 2 closure allowed.**

Apple execution is blocked by unavailable physical hardware, not implementation. This decision supersedes only the requirement in [ADR 0018](docs/adr/0018-physical-gpu-certification.md) and [GPU tile v1](docs/contracts/gpu-tile-v1.md) that Apple physical execution block Phase 2 closure. The historical ADR is retained unchanged; the physical certification requirement and every technical acceptance criterion remain mandatory. The gate is not waived or removed.

Track the deferred gate as **DH-APPLE-METAL-01**, owned by the Product Owner/Architect, at Phase 11 hardening acceptance and before Phase 12 Beta readiness completion or any production release. A physical Apple GPU certification claim requires actual physical Metal dispatch/completion/readback and 14/14 shared CPU parity, fallback disabled, exact tested source SHA and reviewed device/runtime evidence. Hosted compilation, simulator execution, Android PASS and this phase-closure decision cannot satisfy it. Hardware unavailability must continue to block those later acceptance/release boundaries until the gate passes; agents must not silently forget or auto-waive it.

Phase 2 Steps 1-11 remain FULL PASS; no additional step is added. The [exit-closure report](docs/reports/phase-2-exit-closure.md) preserves implementation, hosted and accepted physical Android evidence and limitations. This decision closes Phase 2 only and does not authorize Phase 3 implementation or further Phase 2 work.

| ADR | Status | Decision |
| --- | --- | --- |
| [0018](docs/adr/0018-physical-gpu-certification.md) | Accepted; Phase 2 closure timing superseded by the decision above | Physical Vulkan/Metal dispatch and shared CPU parity; bounded FP32 tiles and fallback; separate hardware certification; technical criteria remain mandatory |
| [0017](docs/adr/0017-interactive-preview-generation-fence.md) | Accepted | Fixed nonwrapping context request generations; latest-wins native and platform publication fences; stop/join ownership |
| [0001](docs/adr/0001-native-shells-shared-core.md) | Accepted | Native mobile UIs; shared C++17 C ABI |
| [0002](docs/adr/0002-local-project-history.md) | Accepted | Immutable originals, local SQLite + asset store, shared revision graph |
| [0003](docs/adr/0003-deterministic-photo-pipeline.md) | Accepted | CPU semantic oracle, platform GPU adapters, SDR first |
| [0004](docs/adr/0004-ai-execution-boundary.md) | Accepted | Typed local plans, bounded executor, explicit approval |
| [0005](docs/adr/0005-local-inference-and-tiers.md) | Accepted | Provider-neutral AI; ONNX interchange, measured progressive tiers |
| [0006](docs/adr/0006-build-and-security-policy.md) | Accepted | Multi-host quality gates and dependency/privacy policy |
| [0007](docs/adr/0007-shell-navigation-and-build-inputs.md) | Accepted | Typed minimal shell coordinators, pinned inputs, deterministic Xcode project |
| [0008](docs/adr/0008-document-stacks-and-manifest.md) | Accepted | Ordered stacks per retained revision, explicit undo/redo path, bounded canonical JSON checkpoints; SQLite remains live authority |
| [0009](docs/adr/0009-explicit-document-context.md) | Accepted | Explicit caller-owned document registry contexts; context-scoped opaque tokens; ABI 1 unchanged |
| [0010](docs/adr/0010-durable-native-project-storage.md) | Accepted | Normalized SQLite WAL/FULL authority, durable epochs, verified streamed SHA-256 asset publication and derived atomic checkpoints |
| [0011](docs/adr/0011-shared-bounded-sdr-decode.md) | Accepted | Shared portable bounded JPEG/PNG SDR decoders; supersedes platform codec ownership for these formats only; transient RGBA8 and configurable hard limits |
| [0012](docs/adr/0012-bounded-cpu-exposure-evaluation.md) | Accepted | Frozen CPU exposure recipe, finite unclamped RGB, bounded ordered stacks and one private output raster |
| [0013](docs/adr/0013-bounded-geometry-and-cancellation.md) | Accepted | Approved outward crop rasterization, ordered geometry preflight, lazy tiles and cooperative atomic cancellation |
| [0016](docs/adr/0016-bounded-bilinear-platform-preview.md) | Accepted | Rational-floor no-upscale preview fit; binary64 premultiplied-linear bilinear; bounded platform-owned Bitmap/CGImage copies; unchanged ceilings |
| [0015](docs/adr/0015-bounded-reference-preview.md) | Accepted | Independently owned 1:1 SDR RGBA8 preview; unchanged ceilings and explicit aggregate resource rejection/retry |
| [0014](docs/adr/0014-two-raster-geometry-execution.md) | Accepted | Exact ordered geometry movement within two rasters using in-place compaction and a bounded bitmap |

Accepted architecture does not imply an implementation or verified runtime. Exact models, Android GPU backend fallback coverage, shader tolerance and subscription grace remain evaluation items in relevant phases. Major changes require new numbered ADRs and index updates.

Phase 2 Step 5 refines existing ADR 0003/0009/0011 in the [CPU working-image contract](docs/contracts/cpu-working-image-v1.md): canonical premultiplied linear-sRGB RGBA32F, fixed bounded transfer tables, explicit unsupported profile transformation, existing context-scoped lifetime. No new dependency or superseding architecture decision is introduced.

Phase 2 Step 6: [ADR 0012](docs/adr/0012-bounded-cpu-exposure-evaluation.md) freezes exposure/1/1 numerical evaluation, finite unclamped RGB policy, 256-operation admission and one-private-destination buffering. No historical tuple, document/storage schema or ABI 1 meaning changes.

Phase 2 Step 7: [ADR 0013](docs/adr/0013-bounded-geometry-and-cancellation.md) records the Product Owner approved outward crop rasterization, transient ordered geometry plans, lazy 128x128 tiles and cooperative cancellation. Raster/context ceilings, historical tuples and ABI 1 remain unchanged.

Phase 2 Step 8: [ADR 0014](docs/adr/0014-two-raster-geometry-execution.md) activates exact crop/rotate movement and specifies in-place compaction/permutation under the two-raster limit with a <=1 MiB admitted bitmap. Historical operation meanings, raster/context ceilings and ABI 1 remain unchanged.
