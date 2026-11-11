# Phase 10.1 Mac Studio governance successor

Authority: Product Owner's master autonomous engineering mission, 2026-11-11.
This successor replaces the development-machine allocation in the original
Phase 10.1 roadmap. It does not rewrite historical reports, assert that the old
two-machine run occurred, or change product/runtime contracts.

Mac Studio is the primary development environment for BOTH platforms. Android
Kotlin/Compose, NDK/JNI, shared C++, Swift/C ABI, SwiftUI, Xcode and Simulator
maintenance may proceed here. Windows remains a supported CI host. One integration
owner reviews and pushes to canonical `main`; no shared native ownership split.

The old requirement to run scenarios side by side on Windows and Mac clones is
replaced by auditable same-source dual-platform validation: source digests and
configuration provenance; local Android ARM64 instrumentation and iOS Simulator
unit/UI results; a scenario-to-test parity matrix; retained Windows portable CI;
and both full hosted workflows on every integration and eventual closure SHA.
Visual layouts may differ. Approval, source/epoch binding, single-use consumption,
Cancel, ordinary history, export and resource admission remain native authority.
This provides direct platform runtime validation without assigning domain semantics
to a development machine. The historical two-machine experiment did not occur.

Frozen roadmap/status assertions in tests/phase-10.1-hardware-gate.test.mjs and
phase-10.1-v1.json remain historical evidence. Active progress is recorded in the
successor report and checkpoint; future acceptance must consult them. The original
Step 1/4 reports' missing observations are not statements about today's host.
Xcode 27.2 is authorized for local development by this mission; the hosted Xcode
26.6 selection remains unchanged. No automatic toolchain installation or selector
change is required. Legacy Windows workspace limits apply historically; current
engineering writes are restricted to this authorized PixAuraAI checkout/build.

Physical scope remains separate: Mac CPU performance cannot certify iPhone
thermals/battery, physical Metal, Android Vulkan, or ARMv7 runtime behavior.
DH-APPLE-METAL-01 retains every technical prerequisite and mandatory Phase 11,
Beta, production and physical-GPU-claim boundary in QUALITY_GATES.md. Phase 10.1's
roadmap explicitly allows Metal UNKNOWN after Mac performance measurements.
The phase can only close after all five local/integrated gates, both final
implementation and distinct documentation closure workflows, and independent
certification review. No engineering report self-activates acceptance.
