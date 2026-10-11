# Phase 4 Step 1 — Capability Contract & Probe Foundation

Status: **IN PROGRESS**. Baseline: `56cd60560b6271b7d27bf5e03fd1d720a3aad2fa`.
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
Exact-SHA hosted gates are pending observation.
ADB audit found no attached Android device; no physical profile is claimed.

Initial implementation SHA `20d930e26dac0d7b6f9521a7eb37860b7026ebb1` is not
accepted: sanitizer job 114519743451 in Foundation 38156255220 failed linking
the new C consumer through clang's C linker against the C++-instrumented core
(`__ubsan_vptr_type_cache` / dynamic-type handler unresolved). The existing
C-consumer convention selects LINKER_LANGUAGE CXX. Applying that same one-line
setting supplies the C++ sanitizer runtime; no instrumentation, warning, test,
security control or resource bound is reduced. Local WSL affected ASan/UBSan
foundation tests now pass **2/2**, including 80000 concurrent snapshot calls.
Fresh complete exact-SHA closure remains required.
No runtime pressure/thermal downgrade, policy-refresh/editor races, physical
profiling or cross-tier execution certification is claimed in Step 1; these are
the explicit later bounded roadmap items.

## Files and review

Core: capability.h/capability.cpp; C/C++ foundation and fixture/boundary tests;
CMake and Swift Package source wiring. Platform: conservative Kotlin/Swift seams,
JNI foundation diagnostics, Android JVM/instrumentation and Swift boundary tests.
Engineering: capability source/contract tests, existing CI/Apple source-test lists.
Documentation: capability contract, finite roadmap proposal, this report and
current authorization entries in ROADMAP/QUALITY_GATES/ARCHITECTURE/DECISIONS.
Historical Phase 3 closure and DH-APPLE-METAL-01 are not rewritten.

A: complete Step 1 validation and any narrowly required remediation.
B: proposed Steps 2–7 after roadmap freeze/acceptance/new authorization; mandatory
physical Apple Metal certification at its existing later hardening/Beta/release gates.
C: optional dashboards/device scores/broad diagnostics/polish.
No unresolved architecture conflict. No Phase 5, AI/model/runtime, Editing Agent,
new manual tool/preset/UI feature, GPU expansion, network or storage-schema work.
STOP after Step 1 delivery; do not implement Steps 2+ automatically.
