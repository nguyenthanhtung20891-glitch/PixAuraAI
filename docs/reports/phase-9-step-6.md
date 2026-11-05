# Phase 9 Step 6 — End-to-end certification

Status: AUTHORIZED / IN PROGRESS. This commit does not certify itself and does
not close Phase 9.

## Observed Step 5 closure

Implementation `6a30b1512c995df511e420945d845a92b85364dc`: Foundation
`41976330668` SUCCESS 7/7 and Native shells `41976330587` SUCCESS 3/3, attempt
1, exact SHA.

Closure `7e334b7962b1974bc4f674b9cc6818831dd408de`: Foundation `41981405571`
SUCCESS 7/7 and Native shells `41981405588` SUCCESS 3/3, attempt 1, exact SHA.
The receipt is `docs/reports/evidence/phase-9-step-5-closure-certification.json`.

`ec3614ce69033b720d94f207ef1ffb5174712062` is not certified. Foundation
`41985713323` and Native shells `41985713262` failed because the Step 1
governance test still required roadmap row 6 to read NOT STARTED after Step 6
had started. That SHA is not rerun.

## Journey binding

The certified native export test already runs the journey pieces on every
Foundation and Native workflow:

- Manual crop commits one ordinary revision, then export reads that revision.
- An approved Auto plan commits one ordinary revision, then export reads it.
- Undo and redo select a committed revision without export creating another.
- A denied or cancelled export leaves the document bytes unchanged.
- A second open document cannot be published through the first export.
- JPEG output has no APP1/EXIF payload, and PNG output has no eXIf, iCCP, or text chunk.

Assist remains navigation to the manual tool. Review edit still mints a new
Plan identity before the existing HUMAN Apply. Export is not on either path.
Android MediaStore/SAF/sharesheet and Apple Photos/Files/share sheet stay the
certified destinations. Presenting a share sheet is not delivery.

## Threats

The export contract's threat rows A through N stay closed by those tests, the
destination atomicity already certified on Android and Apple, and the existing
release scanners. No new test hook, publisher, or model asset is added here.

Phase 10 is not started.
