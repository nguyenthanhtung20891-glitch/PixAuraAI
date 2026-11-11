# Phase 10.1 integrated engineering evidence successor

Status: IN PROGRESS — implementation certified; independent Principal Architect review required.
Initial product baseline: `9184cf4d6c58ada39ba1b0538d4c32eedf6853d8`.
This record supersedes phase-relative environment/ownership/missing-observation
statuses in the original roadmap and Step 1–5 reports. Those reports and their
fingerprints remain immutable. No Phase 10.1 closure SHA is asserted here.
Phase 11 and Phase 12 remain NOT STARTED.

## Acceptance matrix

| Step | Engineering result | Evidence and remaining condition |
| --- | --- | --- |
| 1 Environment | VERIFIED within observed host scope | environment.json; Apple M5 Max, macOS 27.2.1 build 26B131; Xcode 27.2 27B52; Swift 6.4.2; Clang 21; CMake 4.4.5; Node 26.13.0; Temurin 21.0.10; Android SDK 36 / NDK 28.2.13676358 / CMake 3.22.1 / emulator 37.2.13; Git/gh authenticated; fetched main equals baseline. Mac Studio is primary for BOTH platforms under the Product Owner's explicit successor decision. |
| 2 Apple integration | VERIFIED for accepted source and recorded host/Simulator targets | Retained generic Debug/Release builds, release scanners, app unit 11/11 + UI 18/18, host CTest 65/65, macOS core 53/53, iPhone Duo Simulator core 53/53. Result bundles re-inspected; no skipped tests. No physical iPhone claim. |
| 3 Dual-platform parity | VERIFIED within software scope | Android retained Debug/Release, JVM 7/7 each, lint, release isolation/scanner, ARM64 + ARMv7 libraries built and packaged, ARM64 emulator instrumentation 58/58. Same-source scenario matrix below and source manifest; old two-machine experiment did not occur and is replaced explicitly. ARMv7 runtime and mobile GPU behavior remain unqualified. |
| 4 Host performance/resources | VERIFIED within measured host scope | New Release native host harness, 2 warmups + 7 samples at 1024x768 and 3000x2000; 12/12 focused resource/fault tests; leaks instrument reports 0 leaked bytes. Raw samples and limitations below. No physical-mobile tier/battery/thermal claim. |
| 5 Integrated certification/closure | HOSTED IMPLEMENTATION VERIFIED / INDEPENDENT REVIEW REQUIRED | Implementation `668a79741c198e9d5c39772a4514fb0731720a76`: Foundation 43053578303 7/7, Native 43053578225 3/3, exact SHA and attempt 1. Review and distinct formal closure commit with its own 10 successful jobs remain mandatory. This documentation evidence receipt is not formal closure. |

## Evidence provenance and reuse

All paths in this section are repository-relative. Exported evidence is under
`docs/reports/evidence/phase-10.1-local/`. The product-source manifest records
354 owned UTF-8 source/configuration files, SHA-256 after LF normalization.
Vendor/binary inputs retain existing repository checksum/build gates. A new test
in the existing hardware gate suite checks every manifest entry and harness digest.
It retains every original historical hardware-gate assertion unchanged.

Accepted local Apple results belong to source SHA
`dcef1ac59e718ac17425c1b6086dd6f9840ac311`, macOS 27.2 build 26B123, Xcode 27.2,
iPhone Duo iOS 27.2 build 24B91016s ARM64 Simulator. `git diff` from that SHA to
baseline contains only the Android macOS aapt2 checksum and Step 3/roadmap records;
Apple/shared product bytes are unchanged. Retained source attribution uses the
original Step 2 report and logs; result bundles alone do not embed a Git certificate.
These results are not relabelled as new macOS 27.2.1 test runs. New host performance,
resource and leak observations below belong to macOS 27.2.1 on the same Mac Studio.
The Simulator runtime and product source are unchanged, so costly platform suites
were not repeated just for this documentation/instrumentation assignment.

Re-inspected artifacts:

- `build/ios-shell-results.Yqznsy/tests.xcresult`: 29 passed, 0 failed/skipped;
  `build/cert-ios-shell-check-3.log` distinguishes 11 unit and 18 UI, generic
  Debug/Release and release scanner results.
- `build/apple-results.TgA9yE/ios-tests.xcresult`: core 53 passed, 0 failed/skipped.
- `build/apple-swift/Logs/Test/Test-PixAuraCore-2026.11.08_21-26-17-+0700.xcresult`:
  host core 53 passed, 0 failed/skipped.
- `build/cert-apple-check-2.log`: accepted full local Apple check, CTest 65/65.
- `platforms/android/app/build/outputs/androidTest-results/connected/debug/TEST-*.xml`:
  58 tests, 0 failures/errors/skipped; exported case names and artifact digest.
- `platforms/android/app/build/test-results/test{Debug,Release}UnitTest/TEST-*.xml`:
  7/7 each; exported counts and artifact digests.

The accepted Android local build/lint/scanner/ABI observations are preserved in
Step 3 and the mission baseline. This audit re-reads retained test artifacts; it
does not invent fresh Gradle builds. No product/platform configuration was changed.

Baseline GitHub API audit confirms Foundation `42495980796`, workflow ID
`372386439`, 7/7, and Native shells `42495980827`, workflow ID `373262165`, 3/3:
exact baseline SHA, attempt 1, all required jobs completed/success. JSON receipts
preserve job IDs/names/results. These do not certify the new implementation SHA.

## Cross-platform semantic parity

Paths below are relative to `platforms/android/app/src/androidTest/java/ai/pixaura/app/`
and `platforms/ios/` respectively. Full retained suites executed the listed test
classes. Shared-core tests provide exact numerical and state-authority coverage;
UI tests are not substitutes for native approval checks.

| Contract / scenario | Android evidence | Apple evidence |
| --- | --- | --- |
| Import, owned image, source provenance, close/reopen | DecodeBoundaryTest, AssetPublicationTest, ManualEditorTest | PixAuraCoreTests decode/storage; ShellUITests.testManualEditorGestureHistoryCancelBackgroundAndReopen |
| Every manual tool, gesture Apply/Cancel, preview not publication | ManualEditorTest.certifyEveryDescriptorValidationCancelCommitUndoRedoAndReopen | PixAuraCoreTests manual boundary; ShellUITests manual gesture/history case |
| Assist non-mutating; suggestion navigates; explicit manual Apply | AssistExperienceTest | ShellUITests.testAssistAdviceDismissNavigationAndExplicitManualApply |
| Auto proposal read-only; review abandonment and cancellation | AutoProposalTest, AutoReviewTest.userCancelAndModeDepartureFenceExecutionAndRequireFreshProposal | ShellUITests.testAutoProposalIsReadOnlyAndModeAbandonmentDoesNotRestoreIt; testAutoCancellationAndProviderFailureLeaveManualAvailable |
| Exact Plan inspection/order, editable review, stale authority retirement | PlanInspectionTest, ReviewEditTest | ShellUITests.testPlanInspectionMultiActionOrderUsesExactRawValuesAndSamePreview; testEditableReviewReorderRemoveAndRestoreRebuildExactInspection |
| Preview failure/cancellation retains manual ability | ReviewPreviewTest | ShellUITests.testReviewPreviewCancellationFailureAndDepartureKeepManualAvailable |
| HUMAN single-use source-bound approval, one ordinary revision | AutoReviewTest.explicitHumanApplyBindsExactReviewOnceAndSharesManualUndoRedoHistory; AgentPlatformTest | ShellUITests.testReviewHumanApplyOneOrdinaryRevisionAndManualUndoRedo; core agent execution tests |
| Undo/Redo and committed Result Review | ResultReviewTest | ShellUITests.testResultReviewUsesOrdinaryUndoRedoAndLeavesEditing |
| Background/foreground, recreation, late callbacks, stale source | AutoReviewTest.recreationStopsAndJoinsApplyingOwnerWithoutPublicationOrApprovalRecovery; AndroidCapabilityLifecycleTest | ShellUITests.testIntegratedModeBackgroundNavigationAndRestartNeverRestoreReviewAuthority; ShellTests.testInterruptDropsAutoAuthorityAndManualGestureWithoutRevision |
| Export JPEG/PNG, failure and destinations without revision | ExportDestinationTest; native export_test | ShellUITests.testExportControlsStayAvailableInManualMode; ShellTests / core export coverage |
| Resource limits, downgrade and failure recovery | AndroidCapabilityIntegrationTest, AndroidCapabilityLifecycleTest, CrossTierReplayTest | PixAuraCoreTests capability/resource cases; shared fault suites; host instrumentation |
| English/Vietnamese presentation | Catalog equivalence/source assertions in tests/product-ux-locale.test.mjs | Same catalog assertions; retained UI harness explicitly uses English. Earlier Vietnamese catalog observation is in Step 2; a dedicated full Vietnamese interactive suite is not claimed. |

The old separate-Windows/Mac side-by-side test remains unexecuted. Its workflow
requirement is explicitly superseded by `docs/plans/phase-10.1-mac-successor.md`.
Equivalent contract checks now run on both platform runtimes against unchanged
shared source, plus retained Windows CI and exact-SHA integration workflows.
`check-android-emulator.sh` remains the Linux CI orchestrator: GNU `realpath -m`
is unavailable on macOS. Its 19 historical local failures are a host portability
limitation, not suppressed product tests. Direct authorized ARM64 emulator Gradle
execution is already observed. No change to Linux coverage is required.

## New Mac Studio CPU and resource observations

Reproduce: `python3 scripts/benchmarks/mac-host.py`. Python standard-library PNG
generation uses synthetic RGB pixels only; no user photos. The script builds the
unchanged native core Release (`-O3 -DNDEBUG`, strict floating point), builds the
engineering harness `-O2`, and invokes the public C ABI. No production dependency,
ABI, model, schema, GPU admission, or editing implementation changes.

Each size runs in its own process: 2 complete warmup lifetimes, 7 measured
lifetimes. Each lifetime imports, renders four gesture previews, cancels four
edits, explicitly applies one edit, checks Undo/Redo, requests and abandons a
proposal, performs 100 exact review inspections, queries state 100 times, exports
PNG/JPEG, forces a short-output-buffer failure, verifies ordinary manual editing
and interruption, stops/destroys, reopens with a fresh identity, validates committed
revision preservation, then stops/destroys again. Thus 36 owner lifetimes and 72
preview renders across both sizes, including warmups. Timings use steady_clock.
Open includes IO/import/decode and initial native preparation. Preview includes
CPU evaluation, fit and pixel copy. Apply persists the edit; full display rendering
is separately timed. No isolated shader/GPU time is inferred.

Medians, with measured minimum–maximum in parentheses, milliseconds:

| Operation | 1024x768 | 3000x2000 (6 MP) |
| --- | --- | --- |
| Open/import | 47.138 (43.305–49.300) | 85.273 (83.425–89.449) |
| Four preview renders total | 55.978 (55.325–56.130) | 188.329 (187.595–189.346) |
| Apply | 9.861 (9.701–10.813) | 15.347 (15.129–16.267) |
| Proposal request | 0.061 (0.058–0.065) | 0.066 (0.063–0.069) |
| 100 native review inspections total | 151.691 (150.853–152.070) | 382.177 (381.583–386.844) |
| PNG export | 31.724 (31.583–31.968) | 169.382 (169.159–170.478) |
| JPEG export, quality 90 | 17.425 (17.291–17.476) | 131.121 (130.829–131.790) |
| Stop/destroy | 7.245 (6.178–8.322) | 7.332 (6.342–7.454) |

Raw microseconds, cancellation batches, state-query batches, warmups, fixture
hashes, sizes, harness hashes and min/median/max are in host-measurements.json.
Zero-microsecond cancellation samples indicate timer resolution, not zero work.
There is no p95 claim from seven samples and no new pass threshold. Existing mobile
frame/preview/export targets in QUALITY_GATES.md remain unchanged and are not
certified by these synthetic host samples. A 12 MP mobile export was not measured.

`getrusage(RUSAGE_SELF).ru_maxrss` on Darwin reports bytes: maximum observed
116,277,248 bytes (110.89 MiB) at 1024x768 and 328,777,728 bytes (313.55 MiB)
at 6 MP. These are cumulative per-process high-water values, including native
rasters, input, SQLite, encoder allocations, 32 MiB output and 4 MiB preview
caller buffers and runtime overhead. They are neither live retained native bytes
nor a process-RSS admission limit. The last five measured high-water points grow
only within the reported range; no claim of a universal memory bound follows.
Frozen native raster/resource lease caps remain authoritative; process RSS can
exceed those individual accounting classes.

Focused existing CTest cases passed 12/12 on this host: intelligence resources,
review preview/edit allocations, review edit, export, review preview, editor,
gesture lifecycle, preview, preview allocations, decode allocations and document
allocations. These inject actual allocation failures and check cleanup, admission,
leases, cancellation, owner retirement and manual recovery. They complement the
new observed happy-path lifetimes; short-buffer rejection alone is not an
allocation-failure test. Retained platform lifecycle cases cover background and
worker stop/join ordering. Hosted ASan/UBSan remains required for the final SHA.

Apple `/usr/bin/leaks --atExit --` ran the final harness separately at each size.
Both reported 0 leaks / 0 total leaked bytes. Logs preserve the instrument's own
footprint observations separately from uninstrumented getrusage samples. This is
exit-time leak observation for these workloads, not a proof against every leak.
The harness has no platform callback queue; stale callbacks/queue shutdown are
covered by retained platform/native concurrency regressions, not invented here.

Two engineering-harness defects were diagnosed without changing product code:
reopening with the same session identity correctly failed native stale validation;
auto Plan bytes are exact canonical bytes, not a NUL-terminated binding transport.
The corrected harness uses a fresh identity and exact Plan length. Failed attempts
are diagnostic logs, not accepted timing samples.

## Hardware admission and precise external prerequisites

- `cpu.reference.v1`: VALIDATED (existing trust; no new trust promotion).
- `apple.metal.v1`: UNKNOWN. `android.vulkan.v1`: UNKNOWN.
- `DH-APPLE-METAL-01`: DEFERRED / NOT WAIVED / NOT SATISFIED.

Mac Studio can compile Metal shaders, run native CPU/resource tests, and provide
host timings. None promotes mobile Metal, Vulkan, battery/thermal or physical tier
trust. The historical Fold7 tile PASS 14/14 in the Phase 2 Android hardware report
remains that frozen identity/exposure scope and source SHA; it does not admit the
current release inference backend or certify current ARMv7/mobile runtime.

Discovery: no Android device attached; no available physical Apple target.
Apple discovery lists an unavailable iPad8,9, whose eligibility is untested.
No device serial, signing identity, credentials or user photos are exported.

To discharge DH-APPLE-METAL-01: provide a supported connected/unlocked/trusted
physical iPhone/iPad, Developer Mode and existing owner-authorized signing/team/
provisioning, clean committed exact checkout, then run
`bash scripts/check-gpu-apple-hardware.sh 'platform=iOS,name=DEVICE_NAME' EXISTING_TEAM_ID`.
Require actual hardware classification, Metal metadata/pipeline, completed dispatch
and readback, all 14/14 CPU parity cases with fallback disabled, exact source SHA,
reviewed JSON XCTest attachment, xcresult/log and date. Unsupported hardware,
Simulator, Mac benchmarks and successful process exit without JSON PASS cannot
satisfy the gate. No signing/account setup was attempted.

Current physical Android Vulkan qualification needs a supported owner-authorized
physical device, authorized ADB, wrapper invocation with a fresh run/source SHA,
real hardware pipeline/dispatch/readback and 14/14 parity without fallback.
ARMv7 runtime qualification needs a compatible physical 32-bit runtime; packaged
ELF32 and hosted ABI compilation alone are build evidence. Full production backend
promotion additionally requires the existing inference/admission protocols and
resource/performance evidence; tile PASS alone never grants it.

These unavailable physical checks remain explicit external prerequisites at their
existing required boundaries. The Phase 10.1 roadmap specifically separates Mac
measurements from mobile Metal and leaves UNKNOWN valid; this report does not
waive, remove or postpone any existing physical gate. Independent certification
review must confirm the acceptance scope before formal closure.

## Accelerator inference admission audit supplement

The separate `docs/contracts/inference-provider-certification-v1.md` requires
full tensor comparisons on all 15 accepted weight triples at 256x256 and retained
1x1/2x2/8x8/1024x1024/normalization cases, including zero/one/gradient/checker/IEEE
boundary vectors and independent dyadic CPU oracles. Vulkan/Metal probability
parity is `abs(accelerator-CPU) <= 4e-38 + 3e-7*abs(CPU)`; classification around
0.5 follows the frozen tolerance-band rule. This is distinct from exposure-tile
14/14 certification. Signed package/model verification, full malformed/input/output,
handle/context, cancellation/publication, allocation/resource, trust/tier/thermal,
fallback and release-isolation matrices remain required. Metal transient reservation
is 1,114,112 bytes; Vulkan is 1,638,400 bytes, with at most 16,384 pixels per tile;
these are adapter bounds, not measured driver/process RSS ceilings.

`inference-admission-v1.md` requires BOTH a reviewed descriptor validation at least
RUNTIME_VALIDATED and matching native platform capability permission. Availability,
model/device name, successful direct dispatch or host latency cannot grant this.
UNKNOWN descriptors continue to select CPU automatically. Promotion additionally
needs the separately approved evidence policy/existing explicit authority required
by the certification contract. This mission adds no such policy, model or promotion.

## Implementation certification and formal closure boundary

Implementation SHA: `668a79741c198e9d5c39772a4514fb0731720a76`.
Foundation `43053578303` is completed/SUCCESS 7/7, workflow ID `372386439`.
Native shells `43053578225` is completed/SUCCESS 3/3, workflow ID `373262165`.
Both are main push events, exact implementation SHA, attempt 1. Job membership
was checked against all seven/three required names, with every job completed and
successful; there are no skipped, failed, hidden extra or foreign-SHA jobs.
Exported `implementation-*.json` receipts record job IDs, dates and URLs.

The follow-up documentation evidence commit records these observed results,
GPU detection-only metadata and the admission audit. It changes no product source,
harness, tests, ABI, schema, workflow or platform configuration. It still requires
both complete workflows on its own SHA. Its observed receipt is written locally
to `build/phase-10.1-final-engineering-receipt.json` after those workflows finish
and reported in the final engineering delivery, avoiding a self-referential SHA.
It is an evidence receipt, not the formal Phase 10.1 closure commit.

The remaining binding gate is independent Principal Architect certification review,
as required by the active AGENTS.md successor and the mission's separation of
engineering from independent certification. The engineering agent cannot create
that independent review itself. Review must assess the recorded target/source
reuse, same-source dual-platform successor, host resource measurements and the
existing physical-gate timing. No new Product Owner decision is requested for
ordinary engineering actions. Every identified executable engineering task has
been addressed; no product defect or in-scope source fix remains known.

After independent acceptance, create a distinct documentation-focused formal
closure commit and require both complete exact-SHA workflows, attempt and all
10 job outcomes recorded, before declaring CLOSED / COMPLETE / FULL PASS.
Until then formal closure SHA is absent and Phase 10.1 stays IN PROGRESS.
Physical mobile evidence remains unavailable at its existing deferred gates;
independent review must not relabel it PASS or waive DH-APPLE-METAL-01.

Fresh implementation hosted log audit: source suite 235/235 passed, fail 0;
Android build/JVM/lint/test-APK stage BUILD SUCCESSFUL and emulator stage executed
58 tests followed by BUILD SUCCESSFUL; Apple retained core CTest 65/65, package
53/53 and app unit 11/11 plus UI 18/18, 0 failures. These are hosted results,
separate from the retained Mac Studio observations above. Raw implementation
workflow log is saved locally at `build/phase-10.1-implementation-shells-ci.log`.
