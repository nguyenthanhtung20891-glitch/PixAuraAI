# Phase 5 frozen finite roadmap - Local inference

Status: FROZEN; eight steps only. Phases 0-4 and Steps 1-4 remain accepted.
Step 1: `8d5a30338c97a05e987ecf1324b86a2afb795356`.
Step 2: `58202caf1fd86215abe4650b208b2134cdca9b58` (Foundation 38605609607
7/7, shells 38605609516 3/3). Step 3: `bf31eb33d5b5027236d71cf56da4c648d6b56255`
(Foundation 38685139496 7/7, shells 38685139454 3/3).
Step 4 COMPLETE / FULL PASS at `a3ad7e7a61030992d933b3ca68572f83604dd5a9`
(Foundation 38730904780 7/7, shells 38730904823 3/3).
Step 5 COMPLETE / FULL PASS at 59d846967fffef7e735597f6b65301841659336f (Foundation 38999701451 7/7; shells 38999701458 3/3), under approved Option A: bounded synthetic
evaluation; production selection UNRESOLVED. No model/runtime/operator expansion.
Step 6 AUTHORIZED / IN PROGRESS: shared capability-aware admission and pre-execution fallback only. Steps 7-8 and Phase 6 NOT STARTED.
Eight steps only. Step 9+, Phase 6 and the Editing Agent require new authorization.

| Step | Scope | Exit evidence |
| --- | --- | --- |
| 1 Model Package & Manifest Contract | Signed, bounded local segmentation package verification; immutable verified payload ownership; synthetic fixtures; checked shared boundary | Reject malformed/untrusted/tampered packages; native/platform/security gates and both complete exact-SHA workflows |
| 2 Provider Abstraction & CPU Reference Provider | Explicit load/infer/cancel/release; approved dependency-free narrow genuine ONNX CPU reference profile; bounded tensors/operators and offline execution | CPU correctness, cancellation, lifetime and allocation failure; no platform policy duplication |
| 3 Android Local Inference Provider | Approved android.vulkan.v1 exact-profile execution, shared CPU authority, bounded existing Vulkan ownership | Offline execution, API/ABI checks, cancellation and CPU fallback; no NNAPI foundation |
| 4 Apple Local Inference Provider | Approved apple.metal.v1 exact-profile execution; bounded reused Metal worker; shared CPU authority | Swift/C parity, unsupported operators and provider failure; simulator distinct from hardware evidence |
| 5 Compact Segmentation Evaluation | Approved Option A: bounded metrics/synthetic evaluation, candidate/license/profile audit; production model selection may remain UNRESOLVED | Exact metric semantics/goldens, reproducible synthetic results, timing/resource evidence, accurate candidate gaps; no unapproved weights/runtime/profile expansion or UI |
| 6 Capability-Aware Admission & Fallback | Shared T0/T1/T2, truthful occupancy, availability/validation, thermal/pressure; pre-execution CPU fallback only | Unknown/pressure/failure rejection, deterministic fallback, retained manual editing and no raised Phase 4 ceilings |
| 7 Provider Parity & Security Certification | CPU/provider golden corpus, package tampering/races, operator/quantization parity and security | Native/JNI/Swift parity; sanitizer and lifetime/security corpora; evidence classification |
| 8 Phase 5 Closure | Finite audit and retained regression certification | Both complete exact-final-SHA workflows, A/B/C classification and explicit stop before Phase 6 |

ADR 0005 retains ONNX interchange, CPU/XNNPACK fallback, evaluated Core ML and
optional converted LiteRT artifacts. Provider choices are not new commitments
to unreviewed dependencies. No remote downloads, network inference, model
marketplace, generative/text/chat tasks, telemetry, planner/advisor/operator or AI UX.

Step 1 does not execute models or change document Schema 3. Inference requirements
are declarations only; Step 6 owns their later admission integration. Existing
dynamic policy is coupled to decode/PRV1 contexts and is not an inference scheduler.
No inference request may be disguised as an existing preview/import request.

Ed25519/shared authority approved: [ADR 0022](../adr/0022-model-package-ed25519-authority.md).
Dependency audit accepted: libsodium 1.0.22 (ISC), pinned official archive and
exact source checksums. [Frozen model package contract](../contracts/model-package-v1.md).
No production secrets or production model weights required.
DH-APPLE-METAL-01 remains unchanged and DEFERRED at its existing later gates.
