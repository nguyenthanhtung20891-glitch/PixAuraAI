# Phase 10.1 Step 3 — Parallel Android and iOS workflow

Status: POLICY RECORDED / LOCAL PARITY BLOCKED.

The ownership rules are in `docs/plans/phase-10.1-roadmap.md`. Windows and
Mac Studio keep separate clones. One owner integrates to `main`. Shared C++
has one owner. Android presentation is Windows work. Apple presentation,
Swift/C ABI, Xcode, and Simulator work belong on the Mac clone.

The common scenario list is open/import, manual edit, Assist navigation,
Auto proposal, plan inspection, preview, edit review, explicit HUMAN Apply,
ordinary history, undo/redo, JPEG/PNG export, save/share, English and
Vietnamese presentation, cancellation/failure, and lifecycle or stale-state
rejection. Existing source gates already lock the shared sentences. A
side-by-side run on the Windows clone and the Mac clone has not happened.
Hosted CI success is not that local parity result.
