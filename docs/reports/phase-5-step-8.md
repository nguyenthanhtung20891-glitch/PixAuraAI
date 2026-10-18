# Phase 5 Step 8 — closure certification

Baseline `b46b30c26af0790d0715bc48d24093b83290b8fe`; clean at audit. Step 7 accepted
COMPLETE / FULL PASS: Foundation 39038860025 7/7 and shells 39038860056 3/3.
Steps 1–7 COMPLETE / FULL PASS. Committed closure candidate status PENDING_EXACT_SHA.
Phase 6 NOT STARTED. Production model UNRESOLVED.

## Acceptance and scope

Only closure documents, source/document consistency checks, engineering manifest
and final-SHA evidence handling are added. No native/platform product code,
accepted contract, dependency, shader, profile, resource limit, trust, policy or
Schema 3 change. AGENTS.md's stale Phase 2 scope is updated to authorized Step 8.
Root architecture/decision/quality/roadmap notes identify current authority while
preserving historical entries. The [consolidated report](phase-5-closure.md) and
[manifest v1](phase-5-closure-v1.json) contain the audited eight-step state matrix,
eight contract versions/digests, exact historical SHAs/workflows, resource bounds,
provider evidence classes and deferred B/C items.

Closure becomes CLOSED / COMPLETE / FULL PASS only after both complete workflows
on the SAME final Step 8 SHA succeed: Foundation boundaries 7/7; Native application
shells 3/3. No pending/unobserved/skipped required job or historical SHA can close
Step 8. The engineering evidence checker rejects mismatched SHAs, incomplete,
failed/skipped/duplicate/missing jobs, foreign repositories and historical run IDs.
It emits an observed receipt under build/ only after reading final snapshots from
scripts/codex-gh.ps1 and checking clean HEAD. This is reporting, not a new runtime
trust/signing system. Committed candidate status stays PENDING_EXACT_SHA; the
observed receipt/final delivery resolve actual final SHA and completed states.

## Audit outcome

All eight frozen contracts agree in cumulative scope. Earlier provider contracts'
explicit selection is extended by existing Step 6 admission; Step 7 Option A
preserves release UNKNOWN despite software PASS. Registry declarations are not
executable operator/runtime support. No semantic conflict or new ADR is required.

CPU release VALIDATED; Vulkan/Metal release UNKNOWN; no provider CERTIFIED.
All three accepted software suites PASS and are required again on closure SHA.
Production zero-anchor behavior and test-purpose isolation unchanged. CPU exact
goldens/independent oracle, GPU tolerance `4e-38 + 3e-7 * abs(CPU)`, threshold >=0.5
and closed-band classification semantics retained. Step 5 metrics/synthetic corpus
retain explicit empty-mask behavior and no production-quality claim. U2NETP
AUDITED / UNSUPPORTED BY CURRENT PROFILE / UNAPPROVED / UNINCORPORATED /
CHECKPOINT RIGHTS UNRESOLVED. No weights or invented measurements.

Resources agree: 131072-byte context control; 4096-byte explicit prepare versus
8192-byte admission control (alternative paths); Phase 4 safety reserve unchanged;
Vulkan transient1638400 / Metal1114112; <=16384-pixel tiles; <=32 handles;
<=256 MiB context; reference input12 MiB/output4 MiB; package and parser limits
unchanged. Context independence is not a global RAM allocator or driver RSS claim.
Capability epochs, exact recovery, cancellation, latest-generation publication,
BUSY rejection and pre-execution-only fallback remain shared authority.

DH-APPLE-METAL-01 DEFERRED, NOT WAIVED, NOT SATISFIED at frozen later boundaries.
Apple evidence is hosted/simulator only; no physical Apple inference certification.
Historical Android exposure is supporting only, insufficient for inference trust.

## Validation ledger

No Step 8 PASS claim before execution. Required fresh gates: full Windows/Linux
native/source, ASan/UBSan with active negative probes, Android arm64-v8a and
armeabi-v7a build boundaries/debug/release/JVM/lint/instrumentation, Apple native,
Swift macOS/iOS Simulator, iOS app/UI, all retained Phase 0–4 and Steps 1–7,
closure source/document consistency, workflow lint and git diff --check.
Actual local commands/counts and final exact-SHA workflow IDs/results are recorded
in delivery and the observed receipt after execution; no baseline result transfers.

A: consistency/regressions/fresh certification. B: production model/runtime/operator
work, provenance/license, trust-promotion policy and deferred physical certification.
C: optional measurements/research. Fix A only. No architecture blocker found.
Schema 3 unchanged. No weights, external runtime, AI UX or Editing Agent.
Phase 6 NOT STARTED. No Step 9+. STOP AFTER PHASE 5 CLOSURE.

## Local closure validation

Windows-applicable full source list: 100/100 PASS, zero skips. Full Linux source:
142/142 PASS, zero skips (166003 ms). Final focused closure checks: 4/4 PASS.
Actionlint and git diff --check PASS. The initial Windows source attempt was 99/100
because the rewritten roadmap lacked a final newline; the file was corrected and
the unchanged hygiene gate rerun successfully. No gate or timeout was relaxed.
Full native/platform/ASan/UBSan execution remains required on the final closure
SHA; no old-SHA native/platform result is reported as Step 8 PASS here.

## Exact file scope

* `.github/workflows/foundation.yml`
* `.github/workflows/native-shells.yml`
* `AGENTS.md`
* `ARCHITECTURE.md`
* `DECISIONS.md`
* `docs/plans/phase-5-roadmap.md`
* `docs/reports/phase-5-closure.md`
* `docs/reports/phase-5-closure-v1.json`
* `docs/reports/phase-5-step-8.md`
* `QUALITY_GATES.md`
* `ROADMAP.md`
* `scripts/phase5-closure.mjs`
* `tests/phase5-closure.test.mjs`
* `platforms/ios/PixAuraAIUITests/ShellUITests.swift`

## Closure CI remediation

Candidate `28906d5b30ba0478f195dfc91cb91eac29f1f85f` passed Foundation
39085975402 (7/7), but shells 39085975388 failed its retained gesture UI test.
The cancel-completion waiter chained `exists` and `isEnabled` remote snapshots:
the log shows Cancel resolution retries consuming the 20-second wait, while
later editor actions and revision checks succeeded. This is not closure PASS.
The A-class test fix queries Cancel existence/disabled state together, retaining
the separate no-busy condition and original 20-second bound. Timeout diagnostics
are bounded to 16 KiB. No product, native or inference behavior changes.
Both complete workflows must execute again on the remediation SHA; the first
candidate's successful jobs cannot substitute for final-SHA results.
