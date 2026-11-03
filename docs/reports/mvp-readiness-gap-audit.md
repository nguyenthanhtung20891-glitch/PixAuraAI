# MVP readiness gap audit

Observed after Phase 8 closure `029f84eaa86f3e5a8bffc696a6abd10f9477ae2a`.
Foundation 41709039261 SUCCESS 7/7 and Native application shells 41709039298
SUCCESS 3/3, exact SHA, attempt 1, every required job SUCCESS. The receipt is
`docs/reports/evidence/phase-8-step-6-closure-certification.json`.

This audit inspects the current native and platform code. A missing name is not
treated as missing behavior, and an existing name is not treated as a finished
product surface.

## Journey

| Step | What exists | Gap class |
| --- | --- | --- |
| Open / import a photo | Android `ActivityResultContracts.OpenDocument` calls `ManualEditorModel.importImage`. Apple `fileImporter` accepts JPEG and PNG and calls `ManualEditorModel.importImage`. Native `pixaura_editor_open_policy` copies source bytes into private storage and records the import revision. | Present |
| Manual / Assist / Auto | Manual tools, non-mutating Assist, and Auto proposal through the untrusted provider are present. Use suggestion stays navigation-only. Apply stays an explicit human action. | Present |
| Plan review | Native inspection prints Registry order, raw integers, units, and bounds. | Present |
| Before/after preview | Native preview projection with before, after, and split. Labels say the after image is not committed. | Present |
| Optional edit review | Bounded value, remove, move, and restore create a new Plan and review identity. Restore is labeled as review content, not document undo. | Present |
| Human Apply | Existing single-use approval, private candidate, and one ordinary revision. | Present |
| Ordinary history / result review | Result Review binds the publication receipt to ordinary history and uses editor undo/redo. | Present |
| Export / save / share | No production export, destination writer, or share action was found in native core or either platform UI. | BLOCKING MVP |

## BLOCKING MVP

Export of a chosen committed revision is not implemented. The reviewed journey
ends after Result Review. Export must be a deterministic projection of an exact
committed revision, must not create a revision, and must not require the
Assistance Provider. That work is the Phase 9 file-lifecycle gap. It is not
started in this audit.

## IMPORTANT NON-BLOCKING

- Android can reopen the last ordinary project through `openLast`. A first-run
  empty state is only the existing manual screen, not a separate onboarding flow.
- Source metadata stores orientation. This audit did not find a separate user
  control for reinterpreting EXIF, output dimensions, or JPEG quality, because
  there is no export surface yet.
- Permission denial is the platform picker’s own cancellation. There is no
  additional product explanation screen.
- Localization is not a product catalog. Strings are inline.
- Hosted performance numbers exist. Physical battery, thermal, and memory-pressure
  evidence does not.

## POST-MVP

- Free-form language understanding, scene understanding, and arbitrary-photo
  diagnosis. Free-form Intent v1 stays uninterpreted.
- A learned provider. `reference.assistance.v1` remains the admitted MVP provider.
  EmbeddingGemma stays disqualified. NIMA, FFCC, and MobileIQA stay unselected.
- Vulkan or Metal promotion. Both remain UNKNOWN. DH-APPLE-METAL-01 stays
  DEFERRED / NOT WAIVED / NOT SATISFIED.
- Cloud upload, remote review, accounts, collaboration, and generative creation.
- A second raster engine, Plan v2, Registry v2, or an AI-specific history class.

## Not gaps

Schema 3 persistence, ordinary undo/redo, and the shared native editing path are
already the authority for Manual, Assist, and Auto. Result Review did not add a
history class. Android and Apple do not calculate revision identity themselves.
