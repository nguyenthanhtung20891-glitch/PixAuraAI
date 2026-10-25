# AI interaction lifecycle integration v1

Phase 7 Step 6 only. Additive platform interaction/lifecycle contract. Steps 1-5
and Phase 6 authority are unchanged; no new ABI, provider, tool or persistence.

## Authority and ownership

MANUAL uses existing manual tools. AI ASSIST uses explicit goal -> UNTRUSTED ADVICE
-> native validation -> recommendation -> optional navigation-only Use suggestion.
AI AUTO-EDIT uses explicit goal -> provider -> UNTRUSTED Envelope -> native
validation/planner/Registry -> REVIEW_READY -> explicit HUMAN Apply of that exact
immutable review -> existing approval/candidate/execution -> native publication
receipt -> one ordinary non-destructive revision or none. Provider and UI are not
editing authority. Original immutable; one document/source/history/undo/redo.

Application model owns mode, request, advice, proposal, review, Apply attempt,
cancellation request and receipt presentation ephemerally. No ownership transfers
between native contexts or UI owners. Interaction epochs suppress old presentation;
they do not replace native context/source serial, generation, ABA or repository CAS.

## Mode transition matrix

| From | To | Effect |
| --- | --- | --- |
| MANUAL | ASSIST | Cancel/abandon obsolete AI state; begin idle Assist |
| ASSIST | MANUAL | Cancel advice; drop recommendation; existing Manual |
| MANUAL | AUTO | Cancel/abandon obsolete AI state; begin idle Auto |
| AUTO | MANUAL | Drop review; request cancellation if applying; existing Manual |
| ASSIST | AUTO | Cancel advice; never convert advice into approval |
| AUTO | ASSIST | Drop review; request cancellation if applying; idle Assist |

Every edge causes zero edits, approvals, candidates or history by itself. No stale
proposal resurrection on a round trip. Return requires a fresh explicit request.
Pending cancellation/provider completion occupies bounded admission; BUSY rejects
new work instead of queuing it. No automatic retries, replan or reapproval.

## Android and Apple lifecycle

Android ViewModel/model holds in-memory presentation; Compose recomposition has no
approval effect. Screen disposal closes its editor; configuration/Activity recreation
abandons AI state and reopens ordinary persisted document. ON_STOP interrupts
pending interaction/manual gesture through existing APIs. No authority in Bundle,
SavedStateHandle or Android state restoration. Backgrounding does not imply undo.

Apple ObservableObject holds in-memory presentation; SwiftUI appearance/observation
never approves. onDisappear closes owner; non-active scene interrupts through
existing APIs. Navigation/reappearance reopens ordinary document only. Weak model
callbacks and interaction epochs suppress stale presentation. No authority in
UserDefaults, state restoration or files. Existing project locator is not approval.

Process death drops Advice, proposal, review, approval, candidate and provider state.
After restart only ordinary durable project/history reopen. Never resume old Apply.

## Stop, join and callbacks

ADR 0009: stop -> join ALL in-flight native calls -> destroy. Existing wrapper call
counts include provider, preview, Apply, stage notification, approval cleanup and
cancellation. Teardown runs on owner/command lane; never join one's own call from
a synchronous stage callback. No long lifecycle lock over raster work. Existing
bounded lanes/mailboxes remain; no unmanaged production worker or new queue.

Results must match owner and current interaction epoch before presentation. Advice
or Auto result after mode/source change is discarded/revalidated natively. An old
completion must not release admission or clear loading for newer work. Android
keeps provider admission through command-lane validation/presentation; both platforms
reject admission during pending cancellation. Cancel callbacks cannot target fresh
work. Disposed owners receive no resurrected review or automatic Apply.

## Apply departure, cancellation and history

Apply captures exact reviewed bytes/context, never the latest proposal lookup.
Single-use consumption and native candidate/publication fencing remain frozen.
Reject before Apply is non-mutating. Cancellation requests are not cancellation
success. Revoke unused approval before existing execution cancellation; after
admission approval remains consumed regardless of failure/cancel/stale outcome.

During departure, cancellation wins -> no revision; publication wins -> committed
ordinary revision remains. UI disappearance cannot fabricate success/cancellation
or automatically undo. Receipt is evidence, not authority. After every Apply
outcome refresh ordinary native history, including malformed receipt/transport
failure or abandoned Auto mode. On return/reopen use authoritative history, never
serialized receipt/review. No hidden retry. One atomic revision or none.

## Concurrency certification matrix

| Work | Competing authority | Required result |
| --- | --- | --- |
| Assist / Auto request | Manual edit, undo, redo | Native stale check; discard obsolete result |
| Assist / Auto request | Mode switch, close, teardown | Cancel/stop; no late presentation |
| Review | Manual edit, undo, redo, document/context change | Invalidate; native Apply recheck rejects |
| Apply | Duplicate Apply, Reject | At most one attempt/publication; Reject requests cancellation if active |
| Apply | Cancel, mode departure, close, teardown | Native publication winner; no partial history |
| Apply | Undo/redo/Manual mutation | Source serial/epoch authority; no implicit rebase |
| Completion | New request/owner | Owner/epoch filter, refresh ordinary truth only |
| Old cancellation | Fresh execution | Pending cancellation blocks new admission |
| Rapid mode/request churn | Provider BUSY | Bounded rejection; no queue or authority transfer |

Tests use existing native rendezvous, queue barriers, semantic Compose/XCTest
queries and native outcome assertions. Sleeps/coordinates are not proof of ordering.

## Parity, security and persistence

Both platforms: IDLE, REQUESTING, advice/review available, APPROVING/EXECUTING,
pending cancellation, COMPLETED, REJECTED, STALE, CANCELLED, FAILED have equivalent
meaning. COMPLETED requires authoritative valid native receipt. Unknown nonzero
status fails safely. Invalid/stale/provider/resource failures never disable Manual.
Manual before/after AI, shared undo/redo and branch-after-undo remain first-class.

Threats: lifecycle bypass; stale resurrection; replay/double Apply after recreation;
old cancellation; cross-document/owner confusion; callbacks after destruction;
mode leakage; spoofed/malformed completion; capability persistence; publication vs
teardown; release-hook leakage. Native validation and ownership remain authority.
No pending review/approval/candidate/provider/native pointer/cancellation capability
is persisted. Schema 3 unchanged. Release scans exclude test rendezvous/debug
launch authority/fake human Apply/direct commit/provider bypass. No new test hooks.

## Frozen boundaries

Intent/Structured Intent/Plan/Registry/Envelope/Provider/AI Assist/Auto Proposal/
Review Approval v1, reference planner, Phase 6 approval/candidate/cancellation,
manual/history, ADR 0009, Schema 3 and Phase 5 trust unchanged. No v2.
Production model UNRESOLVED; U2NETP unapproved/unincorporated/rights unresolved.
CPU VALIDATED; Vulkan/Metal UNKNOWN. No model/runtime/weights/NLP/cloud/network AI.
DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED; hosted software evidence
is not physical certification. Steps 7-8 NOT STARTED; Phase 8 NOT STARTED.
Step 7 alone owns later production intelligence admission. STOP after Step 6.
