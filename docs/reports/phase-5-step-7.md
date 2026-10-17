# Phase 5 Step 7 — Provider parity and security certification

Baseline: `5592cd839c2607379c6cb8c02a4b42a44e80311d` (Step 6 accepted FULL PASS).
Historical baseline workflows independently observed SUCCESS: Foundation 39015611339
(7/7), Native shells 39015611451 (3/3); supporting baseline only.
Status: certification candidate; complete same-final-SHA Foundation boundaries
and Native application shells SUCCESS is mandatory for acceptance. Final delivery
records exact SHA/run IDs/results after observation; no later uncertified
status-only commit. Step 8 and Phase 6 NOT STARTED.

## Approved architecture decision and trust

Product Owner explicitly approved Option A after the trust audit. Software-suite
results do not grant release trust. No new hosted-evidence promotion policy.
[Certification contract v1](../contracts/inference-provider-certification-v1.md)
is frozen; [engineering record](../../tests/fixtures/inference-certification-v1.json)
is not a runtime authority or Schema 3 data.

| Provider / implementation | Availability | Software suite | Hosted/simulator evidence | Physical inference evidence | Release trust |
| --- | --- | --- | --- | --- | --- |
| cpu.reference.v1 / version 1 + final source SHA | Available | Exact goldens, independent full-vector dyadic oracle and shared security suite; final-SHA workflows required | Windows/Linux/Apple host/Android x86_64 runtime; ARM ABI builds | No physical certification claim | VALIDATED (1), unchanged |
| android.vulkan.v1 / version 1 + final source SHA | Actual descriptor probe | CPU-relative full-vector suite and shared security suite; observed Android execution required | Emulator software/integration only | NOT OBSERVED for Phase 5 inference | UNKNOWN (0), unchanged even when software suite passes |
| apple.metal.v1 / version 1 + final source SHA | Actual descriptor probe | CPU-relative full-vector suite and shared security suite; observed Apple execution required | Hosted macOS/iOS Simulator software/integration only | NOT OBSERVED; CERTIFIED forbidden | UNKNOWN (0), unchanged even when software suite passes |

Release descriptor assignments are unchanged. AVAILABLE=2 belongs to a separate
enum from validation; it never grants trust. Capability vocabulary remains
NOT_VALIDATED/RUNTIME_VALIDATED/PHYSICAL_CERTIFIED (0/1/2); only private test
adapters simulate all three states. Step 6 requires BOTH reviewed inference trust
and Phase 4 backend permission. Dynamic changes do not promote compiled trust;
three-observation Phase 4 recovery is unchanged. Existing production adapters
continue reporting unvalidated GPU facts. No vendor/device/OS/timing heuristic.

Historical Z Fold7 Vulkan PASS 14/14 at
`ade92bea17561a99d0fe3275cbfc75c21fd07dae` covers identity/exposure dispatch,
completion/readback and CPU parity. It is supporting backend evidence only,
insufficient for Phase 5 inference certification. No fresh physical inference
claim. Apple hosted results never satisfy DH-APPLE-METAL-01, which remains
DEFERRED at its existing hardening/Beta/release/physical-claim boundaries.

## Implemented certification coverage

The new common native suite runs from retained C/JNI/Swift certification entry
points for each supported provider. Every execution starts with canonical Step 1
verification of signed purpose-2 fixtures, including trust/key/signature/payload
checks; it then adopts immutable verified ownership and calls the checked native
prepare/execute/query/copy/release API. No unsigned or raw-path execution.
Production store remains empty by frozen policy, release rejects test keys, and
positive test-build execution is not positive release-package execution.

* All 15 accepted quarter-weight triples, plus retained 1x1/2x2/8x8/256x256/1024x1024
  shapes and signed normalization, execute zero/one/gradient/checker/IEEE-boundary
  patterns. 105 complete vectors, 10,159,065 probabilities/provider; full tensor
  metadata and complete arrays are compared. Sampled retained normalization,
  maximum-shape and context-isolation checks were also replaced with full arrays.
* CPU exact four binary32 goldens remain byte-identical. New zero/one/gradient/
  checker arrays additionally compare every CPU element with an independent
  integer/dyadic oracle, not a second invocation of the interpreter alone.
* GPU bounds remain independently accepted
  `abs(Vulkan - CPU) <= 4e-38 + 3e-7 * abs(CPU)` and
  `abs(Metal - CPU) <= 4e-38 + 3e-7 * abs(CPU)`. Different backend arithmetic,
  same resulting formula; neither tolerance nor CPU semantics changed.
* Threshold remains foreground >=0.5. Outside the closed CPU tolerance band,
  classifications must match; inside, permitted rounding may disagree. Explicit
  CPU 0.5 versus accelerator 0x3effffff proves allowed disagreement without a
  shifted threshold. Boundary vectors cover near-zero/half/one, rounding and
  subnormals; full probabilities must still satisfy parity everywhere.
* Retain full Step 1 package corpus (45 rows, independent RFC checks and filesystem
  checks), add 96 deterministic manifest/signature/payload bit mutations with seed
  0x705ec001. No mutation may produce a verified package.
* Retain 32 signed ONNX graphs; add 15 signed malformed wire/profile cases:
  unsupported fixed32/group wire types, excessive nesting, bounded truncations,
  NaN/positive Inf/negative Inf/negative initializer, topology mismatch. Signing
  authenticates their malformed bytes so semantic rejection reaches the parser.
* Add descriptor/request/tensor size/version/enum/rank/dimension/byte-count
  mutations, UINT32/UINT64 boundaries, normalization mismatch, nonfinite/range
  values including -Inf, null and invalid overlap buffers; no clamping.
* Inject invalid returned output before final validation: NaN, Inf, negative,
  above one, and invalid LAST element. No partial output publishes; reservation
  returns to baseline and subsequent valid execution succeeds.
* Add malformed/zero/wrong-kind/cross-context/released handles, serial exhaustion
  without wrap, no reuse after release, context destroyed before live result
  access, checked allocation sums, maximum dimension rejection and resource
  restoration. Existing actual allocator/backend faults remain retained.
* Concurrent cancellation/retirement at final publication supplements retained
  pre-cancel, bounded checkpoint and completed-GPU-tile tests. Both lock orderings
  are accepted; neither permits successful revoked output. Destruction remains
  quiescent as required by the owner contract.
* Retain context ownership/accounting isolation and bounded interleaving. Retain
  epoch N -> N+1 revocation, latest successful admission generation, BUSY without
  queue and Phase 4 exact recovery. UNKNOWN/NORMAL/ELEVATED/SEVERE/CRITICAL and
  synthetic UNKNOWN/VALIDATED/CERTIFIED remain in the 180-row matrix.
* Started synthetic accelerator failures now cover INTERNAL, unavailable,
  RESOURCE, CANCELLED and OUTPUT_INVALID (six injections including the retained
  INTERNAL adapter). Separate CPU execution counter proves ZERO CPU replay for
  each, one accelerator call, no output. Valid later requests recover.
* Pre-start unavailable/trust/capability/model/resource fallback and security/
  malformed/stale rejection remain retained. Production-descriptor native-platform
  integration selects CPU automatically despite otherwise eligible GPU facts;
  synthetic trust adapters do not become release evidence.

## Fixed shader, boundary and resource audit

Model/package bytes remain parsed weights/metadata only, never shader/SPIR-V/
Metal/AIR/metallib/machine code. Fixed source identities (Git blobs):
Vulkan segmentation.comp `024640f1b836b8cc14b00a7026f55c426bdc5852`;
Metal segmentation.metal `6937ee48ba567d5998b618d484f026b805ae40ac`.
Pinned NDK 28.2.13676358 glslc and SPIR-V arithmetic inspection are retained.
Apple fixed embedding, Metal 2.4 safe math and all SDK compile gates are retained;
no runtime compilation of untrusted model code and no new shader signing system.

CPU Step 2 limits unchanged. Vulkan tile/reservation remains <=16384 pixels /
1638400 transient bytes; Metal <=16384 / 1114112. Step 6 reservation accounting,
signed memory floor and all hard ceilings unchanged. Integer checks precede
allocation. Driver-private overhead is opaque, not an RSS guarantee. Cancellation
is cooperative; no wall-clock preemption guarantee.

No shipping C ABI, JNI or Swift facade changed. The Android certification JNI
entry is argument-free/debug-only, invoking shared native ownership rather than
unchecked Java tensor buffers. Swift detached calls use that same checked suite;
there is no new Swift-owned native pointer or domain implementation. Additional
faults/counters are under PIXAURA_CERTIFICATION_TESTING only; production object
scans now also reject PXSEC, the suite symbol and new fault/counter markers.

Output-count/truncated readback evidence is classified carefully: GPU interfaces
use fixed privately allocated capacities and graph-derived counts; existing
injected completion/readback failures plus invalid-last-element tests prove
rejection/cleanup. This is not observation of a physical short read or real device
loss. No unavailable backend counts as a parity PASS.

## Validation ledger

Initial audit: focused source 8/8 and corrected Linux full source 136/136 PASS,
zero skips. Earlier Windows POSIX-harness and incomplete-local-PATH attempts failed
and were not counted as PASS. New certification source tests 2/2 PASS.

Final local affected gates PASS: Windows existing Zig compiler fallback 9/9,
Linux native 9/9, ASan/UBSan 11/11 including negative probes (actual exits 86/87),
full Linux source 138/138 with zero skips, actionlint PASS, diff whitespace PASS.
CPU PXSEC: 105 vectors, 10,159,065 values, 36,727,185 checks, zero failures;
admission: 180 rows, 3749 checks, zero failures; package mutations: 96, zero
failures. Logs are under build/phase5-step7-{windows,linux,sanitize}-final.log
and build/phase5-step7-source-final.log. Hosted platform/full retained execution
and both complete same-final-SHA workflows still govern acceptance. Final delivery
supplies observed SHA/run IDs/counts; no prior SHA or pending job satisfies it.

One expanded full-source attempt returned 137/138: the retained stale-transport
fixture's bash subprocess exceeded its unchanged timeout while native/sanitizer
work ran concurrently on the mounted workspace. Its failure log is retained;
the independent rerun passed 138/138. No timeout or assertion was weakened.

During implementation, a new test attempted to reinitialize destroyed caller
storage without zeroing it. Existing lifecycle validation correctly rejected it;
the fixture now zeroes storage and supplies a fresh identity before reinit.
Production lifecycle checks were preserved.

## Files and findings

Changed production compilation units contain only test-conditional hooks/counters:
`inference.cpp`, `inference_admission.hpp`. Shared test additions are
`inference_certification_boundary.hpp`, deterministic generated certification
fixtures and generator, retained CPU/Android/Apple/admission/model-package test
extensions, source tests and engineering JSON record. Release-object scanner and
workflow source-test lists retain isolation/execute the new checks. Contract,
roadmap/quality/architecture/decision entries record Option A; no ADR supersession
or new promotion/signing architecture is needed.

A: complete current software/security evidence and all exact-SHA gates; acceptance
requires zero unresolved A failures. B: production model/runtime/operator work and
existing deferred physical certification. C: optional tuning/additional hardware
measurements. Only A is implemented. No architecture blocker after Option A.

Production model remains UNRESOLVED. U2NETP remains AUDITED, UNSUPPORTED,
UNAPPROVED, UNINCORPORATED. Frozen provider profile, normalization, CPU arithmetic
and Schema 3 unchanged. No weights, external runtime, network inference, telemetry
or AI UX. DH-APPLE-METAL-01 unchanged. Step 8 NOT STARTED; Phase 6 NOT STARTED.
STOP AFTER STEP 7.

## Exact file scope

* `.gitattributes`
* `.github/workflows/foundation.yml`
* `.github/workflows/native-shells.yml`
* `ARCHITECTURE.md`
* `DECISIONS.md`
* `docs/contracts/inference-provider-certification-v1.md`
* `docs/plans/phase-5-roadmap.md`
* `docs/reports/phase-5-step-7.md`
* `packages/core/src/inference.cpp`
* `packages/core/src/inference_admission.hpp`
* `packages/core/tests/inference_admission_boundary.hpp`
* `packages/core/tests/inference_android_boundary.hpp`
* `packages/core/tests/inference_apple_boundary.hpp`
* `packages/core/tests/inference_boundary.hpp`
* `packages/core/tests/inference_certification_boundary.hpp`
* `packages/core/tests/inference_certification_fixtures.hpp`
* `packages/core/tests/model_package_boundary.hpp`
* `QUALITY_GATES.md`
* `ROADMAP.md`
* `scripts/check-model-release.mjs`
* `scripts/inference-certification-fixtures.mjs`
* `scripts/inference-fixtures.mjs`
* `tests/fixtures/inference-certification-v1.json`
* `tests/inference-certification.test.mjs`
