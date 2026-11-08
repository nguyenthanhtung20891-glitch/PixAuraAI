# Phase 10.1 — Dual-platform integration and Apple hardware enablement

Phase 10.1 AUTHORIZED / IN PROGRESS / LOCAL HARDWARE GATE BLOCKED.
The preparing session was Windows at `G:\PixAuraAI`. No Mac Studio command
output was collected. Hosted Apple CI remains a separate evidence source.
Phase 11 is not started. Phase 12 is not started.

`docs/plans/phase-10.1-readiness.md` remains the pre-authorization planning
note. This roadmap is the current Phase 10.1 record.

| Step | Scope | Status |
| --- | --- | --- |
| 1 | Mac Studio development environment | LOCAL HARDWARE GATE BLOCKED |
| 2 | Local Apple native and iOS validation | LOCAL HARDWARE GATE BLOCKED |
| 3 | Parallel Android and iOS workflow | LOCAL ANDROID OBSERVED / PHASE NOT CLOSED |
| 4 | Mac Studio hardware performance | LOCAL HARDWARE GATE BLOCKED |
| 5 | Integrated certification and phase closure | NOT CLOSED |

## Two-machine ownership

Windows and Mac Studio use separate clones of
`github.com/nguyenthanhtung20891-glitch/PixAuraAI`. `main` is the only
canonical branch. One integration owner pushes a reviewed SHA. Shared C++
changes have one owner. Android presentation stays with the Windows clone.
Apple presentation, the Swift/C ABI, Xcode, and the Simulator stay with the
Mac clone. Platform layouts may differ. Approval, history, and export
authority stay in the shared native core.

## Supported toolchain pin

Hosted Apple jobs select
`/Applications/Xcode_26.6.app/Contents/Developer` on `macos-26`.
`cmake_minimum_required` is 3.22. The workflows do not pin a Node version.
Android native builds pin NDK `28.2.13676358` and the Android SDK CMake
`3.22.1`. Those Android pins are not a Mac C++ compiler pin.

`cpu.reference.v1` stays VALIDATED. `android.vulkan.v1` stays UNKNOWN.
`apple.metal.v1` stays UNKNOWN. DH-APPLE-METAL-01 stays DEFERRED / NOT WAIVED
/ NOT SATISFIED.

The Product Owner authorized the existing Android workflow on the Mac Studio
clone. The local observation is `docs/reports/phase-10.1-step-3.md`. That
observation does not close Phase 10.1 and does not replace hosted CI.
