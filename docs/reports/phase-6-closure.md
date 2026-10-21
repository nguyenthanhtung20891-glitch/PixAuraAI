# Phase 6 closure — Editing Agent foundation

Step 8 AUTHORIZED / IN PROGRESS. Closure certification pending.
Phase 7 NOT STARTED. This report is engineering evidence, never runtime authority.

## Purpose and audit method

Close the bounded provider-neutral Agent foundation as one system, without adding
intelligence or product capability. Clean main and origin/main were verified at
`2f5a42138d1a7270a37b8a52dc95061070195853`, before and after fetch. Both baseline
workflow roll-ups and all ten required jobs were independently observed completed
SUCCESS. Historical records below identify final closure commits, not earlier
implementation candidates.

Audit reconciles Steps 1–7 contracts/reports, PRODUCT_SPEC, current root authority,
ADR 0009, canonical schemas, the manual operation registry, editor service execution
and repository publication, native security/race tests, Kotlin/JNI and Swift/C
ownership, retained workflows and production release scanning. Frozen contract
LF-normalized SHA-256 values are in [the inventory](phase-6-closure-v1.json).
Contract bodies are unchanged. Their step-relative implementation/future-status
statements describe their original freeze scope; this report governs integrated
status. Prior reports and authority sections are explicitly historical.

| Step | Scope | Accepted final SHA | Foundation / shells runs | Status |
| --- | --- | --- | --- | --- |
| 1 | Core safety boundary | f07016da62556b0634fde8ed081b1361b8c9eb57 | 39104299393 / 39104299376 | COMPLETE / FULL PASS |
| 2 | Intent and Plan | 578958979f2fc4b5fea4b53dfc3ecfee7aafe34f | 39106599021 / 39106599002 | COMPLETE / FULL PASS |
| 3 | Registry and binding | fa6462e2bc1c0a88b30a46e69a9d982f8d843fe4 | 39131908247 / 39131908300 | COMPLETE / FULL PASS |
| 4 | Reference planner | 7fbde1668be48da347db084037f3ff12e5624445 | 39194577029 / 39194577014 | COMPLETE / FULL PASS |
| 5 | Approval and execution | 80d9fb05b7ceb1f6dbb464fe371070e236a33bad | 39202326671 / 39202326788 | COMPLETE / FULL PASS |
| 6 | Candidate lifecycle | 0251b485f844d70b496a525eb6541aea384ce51f | 39283971353 / 39283971479 | COMPLETE / FULL PASS |
| 7 | Platform/security integration | 2f5a42138d1a7270a37b8a52dc95061070195853 | 39613164583 / 39613164251 | COMPLETE / FULL PASS |
| 8 | Phase 6 closure | Resolved by final exact-SHA receipt | Fresh full workflows required | AUTHORIZED / IN PROGRESS |

Each accepted historical row has Foundation SUCCESS 7/7 and Native application
shells SUCCESS 3/3. These histories do not certify Step 8 changes.

## Final authority chain and frozen contracts

Free-form Intent v1 is bounded, untrusted, non-interpreted user intent. Separately
versioned Structured Intent v1 expresses one of eleven exact integer editing
requests. `reference.deterministic.v1` maps that request to one canonical Plan v1
action, using the existing canonical writer and Registry v1. It never interprets
prose. Plan v1 also supports ordered multi-action input; structure, version,
binding, tool and parameter validation precede approval eligibility.

The native editor service derives trusted source identity and repeats validation.
Only explicit human confirmation represented by trusted host application code
may create approval. Native code cannot authenticate a physical button press;
the application is trusted, model/payload data is not. Approval binds exact
canonical bytes, including terminal LF, with SHA-256 identity and full-byte
equality. It includes project/document/revision/session/context/generation/source
binding. JSON flags, model confidence, timers and actor labels confer no authority.

Approval creation/validation/inspection/revocation are inert. Execution without
matching live approval rejects. Pre-start admission failures preserve approval;
once execution begins it is permanently consumed, including cancellation, stale
source, resource/tool/storage failure or lost publication. BUSY does not consume
a competing approval. No retry, transfer, reapproval, rebase or hidden replan.

One private candidate runs per service: CREATED → RUNNING → READY → PUBLISHED,
or terminal CANCELLED, STALE, FAILED. No queue, exposed candidate handle,
resurrection or persistence. Source serial protects revision ABA, even undo back
to visibly identical R1. Private pixel work releases the owner lock; publication
rechecks source/cancellation/stop and uses the existing repository epoch CAS.
Owner → publication → raster cancellation is the lock order. Cancellation and
publication have one authority winner: cancel first gives no commit; publish
first gives one ordinary commit and late cancellation cannot undo it.

Existing deterministic manual operations execute in exact plan order in private
work. One atomic repository publication creates at most one ordinary revision,
with immutable original and shared history/undo/redo/branch semantics. Failure
publishes no partial action history. Manual edits and undo/redo invalidate stale
Agent work and remain available without planning or inference.

Frozen version-1 contracts: editing-agent-core, editing-intent-plan,
editing-tool-registry, editing-structured-intent, editing-reference-planner,
editing-approval-execution, editing-candidate-lifecycle and
agent-platform-integration-security. No v2 or semantic revision is introduced.

## Bounds and platform/security audit

Plan actions 1–8; canonical Plan ≤8192 bytes; Structured Intent ≤2048 bytes.
Eleven tools retain exact integer schemas, versions and bounds from manual
authority: crop, rotate, exposure, brightness, contrast, highlights, shadows,
saturation, temperature, sharpen, blur. No clamp/default guessing/substitution.
Combined native handle ceiling remains 64, approvals at most eight within the
existing proposal budget; one active candidate, no queue. Registry remains static
and immutable with existing descriptor limits. No memory/raster ceiling is raised.

Android Kotlin/JNI and Apple Swift/C remain transport/ownership adapters. The sole
Step 7 public addition `pixaura_editor_agent_plan` delegates existing trusted
planning through the native owner; no Step 8 ABI addition. Both wrappers retain
native status meaning, bounded copied plan data, typed approval ownership and
bounded in-flight call accounting. Shutdown is stop → join ALL calls → destroy
under ADR 0009. No native callback into platform-owned state survives shutdown.

Retained security corpora cover malformed/version/UTF-8/overflow/unknown tools,
wrong type/context/document/revision, stale/released/forged approvals, replay,
cancel/revoke/release/stop races, manual/undo/redo ABA, competing executions,
exactly-once publication, allocation/action/repository failure rollback and output
sentinels. Production object and release app scans reject test fixtures, keys,
synthetic trust and failure/checkpoint hooks. No test authority enters release.

## Certification evidence and findings

Step 8 local and exact-SHA evidence will be recorded only after execution.
The complete retained quality system remains mandatory: Phase 0–5 and Steps 1–7,
manual/native/security, Windows/Linux/Apple, Android both ABIs/apps/JVM/lint/
instrumentation, Swift macOS/Simulator, iOS app/UI, active ASan/UBSan negative
instrumentation probes, release scanning, workflow lint and source/diff checks.
Hosted Apple and simulator evidence is software only, never physical certification.

A: historical pending status lacked clear current closure authority; corrected by
historical labels, this consolidated evidence record and current source-consistency
checks. No production defect or architecture conflict found. A remains open only
until audit and final closure exact-SHA certification complete.

B: Phase 7 product UX, actual model integration, free-form interpretation,
production model provenance/rights, accelerator trust-promotion policy and required
physical provider certification. None invalidates the frozen foundation.
C: optional measurements, optimization and research. None implemented.

## Trust, limitations and future boundary

Production model UNRESOLVED. U2NETP AUDITED, UNSUPPORTED BY CURRENT PROFILE,
UNAPPROVED, UNINCORPORATED; CHECKPOINT RIGHTS UNRESOLVED. No weights.
cpu.reference.v1 VALIDATED; android.vulkan.v1 UNKNOWN; apple.metal.v1 UNKNOWN.
No provider CERTIFIED. Software PASS never promotes release trust.
DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED, mandatory at its frozen
later release gates. Historical Android Vulkan evidence remains scope-specific
supporting evidence, not Phase 5/6 inference physical certification.

Intent v1, Structured Intent v1, Plan v1, Registry v1, approval authority,
candidate lifecycle, Step 7 platform security, Schema 3, revision/history and
Phase 4/5 authority unchanged. No NLP/model, external runtime, weights,
network/cloud AI or AI UX. Ephemeral candidates/approvals do not survive restart;
crash guarantees remain existing repository transaction guarantees only.
Phase 7 may rely on the validated proposal/approval/execution foundation but is
NOT STARTED and not authorized here.

Final Phase 6 closure SHA and exact run IDs cannot be embedded in that same commit
without self-reference. After both complete final-SHA workflows succeed, an observed
engineering receipt under build/ and final delivery resolve that commit and run
IDs. Until then closure certification is pending. No earlier SHA substitutes.
Phase 6 CLOSED / COMPLETE / FULL PASS is effective only after that observation.
