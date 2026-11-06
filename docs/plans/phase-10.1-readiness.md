# Phase 10.1 — Dual-platform integration and Apple hardware enablement

Status: NOT STARTED. This note is planning only. It does not authorize
implementation, and it does not change Metal trust.

Phase 10.1 sits after the observed Phase 10 closure
`e4835cfcc008fb1e6db276cdcbeca47527e39cd4` and before Phase 11. Phase 11 and
Phase 12 remain not started.

1. Mac Studio Xcode, toolchain, and Simulator environment. Record the exact
   Xcode and Simulator versions. Hosted Apple CI remains a separate evidence
   source.
2. Local Apple native C++ and Swift bridge validation on that machine. The
   shared native editor stays the authority.
3. Android and iOS end-to-end parity using separate Windows and Mac clones.
   Platform visuals may differ. Approval, history, and export authority stay
   shared and native.
4. Mac Studio hardware performance and memory qualification. macOS measurements
   are not physical iPhone measurements. They do not satisfy DH-APPLE-METAL-01
   and do not change `apple.metal.v1` from UNKNOWN.
5. Integrated cross-platform CI, evidence, and closure. A future Phase 10.1
   implementation SHA and a separate closure SHA would each need their own
   exact-SHA Foundation and Native shells workflows. This note is not either
   SHA.

`cpu.reference.v1` stays VALIDATED. `android.vulkan.v1` stays UNKNOWN.
`apple.metal.v1` stays UNKNOWN. DH-APPLE-METAL-01 stays DEFERRED / NOT WAIVED
/ NOT SATISFIED.
