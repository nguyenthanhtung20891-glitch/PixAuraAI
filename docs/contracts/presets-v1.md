# Preset/filter foundation v1

Authority: Phase 3 Step 5. Filter means a user-facing named appearance recipe;
preset means its canonical representation. Both expand ordinary accepted tools,
not a new pixel engine. No shipping names/recipes are approved. Built-in catalog
is empty; reference.* recipes exist only in tests. No final UI/import/sharing/AI.

## Canonical recipe

Exactly: name, operations, preset_id, recipe_version, schema_version (ASCII sorted
keys). Both versions are exactly 1; unknown versions reject UNSUPPORTED_SCHEMA.
Operations are full existing operation records: id, operation_version,
parameter_version, parameters, type. Preserve array order, integer-only shared
parameters/validation and all accepted numerical semantics. IDs here are unique
recipe-local 32-lowercase-hex identities, not durable history IDs. Metadata name
is nonempty printable ASCII, 1..64 bytes; a semantic fallback, never identity.
No description/category/script/plugin/blob/URL or unknown fields are admitted.

preset_id: 1..64 lowercase ASCII bytes, dot-separated nonempty segments of
[a-z0-9_], first segment pixaura/local/reference. pixaura is reserved for a future
product-approved catalog; local identifies owner-controlled in-memory recipes,
not an external import feature; reference is test-only by convention. Names may
change without identity changes; numerical edits require a new recipe_version.
Catalog IDs are unique regardless of recipe_version. No dynamic namespaces.

Bounds: recipe UTF-8 <=16384 bytes; 1..16 operations; each operation <=1024 input
bytes; nesting <=4 (root/object/array containers counted; scalar leaves do not
increase container depth); all existing parser string/number/UTF-8 protections.
Catalog <=16 recipes and <=262144 canonical bytes, controlled construction then
read-only (assignment disabled), sorted by ASCII preset_id; no runtime discovery/registration/network. Bindings <=1024 bytes,
exact object {"operation_ids":[ID,...]}, exactly one fresh ID for every operation.

Allow only accepted tone_color/detail tuples (exposure, brightness, contrast,
highlights, shadows, saturation, temperature, blur, sharpen), currently /1/1.
Crop/rotate and unknown/unapproved tools reject UNSUPPORTED_OPERATION. Empty,
duplicate IDs/fields, malformed/truncated input reject INVALID_PROJECT; invalid
parameters reject INVALID_PARAMETERS; byte/count/depth ceilings RESOURCE_LIMIT.
Shared parser rejects fractional/exponent/negative-zero notation, invalid UTF-8,
BOM, escape-equivalent duplicate fields, trailing input and executable fields.
No partial validation or execution. Canonical JSON ASCII-sorted object keys,
array order unchanged, shortest integers, standard escapes, exactly one LF,
no NUL/BOM/locale/platform-newline dependence. Equivalent decoded values/IDs/order
produce identical bytes. Whitespace/key ordering may be normalized.

## Application and history

Owner provides base and live snapshots, fresh operation IDs in recipe order,
and one fresh revision ID. No randomness/clocks/ID hashing inside core. Rebind
recipe-local operation IDs to those caller-provided IDs; preserve every tuple,
parameter and order. Append to the current explicit stack without deduplication,
folding/reordering. Check full project/document/source hash/current/session/
generation binding; stale fails STALE_BASE even after undo/redo ABA. Validate
ALL input and bounds/fresh IDs before proposal; no partial output on failure.

All-neutral recipe returns unchanged borrowed live state, no history growth.
Changed recipes retain ALL constituent operations, including interleaved neutral
ones, as one detached immutable revision proposal (manual actor, null plan).
Approval/persistence/publication remain application-owner responsibilities. Pixel
failure or owner cancellation means discard proposal, preserving approved state.
Preview uses existing evaluation/PRV1 cancellation/publication fences, no scheduler
or preview-history mutation. Synchronous bounded metadata preparation adds no
background task/cancellation token; owner cancels before publication or discards.

A->B, repeated A, manual->preset and preset->manual append distinct fresh IDs in
requested order. Undo removes the whole batch as one action; redo restores it.
All operations remain normal inspectable/editable records for existing replacement
semantics. Same source/ordered operations/parameters/versions replay identically
within accepted CPU reference contracts, same-platform restart bit identity.

## Storage and security

No durable preset-origin metadata is required for replay; resulting ordinary
operations/revisions preserve full semantics and inspectability. Schema 3 already
represents every permitted tuple. No Schema 4 or migration change. Fresh catalogs
remain v1; existing owner-requested 1->2->3 hops are still required for newer tools.
All historical SQL/fingerprints remain immutable, no automatic migration. Store
only approved results through existing atomic SQLite API, not detached recipes.
No public recipe import/export, remote config, scripts, plugins, marketplace,
cloud or AI behavior. No global ceiling changes (256-stack,4096-history/operation,
64 handles and existing raster/context limits); over-limit application rejects
without pruning. Checked C boundaries retain exception/output/ownership rules.

Future planners may reference this same canonical recipe and ordinary operation representation. They gain no execution/approval authority here; actor/plan provenance and approval remain governed by the existing candidate contract and separate AI authorization.
