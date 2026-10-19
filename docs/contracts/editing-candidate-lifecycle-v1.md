# Editing candidate lifecycle v1

FROZEN Phase 6 Step 6. Runtime engineering contract; no persistence or new public handle.
Approval/execution v1 authority, consumption, Plan v1 and editing semantics remain
frozen. The synchronous execute API returns a receipt only after publication.

## Ownership and state

One private candidate at most belongs to one editor service, consumed approval,
exact plan and source live serial. No queue, transfer, rebinding or resurrection.
The bounded internal record has these states, distinct from API statuses:

| State | Allowed next state |
| --- | --- |
| CREATED | RUNNING, CANCELLED, FAILED |
| RUNNING | READY, CANCELLED, STALE, FAILED |
| READY | PUBLISHED, CANCELLED, STALE, FAILED |
| PUBLISHED / CANCELLED / STALE / FAILED | none |

There is no separately retained READY object or public publish/discard endpoint.
Failure/cancellation discards private memory through scoped cleanup. A subsequent
execution creates a different candidate; the fixed diagnostic slot may be replaced
only after prior work terminates. Idle means no candidate has yet been created.
No candidate token can be confused with an approval or proposal token.

Approval remains unconsumed on pre-start rejection. CREATED begins immediately
after the Step 5 fenced LIVE-to-CONSUMED transition; every later failure leaves
approval consumed. No implicit retry, rebase, replan or renewed approval.

## Authority and publication

Admission is try-lock plus one active slot: a competing execution returns
RESOURCE/BUSY=8 without consuming its own approval. No newer request silently
supersedes admitted work. Each request requires its own approval, and publication
advances source authority so another approval against the old source becomes stale.

Private evaluation releases the editor owner mutex. Immutable projection, digest,
source serial and candidate binding are copied before unlock; no reference to an
approval-map entry is used afterward. Releasing/revoking a consumed approval cannot
revoke admitted work or cause use-after-free; explicit cancel controls in-flight
publication. Original source and context storage stay alive until caller quiescence.

After private evaluation, reacquire the owner mutex and check stop/cancel epoch,
then exact source live serial and absence of a new manual gesture/pending command.
Any intervening committed manual edit, undo or redo changes nonreused live serial
even if the visible revision returns to its old value (ABA). Reject STALE=9;
never overwrite human work or clear a newly started manual gesture. Source digest
alone does not grant authority. Full Step 5 canonical binding validation remains.

READY prepares one ordinary DetachedCandidate/commit. Owner lock remains held
through the authoritative repository epoch CAS and live-state acceptance. The
separate publication fence checks cancellation epoch immediately before durable
publication. Cancel first means CANCELLED=13 and no commit. Publication first
means exactly one commit; a later cancel never changes PUBLISHED to CANCELLED.
Repository failure rolls back and returns existing status; no partial history.

## Cancellation, stop and locking

Cancellation retains the Step 5 nonsticky, nonwrapping epoch and cooperative raster
token. Exhaustion fails closed. No forced interruption or thread killing. Stop
takes owner authority, advances/revokes the cancellation epoch, signals active
work, and stops previews. A candidate returning from private evaluation cannot
publish after stop wins. Caller must stop, join **all** calls, then destroy under
ADR 0009; stop is not an internal worker join and creates no worker.

Lock order: editor owner -> publication fence -> raster cancellation mutex.
Pixel work holds neither editor owner nor publication fence; existing raster
registry mutex still serializes evaluator memory/admission. Private work releases
that mutex before reacquiring editor owner. Cancel uses publication fence then
raster cancellation mutex only. No inverse owner acquisition from those locks.
Durable publication necessarily holds its authority fence through repository CAS.

## Bounds and retained policy

One original plus one private result and existing admitted evaluator scratch;
no retained second candidate raster. Existing combined 64 handles, eight approvals,
eight actions, 8192-byte plan, context/raster/package limits remain unchanged.
No candidate persistence; restart restores only existing committed repository
state. No crash guarantee beyond repository transaction guarantees.
Phase 4 remains authoritative at admission and existing raster resource checks;
there is no new capability epoch, tier, recovery policy or provider-trust rule.
An observation alone does not rewrite frozen edit semantics. Manual tools and
preview generation semantics remain unchanged.

## Evidence and scope

Deterministic checkpoints certify manual/undo/redo ABA, dual execution, stop,
cancel/publication, release, exactly-once commit, failure rollback, nonsticky
recovery and no mutation before publication. Transaction failure injection occurs
after SQL updates and before COMMIT in certification builds only. Production
object scanning excludes every lifecycle/storage test hook.
No new ABI, persistence, planner, tool, approval authority, NLP/model, network or
UI. Production model UNRESOLVED. DH-APPLE-METAL-01 remains DEFERRED, NOT WAIVED,
NOT SATISFIED. Step 7 NOT STARTED; Phase 7 NOT STARTED.
