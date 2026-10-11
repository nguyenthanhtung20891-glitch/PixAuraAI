# Phase 3 Step 8 — Certification & Closure

Status: **IN PROGRESS**. Authorized baseline:
`c0a0020d41b22b55fad494fab50d72b5903f7c22`. Steps 1–7 remain accepted FULL PASS.
Phase 3 remains open until every exit criterion and both final exact-SHA hosted
workflows are observed PASS. Phase 4 has NOT started.

The [authoritative certification matrix](../contracts/phase-3-certification-v1.md)
consolidates all 11 accepted tools, metadata, platform parity and test traceability.
Step 8 adds certification tests, an explicit SELinux Enforcing runtime precondition
and closure documentation. It adds no feature or new architecture decision.
Accepted ADR 0021 remains narrow and unchanged; Schema 3 is unchanged.

## Required evidence

- Independent geometry/tone/detail reference corpus, invalid/boundary/alpha/order coverage.
- Gesture/history, generation fences, deterministic races, churn, allocation/failure/resource/security regressions.
- Schemas 1/2/3 and explicit migrations, immutable assets, crash/reopen and deterministic replay.
- Every descriptor through neutral/invalid/cancel/changed preview/commit/Undo/Redo/reopen on Android and Swift owners.
- Actual hosted Android runtime, enforced SELinux and direct Android RENAME_NOREPLACE corpus.
- Native Windows/Linux/Apple, ASan/UBSan, both Android ABI boundaries, builds/lint/JVM/instrumentation.
- Swift macOS/simulator, iOS Debug/Release builds, app/UI flows and basic layout/accessibility.
- Foundation boundaries (7 jobs) and Native application shells (3 jobs), all SUCCESS on the same final closure SHA.
- Source/documentation/workflow checks and git diff --check.

## Observed local gates

- Source tests: **80/80**, zero failures/skips, including matrix/descriptor consistency.
- Windows existing Clang/Zig fallback: **28/28 native tests**, 167.37 seconds,
  including full allocation faults, gesture races and storage regressions.
- Independent tone fixture/matrix check PASS (940 fixtures); exact-rational
  detail corpus PASS (**280 fixtures**). Existing geometry independent references retained.
- Android Debug/Release, strict dependency verification, both lint variants,
  JVM variants and instrumentation APK build PASS (151 Gradle tasks).
- Workflow actionlint, affected shell syntax and git diff --check PASS.
- Local WSL has no Node executable; emulator-harness runtime tests remain a
  hosted Linux gate. Actual Android runtime and Apple execution remain hosted gates.

Unexecuted or pending checks are not PASS. Hosted evidence will be recorded after execution.

## Follow-ups and scope

A: certification coverage and exact-SHA closure; currently in progress.
B: DH-APPLE-METAL-01 remains DEFERRED and mandatory at the previously accepted
later hardening/Beta/release boundaries. Phase 2 is not reopened.
C: optional production polish and future catalogs/tools remain outside this step.
No unresolved architectural conflict identified. No Step 9 is created; no Phase 4 implementation.
