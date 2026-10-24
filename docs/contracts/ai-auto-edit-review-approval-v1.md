# AI Auto-Edit review and explicit approval v1

Phase 7 Step 5 only. Additive application interaction contract; every Phase 6
contract and Auto Proposal v1 remain frozen. No new public C ABI or Approval v2.

## Authority and exact review

Explicit request -> reference Assistance Provider -> UNTRUSTED Envelope v1 ->
existing native validation/planner/Plan v1/Registry -> REVIEW_READY -> human
inspection -> explicit HUMAN Apply -> existing native approval -> existing private
candidate/execution -> authoritative receipt -> one ordinary non-destructive
revision/shared history. Neither provider nor UI is publication authority.

Review displays ordered action count, tool labels and exact integer parameter
names, values and units, alongside friendly display values and source revision.
Display formatting never rewrites Plan bytes. The wrapper captures immutable
canonical Plan bytes and source identity after native freshness and Phase 6
eligibility validation. It does not create approval or candidates. The existing
8192-byte/eight-action bound, exact versions and registered tool semantics apply.
Provider/UI metadata remain outside Plan v1; there is no editable Plan surface.

Apply captures THIS review object, not a lookup of the latest proposal later.
The application owner gate rejects wrong owners, abandoned/attempted reviews,
competing work and a previously attempted proposal serial. It rechecks the exact
native cached proposal/current context, canonical bytes and existing Phase 6 full
validation before calling the existing human approval issuer. No field, string,
provider signal, observer, mode entry, appearance callback or timer can approve.
Only production review Apply controls call the application Apply entrypoint.
As frozen in Phase 6, trusted native application code represents human action;
the C ABI is not a sandbox that can prove a physical touch against malicious host
code. A simulated UI test tap represents that explicit application event.

## Approval, execution and receipt

An application review object is not an approval token or new authority format.
Native Approval v1 still binds every exact canonical byte including terminal LF,
SHA-256 identity, document/revision/session/context/generation/source and live
serial. Existing execution revalidates full bytes, not hash-only trust. Reordered,
modified, foreign or stale plans inherit no approval. No clamping or repair.

One Apply attempt per review/proposal, even if the attempt fails. A new explicit
request is required afterward; no hidden retry, replan or reapproval. Double taps
are rejected by UI admission and wrapper one-shot fencing. Native consumption
remains unchanged: pre-start rejection does not consume; execution admission
consumes permanently, including later failure/cancellation/staleness. BUSY cannot
steal another approval. Private tokens are released at call cleanup, never stored
in UI state or provider objects. Existing combined 64 handles/eight approval
ceilings are unchanged. The UI owns one ephemeral review and one Apply at most.

The SAME Phase 6 synchronous execution bridge performs all actions in canonical
order, privately and atomically. Platforms never iterate actions to execute them.
One Plan publishes at most one ordinary revision, or none. No partial history,
parallel executor, AI revision class, candidate API or direct repository write.
Original asset remains immutable. Repeated tools and up to eight actions retain
existing native semantics; release reference UI still generates single actions.

APPROVING/EXECUTING are presentation, not document state. COMPLETED requires native
success and a valid bounded version/size/reserved/consumed receipt with canonical
hex digest/source/result identities and the reviewed source revision. Approval,
candidate creation or execution start is never completion. A malformed receipt
is failure, never a fabricated success or reason to retry; refresh ordinary
history because publication may already have won before transport inspection.

## Cancellation, stale state and modes

Reject before Apply discards review with no approval, candidate or history effect.
Cancellation before approval abandons the review. During Apply, revoke the unused
native approval FIRST, then signal existing native cancellation. This prevents a
cancel-before-admission gap; consumed approval stays consumed. Native publication
fencing decides the winner. Cancel first means no revision; publication first
means its ordinary committed result remains, with no automatic undo.

Cancellation request is not a CANCELLED completion. Show pending cancellation
until native outcome. Leaving AUTO abandons presentation and requests cancellation;
it does not transfer authority, resurrect a review or hide a committed revision
from shared history. Returning requires a fresh explicit request. Manual edits,
undo/redo, source/context/session changes invalidate review; native source serial,
ABA protection and repository epoch CAS remain final authority at publication.
No silent refresh, rebasing, replacement plan or approval reuse.

## Ownership, platforms and failure

Android Compose/JNI and SwiftUI/C wrappers are adapters over existing shared
native authority. Existing fixed command/advice lanes are reused; no new worker
queue is introduced. Count the entire Apply call, stage notification and token
cleanup, plus concurrent cancellation, for stop -> join ALL calls -> destroy
(ADR 0009). Do not hold the application owner gate over raster work. Stage
notifications only enqueue presentation work; they must not synchronously close/
join their own in-flight caller. Application teardown runs on its owner lane.
Revocation may wait for native admission without holding that wrapper gate. Pending
cancellation rejects admission of another review execution, preventing a late
cancel from targeting fresh work. Late results cannot revive abandoned UI state.

Always refresh ordinary document/history after execution, even if Auto was left.
Manual works without provider and before/after Auto. Auto revisions use existing
undo/redo and branch-after-undo semantics, with no new history class. Provider,
validation, resource, stale, cancellation and execution failures never disable
Manual. No automatic retry or source substitution. Unknown status means failure.

Stable semantic accessibility IDs cover exact values, source, Apply, Reject,
applying/cancel/completed/error and existing modes. Tests use semantic controls,
not coordinate-only automation. Debug rendezvous use existing certification-only
hooks; release scans reject debug launch flags and hook symbols. No test approval
issuer, fake human event, candidate privilege or direct commit shortcut ships.

## Persistence, security and phase boundary

Review, provider, approval and candidate state are ephemeral, never Schema 3 or
process-restart execution authority. Durable output is only ordinary history.
Security coverage includes provider non-authority, exact bytes, forgery/wrong
owner/source, replay/double Apply, cancel/admission/publication, Manual/undo/redo,
shutdown/recreation, malformed receipts, atomic failure and release isolation.

Intent/Structured Intent/Plan/Registry/Envelope/Provider/AI Assist/Auto Proposal v1,
planner, approval/candidate and Phase 5 trust are unchanged. Production model
UNRESOLVED; CPU VALIDATED; Vulkan/Metal UNKNOWN. No NLP/model/runtime/weights/cloud.
DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED; hosted tests are software
evidence only. Steps 6-8 and Phase 8 NOT STARTED. No advanced comparison workspace,
multi-candidate inspection, editable Plan review or timeline analysis. STOP here.
