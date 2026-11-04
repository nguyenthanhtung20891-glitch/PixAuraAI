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

## Certification still required

This commit must pass Foundation boundaries and Native application shells on its
own SHA. A later documentation-only closure must pass both workflows again.
Until those observed results exist, Step 1 is not COMPLETE / FULL PASS.
