# Phase 11 final acceptance matrix — working inventory

Status: ENGINEERING IN PROGRESS. This is a resumable inventory, not the final
engineering delivery, independent acceptance, or closure package. Phase 12–14
implementation is not authorized. Governing definitions: QUALITY_GATES.md G0–G11;
execution authority: AGENTS.md current full-completion successor and approved
CR18-01/CR18-02. Historical reports remain unchanged.

Current committed baseline: `2f823a55c36271b19847fade7134aa9b460b715d`.
Implementation is uncommitted; no successor SHA or successor CI is claimed.
Baseline Foundation 43809264602 and Native shells 43809264489 were independently
accepted, exact SHA, attempt 1. They do not certify the current working tree.

| Gate | Current engineering evidence | Required before acceptance |
| --- | --- | --- |
| G0 specification | Approved Schema 4/editor-v2 scope; ADR-0024 commercial policy; versioned reader/stage/writer contracts; full-completion plan and chronological checkpoint | Reconcile final integrated documentation, capabilities and exact-source receipts; independent review |
| G1 source quality | Protected numerical sources and legacy C headers preserved; only two approved storage extensions; current source contracts 227/227 on supported macOS scope; owner PBX excluded | Final strict source-v20 inventory, complete Linux controller harness, platform lint/warnings/release checks and exact-SHA CI |
| G2 behavior | Latest iOS units 33/33; Android instrumentation 88/88; native real Free quota delivery/denial/manual recovery; earlier complete UI receipts preserved | Latest Duo 20/20 full UI precedes the frame repair; current affected UI 3/3, max-text/dark EN/VI 2/2 and conventional comparison 3/3 pass. Remaining store integration and final complete affected regressions |
| G3 build | Current Simulator/Emulator Debug applications built for actual tests; shared native and platform bridges previously built locally | Final changed-source Debug/Release/security builds for both platforms, all hosted boundary/app jobs |
| G4 native safety | Existing parser/fault/resource/lifecycle evidence retained; latest native store/quota/coordinator/crash host 6/6 and ASan/UBSan 6/6; preview sanitizer 4/4 | Remaining integration safety tests and final complete affected sanitizer/fuzz/malformed-input certification |
| G5 persistence | Opt-in Schema 4, same-transaction native revision/receipt, migration and native exactly-once trial reconciliation; v1 default remains Schema 3 | Opt-in store journal now proves atomic facts/high-water/ack, crash rollback/reopen, corruption and revocation in synthetic protocol tests; actual platform integration and final source certification remain |
| G6 parity | Native 12 MP tiled/row-reader CPU reference, geometry, codec and full-output oracles; shared source/revision binding | Final full tool/geometry/codec qualification and platform application receipts; provider admission remains separate |
| G7 offline/privacy | Platform-scoped import/export, private key/storage/backup, negative-input and synthetic cleanup tests; no checkout/network/photo-upload expansion | Physical permission denial, airplane-mode/traffic, data-protection/storage/cleanup/recovery observations from operator runbook |
| G8 device/performance | Real 4000×3000 native and application workflows; bounded LOW 64 MiB tests, original-size JPEG/PNG oracles, cancellation/resource rejection evidence; original XR measurements retained | Physical LOW/MID/HIGH Apple and Android latency distribution, peak RSS, pressure and thermal qualification of final implementation; no host/Simulator substitution |
| G9 AI quality/control | Existing admitted deterministic provider/planner; native source-bound single-use Human Apply; stale/cancel/resource/manual recovery and actual Free delivery tests | Complete affected final native/platform/CI regressions; no learned model or GPU provider promotion |
| G10 commerce | Approved 5-photo/3-AI Free policy and 14-day native-receipt trial; actual application admission; usage-counter/denial correction; UTC/local reset display; Apple signature/cache and Google verifier components | Opt-in native verified-store owner/journal and synthetic crash/revocation/grace/uncertainty proofs now pass. **Software still unfinished:** version3 Swift/JNI application ingress and exact-proof protected-cache repair are implemented with initial synthetic signature tests; current assembled platform/lifecycle/resource and final regressions remain. **External:** legitimate catalog/accounts/backend, Billing privacy admission and authentic sandbox lifecycle |
| G11 accessibility | EN/VI strings, native status presentation and automated UI/accessibility checks; earlier maximum-text/dark receipts preserved | Current max-text/dark EN/VI and fontScale2 automated comparisons pass; final integration regressions plus actual human VoiceOver/TalkBack, contrast, focus and alternative-input evidence; automated tests are not human acceptance |

G12 is Phase 12 release scope and is not started by this mission.

## Latest local receipts

All paths below are relative to `build/phase-11-final/`, excluded from source Git.
Counts describe the named run only; neither local success nor a draft fingerprint
manifest constitutes exact-final-SHA hosted certification.

- `local-reset-complete-ios-units-test.log` / `.xcresult`: 33/33 PASS, 21.389 s.
- `quota-reset-android-complete-test.log` / `-results/`: 88/88 PASS, no skipped or
  failed cases, 3m35s. Owned Emulator controller exited successfully; transport status TRANSPORT_TEST_ONLY, XML 88 tests/0 failures/0 skipped.
- `quota-reset-source-macos-supported-test.log`: 222/222 PASS, no skips,
  1501.045 ms. Excludes the separate 36-test `android-emulator.test.mjs` file:
  GNU `realpath -m` is absent locally. Full source coverage remains mandatory in
  Linux CI; no test was disabled or assertion changed to hide this prerequisite.
- `retained-installation-native-test.log`: coordinator/crash 2/2 PASS, 0.97 s.
- `retained-installation-sanitizer-test.log`: coordinator/crash 2/2 PASS, 2.22 s.
- `quota-reset-isolated-duo-complete-ui-test.log` / `.xcresult`: 20/20 PASS, 1169.396 s (before the subsequent preview contention repair).
  Exact same iPhone Duo device type and iOS runtime, fresh synthetic installation;
  retained original Simulator/ledger/keys are unchanged. Device metadata is in
  `isolated-duo-device.json`. This is Simulator evidence, not physical evidence.

Predecessor failures remain retained: full iOS 28/31, focused retained-installation
lifecycle failures, Free quota presentation's five assertions, and Android fixture
cleanup/secondary failures. Their successors do not erase or reclassify them.

## Engineering dependencies that must not be mislabeled external

1. Complete final validation of the native verified-store owner and typed application bridges.
   A caller-supplied paid boolean, arbitrary local MAC, mock envelope or opaque
   cache blob is not an authenticated store transaction or native success receipt.
2. Exact-proof native acknowledgment now gates per-product cache repair; finish
   current lifecycle/resource/security regressions and actual assembly checks.
3. Preview BUSY/STALE distinction is implemented and focused host/ASan/platform
   regressions pass; complete final full affected integration regressions.
4. Complete final tool/geometry/codec, fault/concurrency/sanitizer, platform release,
   privacy/security and strict provenance checks.
5. Audit/stage only legitimate source/docs (exclude personal PBX/signing changes and
   synthetic root test directories), commit/push main, then verify all ten jobs on
   the exact final SHA and record attempts. Prepare one consolidated review package.

## External acceptance boundaries

- Physical G7 observations and G8 LOW/MID/HIGH Apple **and** Android cohorts,
  operator access and actual final-source raw measurements.
- Genuine StoreKit/Play configuration, authorized store accounts, authentic signed
  sandbox purchase/restore/refund/revoke/expiry evidence, approved verifier
  deployment and Android Billing dependency/privacy admission. Checkout stays off.
- Human G11 assistive-technology and alternative-input walkthroughs with recorded
  observations. Operator instructions: [operator runbook](../plans/phase-11-final-operator-runbook.md).
- Independent Principal Architect review and acceptance before any closure action.

DH-APPLE-METAL-01 remains PASS/CLOSED for its independently accepted historical
XR/14-case hardware scope. It is not reopened here. `cpu.reference.v1` remains
VALIDATED; `apple.metal.v1` and `android.vulkan.v1` remain UNKNOWN. No production
GPU activation, physical all-device claim, Phase 11 closure, or production release
readiness follows from this inventory.

## 2026-11-25 successor receipts

- `store-owner-source-refresh-test.log`: 227/227 PASS, 0 skips, 2118.466 ms,
  supported macOS source scope. The separate 36 Linux controller tests remain
  mandatory in hosted CI; the primary Git index and owner PBX were untouched.
- `store-owner-v3-grace-test.log`: native store/quota/coordinator/crash 6/6 PASS,
  1.61 s; `store-owner-v3-grace-sanitizer-test.log`: 6/6 PASS, 3.23 s.
- `frame-currency-sanitizer-test.log`: 4/4 PASS; iOS app 33/33 and affected UI
  3/3 PASS; Android 16/16 and fontScale2 EN/VI 2/2 PASS.
- `frame-currency-maxtext` 2/2 PASS and `frame-currency-conventional` 3/3 PASS
  are Simulator automation, not human accessibility observations.
- `store-owner-full-native-test.log`: in progress; inspect final summary before
  recording a result. No final implementation commit/CI receipt exists yet.

## Version3 application integration — current working successor

The shipping native runtime opens explicit private commercial format3. New
`store_ingress_v3.h` callbacks bind the actual configured platform verifier to the
SAME owner, retain it through active work and reject nonce replay. Native receipt
includes exact signed-proof SHA256. Extra persistent/transient 262144-byte leases
are accounted under unchanged aggregate/effective ceilings. This is not a v1 ABI
or numerical-kernel change.

Swift worker bridge and optional actual StoreKit/pinned-JWS adapter, private SDK
freshness fence and post-native-COMMIT cache repair are implemented. The application
pool/factory uses optional actual bundle configuration; missing config stays off.
Android JNI/configured P-256 adapter, scoped request/lease and per-product native-ack
cache repair are implemented. The optional manifest loader supplies no product/key
or endpoint by default. No Billing runtime, checkout or network permission is added.

Receipts: Swift host6/6 PASS29.635s; isolated iOS34/34 PASS21.633s before the last
factory assembly edits; Android synthetic JNI/cache8/8 PASS11s, XML0 failures/errors/
skips. Latest full native/sanitizer/iOS/Android and assembled-source suites are
running, not PASS until final summaries are verified. No authentic purchase, human
accessibility or physical cohort result follows from those synthetic tests.
