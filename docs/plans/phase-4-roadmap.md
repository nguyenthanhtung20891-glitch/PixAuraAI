# Phase 4 proposed numbered roadmap — Capability Profiling

Status: **PROPOSED — NOT FROZEN**. Product Owner review is required before
proceeding beyond Step 1. Authority: Phase 4 authorization at baseline
`56cd60560b6271b7d27bf5e03fd1d720a3aad2fa`; Phase 0–3 remain closed.
This proposal is finite: seven steps, no Step 8+ without explicit approval.
Only the minimum Step 1 foundation is authorized now. Steps 2+ are NOT STARTED.

| Step | Proposed name | Status | Bounded deliverable and exit |
| --- | --- | --- | --- |
| 1 | Capability Contract & Probe Foundation | IMPLEMENTED / TECHNICAL PASS / REVIEW PENDING | Audit; versioned static/dynamic normalized data; bounded diagnostic representation; synthetic LOW/MID/HIGH inputs; conservative C/JNI/Swift probe seams; malformed/unknown/availability-vs-validation tests; preserved full regression CI. Stop for roadmap freeze and Step 1 acceptance. |
| 2 | Budget Admission & Execution Tiers | PROPOSED / NOT STARTED | Shared deterministic BudgetAdmission over facts, request buffers/dimensions and context occupancy; freeze multi-input LOW/MID/HIGH thresholds and T0 rules; runtime caps never exceed existing hard maxima; threshold/overflow/unknown/insufficient-budget tests. No inference. |
| 3 | Dynamic Memory/Thermal Downgrade | PROPOSED / NOT STARTED | Shared derived execution strategy; immediate downgrade, bounded recovery hysteresis, policy epochs and existing PRV1 ownership; pressure/GPU-loss/constrained transitions; deterministic races and active-gesture preservation. No semantic or source-fidelity downgrade. |
| 4 | Android Capability Integration | PROPOSED / NOT STARTED | Internal safe Android probes and lifecycle/pressure/thermal adapters; central policy consumption; actual hosted runtime and available physical probe evidence; availability separate from scope-matched accepted Vulkan certification; no chipset-name tiers. |
| 5 | Apple Capability Integration | PROPOSED / NOT STARTED | Safe Swift/iOS probe/signal adapters; macOS/simulator/app test seams; CPU fallback and uncertified Metal availability; explicitly no physical Apple evidence or DH-APPLE-METAL-01 waiver. |
| 6 | Cross-tier Replay & Certification | PROPOSED / NOT STARTED | Recorded synthetic LOW/MID/HIGH policy profiles plus separately labeled meaningful emulator/physical observations; HIGH-created project reopened under MID/LOW; same IDs/stacks/logical result; T0 complete manual flows; pressure/thermal/resource/security/race corpus and full platform parity. |
| 7 | Phase 4 Closure | PROPOSED / NOT STARTED | Certify frozen exit criteria, all same-final-SHA workflows, bounded local/private profiles, complete T0 and downgrade/recovery evidence; A fixes only; classify B/C; close only on observed PASS. Stop; Phase 5 requires separate authorization. |

## Audit and fit

The accepted core already owns raster/context/handle/operation/scratch ceilings,
resource rejection, manual history, immutable storage and PRV1 generation fences.
The Compose and SwiftUI owners already serialize lifecycle/publication and bound
command/render lanes. GPU diagnostics are bounded standalone tile primitives,
not a full manual-preview backend. This proposal composes those owners; it does
not introduce a second scheduler, publication owner or history store.

[Device capability strategy](../../DEVICE_CAPABILITY_STRATEGY.md) contains
720/1280/2048-preview and 128/256/512-MiB profiling **targets**, not current
allocations or authority to increase frozen limits. Existing whole-raster decode
and CPU execution remain bounded; this phase does not promise a new tiled decoder
or GPU tool stack. Step 2 must choose runtime budgets below accepted maxima and
retain explicit rejection/retry when a source cannot fit. T0 preserves supported
manual flows under existing limits; it cannot promise execution without sufficient
OS memory. Unknown facts cannot create extra admission authority.

Accepted Z Fold7 Vulkan evidence remains valid only for its tested backend/runtime
scope. No Phase 4 physical profile or tier classification has been measured yet.
No Apple hardware is available. Synthetic data is never reported as a device.
DH-APPLE-METAL-01 remains deferred and mandatory at its existing later gates.

## Review boundary and follow-ups

A: Step 1 foundation and technical gates PASS; final documentation-only delivery
SHA must pass both full workflows before delivery. No unresolved technical defect.
The roadmap must be explicitly frozen and Step 1 reviewed/accepted before Steps 2+.
B: Steps 2–7 after separate authorization; physical Apple Metal certification at
Phase 11 hardening acceptance / Phase 12 Beta readiness / production / physical
GPU claims; Phase 5 inference is outside Phase 4.
C: optional device dashboards, performance scores, broad diagnostics and polish.
No new tools/presets, manual numerical semantics, AI/model/runtime, network/cloud,
telemetry, GPU kernel expansion or storage-schema changes are proposed here.
