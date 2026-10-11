# Phase 3 certification matrix v1

Authority: authorized Phase 3 Step 8, baseline
`c0a0020d41b22b55fad494fab50d72b5903f7c22`. This consolidates accepted contracts;
it introduces no tool, parameter, numerical, lifecycle or storage semantics.
All rows are certified PASS at implementation SHA `e73605501019afb1aca2a12c521539b68176618e`. Execution status and exact-SHA evidence live in the [Step 8 report](../reports/phase-3-step-8.md).

## Authoritative tool and platform matrix

All operation types equal the stable tool ID below. Operation/parameter versions
are **1/1**. Parameters are integers with step **1**. Android Compose and iOS
SwiftUI obtain these descriptors from the shared registry through their existing
boundaries; neither screen contains a separate ID/range/default registry.

| Tool / shared ID | Parameter | Unit | Inclusive range; neutral/default | Core / Compose / SwiftUI |
| --- | --- | --- | --- | --- |
| Crop / pixaura.crop | height_ppm, width_ppm | millionths_current_extent | 1..1000000; 1000000 | PASS / PASS / PASS |
| Crop / pixaura.crop | x_ppm, y_ppm | millionths_current_extent | 0..999999; 0 | PASS / PASS / PASS |
| Orthogonal rotation / pixaura.rotate | quarter_turns | clockwise_quarter_turns | 0..3; 0 | PASS / PASS / PASS |
| Exposure / pixaura.exposure | milli_ev | thousandths_exposure_stop | -5000..5000; 0 | PASS / PASS / PASS |
| Brightness / pixaura.brightness | milli_linear | thousandths_linear_white | -1000..1000; 0 | PASS / PASS / PASS |
| Contrast / pixaura.contrast | milli_stops | thousandths_log2_contrast_slope | -2000..2000; 0 | PASS / PASS / PASS |
| Highlights / pixaura.highlights | milli_ev | thousandths_highlight_exposure_stop | -2000..2000; 0 | PASS / PASS / PASS |
| Shadows / pixaura.shadows | milli_ev | thousandths_shadow_exposure_stop | -2000..2000; 0 | PASS / PASS / PASS |
| Saturation / pixaura.saturation | milli_ratio | thousandths_chroma_multiplier | 0..2000; 1000 | PASS / PASS / PASS |
| Temperature / pixaura.temperature | kelvin | daylight_white_kelvin | 4000..25000; 6504 | PASS / PASS / PASS |
| Sharpen / pixaura.sharpen | milli_amount | thousandths_fixed_kernel_unsharp | 0..1000; 0 | PASS / PASS / PASS |
| Blur / pixaura.blur | milli_strength | thousandths_fixed_kernel_blend | 0..1000; 0 | PASS / PASS / PASS |

Crop additionally requires x+width and y+height <=1000000. Invalid coupled values
reject without clamping; outward floor/ceil rasterization remains shared-core
authority. Tool ordering is the registry's deterministic ASCII ordering within
the existing category presentation. Rotation remains clockwise quarter turns.
EV/percent/K labels are presentation; persisted values remain exact integers.

Every row has the same certification obligations: invalid values reject while
preserving the last accepted pending state; neutral Apply adds zero revisions;
changed Apply adds exactly one; Cancel adds zero; preview does not change history;
Undo/Redo uses the core history; reopening retains IDs, stacks, redo and committed
pixels. Native `editor_test`, Android `ManualEditorTest` and Swift
`ManualEditorTests` explicitly exercise every descriptor through changed edit,
preview, commit, Undo, Redo and reopen. Both platform certification tests also
exercise every parameter's out-of-range rejection, neutral Apply and Cancel.
The existing tone/detail/geometry suites certify coupled validation and numerical
boundaries independently of these application-service tests.

Filters/Presets is a visible empty-state category on both platforms. The shipping
catalog contains **zero** recipes. Canonical preset foundation remains inspectable
integer-only tone/detail operations, versions 1/1, stable recipe IDs and array
order, <=16384 bytes/16 operations per recipe, <=16 recipes/262144 catalog bytes,
<=1024 binding bytes. Geometry, opaque executable fields and malformed/unknown
versions reject. One changed batch is one revision; no live gesture interleaving.

## Certification traceability

| Obligation | Independent / runtime evidence source |
| --- | --- |
| IDs, versions, units, ranges/defaults, malformed input | manual_test, manual_geometry_test, tone_test, detail_test and existing C/JNI/Swift boundaries |
| Geometry references | geometry_execution_test: literal golden permutations, independent ppm floor/ceil, crop/rotation ordering, borders, unusual alpha/float bits |
| Tone references | scripts/tone-reference.py --check; Decimal independent oracle and checked-in tone_fixtures.hpp |
| Detail references | scripts/detail-reference.py --check; rational independent oracle and checked-in detail_fixtures.hpp |
| BEGIN/UPDATE/COMMIT/CANCEL, one revision, bounded 1000/10000 churn | gesture_lifecycle_test, editor_test; Android and Swift owner tests |
| Fence races, late completion, failed newest preview, detached copies | preview_test, gesture_lifecycle_test, JNI/Swift owner tests; native barriers rather than timing-only assumptions |
| Tool switch, background/navigation/disposal, retry and faults | gesture_lifecycle_test, editor_test, Compose/controller and SwiftUI/model/app tests |
| Every tool changed/neutral/invalid/cancel/redo/reopen | Android certifyEveryDescriptorValidationCancelCommitUndoRedoAndReopen; Swift testCertificationEveryDescriptorValidationCancelCommitUndoRedoAndReopen; native editor_test |
| Schema 1/2/3, explicit migrations, no auto migration, IDs/stacks/redo/hash | storage_test, independent storage_android_test, C/JNI/Swift persistence tests |
| Android-only no-replace publication under enforced SELinux | AssetPublicationTest direct production branch corpus and emulator selinux-policy.txt |
| Dedup, tamper, concurrency, durability/crash, unsupported syscall | android_publication_boundary.hpp and full storage_android_test crash corpus; all existing storage regressions |
| Preset canonicalization, bounds, malformed input, one revision | preset_test and C/JNI/Swift preset boundary tests |
| Allocation/admission/scratch/context/handle/operation bounds | unchanged native fault suites, ASan/UBSan and negative sanitizer probes |
| Accessibility/basic responsive layout | Compose essential UI flow; SwiftUI app/UI portrait/landscape flow and source semantics checks |
| No UI-local business rules or forbidden scope | tests/manual-editor.test.mjs, foundation/source checks and reviewed diff |

## Preserved ownership and scope

IDLE -> ACTIVE -> COMMITTING -> COMPLETED, or CANCELLED/INVALIDATED, remains the
Step 6 model. UPDATE coalesces one pending operation; PRV1 generation ownership
alone decides publication, including late cancelled work. Failed newest preview
keeps the prior display copy. Apply yields the existing detached immutable proposal
and normal approval/persistence flow; allocation/admission failures are retryable,
storage approval retry reuses its proposal, stale state invalidates. Cancel is
idempotent, revokes publication and releases transient ownership. Tool switch
cancels first. Background/navigation/disposal cancel; stop/join precedes destroy.
Undo/Redo and preset application reject while a manual gesture is active.

Frozen ceilings remain unchanged: one active gesture/context, one <=1024-byte
pending operation, one preview ownership ticket, <=256 captured/ordered operations,
combined 64 document/gesture handles; existing raster/context and platform render
admission limits. Raster <=128 MiB/8388608 pixels, context <=256 MiB,
manifest <=8 MiB, command <=64 KiB, stored operations/revisions <=4096 each,
detail three-row scratch <=786432 bytes and shared scratch <=1 MiB remain frozen.
Registry <=16 descriptors/8 parameters each, <=64-byte text and <=32 KiB JSON;
platform detached copy <=4 MiB and existing two-render admission remain unchanged.
There is no durable gesture state or new schema. Historical
Schema 1/2 remain immutable; Schema 3 is current, explicit 1->2 then 2->3 only.

ADR 0021 remains Accepted and Android-only: renameat2(RENAME_NOREPLACE) between
pinned staging/final directory capabilities; synced read-only staging file,
destination/staging directory sync, exact digest/length verification including
EEXIST, immutable asset before SQLite reference. Unsupported/error rejects IO_ERROR
without overwrite/copy/link fallback. Production Linux linkat, Darwin exclusive
rename and Windows non-replacing move remain unchanged. SELinux is enforced.

DH-APPLE-METAL-01 remains DEFERRED, mandatory before Phase 11 hardening acceptance,
Phase 12 Beta readiness completion, production release or physical Apple GPU claims.
This certification does not claim physical Metal execution. No new tools, presets,
UI features, AI, GPU, network/cloud, telemetry, schema, Step 9 or Phase 4 work.
