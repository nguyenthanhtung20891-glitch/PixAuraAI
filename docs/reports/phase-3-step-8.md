# Phase 3 Step 8 — Certification & Closure

Status: **IN PROGRESS**. **Phase 3 — OPEN / CERTIFICATION IN PROGRESS.** Authorized baseline:
`c0a0020d41b22b55fad494fab50d72b5903f7c22`. Steps 1–7 remain accepted FULL PASS.
Steps 1–7 remain accepted. Final exact-SHA iOS UI revalidation failed; closure
is withheld until remediation and all exact-SHA gates pass. Phase 4 has NOT started.
Implementation certification SHA: `f9faf60ea98ff38bc5a6fa6fd35540791918bf3b`.

The [authoritative certification matrix](../contracts/phase-3-certification-v1.md)
consolidates all 11 accepted tools, metadata, platform parity and test traceability.
Step 8 adds certification tests, an explicit SELinux Enforcing runtime precondition
and closure documentation. It adds no feature or new architecture decision.
Accepted ADR 0021 remains narrow and unchanged; Schema 3 is unchanged.

## Certification criteria and retained passing evidence

- Independent geometry/tone/detail reference corpus, invalid/boundary/alpha/order coverage.
- Gesture/history, generation fences, deterministic races, churn, allocation/failure/resource/security regressions.
- Schemas 1/2/3 and explicit migrations, immutable assets, crash/reopen and deterministic replay.
- Every descriptor through neutral/invalid/cancel/changed preview/commit/Undo/Redo/reopen on Android and Swift owners.
- Actual hosted Android runtime, enforced SELinux and direct Android RENAME_NOREPLACE corpus.
- Native Windows/Linux/Apple, ASan/UBSan, both Android ABI boundaries, builds/lint/JVM/instrumentation.
- Swift macOS/simulator, iOS Debug/Release builds, app/UI flows and basic layout/accessibility.
- Foundation boundaries (7 jobs) and Native application shells (3 jobs), all SUCCESS on the same final closure SHA.
- Source/documentation/workflow checks and git diff --check.

## Observed local gates

- Source tests: **80/80**, zero failures/skips, including matrix/descriptor consistency.
- Windows existing Clang/Zig fallback: **28/28 native tests**, 167.37 seconds,
  including full allocation faults, gesture races and storage regressions.
- Independent tone fixture/matrix check PASS (940 fixtures); exact-rational
  detail corpus PASS (**280 fixtures**). Existing geometry independent references retained.
- Android Debug/Release, strict dependency verification, both lint variants,
  JVM variants and instrumentation APK build PASS (151 Gradle tasks).
- Workflow actionlint, affected shell syntax and git diff --check PASS.
- Local WSL has no Node executable; the 36 emulator-harness tests were executed
  on hosted Linux. Actual Android and Apple runtime evidence is recorded below.

No unexecuted or skipped gate is counted PASS. Full hosted evidence follows.

## Certification remediation

At certification SHA `6466d67f4267c47764b5654b84fe896c9cbf5bd7`,
[Foundation 38139283398](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/38139283398)
passed all 7 jobs. Source passed 116/116. Android builds/lint/JVM passed and
runtime explicitly observed SELinux Enforcing, direct ADR 0021 publication and
the new every-tool certification test. However,
[Android job 114469601362](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/38139283404/job/114469601362)
crashed during the existing Compose activity recreation test: SlotWriter.moveSlotGapTo
negative index while disposing Scaffold's content subcomposition. Its aborted
12/15 run is FAILURE, never certification PASS. Local API 36.1 reproduction
passed 3/3 owner/UI tests, showing this teardown failure is not universal.

Required Category A remediation replaces the unused Scaffold slot container
with a regular themed Surface and safe drawing insets, removing the implicated
subcomposition teardown path without adding controls or changing editing semantics.
The same UI test now performs three recreations and waits for a newly rendered
display object plus completed preview, so retained pre-reopen state cannot satisfy
the test. Dependency pins, tests, storage, lifecycle authority and security remain
unchanged. The corrected exact-SHA full hosted matrix below passed.

Affected local rerun: Android Debug/Release/JVM/lint/instrumentation build PASS;
source 80/80 PASS; actual API 36.1 owner/UI instrumentation **3/3 PASS** in
25.397 seconds, including three recreations with fresh rendered reopen copies.
The first certification iOS job independently passed Debug/Release builds,
6 app tests and 2 UI tests (edit/history/cancel/background/reopen/landscape).
The simulator's Metal test asserts UNSUPPORTED; it is not physical certification.

## Exact-SHA hosted certification

Current closure attempt `e0c5fc3d4176344a616ad88ed6394b1fe619cac4` is **NOT accepted**:
Foundation 38142375346 passed 7/7; shells 38142375391 passed source/Android but
iOS job 114478691819 failed its manual-editor UI test. Its native 28/28,
Swift macOS/simulator 14/14 each, both app builds and 6/6 app tests passed;
one of two UI methods failed (two assertions). No earlier green substitutes.

The exact xcresult hierarchy after Exposure's synthesized tap shows Tone/Color
Selected, Exposure not Selected, Apply/Cancel disabled, revision 1 and no core
error. Exposure's rectangle is x227.3/y505.0/w95.0/h34.3; its center y522.2
overlaps the horizontal scrollbar accessibility region y519.2..536.4.
The test treated event synthesis as selection success. Category A remediation
targets the visible upper quarter of the existing tool button, uses its already
shared-core ID, and waits for its core-backed Selected trait before seeking
controls. No tap retries, sleeps, timeout increase, assertion removal, production
SwiftUI change or semantic change. Failed targeting now reports the hierarchy
at the actual selection boundary. All original end-to-end assertions remain.

The passing implementation certification below is retained historical evidence;
fresh complete exact-SHA closure is still required.

All jobs below ran on **the same** implementation SHA
`f9faf60ea98ff38bc5a6fa6fd35540791918bf3b`, with SUCCESS conclusions:

| Workflow / job | Run ID | Job ID | Observed result |
| --- | --- | --- | --- |
| Foundation / workflow-lint | 38140611496 | 114473541445 | PASS |
| Foundation / Windows MSVC Debug | 38140611496 | 114473541446 | 28/28 native PASS |
| Foundation / Linux | 38140611496 | 114473541511 | 29/29 native PASS, including independent Android publication target |
| Foundation / ASan+UBSan | 38140611496 | 114473541456 | 31/31 PASS; active negative probes exit 86/87 |
| Foundation / Apple boundary | 38140611496 | 114473541349 | 28/28 native, 14/14 macOS Swift, 14/14 simulator Swift PASS |
| Foundation / Android armeabi-v7a | 38140611496 | 114473541570 | native boundary build PASS |
| Foundation / Android arm64-v8a | 38140611496 | 114473541628 | native boundary build PASS |
| Native shells / source | 38140611504 | 114473541360 | 116/116, zero failures/skips |
| Native shells / Android app | 38140611504 | 114473541461 | Debug/Release/lint; JVM 3/3 each; instrumentation 15/15, zero failures/errors/skips |
| Native shells / iOS app | 38140611504 | 114473541521 | native/Swift/simulator gates; Debug/Release builds; app 6/6 and UI 2/2 PASS |

[Foundation 7/7 SUCCESS](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/38140611496)
and [Native application shells 3/3 SUCCESS](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/38140611504):
**10/10 jobs**. No prior-SHA green substitutes for these results.

The previous closure commit changed documentation only.
Delivery is gated on observing BOTH complete workflows again on that closure
commit's exact SHA; the final delivery supplies its SHA, run IDs and job conclusions.
The implementation and test corpus remain identical to the certified SHA above.

## Tool matrix, parity and numerical references

The [single authoritative matrix](../contracts/phase-3-certification-v1.md) certifies
Crop, orthogonal rotation, Exposure, Brightness, Contrast, Highlights, Shadows,
Saturation, Temperature, Sharpen and Blur on core/Compose/SwiftUI, all PASS.
Each shared stable ID equals its operation type; versions are 1/1, step 1 integers,
and exact units/ranges/defaults are recorded there. Parameter-row checks match all
14 shared parameter descriptors. Screens consume shared metadata, validation,
lifecycle, history and replay authority without platform-local shadow rules.
Every tool's neutral/invalid/cancel/changed preview/commit/Undo/Redo/reopen is
observed through platform owners. Crop remains coupled integer ppm with shared
outward rasterization; rotation remains 0/1/2/3 clockwise quarters.

Independent references remain independent: 940 Decimal tone fixtures, 280 exact
rational detail fixtures, literal geometry permutations and independent ppm
floor/ceil references with 2048 mixed geometry properties. Corpora cover bounds,
neutral/default, alpha 0/partial/1, dark/mid/high/HDR values, impulses, edges,
checkerboards, borders, repeated operations and order non-commutativity. Same-platform
reopen/replay pixel identity is observed; accepted CPU tolerances remain unchanged,
with no universal cross-GPU bit-identity claim.

## Gesture, preview, history, faults and resources

Explicit IDLE -> ACTIVE -> COMMITTING -> COMPLETED, or CANCELLED/INVALIDATED,
remains shared. BEGIN binds immutable base/source/session/tool/version without
history. UPDATE retains one pending edit; invalid input preserves the last valid
state. 1000/10000 churn remains bounded with no per-tick revisions. Changed Apply
creates exactly one detached immutable proposal and normal approved durable revision;
neutral/cancel creates zero. Repeated Apply rejects, cancellation is idempotent.
Allocation/admission/final-validation failure is retryable; storage approval retry
reuses the proposal; stale ownership invalidates safely, preventing double commit.

PRV1 generation fences reject superseded/late output regardless of cancellation
timing. Failed newest preview preserves prior valid display; detached platform
copies survive native release. Barrier-controlled late-preview/update/cancel/commit,
tool-switch, interruption, teardown and isolated-context races pass. Tool switch
cancels first; background/navigation/disposal cancels, with stop/join before destroy.
Undo/Redo and preset application reject while ACTIVE; no hidden auto-commit.

Linux gesture suite: **200506 checks**, editor **50015 checks / 3740 recovered
allocation failures**. Windows: gesture **229502 checks**, editor **153451 checks /
11648 recovered allocation failures**. Existing parser, decode, preset, preview,
detail scratch and admission fault suites also pass under native and sanitizers.
Preview includes **128 cancellation races** and **2048 properties**. No timing-only
test substitutes for ownership fences.

All frozen ceilings are unchanged: one active gesture/context, <=1024-byte pending
operation, one preview ticket, <=256 ordered stack, <=4096 stored operations/revisions,
64 combined document/gesture handles, <=8 MiB manifest/64 KiB command, <=128 MiB raster/
8388608 pixels and <=256 MiB context. Detail scratch <=786432 bytes within shared
1 MiB scratch; platform detached copies <=4 MiB and existing render admission remain.
No unbounded task/update queue, retained operation cloning or ceiling increase.

## Storage, security, presets and ADR 0021

Schemas 1/2 historical compatibility and Schema 3 current authority PASS. Explicit
1->2 and 2->3 transactional migrations, rejected direct 1->3/no automatic migration,
exact schema fingerprints, IDs, source SHA-256, ordered stacks, redo, current revision
and session/history preservation pass. Restart/reopen reconstructs committed pixels
and history on both platforms. No Schema 4 or new migration exists.

Accepted ADR 0021 is unchanged: Android-only renameat2(RENAME_NOREPLACE), pinned
private staging/final capabilities, synced read-only staging file, directory syncs,
exact digest/length verification and asset-before-SQLite-reference. EEXIST verifies
dedup; corruption is rejected without replacement. Unsupported/error returns IO_ERROR
without fallback. Linux linkat, Darwin exclusive rename and Windows non-replacing
move remain unchanged. Android artifact selinux-policy.txt explicitly says **Enforcing**;
actual AssetPublicationTest logcat says **RENAME_NOREPLACE actual branch: 71 checks PASS**.
The independent native Android branch records four real no-replace publications,
same/different-digest concurrency, existing-correct/corrupt dedup, staged-reader and
post-file-sync failure, both directory sync failures, ENOSYS/EOPNOTSUPP, all nine
publication/SQLite crash boundaries and ten migration crash boundaries. Full existing
storage regressions pass. No SELinux, NOFOLLOW or immutable protection was weakened.

Preset foundation PASS: stable IDs, canonical integer-only versions/payload, ordered
inspectable constituents, malformed/unknown/executable/geometry rejection and bounds
(16384 bytes/16 operations per recipe, 16 recipes/262144 catalog bytes, 1024 binding
bytes). Changed batch = one revision, neutral = zero, no live-gesture interleaving.
Shipping catalog remains empty on both platforms; test reference recipes do not ship.

Malformed/truncated data, unknown versions/IDs, invalid parameters, path traversal,
symlink/reparse/alias attacks, storage tamper, no-overwrite and resource rejection
regressions PASS. No script/plugin execution or production network dependency added.

## Platform UX and scope

Existing Compose/SwiftUI preview, Geometry/Tone-Color/Detail/Filters navigation,
descriptor controls, value display, Apply/Cancel/Undo/Redo and loading/error states
remain. Shared authority governs every editing action. Android actual UI coverage
includes semantic slider actions, scrolling, cancellation/background and **three
recreations**, requiring fresh rendered display copies and deterministic committed
pixels. iOS UI covers crop/scalar/rotation, history/cancel/background/reopen and
landscape; Swift owner coverage adds every detail/tone tool's changed reopen path.
Native accessibility labels/semantics, selected/disabled states, loading/error
announcements and system-safe scroll layouts are retained. This is basic usability
coverage, not physical-device/full accessibility certification or production polish.

Step 8 changed only tests, certification documentation, the enforcing runtime gate
and the narrow Android shell teardown correction. Exact affected files:

- ARCHITECTURE.md, DECISIONS.md, QUALITY_GATES.md, ROADMAP.md
- docs/contracts/phase-3-certification-v1.md, docs/reports/phase-3-step-8.md
- packages/core/tests/editor_test.cpp, packages/core/swift/Tests/ManualEditorTests.swift
- platforms/android/app/src/androidTest/java/ai/pixaura/app/ManualEditorTest.kt
- platforms/android/app/src/main/java/ai/pixaura/app/MainActivity.kt
- platforms/ios/PixAuraAIUITests/ShellUITests.swift
- scripts/check-android-emulator.sh, tests/android-emulator.test.mjs
- tests/manual-editor.test.mjs, tests/manual-geometry.test.mjs, tests/tone-color.test.mjs
- tests/detail-tools.test.mjs, tests/presets.test.mjs, tests/gesture-lifecycle.test.mjs

## Follow-ups and scope

A: narrow iOS UI input-targeting remediation and complete exact-SHA closure remain
required. Android teardown remediation and all retained certification evidence pass.
B: DH-APPLE-METAL-01 remains **DEFERRED**, mandatory before **Phase 11 hardening
acceptance, Phase 12 Beta readiness completion, production release, or any physical
Apple GPU certification claim**. It is not waived; Phase 2 is not reopened.
C: optional production polish and future catalogs/tools remain outside this step.
No unresolved architectural conflict identified. No Step 9 is created; no Phase 4 implementation.
No new tools/presets, numerical semantics, UI feature, AI, GPU expansion,
network/cloud, telemetry or storage-schema scope was added. STOP after Phase 3 closure.
