# Phase 3 Step 7: Cross-platform Manual Editor Integration

Status: AUTHORIZED / IN PROGRESS. Baseline
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

Local Windows Zig fallback: all 28 native tests PASS (304.57 seconds). The editor
service passes 42,040 assertions and recovers 3,162 injected allocation failures;
the retained Step 6 suite passes 196,726 assertions. Preview regression includes
10,000 updates, 256 interactive races and 128 cancellation races. Editor coverage
includes verified import, changed previews, all eleven tools, invalid-update
retention, neutral/cancel/switch/interruption, one-revision approval, actual SQLite
busy failure/retry, undo/redo and close/reopen pixel equivalence.

Portable source/documentation checks: 78/78 PASS, no skips. Workflow actionlint
and git diff --check PASS. Android debug/release builds, both lints and
instrumentation APK build PASS; debug JVM 3/3 and release JVM 3/3 PASS, no skips.
No local Android device is connected. Apple execution, Android instrumentation,
Linux/sanitizers and exact-SHA hosted gates remain pending; no unobserved gate
counts as PASS. Final hosted evidence and changed-file list follow after execution.

The initial allocation test timed out while reprovisioning schema at every fault
index. Verified closed-project portability copies now isolate fault injection
without repeated schema provisioning; all allocation indices remain exercised.
The frozen native limits, tests, sanitizers and security controls are unchanged.

## Scope

A: required generic service/manual screens/ownership/parity/validation.
B: Step 8 certification/closure. C: overlays/catalog/polish. No new dependency/tool
semantics, AI/GPU/network/storage-schema scope.
Phase 3 Step 8 has NOT started.

## Architecture decision pending

Hosted Android instrumentation discovered a genuine persistence backend conflict:
SELinux denies the frozen POSIX `linkat` asset publication, so real import returns
IO_ERROR (12). [Proposed ADR 0021](../adr/0021-android-no-replace-asset-publication.md)
contains the exact job evidence, narrowly scoped Android-only no-replace rename
proposal, invariant-preserving validation and alternative. Storage implementation
is stopped at this decision; no backend change or claimed Android PASS is made.
The iOS app compiler findings are separately remediated without changing semantics.

## Files changed

- `.github/workflows/foundation.yml`
- `.github/workflows/native-shells.yml`
- `ARCHITECTURE.md`
- `CMakeLists.txt`
- `DECISIONS.md`
- `QUALITY_GATES.md`
- `ROADMAP.md`
- `docs/contracts/manual-editor-integration-v1.md`
- `docs/adr/0021-android-no-replace-asset-publication.md` (PROPOSED only)
- `docs/reports/phase-3-step-7.md`
- `packages/core/Package.swift`
- `packages/core/include/pixaura/editor.h`
- `packages/core/src/document.cpp`
- `packages/core/src/document.hpp`
- `packages/core/src/editor_api.cpp`
- `packages/core/swift/Sources/ManualEditor.swift`
- `packages/core/swift/Tests/ManualEditorTests.swift`
- `packages/core/tests/editor_test.cpp`
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
