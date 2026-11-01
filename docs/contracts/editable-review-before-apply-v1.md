# Editable Review Before Apply v1

Phase 8 Step 4 only. REVIEW != APPROVAL. EDIT != APPROVAL.
PREVIEW != EXECUTION. UI STATE != NATIVE AUTHORITY.

## Authority and immutable content

An already-reviewed Plan is immutable. The native edit owner retains one bounded
original canonical Plan (at most 8192 bytes), while the existing Auto review owner
retains the current canonical Plan. There is no review undo stack, durable draft,
new history, raster engine, Registry, parser, planner or execution authority.
Schema 3 and all frozen contracts remain unchanged. Step 5 NOT STARTED.

An explicit bounded edit request constructs a NEW complete Plan candidate from
trusted native inspection records, or from the immutable original for restore.
Existing canonical Plan v1 parsing, native Registry binding and existing full-stack
geometry/resource validation check the entire candidate. No partial acceptance,
clamping, compensation, hidden reorder, replacement, retry or rebase is permitted.
The existing reference planner is not asked to interpret arbitrary multi-actions.

Only after all construction/validation succeeds does a non-allocating install make
the NEW canonical Plan and NEW native review current. Native inspection must then
be extracted anew. No UI patch is an inspection or a Plan. No image is allocated,
rendered or published by editing. Preview must be explicitly requested again.

## Bounded ABI family 1

| Record | Bytes | Content |
| --- | ---: | --- |
| Edit ticket RED1 | 136 | version/size/kind/reserved; monotonic serial, source serial, repository epoch; context[32], exact Plan hash[64] |
| Edit request | 776 | version/size, operation, action/parameter/destination indices, exact int32 raw value, reserved; ticket; tool ID[32], parameter name[32]; exact native action record[544] |
| Edit result | 480 | version/size/state/reserved; new ticket; existing proposal presentation |

All buffers are caller-owned, aligned, disjoint and stable for the synchronous
call. No pointer/heap ownership crosses the ABI; no release is required. Strings
are exact zero-padded native identities, not prose or arbitrary paths. Unknown
versions/kinds/fields fail closed. Output is unchanged on failure. Current-query
returns the authoritative session state after an admitted failure. Hash is byte
identity only, not authentication, approval or a publication capability.

`pixaura_review_edit_begin` binds a session to a current exact inspection and
snapshots original content. It is not an edit and does not retire that review.
There is at most one active session per editor interaction. Repeated begin for
the same active lineage returns BUSY. A new ordinary proposal/source/mode epoch
invalidates the old session; its ticket cannot transfer to a new owner/document.

`pixaura_review_edit` uses a try-lock: competing work returns existing BUSY/resource
status 8, without a queue or automatic retry. Wrong owner/ticket/source, stale
repository epoch, active execution/request/render, malformed transport or stopped
owner reject before admission. Native target/value/whole-Plan validation follows
admission. Admission permanently retires the old native review and inspection,
invalidates/releases its preview, and revokes any unused existing approval using
its existing revoked state. A consumed/running approval is not editable.

Every successful edit advances both native proposal/review sequence and edit
ticket serial. Restore advances identity even when bytes equal an earlier Plan.
No old inspection, preview, approval or callback can become current again (ABA).

## Operations and states

CHANGE_VALUE identifies exact current action/tool and parameter/name, accepts an
exact native action record matching the requested index (including all raw values
and versions), then an
exact int32 value, and preserves every other raw value/action/order. Registry
min/max and complete Plan geometry constraints are authoritative. No snapping or
lossy decimal authority; platforms transport raw integers only.

REMOVE_ACTION removes exactly one action, preserving remaining relative order.
Removing the last action returns NO_ACTIONS without constructing an empty Plan.
The result's proposal field is not actionable in NO_ACTIONS or EDIT_FAILED.
Neither Apply nor Preview is available in these states; explicit restore remains
possible if source/session is still current.

MOVE_ACTION moves one exact indexed action to one valid destination, reindexes
the constructed candidate and validates the entire result. No commutativity is
assumed. RESTORE_ORIGINAL has no target/value fields and passes immutable original
content through exactly the same full validation/install path.

ORIGINAL and EDITED describe content equality, not authority. NO_ACTIONS has no
executable current Plan. EDIT_FAILED follows admitted validation/allocation failure:
there is no current executable review, old eligibility stays retired, and only
explicit restore or a new ordinary proposal can recover. Failures do not install
partial output. Protocol/pre-admission BUSY rejection never claims an edit happened.
Platform presentation also abandons its old review after an edit error, including
transport/materialization failure. This is conservative UI abandonment, not a
claim about native success/cancellation. Recovery still requires explicit native
restore/validation; no old Apply is offered while presentation is unavailable.

Source/revision/serial/context/session/generation or repository change, Manual
edit, undo/redo/branch, Apply, mode abandonment and teardown invalidate the lineage.
There is no silent source rebase. Once HUMAN Apply begins platform editing is
disabled and native execution admission rejects competing edit work. The existing
exact Plan revalidation, single-use approval and private candidate/execution path
remain the sole route to at most one ordinary revision. No approval is inherited.

## Platforms, lifecycle and resources

Kotlin/Swift decode fixed native results and regenerate inspection from the new
Plan. They do not parse/encode Plan JSON, infer tool rules, patch action rows,
reorder native storage or reconstruct a Registry. Separate accessible controls
propose exact integer values, remove, move up/down and restore original review
content. Restore is explicitly distinct from document undo. Parameter labels
announce action position, tool/name, exact value, unit and allowed native range;
reorder/remove announce their exact target and destination.
Raw text entry is explicitly an unvalidated input draft, not canonical review
content. Each field holds at most 11 characters and the bounded presentation set
has at most 32 parameter keys. Apply is disabled while any field differs from
its native canonical value. Explicit Validate submits only that parameter; a
new native review resets input widgets. No batch Plan or review history is
synthesized from unvalidated fields, and no input draft is persisted.

One synchronous native edit at a time; existing platform command owners fence
presentation by owner/interaction epoch. No new worker pool, queue, cache or
background preview is introduced. Late completion cannot revive discarded UI.
Stop -> join ALL owned calls -> destroy remains mandatory. Stop cannot destroy
storage while a gated edit is using it. No callback joins itself.

Storage is one original plus existing current Plan (each <=8192 bytes), one fixed
session result and monotonic counters, and bounded temporary candidate/validation
and inspection records. Abandon clears original content/eligibility; allocator
capacity may remain bounded until owner destruction. Editing needs zero image
buffers. Existing Phase 4/5 and Intelligence Runtime Resource Admission v1 are
unchanged. Cumulative requested allocation measurements are not live peak memory,
RSS or mobile leases. Manual state is never evicted to admit editing.

All work is local/offline, ephemeral and non-persistent. No network, remote
validation, model, NLP, telemetry authority or disk draft is introduced. Provider
reference.assistance.v1 remains admitted and non-authoritative; planner
reference.deterministic.v1 unchanged. CPU VALIDATED; Vulkan/Metal UNKNOWN.
DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED.

## Required threat protections

| Threat | Required protection |
| --- | --- |
| A: mutate old Plan | immutable old bytes; construct/validate separate candidate |
| B: approval reuse | retire unused approval; fresh exact HUMAN binding only |
| C/D: old preview/inspection | retire native review; native current/association rejects |
| E/F: forged indices/identities | bounds and complete native ID/name matching |
| G: clamp invalid value | reject out-of-range; no clamp |
| H: reorder bypass | complete canonical/Registry/full-stack validation |
| I: empty execution | NO_ACTIONS, no current Plan/Apply/Preview |
| J: restore ABA | fresh ticket and review serial even for equal bytes |
| K: stale source | live serial/context/repository checks, no rebase |
| L: concurrent requests | one try-lock admission, BUSY/no queue/retry |
| M: local row mismatch | new native inspection only; no UI row patches |
| N: hidden authority | edit has no approval issuer, executor or publisher |

Certification requires retained native/platform tests, allocation sweeps, active
ASan/UBSan, final-package release scans/negative fixtures, exact implementation-SHA
and separate documentation closure-SHA full Foundation and Native workflows.
Only externally observed closure success activates COMPLETE / FULL PASS.
