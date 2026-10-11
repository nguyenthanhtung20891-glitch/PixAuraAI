# Gesture lifecycle integration v1

Authority: Phase 3 Step 6. Reuse the Step 1 `Gesture` controller and all accepted
manual descriptors, detached document transitions, SQLite approval authority and
PRV1 publication owner. This is an additive C/JNI/Swift boundary integration,
not another gesture engine, scheduler or final editor UI.

## States and ownership

IDLE means no owned gesture. BEGIN -> ACTIVE; UPDATE/PREVIEW keep ACTIVE.
COMMIT trials ACTIVE -> COMMITTING -> COMPLETED under the document context lock.
COMMITTING is synchronous and cannot be observed by another context call.
Validation/allocation/admission failure restores ACTIVE for explicit retry.
CANCEL -> CANCELLED; stale binding/base release -> INVALIDATED. Successful commit
and both terminal cancellations revoke gesture-owned PRV1 eligibility.
Repeated commit/update/projection reject 13. Cancel is idempotent and does not
rewrite COMPLETED/INVALIDATED. Released tokens reject 3. Release returns to IDLE.

`pixaura_manual_edit_begin` admits ONE active gesture per document context, captures
immutable project/document/source/session/generation/current revision and the
selected /1/1 descriptor, neutral append or selected operation's initial state.
It verifies the source handle is the normalized original and its encoded SHA-256
and byte length equal the document source. Identity/evaluated working candidates
cannot be substituted. It admits the existing bounded controller and advances
PRV1 only after all allocations; failure creates no history or owned gesture.
Unknown tools/versions, malformed IDs, invalid handles and an active owner reject.
Old reference begin remains usable without raster binding; no alternate semantics.

The approved-live handle remains an explicit application-service input. Detached
snapshots never become current merely by allocation. Caller serializes approval,
live replacement and gesture/display operations with the existing platform owner
lock; persistence expected revision/session/generation/epoch remains authority.

## Update and preview

UPDATE validates the shared integer operation envelope, replaces ONE pending
operation and increments its nonwrapping sequence. For an integrated gesture,
bounded trial metadata is validated before PRV1 begin, then swapped without
allocation. Every accepted update requests a new PRV1 generation. Failure keeps
the last valid operation/sequence/ticket, including PRV1 exhaustion/stop. There is
no update queue, per-update document handle, revision, persistence or cache.
Initial neutral state has sequence zero; the frozen Step 1 eligibility rule
requires a nonzero accepted update before a gesture preview may publish. The
previous approved base display remains valid independently.

`manual_preview_ticket` copies the current ticket/sequence. Existing worker
`manual_render` verifies bound original/context/ticket, projects the ordered full
stack and uses existing CPU evaluation and PRV1 rendering. It releases private
evaluated/failed preview handles. Latest sequence and PRV1 are checked again.
Superseded/cancelled computation may finish; the generation fence determines
eligibility. Failure never promotes an older result or blanks the previous image.

`manual_preview_current` checks live binding, sequence, owned ticket and optional
native result. The established platform owner lock MUST cover this call plus
display replacement, and all begin/update/commit/cancel/live changes. Native checks
alone are not a display lock. Detached Bitmap/CGImage copies survive native release.
Raw PRV1 changes must not bypass the established owner. No second publication owner.

## Commit, interruption and adjacent actions

COMMIT returns exactly one detached immutable proposal for a change. It preserves
replacement position, append ordering and neutral replacement removal. No update,
neutral append/return to neutral or identical replacement returns borrowed live
with changed=0 and no revision. Changed=1 returns an owned proposal. The original
controller closes only after proposal-handle admission succeeds; retry cannot
double commit. Neither result grants approval or writes storage. Owner approves
and persists with the ordinary existing transaction, then supplies the resulting
live revision to the next gesture. Persistence failure after proposal remains the
approval service's explicit retry/reject responsibility, not another gesture commit.

Tool switch: `manual_edit_switch` validates arguments/live, then cancels and
releases A before admitting B under the context mutex and existing owner lock.
Admission failure for B leaves A terminal; malformed switch arguments reject
before switching. Never auto-commit. Undo/redo/checkout and
ordinary document transition or preset proposal reject 13 while ACTIVE (including
the synchronous committing critical section). After cancel/commit they use the
unchanged history/preset APIs. A preset is one batch revision action, never a slider.

Background, navigation away and editor disposal call `manual_interrupt`: revoke,
cancel and release all gesture tokens, idempotently. No hidden commit. Context
destroy revokes outstanding tickets and releases controllers. Destruction retains
ADR 0009 / PRV1 stop -> join/quiesce -> destroy requirements. Document context is
destroyed BEFORE the bound raster context; raster/source storage remains stable
and alive until all gestures and workers are quiescent. Resumption begins fresh.

## Failure and bounds

Stale project/document/source/session/generation/revision input invalidates and
revokes safely, including release of the bound original handle. Original liveness
uses the existing raster registry publication mutex; it cannot block UPDATE on
pixel work. All entry insertions/releases use that mutex, with unchanged ordinary
source pinning and final PRV1 fence. Invalid UPDATE preserves ACTIVE/pending/ticket. Allocation,
admission and final validation failures preserve approved history and permit
commit retry. Preview failure preserves the prior platform copy. Cancellation
needs no allocation; repeated cancel succeeds; invalid/released handles reject.
No output is overwritten on failure except established projection size queries.

One active controller, one immutable base, <=256-operation captured stack, one
<=1024-byte logical pending operation, one PRV1 ticket and one original token.
Trial stack cloning is bounded by 256, discarded after each call; no retained
clones or update queue. Terminal tokens count toward the unchanged combined 64
document/gesture handle ceiling until release. PRV1 control, raster/context ceilings,
platform two-render admission and <=4 MiB detached copy bounds remain unchanged.
Gesture state is ephemeral; Schema 3 and immutable schemas 1/2 are unchanged.

A: lifecycle integration, boundaries and acceptance evidence. B: final platform
editor controls/flows in frozen Step 7, certification/closure Step 8. C: optional
latency telemetry, artistic catalogs and new tools. No AI/GPU/storage schema work.
DH-APPLE-METAL-01 remains unchanged. Step 7 has NOT started.
