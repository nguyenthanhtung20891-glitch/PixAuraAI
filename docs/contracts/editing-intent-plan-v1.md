# Editing intent and plan v1

FROZEN Phase 6 Step 2. Additive to editing-agent-core-v1; INTENT != PLAN !=
EXECUTION. Parsing, canonicalization and document binding grant no approval,
eligibility, tool execution, revision or persistence authority. Step 3 NOT STARTED.

## Semantic objects

Intent version 1 has EXACTLY `binding`, `goal`, `intent_version`, `scope`, `type`.
`intent_version` is integer 1, `type` is `edit`, `scope` is `document`.
The single category avoids speculative tool/generative taxonomies. `goal` is
nonempty UTF-8, <=256 decoded UTF-8 bytes, excluding U+0000..001F and U+007F.
It states WHAT the human wants, never a command, prompt template or executable.
No language parser/planner exists. A valid intent need not admit any valid plan.
Original immutability and approval are unconditional system constraints, not
model-editable constraint flags. No extensible constraints/metadata object.

Plan version 1 has EXACTLY `actions`, `binding`, `plan_version` (integer 1).
Actions number 1..8; each has EXACTLY `action_index`, `operation_version`,
`parameter_version`, `parameters`, `tool_id`. Indices are integers 0..N-1 in
array order. Wrong, duplicate, skipped, negative or reordered indices reject.
Array order is semantically significant and is never sorted or optimized.
Repeated tool IDs, including two exposure actions, are valid; distinct indices
identify positions. No no-op/fusion/quality/eligibility rule is invented.

Tool IDs and both versions map exactly to accepted Phase 3 operation contracts.
Parameters use the same names, exact integer units/ranges and relational checks.
Shared `document::operation` validation is reused; no second parameter table or
Step 3 agent capability registry. Unknown tool/version/parameter rejects, no
float/exponent, expression, implicit unit, coercion or clamp. Current tools have
at most four parameters; unknown keys reject through existing authoritative rules.
The adapter uses private index-derived temporary operation IDs solely to invoke
that validator; they are not serialized or durable operation identities.

No semantic intent/plan IDs, hashes or random UUIDs are needed in Step 2. No
planner association/provenance/model/vendor/tokenizer/request/chain-of-thought
fields. Plan bytes stand for HOW edits are proposed, separately from the intent's
WHAT. A future planner/application may associate the two; no fake plan or new
identity authority is created here. Later execution must assign ordinary fresh
operation/revision identities under existing history rules, after approval.

## Binding and validation layers

Both objects require EXACT binding fields:

| Field | Type / authority |
| --- | --- |
| context_id | 32 lowercase hex; existing native context lifetime identity |
| project_id / document_id | 32 lowercase hex; immutable native snapshot |
| revision_id | 32 lowercase hex; snapshot's current approved revision |
| session_id | 32 lowercase hex; existing session lifetime identity |
| generation | integer 0..9007199254740991; existing session generation |
| source_sha256 | 64 lowercase hex; immutable source descriptor |

`pixaura_agent_schema_binding` derives canonical binding bytes from the trusted
application-supplied live snapshot, never model claims. Canonicalization validates
the represented binding against this same native authority. Malformed identities
reject 6; well-formed mismatches reject STALE_BASE=9, including cross-context
bytes and same-revision/different-generation ABA. Wrong/released native handles
reject 3. Context ID is runtime binding, not permission to relocate a plan between
sessions. Caller/app owner must provide its actual live handle under its existing
ownership rules; this is not a sandbox against dishonest native host code.

Structural/schema validity, tool identity/version validity, parameter validity,
and binding match are separate checks. Dynamic capability/stack/pixel/resource
eligibility and human approval remain distinct later checks. No parsed or
canonical plan is approvable/executable merely because these checks passed.
There is no bridge to Step 1 operation staging, document transition or storage.

## Canonical encoding and bounds

Reuse existing UTF-8 JSON parser and canonical writer: ASCII-sorted object keys,
arrays in recorded order, exact decimal integer tokens, no insignificant spaces,
canonical string escapes and exactly one terminal LF; no BOM or terminal NUL.
UTF-8 is validated, including continuation/overlong/surrogate limits. Unicode
normalization is NOT performed; decoded strings with different code points remain
different semantic values. Equivalent JSON escape spelling canonicalizes equally.
Negative zero, floating/exponent notation and integers above 2^53-1 reject.

Input and canonical output each <=8192 bytes, including terminal LF. Input parser
limits decoded strings to 256 bytes; tool ID semantic bound remains 64 bytes,
identities have their exact lengths, actions <=8 BEFORE array growth. Grammar
allows only fixed objects/arrays, with <=4 container levels; no recursive extension
or arbitrary metadata. A private bounded parse tree/output string is the only
new payload ownership. Output may expand due to escaping; >8192 rejects.

Unknown fields at EVERY semantic level reject v1. Duplicate decoded keys reject,
including conflicting escaped spellings. Missing fields, wrong JSON types,
versions, UTF-8, trailing garbage and nested scalar objects reject. No implicit
migration/defaults. Unknown object version returns existing UNSUPPORTED_ABI=2;
other statuses reuse Step 1/document.h, including malformed=6, unsupported
tool/version=5, invalid parameter=7, RESOURCE=8, STALE=9, CANCELLED=13.

Normalization input may use arbitrary valid key order/whitespace/string escapes.
With `require_canonical=1`, input MUST equal the entire canonical output including
LF; otherwise reject 6. Flags/enums require exact accepted values, no unknown bits.

## ABI, lifecycle and cancellation

`agent_schema.h` is C family version 1. Request is exactly 32 bytes with version,
size, kind INTENT=1/PLAN=2, boolean canonical/pre-cancel fields and reserved zeros.
Buffers retain checked alignment/ranges/overlap and truthful accessible memory
requirements. All exceptions remain behind the existing C exception boundary.
Query/short buffer returns 11 and required bytes, no partial copy. Other failures
preserve ALL outputs, including required length. Full bytes publish only after
parse, canonical checks, cancellation fence and bound native token liveness.

No intent/plan handles, retained objects, serials, registry entries or persistent
state are added. Caller owns ephemeral input/output bytes; ordinary release is
caller buffer disposal. Existing proposal/document ceilings and lifecycle remain
unchanged. Cross-context native handles reject; cross-context bound bytes cannot
canonicalize successfully. Already-owned public bytes have no secrecy/authority.

At most one schema parse per document context; contention returns RESOURCE=8,
without queue/retry. Existing registry mutex admits/releases the slot. Parsing
runs on a private immutable snapshot outside that mutex so
`pixaura_agent_schema_cancel` can revoke active work by advancing a context-local
nonwrapping epoch. Pre-cancel returns 13. Cancellation after private parse but
before publication returns 13 with no output. New calls capture the new epoch;
cancellation is not sticky for later work. Epoch exhaustion revokes active work
and denies new work with RESOURCE, never wraps. No separate editing recovery or
capability policy. Source/live release while parsing rejects at final fence.
Context init/destroy stay exclusive; owner joins/quiesces calls before destroy.
These bounded schema-copy fences are not the future Step 6 candidate lifecycle.

## Evidence and exclusions

Repo-owned goldens cover intent, exposure, crop/rotate, tone/color, detail,
eight actions with repeated exposure, and rejected nine actions. Expected canonical
bytes and SHA-256 digests are fixture evidence only, not semantic identities or
runtime authority. Independent Node canonical-byte checks and shared native
fixtures run on Windows/Linux/Apple; Android debug JNI runs the SAME native
goldens, no Kotlin semantic implementation. Both Android ARM ABI builds retained.
Private deterministic cancellation/busy/release checkpoints are certification-only
and production object scans prove absence of hooks/fixtures. No release adapter.

Manual editing and Schema 3 unchanged. No execution, approval issuer, production
planner, capability registry, external runtime, weights, AI UX or network.
Phase 5 provider trust/admission/profile/package semantics unchanged; production
model UNRESOLVED. DH-APPLE-METAL-01 unchanged. Step 3 NOT STARTED; Phase 7 NOT STARTED.
