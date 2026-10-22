# Phase 7 Step 1 — AI-Assisted MVP contract and interaction state machine

## Phase 7 Step 1 - current authority

Phase 6 CLOSED / COMPLETE / FULL PASS. Phase 7 AUTHORIZED / IN PROGRESS.
Step 1 COMPLETE / FULL PASS.
Implementation certification SHA: `648a66de49e1e5bf428697ced28812a0a597bff8`.
Foundation 39766049277 SUCCESS 7/7; Native application shells 39766049069 SUCCESS
3/3, both completed on that exact SHA, attempt 1, every required job SUCCESS.
This Step 1 closure-only documentation commit requires independent complete
closure-SHA Foundation and Native shells certification again. Until that
observation: FINAL STEP 1 CLOSURE CERTIFICATION PENDING. The observed engineering
receipt/final delivery resolve its actual final SHA/run IDs without self-reference;
no old-SHA substitution and no unobserved job is PASS.
Phase 7 Steps 2-8 NOT STARTED. Phase 8 NOT STARTED.

## Purpose and certified baseline

Define the normative three-mode product/authority boundary before provider, UX or
model work. Clean main/HEAD/origin/main were verified before/after fetch at
`523ffcad47bddd4c13a2615a76f211a6bd38efe2`. Foundation 39724919418 completed/SUCCESS
7/7 and Native shells 39724919325 completed/SUCCESS 3/3, attempt 1, independently
accepted by Principal Architect/Product Owner. No old SHA certifies this step.

Audit covered PRODUCT_SPEC, root architecture/decisions/roadmap/quality authority,
Phase 6 closure, Intent/Structured Intent/Plan/Registry/reference planner,
approval/execution, candidate and platform-security contracts, ADR 0009 and
existing Kotlin/Swift presentation mode enums. Legacy product sketches and shell
labels are not new execution authority; frozen pre-execution approval remains.
No frozen contract, production runtime or C ABI requires change.

## Product and interaction model

[AI-Assisted MVP v1](../contracts/ai-assisted-mvp-v1.md) freezes MODE_MANUAL,
MODE_ASSIST and MODE_AUTO. Manual operates directly without AI. Assist is
advisory-first, inspectable/dismissible and non-mutating; partial manual follow-up
is an ordinary human manual operation. Auto automates proposal/planning, never
approval. All modes use the same document/source/revision/history/undo/redo.

The finite conceptual state machine separates interaction, document state and
native execution authority. IDLE → REQUESTING → PROPOSAL_AVAILABLE → REVIEWING
does not edit. Advice cannot become APPROVAL_REQUIRED without an explicit
conversion and native-validated exact Plan v1. Human Apply and native admission
are required for EXECUTING. Only native publication receipt means COMPLETED;
REJECTED/CANCELLED/STALE/FAILED never resurrect old authority. The normative
transition/guard table includes BUSY without queue/retry and all terminal outcomes.
No runtime reducer or fake runtime tests are implemented.

Mode switching abandons ephemeral responses/review, revokes unused approval and
requests existing cancellation for started work. Native publication/cancel has one
winner; a mode flag cannot hide a committed revision or promise cancellation.
Manual/source/undo/redo invalidate stale AI work using existing source serial/
generation and epoch CAS, including ABA. Owner teardown remains stop -> join ALL
calls -> destroy. Provider failure cannot disable manual editing or invent a
native execution outcome; manual work after AI publication stays first-class.

## Authority, provider, proposal and persistence boundaries

Request → replaceable Assistance Provider → UNTRUSTED Assistance Proposal →
existing trusted intent/plan validation and Registry binding → review → trusted
explicit HUMAN approval → private Phase 6 execution → ordinary revision/history.
No provider/UI/platform owns editing authority or receives approval/publication
capabilities. Future provider replacement cannot redesign document/history/core.

Future proposals must be ephemeral, bounded, provider-identified through trusted
owner, source/document/revision/session/context/request bound, inspectable,
rejectable and invalidatable. Step 2 owns exact envelope schema/encoding/limits;
none is prematurely implemented here. Existing eight-action/8192-byte Plan and
other ceilings remain. Free-form intent is not interpreted. No model selected.

Schema 3 unchanged. No requests/proposals, pending approval/candidate authority,
provider handles or hidden reasoning are persisted. Only ordinary committed
editing history is durable. Local-first: no network requirement, cloud AI,
analytics dependency for authority or remote approval. No permission/endpoint,
upload, prompt/chat UI, inference runtime or weights added. Kotlin/Swift remain
presentation/ownership adapters, with shared native validation/execution semantics.

## Security and graceful degradation

Threat matrix in the contract covers A authority bypass; B malformed/forged
output; C stale/ABA work; D approval confusion/replay; E mode-switch leakage;
F platform mismatch; G owner lifetime; H cancellation/publication race;
I resource exhaustion; J release/test-hook leakage; K privacy/data exposure.
No unresolved category A architecture issue. Invalid/rejected/stale proposals
leave history unchanged; pre-publication failure has no partial revision.
Cancel-after-publication cannot relabel success. No auto-retry/replan/reapproval.
Provider unavailable/failure means assistance unavailable, never editing unavailable.

## Roadmap, boundaries and findings

The [frozen eight-step roadmap](../plans/phase-7-roadmap.md) preserves exactly the
authorized numbering. Only Step 1 is started. Step 7 later evaluates intelligence
quality, repeatability, latency, memory/package/battery/thermal/hardware, trust,
license/checkpoint rights/provenance, offline/privacy and failure/fallback.
Phase 8 FULL REVIEW EXPERIENCE NOT STARTED: no advanced compare, inspection,
multi-candidate timelines or production export. No licensing/subscription work.

A: normative modes/state transitions, human/native authority separation and source
governance consistency required here; addressed without runtime/architecture change.
B: Steps 2-8 provider/envelope/UX/lifecycle/model admission and later physical gates.
C: optional product/model research. No B/C implementation; no architecture blocker.
Production model UNRESOLVED; U2NETP unapproved/unincorporated/profile unsupported,
checkpoint rights unresolved. CPU VALIDATED; Vulkan/Metal UNKNOWN.
DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED. Hosted evidence is not physical.

## Changed files and certification discipline

New: docs/contracts/ai-assisted-mvp-v1.md; docs/plans/phase-7-roadmap.md;
docs/reports/phase-7-step-1.md. Updated: AGENTS.md, ARCHITECTURE.md, DECISIONS.md,
QUALITY_GATES.md, ROADMAP.md, tests/agent-execution.test.mjs. No production code,
Schema 3, Phase 6 contract, provider trust or workflow behavior change.

Governance checks validate documentation/authority/state-table consistency, not
invented runtime behavior. Complete retained Phase 0-6/manual/native/platform/
security/sanitizer/release-isolation/source/workflow gates remain mandatory.
Both complete implementation-SHA workflows above are observed SUCCESS; no pending
job counted as PASS. The separate closure-SHA must independently repeat both full
workflows before final delivery claims COMPLETE / FULL PASS. No old-SHA substitution.
Pre-push Windows-applicable retained source checks 122/122 PASS, zero skips;
workflow lint and git diff --check PASS. These are governance/source observations,
not execution of an unimplemented Phase 7 runtime. Hosted/native/app/sanitizer
evidence requires the full new exact-SHA workflows, with no retained gate reduced.
Exact implementation/closure SHA, run IDs/attempts/job results are recorded after
observation in the engineering receipt/final delivery, avoiding self-reference.
STOP after Step 1. Do not implement Step 2. Phase 8 NOT STARTED.

## Observed implementation certification

Implementation `648a66de49e1e5bf428697ced28812a0a597bff8`:
[Foundation 39766049277](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/39766049277)
completed/SUCCESS 7/7 and
[Native application shells 39766049069](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/39766049069)
completed/SUCCESS 3/3, each exact SHA, attempt 1, all required jobs individually
completed/SUCCESS. Windows native 46/46; Linux and Apple native 47/47; retained
source 122/122 Windows and 164/164 Linux, zero skips. Active ASan/UBSan 49/49,
negative probes 86/87, Agent corpus 1548 assertions. Android both ARM ABIs,
debug/release/JVM/lint and instrumentation 35/35; Swift macOS/Simulator 35/35 each;
iOS app 7/7 and UI 2/2 PASS. Release/test isolation, workflow lint, manual/history/
undo/redo and all retained Phase 0-6 gates PASS. Hosted/simulator evidence remains
software only. No tests weakened/skipped/optionalized and no rerun-until-green.

No unresolved A finding or architecture blocker. This closes the normative
contract/governance step, not a runtime assistance feature. Contract body and
governance tests are unchanged in the separate closure commit; only the five root
authority documents, Phase 7 roadmap and this report receive status/evidence
updates. Phase 7 remains AUTHORIZED / IN PROGRESS with Steps 2-8 NOT STARTED.
Phase 8 NOT STARTED. Stop after Step 1.
