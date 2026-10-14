# Provider-neutral inference v1

Status: FROZEN for authorized Phase 5 Step 2. Binary segmentation only.
Shared native owns tensor/status/normalization/ownership/publication semantics.
Static local discovery contains `cpu.reference.v1` version 1, CPU/reference,
deterministic, available without GPU, profile `pixaura.onnx.reference-segmentation.v1`.
Validation is reference-corpus validation, not physical acceleration certification.

Versioned C structs carry explicit sizes, reserved zeros, checked enums and bounded
buffers. Caller pointers must be truthful, accessible, aligned where required and
nonoverlapping; arbitrary invalid C addresses cannot be made safe. Inputs remain
stable for the synchronous call; execution copies validated input before processing.
No exceptions/STL/vendor errors cross the ABI. Failures preserve output handles.

ProviderDescriptor/InferenceCapabilities map to the descriptor's task/type/layout,
profile/availability/validation and resource fields. InferenceInput is a tensor
descriptor plus immutable borrowed bytes; InferenceRequest adds task/generation/
pre-cancel. InferenceOutput is an owned result token plus tensor/generation/model
identity metadata; InferenceStatus is the stable enum. ContextHandle is explicit
caller storage, with package/model/output token kinds. No CPU-specific request
fields: shared code validates and normalizes before invoking a prepared backend.
The bounded static provider registry and prepared-backend interface have one CPU
implementation; no plugin discovery or untrusted dynamic-library loading.

## Ownership and lifecycle

Caller-owned zeroed context storage, fresh 32-byte identity per lifetime, bounded
non-pointer serial handles following ADR 0009. No global mutable registry. Context
init/destroy require exclusive access; live calls synchronize internal ownership.
Verification entry delegates to Step 1; only fully verified immutable snapshots
enter the context. Execution accepts prepared model tokens, never paths/raw ONNX.
No caller trust injection, unsigned fallback or release test anchors.

Lifecycle: initialize context -> verify package -> prepare explicit provider ->
query authenticated model metadata -> execute -> copy/query owned output ->
release output/model/package -> destroy. Model query provides signed input shapes/
normalization, validated output shapes, manifest digest and required input+output
execution bytes; callers do not parse untrusted manifests to construct requests.
Releasing a package after preparation is safe: the prepared model retains immutable
ownership. Acquired snapshots survive concurrent release; retired models cannot
publish later results. Stale, wrong-kind, cross-context and double-release tokens
reject. Destroy invalidates all tokens; storage/identity cannot be reused improperly.

Independent contexts operate independently; at most one execution per context.
No worker pool, daemon, persistent cache or document/history changes. Handles <=32;
context ceiling 128 KiB..256 MiB includes 128 KiB control reservation, all retained
package data, prepared metadata and owned input/output capacities. No allocation
eviction or raised Phase 4 ceiling. Resource accounting is local ownership, not
duplicated tier/pressure/admission policy; Step 6 retains that authority.
Borrowed caller input/output-copy buffers remain the caller's responsibility;
the context counts its private input copy and retained output, not external memory.

Requests carry increasing nonzero generations. Pre-cancel returns CANCELLED and
publishes nothing. A generation-qualified cancel request signals only matching
active work; checks at admission, every 1024 pixels and publication save work.
Release revokes model publication. Cancellation never mutates package/weights;
later requests remain valid. Cancellation after completed publication cannot revoke
an already returned immutable output; consumers must also fence their own current
request/generation before display. No implicit project or AI approval authority.

## Tensors and statuses

Input is F32 NCHW `[1,3,H,W]`, exact signed shape/bytes, finite RGB in [0,1].
Request normalization must exactly match signed scales and zero biases. Output
is owned F32 NCHW `[1,1,H,W]`, exact bytes, finite probabilities in [0,1].
Validate full output before publishing its handle; output copy validates capacities
and overlap before writing, and never executes a model. Invalid output rejects.
Output tensor normalization slots are zero (not applicable to probabilities).
An execution attempt consumes its generation after ABI/task/handle/concurrency
checks, even if tensor validation, cancellation or allocation later fails.

Codes: 0 OK; 1 INVALID_ARGUMENT; 2 UNSUPPORTED_ABI; 3 UNVERIFIED_MODEL;
4 PROVIDER_UNAVAILABLE; 5 MODEL_UNSUPPORTED; 6 MODEL_INVALID; 7 TASK_UNSUPPORTED;
8 INPUT_SHAPE; 9 INPUT_TYPE; 10 INPUT_RANGE; 11 RESOURCE; 12 CANCELLED;
13 OUTPUT_INVALID; 14 STALE_HANDLE; 15 INTERNAL; 16 BUSY;
17 BUFFER_TOO_SMALL; 18 NORMALIZATION. Diagnostics expose bounded numeric codes
only; package-verification failure also returns Step 1's bounded reason. No paths,
payload logs, network, telemetry, signing, acceleration or inference policy.
