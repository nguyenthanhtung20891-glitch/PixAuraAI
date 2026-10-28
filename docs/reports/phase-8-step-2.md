# Phase 8 Step 2 - Before/After Preview Foundation

Status: AUTHORIZED / IN PROGRESS; implementation certification pending.
Step 3 and Steps 4-6 NOT STARTED. This report does not certify its own SHA.

## Baseline and scope

Starting SHA `7aa21dc449de5143ddf1af7998ea7e233bb691b7`, main == origin/main,
clean tracked/staged tree after fetch. Step 1 accepted COMPLETE / FULL PASS:
Foundation 40553987747 (7/7), Native shells 40553988252 (3/3), attempt 1, exact
closure SHA, every job completed/SUCCESS. The job-level receipt is preserved in
[Step 1 evidence](evidence/phase-8-step-1-closure-certification.json). Phase 7
remains CLOSED / COMPLETE / FULL PASS; historical evidence is unchanged.

Production changes implement optional native non-publication previews and thin
Android/Apple display/lifecycle adapters only. No editable review, new action
inspection architecture, model research, editing authority or persistent state.
The [implementation contract](../contracts/review-preview-foundation-v1.md)
binds the narrow API and resources. The [exact changed-file inventory](evidence/phase-8-step-2-files.json)
and [before/after runtime hashes](phase-8-step-2-v1.json) retain exact scope.

## Architecture and exact binding

The editor Service owns one small ReviewPreviewState: immutable canonical Plan
bytes, exact Auto proposal, source/owner/serial/repository/policy epochs, one
pending/active request, one optional native image pair and a private cancellation
token. It owns no approval issuer, candidate, document transaction or publisher.
Begin revalidates the exact cached Plan through existing Plan/Registry authority.
Render snapshots source/Plan under the service gate, then runs raster work off the
platform UI and outside lifecycle gates. The existing evaluation/geometry/preview
primitives replay BEFORE's bound current revision and append ordered Plan actions
for AFTER PREVIEW. There is no second raster engine or new tool math.

The preview-specific ticket (RVP1, 56 bytes) and pair (176 bytes) are correlation
and image eligibility records, distinct from document/candidate/approval handles.
Copies are bounded, side/offset/count checked. Native images are released after
platform copy; final installation rechecks current native eligibility under the
platform lifecycle gate. A raster completion or copy alone cannot install a result.
The exact native record includes source/Plan/request association and geometry.

Stale source, Manual edit, undo/redo ABA, source/owner replacement, mode abandonment,
successful Apply, stop, Plan replacement, repository epoch or dynamic-policy
changes invalidate optional preview. No silent rebase, refresh or retry. Platform
attempt/epoch guards reject callbacks and cancellation from older requests. Native
BUSY prevents replacement admission while active work drains. Native and detached
display resources are dropped on abandonment. Stop -> join ALL calls -> release ->
destroy includes render/copy/current/release, not just the initial raster call.

Preview cancellation uses its own raster token; it never signals approved execution
cancellation. It can leave the current Plan inspectable and explicitly retryable
after drain. Apply is unavailable during preview loading/stale presentation, and
still requires the existing explicit HUMAN event. Preview success never approves,
creates an execution candidate, executes, publishes or changes history. Once Apply
is explicitly requested, the unchanged Phase 7 publication/cancellation winner wins.

## Resources and geometry

Existing Budget Admission v1 PREVIEW and dynamic policy apply. The existing raster
registry mutex covers occupancy snapshot, preflight and both renders, preventing
an unleased preflight/allocate race. Counts include existing retained resources,
one full working raster at a time, existing evaluation/geometry scratch, two native
image handles, own cancellation, fixed existing control overhead and eight bounded
RGBA display capacities (native pair, new platform pair, superseded presentation
pair, current Manual image, transfer). The existing automatic PREVIEW display charge
is subtracted from explicit temporary bytes, avoiding double counting.

Platforms request existing fit bounds 512; native caps positive dimensions at 1024
and the current policy edge. No upscale/full-resolution review display. No existing
resource class or ceiling changes; no RSS-derived ceiling or intelligence-runtime
owner is used. Resource denial preserves document and Plan inspection; the native
test proves Manual commit still works when optional pair admission fails.
Bitmap/CGImage release follows platform ownership, not claimed immediate process
RSS reclamation. No accumulating native/platform preview cache or disk persistence.

Tone/detail comparisons support fit/toggle, split, shared bounded zoom/pan and
reset. Crop/rotation conservatively disables spatial split/zoom/pan and uses honest
aspect-preserving fit/toggle with native resulting dimensions. No stretching or
unproved spatial registration. Preview/full-resolution final pixels are not claimed
universally equal; test comparisons use the same deterministic fit resolution.

## Android and Apple presentation

Both expose separate Preview and Apply, Before (bound source), After Preview (not
committed), loading/cancel, stale/error, toggle/split/reset and semantic identifiers.
Controls alter viewport only. Native owns Plan/Registry/source/raster/freshness.
JNI transports fixed packets and row copies, packing RGBA to ARGB without editing
math. Swift wraps the same C family with CGImage/RGBA/sRGB. Existing command/render
ownership lanes and stop/join counters include the whole new request lifetime.
No handles or comparison state enter SavedState, UserDefaults, files or Schema 3.

Android instrumentation covers explicit preview versus Apply, zero preview history,
toggle/split/reset, cancellation rendezvous, resource failure, mode departure,
stale Manual changes, geometry, recreation and cross-owner installation rejection.
Swift tests cover detached exact repeats, no authority/history effects, stale,
wrong owner, cancellation/mode/Manual/stop drain and geometry. Semantic iOS UI tests
cover preview, explicit Apply, fit geometry, cancellation/failure and navigation.
Actual platform execution is reported only after observed workflows below.

## Numerical, failure and performance evidence

Windows Zig 0.14.1 Debug local native preview suite: 965 assertions before final
platform certification; all eleven tools, 2/8 actions, nine rejected, malformed/
unsupported/invalid parameters, exact cached bytes, stale/forged/cross-owner records,
BUSY/no queue, cancellation/manual/undo/redo/mode/stop/external-epoch rendezvous,
resource denial, failure atomicity and zero preview approval/candidate/history.
BEFORE includes an existing Manual edit. AFTER equals ordinary approved execution
followed by the same fit render, including crop + quarter rotation + tone ordering.
Project-owned deterministic 640x480 gradient/checker numerical PNG has recorded
generation and SHA-256; it is not model/photo quality evidence.

An independently compiled shipping-source allocation sweep (no certification layout)
passes 316 Begin and 420 Render fail-at-every-new injections, unchanged output PODs
and public document state, then 70 successful render/release cycles. Maximum
cumulative requested bytes on the tiny fixture: Begin 46,476; Render 68,988.
These cumulative allocation diagnostics are not a live peak, process RSS or lease.

Local representative six-request runs (one first + five warm), Windows Debug,
640x480 source, 512 fit, no physical-device battery/thermal claim:

| Plan | Begin mean ms | First render ms | Warm median ms | Warm p95 ms | PREVIEW admission peak bytes | Retained native pair bytes | After native release |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Exposure | 6.519 | 264.428 | 263.891 | 266.789 | 19,386,024 | 1,572,864 | 0 |
| Exposure + contrast | 6.715 | 306.073 | 305.544 | 308.216 | 19,386,024 | 1,572,864 | 0 |
| Crop + rotate + brightness | 6.977 | 248.615 | 259.525 | 261.474 | 19,410,600 | 1,572,864 | 0 |

Admission peak is the applicable conservative accounting bound including existing
retained resources and simultaneous display allowances, not measured whole-process
heap/RSS. Retained native image bytes are exact metadata and return to zero after
release; detached platform images remain explicitly platform-owned until discarded.
Repeated runs compare exact output bytes, not timing equality.

## Security, release and retained contracts

| Threat | Protection / evidence | Result pending full certification |
| --- | --- | --- |
| Preview becomes approval/publication | image-only owner/API; existing authority code unchanged; zero authority/history assertions | Native verified |
| Forged/stale/cross-owner pair | exact native ticket/pair, Plan/source/serial/epoch checks before raster and installation | Native verified |
| Late callback / old cancel / teardown | one active request, attempt/epoch guards, separate fixed token, stop/join | Native verified; platform execution pending |
| Resource exhaustion harms Manual | existing PREVIEW preflight, all simultaneous ownership, real allocator sweep, Manual fallback | Native verified |
| Pixel/geometry semantic drift | existing evaluation primitives; exact eleven-tool and ordered-Plan reference output | Native verified |
| Test-hook/research leakage | debug/certification flags; expanded native/APK/Apple markers and negative fixtures | Final package execution pending |
| Privacy/persistence | local-only memory; no endpoint/upload/cache/Schema 3 additions | Source verified |

A findings: none identified in inspected implementation; closure remains conditional
on full exact-SHA certification. B: spatially registered geometry split is deliberately
unavailable, with safe fit/toggle as authorized. C: timings are hosted engineering
diagnostics; physical battery/thermal and allocator-level platform measurements are
not claimed. No existing tests weakened, skipped or optionalized.

All frozen contracts are byte-identical, including Full Review Experience v1.
Existing decode APIs are verified byte-identical after removing only the additive
composition/include. Native evaluation/geometry/preview math, approval/candidate,
document/storage/history and Schema 3 are unchanged. The historical Phase 7 inventory
is preserved; governance allows only the exact per-file Step 2 before/after hashes
and explicit additions, without a blanket runtime exemption. Source consistency's
Swift shared-admission count changes from three to four because preview adds the
same bounded admission guard; all three original guards remain.

reference.assistance.v1 remains ADMITTED; planner reference.deterministic.v1.
Learned model NONE REQUIRED / NONE ADMITTED FOR PHASE 7 MVP. No learned/runtime/
weights/NLP/cloud/network work or research reopened. CPU VALIDATED; Vulkan/Metal
UNKNOWN. DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED. Hosted Apple is
software evidence only. Step 3 and editable review NOT STARTED.

## Certification and closure

Local retained Windows Zig native suite: 56/56 PASS, 189.77 seconds. Android
debug JNI builds for arm64-v8a/armeabi-v7a/x86_64 and final instrumentation Kotlin
compilation pass. Release APK build plus debug/release lint pass; final APK scan
passes 12 native libraries, 2 DEX and 31,969,462 uncompressed bytes. Android/Apple
scanner corruption fixtures and actionlint pass. No physical device is claimed.
Local source hygiene caught and corrected trailing whitespace and the new PNG's
missing exact binary allowlist entry; its hash/size/dimensions and substituted/
unlisted negative cases are now enforced. No blanket binary exemption was added.
The retained source suite is also required on the committed exact SHA below.

Initial implementation `b01b943cc49984476c5cc563be1b36396199fadf` failed Foundation
40672064945 (attempt 1), so is NOT certified. GCC rejected four compact adjacent
`if` statements under misleading-indentation/Werror; braces/newlines correct them
without warning suppression. MSVC found the allocation clone missing shipping
SODIUM_STATIC and the C consumer missing C11: canonical production definitions
(excluding testing macros) are now inherited and C11 explicitly requested. ASan
found a test-only 64-byte memcpy from a 56-byte ticket in the wrong-handle fixture;
the negative test now copies the minimum of the two bounded POD sizes. No shipping
overread or authority bypass was implicated; no sanitizer suppression added.
Corrected implementation certification is pending. Local checks do not substitute
The next SHA `ab47d7a1eb069be7ec8c5e16e82a4f538b35f3da` passed Linux, Apple,
active ASan/UBSan, both Android native ABIs, source and workflow lint. It did not
certify: Windows reached its unchanged workflow timeout in the new allocation
sweep, and Android completed 51 tests with two new harness failures. The mode-chip
helper scrolled only the nested horizontal row, leaving the containing vertical
viewport offscreen; it now uses the retained two-level semantic scroll helper and
asserts selection. Neither failed assertion was removed. The Windows sweep gains
the existing fail-fast CRT/terminate reporting pattern, bounded per-case diagnostics
and a stricter 120-second test timeout; persistent fail-at-every-new assertions are
unchanged. Further diagnostic results must establish its actual allocation failure.
An independent JNI review also corrected the pre-copy freshness check to transport
the exact native stale/cancelled status instead of a null/generic allocation error;
a stale-record transport regression verifies it. No stale pixels were installable.
Local checks do not substitute
for full exact-SHA Foundation 7/7 and Native shells 3/3, every required job observed.
No Apple/Simulator test is labeled PASS before execution. Active ASan/UBSan and
final release packages must be observed in those retained workflows.

After implementation certification a separate documentation-only closure must pass
both full workflows on its own exact SHA. Only the external observed final receipt
activates Step 2 COMPLETE / FULL PASS. No older SHA, self-certification or
rerun-until-green. Stop after Step 2.
