# Phase 10 Step 4 — Error recovery and lifecycle

Status: CLOSURE CERTIFICATION PENDING. Implementation
`359c2f7ffd7e70b0910019ab8c38acafa29848fb` passed Foundation `42134548234`
SUCCESS 7/7 and Native shells `42134548288` SUCCESS 3/3, attempt 1, exact SHA.
This closure commit does not certify itself.
Steps 5-6 are not started.

## Observed Step 3 closure

Implementation `8e5a64c51acb87f25b76bafb5bc1106dadfca686`: Foundation
`42108036683` SUCCESS 7/7 and Native shells `42108036667` SUCCESS 3/3, attempt
1, exact SHA.

Closure `b2e5809aba185f27c4362b5a56c1b448953ed743`: Foundation `42114973998`
SUCCESS 7/7 and Native shells `42114973771` SUCCESS 3/3, attempt 1, exact SHA.
The receipt is `docs/reports/evidence/phase-10-step-3-closure-certification.json`.

`9397fc84b7727c91c1516f1255254de3569a6e73` is not certified. Native failed
because the authority block preceded each document title, ReviewEditPanel did
not import `dp`, and the Android screen omitted the English save labels.
`b526e21e58e3d17f723bb65a2df9d41af81c45cb` is not certified. Foundation
succeeded, and Native failed Android release lint on three unreferenced
strings. Those SHAs are not rerun.

`789d1170f0811b0c1a7715ab5411b56270e75718` is not certified. Foundation
`42121218372` failed android-boundary armeabi-v7a while unzipping NDK
28.2.13676358 with `Error on ZipFile unknown archive`. That SHA is not rerun.

`e383efc453f64d1a295febab643da877a24d314a` is not certified. Foundation
`42123770790` succeeded. Native ios-app failed because the review field
keystrokes did not change the value before Apply was checked. That SHA is not
rerun.

`9a71d4ca3c10a0e4a700ab81730764768eabcb2a` is not certified. Foundation
`42129181851` succeeded. Native ios-app failed the review-field hittable check
added after the field was already accepted, plus two unrelated timeouts in the
same run. That SHA is not rerun.

## Recovery

Leaving the editor, stopping the Android activity, or leaving the active Apple
scene cancels the current export and sends the existing cancellation action.
A destroyed owner does not accept a later callback. A failed export says that
nothing was saved. Saving or sharing again is a new explicit action. Retry
Apply stays a visible control for a pending approval and does not run by
itself. Reopening a project closes the current owner and reads ordinary
history. It does not restore an approval, a review, or a preview. Native
publication and cancellation remain the winners. Schema 3, HUMAN Apply, and
export revision binding are unchanged.

Phase 11 and Phase 12 are not started.
