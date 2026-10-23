# Manual + AI Assist Experience v1

Phase 7 Step 3 adds advisory presentation over the certified Phase 6 editor and
Phase 7 Assistance Provider & Proposal Envelope v1. It does not add intelligence,
tool semantics, approval, execution or persistence authority.

## Manual and modes

MANUAL remains first-class: launch, import, select tools, change values, Apply,
cancel, undo and redo require no Assistance Provider. AI ASSIST uses the same
document, editor owner and ordinary history. AI AUTO-EDIT is visibly disabled;
Steps 4–8 and Phase 8 are NOT STARTED. Changing mode alone never edits a document.

AI ASSIST exposes eleven explicit requests, derived by native identity lookup from
the existing Structured Intent v1 goal table and agent-exposable manual registry.
The request amount is shown before Request advice. The reference provider does
not analyze the image or interpret language. These are explicit requests, not
inferred recommendations. The choices are:

| Goal | Explicit request |
| --- | --- |
| adjust.brightness | milli_linear 100 |
| adjust.contrast | milli_stops 250 |
| adjust.exposure | milli_ev 500 |
| adjust.highlights | milli_ev -500 |
| adjust.saturation | milli_ratio 1200 |
| adjust.shadows | milli_ev 500 |
| adjust.temperature | kelvin 7000 |
| detail.blur | milli_strength 250 |
| detail.sharpen | milli_amount 250 |
| transform.crop_normalized | height_ppm 800000, width_ppm 800000, x_ppm 100000, y_ppm 100000 |
| transform.rotate_quarter | quarter_turns 1 |

These amounts are UI request choices, not a second parameter schema. Existing
native Structured Intent/Plan/Registry validation alone decides validity. No
clamping, defaults, substitution, repair, heuristic or free-form interpretation.

## Advice flow and interaction state

Choose goal -> Request advice -> native reference provider -> UNTRUSTED ADVICE
Envelope v1 -> existing shared-native validation -> bounded presentation data.
The platform never parses an envelope. Advice has no Plan output or authority.

| Mode/state | Event | Result |
| --- | --- | --- |
| MANUAL/IDLE | Enter Assist | ASSIST/IDLE, no request |
| ASSIST/IDLE | Explicit goal + Request | ASSIST/REQUESTING |
| ASSIST/REQUESTING | Valid result, current source/owner/UI epoch | ASSIST/PROPOSAL_AVAILABLE |
| ASSIST/REQUESTING | Invalid/provider/resource failure | ASSIST/FAILED; Manual available |
| ASSIST/REQUESTING | Cancel | ASSIST/CANCELLED; drop late result |
| ASSIST/PROPOSAL_AVAILABLE | Dismiss | ASSIST/REJECTED; no edit |
| ASSIST/PROPOSAL_AVAILABLE | Use suggestion | MANUAL/IDLE; existing manual tool opens |
| ASSIST/REQUESTING or PROPOSAL_AVAILABLE | Source/manual/history change | ASSIST/STALE; discard |
| Any ASSIST state | Leave mode or owner | MANUAL/IDLE; abandon/cancel ephemeral state |

These interaction states are distinct from document state and execution authority.
No transition creates approval or consumes one. No automatic retry/re-request.
Returning to Assist starts safely without resurrecting an old recommendation.

## Use suggestion and manual reuse

**Use suggestion is navigation-only, NOT Apply.** It rechecks live native advice
then calls the existing manual tool-selection API. The tool opens at its ordinary
default; the advice amount is not applied or prefilled. No update, render,
candidate, approval, Agent execution or history call occurs in this action.
The human chooses a value using the SAME existing manual controls, may change
tools or Cancel, and separately presses the existing Apply button to commit an
ordinary manual revision. The UI explains this distinction. There is no AI tool UI.

## Shared-native bridge and bounds

`pixaura/assist.h` is the narrow version-1, advice-only presentation adapter:
`pixaura_assist_choice`, `pixaura_editor_assist_request`,
`pixaura_editor_assist_current`, `pixaura_editor_assist_cancel`.
The existing internal Step 2 provider has no platform ABI; these delegates are
necessary for typed JNI/Swift transport without duplicating envelope validation.
They expose no generic provider invocation or execution entry point.

Request is 16 bytes (version, size, choice index, reserved zero). Presentation is
88 bytes (version, size, choice/tool index, parameter count, reserved zero,
four exact int32 values, 32-byte context ID, uint64 request/source serials).
Pointers, alignment, ranges, nonoverlap, sizes, versions and indices are checked;
exceptions never cross C ABI. Errors preserve output buffers. No new handles.
Catalog has exactly 11 entries, zero live stamps. A catalog entry is not advice.
One active request and one private current advice record per editor, no queue.
Envelope 12288-byte, payload 8192-byte/structured-intent 2048-byte limits and all
existing context/handle/raster budgets remain unchanged. Parameter count <=4.

## Freshness, cancellation and lifetime

Provider receives data only, never editor/approval/candidate authority. Request
captures native immutable snapshot/context, existing nonwrapping live source
serial and repository epoch. On completion native rechecks source/epoch, manual
gesture/admission and cancellation serial under the editor gate. Inspection
requires exact cached record, same owner/context/source/epoch and no manual
gesture. Forged, altered, foreign, cancelled and stale records reject.

Manual selection/update/Apply/cancel/history action, source replacement and owner
stop invalidate advice. Undo returning to an earlier-looking revision does not
restore validity (source serial/epoch ABA fence). UI state also has a nonwrapping
epoch; callback publication requires matching epoch, owner and Assist mode.
Cancellation does not touch history; later requests can succeed. Duplicate
requests are rejected/bounded, not queued. Invalid/advice-failure status never
enters the manual error/availability authority. Resource or sequence exhaustion
fails assistance closed, leaving Manual available.

Android ViewModel owns one bounded advice worker alongside existing manual lanes.
Apple EditorWorker has one advice lane. No work runs on UI threads. Native wrapper
close fences stop, joins all render/Agent/advice calls, then destroys (ADR 0009).
Mode departure, background interruption and screen closure invalidate UI epochs;
late results cannot republish. Recreation reopens ordinary durable state only.

## Status parity, accessibility and privacy

Both adapters preserve native statuses: 0 success; 1 invalid arguments; 2 unsupported
ABI; 3 invalid owner; 5 unknown goal/tool; 6 malformed; 7 invalid parameter;
8 BUSY/resource; 9 STALE; 13 CANCELLED/stopped; 14 internal. Unknown failures
produce safe assistance failure, never success, retry or approval.
Native owns semantic validation, Kotlin/Swift only fixed typed data/presentation.

Stable accessibility IDs/tags cover mode-selector, mode-manual/assist/auto,
assist-goal and each goal, assist-request/loading/cancel/recommendation/use/dismiss,
assist-error/stale/cancelled/dismissed. Ordinary manual controls remain accessible.
Compose tests use semantics; XCTest uses specific-element tap, no coordinate or
frame/window snapshots. Bounded static local text is presentation, not authority.

Ephemeral state is never Schema 3, preferences, history or a proposal database.
No pending advice/provider authority survives restart. No network, account,
telemetry, model, NLP, weights or remote approval. No token/address logging.

## Security and frozen boundary

Certification covers read-only advice, Use suggestion without commit, malformed
ABI/proposal, wrong owner, stale/ABA/manual/undo/redo, cancel/request/stop races,
BUSY, duplicate requests, lifecycle recreation, late UI suppression and production
test-hook isolation. Test checkpoints/faults exist only in certification/debug
objects and must fail release scans if leaked.

Intent v1, Structured Intent v1, Plan v1, Registry v1, Proposal Envelope v1,
Assistance Provider v1, reference planner, approval/candidate semantics, shared
history/manual tools and Step 1 interaction contract remain unchanged. No approval
issuer/candidate/execution path is added. Step 4 owns future Auto-Edit proposal
flow; Step 5 owns review/approval UX, neither implemented here. Phase 8 NOT STARTED.
Production model UNRESOLVED; CPU VALIDATED; Vulkan/Metal UNKNOWN.
DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED.
