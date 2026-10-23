# Assistance Provider & Proposal Envelope v1

Status: implementation certification pending. Contract version 1.
Phase 7 Step 2 only. Step 1 three-mode interaction contract remains frozen.

## Authority and scope

An Assistance Provider is a replaceable, statically constructed source of
UNTRUSTED proposal bytes. It is neither document, validation, approval, execution,
candidate nor history authority. The internal shared C++ interface receives only
bounded explicit machine-semantic request data, never an editor service, mutable
document, approval issuer, execution callback or platform/native handle.
No new public C ABI, JNI or Swift surface is added.

User request -> provider -> untrusted envelope -> trusted envelope validation ->
existing Structured Intent v1/planner or Plan v1 -> Registry v1 binding -> ordinary
Plan v1 -> later review and explicit HUMAN approval -> existing Phase 6 candidate
execution/publication -> ordinary non-destructive revision/history/undo/redo.
Advice terminates before Plan output. Nothing here approves or executes.
This is an in-process capability boundary, not a sandbox against malicious native
code already possessing host privileges.

## Envelope grammar and canonical encoding

Exactly eight fields; every field is mandatory and unknown fields reject:

```json
{"binding":B,"envelope_version":1,"kind":K,"payload":P,"proposal_id":"proposal.1","provider_id":"reference.assistance.v1","provider_version":1,"registry_version":1}
```

The displayed B/K/P are grammar metavariables, not literal accepted JSON.
The actual bytes use the existing native document Parser/Writer: UTF-8, sorted
object keys, exact integers, preserved action order, minimal escaping, terminal
LF. No BOM, trailing data, duplicate keys, floating point, exponent, negative
zero, malformed UTF-8 or noncanonical input. No Unicode normalization or repair.
No free-form explanation/summary is needed in v1; no hidden reasoning field.
No digest is necessary: exact canonical bytes are inspectable and not authority.

| Kind K | Payload P | Trusted validation result |
| --- | --- | --- |
| 1 ADVICE | Exact Structured Intent v1 object, one explicit existing tool suggestion | Validate existing goal/parameters/binding; return no Plan |
| 2 STRUCTURED_INTENT | Exact Structured Intent v1 object | Existing deterministic reference planner produces unchanged canonical Plan v1 |
| 3 PLAN_CANDIDATE | Exact Plan v1 object | Existing Plan parser/parameter validation and Registry v1 binding; unchanged canonical Plan v1 |

The payload and outer binding must both match the trusted caller's live immutable
snapshot/context. K does not authorize reinterpretation of one payload as another.
Unknown kinds/versions reject. Free-form Intent v1 is not interpreted or accepted
as this payload. In particular, “make this photo brighter” has no mapping here.
All eleven existing goal/tool identities and integer contracts remain unchanged.
No clamping, defaults, tool substitution, action reordering, migration or replan.

## Bounds

| Object | Ceiling |
| --- | --- |
| Canonical envelope | 12288 bytes |
| Canonical Plan payload | 8192 bytes / 8 actions |
| Structured/advice payload | 2048 bytes / one goal |
| Canonical binding input | 512 bytes |
| Provider ID and proposal ID | 1..64 ASCII bytes each |
| Explanation/advice list | None; one explicit advice item per envelope |
| In-flight request per provider | One synchronous call, no queue |

IDs consist of nonempty dot-separated segments of lowercase ASCII letters,
digits, underscore or hyphen; no empty segment. Proposal IDs are correlation
labels only. Changing/guessing/repeating a valid ID grants no approval, freshness,
candidate or execution capability. No new native handles or raised ceilings.
The envelope allows the unchanged 8192-byte Plan plus two bindings and small
fixed metadata within a conservative 12 KiB cap; no arbitrary extension object.
The existing parser's finite field sets, array limits and string bound (256)
also apply; this grammar cannot recursively nest itself.

## Binding and trust

B is the exact existing Phase 6 binding: context_id, document_id, generation,
project_id, revision_id, session_id, source_sha256. Identity spelling and integer
range are unchanged. The caller supplies the current snapshot and context, never
accepts a provider's self-asserted document truth, and chooses the expected provider
from trusted static construction. Only reference.assistance.v1/version 1 is
currently registered; unknown providers/capabilities are unsupported.
Provider identity is identification, not provider certification or trust promotion.
Trusted construction checks Identity with check_identity: exact known ID/version
and capability mask; unlisted bits or identities reject, never dynamic discovery.

Envelope validation is not execution admission. An immutable snapshot can later
become stale. Every subsequent trusted application/approval/execution boundary
must use existing live source serial/generation and repository epoch CAS checks.
Manual edit, undo/redo/ABA, source/context/session replacement and mode abandonment
must not be bypassed by holding old validated bytes or a proposal ID. This layer
creates no parallel freshness epoch; its local cancellation counter only fences
provider request completion. Phase 6 remains final publication authority.

## Provider interface and reference behavior

The internal Provider interface declares identity/version/capabilities, synchronous
request, cancel and stop. Capabilities are the finite mask 7 (kinds 1, 2, 3), not
dynamic tool registration. The reference provider wraps only an explicit canonical
payload and binding supplied as data; encoding is NOT semantic validation.
It can therefore return an invalid untrusted candidate which the trusted validator
must reject. No parser of human language, inference, heuristic, random seed,
clock, device dependency, network, worker thread or learned memory exists.
Identical request data produces identical canonical bytes. Provider replacement
must retain this untrusted output boundary, not this reference wrapping algorithm.

## Lifecycle, concurrency and cancellation

Create -> request -> finish, optionally cancel; stop is terminal. Caller ownership
is stop -> join ALL calls -> destroy (ADR 0009). Stop does not join threads.
There are no callbacks, asynchronous tasks or provider-owned document pointers.
Request input remains immutable and alive until the synchronous call returns.

A try-lock admits at most one request. A competing call returns RESOURCE without
queueing. A separate short fence protects stopped state and a nonwrapping
cancellation epoch. Work occurs outside that fence; final return reacquires it.
Cancel/stop winning that fence before result completion yields CANCELLED and no
returned envelope. If completion wins, returned bytes remain untrusted; a later
stop cannot undo a completed call or create authority. Cancellation is nonsticky
for future requests. Epoch exhaustion permanently stops the provider, fail closed.
Lock order is active-request lock then completion fence; cancel/stop only acquire
the fence. No document/repository lock is acquired. Independent instances isolate
lifetime and cancellation. No automatic retry after BUSY, cancellation or failure.
Mode/request ownership and native freshness must be checked again by future
interaction integration; Step 6 owns that platform work.

## Status mapping

Reuse native numeric statuses: 0 SUCCESS; 1 INVALID_ARGUMENT (request, provider,
kind or ID); 2 UNSUPPORTED_ABI/version; 5 UNSUPPORTED_OPERATION/tool; 6
INVALID_PROJECT/malformed proposal; 7 INVALID_PARAMETERS; 8 RESOURCE_LIMIT/BUSY;
9 STALE_BASE; 13 CANCELLED/stopped; 14 INTERNAL_ERROR/provider failure.
No new status enum. Other/unknown nonzero outcomes are failure, never success or
approval. Empty result on failure; no partially accepted envelope/Plan. Allocation
failure maps to RESOURCE. Request-level version/shape checks do not replace the
trusted validator. Provider failure cannot disable the independent manual editor.
Internal Outcome uses an empty optional on failure, so creating the failure result
does not allocate a debug-STL string/vector proxy. Successful values remain bounded.

## Security, privacy and persistence

Untrusted bytes are bounded before parsing. Exact field/type/version, selected
provider, outer/inner binding, existing schema/planner and manual-derived Registry
checks precede acceptance. Native handle/token forgery is inapplicable to this
interface: it issues or accepts no such authority object. Production release scans
retain all test-key/hook checks and reject assistance fixture/fault markers.
No approval/test hook, fixture, privileged reference tool or checkpoint ships.
Deterministic pre-work/pre-completion checkpoints exist only when the standalone
assistance_test target defines PIXAURA_ASSISTANCE_TESTING; release layout and
objects exclude them. They certify cancel/stop/BUSY orderings without timing bets.

No proposal creation, encoding, validation or inspection changes pixels, original,
document, revision/history, approval state or candidate state. No UI, persistence,
proposal tables/files, Schema 3 change, provider handle storage or hidden memory.
Only later ordinarily committed revisions are durable. Local/offline only: no
network permission, cloud, HTTP, analytics, account, secret or API key. Diagnostics
must not log images, user content, raw handles, tokens or native addresses.

Intent v1, Structured Intent v1, Plan v1, Registry v1, reference planner, approval,
candidate and Step 1 interaction semantics remain unchanged. Production model
UNRESOLVED; U2NETP unapproved/unincorporated, rights unresolved. CPU VALIDATED;
Vulkan/Metal UNKNOWN. DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED.
Steps 3-8 and Phase 8 NOT STARTED.
