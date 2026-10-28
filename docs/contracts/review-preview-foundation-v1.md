# Review Preview Foundation v1

Phase 8 Step 2 implementation contract, additive to Full Review Experience v1.
REVIEW != APPROVAL. PREVIEW != EXECUTION. PREVIEW != REVISION.
No existing Plan, Registry, approval, candidate, history, Schema 3 or resource
semantics change. No editable review or Step 3 inspection work is authorized.

## Native non-publication composition

The existing editor service owns one `ReviewPreviewState`. It holds correlation
records and disposable images, not an execution candidate or repository publisher.
Begin requires the current trusted Auto proposal and byte-for-byte identical cached
canonical Plan (1-8 actions, at most 8192 bytes), then invokes the existing Plan
parser and Registry binding. Provider prose, approval tokens and UI action lists
are not inputs. The immutable native snapshot is retained for computation only.

BEFORE is the replay of the exact bound committed revision, including earlier
Manual edits. AFTER PREVIEW appends the exact validated ordered Plan operations
to that replay. Existing geometry planning, evaluation and preview resampling
implement both images. No editing math is duplicated. The disposable operation IDs
are collision-free bookkeeping; tool versions, values and action order stay exact.
No approval issuance, candidate admission, document transaction, repository publish
or revision/history mutation is called by this path.

The existing float working representation and sRGB RGBA display conversion apply.
Fit is aspect-preserving, with no upscale, through existing native preview rules.
Platforms request fit bounds 512 x 512; native accepts positive bounds at most
1024 and restricts them to the existing dynamic-policy preview edge. These bind
existing preview/display limits; they do not authorize full-resolution displays.
Native evaluation still uses existing bounded source/working limits. Display
resampling may differ from full-resolution final output; no universal full-resolution
pixel identity is claimed. Numerical fixtures compare identical fit resolutions.

## Bounded bridge and ownership

`pixaura/review_preview.h` is preview-only API family 1. The 56-byte ticket has a
distinct RVP1 kind, context identity and nonwrapping request serial. The 176-byte
pair adds exact native image metadata, logical dimensions, geometry flag and
resource diagnostics. Neither record exposes a native image pointer, approval,
document handle or candidate. Records cannot be interchanged with those families.

Begin publishes one pending ticket. Render consumes it once, with at most one
active native call; competitors return BUSY (8), no queue/retry. Exact owner,
proposal, source revision, generation/serial, Plan bytes, repository epoch and
policy epoch are checked before work and again before result publication.
Supersession invalidates the old record; active work must drain before admission
of replacement work. Same bytes in a new request do not revive an old ticket.

Copy accepts only side 0/1, checked byte offsets and bounded counts (at most 4 MiB),
matching actual metadata and live private images. Copy is not installation authority.
Platforms copy to immutable display resources, release both native images, then
must call current under their owner/lifecycle gate at actual installation. The
small exact eligibility record remains after image release. Wrong, stale, forged
or cross-owner records fail closed. Release cannot free another request's images.

Kotlin/Swift own only presentation, bounded transport and detached images. JNI
copies rows and packs RGBA to Android ARGB; Swift preserves RGBA/sRGB via a
CGDataProvider. No platform tool math, approval issuer or publication shortcut.

## Lifetime, invalidation and cancellation

Stop -> reject/cancel new work -> join ALL owned calls -> release -> destroy.
Native render, both copies, current checks and image release belong to the existing
platform in-flight-call join. No detached worker or new queue. Raster work runs
off UI and outside the editor/platform lifecycle gates. The existing raster
registry mutex serializes allocation admission and rendering; native cancellation
uses its independent atomic token/fence so it can interrupt work while that mutex
is held. No callback joins itself. Fixed request tokens and platform attempt/epoch
guards prevent late cancellation from targeting newer work.

Manual edits/ordinary preview, undo/redo (including ABA), successful Apply, source
or editor replacement, mode abandonment, stop, repository epoch changes, policy
changes and Plan replacement invalidate eligibility. There is no silent rebase or
refresh. An old computation can finish but cannot install into a newer session.
Back/disposal discards presentation and cancels preview. Native durable truth wins
if an independently approved execution is active; no automatic undo is introduced.

Preview cancellation signals only its private raster token, never Agent execution
cancellation. It releases disposable images and leaves a still-current Plan
inspectable; explicit Preview may be requested again after drain. PREVIEW_LOADING
disables Apply; preview success never triggers it. Explicit HUMAN Apply continues
through the unchanged Phase 7 approval/execution boundary after preview disposal.

## Existing Budget Admission v1

The existing PREVIEW work class and dynamic policy admit the whole pair before
heavy allocation. The existing raster registry lock covers occupancy observation,
preflight and both renders: Budget Admission is a pure decision, not an invented
allocation lease. Existing retained source/native handles are counted once.
One full working image at a time plus maximum existing evaluation/geometry scratch
is reserved. BEFORE's working image is released before AFTER evaluation.

Temporary display accounting reserves eight bounded RGBA fit capacities: native
pair (2), platform pair (2), superseded platform pair pending UI release (2), current
Manual display (1), and transfer buffer (1). The five new-image capacities use the
current admitted request edge. The three old-image capacities use the existing
editor display API maximum 1024, because they can predate a policy downgrade.
This conservative old-image allowance is not reduced merely because new renders
now fit a smaller edge. PREVIEW v1 already includes one display;
that automatic component is subtracted from explicit temporary bytes to avoid
double counting. Conservative square-fit reservation includes aspect-ratio slack.
Existing fixed Budget v1 control overhead covers bounded Plan/snapshot/metadata
orchestration. Existing maximum live handles and decode/context caps still apply.
No Phase 4/5 ceiling or class changes; no RSS-derived ceiling. Diagnostic admission
peak includes existing retained resources and headroom, not just the native pair.

Platform display deallocation follows Bitmap/ARC/CGImage ownership and normal
platform memory pressure; it is not claimed to be synchronous heap reclamation.
There is no accumulating preview cache. Model state and native preview pixels are
released on replacement/disposal. At most one current pair plus one draining
presentation generation is reserved; requests never queue replacement rasters.
Existing platform allocation/error handling remains in force. Allocation failure
returns resource failure; Manual/document state is never evicted to admit preview.
Process RSS is diagnostic evidence, never an allocation lease. Intelligence Runtime
Resource Admission v1 is unrelated and unchanged.

## Presentation and statuses

PLAN_REVIEW -> PREVIEW_LOADING -> PREVIEW_READY / PREVIEW_FAILED / STALE /
CANCELLED are presentation states mapped from native status. Invalid/version/bounds,
noncanonical Plan, unknown tool/version, invalid parameter, stale source, BUSY,
allocation, cancelled/stopped and internal failures fail closed (including unknown
nonzero statuses). Plan inspection remains available after optional preview failure.
No automatic retry, replan, reapproval or fabricated success.

Both platforms expose explicit Preview, Before, After Preview (not committed),
loading, cancel, failed/stale, toggle, split, reset and distinct Apply semantics.
Shared viewport state is display-only: zoom 1-4, normalized pan -1..1, divider 0..1.
Reset fits both; all controls have semantic accessibility identifiers. Non-geometric
plans permit synchronized split/toggle/zoom/pan. Any crop/rotation conservatively
uses aspect-preserving fit/toggle with correct logical dimensions and disables
split/zoom/pan because spatial registration is not established. No stretched images
or false correspondence. Full spatial geometry comparison is not claimed.

No upload, network endpoint, image telemetry, disk preview cache, Schema 3 field,
restoration of ephemeral authority, model or learned provider is added. Test hooks
are certification/debug-only and forbidden by native/APK/Apple release scans with
negative fixtures. CPU/software evidence does not promote Vulkan/Metal or satisfy
DH-APPLE-METAL-01. Step 3 and editable review remain NOT STARTED.
