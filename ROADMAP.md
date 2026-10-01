# Delivery roadmap

First-run boundary: Phase 0 only. Phases are dependencies, not permission to skip validation. Each milestone performs the workflow in AGENTS.md and publishes evidence under docs/reports.

Current status: Phase 0 CONDITIONAL PASS / READY FOR CI. Local foundation, Windows/Linux host ABI, Android native, workflow syntax and actual WSL Clang ASan/UBSan checks pass, including negative instrumentation probes. macOS core/Swift and iOS package/simulator execution are pending the Apple CI job; they are unavailable locally on Windows. Phase 1 is explicitly prohibited. See [Phase 0 report](docs/reports/phase-0.md) for the final gate matrix and required human action.

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
Phase 1 starts only after Phase 0 platform boundary checks are observed passing. Acceptance: pinned Android Gradle/JDK/SDK/NDK and Xcode/Swift inputs; native shells render ABI version through actual bridge; all three mode entry points preserve a placeholder session; immutable reducer tests; app-private storage/backup policies; lint/type/unit/build and simulator/emulator smoke checks on both platforms. No editing implementation in Phase 1.
