# Deterministic reference planner v1

FROZEN Phase 6 Step 4. Stable identity reference.deterministic.v1, policy version 1,
C API family 1 in pixaura/reference_planner.h. DETERMINISTIC REFERENCE PLANNER only.
Input: canonical Structured Intent v1 plus trusted native context/live snapshot,
planner version 1 and registry version 1. Output: ordinary canonical Plan v1.
Free-form Intent v1, Plan v1 and Registry v1 remain unchanged.

## Deterministic policy

Each of the eleven [structured goals](editing-structured-intent-v1.md) maps to
exactly one tool tuple 1/1. One action, action_index=0, explicit supplied parameters
unchanged. No goal inference, device policy, scoring, search, recipes, randomness,
clock, locale, model or hash-iteration selection. Goal lookup is exact identity.
No multi-tool release recipe or test-only recipe is introduced. Existing Steps 2–3
retain eight-action/multi-action ordering and nine-action rejection tests; the
reference planner itself emits exactly one action, so never exceeds that ceiling.

The binding is derived from the same trusted native immutable snapshot and checked
against the represented input. No late-current-revision substitution. Source digest,
revision, session, generation, project, document and context are preserved exactly.
Staleness at later approval/execution remains the later Step 5/6 responsibility.

The existing canonical Writer creates the plan. Ordinary Step 2 plan parsing and
document::operation validate the result, including exact integer parameters and
crop constraints. The SAME agent_bind_tools guard used by Step 3 binding resolves
every output operation against immutable manual descriptors and agent-exposure
policy before publication. No parallel tool semantics, bypass or callable closure.
Positive tests also call the public Step 3 binding API on every expected result.
Registry/tool removal, denial or incompatible tuple fails 5; unknown registry or
planner version fails 2. Same-version semantic changes are forbidden by retained
contracts; complete table and compatibility fixtures detect drift. Explicitly
coexisting old tuples remain compatible; no implicit version upgrade/substitution.

Identical canonical input, binding, trusted policy and registry content produces
byte-identical output. Caller owns immutable proposal bytes after return; changing
them creates new untrusted input requiring validation, never hidden replanning.
No semantic UUID, planner provenance field, scratchpad or reasoning is serialized.

## ABI, resources and cancellation

Request is exactly 32 bytes: api_version, struct_size, planner_version,
registry_version, pre_cancel and three reserved-zero uint32 values. All versions
exactly 1; boolean 0/1 only. Canonicalizer and planner share this explicit request.
Checked alignment, accessible ranges, address overflow and all pairwise overlap
rules include context, handle, request, input, output and required-length pointer.
No C++ ownership crosses C; exceptions remain inside the existing boundary.

Structured input <=2048 bytes; canonical plan <=8192 bytes and <=8 actions, actual
policy output exactly 1. Private parse/one-operation/JSON/output allocations only,
no recursive search, candidate, raster, GPU, inference or document transaction.
Query/short output returns 11 and required length with no partial copy; every other
failure preserves all output bytes AND required length. No additional handles or
increase to combined 64-handle ceiling. Existing statuses only: 1 argument, 2
version, 3 handle, 5 unsupported/unresolved, 6 malformed/noncanonical, 7 parameter,
8 resource, 9 stale, 11 short buffer, 13 cancellation, 14 internal.

The existing document context's schema parse slot and nonwrapping schema_epoch
are shared with Steps 2–3. Contention returns RESOURCE=8 without queue. Parsing and
mapping run privately outside the registry lock; final liveness and epoch checks
under the lock precede any output publication. Pre-cancel and observed cancellation
before publication return 13. schema_cancel revokes in-flight work, not future
calls. No separate recovery or Step 6 candidate lifecycle. Caller must quiesce all
calls before destroying context under ADR 0009. Test-only existing checkpoints
exercise active-work and pre-publication cancellation, release and busy rejection.
Independent contexts have independent cancellation/admission and deterministic
output; no mutable global planner state or persistent memory.

## Approval and future planners

Output is NOT_APPROVED. No approval token, callback, issuer, commit, candidate or
execution bridge exists. All original/document/history/manual/pixel state remains
unchanged. A test-only ordinary-operation fixture probes the retained Step 1 gate
and gets NOT_APPROVED=15; no production plan-to-stage adapter is introduced.

Future production planners may use different internal algorithms and inputs. They
must still treat model-produced plans as untrusted, validate through unchanged
Step 2 + Step 3 authority and preserve the human approval/execution boundary.
They need not reproduce this reference policy or use a specific model/vendor.
No production NLP/LLM, inference provider, network, external runtime or weights.

## Evidence

[Repo-owned fixtures](../../tests/fixtures/reference-planner-v1.json) contain 39
positive explicit mappings (each parameter min/max and each goal's neutral value),
independent expected canonical bytes and SHA-256 digests, and 42 negative rows.
Hashes are engineering evidence, never runtime authority. Complete byte comparison,
canonical round trip, public registry binding, repeated planning, malformed ABI,
free-form non-mapping, isolation, cancellation and unchanged document/history tests
run natively. Shared Android debug boundary uses the SAME goldens; normal release
objects exclude fixtures/hooks. Host corpus repeats each positive 50 times, plus
two independent contexts concurrently. No timing-based policy or PASS criterion.

Oversized structured input and too-small output are exercised; nine-action plans
remain rejected by retained Step 2/3 tests. Invalid generated parameters are blocked
by the common validator. Output >8 actions or >8192 bytes is unreachable under
the one-action fixed grammar and binding/parameter bounds, with explicit final
bound checks retained. No synthetic release recipe is added to force such output.
DH-APPLE-METAL-01 and Phase 5 unchanged; physical Apple certification is not claimed.
Step 5 NOT STARTED. Phase 7 NOT STARTED. STOP AFTER STEP 4.
