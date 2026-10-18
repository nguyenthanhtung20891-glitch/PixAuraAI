# Structured Intent v1

FROZEN Phase 6 Step 4 under the Product Owner's approved Option B. Separate from
free-form Intent v1, which remains byte/semantic compatible and valid. See the
explicit decision in DECISIONS.md. This is a resolved machine-semantic editing
request, not prose, a plan, approval or executable code.

## Exact envelope

Exactly four root fields: `binding`, `goal_id`, `parameters`,
`structured_intent_version` (integer 1). No optional extensions or defaults.
Binding has exactly the existing Step 2 context/project/document/revision/session,
generation and source_sha256 fields, checked against the trusted live snapshot.
The caller supplies its actual live handle under ADR 0009/application ownership;
this is not a sandbox against dishonest native host code. Malformed binding is 6,
well-formed stale/cross-context bytes are 9, wrong/released native handle is 3.

| Exact goal ID | Authoritative tool ID | Required exact integer parameters |
| --- | --- | --- |
| adjust.exposure | pixaura.exposure | milli_ev |
| adjust.brightness | pixaura.brightness | milli_linear |
| adjust.contrast | pixaura.contrast | milli_stops |
| adjust.highlights | pixaura.highlights | milli_ev |
| adjust.shadows | pixaura.shadows | milli_ev |
| adjust.saturation | pixaura.saturation | milli_ratio |
| adjust.temperature | pixaura.temperature | kelvin |
| detail.sharpen | pixaura.sharpen | milli_amount |
| detail.blur | pixaura.blur | milli_strength |
| transform.rotate_quarter | pixaura.rotate | quarter_turns |
| transform.crop_normalized | pixaura.crop | height_ppm, width_ppm, x_ppm, y_ppm |

All tool tuples are operation_version=1 / parameter_version=1. Identity mapping
only is compiled in structured_goals.hpp; no parameter semantic table is added.
The exact existing [registry table](../../tests/fixtures/agent-tools-v1.json),
manual geometry/tone/detail contracts and document::operation remain authority
for types, units, bounds, steps, neutral values and crop extent constraints.
Missing values reject; neutral/default descriptors never fill missing parameters.
Unknown goal is 5; invalid parameters 7; duplicates/malformed encoding 6. No clamp,
float conversion, expression, nearest match, direction-only or generic enhancement.
No caller-supplied tool tuple/version, recipe composition or goal-text field.

## Canonicalization and bounds

The existing native JSON Parser/Writer are reused: deterministic UTF-8, sorted
object keys, exact decimal integers, canonical escaping, one terminal LF, no NUL
or BOM. Canonical input/output each <=2048 bytes. Fixed grammar has only root,
binding and parameter objects; no arrays or recursive scalar containers. Existing
parser strings <=256 decoded bytes; goal_id is nonempty and <=64 bytes, with
membership in the eleven exact IDs. Binding identity bounds are unchanged.
Unknown/missing/duplicate fields at every level, invalid UTF-8, float/exponent/
negative-zero, overflow, wrong types and trailing garbage reject. Unknown version
returns 2; no migration. Input remains untrusted even if produced locally.

pixaura_structured_intent_canonicalize normalizes structurally valid input only
after binding and authoritative parameter/tool-exposure validation. The planner
requires full canonical byte equality. Free-form Intent v1 is recognized solely
to reject it as unsupported (5); its goal text is never interpreted, tokenized,
matched to a recipe or translated. The existing free-form API still accepts that
same valid envelope. Invalid envelopes retain fail-closed errors.

## Ownership and exclusions

Ephemeral caller-owned bytes; no retained object, new handle, persistence, Schema 3
change or conversation memory. No plan approval/execution, candidate/revision,
pixel work, original mutation or manual state change. No NLP/model, runtime,
weights, network or AI UX. Shared live snapshot/cancellation authority is defined
by [reference planner v1](editing-reference-planner-v1.md). Step 5 NOT STARTED.
Phase 7 NOT STARTED. DH-APPLE-METAL-01 unchanged.
