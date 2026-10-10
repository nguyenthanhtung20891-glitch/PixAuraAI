# Delivery roadmap

## Frozen Phase 3 numbered roadmap — Manual Tools

Authority: explicit Product Owner / Architect decision. Step 1 is accepted
**COMPLETE / FULL PASS** at `b29123037effd76d984780f59889f1fcbb3c8c76`;
see the [contract](docs/contracts/manual-tools-v1.md) and [report](docs/reports/phase-3-step-1.md).
Step 2 is **COMPLETE / FULL PASS** (technical gates); crop and orthogonal rotation only.
Step 3 is **COMPLETE / FULL PASS** (technical gates); see the [Step 3 report](docs/reports/phase-3-step-3.md). Steps 4–8 remain **NOT STARTED** and require separate authorization.
See the [Step 2 report](docs/reports/phase-3-step-2.md).

| Step | Official name | Status | Frozen scope and acceptance boundary |
| --- | --- | --- | --- |
| 1 | Manual Tool Contract & Registry Foundation | COMPLETE / FULL PASS | Accepted shared descriptors, bounded deterministic registry, versioned parameter validation/serialization/replay and gesture reference semantics. |
| 2 | Geometry Tools | COMPLETE / FULL PASS | Implement approved MVP crop and orthogonal rotation controls on shared non-destructive operations/history. Resize remains PROPOSED and requires separate approval. Do not add perspective, free-angle rotation or other geometry tools. |
| 3 | Tone & Color Tools | COMPLETE / FULL PASS | Implement exposure, brightness, contrast, highlights, shadows, saturation and temperature. Each requires explicit units/ranges, versioned semantics, deterministic validation and replay. Whites, blacks, tint and vibrance remain backlog unless separately approved. |
| 4 | Detail Tools | NOT STARTED | Implement sharpen and blur. Denoise, clarity, texture and other detail controls require separate approval. |
| 5 | Filters & Presets Foundation | NOT STARTED | Define and implement deterministic, inspectable, non-destructive filter/preset representation. Names and recipes must be separately frozen before shipping. No destructive hidden processing. |
| 6 | Gesture Editing Lifecycle Integration | NOT STARTED | Integrate BEGIN -> UPDATE/PREVIEW -> COMMIT once, or CANCEL without history mutation, into real editing flows. Preserve stale rejection, coalescing, interruption, tool switching and failure semantics from Step 1. |
| 7 | Cross-platform Manual Editor Integration | NOT STARTED | Integrate Android Compose and iOS SwiftUI against the same shared-core tool IDs, ranges, validation, serialization, replay and history semantics. Do not duplicate core business rules per platform. |
| 8 | Phase 3 Certification & Closure | NOT STARTED | Validate offline tool matrix, documented units/ranges, deterministic replay, gesture history, restart/reload consistency, invalid/boundary cases, Android/iOS parity and regression closure. CLOSE Phase 3 here only if all exit criteria pass; do not create Step 9 automatically. |

Approved future tool scope does not freeze missing parameter tuples or numerical
recipes, register tools, or establish implementation PASS. Step 1's frozen
parameters remain unchanged. [Scope discipline](QUALITY_GATES.md#phase-3-scope-discipline)
governs all new proposals. No additional Phase 3 steps, shipping tools, Phase 3.5,
AI behavior or Phase 4 work may be introduced without explicit Product Owner approval.
Historical authorization records below predate this decision and do not reopen work.

**Phase 2 COMPLETE / CLOSED.** Steps 1-11 are accepted FULL PASS, the original first GPU pipeline is complete, hosted compile/regression is green, and physical Z Fold7 / Adreno 830 Vulkan certification is accepted PASS 14/14. The explicit [Product Owner/Architect decision](DECISIONS.md#phase-2-closure-decision-2026-10-08) defers Apple physical Metal certification because no real Apple hardware is available. It is not waived or removed: [DH-APPLE-METAL-01](QUALITY_GATES.md#dh-apple-metal-01-deferred-physical-apple-metal-certification) remains mandatory at Phase 11 hardening acceptance and before Phase 12 Beta readiness completion, production release, or any physical Apple GPU certification claim. See the [exit-closure report](docs/reports/phase-2-exit-closure.md). No new Phase 2 step is added; no further Phase 2 work is authorized. Phase 3 has not started and requires separate authorization. The milestone entries below are historical and do not reopen work.

Phase 2 Step 11 is authorized: [interactive preview request generation and publication ownership](docs/contracts/interactive-preview-lifecycle-v1.md) under [ADR 0017](docs/adr/0017-interactive-preview-generation-fence.md). Latest-wins native/platform fences, cooperative supersession and failed-request display preservation keep synchronous execution, existing ceilings and quiescent destruction. All existing local/hosted gates plus churn/races/overflow/platform ownership remain mandatory. [Step 11 report](docs/reports/phase-2-step-11.md); Step 12 is not started.

Phase 2 Step 10 is authorized: bounded-fit reference previews and platform consumption under [ADR 0016](docs/adr/0016-bounded-bilinear-platform-preview.md). [Step 10 report](docs/reports/phase-2-step-10.md) tracks validation; Step 11 is not started.

Current authorization: Phase 2 Step 9 bounded 1:1 CPU reference preview, including autonomous validation and commit/push/CI closure. Accepted Step 8 HEAD: 86520909fa31372c7926676650873cd105352695, Product Owner FULL PASS. Option A aggregate resource rejection/retry approved; all ceilings unchanged. Step 10 is not started. [Step 9 report](docs/reports/phase-2-step-9.md).

Current authorization: Phase 2 Step 8 minimal geometry pixel execution, including autonomous local validation and commit/push/CI closure. Accepted Step 7 HEAD: 2e0f975ca894372684c3febbc391dc3ac92d5407, user-attested FULL PASS. Frozen outward crop and clockwise turns execute with two rasters under existing ceilings. Step 9 is not started. [Step 8 report](docs/reports/phase-2-step-8.md).

Current authorization: Phase 2 Step 7 bounded geometry, tiling and cancellation contracts, including autonomous implementation, validation, commit/push and CI closure. Step 6 is Product Owner accepted FULL PASS at fd545ac545d526e779ca82daf8ac58e20cf39f2f. Outward crop rasterization is explicitly approved. No Step 8 is started. Evidence: [Step 7 report](docs/reports/phase-2-step-7.md). Historical limits below are superseded.

Current authorization: Phase 2 Step 6 bounded CPU operation evaluation and numerical contracts, including autonomous commit/push and CI remediation until green. Step 5 is Product Owner accepted FULL PASS at 7f1fa96ab0f24b44e4e28a8c6c0aff8e7a4024ce. Step 6 implements the frozen exposure/1/1 tuple and ordered bounded evaluation; it does not start Step 7. Evidence: [Step 6 report](docs/reports/phase-2-step-6.md). Historical authorizations below are superseded.

Current authorization: Phase 2 Step 5 orientation/color normalization and initial bounded CPU identity evaluation, including focused commits/push and one CI remediation cycle maximum. Accepted Step 4 HEAD is d26baf21be2dde5f0c7881e8007658c859572cbe. Step 6 is not started. Evidence: [Step 5 report](docs/reports/phase-2-step-5.md). Historical authorization follows. Step 3 accepted HEAD is `94159bcc071ba1ae6832bd0cd0012017d54d09ad`. Shared JPEG/PNG ownership is explicitly approved under [ADR 0011](docs/adr/0011-shared-bounded-sdr-decode.md). The Step 5 instruction above supersedes this historical Step 4 boundary. Current evidence is in the [Step 4 report](docs/reports/phase-2-step-4.md).

Current authorized boundary: Phase 2 Step 3, durable SQLite persistence and immutable verified asset storage under the frozen Step 1/2 contracts. Do not begin Step 4 automatically. The user attests both Step 2 workflows green on approved revision `f78ba096872249bf80ff1e4ee9e51a0a4ecfb6ef`; this is user-attested completion, not a locally observed Apple/MSVC run. Phases are dependencies, not permission to skip validation. Each milestone performs the workflow in AGENTS.md and publishes evidence under docs/reports. No commit/push is authorized.

Current status: the user reports Phase 0 and Phase 1 fully passed GitHub Actions, including Android emulator JNI/Compose and iOS build/test, and explicitly authorizes Phase 2 Step 1. This final completion is user-attested; no final passing run URL was supplied at this authorization. Historical [Phase 0](docs/reports/phase-0.md) and [Phase 1](docs/reports/phase-1.md) reports retain their earlier observations. Step 1 was subsequently reviewed and approved, and Step 2 plus ADR 0009 were explicitly authorized. Historical contract evidence is in the [Step 1 report](docs/reports/phase-2-step-1.md); native foundation delivery/evidence is in the [Step 2 report](docs/reports/phase-2-step-2.md).

| Phase | Deliverable | Measurable exit |
| --- | --- | --- |
| 0 Engineering foundation | Specifications, ADRs, checked native ABI and platform boundary probes | G0-G4 applicable checks pass on host/platform runners; no feature implementation |
| 1 Native shells | Android Compose/iOS SwiftUI, shared bridge, navigation, services, storage policy, build locks | Debug and unsigned release builds both platforms; boundary smoke screen; no permissions before use; UI state tests |
| 2 Non-destructive engine — COMPLETE / CLOSED | Source import, versioned graph, SQLite revisions, CPU reference/color/geometry and first GPU pipeline | Steps 1-11 FULL PASS; source hash invariance, recovery, CPU/parity and bounded memory validated; physical Android Vulkan PASS 14/14; Apple physical gate deferred by explicit product decision to DH-APPLE-METAL-01 |
| 3 Manual tools | All MVP geometry/tone/detail/filter controls | Offline tool matrix; documented units/ranges; deterministic replay and gesture history |
| 4 Capability profiling | Probe service, budget admission, dynamic downgrade | Recorded low/mid/high devices; T0 manual flows; pressure/thermal fallback evidence |
| 5 Local inference | Verified model packages, provider adapters and compact segmentation evaluation | CPU fallback offline; signed-manifest rejection; provider parity/license/size/task thresholds |
| 6 Editing Agent | Advisor/planner/operator boundaries, typed plans, audit and approval | Invalid/stale/over-budget plans rejected; no mutation before approval; interleaved manual/AI history |
| 7 AI-assisted MVP | Auto Enhance, Smart Lighting, subject tools, background removal/portrait blur | Evaluation fixtures and editable masks; capability unavailability UX; refine/cancel/reject tests |
| 8 Full review experience | Compare, detailed history, cross-mode undo/redo | Restart/branching/AI batch behavior; accessible compare; candidate distinction |
| 9 Export/media integration | Save copy, JPEG/PNG quality/metadata, destination lifecycle | No source overwrite; interrupted export cleanup; color/dimension parity both OSs |
| 10 Licensing | Trial/subscription/lifetime/store restoration | Sandbox lifecycle and clock/offline tests; exact policy/pricing verified; no AI credits |
| 11 Hardening | Device regression, privacy/security/performance/accessibility; tracked deferred Apple physical Metal certification | G7-G11 complete; fuzz corpus, backup/network audit, no critical defects; DH-APPLE-METAL-01 physical Apple PASS required for hardening acceptance |
| 12 Beta | Release packaging, support/recovery notes, beta cohort | G12 complete with signed platform builds and human store approval; DH-APPLE-METAL-01 must PASS before Beta readiness completion or production release |

History, compare, basic save-copy and capability admission cannot wait until late phases: their minimal contracts land with engine/manual work, and phases 8/9 complete their user experience. Likewise resource bounds exist before AI. Phase 4 refines existing safe baseline. Monetization never dictates engine design.

## Historical milestone definitions

Phase 2 is CLOSED. The entries below preserve historical scope and evidence; they authorize no further Phase 2 work or Phase 3 implementation.
Phase 2 Step 1 acceptance: audited existing boundaries; approved [document/operation/history/persistence/ABI contracts](docs/contracts/image-document-v1.md); ADR 0008; deterministic metadata fixtures and failure-path tests; preservation and rerun of applicable Phase 0/1 gates. No shipping parser, pixel algorithms, GPU, AI or editing UI. Stop and report for human review.

Step 2 is approved and user-attested fully CI passed. Authorized Step 3 implements the persistence/managed-asset foundation; current evidence and pending gates are in the [Step 3 report](docs/reports/phase-2-step-3.md). Proposed Step 4 is a separately reviewed bounded decode/metadata-verification contract and implementation milestone, with exact scope frozen before code. Image decoding/rendering require later authorization. The overall Phase 2 exit still requires reference pixels and renderer parity; Step 3 does not meet that full-phase exit.
