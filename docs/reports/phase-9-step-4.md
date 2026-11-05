# Phase 9 Step 4 — Apple Save and Share

Status: AUTHORIZED / IN PROGRESS. This commit adds the Apple destination
adapter over the certified native export. It does not certify itself.

## Observed Step 3 closure

Implementation `093f63b7807935f92a3758552dfd60fc79f38d97`: Foundation
`41945751521` SUCCESS 7/7 and Native shells `41945751568` SUCCESS 3/3, attempt
1, exact SHA.

Closure `df49c9a77bddf2f0a8b45d5bb0f12120ebe4bc3b`: Foundation `41950759628`
SUCCESS 7/7 and Native shells `41950759555` SUCCESS 3/3, attempt 1, exact SHA.
The receipt is `docs/reports/evidence/phase-9-step-3-closure-certification.json`.

`6f62fb1973bd98d9c241c3c687d2f91125c48198` remains uncertified and is not rerun.
`9444584ab93e39d9443f47e1b39f7749fb2f56c7` is not certified. Its iOS compile rejected
main-actor use inside the Photos completion, and the shell check rejected a
hand-edited project file. That SHA is not rerun.

## Destination

Save to Photos uses add-only Photos authorization and a transactional
`PHPhotoLibrary` change. Save to a file uses the system document exporter.
Share writes one cache file and presents the system share sheet. Closing that
sheet is not confirmed delivery. Export reads one committed revision and checks
that the revision count and revision id are unchanged.

No photo-library read permission is requested. Hosted simulator evidence is
software evidence only. Steps 5-6 are not started.
