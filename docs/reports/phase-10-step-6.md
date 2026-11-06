# Phase 10 Step 6 — Cross-platform release readiness

Status: AUTHORIZED / IN PROGRESS. This commit does not certify itself.
`53b6da5a23dc64dbe248fbe616d2be009a6fd765` is not certified and is not rerun.
Phase 10 is not closed. Phase 11 and Phase 12 are not started.

## Observed Step 5 closure

Implementation `e57f827604b3367c983bd756e6dedf66d5644ef1`: Foundation
`42164977378` SUCCESS 7/7 and Native shells `42164977392` SUCCESS 3/3.

Closure `47c6503c111feaa5f21c4f732036490a023107f0`: Foundation `42168863992`
SUCCESS 7/7 and Native shells `42168864184` SUCCESS 3/3, attempt 1, exact SHA.
The receipt is `docs/reports/evidence/phase-10-step-5-closure-certification.json`.

## Journey parity

Android and Apple use the same native editor. The release gate checks the
shared sentences for a cancelled import, an unconfirmed share sheet, a failed
or cancelled export that says nothing was saved, a stale revision, and
navigation-only Assist. Export stays unavailable until a document is open.
Backgrounding cancels export and interrupts the owner. Reopen uses the
existing project and does not restore an approval. There is no hidden Apply
loop. English and Vietnamese remain presentation catalogs. This is software
parity, not a physical-device session.

## Release and threat audit

The Android release build sets `PIXAURA_CERTIFICATION_TESTING=OFF`. Save uses
MediaStore pending publication or a user-selected document. Share uses
FileProvider with a read grant. Apple save requests add-only photo access or
a user-selected file. The usage description is present. Production sources do
not log image bytes, content grants, or approval material. No A-class blocker
is open. Store publication is not performed. Hosted tests are not physical
device certification.

## Remaining limits

B: hosted timings on a 2 by 3 fixture are not a user-photo budget.
C: Vulkan and Metal remain UNKNOWN. DH-APPLE-METAL-01 remains DEFERRED /
NOT WAIVED / NOT SATISFIED. Phase 10.1 is not written in this commit.
