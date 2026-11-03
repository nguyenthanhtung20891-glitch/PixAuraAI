# Result Review v1

Status: Phase 8 Step 5. RESULT REVIEW != NEW HISTORY.

A successful explicit HUMAN Apply already publishes one ordinary revision through
the existing approval, private candidate, deterministic editor and shared history.
Result Review only reads that ordinary history. It does not create a revision,
an AI history class, an approval, a candidate, a preview ticket or a publication.

## Binding

`APPLY_SUCCEEDED` requires the existing authoritative publication receipt and a
matching ordinary revision. Preview success, a UI callback, animation completion,
a provider response or platform optimistic state is not success.

States:

- `NONE`: no receipt and only the root revision. Nothing was applied.
- `CURRENT`: the receipt's resulting revision is the live ordinary revision and its parent is the receipt source.
- `STALE`: the receipt revision is in this document's ordinary history but is not the live revision, or the parent does not match the receipt source.
- `UNCONFIRMED`: no usable receipt while ordinary history has more than the root. This does not claim Apply success and does not claim zero revisions.
- `ABSENT`: a well-formed receipt names a revision that is not in this document. Live ordinary history is still reported.

Flags report previous, undo, redo and publication visibility from native history.
`can_undo` and `can_redo` follow the existing editor rule: no selected manual gesture,
a parent for undo, and a non-empty redo stack for redo.

## Before / after result

`BEFORE RESULT` is the parent of the bound ordinary revision. `AFTER RESULT` is
that committed revision when it is current, otherwise the live committed revision
is reported separately. A preview buffer, preview ticket or 32-byte preview-looking
identifier is not a revision and cannot be rendered as a result.

The display may reuse pixels only after a fresh native query confirms the same
document, live revision, count and undo/redo flags. One displayed bitmap is
replaced on toggle. Geometry changes use fitted comparison, not a pixel-aligned split.

## History

Undo and redo call the existing editor history actions and then re-query. They do
not reverse Plan operations in the UI, regenerate a provider request, or clear the
publication receipt as part of the history action. After undo, a new Manual or
approved edit follows ordinary branch-after-undo rules. The previous Result Review
binding becomes stale and cannot render until re-queried.

A native publication that is visible in ordinary history stays authoritative if a
later transport or UI error arrives. The UI refreshes history. It does not retry
or undo automatically. Cancellation that wins before publication leaves no result
revision. A publication that already won stays visible if a later cancel request
arrives.

Successful Apply permanently consumes the pre-Apply review lineage. A new review
requires a new explicit request or ordinary Manual interaction.

## Boundary

The C ABI is read-only. Query accepts a null receipt. Render evaluates one
committed revision through the existing working evaluation and preview projection,
copies pixels into caller memory, and releases the native image before return.
Render is busy while an agent call, manual gesture, proposal or review preview
is active. Query and render recheck owner, document and revision identity and
fail closed on mismatch. No queue, cache, worker, image allocation retained by
the review, or second raster engine is introduced.

Android and Apple present the same native history. They do not calculate undo,
redo, branch or publication semantics independently. Result Review has no
approval or candidate authority.
