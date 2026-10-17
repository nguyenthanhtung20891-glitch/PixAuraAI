# Phase 5 Step 6 — Capability-aware admission and fallback

Status: IN PROGRESS; no unobserved gate counts as PASS. Baseline
59d846967fffef7e735597f6b65301841659336f. Step 5 accepted FULL PASS:
Foundation 38999701451 7/7, Native shells 38999701458 3/3.
Only Step 6 authorized. Eight-step roadmap retained; Step 7/Phase 6 not started.

## Audit and design

Read product/architecture/quality/roadmap/decision authorities, capability,
dynamic policy, package, inference and provider contracts, Step 5 evidence and
existing native/JNI/Swift ownership. Phase 4 already factors a private fixed-size
policy engine out of its raster facade. Reusing that engine requires no threshold,
recovery, pressure enum, trust or execution-status redefinition. Its strategy is
consumed directly; no inference is sent as a pretend preview/commit request.

Provider descriptors report CPU validation 1, Vulkan/Metal UNKNOWN 0. Availability
and platform GPU validation are insufficient to upgrade inference trust. This
implementation leaves accelerated automatic selection ineligible until separately
reviewed provider validation, while exercising future validated/certified states
ONLY in private synthetic adapters. No production model is selected. U2NETP and
broader runtimes/operators remain unsupported and unincorporated.

[Admission contract](../contracts/inference-admission-v1.md) defines the additive
checked C family, deterministic platform priority, reason codes, pre-execution-only
fallback, context-wide BUSY rejection, one-shot plan and epoch publication fences.
Shared ownership/resource registry is reused. Admission reserves immutable input,
model metadata, output and provider-declared transient storage before returning.
A small native resource mutex serializes lease checks with lower cap publication;
no GPU wait holds that mutex. Existing output capacities remain conservatively
charged until release. There is no global device allocator in Phase 4; this
does not invent one or claim per-context budgets globally partition RAM.

## Validation ledger

Tests cover LOW/MID/HIGH, five pressure states, three synthetic provider trust
states, Android/Apple routing, unavailable/trust/capability/model/resource fallback,
exact and one-byte resource boundaries, overflow rejection, malformed tensor and
normalization, nonfinite/range inputs, cancellation, stale epoch, Phase 4 recovery,
concurrent admission, independent contexts, invalid output, no replay after started
INTERNAL, output publication eligibility and release accounting. Same shared
corpus is invoked by existing native/JNI/Swift checked boundary certifications.
Synthetic trust never establishes production validation/certification.
The final matrix has 180 rows (thermal and memory separately), plus six freshly
signed requirement cases using existing TEST-ONLY keys and unchanged ONNX bytes.
Existing 32 signed ONNX cases/four CPU goldens remain byte-identical. Minimum
tier/thread/validation declarations are retained after existing verification;
signed declared memory is a conservative floor, never a replacement for larger
native estimates. No canonical/package schema or verification behavior changes.

Local full-source 136/136 PASS, zero skips; focused source 8/8 and workflow lint
PASS. Final Windows fallback/native and Linux focused gates 9/9 each PASS;
ASan/UBSan focused 11/11 PASS with negative instrumentation probes confirmed.
Admission corpus: 180 rows, 3734 checks, zero failures on all three local builds.
A redundant local full Linux attempt was stopped during unchanged editor
allocation-fault stress on the slow mounted workspace; it is NOT recorded PASS.
Both final-SHA hosted workflows must run complete retained suites, including that
test. No assertion/sanitizer relaxed. Local admission averages (222 small calls,
including validation/parse/input snapshot/reservation): Linux ~10.3us, Windows
~44.8us, sanitizer ~94.6us; engineering observations, not adaptive policy inputs.

Actual commands/counts/observations and final exact-SHA workflows are recorded
in the final delivery. Both complete fresh workflows on the SAME final Step 6
SHA are required. No old-SHA PASS is transferred. Admission preparation/input
snapshot time is reported separately from execution, not a benchmark heuristic.

First candidate 29e67a9 passed source and both Android ABI boundaries, but GCC CI
rejected misleading single-line control flow and external linkage of a test lambda
capturing a private type. Local C++ builds used Clang and did not expose those
GCC diagnostics. Control-flow braces and internal test-helper linkage fix them;
no warning is suppressed. Windows source CI also exposed CRLF checkout of the
new generated .hpp: an explicit LF attribute now matches existing fixture policy.
The exact fixture check is retained unchanged. These fixes require a new SHA and
both complete fresh workflows; prior candidate passes cannot close Step 6.

A: policy correctness, trusted authority, races, security and complete certification.
B: Step 7 provider certification and separately reviewed production model/runtime
architecture. C: optional tuning/physical measurements. No architecture conflict
identified by audit. No Step 7 work is included in these admission tests.

Schema 3, provider profiles, normalization and CPU semantic authority unchanged.
No AI UX, Editing Agent, external runtime, weights/download or telemetry.
DH-APPLE-METAL-01 unchanged and DEFERRED; no physical Apple claim.
Final delivery supplies exact-SHA acceptance without a later uncertified status-only
commit. Stop after Step 6.
