# Phase 5 Step 1 — model package and manifest foundation

Status: IMPLEMENTED; exact-final-SHA hosted certification governs acceptance.
Only Step 1 authorized. Phases 0–4 remain CLOSED; Steps 2–8 and Phase 6 NOT STARTED.
Baseline: `098ac7890de5bed61b7c4823fd5ea3b1ef82c904`.
[Frozen eight-step roadmap](../plans/phase-5-roadmap.md),
[frozen package contract](../contracts/model-package-v1.md),
[ADR 0022](../adr/0022-model-package-ed25519-authority.md),
[accepted dependency audit](../reviews/phase-5-ed25519-dependency-audit.md),
[dependency/distribution record](../dependencies/libsodium-1.0.22.md).
Exact intended changes: [file manifest](phase-5-step-1-files.txt).

## Audit and resulting implementation

Existing ADR 0005 already selects provider-neutral ONNX interchange, offline
CPU/XNNPACK fallback, evaluated Core ML and optional converted LiteRT. No inference
provider, model package verifier or weights existed. SHA-256 content identity,
no-follow storage primitives, checked C ABI, capability/admission and PRV1 policy
were retained. New model verification has separate ownership and no project DB.

Flat local directory acquisition holds no-follow handles, rejects unsafe names,
symlinks/reparse points/hard links/nonregular entries and oversized counts/sizes,
and copies exact bounded bytes. Canonical binary metadata uses explicit UTF-8,
fixed unsigned big-endian fields and no floating-point/JSON ambiguity. Pure detached
Ed25519 signs exactly those bytes; SHA-256 checks each listed payload. Immutable
owned snapshots publish only after complete trust/file-set/digest/semantic checks.
No durable installer, model graph loading/execution or inference provider added.

Production trust store v1 generation 1 deliberately contains no keys until approved
public-key provisioning. No public API accepts caller-supplied trust anchors.
Key ID, purpose, activation generation and minimum model version support reviewed
rotation/removal without changing package semantics. Trusted key validation uses
libsodium point validation; ED25519_COMPAT absent. Test trust/corpus is compiled
only under certification controls. Node test signing uses public RFC test material;
there is no production private key or runtime signing/key-generation surface.

Model contracts cover binary image segmentation only, F32 RGB input and F32 binary
probability output with bounded shapes/ranges/normalization and explicit labels.
CPU-compatible metadata is mandatory. Provider/version/API/operator and capability
requirements are declarations, not permission to execute or hardware certification.
Actual graph/operator matching belongs to Step 2; task-quality/model selection later.
Model IDs/version + signed manifest/payload digest + trust identity define packages,
not filenames. License/provenance/use/review/attribution fields fail closed.

## Dependency and footprint

libsodium 1.0.22, ISC; official archive SHA-256:
`adbdd8f16149e81ac6078a03aca6fc03b592b89ef7b5ed83841c086191be3349`.
Exact upstream source/license/AUTHORS retained, per-file checksums enforced by
CMake and source gates. Static PIC/hidden portable C, scalar fe25.5 and required
AArch64 NEON initialization closure; no optional ISA/assembly/COMPAT/LTO/native
CPU tuning. Full generic configuration retains point-validation API omitted by
upstream minimal. No vendor source modified to silence warnings. ASan/UBSan covers
vendor and owned parser/trust/verifier. Android asset and Swift package resource
ship ISC notices/AUTHORS. No license UI or other product UI introduced.

Observed source/provenance footprint initially 327 files / 2171122 bytes; approved
source is about 2.1 MiB, not a zero-cost dependency. Linux Debug static archive
1576922 bytes and core .so 9020864 bytes at the first measurement; debug/test
symbols and later layout changes affect these values. Model-footprint JSON records
actual final hosted outputs rather than treating archives as linked deltas.

Approximate Android RelWithDebInfo core link changes against existing historical
Phase 4 cache artifacts (not an exact-SHA size certification): arm64 13216800 →
14574192 bytes (+1357392), ARMv7 10170556 → 11730952 (+1560396), x86_64
13062312 → 14347056 (+1284744). Includes debug-symbol/config differences; not
installed APK/RSS estimates. Final linked outputs and production anchor scans are
recorded in build/CI evidence. Apple linked/archived observations require actual
Apple CI; no unobserved Apple delta or physical hardware claim.

## Validation evidence and gate promotion

Windows native full regression: 34/34 PASS. Linux retained/native full run: 34/35
PASS, one tooling failure (CMake chose Windows node.exe through WSL PATH). Corrected
to native Linux Node; changed model package/C/release-object checks 3/3 PASS.
Sanitizers full then-configured suite 35/35 PASS, including active instrumentation
controls; final changed model/C/release-object checks 3/3 PASS. The union of required
sanitizer cases is 37, but no single local 37/37 full-run claim is made.

Common corpus: 45 deterministic package rows + independent RFC 8032 positive and
negative checks. Native host test adds seven filesystem scenarios; C consumer adds
four ABI/argument/output-preservation checks. Shared in-memory corpus is identical
on native/JNI/Swift. Real compiled production-object scan excludes test anchor,
fixture identity and certification symbol. Actual Android release libraries for
arm64/ARMv7/x86_64 passed scans; dynamic export inspection exposed neither libsodium
crypto functions nor internal verify_owned trust authority.

Windows-applicable source: 92/92 PASS, zero failures/skips. Full Linux source:
128/128 PASS, zero failures/skips, after an earlier overlapping WSL run timed out
in an unchanged Bash fixture. Final evidence uses a separate uncontended log.
Android debug/release/JVM/lint/instrumentation APK build PASS; final native corpus
updates are rebuilt before delivery. Actual instrumentation and Apple execution
remain hosted-only until observed. Local workflow lint and diff check PASS.

Recovery validation preserved the normalization rejection and added explicit invalid
rank, maximum supported tensor, zero-scale, oversized-file and entry-count cases.
Current affected source 16/16 PASS; Windows/Linux/ASan+UBSan package, C consumer and
release isolation 3/3 PASS on each host configuration. Final Android debug/release
and instrumentation APK rebuilding PASS. Rebuilt release core sizes: arm64
14577360, ARMv7 11732808, x86_64 14349776 bytes; all three exclude test anchors and
certification symbols. arm64 dynamic exports expose no vendor crypto/signing or
internal trust API. Apple and instrumented parity must still be observed at the
exact final SHA; local build success does not substitute for those executions.

Initial hosted recovery exposed two build/checkout defects: Windows converted the
generated fixture header to CRLF, and the new C consumer omitted the existing C++
linker policy needed for UBSan's C++ runtime. The header is now pinned to LF and
the C source consumer links with CXX, matching existing consumers. Strict byte
comparison and all sanitizer instrumentation remain active. Upstream vendor
whitespace is preserved through vendor-only Git attributes, never source edits.
These earlier failed candidates cannot certify the corrected final SHA.

Local evidence remains ignored under build/phase5-*; unexecuted gates never PASS.
Before final acceptance BOTH complete workflows must finish SUCCESS on the SAME
final commit containing this implementation/report: Foundation 7/7, application
shells 3/3. The final delivery records the exact SHA/run IDs/job totals after
observation; prior-SHA green cannot certify later changes. This avoids a subsequent
uncertified report/status commit or self-referential SHA amendment. No skipped
required job may promote Step 1. Review exact current failure logs before remediation.

## Classification and stop boundary

A: no unresolved architecture conflict or observed product defect after local
remediation; required final hosted gates pending, so acceptance is not claimed yet.
B: production public-key provisioning and actual licensed-model/task-quality
selection remain their separate later gates. DH-APPLE-METAL-01 unchanged and DEFERRED
at existing hardening/Beta/release/physical-claim boundaries. No physical Apple claim.
C: no additional polish scope added.

Schema 3 unchanged. No model loading/execution, CPU/NNAPI/Core ML inference,
segmentation execution, remote download/inference, AI UX, planner/advisor/operator
or Phase 6 work. Phase 5 Steps 2+ NOT STARTED. STOP after Step 1 certification.
