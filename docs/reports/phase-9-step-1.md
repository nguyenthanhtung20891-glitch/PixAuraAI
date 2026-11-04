# Phase 9 Step 1 — Export contract and authority

Status: AUTHORIZED / IN PROGRESS. This commit freezes the contract only.
No encoder, platform destination, or revision behavior is added. Steps 2-6 are
not started. No unexecuted gate is PASS.

## Baseline

HEAD matched `45d121a2e0dd2fce8e925d3e36b3289a696c136a` and `origin/main`.
Phase 8 closure `029f84eaa86f3e5a8bffc696a6abd10f9477ae2a` remains the certified
closure: Foundation 41709039261 7/7 and Native shells 41709039298 3/3.
The gap audit's blocking item is export of a committed revision.

Local copies of several Phase 8 evidence files were stat-dirty and byte-identical
to HEAD. They were not changed.

## Decision

[ADR 0023](../adr/0023-export-is-a-projection.md) accepts export as a projection
through the already vendored libjpeg-turbo 3.2.0 and libpng 1.6.59. No new
dependency is admitted. Vendor sources stay unchanged. [Export v1](../contracts/export-file-lifecycle-v1.md)
binds the request to one ordinary revision, strips location metadata, and does
not claim ICC preservation.

## Rejected implementation SHA

`0bcd873ce26697c29fbe46ad0239b4cda6a9d66a` is not certified.
Foundation `41857947698` failed. Portable Ubuntu and Windows stopped in the Node
suite: the new ADR omitted the required Alternatives and Validation sections, and
the Phase 7 inventory did not yet allow the new contract and ADR. Apple boundary
job `126671553099` was cancelled without a runner. Sanitizer, workflow lint, and
both Android boundary jobs succeeded. Native shells run `41857947568` also failed
and is not certification. The correction is a new SHA.

## Unacquired Apple runners

`61ec322ffc4ef102af045a3acf17d660ec9300ef` is not certified. Every non-Apple
job succeeded on attempt 1. Foundation `41862714321`: workflow-lint, sanitizer,
Ubuntu, Windows, and both Android ABIs succeeded. `apple-boundary` job
`126688656496` was cancelled with no steps because the hosted runner was not
acquired. Native shells `41862714682`: source and android-app succeeded.
`ios-app` job `126688657635` was cancelled the same way. This is runner
capacity, not a contract failure. The failed SHA is not rerun.

## Infrastructure on the following SHA

`68edee8332fd17672e6bc9f87d28dd83ffb436a3` is not certified. Foundation
`41869181560` succeeded for workflow-lint, sanitizer, Ubuntu, Windows, and
armeabi-v7a. `android-boundary` arm64-v8a job `126711238879` failed while
sdkmanager unpacked NDK 28.2.13676358: "Error on ZipFile unknown archive."
That download is not a contract result. `apple-boundary` job `126711239374`
was cancelled with no steps. Native shells `41869181518` succeeded for source
and android-app. `ios-app` job `126711239100` was cancelled with no steps.
Neither failure is rerun on this SHA.

## Apple runners still unacquired

`c46695ef588cb768a240039a34edf65eb6d0c531` is not certified. Foundation
`41872559838` succeeded for workflow-lint, sanitizer, Ubuntu, Windows, and both
Android ABIs. `apple-boundary` job `126723295130` was cancelled with no steps.
Native shells `41872559742` succeeded for source and android-app. `ios-app`
job `126723293831` was cancelled with no steps. The NDK unpack error did not
recur. The failed SHA is not rerun.

## Observed implementation certification

`64c01c1fba688ff266951ce2eb608982331323f8` passed both workflows on attempt 1.
Foundation `41876428603` succeeded 7/7, including `apple-boundary` job
`126736960376`. Native shells `41876428763` succeeded 3/3, including `ios-app`
job `126736959363`. Earlier SHAs are not substitutes. This documentation-only
closure still needs both workflows on its own SHA. Step 1 is not
COMPLETE / FULL PASS until that receipt is observed. Steps 2-6 are not started.
