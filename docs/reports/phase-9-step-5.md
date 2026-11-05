# Phase 9 Step 5 — File Lifecycle Hardening

Status: CLOSURE CERTIFICATION PENDING. Implementation
`6a30b1512c995df511e420945d845a92b85364dc` is observed: Foundation
`41976330668` SUCCESS 7/7 and Native shells `41976330587` SUCCESS 3/3, attempt
1, exact SHA, every required job SUCCESS. This documentation-only closure does
not certify itself. Step 5 is not COMPLETE / FULL PASS until that separate SHA
is observed. Step 6 is not started.

## Observed Step 4 closure

Implementation `6a6b8d30ba69316822dcffafbee6894e4d1f0271`: Foundation
`41967410493` SUCCESS 7/7 and Native shells `41967410497` SUCCESS 3/3, attempt
1, exact SHA.

Closure `b04fd291cda9e88e6a03ad20140f634294b35168`: Foundation `41971443062`
SUCCESS 7/7 and Native shells `41971442960` SUCCESS 3/3, attempt 1, exact SHA.
The receipt is `docs/reports/evidence/phase-9-step-4-closure-certification.json`.

## Audit

The certified export path remains authoritative committed revision, existing
evaluation, bounded encode, and caller-owned bytes. Import, Schema 3, and
ordinary history are unchanged. Export adds no revision, approval, candidate,
or provider call.

Repeated PNG export of one revision is byte-identical and leaves the revision
count at one. A second open document exports independently. An output buffer
below 64 bytes is denied and does not change either document. Cancellation,
stale epoch, and stop were already covered by the export engine tests.

Android and Apple destination atomicity stays the certified save and share
behavior. A failed encode does not finalize a destination because the platform
receives bytes only after native success.

## Resources and hosted timings

Export keeps the existing admission limits and the 64-byte through 64 MiB
output bound. This step adds no process-RSS ceiling. `export_test` prints
hosted repeat and JPEG durations and encoded sizes. Those numbers are software
evidence from the runner that executed the test. They are not physical-device
battery or thermal measurements. The closure records the observed lines. This
commit does not invent them.

Step 6 is not started. No Phase 10.
