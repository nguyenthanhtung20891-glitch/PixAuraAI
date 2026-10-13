# Phase 4 Step 5 — Apple capability integration

Status: COMPLETE / FULL PASS. Certified implementation SHA:
`944517952252cf1c9e6970bb3dfb2c62fe7f908a`.
Baseline HEAD and origin/main initially
`40a0c9bef05df328a2b3e89497316c368b81aa24`; working tree and diff check clean.
Steps 1–4 COMPLETE / FULL PASS; frozen seven-step roadmap unchanged.
Steps 6–7 NOT STARTED. [Contract](../contracts/apple-capability-integration-v1.md).

Implementation adds one caller-owned Apple service, trusted static/runtime
observations, bounded observer lifetime and observation coalescing on existing
command lanes. Policy-aware editor open precedes decode; apply/query reuse
existing lifecycle/display gate. Shared policy, PRV1, schema, history, parameters,
reference semantics and hard ceilings remain unchanged. No new architecture
decision or conflict. No Android-only/shared semantics are introduced.

Tests add legitimate probe/mapping checks, synthetic LOW/MID/HIGH pressure and
recovery, large-source FIT resizing, delayed candidate rejection, gesture commit/
cancel, process-style reopen, malformed records and trust rejection, observer
cleanup/reconstruction, 10000 callback churn and concurrent close, 1000 stable
updates per profile and 48 commit/cancel-pressure races. Retained native tests
remain actual late-kernel checkpoint authority and EXACT-vs-FIT proof.

No physical Apple hardware available. Simulator and hosted macOS evidence are
nonphysical target evidence only. Metal availability never certification;
DH-APPLE-METAL-01 remains DEFERRED. Schema 3 unchanged. No Phase 5/AI/model work.

## Observed validation

Local source/docs 89/89 (zero skips), workflow actionlint and git diff check PASS.
Local retained Windows native 30/30 PASS (`build/step5-windows.log`). First hosted
implementation `8bf44312b0c97b4be88fdd0d582624b4c9ff9d84` failed Apple test
compilation (B): Swift concurrency diagnostics in the new race harness. Remediation
moves blocking joins and synchronized immutable captures into a synchronous test
fixture; production policy and strict warnings/assertions remain unchanged.

Corrected implementation `7a0447a4c7815640815fa6e525b6e04262a6c941` observed
Apple boundary SUCCESS: native 30/30, Swift macOS 24/24 and iOS Simulator 24/24,
including all five new Apple cases; Linux and ASan/UBSan SUCCESS. Android app
job SUCCESS. At that intermediate observation Windows/iOS application closure
was pending; final implementation certification below supersedes that status.
Final test review adds the explicit NORMAL→ELEVATED→SEVERE→NORMAL thermal trace
and UInt64.max budget rejection to the same fixture corpus before acceptance.
Final adapter review found A: coalescing older unavailable backend with newer
unknown must retain UNKNOWN, rather than accidentally create known recovery
evidence. Availability merge now preserves unknown in either pending reading;
tests cover both backend and memory unknown coalescing. Shared policy unchanged.
Earlier corrected iOS app job exposed B: the new test expected initial reopen
under CRITICAL to succeed despite the frozen 32 MiB codec scratch reservation,
metadata and control exceeding the critical 32 MiB cap. Already-open preview,
commit and cancel passed. Harness now explicitly REQUIRES Open resource failure
under critical pressure, then supplies normal test observations to a fresh
owner and proves admitted reopen retains both revisions with fresh epoch one.
No ceiling, admission, semantic assertion or production behavior is weakened.

## Final implementation certification

Both COMPLETE workflows observed SUCCESS on the SAME certified implementation
SHA `944517952252cf1c9e6970bb3dfb2c62fe7f908a`:

| Workflow | Exact run ID | Jobs |
| --- | --- | --- |
| Foundation boundaries | 38355260289 | 7/7 SUCCESS |
| Native application shells | 38355259361 | 3/3 SUCCESS |

Every job SUCCESS: source, workflow-lint, Windows portable, Linux portable,
sanitizer, Apple boundary, Android arm64-v8a boundary, Android armeabi-v7a
boundary, Android application and iOS application. No required job/test skipped
or unobserved. Any later documentation closure must also pass BOTH workflows
on its OWN SHA; final delivery records those workflow IDs and closure SHA.

| Executed gate | Observed result |
| --- | --- |
| Hosted source/documentation | 125/125, zero skips |
| Portable source per native host | 83/83, zero skips |
| Windows native | 30/30 |
| Linux native | 31/31 |
| ASan/UBSan and active negative controls | 33/33 |
| Apple native | 30/30 |
| Swift macOS | 24/24 |
| Swift iOS Simulator | 24/24 |
| iOS unsigned debug/release builds | PASS |
| iOS application | 7/7 |
| iOS UI | 2/2 |
| Android arm64-v8a and armeabi-v7a boundaries | PASS |
| Android debug/release builds and lint | PASS |
| Android JVM | 7/7 per variant, zero ignored/failures |
| Android emulator instrumentation | 26/26, zero errors/failures/skips |
| Workflow actionlint, shell syntax and diff check | PASS |

Retained Phase 0–3 and Phase 4 Step 1–4 gates execute in these complete runs.
Evidence: `build/step5-certified-{foundation,shells}-view.txt`,
`build/step5-certified-{source,windows,linux,sanitizer,apple,android,ios}-evidence.log`
and Android HTML/XML artifacts in
`build/gh-artifacts-38355259361/android-shell-evidence`. Build artifacts ignored,
not committed. Earlier-SHA success is historical evidence only.

## Accepted behavior and limits

Static probe fields: bounded active CPU count, compiled architecture/platform,
OS major, physical RAM and Metal availability. iOS remaining allocatable bytes
are fresh advisory observations; macOS process budget stays unknown. No model
heuristic or identifier. Static facts are immutable; actual initial pressure
stays separate. Shared core exclusively owns admission, tier/cap/edge, recovery,
epoch and fallback. Versioned checked Swift/C records retain existing lifetime
and display gate; no Swift object retained by native core.

Thermal nominal/fair/serious/critical → NORMAL/ELEVATED/SEVERE/CRITICAL;
unsupported → UNKNOWN. Dispatch pressure normal/warning/critical → NORMAL/
SEVERE/CRITICAL; UIKit warnings → SEVERE, never inventing a clear signal.
Startup/ambiguous pressure UNKNOWN. Active unconstrained; inactive/background
constrained with existing interruption and no automatic commit.

LOW/MID/HIGH synthetic normal outcomes T0/96 MiB, T1/192 MiB, T2/256 MiB;
critical collapses to T0 ≤32 MiB/360. Recovery requires three stable shared
observations and never exceeds baseline. 1000 stable updates per profile leave
epoch unchanged. Malformed enum/version, trust escalation and UInt64.max budget
reject. 1024×512 FIT displays 512×256 normally and 360×180 critically. EXACT
remains exact-or-resource-failure; source/reference and operation meaning unchanged.

Active values 1234 survive pressure; downgrade adds no history. Old candidates
cannot install after policy change/new display; prior valid display survives
rejection. Changed commit adds exactly one revision, pressure/cancel zero.
Fresh critical reopen REJECTS Open status 8; scratch plus metadata/control cannot
fit the frozen cap. Normal observations/fresh owner permit admitted reopen with
both revisions retained and epoch one. Runtime state never persisted; Schema 3
and raster/context/operation/scratch/storage/handle ceilings unchanged.

At most eight subscribers share one registration. Final unsubscribe/destruction
invalidates callback identity before unregister/cancel. 10000 callback churn,
20000 subscriber deliveries, stale callback rejection, weak owner release,
reconstruction and concurrent callback/close pass. 48 commit/cancel-pressure
races pass. Retained native checkpoint, bounded churn, payload/resource accounting
and stop/join/destroy gates pass. No new transient cache/backend resource layer.
One pending observation/drain preserves strongest pressure, constrained OR,
minimum budget and unknown/unavailable backend; no trust escalation or second
publication generation. A and B remediations are described above; C: no
infrastructure retry required. No architecture conflict or new ADR.

Documentation closure exposed B: the historical source guard required Step 5
NOT STARTED. Guard now requires Step 5 COMPLETE / FULL PASS and explicitly
requires Steps 6/7 NOT STARTED, retaining the seven frozen names/step count.
Closure changes documentation/status and this status guard only; production
and runtime test implementation unchanged.

Simulator proves wiring/policy/lifecycle/app flow only. Hosted macOS proves
compiler/runtime/native/Swift execution, not physical iPhone/iPad certification.
No physical Apple hardware/evidence available. AVAILABLE != VALIDATED/CERTIFIED;
CPU T0 fallback where admitted. DH-APPLE-METAL-01 remains DEFERRED and unchanged.
No Phase 5/AI/model work. Phase 4 Step 6 has NOT started. STOP after Step 5.
