# Manual tool contract v1

Current Phase 3 Step 4 extension: the registry contains eleven tools, adding only
Sharpen/Blur under the [detail numerical contract](manual-detail-v1.md) and
explicit schema-3 persistence. All bounds and shared gesture APIs remain unchanged.

At Phase 3 Step 3 the compiled registry contained nine tools,
including the six newly frozen tone/color tuples. See the authoritative
[tone/color numerical contract](manual-tone-color-v1.md) for parameters,
formulas and schema-2 persistence. Generic `pixaura_manual_*` calls reuse the
existing controller/token/PRV1 path; `geometry_begin` remains geometry only.
The Step 1 inventory/status below is historical, not current tool status.
Exposure/crop/rotation parameters and registry/gesture bounds remain unchanged.

Status: Phase 3 Step 1 contract and executable reference. This refines the accepted
Phase 2 model; it adds no editing kernel, shipping control, AI behavior or final UI.
At Step 1 delivery, the Product Owner authorized Step 1 only and no subsequent
step sequence was frozen. The later Product Owner / Architect decision freezes
[Phase 3 Steps 1–8](../../ROADMAP.md#frozen-phase-3-numbered-roadmap--manual-tools),
accepts Step 1 as COMPLETE / FULL PASS and leaves Steps 2–8 NOT STARTED. Approved
future tool scopes do not freeze their missing parameter tuples or recipes;
this contract's executable registry and parameter semantics remain unchanged.

## Authority and integration

Reuse [document v1](image-document-v1.md), [CPU evaluation](cpu-evaluation-v1.md),
[geometry](geometry-v1.md), [storage](project-storage-v1.md) and
[preview lifecycle](interactive-preview-lifecycle-v1.md), including ADR 0008/0009.
The original managed source and verified digest are immutable. Each committed
revision names its full ordered operation stack; history retains old operations
and branches. Planning and previews never mutate a document. A manual gesture
produces a detached revision proposal through the existing transition engine;
the application owner still validates, approves, persists and publishes it.
The reference controller grants no durable commit or display publication authority.
CPU evaluation remains authoritative; GPU eligibility and numerical recipes are
unchanged. No architecture incompatibility was found; no superseding ADR is needed.

## Taxonomy and product inventory

| Category ID | Frozen executable tools | PROPOSED product MVP tools |
| --- | --- | --- |
| geometry | Crop, Rotate (orthogonal only) | Resize |
| tone_color | Exposure | Brightness, Contrast, Highlights, Shadows, Saturation, Temperature |
| detail | None | Sharpen, Blur |
| filters_presets | None | Basic filters/presets; names and recipes unfrozen |

This is the Step 1 delivery inventory from PRODUCT_SPEC P02-P04. PROPOSED here means no registered
operation/version, parameter range, default, recipe or shipping claim. Reserved
operation identifiers do not freeze behavior. Whites, blacks, tint, vibrance and
free-angle rotation are backlog/non-MVP proposals, not additional shipping tools.
Presets must eventually resolve to bounded, explicit ordered versioned operations;
there is no opaque executable preset or runtime script in this foundation.

## Canonical descriptor

`pixaura_manual_registry_json(1, ...)` exposes descriptor schema version 1 as UTF-8
JSON. Tool IDs and parameter names are strictly ascending ASCII, keys have the
fixed canonical order emitted by the native implementation, and one LF terminates
the object; no NUL is serialized. Stable fields are `tool_id`, `type`,
`operation_version`, `parameter_version`, `category`, semantic `name`,
`parameters`, `invalid`, `serialization`, `replay`, `preview`, `commit`, `capability`.
Each parameter has `name`, integer `type`, `unit`, `minimum`, `maximum`, `default`,
and `step`. Tool ID currently equals the operation type. All three tuples are
operation version 1 / parameter version 1. Descriptor schema and C API versions
are independently checked. Changes to behavior require reviewed versioning, never
an unversioned platform override.

Common policies: invalid=`reject`, serialization=`schema1_canonical_integer_json`,
replay=`ordered_phase2_versions`, preview=`detached_latest_generation`,
commit=`one_immutable_revision_per_changed_gesture`, capability=`offline_cpu`.
The registry is static native storage with three entries, at most 16 descriptors
and 8 parameters per descriptor, bounded 64-byte text fields, 32 KiB serialized
registry output and integer step 1.
Controlled table validation rejects duplicate IDs/tuples, unsorted IDs/parameters,
unknown categories, invalid defaults/ranges/steps and excess counts. It never
installs a table. No runtime registration, discovery, reflection, network, remote
configuration, arbitrary script or dynamic native module exists. Unknown tool or
operation/parameter version fails; no fallback to another version occurs.

## Frozen parameters

| Tool / parameter | Unit | Inclusive range | Neutral/default | Step |
| --- | --- | --- | --- | --- |
| pixaura.exposure / milli_ev | thousandths of an exposure stop | -5000..5000 | 0 | 1 |
| pixaura.rotate / quarter_turns | clockwise 90-degree turns | 0..3 | 0 | 1 |
| pixaura.crop / x_ppm, y_ppm | millionths of current input extent | 0..999999 | 0 | 1 |
| pixaura.crop / width_ppm, height_ppm | millionths of current input extent | 1..1000000 | 1000000 | 1 |

Crop additionally requires x+width and y+height <=1000000. Crop follows existing
horizontal x/width and vertical y/height dimensions, respectively, with
outward floor/ceil rasterization on the current operation input; rotation follows
the existing exact orthogonal mappings; exposure follows the existing CPU recipe.
All values are exact integer JSON tokens. Floating notation, exponent notation,
negative zero, strings, booleans, null, duplicate/extra/missing or escaped keys,
overflow and out-of-range values reject in the shared Phase 2 parser. No clamp,
locale-dependent parsing or platform rounding is allowed. A future UI must map
its display units explicitly to these integers before sending a request.

`pixaura_manual_canonical_operation` accepts exactly one operation in the existing
`{"operations":[...]}` evaluation envelope, at most 64 KiB. Operation ID is the
existing lowercase 32-hex identity. Canonical key order is id, operation_version,
parameter_version, parameters, type; parameter names are ascending. It emits one
LF. C buffers use the two-call size protocol: a short/query buffer returns
BUFFER_TOO_SMALL and required size; all other failures preserve outputs. Caller
buffers must be truthful and nonoverlapping; input is never retained; exceptions
never cross C. Serialization is validation, not approval or execution.

## Gesture lifecycle and reference behavior

BEGIN captures an authoritative immutable base, project/document, source digest,
session identity/generation, current revision, fresh gesture identity and one known
tool. It optionally selects an existing operation of that tool in the current
stack; otherwise it appends. The application owns unique gesture/operation/revision
IDs and serializes access; the reference is not a scheduler or concurrent owner.

UPDATE validates one fresh operation and replaces a single pending value. Accepted
updates increment a uint64 sequence (overflow rejects); failed updates leave the
last valid value/sequence intact. There is no update list or history allocation
per slider movement. A preview request contains the projected full ordered stack
and never changes the base. Replacing preserves position; appending follows all
existing operations. A neutral append is omitted; a neutral replacement removes
that operation from the proposed stack while retaining its historical record.
Existing stored neutral operations are not rewritten or deleted.

COMMIT is accepted once. No updates, a neutral append, or an identical replacement
returns the same base and creates no revision. A change creates exactly one new
immutable revision proposal and at most one new operation. A neutral replacement
creates one revision with no new operation. The caller supplies a fresh revision
ID; duplicate IDs reject even on no-op. Existing history limits still apply.
After success, duplicate commit/update/preview fails as cancelled. CANCEL closes
the reference and drops pending parameters without any history mutation.

Eligibility requires exact gesture ID, latest nonzero update sequence and live
base/session/source/revision binding. Stale commit cancels and rejects STALE_BASE;
failed validation, admission, allocation or transition produces no document and
preserves pending state for explicit retry/cancel. A preview failure preserves
the last displayed preview under the existing owner. Eligibility is necessary but
not a publication lock: render owners must additionally use PRV1 generation and
the native/platform publication fence while holding the established owner lock.

Tool switch, source/document/session change, background interruption and disposal
must CANCEL and revoke the owner's outstanding PRV1 request before starting any
new gesture. Interruption never implicitly commits. Resumption begins a fresh
gesture; a lost pointer-up cannot create a durable edit. These are contract
requirements for future platform controls, not a final UI implemented here.

## Replay, bounds and acceptance

The same verified source plus ordered operations, exact integer parameters and
accepted versions reconstructs the same logical edit state through existing
replay/evaluation. IDs and full stack ordering are retained; sorting operations or
combining noncommuting operations is forbidden. Canonical round trips are stable.
This does not introduce a new floating-point cross-GPU certification claim.

Existing ceilings remain: 256 operations per replay stack, 4096 stored operations
and revisions, 8 MiB manifest, 64 KiB command, 64 context handles, 128 MiB raster,
8,388,608 pixels and 256 MiB context residency. A gesture keeps one immutable base,
one bounded replay stack and one pending operation. Application owners allow one
active gesture per editing session; reference instances are controlled ownership,
not an unbounded global service. No original bytes or private phone data are exposed.

Acceptance tests exercise the actual shared parser, canonical C boundary, compiled
registry, immutable transition and replay: parameter edges/invalid notation,
future versions, duplicate registry IDs, neutral edits, 1000 coalesced updates,
stale/cancelled results, replacement ordering and canonical round trips. Android
JNI and Swift consume the same descriptor metadata; neither redefines semantics.
Required work for this acceptance is category A. Future MVP controls/parameter
freezes belong to the frozen later Phase 3 steps (category B), whose implementation
requires separate authorization. Resize remains PROPOSED/category C and requires
separate product approval before inclusion in any frozen step.
Non-MVP reservations and optional extensibility are category C backlog.
