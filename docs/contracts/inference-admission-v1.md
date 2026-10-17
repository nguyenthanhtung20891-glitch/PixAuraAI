# Capability-aware inference admission v1

Scope: Phase 5 Step 6 only. Shared native authority, local/offline binary
segmentation; production model selection UNRESOLVED. No operator, package,
normalization, provider arithmetic, Schema 3 or AI UX changes.

## Authority and trust

`inference_admission.h` is an additive version-1 checked C ABI. Configure once
from the existing trusted platform capability service's immutable normalized
profile and observations. This is the same internal adapter boundary as Phase 4,
not an API for external/diagnostic profile loading. Kotlin/Swift do not derive
tiers, select providers, supply resource estimates, or promote provider trust.
Native build platform must match configured hardware platform. Private synthetic
platform/trust adapters exist ONLY under PIXAURA_CERTIFICATION_TESTING; production
objects are scanned for their absence. No package-supplied key/profile or trust
record becomes authority. Step 1 signature/license/digest verification is mandatory.

The existing Phase 4 `policy::State/configure/update` engine is reused without
modification, including T0/T1/T2 derivation, material epochs, unknown handling,
componentwise downgrade, and three-sequence normal recovery. Inference consumes
its strategy directly; it never disguises inference as a raster work request.
No new device score, dynamic thresholds, recovery counter or global scheduler.

CPU reference is eligible after exact-profile validation and resource admission.
Acceleration requires BOTH the reviewed inference provider descriptor validation
>= RUNTIME_VALIDATED and Phase 4 permitting the matching backend. CPU Vulkan/Metal
availability is never inferred from device model, package compatibility or timing.
Provider CERTIFIED may be stronger metadata, but is not required. Static platform
GPU validation alone does not validate an inference provider. The actual Vulkan
and Metal descriptors remain UNKNOWN and cannot be promoted by this API: current
automatic production execution is therefore CPU. Explicit provider execution is
retained separately for tests/engineering. Step 7 may establish reviewed provider
validation later; no Step 7 certification is invented here.

## Deterministic order and failures

Android priority: android.vulkan.v1 then cpu.reference.v1. Apple priority:
apple.metal.v1 then cpu.reference.v1. Portable host: CPU only. No cross-platform
backend routing, plugin scan, exploration, benchmark or hidden mutable heuristic.
All providers retain the exact frozen pixaura.onnx.reference-segmentation.v1
profile. CPU preparation first verifies full compatibility. An unsupported profile
cannot fall back into support. Exact tensor shape/type/bytes/normalization and
finite [0,1] inputs validate before selection; no clamping or reduced dimensions.
Authenticated minimum tier and logical CPU-thread availability are mandatory;
UNKNOWN cannot satisfy a declared minimum. Required validation applies to the
selected provider's authoritative descriptor (in addition to the automatic GPU
minimum). Budget class and optional accelerator preference remain signed advisory
labels, not a second tier table or permission to bypass fixed priority. Retaining
these already signed declarations in the private immutable binding changes no
manifest/canonical bytes, signature, public package ABI or parser acceptance.

Fallback precedence: unavailable -> insufficient provider trust -> Phase 4 backend
disallowed -> accelerated model incompatibility -> bounded resource non-fit.
Only pre-execution fallback is allowed. Model preparation is bounded parsing/
metadata binding, not provider execution or shader dispatch. Unexpected INTERNAL,
invalid model/security, malformed caller input, stale/cross-context handle,
cancellation or arithmetic overflow rejects. Backend prepare OOM may reject
RESOURCE directly; no allocation retry is promised. Both providers non-fitting
reject RESOURCE. Context-wide BUSY rejects BUSY; neither a CPU retry nor a queue
is useful because the existing single execution/pending slot applies to all
providers. No background workers or persistent cache.

| Primary condition (before execution) | CPU eligible | Outcome |
| --- | --- | --- |
| Available, validated, permitted, compatible, fits | yes | Primary |
| Unavailable | yes | CPU / FALLBACK_UNAVAILABLE |
| UNKNOWN provider validation | yes | CPU / FALLBACK_TRUST |
| Phase 4 disallows backend | yes | CPU / FALLBACK_CAPABILITY |
| Primary rejects exact supported model requirements | yes | CPU / FALLBACK_MODEL |
| Primary reservation does not fit | yes | CPU / FALLBACK_RESOURCE |
| Neither reservation fits | no | RESOURCE |
| Context pending/running | either | BUSY; no queue or replay |
| Invalid/unverified/stale/cancelled request | either | Reject; no fallback |
| Any failure after provider start | either | Return failure; no replay |

Accelerated transient reservations are nonnegative additions to CPU's otherwise
identical tensor/model floor. Accelerated resource fit with CPU resource non-fit
is therefore impossible for current providers; tests do not fabricate that case.
A stricter signed trust requirement can exclude CPU while a synthetic certified
test backend remains eligible.

Reason enum in header is frozen in order, starting zero: PRIMARY, CPU_ONLY,
FALLBACK_UNAVAILABLE, FALLBACK_TRUST, FALLBACK_CAPABILITY, FALLBACK_RESOURCE,
FALLBACK_MODEL, REJECT_INPUT, REJECT_MODEL, REJECT_RESOURCE, REJECT_CANCELLED,
REJECT_STALE, REJECT_BUSY, REJECT_UNCONFIGURED, REJECT_SECURITY, REJECT_TRUST. Result carries
version/size, admitted flag, inference status, reason, tier, epoch, generation,
reservation/peak/cap, provider ID and manifest digest. Raw platform errors never
escape. ABI/pointer errors preserve outputs; ordinary policy rejections preserve
the handle and write bounded result. Allocation failure may preserve both.
Execution status values from Step 2 are unchanged. Policy configure/update/query
return the existing Phase 4 codes, explicitly separate from inference codes.

## Ownership and resource admission

Initialize inference context -> verify package -> configure policy -> admit ->
execute admitted once -> check current publication eligibility -> release output.
Kind-4 plan is an opaque context identity/serial token in the SAME existing
registry, not a caller-modifiable authority or prepared-model handle. It owns
immutable verified package/model, copied validated caller input, exact request,
epoch, selected backend and native reservations. Package release cannot substitute
bytes. Increasing successful admission generations are context-wide and nonzero.
Rejected admissions do not consume a generation. One pending OR running admission
per context, no unbounded request table. Explicit execution returns BUSY while an
admission is pending/running; otherwise direct engineering calls retain their
old semantics. External borrowed/copy buffers remain the caller owner's budget
responsibility, as in Step 2; native accounting is not a total-process RSS claim.

Under the context mutex, all needed leases are charged before the plan is returned:
8192 bytes conservative model/plan/preparation control, exact private F32 input,
exact F32 output and selected backend's own transient_bytes() estimate. No second
resource table duplicates provider requirements. Existing package/control/output
occupancy remains charged. The existing 128 KiB context control plus 896 KiB
unusable safety room retains Phase 4's 1 MiB reserve. Effective lease ceiling is
max(0, Phase 4 runtime cap minus 896 KiB), never above context/hard ceilings.
Checked addition and the actual native occupancy prevent overcommit/overflow.
The signed declared peak memory is a conservative floor for the combined new
model/request reservation: reserve max(8192 + input + output + backend, declared
peak). It never substitutes for a larger native estimate or already retained
package/control occupancy. Synthetic fixtures declare 16 MiB, so admission
reserves at least that much even for tiny tensors; this is not measured RSS.
The reservation check/allocation and dynamic cap publication share a small resource
mutex; cap shrink cannot race a stale check/CAS. No lock is held over GPU dispatch.

Unused input/backend reservation stays conservatively charged with a completed
output until output release. This is deliberate bounded accounting, not measured
RSS, driver memory or a cache. Released/stale/double-release/cross-context tokens
reject through the existing registry. Stop/quiesce/join precedes context destroy.
Independent contexts have independent reservations. No shared global device budget
authority exists today; this does NOT promise a global cross-context RAM allocator.
Trusted adapters must partition truthful per-context budgets at that boundary.

## Epochs, cancellation and publication

Admit and prepare are one serialized operation bound to the current epoch.
Material update immediately publishes the lower cap and revokes outstanding
admission models. Pre-start stale plan returns existing STALE_HANDLE(14); caller
must release and re-admit, never silently execute with the old larger ceiling.
Recovery epoch changes also revoke plans. Nonmaterial observation/recovery count
does not churn epochs. Same sequence identity/conflict and overflow retain Phase 4
rules. Epoch exhaustion stops new allocation/admission and revokes pending work.

After execution starts, checkpoints detect revocation/cancellation; submitted GPU
work retains existing synchronous completion/cleanup ownership before reclamation.
Final registration checks epoch, retirement and cancellation under the same context
mutex as updates. No partial/stale/cancelled result publishes. Pre-cancel or a
generation-owned plan cancellation stops execution, never selects a second backend.
Every started outcome consumes the one-shot token. A stale/pre-cancelled unstarted
plan remains owned until explicit release. No post-start replay, including INTERNAL,
device loss, OUTPUT_INVALID, RESOURCE or cancellation. A failure never mutates
verified package, CPU provider or document/history state.

`admission_current` checks an output's internal bound epoch AND latest successfully
admitted request generation under that mutex. A later admission revokes an old
output's eligibility even if capability epoch is unchanged.
Consumers must hold their established owner lock across admission/cancellation,
capability update, eligibility check and installation, retaining their own current
request/cancellation fence. This is eligibility, not a new display owner.
Old immutable outputs remain readable after later revocation, matching retained
Step 2/Phase 4 behavior. Background/constrained and thermal/pressure changes use
the existing Phase 4 observations and recovery; no special inference hysteresis.

No plan/provider/epoch/pressure state is persisted. No inference profile expansion,
new runtime/model, production trust upgrade or physical certification. Hosted and
simulator results are integration evidence. DH-APPLE-METAL-01 remains DEFERRED.
Step 7 and Phase 6 NOT STARTED. Stop after Step 6.
