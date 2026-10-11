# Phase 3 Step 7: Cross-platform Manual Editor Integration

Status: COMPLETE / FULL PASS (technical gates). Baseline
`116c4efc65f55025f717241a01bc6056ed27d042`. Steps 1–6 accepted FULL PASS.
Step 8 NOT STARTED; DH-APPLE-METAL-01 unchanged/unexecuted.

## Acceptance and implementation

[Integration contract](../contracts/manual-editor-integration-v1.md) defines
shared authority, explicit append adjustments, integers, application work lanes,
PRV1 owner/fences, approval retry and schema-3 reopen.
Compose/SwiftUI provide picker/preview/categories/controls/default/Apply/Cancel/
Undo/Redo plus loading/error/empty-preset states. Crop uses exact ppm fields;
rotation buttons derive their values from the descriptor. No platform kernels,
operation serialization, no-op detection, history or preset rules are introduced.

## Parity matrix

Both platforms read the same compiled registry. Every row uses /1/1 and step 1;
changed Apply = one revision, neutral/cancel = zero; history/reopen/replay is shared.

| Tool | Shared ID | Integer parameter/range/default | Android / iOS |
| --- | --- | --- | --- |
| Crop | pixaura.crop | x/y 0..999999 / 0; width/height 1..1000000 / 1000000 | ppm fields / ppm fields |
| Rotate | pixaura.rotate | quarter_turns 0..3 / 0 | clockwise buttons / clockwise buttons |
| Exposure | pixaura.exposure | milli_ev -5000..5000 / 0 | descriptor slider / descriptor slider |
| Brightness | pixaura.brightness | milli_linear -1000..1000 / 0 | descriptor slider / descriptor slider |
| Contrast | pixaura.contrast | milli_stops -2000..2000 / 0 | descriptor slider / descriptor slider |
| Highlights | pixaura.highlights | milli_ev -2000..2000 / 0 | descriptor slider / descriptor slider |
| Shadows | pixaura.shadows | milli_ev -2000..2000 / 0 | descriptor slider / descriptor slider |
| Saturation | pixaura.saturation | milli_ratio 0..2000 / 1000 | descriptor slider / descriptor slider |
| Temperature | pixaura.temperature | kelvin 4000..25000 / 6504 | descriptor slider / descriptor slider |
| Sharpen | pixaura.sharpen | milli_amount 0..1000 / 0 | descriptor slider / descriptor slider |
| Blur | pixaura.blur | milli_strength 0..1000 / 0 | descriptor slider / descriptor slider |
| Filters/Presets | existing empty catalog | no shipping recipe | visible empty state / visible empty state |

## Validation

Android embeds the manual screen in the existing Compose editor destination;
iOS embeds it in the existing SwiftUI shell. Each has a bounded-fit preview,
scrollable category/tool rows, descriptor controls, default reset, explicit Apply
and Cancel, and core-derived Undo/Redo availability. System JPEG/PNG pickers copy
immutable source bytes into private managed project storage off the UI thread.

Scalar positions map to the descriptor integer lattice, ties toward the larger
integer on both platforms. EV/K/percentage formatting is presentation only.
Crop's smallest usable interaction is four exact ppm fields plus Preview crop;
invalid rectangles are rejected by core, never clamped. Rotation uses descriptor
quarter turns and clockwise buttons; tone/color and detail use the same generic
scalar controls. Filters/Presets visibly explains the empty shipping catalog.

Selection begins a neutral append adjustment; updates retain one pending logical
payload and request Step 6/PRV1 preview. Two fixed work lanes keep rendering and
approval off the main thread. There is one coalesced input, one pending preview,
one in-flight render and bounded detached display copies. Actual display install
and lifecycle use the same owner lock and native generation fence. Swift orders
its bounded candidate notification after the requesting command's state event,
so a tiny preview cannot arrive before its UI owner is bound.

Apply approves/persists one detached proposal; unchanged Apply creates no
revision. Cancel, tool/category switch, background, navigation and disposal cancel
without auto-commit and restore the committed display copy. Undo/Redo are disabled
while an adjustment or approval retry is open and use native history. Re-entry
opens schema 3's committed state with a fresh session; no transient gesture is
restored. Invalid updates retain the last accepted values. Allocation/admission,
stale, preview and commit failures are visible; failed newest preview preserves
the previous image. Retry Apply reuses the exact proposal. Stale approval cancels
and exposes reopen recovery. No platform shadow history exists.

Accessibility names tools/actions/sliders, exposes values and selection, and
announces errors/loading with native semantics. Scrollable content/tool rows,
bounded preview and existing shell safe areas provide phone layout resilience;
iOS UI automation exercises landscape and return/reopen. Broader accessibility,
device-size certification and production polish remain Step 8/later scope.

Implementation acceptance SHA: `3f3e10e61814cb4b90617443c4b37c7e9d7688b2`.
Observed [Foundation 7/7](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/38129297272)
and [application shells 3/3](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/38129297283)
on that exact SHA. The final delivery also observes both workflows on the
documentation closure SHA; these links record implementation acceptance evidence.

| Executed gate | Result |
| --- | --- |
| Windows MSVC native | 28/28 PASS; editor 153,231 assertions, 11,648 recovered allocation failures, 380.09 seconds |
| Linux native | 29/29 PASS; editor 49,795 assertions, 3,740 recovered allocation failures; includes separate Android publication/storage target |
| ASan/UBSan | 31/31 PASS, including negative probes proving active instrumentation |
| Apple native | 28/28 PASS |
| Swift boundary | 13/13 macOS and 13/13 iOS simulator PASS |
| Android ABI boundaries | arm64-v8a and armeabi-v7a PASS |
| Android app | Debug/release builds and lint PASS; JVM 3/3 debug and 3/3 release, zero ignored |
| Android runtime | 14/14 emulator instrumentation PASS, zero failures/errors/skips; direct asset corpus and owner/Compose editor flows |
| iOS app | Debug/release builds PASS; simulator app 6/6 and UI 2/2 PASS |
| Hosted source/parity/documentation/workflow | 114/114 source tests, zero skips; workflow lint PASS |
| Local Windows Zig fallback | Full 28/28 native PASS in 152.85 seconds; focused editor 42,119 assertions / 3,161 recovered faults PASS |
| Local portable checks | 79/79 PASS, zero skips; actionlint and git diff --check PASS |

Editor coverage includes verified image import, changed pixels, every frozen
descriptor's changed/neutral actions, invalid-update retention, 10,001 update
churn, cancel/switch/interruption, exactly one revision per changed approval,
actual SQLite busy failure and same-proposal retry, authoritative undo/redo and
close/reopen pixel equivalence. Retained Step 6/PRV1 tests include 10,000 updates,
deterministic race checkpoints, 256 interactive races and 128 cancellation races.
Android and iOS execute import/edit/preview/commit/undo/redo/reopen and cancel
paths; the iOS UI flow also exercises backgrounding, landscape, navigation away
and process relaunch without restoring an uncommitted gesture.

The exhaustive allocation test uses verified closed-project portability copies
and four fixed isolated workers partitioning every fault index. MSVC diagnostics
exposed a separate runtime failure in the editor's chained base-string
concatenations: iterator-debug moves can allocate inside noexcept. The service
now assembles envelopes through throwing String append/return paths. The complete
MSVC sweep proves RESOURCE_LIMIT recovery and retry without unintended history.
No fault index, assertion, iterator debugging, test deadline, sanitizer, storage
security check or product ceiling was removed or increased. Android's loading
notification now precedes its value refresh, eliminating an observable new-value/
old-image window; iOS UI automation uses native target-specific scroll-to-visible.

## Scope

A: required generic service/manual screens/ownership/parity/validation, including
the explicitly approved Android publication remediation and its storage proof.
B: Step 8 certification/closure. C: overlays/catalog/polish. No new dependency/tool
semantics, AI/GPU/network/storage-schema scope.
Phase 3 Step 8 has NOT started.

## Accepted ADR 0021 and storage validation

Hosted Android instrumentation discovered a genuine persistence backend conflict:
SELinux denies the frozen POSIX `linkat` asset publication, so real import returns
IO_ERROR (12). The Architect approved [ADR 0021](../adr/0021-android-no-replace-asset-publication.md)
on 2026-10-11. Only Android selects atomic renameat2(RENAME_NOREPLACE), preserving
all immutable asset, no-overwrite, verified-dedup and durability invariants.
Non-Android Linux retains linkat; Darwin/Windows and Schema 3 remain unchanged.
The same branch passes a separate Linux storage/crash regression target, ASan/
UBSan, and actual Android syscall/fault/concurrency instrumentation. Android's
uploaded logcat explicitly reports `ADR0021 Android RENAME_NOREPLACE actual
branch: 71 checks PASS`; its instrumentation XML reports 14 tests and zero
failures/errors/skips. This is direct branch evidence under actual SELinux policy,
not an inference from the UI. The native corpus reports four actual successful
no-replace syscall publications, in addition to production AssetStore ingestion.

| Required publication case | Observed proof |
| --- | --- |
| New asset | Production ingest/verify, read-only mode, one link; real syscall succeeds |
| Duplicate digest / existing correct object | Exact digest/length reverified; existing inode retained |
| Existing corrupt digest | Hash/length rejection; corrupt object remains seven bytes, never replaced |
| Concurrent same/different digests | Barrier-synchronized publishers; equal/distinct identities and verified bytes |
| Staged-file failure | Reader failure returns IO_ERROR; no additional final object |
| After file sync / before publication | Synced/read-only staged fixture, injected EIO; no final object |
| Directory sync failure | Both destination and staging sync failure cases return IO_ERROR |
| Crash/reopen | All nine asset/SQLite abrupt-exit boundaries on the forced Android branch; all ten existing schema migration crash cases retained |
| Unsupported syscall/filesystem | ENOSYS/EOPNOTSUPP return IO_ERROR, staging retained and no final object |
| No insecure fallback | Exact syscall/flag source check, one attempted publication on failure, no overwrite fallback |

All original storage regressions run alongside the independent Android branch.
Published assets remain content-addressed and immutable; pinned capabilities,
NOFOLLOW, file-before-directory sync, verified deduplication and asset-before-
SQLite-reference ordering remain authoritative. No architecture conflict remains.

Local WSL runs, but its DrvFS-backed
workspace does not enforce Unix chmod permissions: the existing nonwritable-root
test and the Android corpus's read-only mode assertion reject locally. Those
checks are retained unchanged; hosted Linux's native filesystem and actual
Android sandbox supply the required authoritative execution. No such local
filesystem failure is counted as PASS.

## Files changed

- `.github/workflows/foundation.yml`
- `.github/workflows/native-shells.yml`
- `ARCHITECTURE.md`
- `CMakeLists.txt`
- `DECISIONS.md`
- `QUALITY_GATES.md`
- `ROADMAP.md`
- `docs/contracts/manual-editor-integration-v1.md`
- `docs/adr/0021-android-no-replace-asset-publication.md` (Accepted)
- `docs/reports/phase-3-step-7.md`
- `packages/core/Package.swift`
- `packages/core/include/pixaura/editor.h`
- `packages/core/src/document.cpp`
- `packages/core/src/document.hpp`
- `packages/core/src/android_asset_publication.hpp`
- `packages/core/src/storage_files.cpp`
- `packages/core/src/editor_api.cpp`
- `packages/core/swift/Sources/ManualEditor.swift`
- `packages/core/swift/Tests/ManualEditorTests.swift`
- `packages/core/tests/editor_test.cpp`
- `packages/core/tests/storage_test.cpp`
- `packages/core/tests/android_publication_boundary.hpp`
- `platforms/android/app/src/androidTest/java/ai/pixaura/app/AssetPublicationTest.kt`
- `platforms/android/app/src/androidTest/java/ai/pixaura/app/ManualEditorTest.kt`
- `platforms/android/app/src/androidTest/java/ai/pixaura/app/ShellTest.kt`
- `platforms/android/app/src/main/java/ai/pixaura/app/IntegerControls.kt`
- `platforms/android/app/src/main/java/ai/pixaura/app/MainActivity.kt`
- `platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorModel.kt`
- `platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorScreen.kt`
- `platforms/android/app/src/test/java/ai/pixaura/app/IntegerControlsTest.kt`
- `platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/CoreProbe.kt`
- `platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/ManualEditor.kt`
- `platforms/android/native/CMakeLists.txt`
- `platforms/android/native/editor_bridge.cpp`
- `platforms/android/native/asset_publication_bridge.cpp`
- `platforms/ios/PixAuraAI.xcodeproj/project.pbxproj`
- `platforms/ios/PixAuraAI/ManualEditorModel.swift`
- `platforms/ios/PixAuraAI/ManualEditorScreen.swift`
- `platforms/ios/PixAuraAI/ShellView.swift`
- `platforms/ios/PixAuraAIUITests/ShellUITests.swift`
- `scripts/check-apple.sh`
- `scripts/codex-gh.ps1` (read-only, repository-bounded CI artifact inspection)
- `scripts/generate-ios-project.mjs`
- `tests/detail-tools.test.mjs`
- `tests/gesture-lifecycle.test.mjs`
- `tests/manual-editor.test.mjs`
- `tests/manual-geometry.test.mjs`
- `tests/presets.test.mjs`
- `tests/tone-color.test.mjs`
