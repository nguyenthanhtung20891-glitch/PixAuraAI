# Delivery roadmap

Current authorization: Phase 2 Step 5 orientation/color normalization and initial bounded CPU identity evaluation, including focused commits/push and one CI remediation cycle maximum. Accepted Step 4 HEAD is d26baf21be2dde5f0c7881e8007658c859572cbe. Step 6 is not started. Evidence: [Step 5 report](docs/reports/phase-2-step-5.md). Historical authorization follows. Step 3 accepted HEAD is `94159bcc071ba1ae6832bd0cd0012017d54d09ad`. Shared JPEG/PNG ownership is explicitly approved under [ADR 0011](docs/adr/0011-shared-bounded-sdr-decode.md). The Step 5 instruction above supersedes this historical Step 4 boundary. Current evidence is in the [Step 4 report](docs/reports/phase-2-step-4.md).

Current authorized boundary: Phase 2 Step 3, durable SQLite persistence and immutable verified asset storage under the frozen Step 1/2 contracts. Do not begin Step 4 automatically. The user attests both Step 2 workflows green on approved revision `f78ba096872249bf80ff1e4ee9e51a0a4ecfb6ef`; this is user-attested completion, not a locally observed Apple/MSVC run. Phases are dependencies, not permission to skip validation. Each milestone performs the workflow in AGENTS.md and publishes evidence under docs/reports. No commit/push is authorized.

Current status: the user reports Phase 0 and Phase 1 fully passed GitHub Actions, including Android emulator JNI/Compose and iOS build/test, and explicitly authorizes Phase 2 Step 1. This final completion is user-attested; no final passing run URL was supplied at this authorization. Historical [Phase 0](docs/reports/phase-0.md) and [Phase 1](docs/reports/phase-1.md) reports retain their earlier observations. Step 1 was subsequently reviewed and approved, and Step 2 plus ADR 0009 were explicitly authorized. Historical contract evidence is in the [Step 1 report](docs/reports/phase-2-step-1.md); native foundation delivery/evidence is in the [Step 2 report](docs/reports/phase-2-step-2.md).

| Phase | Deliverable | Measurable exit |
| --- | --- | --- |
| 0 Engineering foundation | Specifications, ADRs, checked native ABI and platform boundary probes | G0-G4 applicable checks pass on host/platform runners; no feature implementation |
| 1 Native shells | Android Compose/iOS SwiftUI, shared bridge, navigation, services, storage policy, build locks | Debug and unsigned release builds both platforms; boundary smoke screen; no permissions before use; UI state tests |
| 2 Non-destructive engine | Source import, versioned graph, SQLite revisions, CPU reference/color/geometry and first GPU pipeline | Source hash invariance; crash recovery; golden pixel/tile parity; JPEG/PNG fixtures; bounded memory |
| 3 Manual tools | All MVP geometry/tone/detail/filter controls | Offline tool matrix; documented units/ranges; deterministic replay and gesture history |
| 4 Capability profiling | Probe service, budget admission, dynamic downgrade | Recorded low/mid/high devices; T0 manual flows; pressure/thermal fallback evidence |
| 5 Local inference | Verified model packages, provider adapters and compact segmentation evaluation | CPU fallback offline; signed-manifest rejection; provider parity/license/size/task thresholds |
| 6 Editing Agent | Advisor/planner/operator boundaries, typed plans, audit and approval | Invalid/stale/over-budget plans rejected; no mutation before approval; interleaved manual/AI history |
| 7 AI-assisted MVP | Auto Enhance, Smart Lighting, subject tools, background removal/portrait blur | Evaluation fixtures and editable masks; capability unavailability UX; refine/cancel/reject tests |
| 8 Full review experience | Compare, detailed history, cross-mode undo/redo | Restart/branching/AI batch behavior; accessible compare; candidate distinction |
| 9 Export/media integration | Save copy, JPEG/PNG quality/metadata, destination lifecycle | No source overwrite; interrupted export cleanup; color/dimension parity both OSs |
| 10 Licensing | Trial/subscription/lifetime/store restoration | Sandbox lifecycle and clock/offline tests; exact policy/pricing verified; no AI credits |
| 11 Hardening | Device regression, privacy/security/performance/accessibility | G7-G11 complete; fuzz corpus, backup/network audit, no critical defects |
| 12 Beta | Release packaging, support/recovery notes, beta cohort | G12 complete with signed platform builds and human store approval |

History, compare, basic save-copy and capability admission cannot wait until late phases: their minimal contracts land with engine/manual work, and phases 8/9 complete their user experience. Likewise resource bounds exist before AI. Phase 4 refines existing safe baseline. Monetization never dictates engine design.

## Next milestone definition
Phase 2 Step 1 acceptance: audited existing boundaries; approved [document/operation/history/persistence/ABI contracts](docs/contracts/image-document-v1.md); ADR 0008; deterministic metadata fixtures and failure-path tests; preservation and rerun of applicable Phase 0/1 gates. No shipping parser, pixel algorithms, GPU, AI or editing UI. Stop and report for human review.

Step 2 is approved and user-attested fully CI passed. Authorized Step 3 implements the persistence/managed-asset foundation; current evidence and pending gates are in the [Step 3 report](docs/reports/phase-2-step-3.md). Proposed Step 4 is a separately reviewed bounded decode/metadata-verification contract and implementation milestone, with exact scope frozen before code. Image decoding/rendering require later authorization. The overall Phase 2 exit still requires reference pixels and renderer parity; Step 3 does not meet that full-phase exit.
