# Quality gates and promotion policy

## Phase 3 Step 4 acceptance

Only Sharpen/Blur are authorized. Require the frozen [detail contract](docs/contracts/manual-detail-v1.md), independent reference fixtures, exact neutral/alpha/border behavior, integer validation, ordered mixed stacks, shared gesture/preview fences and persistent restart/replay. Preserve schema 1/2 exact fingerprints; require independent strict schema 3 and explicit transactional 2->3 migration with complete state preservation and allocation/SQLite/crash rollback tests. Never auto-migrate or accept direct 1->3. Preserve all resource ceilings and two-raster ownership; bound and admit reusable three-row scratch and cancellation checkpoints. Windows/Linux/native/sanitizers, Android build/JVM/instrumentation, Apple/Swift/iOS simulator/app and exact-SHA hosted workflows must execute successfully. No detail GPU, final UI or additional tools. Steps 5-8 remain NOT STARTED; DH-APPLE-METAL-01 is unchanged/unexecuted. Step 4 is IN PROGRESS; see the [report](docs/reports/phase-3-step-4.md). No pending gate is PASS.

## Phase 3 Step 3 acceptance

Authorized scope: exposure (unchanged), brightness, contrast, highlights, shadows,
saturation and temperature. Require the [numerical contract](docs/contracts/manual-tone-color-v1.md),
independent reference fixtures, integer/boundary/version/alpha/no-op/order tests,
shared manual gesture/PRV1 composition, canonical storage/restart replay and
unchanged source hash. [ADR 0019](docs/adr/0019-explicit-storage-schema-evolution.md)
requires immutable schema-1 SQL/fingerprint, independent strict schema 2,
explicit transactional 1->2 migration, complete state preservation and rollback/
SQLite/allocation/crash tests. Ordinary open must not migrate.
Windows/Linux/native/sanitizers, Android build/JVM/instrumentation and Apple
compile/Swift/simulator plus exact-SHA hosted workflows must actually pass.
No new GPU kernels, final editor UI or additional tools. Step 4 is separately authorized; Steps 5-8 remain NOT
STARTED. DH-APPLE-METAL-01 is unchanged and unexecuted.
Step 3 is COMPLETE / FULL PASS (technical gates); observed evidence is in the [Step 3 report](docs/reports/phase-3-step-3.md). Unexecuted gates never count as PASS.

## Phase 3 Step 2 acceptance

Explicitly authorized: crop and clockwise orthogonal rotation only. Acceptance
requires the [manual geometry contract](docs/contracts/manual-geometry-v1.md):
exact existing /1/1 parameters and rasterization, shared checked C/JNI/Swift
boundaries, ordered pixels/replay, one detached proposal for 1,001 updates,
neutral/cancel/stale/failure safety, PRV1 publication checks and unchanged ceilings.
Verify canonical SQLite commit/reopen and immutable source hash, boundary/invalid
cases, exhaustive allocation failures, Windows/Linux native and ASan/UBSan,
Android JVM/instrumentation, Apple compile/Swift/simulator and exact-SHA hosted
regression closure. No unexecuted check is PASS. Resize remains PROPOSED.
Step 3 is COMPLETE / FULL PASS; Step 4 is separately authorized; Steps 5–8 are not started. DH-APPLE-METAL-01 is unchanged and not executed.
See the [Step 2 report](docs/reports/phase-3-step-2.md).

## Phase 3 Step 1 acceptance

Step 1 is COMPLETE / FULL PASS, accepted by the Product Owner at
`b29123037effd76d984780f59889f1fcbb3c8c76`. The following criteria retain its
acceptance requirements; that acceptance is unchanged by Step 2.
Acceptance requires shared bounded descriptors, exact integer validation and
canonical serialization, unknown version rejection, ordered replay, neutral/no-op
behavior, one revision per changed gesture and cancel/stale/failure atomicity.
Run existing source/documentation/native regressions, Windows and Linux ASan/UBSan,
Android JVM/native boundaries and Apple compile/Swift contract gates. Existing
gates and ceilings remain mandatory. See [report](docs/reports/phase-3-step-1.md)
and [contract](docs/contracts/manual-tools-v1.md). The official [eight-step Phase 3
roadmap](ROADMAP.md#frozen-phase-3-numbered-roadmap--manual-tools) is frozen;
Steps 1/2 are complete; Step 3 is COMPLETE / FULL PASS; Step 4 is separately authorized; Steps 5–8 remain NOT STARTED.

## Phase 3 scope discipline

Every new proposal must be classified as:

- **A:** required for current step acceptance.
- **B:** belongs to an already frozen later Phase 3 step; defer implementation until
  that step is separately authorized.
- **C:** backlog / nice-to-have; no shipping commitment or implementation approval.

Codex must not create additional Phase 3 steps, new shipping tools, Phase 3.5,
AI behavior or Phase 4 work without explicit Product Owner approval. A frozen
roadmap is not permission to begin the next step. Tool exclusions and separate
parameter/recipe freezes in ROADMAP.md are mandatory. Step 8 closes Phase 3 only
after all phase exit criteria pass: offline tool matrix, documented units/ranges,
deterministic replay, gesture history, restart/reload consistency, invalid/boundary
cases, Android/iOS parity and regression closure. Do not create Step 9 automatically.
Existing gates and resource/security invariants remain mandatory; no unexecuted
check is PASS. Historical records below are evidence, not current scope authority.

Phase 2 is CLOSED by the explicit [Product Owner/Architect decision](DECISIONS.md#phase-2-closure-decision-2026-10-08). Steps 1-11 are accepted FULL PASS; original first GPU pipeline implementation and hosted compile/regression are complete, with accepted physical Android Vulkan PASS 14/14. Apple physical Metal certification is DEFERRED, not waived, under DH-APPLE-METAL-01 below. Existing ceilings, CPU authority and GPU semantics remain unchanged. No new Phase 2 step is added; Phase 3 has not started and requires separate authorization. Historical milestone authorizations below do not reopen Phase 2.

## DH-APPLE-METAL-01: Deferred physical Apple Metal certification

- **Status:** DEFERRED / NOT EXECUTED; no real Apple hardware is currently available. This is not PASS, a waiver or removal of the gate.
- **Owner:** Product Owner/Architect; controlled physical Apple execution requires separately authorized device/signing access.
- **Decision:** [Phase 2 closure decision](DECISIONS.md#phase-2-closure-decision-2026-10-08) permits Phase 2 closure only. Technical criteria in [ADR 0018](docs/adr/0018-physical-gpu-certification.md) and [GPU tile v1](docs/contracts/gpu-tile-v1.md) remain mandatory.
- **Required completion:** before Phase 11 hardening acceptance, Phase 12 Beta readiness completion, production release, or any claim of physical Apple GPU certification.
- **PASS evidence:** real physical iPhone/iPad Metal pipeline creation, actual command dispatch/completion/readback and all 14/14 shared CPU parity cases; fallback disabled, clean committed checkout/exact source SHA, reviewed model/OS/GPU/runtime metadata and JSON XCTest attachment/result bundle/log evidence in the [Apple hardware report](docs/reports/phase-2-gpu-apple-hardware.md).
- **Promotion rule:** Phase 11 hardening acceptance, Beta readiness completion and production release remain blocked while this gate is DEFERRED, BLOCKED, FAIL or unexecuted. Hardware unavailability cannot silently extend the deadline or auto-waive the gate. Subsequent acceptance/release reports must explicitly record this gate's status and evidence; agents may mark PASS only from reviewed physical execution evidence.
- **Claim rule:** hosted Apple green builds, shader compilation, simulator rejection and Android certification cannot establish physical Apple GPU certification. Phase 2 CLOSED does not imply Apple hardware PASS.

Phase 2 Step 11 is authorized: [interactive preview request generation and publication ownership](docs/contracts/interactive-preview-lifecycle-v1.md) under [ADR 0017](docs/adr/0017-interactive-preview-generation-fence.md). Latest-wins native/platform fences, cooperative supersession and failed-request display preservation keep synchronous execution, existing ceilings and quiescent destruction. All existing local/hosted gates plus churn/races/overflow/platform ownership remain mandatory. [Step 11 report](docs/reports/phase-2-step-11.md); Step 12 is not started.

Step 10 requires existing reference-preview regressions plus rational-fit, premultiplied-linear bilinear goldens/oracle, malformed requests, cancellation/lifecycle/resource/fault recovery and real bounded Bitmap/CGImage consumers. All existing source/native/sanitizer/mobile/hosted gates remain mandatory; exact evidence is tracked in [Step 10 report](docs/reports/phase-2-step-10.md).

Phase 2 Step 9 requires independently owned 1:1 reference preview, exact RGBA8 goldens/transfer/alpha transitions, source immutability, aggregate admission/retry, lifecycle/stale/wrong-kind/cap/race/cancellation/failure injection, independent numerical properties and full existing local/hosted matrix. Real MSVC/Apple execution and actual ASan/UBSan remain mandatory. No GPU parity or production display certification. [Contract](docs/contracts/reference-preview-v1.md); [report](docs/reports/phase-2-step-9.md).

Current Product Owner authorization is Phase 2 Step 8, superseding historical limits below. Step 7 is user-attested FULL PASS at 2e0f975ca894372684c3febbc391dc3ac92d5407. Gates add bit-exact crop/rotate/mixed execution, plan agreement, bounded bitmap admission, cancellation/failure injection, C/JNI/Swift execution and the full existing local/hosted matrix. Autonomous commit/push/CI closure is authorized. Step 9 is not authorized. [Step 8 report](docs/reports/phase-2-step-8.md).

Current Product Owner authorization is Phase 2 Step 7, superseding historical limits below. Step 6 is accepted FULL PASS at fd545ac545d526e779ca82daf8ac58e20cf39f2f. Step 7 requires approved outward crop geometry, exact orthogonal mappings, checked admission, tile coverage/order, cancellation lifecycle/checkpoints/publication atomicity, adversarial/property/golden/bounded fuzz and allocation recovery tests, C/JNI/Swift consumers and the full existing local/hosted matrix. Autonomous commit/push/CI remediation is authorized until green. Step 8 is not authorized. Evidence: [Step 7 report](docs/reports/phase-2-step-7.md).

Current Product Owner authorization is Phase 2 Step 6, superseding historical authorization/remediation limits below. Accepted Step 5 HEAD: 7f1fa96ab0f24b44e4e28a8c6c0aff8e7a4024ce. Step 6 requires exact exposure numerical contracts, finite/alpha/overflow safety, strict shared operation parsing, bounded 256-operation stacks, failure atomicity and allocation recovery, deterministic golden/property/fuzz coverage, concurrent immutable ownership, informational performance baseline, independent C/JNI/Swift consumers, and the full existing local/hosted matrix. All required CI remediation cycles are authorized. Step 7 is not authorized. Evidence: [Step 6 report](docs/reports/phase-2-step-6.md). G6 GPU/renderer/export parity remains outside Step 6 because those implementations do not exist.

Current Product Owner authorization is Phase 2 Step 5, superseding historical Step 3/4 limits below. Step 4 is accepted at d26baf21be2dde5f0c7881e8007658c859572cbe. Step 5 requires exhaustive orientation/color/alpha/layout tests, bounded identity evaluation, deterministic property corpus, failure injection, C/JNI/Swift ownership and the existing full local/hosted matrix. Quota-aware CI permits at most one focused remediation cycle; Step 6 remains unauthorized. Evidence: [Step 5 report](docs/reports/phase-2-step-5.md). Step 4 adds verified asset-to-decoder admission, hostile metadata/profile/orientation cases, checked preallocation bounds, independent C/C++/JNI/Swift decode ownership, deterministic bounded fuzz seeds, failure injection and actual codec ASan/UBSan. Existing G0-G5 and mobile/Apple/workflow gates remain mandatory; renderer/GPU parity is outside this step. Evidence: [Step 4 report](docs/reports/phase-2-step-4.md).

Status vocabulary: PASS (executed with evidence), FAIL (executed and failed), BLOCKED (mandatory environment unavailable), DEFERRED (explicit product decision moves execution to a named later boundary; never PASS or waiver), NOT_APPLICABLE (out of milestone scope with reason). Skips are never PASS. A milestone advances only when mandatory gates at its acceptance boundary pass and advancement is authorized. An explicit Product Owner/Architect decision may carry an unexecuted gate to a named later boundary, as recorded for Phase 2 above; that gate remains mandatory and is not counted as PASS. Current authorization ends at Phase 2 Step 3 persistence/immutable storage; Step 4 requires a new instruction. User-attested prior Phase 0/1/Step 2 CI completion is recorded separately from independently observed execution. No unobserved CI run is evidence.

| Gate | Required evidence | Applies |
| --- | --- | --- |
| G0 specification | Required docs, ADRs, link validation, measurable milestone criteria, no product contradiction | Every phase |
| G1 source quality | Whitespace/conflict checks; warning-clean native build; platform lint/format/type checking when source exists | Every phase |
| G2 behavior | Unit/integration tests for changed behavior and safety/error paths | Every phase |
| G3 build | Host core build; Android native adapter compile; Apple Swift/C boundary compile; full app debug/release builds once shells exist | Phase 0 boundaries; Phase 1+ apps |
| G4 native safety | ASan/UBSan and malformed-input fixtures; fuzz regression seeds when parsers exist | Phase 0+ native code |
| G5 persistence | Migrations, interruption recovery, source hash preservation, atomic revision commits | Phase 2+ |
| G6 parity | CPU/GPU numeric tolerance and preview/export geometry consistency | Phase 2+ renderers |
| G7 offline/privacy | Core flows airplane-mode; zero photo traffic; permissions/backup/log checks | Phase 1 policies, Phase 3+ flows |
| G8 device/performance | Physical low/mid/high cohort; p50/p95, peak RSS and thermal data; bounded fallback | Phase 4+ |
| G9 AI quality/control | Allowlist/ranges/stale plans, model provenance/parity, suggestion evaluation, explicit approval | Phase 5+ as applicable |
| G10 commerce | Store sandbox lifecycle, offline entitlement, restore/refund tests; pricing/policy verification | Phase 10+ |
| G11 accessibility | Assistive-tech walkthrough, dynamic text, contrast, input alternatives | Shell baseline Phase 1; full Phase 11 |
| G12 release | Both platforms signed builds, SBOM/license/security review, rollback/recovery/support checklist; no critical defects | Phase 12 |

## Phase 0 acceptance
All thirteen requested authoritative root documents; README and source/build instructions; docs/adr decisions with accepted/proposed status and consequences; skeleton portable C ABI with independent C/C++ consumers, Android JNI and Swift bindings; automated foundation check; local warning-clean native build/tests; platform/sanitizer build automation. G0/G1/G2 must execute locally. G3 includes Android native compile/link, macOS core/C/Swift tests, unsigned iOS package build and iOS simulator Swift/C ABI tests. G4 requires actual execution of the shared core tests under ASan/UBSan plus negative probes whose runtime diagnostics prove instrumentation; compiler flags alone never qualify. Unavailable host prerequisites leave Phase 0 validation incomplete even if automation is configured. Do not proceed to Phase 1 on partial status or without the user's authorization to advance.

Phase 0 reports separately classify evidence as locally validated, CI validated, pending CI execution, or legitimately unavailable on the current host. The last category describes host availability and does not waive a mandatory gate. READY FOR CI means all mandatory local checks have passed and remaining platform jobs are executable but unobserved. FULLY PASSED requires every mandatory gate to have executed successfully, with a run URL/logs for CI evidence. No unobserved workflow receives CI validated status.

## Phase 1 acceptance
Phase 1 acceptance: native Compose/SwiftUI apps render Home/Editor/Projects/Settings and show ABI 1 through the actual shared boundary; mode selection survives navigation without project mutation; Android recreation and iOS scene restoration preserve shell state; debug/unsigned release builds; strict Android lint, JVM reducer tests, Android emulator JNI/Compose tests; Apple package/core tests, iOS simulator Swift/C ABI/UI tests; workflow/source checks and existing sanitizers; no dangerous/media/network permissions, analytics, authentication or content networking. Native labels, scrolling and system text/color/touch defaults supply the accessibility baseline; physical TalkBack/VoiceOver walkthrough evidence is tracked separately and never inferred from compilation.

CI executes both Foundation boundaries and Native application shells. Apple app/package execution remains mandatory even when Windows gates pass. Signing is outside Phase 1; no store credentials are needed for simulator tests or unsigned builds.

## Budgets to validate

Phase 2 Step 1 G0/G1/G2 acceptance is the [document contract](docs/contracts/image-document-v1.md), ADR 0008 and executable metadata/history fixtures, while rerunning existing applicable Phase 0/1 checks. No production persistence or renderer is introduced, so new G5 durability/G6 numerical execution is not applicable to this contract-only step; those gates remain mandatory for their later production implementations and full Phase 2 promotion. Metadata replay tests cannot certify pixel determinism, original-byte preservation, crash recovery or native handle safety. Apple execution on Windows remains BLOCKED, not PASS; unchanged prior CI success is user-attested, not a new result for the working tree.

Initial targets on recorded reference devices: UI frame p95 <=16.7 ms for ordinary interaction at 60 Hz; T0 720px basic adjustment preview p95 <=150 ms warm; cancel acknowledgement <=250 ms; import/project UI response <=100 ms excluding decode; 12 MP JPEG export <=10 s T1 and <=30 s T0; no unbounded allocations and tier caps per DEVICE_CAPABILITY_STRATEGY.md. Cold model loading separately measured. Targets are refined only via ADR with data, never quietly relaxed. Phase 2 sets numerical tolerances per operation before implementation; Phase 5 sets task-quality thresholds before model selection.

## Reporting and exceptions
Each report names criteria, exact commands/tool versions, results, affected files, failed/unrun gates, blockers and next milestone. Required environmental blocker includes remediation and owner. Never waive source overwrite, approval or privacy invariants. Temporary noncritical exception requires explicit documented owner, expiry and bounded exposure; user authorization cannot be inferred for a product-level compromise.

Phase 2 Step 2 requires bounded native parsing/canonical serialization, typed immutable metadata/history transitions, an independently versioned explicit-context C API, golden C/C++/platform fixture parity, invalid/stale ownership and allocation-failure tests, structured parser adversaries under actual ASan/UBSan, and preserved source/Windows/Linux/Android gates. Apple native/Swift fixture execution is mandatory on CI and remains BLOCKED locally on Windows. Step 2 does not implement a storage adapter or pixel processing; new G5 durable transaction/recovery and G6 pixel/GPU parity remain mandatory for those separately authorized implementations. Context runtime handle budgets do not alter manifest limits or permit pruning. See the [Step 2 report](docs/reports/phase-2-step-2.md).

Phase 2 Step 3 activates G5: normalized SQLite constraints, explicit storage version rejection/migration admission, atomic history/current/redo/session/epoch commits, verified streamed immutable assets before references, reopen/canonical parity, crash subprocesses and failure injection, reader/writer/duplicate-ingestion concurrency and checked platform storage boundary. Existing G0-G4 and shell gates remain mandatory. Actual hosted MSVC Debug and Apple execution must pass on the corrected published tree; local fallback/wiring never substitutes. G6 pixels, decoding/rendering and Step 4 are outside this authorization. See [storage contract](docs/contracts/project-storage-v1.md) and [Step 3 report](docs/reports/phase-2-step-3.md).
