# Phase 4 Step 1 — Capability Contract & Probe Foundation

Status: **IMPLEMENTED / TECHNICAL GATES PASS / READY FOR REVIEW**.
Baseline: `56cd60560b6271b7d27bf5e03fd1d720a3aad2fa`.
Implementation SHA: `9fb321fc0d3b125d50cd31dc260ac6bb015dfff5`.
Phase 0–3 remain CLOSED/COMPLETE; Phase 3 final Foundation 38145575170 (7/7)
and Native application shells 38145575199 (3/3) passed on that baseline.
Only the minimum Step 1 foundation is authorized. Steps 2+ are NOT STARTED.

## Audit findings and proposed roadmap

Existing native limits/resource rejection, PRV1 ownership, shared manual lifecycle,
platform command/render lanes and immutable Schema 3 storage remain authorities.
There was no normalized profile service or centralized dynamic admission policy.
Existing GPU primitives are standalone bounded diagnostics, not manual preview
backend integration. Historical tier memory/preview values are targets, not
permission to increase hard ceilings. No frozen architecture conflict was found.

The finite [seven-step proposal](../plans/phase-4-roadmap.md) is PROPOSED / NOT
FROZEN: contract/probes; admission/tiers; dynamic downgrade; Android; Apple;
cross-tier replay/certification; closure. Product Owner must freeze the roadmap
and review/accept Step 1 before any Step 2+ work. No Step 8+ is proposed.

## Delivered foundation

[Capability v1](../contracts/capability-profile-v1.md) defines immutable/caller-owned
static facts and dynamic state, unknown values, separate GPU availability and
validation/provenance, checked C normalization and fixed canonical 76-byte
diagnostics. Pure C++ code allocates nothing, retains no state or callbacks, and
touches no editor/history/source/storage/resource authority. Platform protocols
and conservative JNI/Swift stubs report compile-target identity plus unprobed
unknown facts; they are explicitly not real OS/hardware probes.

Synthetic LOW/MID/HIGH fixtures contain 2/4/8 logical CPUs, 2/4/8 GiB RAM and
128 MiB/256 MiB/1 GiB process estimates. These are **SYNTHETIC POLICY PROFILES**,
not classification thresholds or physical measurements. The deliberately larger
HIGH estimate cannot change existing default limits. No physical device profile
or measured Z Fold7 tier is claimed. No physical Apple evidence exists.

No unnecessary identifying strings, user content, network, telemetry, analytics,
external profile loader, diagnostic cache or persistent profile state is added.
The schema and accepted manual/GPU numerical semantics remain unchanged.

## Validation required / execution status

Required: source/documentation/workflow checks; C/C++ native foundation tests
(synthetic/malformed/version/enum/GPU distinction/output preservation/independent
canonical golden/unchanged limits); 80000 parallel snapshot calls; Windows/Linux;
ASan/UBSan; both Android ABI builds; Android JVM/app builds/lint and actual JNI
instrumentation; Swift macOS/iOS simulator and app/UI regressions. Preserve all
Phase 0–3 gates. No unexecuted or skipped gate counts as PASS.

Observed local source/documentation checks: **84/84 PASS**, zero failures/skips.
New Windows C/C++ foundation tests: **2/2 PASS**, including independent byte
golden and 80000 parallel normalize/serialize calls. Android Debug/Release,
strict dependency verification, JVM variants, lint variants and instrumentation
APK build PASS (151 Gradle tasks). Actionlint and git diff --check PASS.
Full Windows fallback native regression: **30/30 PASS**, 232.11 seconds;
the final malformed-enum sweep also passes in an affected 2/2 foundation rerun.
All implementation-SHA hosted gates PASS; exact evidence follows.
ADB audit found no attached Android device; no physical profile is claimed.

Initial implementation SHA `20d930e26dac0d7b6f9521a7eb37860b7026ebb1` is not
accepted: sanitizer job 114519743451 in Foundation 38156255220 failed linking
the new C consumer through clang's C linker against the C++-instrumented core
(`__ubsan_vptr_type_cache` / dynamic-type handler unresolved). The existing
C-consumer convention selects LINKER_LANGUAGE CXX. Applying that same one-line
setting supplies the C++ sanitizer runtime; no instrumentation, warning, test,
security control or resource bound is reduced. Local WSL affected ASan/UBSan
foundation tests now pass **2/2**, including 80000 concurrent snapshot calls.
The fresh complete exact-SHA validation below passed after this remediation.
The new foundation targets also use the existing fail-fast ASAN_OPTIONS/
UBSAN_OPTIONS test environment, preserving leak checking and halt-on-error.
Local affected foundation tests PASS 2/2 and active negative controls PASS 2/2
(ASan exit 86 / UBSan exit 87), with the fail-fast environment observed.
No runtime pressure/thermal downgrade, policy-refresh/editor races, physical
profiling or cross-tier execution certification is claimed in Step 1; these are
the explicit later bounded roadmap items.

## Exact-SHA hosted implementation evidence

All jobs below executed on `9fb321fc0d3b125d50cd31dc260ac6bb015dfff5` and concluded
SUCCESS. The earlier failed SHA is retained as failure evidence, not acceptance.

| Workflow / job | Run ID | Job ID | Observed result |
| --- | --- | --- | --- |
| Foundation / workflow lint | 38156780631 | 114521277872 | PASS |
| Foundation / Windows | 38156780631 | 114521277939 | 30/30 native PASS |
| Foundation / Linux | 38156780631 | 114521277936 | 31/31 native PASS |
| Foundation / ASan+UBSan | 38156780631 | 114521277931 | 33/33 PASS; fail-fast new tests; active negative probes exit 86/87 |
| Foundation / Apple | 38156780631 | 114521277732 | 30/30 native; macOS Swift 16/16; simulator Swift 16/16 |
| Foundation / Android arm64-v8a | 38156780631 | 114521277880 | native boundary build PASS |
| Foundation / Android armeabi-v7a | 38156780631 | 114521277899 | native boundary build PASS |
| Shells / source | 38156780599 | 114521277841 | 120/120; zero failures/skips |
| Shells / Android | 38156780599 | 114521277729 | Debug/Release/lint PASS; JVM 4/4 each; runtime 17/17, zero failures/errors/skips |
| Shells / iOS | 38156780599 | 114521278263 | native/Swift/simulator PASS; Debug/Release builds PASS; app 6/6; UI 2/2 |

[Foundation 38156780631](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/38156780631)
and [Native application shells 38156780599](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/38156780599)
pass **10/10 jobs** on the same implementation SHA. Native tests explicitly run
the common synthetic/malformed/golden/unchanged-limit corpus and 80000 parallel
snapshot calls. Android XML names both `CapabilityFoundationTest` cases and
records actual runtime execution; JVM reports record the detached-seam test.
Apple logs name both `CapabilityFoundationTests` on macOS and simulator.
All existing import/editor/history/reopen/preview/storage/security/resource
regressions remain green. No skipped required gate is counted PASS.

Observed seam facts are intentionally conservative: Android platform=ANDROID,
Apple platform=APPLE; unprobed CPU/RAM/process budget=0 (UNKNOWN), constrained=1,
GPU validation=NOT_VALIDATED. These tests certify transport/normalization/ownership,
not physical-device capability measurements or selected LOW/MID/HIGH tiers.
Shipping UI and manual semantics are unchanged. No attached Android device or
physical Apple probe is claimed. GPU physical certification remains separate.

This finalization changes documentation only. Delivery requires both full
workflows to pass all ten jobs on that final commit's exact SHA; the final
delivery supplies its SHA, workflow IDs and conclusions. No prior-SHA green
substitutes. Product Owner review/acceptance and roadmap freeze remain pending.

The documentation SHA `d93948f0e59bc5a0f2121c91a4861653739c12f2`
passed Foundation 38158116892 (7/7) and source/Android shell jobs, but iOS
job 114525187026 in shells 38158116898 failed its existing rotation-selection
UI postcondition. Native/Swift, debug/release builds and six app tests passed.
The captured hierarchy showed an enabled, unselected rotation tool with no
editor error after a synthesized coordinate tap. A bounded UI-harness correction
re-resolves and retries that unselected tool once above the scrollbar; core
errors do not retry and the original selection/flow assertions remain mandatory.
No production editor code changes. Final delivery requires all ten exact-SHA
jobs on the correction commit; the failed UI run is not counted PASS.

## Files and review

Core: capability.h/capability.cpp; C/C++ foundation and fixture/boundary tests;
CMake and Swift Package source wiring. Platform: conservative Kotlin/Swift seams,
JNI foundation diagnostics, Android JVM/instrumentation and Swift boundary tests.
Engineering: capability source/contract tests, existing CI/Apple source-test lists.
Documentation: capability contract, finite roadmap proposal, this report and
current authorization entries in ROADMAP/QUALITY_GATES/ARCHITECTURE/DECISIONS.
Historical Phase 3 closure and DH-APPLE-METAL-01 are not rewritten.

A: foundation, sanitizer linker/fail-fast remediation and all technical gates PASS;
no unresolved technical defect. Observe final documentation-SHA gates before delivery.
B: proposed Steps 2–7 after roadmap freeze/acceptance/new authorization; mandatory
physical Apple Metal certification at its existing later hardening/Beta/release gates.
C: optional dashboards/device scores/broad diagnostics/polish.
No unresolved architecture conflict. No Phase 5, AI/model/runtime, Editing Agent,
new manual tool/preset/UI feature, GPU expansion, network or storage-schema work.
STOP after Step 1 delivery; do not implement Steps 2+ automatically.
