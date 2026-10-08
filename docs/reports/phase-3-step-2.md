# Phase 3 Step 2 — Geometry Tools

Status: LOCAL PASS; exact-SHA hosted closure pending.
Baseline: `dd76b63799b8f940c378523002527b2ad09e6dd8`.
Authority: explicit Product Owner authorization for Step 2 only. Step 1 remains
COMPLETE / FULL PASS; Steps 3–8 remain NOT STARTED.

## Delivered boundary

Crop and clockwise orthogonal rotation reuse the accepted /1/1 descriptors,
strict integer parser, outward crop rasterization, exact rotation kernels,
immutable original, complete ordered stacks and existing detached transition.
The [geometry integration contract](../contracts/manual-geometry-v1.md) specifies
all parameters, ownership, error/retry, replay and publication obligations.

The additive C family connects Step 1 gestures to the existing synchronized
document context. Snapshot and gesture ownership together remain bounded at 64
handles. Commit trials bounded metadata and admits the proposal before closing
the original gesture, preserving retry after allocation/admission failure.
Worker render composes existing evaluation/cancellation and PRV1; no scheduler,
new geometry model, approval authority or resource ceiling is introduced.

Native/C/JNI/Swift consumers exercise the shared core without redefining domain
semantics. Native integration imports a real deterministic PNG fixture into
verified managed storage, evaluates geometry/preview, approves through the
existing expected-epoch SQLite commit and reopens after every changed commit.
Canonical document bytes and original hash remain stable across restart.

## Acceptance evidence

Observed local evidence: Windows native fallback 21/21; Linux native 21/21;
ASan/UBSan 23/23 including both active negative instrumentation probes; Windows
source/documentation 55/55 and complete Linux source/script suite 95/95. Android
debug/release builds, lint, JVM tests (2/2 each variant) and instrumentation
compilation passed. Final geometry integration passed 8,540 checks on Windows,
Linux and ASan/UBSan. Geometry C allocation sweeps recovered 699 injected
failures on the Windows fallback and 804 on Linux/sanitizers, preserving pending
state and caller output sentinels. Documentation checks passed 8/8; workflow
lint, git diff --check and exact-text deferred Apple gate comparison passed.
No ADB device is connected: instrumentation execution, authoritative MSVC and
Apple compile/Swift/iOS simulator execution remain exact-SHA hosted gates.
No unobserved execution is PASS.

Regression scenarios cover minimum/full/edge crop, invalid zero/extents/overflow,
all quarter turns and invalid turns, exact crop->rotate versus rotate->crop,
crop->rotate->crop, four sequential clockwise turns, neutral interleaving,
1,001-update coalescing, zero-update commit, cancellation/stale bindings,
last-valid pending retry, failed preview/display preservation, combined handle
admission, canonical SQLite restart, replay and immutable original pixels/hash.
Existing independent kernel oracle/property tests remain mandatory.

## Scope classification and limitations

- **A:** shared geometry gesture boundary, PRV1 composition, durable replay,
  boundary/fault/ordering regressions and exact-SHA CI are Step 2 acceptance work.
- **B:** real editing-flow lifecycle and final Compose/SwiftUI editor integration
  belong to frozen Steps 6/7; later tool families and phase closure retain their
  existing Steps 3–5/8 authorization boundaries.
- **C:** Resize remains PROPOSED / unimplemented. Flip, perspective, free-angle,
  keystone and other unapproved geometry are backlog, with no shipping approval.

No architecture conflict or superseding ADR is required: accepted Phase 2 and
Step 1 semantics are reused. CPU authority/GPU rules and every existing ceiling
remain unchanged. Raster-to-source-hash binding and approval/display publication
remain application-owner responsibilities, not powers granted by a handle.
PRV1 fences stale results; optional sticky cancellation controls evaluation.
No hardware certification is performed. DH-APPLE-METAL-01 is unchanged and
unexecuted. No AI, final manual editor UI, new Phase 2 work or Phase 4 work.
Phase 3 Step 3 has NOT started.

## Files changed

- Documentation: `ROADMAP.md`, `QUALITY_GATES.md`,
  `docs/contracts/manual-geometry-v1.md`, `docs/reports/phase-3-step-2.md`.
- Shared core/build: `CMakeLists.txt`, `packages/core/Package.swift`,
  `packages/core/include/pixaura/manual.h`, `packages/core/src/document_api.cpp`,
  `packages/core/src/manual_tools.hpp`, `packages/core/src/manual_geometry_preview.cpp`.
- Native/source tests: `packages/core/tests/document_allocation_test.cpp`,
  `packages/core/tests/manual_geometry_boundary.h`,
  `packages/core/tests/manual_geometry_c_consumer.c`,
  `packages/core/tests/manual_geometry_test.cpp`, `tests/manual-geometry.test.mjs`.
- Platform boundaries/tests: `platforms/android/native/bridge.cpp`,
  `platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/CoreProbe.kt`,
  `platforms/android/app/src/androidTest/java/ai/pixaura/app/ManualBoundaryTest.kt`,
  `packages/core/swift/Tests/ManualBoundaryTests.swift`.
- Gate wiring: `.github/workflows/foundation.yml`,
  `.github/workflows/native-shells.yml`, `scripts/check-apple.sh`.
