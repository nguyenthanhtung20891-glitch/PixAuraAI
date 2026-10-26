# Phase 7 Step 7 — remediation round 1

Status: AUTHORIZED / IN PROGRESS / ADMISSION BLOCKED.
Remediation implementation certification pending; Step 7 is not complete.
Starting SHA f2dd3ceac7f9f18bf9033b9e398ac703d491e3c5 verified clean tracked/staged
main == origin/main after fetch. Existing untracked probe report is retained below
as historical evidence. Ignored probe/audit artifacts are preserved.

## Authorized foundation

The Principal Architect approved a separate application/process intelligence owner,
not a change to Phase 4/5 caps or segmentation. The internal resource mechanism has
one lazy runtime/session, one global active request, fail-closed checked accounting,
protected Manual/platform reserves, cancellation, client abandonment suppression,
stop/join/unload and repeated lazy initialization. It is not wired to any app,
production adapter, provider, ABI or document. No production numeric policy exists.
Synthetic adapters exist only in the certification executable, never application
release targets. Clients do not own a heavyweight model. No editor/history API is
included by the owner. Physical device admission remains outstanding.

Offline provenance tooling rejects missing/unknown/noncanonical fields and checks
exact local final artifact names, sizes and hashes. Structural completeness never
certifies evidence truth, licensing or production admission. No weights are fetched.
Candidate-neutral paired quality measurements use the same explicit bounded input
for baseline/candidate, without expected-label exposure. Tiny generated fixtures
prove scoring mechanics only; thresholds remain pending Principal Architect and
no image-quality/model qualification is claimed.

B-01 root cause: standalone Zig script maintained an obsolete explicit source list.
It now delegates to the existing CMake-based Windows GNU gate, preserving canonical
sources, consumers, warnings and retained tests instead of appending one symbol.
B-02: final Android release APK is now scanned after assembly, including packaged
native/DEX/resource bytes and UTF-16 markers. Unreviewed assets/model suffixes,
ambiguous ZIP paths, encryption and oversized content fail closed. Negative ZIP
fixtures prove rejection on each relevant packaged surface. Existing host/iOS
production scanning remains unchanged.

## Findings, admission and boundaries

A-01: additive architecture/mechanism established; no claim that EmbeddingGemma
fits a production resource policy. OS adapters, physical budgets/measurements and
actual runtime accounting remain mandatory before admission.
A-02: provenance gate established; candidate conversion evidence remains incomplete.
B-01/B-02 repairs require the exact-SHA evidence below before remediation is certified.
B future work: approved physical-device policy, complete candidate provenance,
independent quality corpus/thresholds and subsequent separately authorized model
integration. C optional: broader diagnostic/performance reports.
No editing/approval/publication authority defect was introduced or redefined.

EmbeddingGemma 2 remains CANDIDATE ONLY; production model UNRESOLVED. No production
provider, LiteRT dependency, model assets, free-form NLP or cloud inference added.
CPU VALIDATED; Vulkan/Metal UNKNOWN. U2NETP unapproved/unincorporated/rights unresolved.
DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED.
Intent/Structured Intent/Plan/Registry/Envelope/Provider, reference planner,
approval/candidate/stale/publication, Schema 3, shared manual/history, ADR 0009 and
Phase 4/5 trust/resource semantics unchanged. Step 8 and Phase 8 NOT STARTED.

## Certification

Implementation and any subsequent documentation SHA require their own full
Foundation 7/7 and Native shells 3/3 completed/SUCCESS evidence. No Step 7 closure
commit is created in this round. The final engineering receipt records the actual
SHA and observed run IDs without self-referential commit claims. Pending checks
are not PASS. This record will be reconciled with actual local results before push.

## Preserved historical preflight evidence

Qualification: Phase 5 DOES have separately admitted inference contexts. Their
frozen segmentation profile does not admit this candidate. The historical statement
about no inference class applies to Phase 4 raster work classes, not absence of
Phase 5 inference admission. RSS is not exact owned allocation accounting.

# Phase 7 Step 7 — production intelligence admission preflight

Status: **ADMISSION_BLOCKED**. Step 7 is not complete. Production model remains
**UNRESOLVED**. Step 8 and Phase 8 are NOT STARTED.

## Baseline and scope

Clean `main` and `origin/main` were verified at
`f2dd3ceac7f9f18bf9033b9e398ac703d491e3c5`, the certified Step 6 closure.
Its Foundation run 40198995481 completed SUCCESS 7/7 and Native application
shells run 40198995512 completed SUCCESS 3/3 (baseline evidence only).
This investigation acquired public upstream artifacts into ignored
`build/phase7-step7-audit`; nothing was incorporated into production assets.

## Candidate and provenance

Architect-selected candidate: EmbeddingGemma 2, text+vision 440M. No substitute.
Official model: https://huggingface.co/google/embeddinggemma-2,
revision `914f7f89142e33e77833254d9c9b90c3cef7303b`.
The upstream API declares Apache-2.0. Its raw `model.safetensors` is
1,488,915,288 bytes, published LFS SHA-256
`197a32965d4b1105faf060417baa899e193fb73cd401f42ec9295234d5553d79`.
The raw checkpoint was NOT downloaded or locally hash-verified.

The Google AI Edge documentation links the converted distribution:
https://huggingface.co/litert-community/embeddinggemma-2-text-vision-440m-litert-lm,
revision `e301f74d5551b0c2641bd5cb4652a76239d5c5f8`.
Downloaded `embeddinggemma-2-text-vision-440m.litertlm`: **387,710,976 bytes**,
locally verified SHA-256
`92dcbea108899e5d6e30d919b0744f90d9967e80c67a4ab5503ac16d54f62eb0`.
The model card declares Apache-2.0 and int4 QAT text/embedder, int8 QAT vision.
Exact conversion toolchain/reproduction provenance has not been completed;
this is not a completed production licensing/provenance admission.

Runtime probe: `litert-lm-api==0.18.0`, Linux x86_64 wheel
`litert_lm_api-0.18.0-py3-none-manylinux_2_27_x86_64.whl`, 47,051,760 bytes,
locally verified SHA-256
`b64e2cf6d7dcb90ff094b74af595cc5d53faa07e0889f967d15df8d3e696b53c`.
The wheel was extracted locally, not installed globally. Official runtime source
v0.18.0 resolves to `b2f686e2ed4718fb84ec398a61dd59ca0f0aff27`.
Runtime Apache-2.0 license was retrieved. No runtime entered application builds.

## Actual CPU evidence

Environment: Linux WSL2 x86_64, glibc 2.43; CPU backend, one thread,
vision CPU backend, audio backend disabled. The physical container contains text,
vision, embedder and associated weights, with **no audio model sections**.
The SDK nevertheless reports audio support from inherited audio-preprocessor
metadata (confirmed against its source); no model bytes were modified.

A project-generated 256x256 RGB PNG gradient was used solely as a resource probe,
not a quality benchmark. SHA-256:
`242a3632ec44b281443ff13c0642612102585b623101c74a09a61ca070b5a6cf`.
Actual inference returned 768 finite embedding components with L2 norm
1.0000004133093. No editor, approval issuer, candidate or history was accessible.

| Measurement | Observed |
| --- | ---: |
| Engine initialization in image probe | 0.302525 seconds |
| First image embedding in that process | 1.158499 seconds |
| Process peak RSS (`ru_maxrss`) | 657,223,680 bytes |
| `/proc` high-water RSS | 657,956,864 bytes |
| Anonymous resident memory after inference | 278,863,872 bytes |
| File-backed resident memory after inference | 379,092,992 bytes |
| RSS after engine close | 225,345,536 bytes |

A separate initialization-only process previously took 2.541648 seconds.
These are single-process observations, not a latency distribution, sustained-run
benchmark, cold filesystem-cache measurement, mobile footprint, or admission PASS.
Model asset bytes, mapped address space, process RSS and charged owned capacity
are distinct measurements and must not be equated.

## Blocking resource decision — A finding

[Budget admission v1](../contracts/budget-admission-v1.md) freezes a maximum
256 MiB native context capacity and requires truthful simultaneous allocation
accounting, including relevant platform/native work. It defines no inference
work class. The image probe's peak RSS is approximately 627.5 MiB; anonymous
residency alone is approximately 266 MiB, before reserving editor working space.
RSS includes the host process and is NOT a precise native-owned allocation charge.
Consequently this probe does not prove a specific budget-request rejection, but
there is no demonstrated bounded admission path for this runtime under the
existing authority. Shipping an unaccounted separate allocation owner or raising
limits would conceal rather than resolve that gap.

Principal Architect decision required: define/approve an explicit inference
resource ownership/admission strategy and acceptable measured mobile budgets,
while preserving manual editing and the existing editing ceilings. Alternatively,
require a demonstrably compliant artifact/configuration of the same authorized
candidate. This report does not authorize either change or a substitute model.

The existing Phase 5 package limits (64 MiB/file, 128 MiB aggregate) concern its
frozen segmentation package/profile. They have NOT been silently repurposed as
an embedding profile, nor treated as proof that every application asset must fit
that format. Similarly, 388 MB does not by itself establish a mobile store-size
violation. Actual APK/AAB/iOS impact remains unmeasured.

## Work deliberately not claimed

No production provider, admission manifest, prototype/calibration policy or new
normative admission contract was frozen while resource authority is unresolved.
No quality thresholds/results, false-positive/abstention rates, Android/iOS
runtime tests, accelerator tests, release package sizes, or complete retained
Step 7 certification are claimed. No Step 7 SHA/workflow certification exists.
The retained baseline results do not certify this report.

Local evidence is in ignored `build/phase7-step7-audit`: pinned metadata,
`container-sections.json`, `cpu-preflight.json`, `cpu-inference-preflight.json`,
probe scripts and logs. Large artifacts and raw runtime logs are not committed.

## Findings and unchanged boundaries

A: unresolved resource-admission ownership/budget decision above; admission stops.
B: after that decision, complete conversion provenance, actual mobile integration,
quality/calibration benchmarks, lifecycle/security tests, package measurements and
full exact-SHA certification. These are unfinished Step 7 gates, not waived work.
C: optional additional performance characterization after the blocking gate.

No production code, public ABI, test behavior or workflow changed. Model evidence
cannot approve or execute. Intent, Structured Intent, Plan, Registry, Envelope,
Provider authority, Assist/Auto/review/lifecycle contracts, Phase 6 approval and
candidate/publication semantics, Schema 3, shared manual/history, ADR 0009 and
Phase 5 trust remain unchanged. No cloud/network inference, silent runtime model
download, arbitrary model loading or free-form chat/NLP architecture was added.
Public artifact acquisition for this audit is not an application network feature.

Production model remains UNRESOLVED; U2NETP unapproved/unincorporated, rights
unresolved. CPU release trust VALIDATED; Vulkan/Metal UNKNOWN.
DH-APPLE-METAL-01 remains DEFERRED / NOT WAIVED / NOT SATISFIED.
Hosted evidence does not constitute physical certification.

Changed tracked file: `docs/reports/phase-7-step-7.md` only.
Final admission state: **ADMISSION_BLOCKED**. Step 8 and Phase 8 NOT STARTED.

## Round 1 changed-file inventory

- .github/workflows/foundation.yml
- .github/workflows/native-shells.yml
- AGENTS.md
- ARCHITECTURE.md
- CMakeLists.txt
- DECISIONS.md
- docs/contracts/intelligence-runtime-resource-admission-v1.md
- docs/contracts/production-intelligence-provenance-evaluation-v1.md
- docs/plans/phase-7-roadmap.md
- docs/reports/phase-7-step-7.md
- packages/core/Package.swift
- packages/core/src/intelligence_resources.cpp
- packages/core/src/intelligence_resources.hpp
- packages/core/tests/intelligence_resources_test.cpp
- QUALITY_GATES.md
- ROADMAP.md
- scripts/check-android-release.py
- scripts/check-native-zig.ps1
- scripts/evaluate-intelligence.mjs
- scripts/intelligence-admission.mjs
- tests/android-release-scanner.test.py
- tests/intelligence-admission.test.mjs
- tests/native-document.test.mjs
- tests/persistence.test.mjs

## Local validation before certification

Retained full Windows CMake/Zig suite: 53/53 PASS, including preserved core UBSan. Retained active ASan/UBSan
full suite: 55/55 PASS, including negative instrumentation probes; final resource
boundary changes additionally pass 49 assertions under the sanitizer build.
Full Linux source suite after delegation/header correction: 172/172 PASS; added
recorded-output CLI plus final tooling checks: 4/4 PASS. Final exact-SHA CI remains
authoritative for the complete final source set. Existing real release APK scan
passes (12 native libraries, 2 DEX files); negative packaged fixtures reject.
That local APK is prior build evidence, not certification of the new SHA. The new
CI step must scan the freshly assembled final release. actionlint and diff checks PASS.
Initial local failures were root-caused: missing initializer_list include; legitimate
Android baseline-profile assets; authority title placement; old source assertions
expecting handwritten Zig inventories. Fixes preserve canonical CMake/test authority,
allow only exact legitimate baseline-profile formats, and retain all prior gates.
No test skip/optionalization, sanitizer suppression or rerun-until-green.

No remediation closure commit or Step 7 admission is claimed here. Actual implementation
SHA and workflow observations are recorded by the final engineering delivery.

## Round 1 cleanup remediation

Initial implementation: 787cf45c0c8a2d66f3fab96ded011d05c7a02e26. Foundation run
40262829420 completed SUCCESS 7/7, attempt 1. Its Native shells run 40262829381
was still in progress when final review found a new foundation cleanup defect.
These results do not certify the correction below.

Cancellation or stop could overwrite RESOURCE/FAILED status before cleanup tested
that status, retaining a runtime whose run had failed. A deterministic rendezvous
now cancels an admitted request immediately before its injected allocation failure.
The test failed against the initial implementation (session destruction missing).
The correction retains execution_failed independently of final status precedence,
so cancellation may win the reported outcome while failed sessions are destroyed.
No frozen approval/candidate/edit semantics are involved or changed. The focused
ASan/UBSan resource corpus now passes 51 assertions; focused source checks pass 8/8.
The new correction SHA requires both complete workflows independently, with no
rerun of the old SHA and no old-SHA substitution. Actual SHA/run IDs are recorded
in final engineering delivery. Step 7 admission remains blocked.
