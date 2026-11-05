# Phase 9 Step 4 — Apple Save and Share

Status: CLOSURE CERTIFICATION PENDING. Implementation
`6a6b8d30ba69316822dcffafbee6894e4d1f0271` is observed: Foundation
`41967410493` SUCCESS 7/7 and Native shells `41967410497` SUCCESS 3/3, attempt
1, exact SHA, every required job SUCCESS. This documentation-only closure does
not certify itself. Step 4 is not COMPLETE / FULL PASS until that separate SHA
is observed. Steps 5-6 are not started.

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
`96dd3b0a1c26dea64537c2a05b760016c8025b07` is not certified. Foundation
`41957799748` failed because the photo-save gate used a forbidden concurrency
marker. That SHA is not rerun.
`eab05531501d57873b849f830a8ac3d601bf8501` is not certified. Foundation
`41959479634` succeeded, but Native `41959479678` rejected an unnecessary
isolation annotation as an error. That SHA is not rerun.
`6a95fd806c3f39dea989cbd9f1c6d396627c6611` is not certified. Foundation
`41961216195` succeeded, but Native `41961216123` could not reach a review
control after the inline export form reduced the review viewport. That SHA is
not rerun. Export now opens from the toolbar and leaves the review layout intact.
`90fe6fb31893018a0dc65e0cfbe87d3f1b872192` is not certified. Native
`41965766306` stopped in the iOS type checker on the combined editor expression.
That SHA is not rerun. The manual controls and export sheet are now separate views.

## Destination

Save to Photos uses add-only Photos authorization and a transactional
`PHPhotoLibrary` change. Save to a file uses the system document exporter.
Share writes one cache file and presents the system share sheet. Closing that
sheet is not confirmed delivery. Export reads one committed revision and checks
that the revision count and revision id are unchanged.

No photo-library read permission is requested. Hosted simulator evidence is
software evidence only. Steps 5-6 are not started.
