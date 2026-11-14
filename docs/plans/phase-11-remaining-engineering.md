# Phase 11 remaining engineering execution plan

Baseline: 8387b167514b4a01f331ef05db32f22d5b4f9f5b. Phase 11 IN PROGRESS;
Phase 12 NOT AUTHORIZED. DH-APPLE-METAL-01 accepted PASS is preserved and is
not rerun for unrelated UI/documentation changes. Provider admission unchanged.
Historical evidence/source manifests stay immutable; owner signing edits stay local.

| Workstream | Reusable evidence | Executable work and acceptance | External boundary |
| --- | --- | --- | --- |
| A Duo | Preserved 19-test run: four failures; same-source conventional comparison four PASS | Compare actual frames/state; reproduce targeted preconditions, correct scroll/toolbar or test transport only where justified; all four plus full Duo and conventional UI regressions without weakened behavior assertions | Distinguish Simulator environment from physical observations |
| B G7 | Existing security-before/after tests, 4163 native malformed inputs, sanitizers, offline physical evidence | Audit scoped ingress/egress, paths, storage/backup and denial/lifecycle recovery; add focused negative tests only for uncovered defects; platform/security/release checks | OS traffic/permissions, real storage pressure and operator observations |
| C G8 | XR raw Release 100 retained samples; 6MP preview and12MP import RESOURCE8/manual recovery | Trace accounting/caps to rejection; preserve limits; document conflicting requirement and bounded resolution proposal, no silent target change; precise cohort/run worksheet | Additional devices, sustained thermal/frame/UI evidence; architecture approval for any policy/contract change |
| D G11 | en/vi automation; Duo screenshots/trees | Repair reproduced reachability/semantics/focus issues; run en/vi large text; operator-ready platform steps and required evidence | Actual VoiceOver/TalkBack/contrast/alternative-input observations |
| E G10 | MONETIZATION baseline, no store implementation | Proposed binding ADR with unresolved choices explicit; safe policy-independent evidence validation/test tooling only, no runtime access gate, fake verifier or entitlement grant | Product Owner policy approval, store accounts/configuration, signed sandbox evidence |

Implement smallest verified changes; preserve failing evidence; focused then affected
platform regression, additive source fingerprints, local source/privacy/release
checks. Commit/push main and verify complete Foundation7/7 and Native3/3 on the
new exact SHA, attempt1 where possible. No hardware/provider promotion or closure.
Final matrix separates software, physical, human, policy and store prerequisites.
