# Editing approval and execution v1

FROZEN Phase 6 Step 5. Additive C family 1 in `pixaura/agent_execution.h`. Plan v1,
Intent v1, Structured Intent v1, Registry v1 and all editing semantics are unchanged.

## Authority

Plan validity belongs to core Step 2 parsing and Step 3 tool binding. Approval
belongs to explicit human action represented by trusted application code calling
`pixaura_editor_agent_approve`. Execution belongs to the existing editing engine.
No planner receives native context storage, approval APIs, filesystem or network
authority. As with the existing manual boundary, this is not a sandbox against
malicious native host code; a native call cannot prove a physical button press.
The trusted application must invoke approval only for explicit human confirmation.
JSON flags, actor strings, model confidence and timers cannot mint approval.

The existing `pixaura_editor` application service remains the sole live-session,
durability and publication owner. No caller-supplied live document handle is
accepted by this bridge. The service derives binding from its actual live state
and repeats canonical parse, schema/version, binding, exact parameter and trusted
registry checks. Validate returns exact canonical Plan v1 bytes for review and
does not approve. Approve repeats validation and does not render or mutate history.
Approval eligibility also preflights the full stack using the existing evaluator's
geometry/reservation calculation and authoritative Phase 4 `policy_admit`, with
actual native retained occupancy. A trusted configured policy is required; the
legacy manual-only service remains usable without one. No model-supplied profile
or duplicate thermal/tier/recovery/epoch policy is accepted. Execution repeats
admission under the same live-session owner lock before consuming approval.
Unknown fields, noncanonical bytes, unknown versions/tools and invalid values
reject. No clamp, substitution, implicit version upgrade or hidden replanning.

## Identity and ephemeral ownership

Approval identity is SHA-256 (existing FIPS 180-4 implementation) over every exact
canonical Plan v1 byte, including the terminal LF, with no prefix or suffix.
The bytes already include context, project, document, revision, session, generation
and source SHA-256. Execution requires full-byte equality, not digest-only trust.
The complete plan is revalidated on execution. Reordering or modifying a plan,
changing its binding or moving to another context never inherits approval.

Approval is a typed 56-byte token: family/version/size, fixed kind, reserved zero,
32-byte owning editor lifetime identity and core-minted nonzero serial. Neither
caller nor model chooses serials. An internal ordinary staged-proposal lease
reserves the existing combined 64-handle budget and nonwrapping serial allocator.
At most eight approvals, further constrained by the existing eight-proposal limit.
No new budget is added. Each approval retains <=8192 canonical plan bytes, a
64-byte digest and fixed metadata; its existing staging lease retains <=8192
operation-envelope bytes. Both sets are bounded to eight records/context.
Approval tokens cannot act as document, gesture or staged-proposal tokens.

LIVE -> REVOKED or CONSUMED; neither returns to LIVE. Revoke is idempotent and
side-effect free. Explicit release removes the lease and record; double release,
zero/random/wrong-kind/released/cross-context tokens reject 3. A live token is
stale after any replacement of the exact source live snapshot, including undo/
redo ABA. Stop denies execution; quiescent destroy releases all approval state.
No approval or capability snapshot is persisted in Schema 3.

## Consumption, execution and atomicity

Pre-execution validation failure, pre-cancel, stale binding, or cancellation
observed at admission leaves approval unconsumed. After those checks, under the
existing service owner lock, LIVE becomes CONSUMED before the first execution
allocation or tool work. Every subsequent failure leaves it consumed, including
resource, evaluation, cancellation and storage failure. There is no retry or
alternate recipe; another attempt needs fresh explicit human approval. A revoked
or consumed approval returns NOT_APPROVED=15. A missing approval also returns 15.

The service assigns ordinary fresh operation/revision IDs using its existing
identity allocator. It appends all 1..8 actions in canonical order to the existing
current stack, preserving repeated tools and exact integer parameters. Existing
`working_evaluate_cancel` evaluates the complete stack privately from the immutable
original using existing geometry/tone/detail implementations and reservations.
No additional retained pixel copy is introduced: original plus one private result,
with existing admitted scratch and raster limits. Existing document transition
accepts one DetachedCandidate, followed by the existing serialize/validate commit,
repository epoch compare-and-swap, and `Service::accept` publication path. There
is exactly one new revision, no intermediate history, and no change to original.
The existing `ai` actor and 32-hex plan attribution field are reused; attribution
is the first 32 hex characters of the full plan digest and grants no authority.
Full canonical bytes remain the approval match. No persistence fields are added.

Every private failure preserves prior current revision and history. Storage
failure follows the existing repository atomicity and epoch contract; no private
candidate is published. Successful commits participate in the same undo/redo,
branch-after-undo, durable restart and manual history semantics. No hidden AI
history, history labels, execution callback, planner call or post-start replay.

Execution uses try-lock admission: BUSY maps to existing RESOURCE=8 without an
internal queue. Approval, revocation, release, manual edits and execution admission
serialize admission/publication under the existing owner mutex, so exactly one
concurrent authority wins. Once execution wins admission, revocation/release cannot
reverse consumption or invalidate its private copied state. If they win first,
execution rejects. Step 6 [candidate lifecycle](editing-candidate-lifecycle-v1.md)
clarifies private evaluation outside the owner mutex and source recheck before
publication; approval authority and consumption are unchanged. Context destruction remains
exclusive stop -> join all calls (including cancel) -> destroy under ADR 0009.

Cancellation is nonsticky for future execution, using a bounded nonwrapping epoch
and the existing raster cancellation token. It can signal while execution owns
the service lock. Admission captures an epoch; cancellation before publication
rejects. Cancel and durable publication share a separate short authority fence:
cancel winning the fence prevents commit, publication winning it completes the
already-approved atomic commit. A later cancel does not undo a completed commit.
Epoch exhaustion fails closed. Advanced candidate orchestration remains Step 6.

## ABI and evidence

Requests are 32 bytes: api_version=1, exact struct_size, registry_version=1,
boolean pre_cancel and four reserved zeros. Success receipt is 144 bytes:
version/size, consumed=1, reserved zero, 64-byte plan digest, 32-byte source and
resulting revision IDs. All outputs are caller owned. Every range is checked for
null/count, alignment, uintptr overflow and pairwise overlap, including context,
request, token, plan and output. Accessible truthful stable memory remains the
C caller obligation. All exceptions map to stable shared statuses at the C boundary.
Success alone writes approval/receipt. Inspection query/short-buffer returns 11
and required size without partial bytes; other failures preserve all outputs.

Statuses retain 0 OK; 1 invalid argument; 2 unsupported ABI/version; 3 invalid
handle; 5 unsupported tool; 6 malformed/noncanonical; 7 invalid parameters;
8 resource/busy; 9 stale; 11 short buffer; 12 IO; 13 cancelled; 14 internal;
15 NOT_APPROVED. Revoked/consumed are authority state, not new status codes.

Certify single/multi/eight-action execution, action order, exact history, forgery,
single use, stale/cross-context isolation, revocation/release/execution races,
cancellation admission/publication, later-action failure rollback, and manual
undo/redo/branch regression. Test observers and injected failures are compiled
only for certification; actual production object scans must prove absence.
Retain Phase 0–5 and Steps 1–4, all native hosts, Android ABIs, app gates, active
ASan/UBSan and both complete final-SHA workflows. No AI UX, NLP/model, runtime,
weights, network or Phase 5 change. Production model UNRESOLVED.
DH-APPLE-METAL-01 DEFERRED, NOT WAIVED, NOT SATISFIED. Step 6 NOT STARTED.
Phase 7 NOT STARTED.
