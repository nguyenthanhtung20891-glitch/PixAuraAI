# Editing Agent core v1

Status: FROZEN Phase 6 Step 1 contract. Additive checked C ABI family 1 in
`pixaura/agent.h`. No intent/plan wire schema, planner, tool-binding registry,
approval issuer, execution bridge or candidate rendering is implemented here.
Those belong to separately authorized Steps 2–6. Phase 7 NOT STARTED.

## Actors and authority

The human is DIRECTOR and APPROVER and may remain the manual OPERATOR. AI may
be ADVISOR, PLANNER and, only after approval, an OPERATOR through registered
editing capabilities. Neither AI nor an agent context owns the project.
AI MAY INSPECT AND PROPOSE. AI MAY NOT SILENTLY COMMIT USER-OWNED EDITS.

Reuse Phase 2 ImageDocument, immutable source, ordered operation stacks,
detached candidates, actor `ai`/plan provenance and ordinary approved history.
Actor strings are attribution, never approval. Reuse ADR 0009's document context
and existing application service's approval/persistence/publication ownership.
No new approval or publication owner. Manual editing requires no agent, model,
network, inference provider or session in this subsystem.

INTENT expresses the requested outcome. PLAN is an inspectable, ordered proposal
of versioned allowed tools and parameters. EXECUTION applies a frozen, approved
plan through existing deterministic tools. They are distinct concepts; Step 1
does not define the future intent/plan serialized schema. A proposal is not an
approved plan, a detached execution candidate, or a committed revision.

Approval must originate at the human-controlled application/core boundary, never
from model output, a flag, actor, timeout, successful validation, or local model
execution. Step 5 will design the actual approval/execution mechanism. Step 1
cannot issue approval and cannot execute even an otherwise valid proposal.
Changed plans must become new proposals requiring review/approval. No hidden
replanning during deterministic execution; planner participation ends before it.

## Minimal executable boundary

`pixaura_agent_stage` accepts 1..8 operations in the EXISTING bounded evaluation
envelope `{ "operations": [...] }`. This is a validation/inspection staging seam,
not a new agent plan schema or a production planner. It reuses shared
`document::parse_evaluation`, compiled `manual::find`, and canonical serialization.
No duplicated parameter table, parser, tool range or numerical semantics.
Unknown tool/operation version/parameter version reject; malformed, duplicate,
extra/missing fields, duplicate operation IDs, noninteger and out-of-range values
reject through the accepted parser. No guessing, coercion, skip or clamp.

Tool identities are the existing stable PixAuraAI type IDs plus explicit operation
and parameter versions, never display labels. Stage establishes structural/manual
registry validity only. It does NOT establish dynamic capability eligibility,
resource fit for editing execution, stack geometry validity against real pixels,
approval or executability. Later validation must consume authoritative Phase 4
capabilities and Step 3 bindings before a plan can be approvable. No shipping
preset is invented; empty preset catalog remains empty.

Stage copies canonical bytes into an immutable owned record, binding the existing
application-supplied document snapshot token. No revision ID, permission,
capability, actor, approval or current-state claim is accepted from model output.
The trusted application owner must supply its actual approved-live handle under
its established owner lock. Like the accepted manual boundary, this ABI does not
independently discover the application's current pointer. It cannot authenticate
a dishonest native application owner. Untrusted planners receive bounded data,
never native context storage, live handles, Repository or document mutation APIs.
This is a data/authority boundary, not a sandbox for arbitrary native code.

`validate` checks owned token and exact bound live snapshot. `inspect` copies the
complete canonical actions, including IDs/versions/parameters/order, into a
caller-owned buffer. No partial comparison/output or opaque enhancement blob.
`execution_authority` checks liveness then ALWAYS returns NOT_APPROVED. There is
no commit, executor callback, approval setter, approval token or document-returning
agent entry point. Reinterpreting proposal tokens as document/gesture tokens
cannot grant their authority: kinds and registry namespaces reject them.

## Ownership, staleness and cancellation

Reuse the existing explicit document context, per-context mutex, identity and
nonwrapping serial allocator; no global mutable registry, new scheduler, worker,
queue or second context owner. Agent records count toward the existing combined
64 document/gesture handle ceiling; at most eight are agent proposals. Manual
calls work with the subsystem unused and after rejected/cancelled proposals;
resource saturation fails with existing RESOURCE rather than raising ceilings.

STAGED -> cancel -> CANCELLED; detected live mismatch or released bound snapshot
-> STALE. Cancel is idempotent, terminal states cannot return to STAGED, release
removes ownership; double release/zero/wrong-kind/cross-context/released tokens
reject. No serial reuse/wrap. Context destruction releases all records and
invalidates tokens. Exclusive destruction/storage-lifetime rules remain ADR 0009.
Same-revision undo/redo is still stale because the exact bound snapshot token is
different; session/generation are inherited from that immutable native snapshot.
Owner must supply every live replacement, including future approval checks.
Detailed asynchronous planning/candidate/publication lifecycle remains Step 6.

Stage/validate/inspect/cancel/release cause ZERO document, revision, redo, source,
asset or storage mutation. Stage is synchronous bounded parsing; cancellation
applies to staged records. No production planning task exists to interrupt yet.
Future planning must be cancellable with no revision; cancellation must prevent
successful stale publication and cannot imply approval.

## Conservative bounds and ABI defense

| Item | Limit |
| --- | --- |
| Input and retained canonical operation bytes per proposal | 8192 each |
| Actions per proposal | 1..8 |
| Retained proposals per context | 8, within combined 64 handles |
| Aggregate retained canonical payload | <=65536 bytes; fixed bounded records/map overhead separate |
| Individual operation / tool identity / operation ID | existing 1024 / 64 / 32 bytes |
| External referenced assets / metadata / free-form diagnostics | 0 / 0 / 0 bytes |
| Source binding | one existing immutable document source; no asset bytes copied |
| Parse transient | existing bounded parser, <=8192 input, <=256 parsed operations before 8-action admission |
| Threads, jobs, queue, persistence, executable payloads | none |

These are logical payload/object limits, not total RSS or a new Phase 4 policy.
No string/asset/prompt/metadata extension is silently accepted. Future plan schema
must define its own bounds within these Step 1 safety requirements.

Token size 56, serial offset 48; version/size/kind/reserved/context/serial checked.
All entry points check ABI version. Caller pointers must be truthful, accessible,
properly aligned, stable and disjoint, including context, token, input, output and
size pointer. Checked uintptr range arithmetic rejects overlap and wrapping sizes.
No C++ objects/exceptions cross C. Existing exception boundary maps allocation to
RESOURCE and other exceptions to INTERNAL. No raw platform exception is authority.
Failures preserve outputs; inspect query/short buffer returns BUFFER_TOO_SMALL
and required bytes (LF included, NUL excluded), writing no partial data.

| Code | Meaning |
| --- | --- |
| 0 | OK; staging/validation/inspection only, never approval |
| 1 / 2 / 3 | INVALID_ARGUMENT / UNSUPPORTED_ABI / invalid context or handle |
| 5 / 7 | unsupported tool/version / invalid parameter |
| 6 | malformed operation envelope (shared INVALID_PROJECT parser status) |
| 8 / 9 / 11 | RESOURCE / STALE_BASE / BUFFER_TOO_SMALL |
| 13 / 14 / 15 | CANCELLED / INTERNAL / NOT_APPROVED |

Shared status values retain their document.h meanings; 15 is agent-family only.

## Security, separation and future obligations

All planner/model output is UNTRUSTED INPUT, including locally generated data.
Structural, version, capability and parameter validation must precede approval.
No bytes become source, script, SQL, shader, shell command, executable expression,
network invocation or host tool. There is no filesystem/network/credential/token/
environment access, dynamic tool/plugin loader, hidden conversational memory,
upload, new dependency, runtime or model. Proposal state is ephemeral only.

Phase 4 alone owns tiers, pressure, recovery and capability epochs. Phase 5 alone
owns inference/package/provider semantics and trust. Agent orchestration is not an
inference provider; no model/backend/platform enters proposal semantics. The same
proposal means the same registered operations on Android and Apple. No platform
semantic reimplementation. Production segmentation remains UNRESOLVED; no weights.
Schema 3 and DH-APPLE-METAL-01 remain unchanged. No AI UX.

Executable security evidence covers malformed ABI/data, action/byte/handle bounds,
unknown tuples/parameters, duplicates, token confusion, stale/cross-context use,
release/cancel/concurrency, closed approval gate, and exact unchanged serialized
history before/after proposal lifecycle. Retained manual and Phase 0–5 gates,
native hosts, Android ARM ABIs, active ASan/UBSan and both complete same-final-SHA
workflows remain mandatory. Step 2 NOT STARTED; Phase 7 NOT STARTED.
