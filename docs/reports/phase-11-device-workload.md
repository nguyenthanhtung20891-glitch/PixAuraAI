# Phase 11 physical workload instrumentation successor

Phase 11 remains IN PROGRESS. No acceptance, closure, or Phase 12 work.
Predecessor: 82f847859397e5217f4a68b511b526524a1ab7d6.

## Certification and diagnostic findings

Runtime loader correction 23ea6ff556ef7c7fbf94c59aa97218128da47391 passed
Foundation 43406518767 (7/7) and Native 43406518751 (3/3), exact SHA, attempt 1.
The predecessor passed Foundation 43407405462 (7/7), but Native 43407405451
was not certified: source/Android succeeded; iOS exceeded its 45 minute budget.
Before cancellation, the reordering UI test failed waiting for the initial
proposal, before any reorder. The incomplete cancelled xcresult cannot provide
normal exported failure attachments. A focused local Simulator run passes all
unchanged assertions (reorder-v8-private-externs.xcresult). Root cause of the
hosted occurrence is not established; no speculative production fix is claimed.
First-failure UI tree/screenshot retention and fail-fast test behavior now keep
subsequent taps from obscuring the initiating failure. All assertions remain.
The iOS job budget is 60 minutes; test waits, selections and required jobs remain.
No identical failed SHA is rerun.

## Measurement method

Test-only instrumentation uses the actual Apple capability snapshot and native
open policy, CPU reference backend, native revision authority and existing limits.
Synthetic 1024x768, 3000x2000 and 4000x3000 PNG sources exercise import, a fresh
720px preview, explicit manual Apply, JPEG/PNG export and cancellation. Timing
uses monotonic clocks; Mach RSS/high-water, thermal and power state are recorded.
No image upload, forced capability tier, new RSS ceiling, ABI or schema change.
A resource rejection is recorded as NOT a performance pass. Small manual editing
recovery must actually succeed. The 12MP case cannot bypass the frozen 8M-pixel
working limit. This exposes a G8 target/admission conflict, not permission to
relax either contract.

Normal CI executes all three fixtures once without percentile claims. Explicit
PIXAURA_BENCHMARK_SAMPLES=100 performs five warmups and 100 retained samples for
admitted workloads. Complete JSON is retained as an XCTest attachment. Individual
operation measurements are distinct from UI frame/import response targets.
Release test builds use ENABLE_TESTABILITY=YES and are not production release
qualification. Device identity, build configuration and sample counts must be
verified before calling results physical or calculating percentiles.

## Local validation before successor commit

Release Simulator smoke and full sampling mode passed; full mode retains 100
admitted small-fixture samples after five warmups. Larger fixtures reject under
the actual policy and manual recovery passes. These are Simulator results only.
All 16 Debug Simulator unit tests pass. Unsigned physical SDK Release test build
passes. Mac-supported source certification passes 205/205. The broader Linux
emulator harness fails unchanged GNU sed/realpath assumptions on macOS; its full
Linux CI coverage remains enabled. No portability change is needed for this
physical iOS scope. The first local UI command omitted canonical symbol-preservation settings
and failed to link; the corrected canonical command passes. Raw logs remain in
ignored build/phase-11-iphone-xr/signing-ready/.

## Evidence scope and remaining gates

Predecessor physical evidence: Metal dispatch/readback 14/14, full units 15/15,
offline installed-app units 15/15, and 210 repeated synthetic journey tests pass.
Raw paths and methodologies are in the local review package/checkpoint. Debug
idle RSS/high-water was 187662336 bytes with debugger overhead; this is not a
workload peak or leak proof. Instruments allocation collection was inconclusive.
Fresh exact-source physical execution and complete hosted certifications are
required for this successor; no predecessor evidence is relabelled.

Independent Metal review remains pending. cpu.reference.v1 VALIDATED;
apple.metal.v1 and android.vulkan.v1 UNKNOWN; DH-APPLE-METAL-01 DEFERRED /
NOT WAIVED / NOT SATISFIED. G7 traffic/permission walkthroughs, G8 device cohorts,
thermal/UI targets and 12MP target, G10 policy/implementation/sandbox evidence,
and G11 human assistive-technology observations remain open. The human XR sheet
is prepared, with observations unfilled. No human result is inferred.
