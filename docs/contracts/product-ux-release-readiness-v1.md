# Product UX and software release readiness v1

Status: Phase 10 Step 1 contract. UI STATE != NATIVE AUTHORITY.

This contract freezes the user-facing information architecture for the existing MVP journey. It adds no Plan, Registry, approval, history, raster, or export authority. English copy below is the canonical presentation. Vietnamese is a later presentation of the same meanings. A localized string is never parsed back into a tool, Plan, Registry identity, revision, approval, or export source.

## Journey

Import or open a photo, then Manual, Assist, or Auto, then review and preview, then an optional review edit, then explicit HUMAN Apply, then ordinary Result Review and history, then Export, Save, or Share. Manual editing does not require an Assistance Provider. Export reads one committed revision and creates no revision.

## Surfaces

Each surface below is a state of the existing editor. Phase 10 does not add a second editor, account screen, or cloud destination.

### First launch and empty project

Entry: no document is open. Exit: native import accepts a JPEG or PNG, or the user remains here after cancellation. Visible state: the product edits a private local copy, and Apply and Export are not eligible. Action: Open Photo, whose visible English label remains "Open JPEG or PNG". Native truth: no document and no revision. Invalidation: none. Fallback: a cancelled picker stays on this surface and does not invent a document.

### Photo import and open

Entry: the user starts Open Photo. Exit: one ordinary import revision, or a visible failure with the empty surface still available. Visible states: busy, accepted, cancelled, unsupported, corrupt, or access denied. Native truth: the existing import copies source bytes into private storage and records the import revision. UI copy does not override a native error. Invalidation: a new import replaces the open document and drops ephemeral review, preview, approval, and export state. Fallback: Manual stays available after a successful import. A failed import leaves the previous document unchanged when one was open.

### Main Manual editor

Entry: a document is open and Manual is selected. Exit: another mode, close, or a successful manual commit through the existing editor action. Visible state: the current revision and the selected manual tool. Actions: the existing manual controls and their ordinary Apply. Native truth: the manual editor and ordinary history. Invalidation: source switch, undo, and redo refresh from native history. Fallback: Manual remains usable when Assist or Auto cannot run.

### AI Assist

Entry: Assist is selected on an open document. Exit: return to Manual, or Use suggestion, which only navigates to the existing manual tool at its ordinary default. Visible state: validated advice, or a clear failure. Native truth: advice is not a Plan and not an approval. Invalidation: mode change and source change drop the advice. Fallback: the user can ignore the advice and edit manually.

### Auto proposal

Entry: the user requests one bounded Auto goal. Exit: a review-ready proposal, or a failure that does not change the document. Visible state: the proposal summary. Native truth: the proposal is an untrusted envelope that has passed existing validation. It is not approved. Invalidation: a new request or a source change retires it. Fallback: no Apply control is shown for a failed proposal.

### Plan inspection

Entry: a review-ready Plan is bound to the current source. Exit: preview, review edit, cancel, or Apply of that exact Plan. Visible state: Registry order, raw integers, units, and bounds. Native truth: native inspection of the exact canonical Plan. Invalidation: any Plan identity change retires the inspection. Fallback: a stale or forged inspection fails closed and does not enable Apply.

### Before and After Preview

Entry: the inspected Plan is still bound. Exit: back to inspection, review edit, cancel, or Apply. Visible states: "Before — bound source revision" and "After Preview — not committed". Actions: show before, show after, and split. Native truth: before is the bound committed revision and after is a disposable projection. Invalidation: source, Plan, or geometry change retires the preview. Fallback: preview failure or cancellation leaves history unchanged and does not report a commit.

### Editable Review

Entry: the user edits the reviewed Plan. Exit: a new Plan and review identity, or a validation failure that keeps the old Plan ineligible for the failed edit. Visible state: the edited values and "Restore original review content — not document undo". Actions: change value, remove, move, and restore review content. Native truth: the edit mints a new Plan through existing validation. Invalidation: the previous review cannot be approved. Fallback: NO_ACTIONS is not executable. Restore of equal bytes still receives a new identity.

### Apply states

Entry: the user presses "Apply reviewed plan" for the exact current Plan. Exit: one ordinary revision, or a failure that does not publish. Visible states: busy, succeeded, failed, and cancelled. Native truth: explicit HUMAN Apply, single-use approval, private candidate, and at most one ordinary revision. Invalidation: the approval cannot be reused. Fallback: a transport error after publication refreshes ordinary history and does not claim zero revisions. There is no automatic retry, undo, or reapproval.

### Committed Result Review

Entry: a successful Apply has published an ordinary revision. Exit: back to editing, undo, or redo. Visible state: "Committed result", the previous revision as before, and the current revision as after. Actions: "Undo document" and "Redo document". Native truth: ordinary history. Invalidation: a later manual edit, source switch, or branch after undo retires the old result review. Fallback: result review never becomes a new history class.

### Ordinary Undo and Redo

Entry: history has a reachable revision. Exit: the selected committed revision is current. Native truth: the existing history commands. Undo does not reduce the revision count by inventing a deletion class. Invalidation: export, preview, and result review rebind to the revision native history reports. Fallback: a failed history command leaves the last native revision current.

### Export configuration

Entry: a committed revision is selected and the user opens Export. Exit: Save, Share, cancel, or a validation failure. Visible state: JPEG or PNG, quality or dimensions when the format uses them, and the statement that export does not change history. Native truth: the named document, epoch, and committed revision. Invalidation: a source or revision change makes the operation STALE and writes nothing. Fallback: Apply and Export stay separate controls. Export does not require Assist, Auto, or another HUMAN Apply.

### Save and Share

Entry: encoded bytes exist for the selected revision and the user chooses a destination. Exit: save completed, save cancelled, save failed, share sheet presented, or share cancelled. Native truth: platform destination adapters. Save completed means the destination write finished. "Share sheet closed. Delivery to another app is not confirmed." is not delivery. Invalidation: a second tap is ignored while one export is in flight. Fallback: failure deletes or withholds an incomplete destination and leaves the document unchanged. Another export requires a new explicit user action.

### Errors and cancellation

Entry: a native or platform operation returns failure or cancellation. Exit: the user can read the state and choose a recovery action that starts a new operation. Visible words stay distinct: Cancelled, Failed, and Stale. Native truth: the status code and the document. UI success is not inferred from a stale callback, a dismissed sheet, or an animation. Fallback: stop rejects new work, cancels the scoped operation, joins owned calls, releases resources, and destroys. A callback after destroy cannot become authority.

## Copy

These English strings keep their existing meanings:

- After Preview — not committed
- Apply reviewed plan
- Restore original review content — not document undo
- Undo document
- Redo document
- Committed result
- Export
- Saved
- Share sheet closed. Delivery to another app is not confirmed.
- Share cancelled
- Save cancelled
- Export failed
- Cancelled
- Failed
- Stale

Presentation may translate them. It may not merge Preview with Committed result, Apply with Export, Restore review content with Undo document, or a presented share sheet with confirmed delivery.

## Accessibility and layout

Open Photo, Apply reviewed plan, Export, Save, Share, Cancel, Undo document, and Redo document are named controls. Preview and Apply remain separate. State is not conveyed by color alone. Critical labels remain available under larger text and on narrow and wide windows. Touch targets and focus order follow the platform. No gesture on a preview image is Apply.

## Release boundary

Phase 10 does not add a learned model, network localization, upload, tracking permission, broader photo or storage permission, second raster engine, or new resource ceiling. CPU remains VALIDATED. Vulkan and Metal remain UNKNOWN. DH-APPLE-METAL-01 remains DEFERRED / NOT WAIVED / NOT SATISFIED. Hosted software evidence is not physical-device qualification.

## Threats

| ID | Threat |
| --- | --- |
| A | Preview shown as a committed result |
| B | Apply offered for a stale Plan |
| C | An old approval reused |
| D | A UI value treated as the native Plan |
| E | Result Review bound to the wrong revision |
| F | Export bound to the wrong revision |
| G | A failed save reported as success |
| H | A share sheet reported as delivery |
| I | A broader permission or a sensitive log |
| J | A callback after the owner is destroyed |
| K | Resource pressure disabling Manual |
| L | A translation that changes an authority meaning |
| M | An accessibility action that applies a Plan |
| N | A release artifact that contains test authority |
| O | A native error shown as UI success |
| P | A reopened project restoring ephemeral approval or review |
