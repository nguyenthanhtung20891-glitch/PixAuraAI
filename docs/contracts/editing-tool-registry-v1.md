# Editing tool registry v1

FROZEN Phase 6 Step 3. Additive to editing-agent-core-v1 and editing-intent-plan-v1.
INTENT != PLAN != EXECUTION. Binding grants neither approval nor execution.

## Authority and exposure

`manual::registry()` in `manual_tools.cpp` is the single compiled descriptor
authority used by manual editing. Agent enumeration uses the SAME formatter,
descriptors, parameter arrays, units, defaults, bounds and categories. Native
`document::operation` remains the authoritative parameter/relational validator.
No manual semantics, rendering, history, operation versions or persistence change.

The trusted `manual::agent_exposable` policy explicitly allows ONLY these tuples,
in ascending ASCII tool-ID order (all operation_version=1, parameter_version=1):

| Tool ID suffix (`pixaura.`) | Exact integer parameters | Inclusive bounds | Default |
| --- | --- | --- | --- |
| blur | milli_strength | 0..1000 | 0 |
| brightness | milli_linear | -1000..1000 | 0 |
| contrast | milli_stops | -2000..2000 | 0 |
| crop | height_ppm, width_ppm; x_ppm, y_ppm | 1..1000000; 0..999999 | 1000000; 0 |
| exposure | milli_ev | -5000..5000 | 0 |
| highlights | milli_ev | -2000..2000 | 0 |
| rotate | quarter_turns | 0..3 | 0 |
| saturation | milli_ratio | 0..2000 | 1000 |
| shadows | milli_ev | -2000..2000 | 0 |
| sharpen | milli_amount | 0..1000 | 0 |
| temperature | kelvin | 4000..25000 | 6504 |

Step is exactly 1 throughout. Crop also requires x+width and y+height <=1000000.
The hand-reviewable [complete golden table](../../tests/fixtures/agent-tools-v1.json)
records exact units and metadata. Defaults describe neutral values; missing
parameters NEVER acquire defaults. No float conversion, clamp, rounding, unit
guess or implicit version upgrade. Display names are non-authoritative.
Categories are existing geometry, tone_color and detail IDs, not localized UI.
Manual registry validation remains in force before enumeration.

Future manual tools are denied until explicitly added to trusted exposure policy.
Known-but-denied, removed, unknown and future-version tools reject status 5.
No synthetic/test tool is installed, even in tests; tests pass a private copied
descriptor to the same pure exposure predicate to prove denial. No setter or
runtime registration exists. Presets are recipes, not a registered operation;
the frozen empty built-in preset catalog grants no additional tool identity.
No inference provider, hardware capability, generic filesystem/network command,
model, shader, source, script, plugin or dynamically loaded symbol is a tool.

## Enumeration and deterministic representation

C API family and registry version are independently exactly 1. Unknown versions
reject status 2. `pixaura_agent_tools_registry` returns bounded native JSON:
`registry_version` and `tools`, with one terminal LF and no NUL. Each descriptor
has the existing manual descriptor fields plus `agent_exposable:true`.
The same formatter preserves sorted keys and sorted tool/parameter IDs. All
eleven descriptors are immutable static data. Existing limits remain <=16 tools,
<=8 parameters/tool, <=64-byte metadata fields and <=32768 serialized bytes.
The current table has at most four parameters/tool. No fingerprint or random
registry identity is needed; reviewed exact full-byte goldens certify semantics.
Registry version plus tool/operation/parameter tuple is the capability identity.

Enumeration is context-independent and thread safe. It describes semantic
existence/exposure, NOT runtime executability, platform availability, resource
admission or Phase 4 hardware policy. Platforms consume identical native bytes.

## Canonical plan binding

`pixaura_agent_tools_bind` takes the existing 32-byte checked schema request,
kind PLAN=2, require_canonical=1, and <=8192 canonical plan bytes. Other kinds or
non-strict mode reject 1. All Step 2 structure/version/action/order/UTF-8/integer
and native snapshot binding checks run unchanged. Its private validated operations
are matched to exact immutable manual descriptors and trusted exposure policy.
No model bytes select a function pointer, closure, module, symbol or executable.

Success returns a bounded inspectable JSON receipt with EXACT keys:
`approval:"not_approved"`, `binding_version:1`, `plan:<unchanged plan object>`,
`registry_version:1`. The embedded plan's complete canonical object bytes are
identical to input excluding only its terminal LF; the receipt has its own LF.
Every action, parameter, version, index and ordering remains unchanged. The plan
itself already exposes exact tool tuple and validated parameters; no opaque
binding token or duplicate mutable action representation is invented. Repeated
tools and valid neutral operations are retained. 1..8 actions; nine reject 8.
Receipt <=9216 bytes, including LF. Rejected binding returns shared status and
NO receipt. Receipt is engineering/runtime data, never an approval or execution
handle; it cannot invoke the editor. Step 1 execution authority remains 15.

Binding uses Step 2's one private parse/context slot, immutable snapshot, context
local nonwrapping cancellation epoch, busy RESOURCE=8 and final live-token fence.
Pre-cancel or cancellation before publication returns 13; stale bytes return 9;
wrong/released native handle returns 3. No queue, candidate or persistent object.
Context destruction requires caller quiescence under ADR 0009. No retained handle
or additional combined-handle budget. New valid calls work after cancellation.
Immutable registry existence is separate from document applicability and later
approval/execution eligibility. Rebinding is required after relevant state change;
receipts cannot override current native authority.

## ABI and security

Caller owns all buffers; no heap ownership crosses C. `agent_tools.h` reuses
checked Step 2 struct size/version, enum/boolean/reserved fields and alignment.
All ranges are checked for null/nonzero, address overflow and pairwise overlap,
including context, handle, request, input, output and required-length pointer.
Caller supplies truthful accessible memory. Exceptions never cross C.
Query/short buffer returns 11, required length and no partial bytes. ALL other
failures preserve output and required length. Shared statuses only: 1 invalid
argument, 2 unsupported ABI/registry version, 3 invalid handle, 5 unsupported tool
tuple/exposure, 6 malformed, 7 invalid parameters, 8 resource, 9 stale, 11 short
buffer, 13 cancelled; allocation/internal failures retain 8/14 respectively.

Unknown/duplicate/missing fields, malformed/oversized IDs, invalid UTF-8, NUL
identities, wrong types, overflow, min-1/max+1 and noncanonical bytes fail closed.
No validation result mutates document, history, current revision, original or
candidate state. No approval issuer, execution bridge, planner, AI UX, network,
new tool, Schema 3 change, model/runtime or Phase 5 change. Step 4 NOT STARTED.
Phase 7 NOT STARTED. DH-APPLE-METAL-01 unchanged and deferred.

## Evidence requirements

Full-table bytes must equal the checked-in fixture on Windows/Linux/Apple and
the shared Android boundary corpus. Fixtures are compiled ONLY into existing
certification test paths; normal production objects are scanned. All Step 2 plan
goldens bind (including crop/rotate, tone/color, detail, repeated tools and eight
actions), nine rejects. Native tests compare every parameter edge/default and
invalid neighbor to shared validation, test malformed ABI/buffers, cancellation,
isolation, concurrency and unchanged serialized document/history. Retain all
manual, Phase 0–5 and Steps 1–2 regressions, active ASan/UBSan, workflow lint,
diff check and both complete same-final-SHA workflows. Software evidence cannot
satisfy the physical Apple gate or promote Phase 5 provider trust.
