# Full Review Experience v1

Status: normative Phase 8 Step 1 contract, version 1. Contract/architecture only.
No Review Session implementation, preview raster path, new bridge, editable-review
UI or Step 2+ functionality is implemented here. Phase 7 is CLOSED / COMPLETE /
FULL PASS at `fe01c4a4222eb5bed63adc840b140f73784e1c18`.

## Scope and existing authority

REVIEW != APPROVAL. PREVIEW != EXECUTION. PREVIEW != REVISION.
VISUAL COMPARISON != DOCUMENT MUTATION. UI STATE != NATIVE AUTHORITY.
Here execution means the existing approved candidate/execution/publication flow;
future preview may compute deterministic pixels without entering that flow.
Inspection, waiting, zooming, comparing or producing a display buffer creates no
approval, execution candidate, revision, history mutation or publication permission.
A native preview image handle is a disposable display resource, not an execution
candidate handle. Similar words in older preview APIs do not equate these objects.

The unchanged AI authority chain is explicit bounded request ->
reference.assistance.v1 -> UNTRUSTED Envelope v1 -> native Structured Intent /
reference.deterministic.v1 or canonical Plan v1 / Registry v1 validation -> exact
review -> explicit HUMAN Apply -> existing approval -> private candidate/execution
-> deterministic editing engine -> at most one ordinary revision/shared history.
Manual remains independent; Assist remains non-mutating and Use suggestion remains
navigation-only. Phase 8 adds understanding, not approval or editing authority.
The initial product sketches do not override this frozen approval-before-execution
order. No provider, preview result, gesture, timer, observer or view appearance approves.

## Information architecture

The future surface consists of six bounded presentation regions, not new owners:

| Region | Information / interaction | Authority boundary |
| --- | --- | --- |
| Context | Originating mode, source revision validity, informational provider identity | Native binding supplies truth; provider label grants nothing |
| Comparison | BEFORE / AFTER PREVIEW, loading/error, toggle/split, viewport reset | Explicitly label projection; never label preview as committed |
| Ordered actions | Count, order, tool name and stable identity, exact values and units | Derived only from native-validated Plan/Registry; no parsing UI prose |
| Feedback | Invalid/unsupported/stale/resource status, static bounded warnings | Unknown errors fail closed; no fabricated success |
| Decisions | Preview, Back/Reject, explicit Apply, valid cancellation | Separate semantic actions; comparison controls cannot trigger Apply |
| Result | Ordinary committed revision/history after confirmed publication | Previous/current revision comparison; no separate AI lineage |

Manual review uses the same document and native tool semantics; it does not need a
provider. Advice cannot be promoted into a Plan review or approval by opening this
surface. Auto review requires an exact validated Plan. The new comparison and action
inspection UI is future Steps 2/3; editable controls are future Step 4, result-review
integration Step 5 and parity/polish/closure Step 6. No Step 7 is authorized.

## Bounded ephemeral Review Session

This is a conceptual application presentation object, not a wire schema or public
ABI. Reuse existing trusted native binding and owner identity; do not invent a
parallel document/source freshness counter. At most one current session per editing
interaction, one current Plan, one active requested preview and one retained current
BEFORE/AFTER pair. Superseded in-flight resources remain accounted until reclaimed.
A superseded call drains before a replacement heavy render is admitted; BUSY
rejects rather than queues. This presentation constraint does not change existing
native adapter call caps. No queue or unbounded historical session/preview list.

| Conceptual field | Source / bound |
| --- | --- |
| Review identity | Fresh nonreused application interaction identity under existing owner lifetime; correlation only |
| Binding | Trusted document/project/context/session/current revision, generation/source serial and repository epoch where existing authority requires |
| Exact Plan | Immutable canonical Plan v1 bytes, <=8192 bytes, 1–8 ordered validated actions; absent for advice-only/non-Plan inspection |
| Origin | Existing mode and trusted selected provider identity when applicable; <=64-byte provider identity inherited from Envelope v1 |
| Preview request | Existing preview ticket/context/generation plus exact source/Plan association; no native pointer as identity |
| Presentation | Finite state below, fixed local labels, bounded Registry-derived action data and viewport selection |

No approval token, approval issuer, execution candidate, revision publisher, history
owner, durable execution token or cancellation capability belongs in this object.
The existing trusted application execution owner retains its existing call ownership
and ephemeral approval cleanup outside Review Session. Review identity alone grants
nothing. No session, Plan display bytes, preview cache, handle or pending authority
is persisted to Schema 3, Bundle/SavedStateHandle, UserDefaults or state restoration.
Process death discards review; ordinary document/history may reopen normally.

## Binding and invalidation

Every preview install, review edit and HUMAN Apply must check the current owner,
exact source and exact canonical Plan association using existing native mechanisms.
Document/project switch, source replacement, manual edit/Apply, undo, redo,
branch-after-undo, session/context/generation/source-serial or relevant repository
epoch change invalidates review and its previews. Same visible revision after undo
does not restore old eligibility: native ABA/source serial protection remains.
Mode abandonment follows Phase 7 lifecycle rules. No silent rebase, refresh,
regeneration, replacement Plan or automatic reapproval. New work requires explicit
user action and fresh trusted validation/identity. Successful Apply consumes the
pre-Apply review permanently; any result presentation is read-only receipt/history
information, never reusable approval eligibility.

## Review presentation states and Phase 7 mapping

These states add no execution enum, authority transition, token or native state
machine. They describe a view of existing native facts; unsupported events fail
closed. Preview state is an overlay on a valid Phase 7 review, not a new approval.

| Presentation state | Meaning / existing authority mapping |
| --- | --- |
| NO_REVIEW | No review; existing Manual/Assist/Auto idle or abandoned interaction |
| PLAN_REVIEW | Current exact validated Plan; existing Auto REVIEW_READY; no approval |
| PREVIEW_LOADING | Bounded display request pending; underlying review unchanged |
| PREVIEW_READY | Current source/Plan/request matched projection; underlying review unchanged |
| PREVIEW_FAILED | Display unavailable; current exact Plan remains inspectable |
| STALE | Native binding invalid; no Apply, preview reuse or rebase |
| APPLY_PENDING | Explicit HUMAN Apply requested; existing approving/admission path, not proof of execution/publication |
| APPLY_EXECUTING | Existing native execution in progress; presentation alone proves no revision |
| APPLY_SUCCEEDED | Valid authoritative native publication receipt; old review no longer eligible |
| APPLY_FAILED | Existing approval/execution/transport failure; no automatic retry; reconcile ordinary history |
| CANCELLED | Confirmed preview/session cancellation or native no-publication cancellation; scope must be explicit |

Required transitions (the accompanying governance data records these same guards):

| From | Event | To | Guard / consequence |
| --- | --- | --- | --- |
| NO_REVIEW | validated_plan | PLAN_REVIEW | Fresh owner, native exact Plan and live binding; no approval |
| PLAN_REVIEW, PREVIEW_READY, PREVIEW_FAILED | request_preview | PREVIEW_LOADING | Explicit action, current binding, bounded admission; zero history effects |
| PREVIEW_LOADING | preview_current | PREVIEW_READY | Native request eligibility AND exact source/Plan/owner match at install |
| PREVIEW_LOADING | preview_failure | PREVIEW_FAILED | Native error/resource status; release private work, preserve Plan inspection |
| PREVIEW_LOADING | preview_cancelled | CANCELLED | Preview scope only; native cancellation confirmed, no document effects |
| CANCELLED | inspect_retained_plan | PLAN_REVIEW | Preview-only cancellation and native binding still current; no closed-session resurrection |
| PLAN_REVIEW, PREVIEW_LOADING, PREVIEW_READY, PREVIEW_FAILED | binding_changed | STALE | Invalidate review and all preview eligibility |
| PLAN_REVIEW, PREVIEW_READY, PREVIEW_FAILED | human_apply | APPLY_PENDING | Explicit HUMAN event, current exact final review; existing Phase 7 one-shot admission |
| APPLY_PENDING | native_admitted | APPLY_EXECUTING | Existing approval binding/single-use execution admission only |
| APPLY_PENDING | approval_failure | APPLY_FAILED | Native status, release unused authority under existing rules; no retry |
| APPLY_EXECUTING | native_published_receipt | APPLY_SUCCEEDED | Valid bounded authoritative receipt, not a callback-start signal |
| APPLY_PENDING, APPLY_EXECUTING | native_stale | STALE | Native outcome, consumed authority never restored |
| APPLY_PENDING, APPLY_EXECUTING | native_cancelled_no_publish | CANCELLED | Native cancellation wins, zero revision |
| APPLY_EXECUTING | native_failure | APPLY_FAILED | Existing failure/transport status; refresh ordinary history |
| PLAN_REVIEW, PREVIEW_LOADING, PREVIEW_READY, PREVIEW_FAILED, STALE | abandon | NO_REVIEW | Drop session/preview; stop/cancel and join owned calls before destroy |

Apply is unavailable while preview is still being prepared in this contract's
presentation model; explicit preview cancellation can return to inspectable Plan.
This avoids conflating two pending actions without modifying native admission.
Terminal Apply states cannot return to approvable review. A new explicit request
and newly validated review are required. Reject/Back do not execute any Plan.

## BEFORE / AFTER PREVIEW and future comparison

BEFORE is the exact authoritative source revision against which the Plan is bound,
including its existing ordinary edit stack. It is not automatically the immutable
original. AFTER PREVIEW is a non-authoritative visual projection of that source plus
the exact ordered reviewed Plan through existing deterministic native tool semantics.
It is not a committed revision, approval, execution candidate, durable history or
proof Apply will succeed. Keep labels distinct from POST-APPLY RESULT.

Future Step 2 must reuse existing native evaluation, bounded preview/resampling and
interactive-preview lifecycle semantics through a non-publication path. No Agent
approval/candidate/execution or repository commit API may be called to render a
preview. No second raster engine or Kotlin/Swift editing math. CPU reference/tool
semantics remain authoritative; display reduction/clipping is labelled as preview,
never a promise of full-resolution final output. This requirement defines no new
public ABI or raster implementation. If it needs frozen semantic changes, STOP.

Toggle and split select views of the same binding. Synchronized zoom/pan shares a
bounded comparison viewport, preserves aspect ratio and canonical orientation;
reset returns to fit. Crop/rotation may change geometry: Step 2 must use existing
native geometry mapping and test registration rather than stretch either image or
claim false pixel correspondence. If correspondence is unavailable, label geometry
change and use safe fit presentation until the mapping is defined. Gesture location,
split position and viewport are presentation only, never Plan values or Apply.

## Ordered Plan inspection and editable review

Show the native-validated action count, stable tool/version identity, human-readable
name, exact integer parameter value, parameter name/unit and order. Friendly decimal
formatting may supplement exact values but cannot truncate or silently substitute
them. Labels/warnings are static local presentation derived from trusted Registry
metadata; no unbounded provider prose. Unsupported tool/version, invalid parameters
or stale binding have distinct error states and cannot become approvable.

NEW INTENT / NEW PLAN CANDIDATE FOR VALIDATION is required for every future user
review change. Editing a review never mutates the already-reviewed Plan in place.
Value change, action removal, reordering and restore/revert each invalidate the old
review and preview eligibility, run full existing trusted planner/Plan/Registry and
binding validation, then obtain NEW exact canonical Plan bytes and NEW review identity.
The new bytes may equal earlier bytes after restore; identity is still fresh and
old approval/review/request eligibility is never resurrected. Original provider
identity is informational provenance, not a claim it produced a user's changed Plan.
If validation fails, remain without an approvable new review and show the validation
error (NO_REVIEW); never silently restore old eligibility. Explicit Restore also
requires the new validation/identity path.

No action is assumed reorderable or commutative. Validate the COMPLETE resulting
ordered Plan. Empty removal yields no executable Plan/Apply; Plan v1 remains 1–8
actions/8192 bytes. No clamping, hidden repair, action substitution or goal expansion.
Do not force multi-action edits into the frozen one-goal reference planner; use the
existing Plan candidate validation semantics where applicable. Step 4 must demonstrate
the existing native ownership/validation path before enabling edits; no new execution
authority is implied. Approved Plans are immutable; editing during Apply is unavailable.

## HUMAN Apply, lifecycle and outcome reconciliation

Apply remains explicit HUMAN confirmation -> existing exact-bound native approval
-> single-use private candidate/execution -> at most one ordinary revision. Revalidate
live source/serial/epoch and exact final reviewed bytes before approval and publication
under Phase 7 rules. No auto-Apply after preview, timer, appearance, observer, double
tap fallback, hidden retry/replan/reapproval or mutable approved Plan. Preview failure
does not forbid inspecting/applying a still-valid Plan after separate HUMAN confirmation.

Before execution, Back/close/mode/source/navigation changes abandon the session,
cancel preview eligibility and release ephemeral resources. During pending/active
Apply, request existing cancellation/revocation and await native winner semantics;
do not immediately assert CANCELLED. Publication first keeps its ordinary revision;
cancel first before publication creates none. UI disappearance never hides or undoes
publication. An old cancellation cannot target new work; pending cancellation occupies
existing admission. No native authority transfers to a new view/session/editor.

Stop -> join ALL calls -> destroy (ADR 0009), including render/copy/install, native
Apply, cancellation, notification and cleanup. Never join one's own call from its
stage callback or hold a UI/lifecycle gate over long raster work. Native request
eligibility and the platform owner gate jointly guard install; a check followed by
an unsynchronized assignment is insufficient. Late results must match owner, request,
source and exact Plan before installation; stale callbacks cannot revive review.
Process/view recreation restores ordinary history only, never an old Apply.

After every Apply outcome refresh authoritative ordinary document/history. A valid
receipt is required for APPLY_SUCCEEDED. A malformed receipt or transport error may
occur AFTER publication: APPLY_FAILED must not assert zero revisions without native
confirmation, automatically retry or undo. Show outcome unconfirmed while refreshing
history; any visible committed result remains authoritative. History observation alone
does not fabricate an execution receipt or recreate approval. Later result review can
inspect that ordinary revision with honest outcome wording.

## Failure presentation

Names below reuse existing native status meanings; no new status/ABI is defined.
Unknown nonzero values always fail safely. Preflight/admission is never a lease.

| Native outcome | Presentation / effect |
| --- | --- |
| 1 invalid request, 2 version, 3 wrong/stale handle, 5 unsupported tool, 6 malformed/noncanonical Plan, 7 invalid parameters | Invalid/unsupported review; no Apply or partial acceptance; distinguish stale binding via native current checks |
| 9 stale base/source/epoch | STALE; discard preview/review eligibility; no silent rebase |
| 8 preview allocation/resource/BUSY | PREVIEW_FAILED; Manual available; retain valid Plan inspection; explicit new preview request only |
| 13 preview cancelled/stopped | CANCELLED preview/session scope; do not reuse ticket; return to Plan inspection only if retained session still current |
| 12 IO, 14 internal or other preview computation error | PREVIEW_FAILED; bounded static error; no document change |
| Apply BUSY/approval failure/15 NOT_APPROVED | APPLY_FAILED; existing cleanup/single-use rules; no automatic second Apply |
| Execution failure or stale before publication | APPLY_FAILED or STALE; native zero-publication outcome; consumed approval stays consumed |
| Cancellation wins before publication | CANCELLED only after native confirmation, no revision |
| Publication wins | APPLY_SUCCEEDED only with valid authoritative receipt; one ordinary revision; later cancel cannot undo it |
| Malformed receipt/transport error | APPLY_FAILED with outcome unconfirmed; refresh native history, never claim no publication or retry |

## Preview cache and resources

Future caching, if used, is bounded and ephemeral: at most the current bound
BEFORE/AFTER display pair and one current request. Key by exact owner/document/source
revision/context/generation/serial/epoch as applicable, exact canonical Plan bytes,
preview request and existing render configuration/geometry. A digest alone is not
substitute authority. No cross-document/Plan reuse or Schema 3/file cache. Invalidate
on every relevant binding/edit/mode change. Do not expose an obsolete After as current.

Protect Manual/editor and platform/UI/OS headroom. Use existing Budget Admission v1
PREVIEW work class where applicable, native evaluation/preview limits and actual
owner admission; do not enlarge or reinterpret them. Charge simultaneously retained
source/working images, evaluation scratch, both display buffers, platform copies,
superseded in-flight work and handles without double counting or omission. Native
image-handle publication is distinct from document publication. Shared mutable
editing state is not a preview cache. No guessed numeric memory ceiling, RSS-derived
lease, new global scheduler or heavyweight intelligence owner is introduced here.

Resource pressure/admission failure discards optional work and keeps Manual usable.
Plan inspection remains where possible, explicit retry creates fresh preview request,
and Apply is a separate authority/admission decision. No automatic retry, cache
growth, work queue or forced eviction of ordinary Manual/document resources. Step 2
must measure the actual before/after peak path and demonstrate applicable existing
resource classes before implementation; STOP if it requires a resource-contract change.

## Platform, accessibility, result review and privacy

Android Compose/JNI and SwiftUI/C share these semantics. Shared native code owns
Plan identity, Registry/validation, freshness, deterministic preview/edit semantics,
approval, execution and history. Kotlin/Swift present validated data and transport
bounded requests; they do not interpret labels, execute actions or publish revisions.
No production platform code/bridge changes in Step 1.

Future semantic identifiers must distinguish review surface, source/validity,
action count and each ordered action/value/unit, before/after selector, split,
viewport reset, preview request/loading/failure/cancel, HUMAN Apply, Reject/Back,
Apply pending/executing/completed/error and result revision. Names are platform UI
contracts, not native authority. VoiceOver/TalkBack must announce selected before/
after state without color alone, exact values/units, stale/error versus success,
loading and pending cancellation. Apply is visually and semantically distinct from
Preview and comparison gestures; Reject/Back never share an Apply handler. Loading
cannot indefinitely disable return to Manual. Tests use semantic actions, not
coordinate-only automation. Polished controls/gestures are not implemented now.

POST-APPLY RESULT REVIEW references an ordinary committed revision. Previous versus
current revision is ordinary history comparison, not a parallel lineage or AI history.
Undo/redo/branch-after-undo keep their existing semantics. A later history change
refreshes/invalidate the comparison; a historic revision may be explicitly inspected
as historical, never silently treated as current or pending approval.

Local-first/offline only: no image upload or cloud preview. No screenshot/image telemetry,
analytics editing dependency, API key, model download or hidden persistence. No raw
image, request, Plan, token or native address in diagnostics. Free-form Intent v1
remains uninterpreted; no NLP/chat, arbitrary-photo diagnosis or learned research.
reference.assistance.v1 remains ADMITTED; reference.deterministic.v1 remains planner.
Learned model NONE REQUIRED / NONE ADMITTED FOR PHASE 7 MVP; future provider deferred.
EmbeddingGemma disqualified; NIMA/FFCC/MobileIQA unselected. No research reopened.

## Threat matrix and testable invariants

These are required protections, not claims that future preview/edit UI already exists.
Step 1 checks the contract/governance and unchanged Phase 7 authority. Future steps
must execute native/platform tests for their new paths before claiming implemented PASS.

| Threat | Required protection | Future proof |
| --- | --- | --- |
| A preview mistaken for result | Explicit AFTER PREVIEW versus committed-result labels; no receipt from render | Steps 2/5 semantic labels and zero-revision checks |
| B stale preview shown current | Exact native source/Plan/request check at install, stale UI state | Step 2 source/edit/undo/redo/epoch races |
| C preview/Plan mismatch | Match every exact canonical byte and bound source/config, preserve action order | Steps 2/3 altered Plan/order/value rejection |
| D edited review reuses approval | New candidate validation + new review identity; old review invalidated | Step 4 old-approval/replay/restore tests |
| E displayed value differs from Plan | Native Registry-derived exact values/units, no label parsing | Step 3 value/order/locale/accessibility assertions |
| F cross-document cache leakage | Bound ephemeral cache, no foreign owner/document key reuse | Step 2 cross-owner/document/recreation tests |
| G preview exhaustion harms Manual | Existing admission/reserves, count full peak, bounded optional work | Step 2 pressure/failure/allocation and Manual fallback |
| H platform semantic drift | Same native validation/tool/render/history authority | Step 6 Android/Apple parity, retained suites throughout |
| I unintended Apply | Dedicated explicit HUMAN control only, no preview observer/gesture Apply | Steps 2–4 preview/toggle/split/Back must never approve |
| J late callback revives review | Owner/request/source/Plan fence, stop/join before destroy | Step 2 deterministic close/cancel/late-install tests |
| K result review hides publication | Native winner/receipt, refresh ordinary history after any Apply outcome | Step 5 publish-before-teardown/transport-failure tests |

R01 inspection/comparison has zero document/approval/candidate effects. R02 current
review matches exact source and canonical Plan. R03 stale/terminal review cannot
Apply. R04 edits produce validated new Plan/review identity. R05 HUMAN Apply only.
R06 one approved Plan publishes one ordinary revision or none. R07 native outcome
alone determines completion/cancellation. R08 Manual survives preview failure.
R09 bounded ephemeral cache/request/resources, no persisted authority. R10 no platform
editing math. R11 accessible exact action values and distinct controls. R12 no
free-form/model/cloud capability. R13 all existing frozen contracts unchanged.
R14 Phase 8 Step 2+ NOT STARTED in Step 1, no production review implementation.

## Frozen dependencies and authorization boundary

Intent v1, Structured Intent v1, Plan v1, Registry v1, Envelope/Provider v1,
reference planner, Phase 6 approval/candidate/publication, Schema 3, shared history,
Manual, platform security, ADR 0009, Phase 4 resource authority, Phase 5 segmentation
inference and Intelligence Runtime Resource Admission v1 retain their semantics.
This additive contract is not any of their v2 versions. CPU VALIDATED; Vulkan/Metal
UNKNOWN. DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED. Hosted Apple is
software evidence only. No Step 2+ UI/raster/bridge/model/network implementation.
Stop after Phase 8 Step 1. No Step 7 without Product Owner / Principal Architect approval.
