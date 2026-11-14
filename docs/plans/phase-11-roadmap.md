# Phase 11 engineering roadmap

## Remaining software workstreams successor

Current implementation evidence: [remaining engineering report and blocker matrix](../reports/phase-11-remaining-engineering.md).
The four original Duo failures are resolved in the new local software regressions
(19/19 Duo and19/19conventional, plus largest-text bilingual tests); historical
failure evidence remains intact. Exact successor7+3CI certification is required.
G7source/negative tests, G8frozen-policy analysis, G11automated reachability and
operator runbook, and G10proposed ADR/development-only untrusted observation
validation are documented separately from physical/human/policy/store acceptance.
[ADR0024](../adr/0024-entitlement-policy-proposal.md) is PROPOSED, not approved.
DH-APPLE-METAL-01 remainsPASS/CLOSED for accepted scope, providers unchanged.
Phase11INPROGRESS, not closed; Phase12NOTAUTHORIZED.

## Current acceptance successor — 2026-11-14

DH-APPLE-METAL-01 = PASS, limited to the reviewed physical XR/iOS18.7.9/A12,
source33c809f6a4b91300491cc1e211a10dbdbccaf7f9 and14-case contract.
See [formal decision](../reports/dh-apple-metal-01-acceptance.md).
Workstream F hardware evidence and independent review are complete for this scope;
integrated Phase11 acceptance is not complete. cpu.reference.v1 VALIDATED;
apple.metal.v1 UNKNOWN; android.vulkan.v1 UNKNOWN. No provider admission or
production enablement. G7/G8/G10/G11 and unresolved Duo UI findings remain.
Phase11 IN PROGRESS, NOT CLOSED; Phase12 NOT STARTED. The original plan/table and
its unavailable-device/deferred-status assertions below are preserved as dated
planning history, superseded by this current acceptance record.

Authority: Product Owner's Phase 11 master autonomous engineering mission,
2026-11-12. Baseline: 874b6645c75cd227fbb974b891ff41991397acc2, main, clean.
Phase 11 AUTHORIZED / IN PROGRESS. Phase 12 NOT STARTED. A–F below are internal
workstreams, not a new frozen numbered step scheme. This plan precedes source
implementation. All historical phase reports and fingerprint records stay intact.

| Workstream | Existing/reusable evidence | Remaining executable work | Acceptance / CI | External prerequisites |
| --- | --- | --- | --- | --- |
| A Security/privacy (G7) | Baseline backup/permission/release scanners; native malformed/path/storage tests; no app networking/logging | Audit complete platform import/export/private storage paths; reproduce invalid-path/failed-import cleanup defects; smallest fixes and negative tests; dependency/input provenance review | No private-content logs/traffic, backups excluded, scoped permissions, safe paths/cleanup; focused native/platform/source tests and full 7+3 CI for changed scope | Physical airplane-mode/network and OS-backup observations when required; human-owned store/signing access is not assumed |
| B Reliability/fuzz/lifecycle (G4/G5/G9) | Baseline CTest 65, allocation/SQLite/crash/race/cancellation and app lifecycle suites | Add meaningful uncovered bounded fuzz/stress cases; run actual sanitizer/fault/teardown checks; preserve revision/approval fences; fix reproduced defects | No partial/stale publication, leaks or history corruption; failure retains Manual; both platform regressions for shared changes, full 7+3 CI | Real OS process-kill/storage/driver behavior beyond synthetic fault coverage remains device evidence |
| C Performance/resources (G8) | Accepted 10.1 host 2 warmups/7 samples at 0.8/6 MP, exit leaks 0; existing caps/dynamic policy | Successor host instrumentation with adequate sample methodology for p50/p95, repeated memory trends and recovery; retain original measurements unchanged | Actual units/sample counts/spread and host target recorded; no new RSS admission ceiling; existing targets unchanged | Physical LOW/MID/HIGH cohort, thermal/peak-memory/frame/cancel/12 MP export observations; unavailable devices block G8 completion |
| D Accessibility/bilingual (G11) | Catalog parity, native semantics and English UI tests; review/compare controls | Audit whole Android/iOS flow; fix reproduced semantics/input/layout gaps; automate available large-text and bilingual scenarios; prepare complete manual walkthrough protocol | English/Vietnamese, useful labels/actions/focus, scalable input, no assertion weakening; platform automation and full 7+3 CI | Required human TalkBack/VoiceOver, contrast and alternative-input walkthrough evidence cannot be fabricated |
| E AI/commerce (G9/G10) | Reference provider/planner, untrusted-envelope/Plan/binding/approval tests; MONETIZATION.md baseline policy | Run AI trust/range/stale/cancel/manual-recovery regressions; inventory all commerce implementation and tests; report exact policy/implementation gaps, not fake purchases | G9 native authority intact; G10 needs actual existing implementation and legitimate sandbox lifecycle/offline/restore/refund evidence | No StoreKit/Play Billing/entitlement implementation found in app source; offline grace still proposed pending ADR; store identifiers/accounts/sandbox and final policy are prerequisites. No new commercial policy chosen |
| F Physical GPU/integration | Phase 10.1 exact closure CI 43127870126 7/7 and 43127870204 3/3, attempt 1; physical tile protocols; GPU inference trust UNKNOWN | Discover devices/signing metadata only; run physical harness only if eligible device and authorization exist; integrate reports/provenance and certify final implementation SHA | All G7–G11 plus DH-APPLE-METAL-01 PASS, independent review, distinct closure commit with own 7+3 CI before formal closure | No physical Android available; paired iPad8,9 unavailable; connected Apple target is simulated; no existing team specified or signing/device authorization supplied. DH-APPLE-METAL-01 blocks Phase 11 hardening acceptance |

Execution: audit -> record this plan -> reproduce targeted issues -> smallest fixes
with successor source fingerprints -> focused fuzz/fault/platform checks -> host
measurements -> complete all remaining executable workstreams -> review/secrets/
source integrity -> commit/push main -> exact-SHA full Foundation/Native workflows.
Correct deterministic failures on a new SHA. Preserve an evidence/blocker matrix;
do not stop just because physical hardware is absent. No false gate PASS or waiver.

Frozen invariants: shared native authority; HUMAN Apply; single-use/source-bound
approval; stale/cancel fences; preview/export not revision; ordinary history;
stop/join/destroy; Manual after AI/resource failure; local/private ownership;
ABI/Schema 3/numerics unchanged. No learned model, GPU promotion, cloud or telemetry.
CPU VALIDATED; Vulkan/Metal UNKNOWN; DH-APPLE-METAL-01 DEFERRED / NOT WAIVED /
NOT SATISFIED. It MUST PASS before Phase 11 acceptance. Phase 12 stays NOT STARTED.
