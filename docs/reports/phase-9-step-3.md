# Phase 9 Step 3 — Android Save and Share

Status: AUTHORIZED / IN PROGRESS. This commit adds the Android destination
adapter over the certified native export. It does not certify itself.

## Observed Step 2 closure

Implementation `64d854d7ec2a5e11787cd0b94ff60b0fdfe7a8e3`: Foundation
`41928862538` SUCCESS 7/7 and Native shells `41928862545` SUCCESS 3/3, attempt
1, exact SHA.

Closure `667bcc3f04c639e41c18f15b4d44f6aee7852107`: Foundation `41935253282`
SUCCESS 7/7 and Native shells `41935253224` SUCCESS 3/3, attempt 1, exact SHA.
The receipt is `docs/reports/evidence/phase-9-step-2-closure-certification.json`.

Earlier Step 2 SHAs `14330b8057105095a8cf8a9749ecd40af87c0250` and
`3799fe2b1bff41d8fc4c7cfad034e96306b6c475` remain uncertified and are not rerun.

## Destination

Save to Photos inserts a pending MediaStore image and publishes it only after
the write completes. A failed write deletes the pending row. Save to a file
uses the system document picker. Share writes a cache file through a
non-exported FileProvider and presents the system sharesheet. Closing that
sheet is not confirmed delivery. Export reads one committed revision and checks
that the revision count and revision id are unchanged.

No broad storage permission is requested. API 26 through 28 use the document
picker. Steps 4-6 are not started.
