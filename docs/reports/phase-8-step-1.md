# Phase 8 Step 1 — Review Experience Contract & Information Architecture

<!-- phase8-step1-current -->
Phase 7 CLOSED / COMPLETE / FULL PASS. Phase 8 AUTHORIZED / IN PROGRESS.
Step 1 AUTHORIZED / IN PROGRESS. Steps 2-6 NOT STARTED.
Only contract, information architecture and minimal governance plumbing are added.
No production review/preview/edit UI, raster path, bridge or editing authority.
Implementation and separate documentation-only closure both need complete exact-SHA
Foundation 7/7 and Native shells 3/3 certification. Until observed final closure
success, Step 1 is not COMPLETE. Stop after Step 1; do NOT begin Step 2.
<!-- phase8-step1-history -->

## Baseline and scope

Verified after fetch: clean tracked/staged main == HEAD == origin/main ==
`fe01c4a4222eb5bed63adc840b140f73784e1c18`. Phase 7 closure Foundation 40502641648
7/7 and Native shells 40502641654 3/3 independently rechecked: exact SHA, attempt 1,
completed/SUCCESS, every required job SUCCESS. The [baseline receipt](evidence/phase-8-step-1-baseline.json)
records job-level evidence. All Phase 7 closure/history and ignored research artifacts
remain preserved; no historical authority, contract or report is rewritten.

The [Full Review Experience v1](../contracts/full-review-experience-v1.md) and
[frozen six-step roadmap](../plans/phase-8-roadmap.md) define future review behavior.
The [governance manifest](phase-8-step-1-v1.json) is documentation-only, not a runtime
schema, C ABI, persistence format or editing authority. Exact file inventories and
implementation/closure certification are recorded below when observed.
The [implementation inventory](evidence/phase-8-step-1-files.json) lists the exact
changed files; production/native/platform runtime files are unchanged.

## Contract and information architecture

REVIEW != APPROVAL; PREVIEW != EXECUTION; PREVIEW != REVISION; visual comparison
does not mutate documents; UI state is not native authority. The surface separates
context/binding, before/after comparison, ordered action inspection, status/warnings,
explicit decisions and ordinary committed-result review. No final UI is implemented.

One bounded ephemeral Review Session references trusted current document/source/
revision/context/session/generation/serial/epoch, exact canonical Plan bytes, mode,
informational provider origin, current preview ticket and finite presentation state.
It owns no approval, candidate, publication/history authority or durable token.
Plan stays <=8192 bytes/1–8 actions. One current session/request/current display pair;
all superseded work remains accounted until drained, no queue or unbounded cache.
No review/preview authority persists to Schema 3 or platform restoration.

Before means the exact source revision bound to the Plan, not necessarily original.
After preview is a disposable non-publication native projection, never proof Apply
will succeed. Post-Apply result means an ordinary committed revision. Future preview
must reuse native deterministic evaluation, display/resampling and lifecycle rules,
not platform tool math, Agent execution, approval or repository publication.

Presentation states: NO_REVIEW, PLAN_REVIEW, PREVIEW_LOADING, PREVIEW_READY,
PREVIEW_FAILED, STALE, APPLY_PENDING, APPLY_EXECUTING, APPLY_SUCCEEDED, APPLY_FAILED,
CANCELLED. These overlay existing Phase 7/native facts; they are not a second
execution state machine. No path from inspection/preview to Apply exists without
the explicit HUMAN event. Success requires the valid authoritative native receipt.
Cancellation scope distinguishes preview/session cancellation from execution outcome.

Future value/remove/reorder/restore creates NEW INTENT / NEW PLAN CANDIDATE for full
existing native validation -> NEW canonical Plan/review identity -> old review and
preview invalidated. Restore cannot resurrect old approval even for identical bytes.
No assumed commutativity, UI-only parameter validation, empty Plan, hidden repair,
mutable approved Plan or broadened reference-planner goals. Step 4 remains NOT STARTED.

## Existing approval, freshness and lifecycle

Apply stays explicit HUMAN confirmation of the exact current Plan -> existing native
approval -> single-use private candidate/execution -> at most one ordinary revision.
No automatic Apply after preview/appearance/timer/gesture, retry, replan or reapproval.
All existing native source/serial/ABA/epoch/CAS/fence checks remain mandatory.
Source/project/document/context/history changes invalidate review and previews; no
silent rebase. Successful Apply permanently ends old review eligibility.

Before execution, Back/close/mode change abandons ephemeral state and cancels preview.
During Apply, existing native cancellation/publication winner decides; UI departure
cannot promise cancellation, hide/undo publication or transfer authority to a new owner.
Stop -> join ALL calls -> destroy remains ADR 0009. Owner/request/source/exact Plan
fences suppress late installs, including the gap between current-check and assignment.
Process recreation restores ordinary document/history only; never resumes old Apply.

Malformed/stale/unsupported Plans fail closed. Preview allocation/cancellation/compute
failure leaves Manual and valid Plan inspection usable. Retry is explicit with a new
preview ticket. BUSY/approval failure does not auto-retry Apply. Native cancellation
confirmed before publication means zero revision; publication win means one ordinary
revision. Malformed receipt/transport failure can happen after publication: refresh
ordinary native history, show outcome unconfirmed and never infer zero or retry.

## Resources, platforms, accessibility and privacy

Future Step 2 must prove actual peak accounting against existing applicable Budget
Admission PREVIEW, evaluation and preview limits. Count retained source, working/
scratch/display/copies and draining work; protect Manual/platform headroom. No new
numeric memory ceiling, RSS-derived lease, heavyweight inference owner or scheduler.
Resource failure disables optional preview, not Manual or ordinary history.

Android and Apple share native Plan/Registry/validation/freshness/render/approval/
execution/history semantics. Platforms own bounded presentation and transport only.
No Kotlin/Swift implementation changes. Future semantic accessibility identifies
source, actions/values/units, comparison state, preview/loading/error/cancel, distinct
HUMAN Apply/Reject/Back and result revision. Before/after and stale/error use text and
announcements, not color alone; loading cannot indefinitely block Manual. Coordinate-
only automation is insufficient. Toggle/split/synchronized viewport/reset are future
requirements only; native crop/rotation mapping must avoid false pixel alignment.

Result review is ordinary previous/current revision comparison, with ordinary undo/
redo/branch behavior and truthful history refresh. No AI history class/lineage.
Local/offline: no upload, cloud preview, screenshot telemetry, analytics editing
dependency, external key or hidden review persistence. No new learned/NLP/chat/
arbitrary-photo diagnosis. reference.assistance.v1 stays ADMITTED; planner remains
reference.deterministic.v1. Learned model NONE REQUIRED / NONE ADMITTED FOR PHASE 7 MVP;
future provider DEFERRED / REPLACEABLE. EmbeddingGemma disqualified; NIMA/FFCC/MobileIQA
unselected. No research reopened.

## Threat analysis and verification boundary

| Threat | Contract protection | Step 1 / future verification |
| --- | --- | --- |
| A preview confused with result | Separate BEFORE/AFTER PREVIEW/committed labels | Contract frozen; Step 2/5 UI assertions later |
| B stale preview current | Native binding + owner/request install fence | Existing lifecycle unchanged; new preview races Step 2 |
| C preview/Plan mismatch | Exact canonical Plan/source/config association | Contract graph; Step 2/3 changed bytes/order tests later |
| D edited review reuses approval | New validation/Plan/review identity, old invalidated | Governance invariant; new edit/replay tests Step 4 |
| E UI values differ from bytes | Native-derived exact values/units/order | Contract; Step 3 semantic/locale assertions later |
| F cross-document cache | Ephemeral bound pair, no foreign reuse | Contract; Step 2 owner/document/recreation tests later |
| G exhaustion harms Manual | Existing admission/headroom, bounded optional work | Resource contracts unchanged; Step 2 failure/pressure tests later |
| H platform drift | Shared native semantics | Retained platform suites; new parity Step 6 |
| I unintended Apply | Explicit HUMAN event only, separate controls | Graph excludes preview-to-Apply; future UI controls tests |
| J callback resurrection | Stop/join, request/owner/source/Plan fence | Existing lifetime tests; new preview tests Step 2 |
| K hidden publication | Native receipt/winner and ordinary history refresh | Existing Phase 7 retained; result-review tests Step 5 |

No new A-class architecture conflict identified. Future preview/UI security behavior
is NOT IMPLEMENTED / NOT VERIFIED in Step 1; governance checks certify requirements,
not future runtime. B: later steps must prove peak preview admission, geometry mapping,
cache/late-install fences, editable validation routing and real platform/accessibility
behavior before admission. C: optional visual polish remains Step 6 scope, not now.
No waiver or claimed physical-device certification.

## Frozen comparison and tests

All 222 previously frozen production/contract/build-input files retain their Phase 7
hashes. Existing Phase 7 closure documents/manifests and all 53 contracts remain
unchanged. Exactly one authorized additive contract is exempted from the historical
inventory's no-addition check; every original hash and the full runtime inventory
remain enforced. New governance tests also pin the unchanged Phase 7 manifest hash
and the new contract hash. This is an explicit additive documentation exception,
not permission for production changes or other contract additions.

Six new governance tests cover the frozen roadmap, graph separation/HUMAN guard,
new editable identity and no session authority, exact baseline/contract identity,
current status/stop boundary and all eleven threat protections. They run in both
retained workflows alongside existing tests. No test weakening, suppression,
timeout increase or rerun-until-green. Full exact implementation and closure SHA
certification remains required. No production UI test is claimed for an unbuilt UI.

Intent/Structured Intent/Plan/Registry/Envelope/Provider/planner, HUMAN approval,
candidate/publication, Schema 3, Manual/history, platform security, ADR 0009, Phase
4/5 resource/segmentation and Intelligence Runtime Resource Admission v1 unchanged.
CPU VALIDATED; Vulkan/Metal UNKNOWN. DH-APPLE-METAL-01 DEFERRED / NOT WAIVED /
NOT SATISFIED. Hosted Apple software evidence cannot satisfy that physical gate.
Stop after Step 1. Phase 8 Step 2 NOT STARTED.

## Executed local validation

- New and retained admission/closure governance: `node --test
  tests/full-review-contract.test.mjs tests/deterministic-admission.test.mjs`, 12/12.
- Complete source command from Native shells workflow, Linux Node 24.14.0 through
  WSL with repository-contained TMPDIR: 186/186, zero skipped, 215.38 seconds.
- Existing actionlint against both workflows: SUCCESS; `git diff --check`: SUCCESS.

No local production preview implementation, physical-device test or new UI test is
claimed. Complete retained native, platform, sanitizer and release scans must pass
the implementation SHA and the later documentation-only closure SHA independently.
