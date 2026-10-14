# Phase 5 frozen finite roadmap — Local inference

Status: FROZEN under Product Owner's Step 1 dependency authorization. Step 1 accepted at
`8d5a30338c97a05e987ecf1324b86a2afb795356`. Phases 0–4 remain closed.
Step 1 COMPLETE / FULL PASS. Step 2 AUTHORIZED / IN PROGRESS under approved Option A.
Steps 3–8 NOT STARTED.
Eight steps only. Step 9+, Phase 6 and the Editing Agent require new authorization.

| Step | Scope | Exit evidence |
| --- | --- | --- |
| 1 Model Package & Manifest Contract | Signed, bounded local segmentation package verification; immutable verified payload ownership; synthetic fixtures; checked shared boundary | Reject malformed/untrusted/tampered packages; native/platform/security gates and both complete exact-SHA workflows |
| 2 Provider Abstraction & CPU Reference Provider | Explicit load/infer/cancel/release; approved dependency-free narrow genuine ONNX CPU reference profile; bounded tensors/operators and offline execution | CPU correctness, cancellation, lifetime and allocation failure; no platform policy duplication |
| 3 Android Local Inference Provider | Android lifecycle/native CPU integration; optional acceleration only after separate dependency/coverage review | Offline execution, API/ABI checks, cancellation and CPU fallback; no NNAPI foundation |
| 4 Apple Local Inference Provider | Apple lifecycle and evaluated Core ML adapter; shared metadata/CPU fallback | Swift/C parity, unsupported operators and provider failure; simulator distinct from hardware evidence |
| 5 Compact Segmentation Evaluation | Licensed compact model selection; thresholds frozen before selection; boundary/task quality and size/latency/memory evaluation | Approved redistribution/license, quality thresholds and reproducible conversion provenance; no UI |
| 6 Capability-Aware Admission & Fallback | Shared T0/T1/T2, truthful occupancy, availability/validation, thermal/pressure; bounded CPU retry | Unknown/pressure/failure rejection, deterministic fallback, retained manual editing and no raised Phase 4 ceilings |
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
