# AI-Assisted MVP v1

Normative Phase 7 Step 1 interaction/authority contract, version 1. Documentation
only: no runtime state machine, Assistance Provider, proposal wire schema or UX
is implemented by this step. Phase 6 CLOSED / COMPLETE / FULL PASS at
`523ffcad47bddd4c13a2615a76f211a6bd38efe2` is the frozen foundation.

## Product modes and independent authorities

AI-first, never AI-only. Human is DIRECTOR / APPROVER and may be manual OPERATOR.
AI is ADVISOR / PLANNER, and OPERATOR only after trusted explicit human approval.
AI MAY PROPOSE. AI MAY NOT APPROVE ITSELF. AI MAY NOT SILENTLY COMMIT.

| Mode | Product meaning | Authority |
| --- | --- | --- |
| MODE_MANUAL | MANUAL: human directly operates existing tools; no provider/proposal required | Existing manual engine/history validation |
| MODE_ASSIST | AI ASSIST: advisory-first; inspect, ignore, partially follow or reject advice; precise manual control remains | Advice grants no mutation or approval authority |
| MODE_AUTO | AI AUTO-EDIT: automatic proposal/planning, never automatic approval | Exact validated Plan v1 still requires trusted explicit HUMAN approval |

These are semantic names, not new ABI enum values or persisted strings. Existing
shell Manual/Assisted/AI labels are presentation placeholders; Step 1 does not
rename runtime cases or assert implemented product flows. Mode, interaction state,
document state and execution authority are DISTINCT. A mode/state label, provider
claim or UI success animation never proves a document revision or native authority.
All modes share one immutable original, document, source generation, current
revision, history and ordinary undo/redo/branch semantics.

## One authority chain

User interaction/request → Assistance Provider → UNTRUSTED Assistance Proposal →
existing trusted validation / intent / plan boundary → validated Plan v1 →
user-facing review → trusted explicit HUMAN approval → existing Phase 6 private
candidate/execution authority → existing deterministic editing engine → at most
one ordinary non-destructive committed revision → shared history / undo / redo.
Successful publication creates exactly one revision; failures create none.

The provider NEVER receives an approval issuer, editor/context pointer, execution
callback or direct revision-publication capability. The UI is not an independent
editing authority: trusted application code represents the human confirmation,
then calls the existing native owner, which repeats all checks. As frozen in
Phase 6, the native boundary cannot authenticate a physical button press and is
not a sandbox against malicious native host code. Model data is always untrusted.
No provider → JNI → revision, model → C ABI → commit, or UI → history shortcut.

The broad initial product sketches are not approval-order authority. In this
contract, HUMAN approval precedes Phase 6 execution admission. Review does not
create/render a new native execution candidate. No new preview/publish authority
is implied. Existing Intent v1, Structured Intent v1, Plan v1, Registry v1,
reference.deterministic.v1, approval/execution v1 and candidate/platform-security
v1 remain unchanged; no contract version bump, new tool or public execution ABI.

## Replaceable intelligence and proposal boundary

An Assistance Provider is a replaceable source of advice/proposals, not an
inference provider or execution owner. Future deterministic/reference, approved
on-device, specialized vision or composite implementations must all terminate at
the SAME untrusted boundary. Replacement cannot change document/manual/history,
approval, candidate, tool or platform authority. No vendor/model/prompt/tokenizer
enters core Plan v1 semantics. Phase 5 providers remain a separate layer.

Future Assistance Proposals MUST be ephemeral, untrusted, bounded, inspectable,
rejectable and invalidatable; bound to trusted project/document/source/revision/
session/context/generation and the application-owned active request. Provider
identity comes from the selected trusted provider owner, not a self-asserted
payload label. Provider output cannot claim current state, capability, approval,
trust or success. Late/wrong-owner replies cannot replace current interaction.

Step 2 owns Assistance Provider & Proposal Envelope v1: exact binary/JSON fields,
encoding, identifiers, sizes, counts, ownership and bounded parser rules are NOT
frozen here. Those explicit limits must exist before any implementation accepts
provider bytes. No recursive/unbounded output, executable code, scripts/shaders,
arbitrary host/filesystem/network tools or hidden reasoning storage is permitted.
Reuse Phase 6 ceilings without raising them: Plan v1 1–8 actions and <=8192 bytes,
existing combined handle/resource limits, one native candidate and no queue.
Provider/UI metadata must not be inserted into Plan v1 or document Schema 3.

The eleven frozen tools remain crop, quarter rotate, exposure, brightness,
contrast, highlights, shadows, saturation, temperature, sharpen and blur. Advice
may reference these only through existing stable IDs/versions and Registry v1;
unknown tools/versions/parameters reject. No clamp, guessed defaults, substitution
or new subject/generative capability. No production model is selected here.

Free-form Intent v1 remains non-interpreted. Structured Intent v1 has exact
machine semantics; the reference planner still maps its eleven goals one-to-one.
Future free-form understanding needs a separately authorized/certified provider.
Step 1 adds no NLP or translation of goal prose into editing operations.

## Finite interaction states

States describe a bounded ephemeral interaction, not document or approval state.
An interaction is associated with its originating mode and trusted owner; it
cannot be rebound to another document, source, context or mode.

| State | Meaning |
| --- | --- |
| IDLE | No active AI request; manual editing is independently available |
| REQUESTING | Explicit assistance request outstanding; no editing authority |
| PROPOSAL_AVAILABLE | Bounded untrusted provider reply received; not an approvable plan |
| REVIEWING | Inspect validated advice OR an exact native-validated/registry-bound plan; these kinds cannot be confused |
| APPROVAL_REQUIRED | Exact validated plan available for human confirmation; state itself grants no approval |
| EXECUTING | Existing native execution admitted after human approval; approval consumed under Phase 6 rules |
| COMPLETED | Native receipt proves successful ordinary revision publication |
| REJECTED | User rejection or invalid/unacceptable proposal; no execution publication |
| CANCELLED | Pre-execution abandonment, or native cancellation confirmed with no publication |
| STALE | Authoritative binding invalidated; never implicitly rebased |
| FAILED | Provider/transport or native failure; no successful publication |

REVIEWING advice is non-executable. Only REVIEWING an exact validated plan can
lead to APPROVAL_REQUIRED. COMPLETED is not a synonym for provider success or
advice acceptance. Advice followed manually returns to IDLE; the user's later
manual operations follow their ordinary explicit manual command/history path.

Allowed transitions below are normative. A set in the From column is shorthand
for each listed state, not another state or wire format. Unlisted events/states
fail closed, leaving document/history unchanged.

| From | Event | To | Required guard / effect |
| --- | --- | --- | --- |
| IDLE | explicit_request | REQUESTING | MODE_ASSIST or MODE_AUTO; trusted current source/request owner; no automatic launch request |
| REQUESTING | provider_reply | PROPOSAL_AVAILABLE | Current owner/request match and bounded envelope admission; still untrusted |
| PROPOSAL_AVAILABLE | advice_validated | REVIEWING | MODE_ASSIST; safe bounded advisory content only; no plan/approval inferred |
| PROPOSAL_AVAILABLE | plan_validated | REVIEWING | Existing native schema/binding/Registry/parameter checks all pass; MODE_AUTO or explicit Assist-to-plan conversion |
| REVIEWING | follow_manually | IDLE | Advice only; clear advisory interaction; no automated settings application |
| REVIEWING | explicit_advice_to_plan_request | REQUESTING | Fresh explicit user choice; same untrusted boundary; not an approval |
| REVIEWING | inspect_exact_plan | APPROVAL_REQUIRED | Plan kind only, exact canonical bytes and trusted live binding; inspect tool IDs, versions, parameters and order |
| APPROVAL_REQUIRED | human_apply_admitted | EXECUTING | Explicit HUMAN Apply, native approval succeeds and native execution admission consumes it; UI cannot assert this transition |
| APPROVAL_REQUIRED | busy_before_start | APPROVAL_REQUIRED | RESOURCE/BUSY; no queue/retry; no approval consumed; release unused approval and require fresh explicit Apply |
| REQUESTING, PROPOSAL_AVAILABLE, REVIEWING, APPROVAL_REQUIRED | user_reject | REJECTED | Invalidate interaction and revoke/release unused approval; ignore late replies |
| REQUESTING, PROPOSAL_AVAILABLE, REVIEWING, APPROVAL_REQUIRED | user_cancel | CANCELLED | Invalidate request and unused authority; zero document mutation |
| REQUESTING, PROPOSAL_AVAILABLE, REVIEWING, APPROVAL_REQUIRED | source_invalidated | STALE | Trusted source/revision/session/context/generation change; no rebase |
| PROPOSAL_AVAILABLE, REVIEWING, APPROVAL_REQUIRED | invalid_proposal | REJECTED | Malformed/unknown/invalid data never becomes approvable |
| REQUESTING, PROPOSAL_AVAILABLE, REVIEWING, APPROVAL_REQUIRED | provider_or_transport_failure | FAILED | Clear pending AI interaction; manual editing remains usable |
| EXECUTING | cancel_or_mode_or_stop_requested | EXECUTING | Request existing native cancellation/stop as applicable; do not announce CANCELLED before native authority settles |
| EXECUTING | native_published_receipt | COMPLETED | Exactly one committed revision; publication won; later cancellation does not undo or relabel it |
| EXECUTING | native_cancelled_no_publish | CANCELLED | Native cancellation fence won; approval remains consumed |
| EXECUTING | native_stale_no_publish | STALE | Native source serial/ABA or repository epoch check rejected publication; consumed approval stays consumed |
| EXECUTING | native_failed_no_publish | FAILED | Existing allocation/tool/resource/repository failure atomicity; no partial history; consumed approval stays consumed |
| COMPLETED, REJECTED, CANCELLED, STALE, FAILED | begin_new_interaction | IDLE | Old interaction is terminal forever; new logical interaction, fresh trusted binding; not resurrection/retry |

Human Apply is separate from approval creation and execution: the native APIs
remain separate. Approval alone has zero document effects. Pre-start failure
preserves the frozen native unconsumed rule; the application releases/revokes
unused approval before a later fresh confirmation. Stale/invalid/admission errors
map to their existing safe terminal outcome; BUSY is never an automatic loop.
There is no model confidence, timer, default, advice click or mode switch approval.

## Mode switching, cancellation and lifecycle

A requested mode switch never clones a document/history, commits, discards durable
manual edits, grants approval or transfers an interaction. Before execution:
abandon the old request/proposal/review, invalidate response authority and revoke/
release unused approval; the target mode starts a new IDLE interaction. Old
terminal interactions cannot be resurrected, even when switching back.

During EXECUTING, request existing native cancellation and drain the owned call
before treating the old interaction as terminal. Publication/cancel authority is
the Phase 6 fence, not a UI mode flag. If cancellation wins: no revision. If
publication already won: record COMPLETED and the ordinary revision; the switch
cannot hide, roll back or relabel it. Never promise cancellation merely because
the user selected a mode. A new AI request cannot queue behind active work.
Manual editing does not wait for a provider response or require provider teardown;
existing native admission/BUSY and manual-source invalidation remain authoritative.
Presentation may show the requested mode without claiming old work has stopped.

Manual edits, undo, redo, project/source change and context replacement invalidate
bound AI work under the existing rules. Same visible revision after undo does not
restore old authority: nonreused source serial/generation and repository epoch CAS
remain mandatory. No duplicate capability/source epoch is introduced here.
Late replies and completion events require the owning request/context match and
must not repopulate a discarded review or mint approval. A valid native published
receipt is reconciled with ordinary history even if its presentation was abandoned.

Provider shutdown cannot mutate manual state or own native execution. Once a
validated plan executes, no continuing model/provider participation is required;
a provider failure cannot invent a native cancellation/publication result.
Owner destruction follows ADR 0009 stop -> join ALL calls -> destroy, including
provider callbacks and native calls before freeing referenced storage. No unmanaged
worker, thread kill, retained invalid pointer or callback into destroyed UI.

## Manual compatibility and graceful degradation

Manual editing works at launch, offline, without AI provider/model/session or
network. Provider unavailability/failure disables assistance only, never photo
editing. Advice can be ignored, rejected or partially followed without effects.
The user may edit manually after AI publication, undo that ordinary revision, redo
where ordinary history allows, and branch after undo under existing semantics.
No AI-only revision class, history owner, durable label or second editing engine.

Invalid, rejected or stale proposals leave the document unchanged. Execution
failure before publication leaves no partial durable revision. Cancellation uses
existing native atomicity and the winner rules above. Retry after execution starts
requires a new request/review and fresh explicit human approval; no hidden replan,
recipe fallback, reapproval, approval replay or post-start alternate execution.

## Persistence, privacy and platform boundary

AI interaction state is ephemeral by default. Schema 3 unchanged. No persistence
of pending requests/proposals, approvals/candidates, provider handles, capability
epochs or hidden reasoning. Only ordinary committed revision/history is durable.
Existing shell preference restoration is presentation only and cannot restore AI
authority. Future user-facing audit metadata needs a separate reviewed contract
and can never become execution authority.

Local-first: NO network requirement, cloud AI dependency, editing-authority analytics
dependency or remote approval service. No network permissions/endpoints, image/
metadata upload, credential/shell/environment access or generic plugin execution.
Future cloud architecture requires separate explicit authorization. Diagnostics
must be bounded and omit raw images/goals, authorization tokens and native addresses.

Android and Apple may present native mode UX and transport requests/results. They
do not duplicate provider validation, tool math, plan binding, approval validity,
execution or history semantics in Kotlin/Swift. Native status meaning remains
authoritative; unknown nonzero outcomes fail safely, never become success/retry.
Phase 4 capability policy and Phase 5 provider trust remain separate authorities.

## Threat analysis and required protections

| Threat | Required protection |
| --- | --- |
| A authority bypass | Provider/proposal/UI never execution authority; exact native validation, HUMAN approval, admission and publication required |
| B forged/malformed output | Bounded untrusted envelope, trusted provider owner, strict version/tool/parameter validation; Step 2 freezes concrete parser |
| C stale proposal | Exact native source/revision/context binding; manual/undo/redo ABA invalidation; no rebase |
| D approval confusion/replay | Distinct typed native authority, exact canonical plan binding, single-use consumption; no approval in payload |
| E mode-switch leakage | Abandon ephemeral work, revoke unused approval, fence/drain started work; no transfer/resurrection |
| F platform mismatch | Same native authority/statuses; no independent Kotlin/Swift semantic engine |
| G owner lifetime | Stop, join all calls/callbacks, destroy; reject late/wrong-owner events |
| H cancel/publish race | Existing native cancellation/publication fence and repository CAS determine one winner |
| I resource exhaustion | Explicit future envelope bounds before acceptance; existing plan/handle/candidate ceilings; no queue |
| J test-hook leakage | Retain production object/app scans; no synthetic provider/approval authority in release |
| K privacy exposure | Local-first/no uploads or network tools; bounded non-sensitive diagnostics; no hidden reasoning persistence |

No unresolved category A architecture issue. These are normative future acceptance
requirements, not invented runtime PASS claims. Later steps must certify their
actual provider/UI implementations against this contract and all retained gates.

## Phase boundaries and deferred intelligence

Phase 7 Step 1 ONLY defines this contract/state machine and governance checks.
Step 2 owns Provider/Proposal Envelope; Steps 3–6 own the authorized-later UX and
lifecycle integration. Production Intelligence Admission & MVP Provider selection
is DEFERRED to Phase 7 Step 7, not preselected by this contract. Candidate evidence
must cover quality, repeatability, latency, peak memory, package size, battery/
thermal behavior, Android/Apple hardware coverage, trust, license/checkpoint
redistribution rights, provenance, privacy/offline capability and failure/fallback.
This is an evaluation requirement, not approval of any model or runtime.

Production model UNRESOLVED. U2NETP audited, unsupported by the frozen profile,
unapproved, unincorporated; checkpoint rights unresolved. No model/runtime/weights,
NLP, prompt/chat UI, cloud/network AI, new tools, autonomous background editing,
public execution ABI, export or licensing/subscription work in Step 1.
cpu.reference.v1 VALIDATED; android.vulkan.v1 UNKNOWN; apple.metal.v1 UNKNOWN.
No provider CERTIFIED. DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED.
Hosted Apple/simulator evidence cannot satisfy physical Metal certification.

Phase 8 FULL REVIEW EXPERIENCE remains NOT STARTED. Later Phase 7 may build only
minimum review/apply/reject necessary for the MVP. Advanced before/after comparison,
full inspection workspace, multi-candidate review, elaborate timelines and
production export workflow remain later scope. This contract authorizes none.
Phase 7 Steps 2-8 NOT STARTED. No Step 9. STOP after Step 1.
