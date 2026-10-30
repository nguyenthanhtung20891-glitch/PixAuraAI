# Plan Inspection & Action Review v1

Phase 8 Step 3 adds read-only presentation only. REVIEW != APPROVAL.
Inspection neither creates a Plan nor changes the frozen Registry, planner,
approval, candidate, publication, Schema 3 or ordinary history authority.

## Native source and projection

`pixaura_review_inspect` accepts the exact current native Auto proposal and the
exact cached canonical Plan v1 (1..8192 bytes, 1..8 actions). Under the existing
editor gate it reuses canonical Plan parsing and Registry binding validation,
checks source/context/owner/request serial and current repository epoch, then
atomically copies a zero-initialized caller-owned description. Failure leaves
the output unchanged. Unknown versions, tools, parameters, noncanonical bytes,
wrong owner, stale source and oversized input fail closed. No subset, repair,
clamping, action sorting, replacement, retry, replan or inference is permitted.

The native `manual::registry()` is the sole source of labels, stable tool IDs,
operation/parameter versions, parameter names/units/bounds/reference values and
geometry category. Registry v1 semantics and bytes remain frozen. Metadata is
bounded product presentation, never provider prose or localized execution input.
Unknown transport fields/versions fail closed on Android and Apple.

The frozen native `auto_presentation` helper reprojects the exact validated Plan
bytes; its complete action array/count must match the current cached proposal.
This detects hidden/reordered/substituted cache actions without a second parser
or tool-semantic implementation. Inspection never silently repairs a mismatch.

## Caller-owned ABI family 1

All sizes are fixed and checked by native and C consumer assertions:

| Record | Size | Bound |
| --- | ---: | --- |
| Parameter | 112 bytes | name[32], unit[64], exact signed raw/min/max/reference |
| Action | 544 bytes | index, tool index, operation/parameter versions, count, geometry/warnings; tool ID[32], label[32], 4 parameters |
| Inspection RIN1 | 4808 bytes | version/size/kind/count/warnings/Registry version; repository epoch; exact proposal; Plan SHA-256 hex[64]; revision[32]; 8 actions |

Names/units/labels are bounded zero-terminated ASCII. Hash/revision fields have
their exact fixed lengths. Unused fields are zero. There are no native pointers,
release obligations, approval tokens, candidate handles, mutable Plan objects or
retained native inspection state. The description is not a second Plan/capability.
The SHA-256 is informational byte identity, not authentication or approval.

`pixaura_review_inspection_current` reconstructs the exact native description and
compares the complete record, including unused bytes. Forged values/labels/order,
hidden actions, identity confusion and stale/cross-document reuse reject.
`pixaura_review_inspection_preview` additionally checks the existing current
preview pair and exact originating proposal/Plan association. Preview A cannot
be installed with inspection B. Neither function grants Apply authority.

## Exact presentation and information architecture

Platforms decode bounded typed records; they do not parse Plan JSON, infer tool
bounds, convert labels into identities, repair values or duplicate Registry math.
Every action appears exactly once, in canonical order, labeled Action X of N.
The authoritative raw integer and native Registry unit are always visible, plus
parameter name, min/max/reference, stable tool ID and operation/parameter versions.
There is no lossy decimal conversion. Friendly localized labels may supplement
these fields but must never replace exact values or be parsed for execution.

The surface distinguishes context (mode/status, provider as informational only,
separate planner), existing Before/After comparison, ordered actions/count,
bounded feedback, and Preview/Reject/explicit Apply decisions. Static warnings
derive solely from multiple-action/order and geometry flags. Crop displays all
four exact millionth coordinates; rotation displays exact clockwise quarter turns.
Existing preview supplies authoritative resulting dimensions when available.
Geometry comparisons retain Step 2's safe fit/toggle fallback.

Action/list/count/detail/geometry/stale/unsupported semantic identifiers expose
position, label, exact value/unit and warning without color or visual order alone.
There are no action sliders, value editors, remove/reorder/replace/restore controls.
Step 4 remains NOT STARTED. Existing viewport sliders change presentation only.

## Freshness, lifecycle, Apply and failure

Inspection is part of the same immutable ephemeral AutoReview, exact Plan and
existing owner/request epoch. Source replacement, Manual edit, undo/redo/branch,
mode abandonment, document/session change, relevant repository change or Apply
invalidates existing native eligibility. A new explicit request is required;
there is no silent refresh or transfer. Stale presentation is labeled or discarded,
and Apply remains unavailable under existing native review semantics.

Extraction/current checks are synchronous bounded metadata calls under existing
owner gates, with no worker/queue/cache added. Preview loading may coexist with
inspection; installation rechecks the native pair/inspection association under
the platform lifecycle gate. Late callbacks remain fenced by existing epochs.
Stop -> join ALL calls -> destroy remains unchanged. No inspection callback owns
UI lifetime. No image allocation/copy is needed for metadata.

Malformed/unsupported/failed extraction produces no actionable partial review and
no Apply control. Preview resource failure still permits current Plan inspection.
Viewing, scrolling, expanding, warning display and previewing never approve.
Only explicit HUMAN Apply enters unchanged exact revalidation/single-use approval,
private candidate/execution and at-most-one ordinary revision/publication fencing.
Inspection never strengthens approval eligibility or permits reuse of old approval.

## Resources, privacy, release and threats

One description per review is bounded to 4808 native transport bytes plus bounded
platform value objects (8 actions, 4 parameters each). Native extraction retains
zero inspection bytes and zero image bytes; existing <=8192-byte Plan/Registry
validation has bounded temporary allocations. Allocation failure is atomic and
Manual remains independent. Existing Phase 4/5 and intelligence resource contracts
are unchanged; no new memory ceiling or RSS-derived lease is introduced.

Everything is local and ephemeral. No Schema 3/disk restoration, image upload,
cloud metadata/localization authority, telemetry, model/NLP or network is added.
Certification-only kind-3 fixture transport has no approval/execution capability
and is excluded from release. Native/APK/Apple scanners reject its markers, with
negative packaged-content fixtures retained.

| Threat | Protection |
| --- | --- |
| Label becomes execution identity | typed native identity; strings only displayed |
| Forged/reordered/hidden action or value | complete native reconstruction and byte comparison |
| Stale/cross-owner inspection applies | exact binding + unchanged native Apply admission |
| Preview A / inspection B | native association check at actual display install |
| Formatting changes value | exact signed raw integer and native unit, no conversion |
| Inspection mutates document/history | projection contains no mutation/approval/candidate authority |
| Teardown/late callback | existing owner/epoch gates and stop/join/destroy |
| Resource/release/privacy exposure | bounded records, no image/cache/worker, final package scans |

Provider reference.assistance.v1 remains ADMITTED; planner reference.deterministic.v1.
No learned production model required/admitted. CPU VALIDATED, Vulkan/Metal UNKNOWN;
DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED. Hosted results are software
evidence only. This contract does not authorize editable review or Phase 8 Step 4.
