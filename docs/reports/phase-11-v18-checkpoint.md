# Phase11 v18 resumable checkpoint

Starting/current implementation HEAD4cf0642119ea39149615b93588068c56a85d48ff;
main/origin baseline verified. v18 changes UNCOMMITTED, not hosted-certified.
Generator mistakenly ignored --check and rewrote primary project; recovered exact
original from retained private patch plus existing runpath changes, verified against
saved original SHA256. A separate ignored original-owner-project backup now exists.
Owner project.pbxproj remains excluded and byte-identical to its original local
signing diff; baseline hash build/phase-11-v18/owner-project-before.sha256.
Plan docs/plans/phase-11-v18-integration.md. No agents used.

## Completed engineering / observed evidence

Additive private reader/stage/preview/revision/stream pipeline; actual4000x3000
PNG→native managed original→ordinary Schema3 manual revision→Undo/Redo/reopen→
explicit64x48FIT preview→full4000x3000JPEG/PNG staged exports. Existing pinned
codecs/color/orientation checks and frozen CPU kernels. No contiguous12MPfloat
original, budget ceiling unchanged; representative test reservation99931581bytes.
All12000000decodedPNGpixels bitwise against independent v1 neighborhood oracle.
Host full74/74PASS166.26s; focused full12MP export+oracle2/2PASS26.55s.
Raw logs build/phase-11-v18/{full-host-test,full-oracle-test}.log.
Golden recipe/digests tests/fixtures/integration-v18/golden.json.

Private native commerce owner binds real managed source/revision, writer fence,
quota reservations, usable native advice and bounded untrusted recoverable inbox.
Tests cover5distinctphotos/3AI, concurrency, cancellation, stale source/revision,
dismissed replay, midnight provider resampling, and four real process-crash orders.
No persisted approval/candidate, no quota token grants execution. Trial committed
edit receipt NOT implemented (CR18-01). Paid test facts synthetic/private only.
OS clock samplers and labelled optional verified-store/cache handoffs added on
both platforms. Shipping Android Billing remains absent; no catalog or checkout.
Android callback-after-close regression reproduced FAIL before correction and PASS
after, same assertion. Focused7/7PASS; full Emulator71/71PASS after both close-order fixes, no physical claim.
Logs sdk-close-before.log, sdk-clock-after.log, instrumentation.log retained.

## In flight / next executable work

Sanitizer76/76PASS352.03s; isolated iOS genericDebug/Release and units21/21PASS.
Stronger rawRGBA8all12Mpixeloracle focused3/3PASS27.10s; ASan/UBSan3/3PASS89.77s.
LinuxGNUcontroller36/36PASS after container-onlyprocps prerequisite fix.
iPhoneDuoUI19/19 and explicitlargest-text1/1PASS; complete iOS shell script exited0. Added reentrant-close normal-return test
reproduced1failure, fixed post-persist close check; full71AndroidPASS.
Source checkout build/phase-11-v18/apple-source excludes owner's signing edits.
LinuxGCC5/5PASS24.77s, same export hashes; Androidrelease/JVM/lint/scansPASS.
Nativeownerleaks0bytes20.4M(tool-reportedunits)peak; real12MPleaks0bytes119.8M(tool-reportedunits)hostpeak;
these are actual host observations, not mobile acceptance or new RSS ceilings.
Source-v18 manifest binds43files plus113unchanged native fingerprints; source220/220PASS
in isolated inventory-aware checkout. Next:
review diff/secrets, commit/push excluding owner project, then exactSHA7+3 CI.
No v18 CI receipt yet. Do not reuse v17 CI as v18 certification.

## Specific acceptance boundaries

CR18-01 atomic committed-edit receipt/time requires explicit versioned storage
transaction/schema approval; existing Schema3 has no commit timestamp/receipt hook.
CR18-02 versioned app editor/commercial bridge required; frozen v1 normalized
original handles cap8MP/128MiB. No Swift/Kotlin12MP editor activation or production
quota enforcement. Fence proof uses ONE SQLite/native publisher instance; legacy
core linked only for pure capability/budget query. Legacy storage transient reads
need bridge-owned accounting, not retrospectively claimed instrumented here.
See docs/contracts/phase-11-v18-compatibility-requests.md.
External G7physical/privacy, G8LOW/MID/HIGHphysical/thermal, G10real store accounts/
catalog/backend/signed sandbox, G11human VoiceOver/TalkBack remain OPEN.
DH-APPLE-METAL-01 PASS/CLOSED unchanged/not rerun; CPUVALIDATED; GPUprovidersUNKNOWN.
Phase11 INPROGRESS. No Phase12–14 work; independent review required.
