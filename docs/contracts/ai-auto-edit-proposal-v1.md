# AI Auto-Edit Proposal v1

Phase 7 Step 4 only. Automatic PROPOSAL/PLANNING, never automatic approval.
This additive interaction contract implements Auto availability previously marked
unimplemented at the Step 3 freeze. AI Assist Experience v1 itself is unchanged:
advice remains non-mutating and Use suggestion remains manual navigation only.

## Authority chain and request

Explicit user machine-semantic goal -> reference Assistance Provider -> UNTRUSTED
Proposal Envelope v1 -> existing native validation -> Structured Intent v1 and
deterministic reference planner OR canonical Plan v1 parser -> Registry v1 ->
source-bound canonical Plan v1 -> ephemeral REVIEW_READY -> STOP.

The provider receives data only, never editor, approval, candidate, execution or
history authority. No approval is created, reused or consumed. No candidate,
pixel work, execution, repository transaction, revision or history mutation.
Review-ready is safe to DISPLAY, not approval or execution eligibility. Phase 6
will remain the authority for later revalidation/approval/execution in Step 5.

The UI selects one of the existing eleven explicit choices (same bounded amounts
and native goal catalog as Step 3). The amount is visible before Prepare proposal.
No free-form text interpretation, heuristic, image analysis, model or network.
Single-action reference output is intentional, not a smart multi-tool recipe.

## Provider paths and validation

STRUCTURED_INTENT: native builds the explicit request from trusted current
document/context and existing catalog; the frozen provider emits Envelope v1
kind 2. Its bytes are UNTRUSTED. Existing canonical parser, identity/version/
binding checks, Structured Intent v1 validation, deterministic planner and
agent-exposable Registry v1 authority produce ordinary unchanged Plan v1.

PLAN_CANDIDATE: an explicit canonical machine-semantic Plan fixture/request may
use kind 3. Existing provider encodes it, then the SAME untrusted envelope
validation checks Plan v1 versions, exact parameters, action order, Registry and
binding. No repair, clamp, default, substitution or replan. Valid repeated tools
and up to eight actions remain supported; nine actions reject. The release UI
requests single structured goals only; native Plan Candidate support certifies
compatibility without inventing release recipes.

Canonical UTF-8, sorted keys/exact integers/terminal LF, strict fields, bounds and
all seven binding fields stay frozen. Canonical Plan bytes have NO provider, UI,
confidence, timestamp or reasoning fields. They are unchanged schema data; a Plan
or digest alone grants no execution authority.

## Review-ready representation and narrow transport

The existing provider/Advice ABI cannot return a Plan by design. A separate narrow
`pixaura/auto_proposal.h` version-1 read-only transport therefore delegates shared
native authority: request, current, canonical Plan inspection, cancel/discard.
It has no approve, candidate, execute, commit, history or dynamic provider API.
Existing editor context ownership is reused; no new handle or raised ceiling.

| Object | Bound |
| --- | --- |
| Request | 16 bytes; version, size, explicit choice, kind 2 or 3 |
| Review summary | 328 bytes; version/size/count/kind/registry/reserved-zero, context, request/source serials, IDs and eight actions |
| Action summary | 24 bytes; native Registry v1 tool index, parameter count and four exact int32 values |
| Canonical Plan cache/output | 8192 bytes, 1–8 actions |
| Structured request | Existing 2048-byte limit |
| Envelope | Existing 12288-byte limit |
| Transport provider/proposal IDs | 32-byte zero-terminated fields (current fixed reference ID and auto.serial fit); correlation only |
| Active request | One per editor/provider, SHARED with Assist; no queue |
| Retained proposal | One native summary and canonical Plan; ephemeral |

Native derives summaries only from an already validated Plan, preserving exact
action ordering and values. Tool indices resolve through the same immutable
version-1 manual registry, not display labels. Canonical Plan retains exact tool,
operation and parameter versions. Provider/UI metadata lives outside the Plan.
Request/source serials and proposal IDs are correlation/inspection data, never
approval tokens. Altered/foreign records fail native exact-cache comparison.
Caller owns all buffers; native validates version/size/kind/alignment/ranges and
overlap; errors do not replace summary outputs. Plan size query returns existing
BUFFER_TOO_SMALL=11 and required size. No C++ object/pointer or exception crosses C.

## State machine and freshness

Mode AUTO distinguishes IDLE, REQUESTING, PROPOSAL_RECEIVED, VALIDATING,
REVIEW_READY, STALE, CANCELLED, FAILED and REJECTED. No active APPROVED, EXECUTING
or COMMITTED state exists in Step 4. Native request phases are synchronous and
bounded: request -> untrusted envelope received -> validation -> final readiness
fence. UI receipt/inspection phases may coalesce into one frame; progress copy
honestly says requesting AND validating, not fabricated timed progress.

| From | Event | To/effect |
| --- | --- | --- |
| IDLE/terminal/REVIEW_READY | Explicit new request | REQUESTING, discard prior proposal |
| REQUESTING | Provider bytes received | PROPOSAL_RECEIVED, still untrusted |
| PROPOSAL_RECEIVED | Existing validators | VALIDATING |
| VALIDATING | Validators and live-source fence succeed | REVIEW_READY |
| Any pending/ready state | Cancel | CANCELLED; no document effects |
| REVIEW_READY | Dismiss | REJECTED; discard |
| Pending/ready | Manual/source/history/context change | STALE; discard |
| Any AUTO | Leave mode/owner | Abandon/cancel; new entry IDLE |
| Pending | Invalid/provider/resource error | FAILED (or STALE/CANCELLED mapping); no retry |

Native captures trusted immutable snapshot/context, live nonwrapping source
serial and repository epoch. Before returning ready it rechecks source/epoch,
manual state and cancellation serial under the existing editor gate. Inspection
rechecks exact private cached summary/current source/epoch. Binding includes
project/document/revision/session/context/generation/source digest as frozen;
digest alone never replaces source serial/ABA protection. Manual selection,
update, Apply, undo/redo and source/owner change invalidate private Auto state.
Later Step 5 must revalidate/rebind through Phase 6; old validation is not durable
authority or a substitute for repository epoch CAS at publication.

AUTO -> MANUAL/ASSIST -> AUTO never resurrects a proposal. No silent refresh,
re-request, replan, approval transfer or retry. At worst concurrent work is
discarded. Mode switching never commits/discards durable manual edits; an active
manual gesture remains under existing Manual controls, and proposal requests are
BUSY until it finishes/cancels. Auto mode hides Manual Apply controls, while the
always available Manual mode restores the same existing editor.

## Cancellation, concurrency and lifetime

One shared native provider and one in-flight transport call; Android/Apple owners
also reject competing work with RESOURCE/BUSY=8, never queue a second provider
request. Independent contexts remain isolated. Cancellation at request,
untrusted-envelope processing, validation/final publication or ready abandonment
produces no accepted late proposal. Native counters and UI owner epochs never
wrap. Exhaustion fails assistance closed, never disables Manual.

Platform work stays off the UI thread. Existing command and advice lanes are
reused, with counted native calls. stop -> join ALL calls -> destroy (ADR 0009)
remains mandatory. UI callbacks require matching owner/epoch/mode; destroyed or
recreated owners cannot receive late publication. No native callback retains UI
objects. Cancellation is cooperative; no thread kill or unsafe interruption.

## Failure, presentation, security and persistence

Existing statuses retain meaning: 1 invalid, 2 unsupported version, 3 invalid
owner, 5 unknown tool, 6 malformed, 7 invalid parameter, 8 resource/BUSY, 9 stale,
13 cancelled/stopped, 14 internal. Unknown nonzero is failure. Platforms present
bounded safe failure/stale/cancel text; no raw exceptions, tokens, addresses or
unbounded user data. Provider failure never disables Manual or changes history.

Android Compose and SwiftUI show explicit goal/value, request/loading, read-only
action count/tool/values, Dismiss and mode return. No Apply AI Edit, Approve,
Execute, Commit or Run Plan control. Detailed review and approval UX belong to
Step 5. Stable accessibility IDs cover auto-goal and each tool, auto-request,
auto-loading/cancel/ready/dismiss/error/stale/rejected and shared mode selector.
Tests resolve both nested scroll ancestors on Android and use hardened specific
element taps on Apple. Presentation is not validation or editing authority.

Certification covers forged IDs/summary, wrong binding/context, malformed
Envelope/Structured Intent/Plan, versions/Registry/overflow/oversize, repeated
requests, cross-mode BUSY, cancellation, source/manual/undo/redo/close races,
late-result suppression and zero approval/candidate/history effects. Failure
injection/checkpoints and launch flags are certification/debug only; production
binary scans reject them. No privileged test provider or execution hook ships.

All state is ephemeral: no Schema 3 field, persistent Plan/proposal, pending
approval, provider handle, interaction memory, reasoning or recovery authority.
Only ordinary existing human Manual commits persist. Local/offline; no network,
cloud, model, NLP, weights, runtime, new tool, plugin or production intelligence.
Intent/Structured Intent/Plan/Registry/Envelope/Provider/AI Assist Experience v1,
reference planner, approval/candidate, manual/history and Phase 5 remain frozen.
Production model UNRESOLVED; CPU VALIDATED; Vulkan/Metal UNKNOWN.
DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED. Hosted/emulator evidence
is software only. Phase 7 Steps 5–8 NOT STARTED. Phase 8 NOT STARTED. STOP here.
