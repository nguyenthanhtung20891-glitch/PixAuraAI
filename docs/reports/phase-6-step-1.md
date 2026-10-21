# Phase 6 Step 1 — Editing Agent core and safety boundary

> Historical Step 1 engineering report. Accepted final closure evidence and current
> integrated Phase 6 status are recorded in [Phase 6 closure](phase-6-closure.md).
> Pending/future-step language below describes the original report time only.


Baseline `a66bbb0a303b707d48590df37cbd369148fba6a8`, clean at audit.
Phase 5 CLOSED / COMPLETE / FULL PASS, accepted with Foundation 39087829531
SUCCESS 7/7 and shells 39087829537 SUCCESS 3/3. Step 1 AUTHORIZED / IN PROGRESS;
completion requires both complete final-SHA workflows observed SUCCESS.
Step 2 NOT STARTED. Phase 7 NOT STARTED. No Step 9+.

## Audit and implementation

Read product/architecture/quality/roadmap/decision authority, ADRs 0008/0009,
document/parser/manual/gesture/editor ownership, capability/dynamic policy,
package/inference/admission/certification and Phase 5 closure records. Phase 2
already distinguishes immutable revisions from detached candidates and assigns
approval to the application service. `actor=ai` is provenance, not authority.
Manual gestures return detached proposals; editor service owns durable approval.
Kotlin/Swift retain lifecycle/OS ownership, shared native owns tool semantics.
No conflicting semantics or new ADR required; DECISIONS records additive scope.

[Core contract v1](../contracts/editing-agent-core-v1.md) freezes human
DIRECTOR/APPROVER, AI ADVISOR/PLANNER/optional approved OPERATOR; separate
INTENT/PLAN/EXECUTION; untrusted planner data; no silent commit or original change.
An additive checked C family reuses the document context/registry/mutex/serials,
shared operation parser, canonical serializer and eleven-entry manual registry.
The existing evaluation envelope is staging only, not a Step 2 plan schema.

Stage creates an owned, bounded, inspectable canonical record tied to an actual
application-supplied native snapshot. Validation is structural and static-tool
validation, not dynamic eligibility/approval. Exact live mismatch/base release
makes it terminal stale. Cancel/release/destroy cannot create history. The
execution-authority gate ALWAYS denies valid proposals with NOT_APPROVED; there
is no approval issuer, executor callback, transition or storage call in agent code.
Native application owners remain trusted; untrusted data cannot supply native
handles, current pointers or actor/approval fields. This is not a native-code sandbox.

Bounds: eight actions, 8192 input/canonical bytes each, eight retained proposals
within existing combined 64 handles, <=65536 retained payload bytes plus bounded
record overhead; no additional assets/metadata/diagnostics, worker, queue or cache.
Statuses reuse document categories plus agent-only NOT_APPROVED=15. No numerical
semantics, hardware/capability/inference trust or resource ceiling changes.

## Acceptance and evidence ledger

New native agent/C-consumer tests cover unsupported ABI; null/overlapping/wrapping
buffers; malformed data and unknown fields; unknown tool/operation/parameter
versions; invalid integer/range; duplicates; excessive actions/bytes/proposals;
typed/stale/cross-context/zero/released tokens; cancel/concurrent validation;
stale live replacement and base release; destruction; closed approval gate;
canonical agreement with manual operation validation. Full document serialization
is identical before/after create, validate, inspect, denied execution and release.
Manual undo works while proposals exist, and another context is unaffected.

All retained Phase 0–5 suites and manual regressions remain required. Windows,
Linux, Apple native, Android arm64-v8a/armeabi-v7a, active ASan/UBSan, complete
application-shell jobs, source checks, workflow lint and diff check must pass.
Actual final SHA, workflow IDs/job results and executed counts are recorded in
final delivery after observation, never inferred from a pending or old run.

A: core boundary and its security/retained evidence. B: Steps 2–6 schemas,
binding/planning/approval/execution/candidate lifecycle, then Step 7 integration
and Step 8 closure. C: optional research/UX/model work. Fix A only.
Production model UNRESOLVED; no U2NETP/model weights, external runtime,
AI UX, network or hidden memory. Original assets and Schema 3 unchanged.
CPU release VALIDATED, Vulkan/Metal UNKNOWN. DH-APPLE-METAL-01 unchanged and
DEFERRED, NOT WAIVED, NOT SATISFIED. Step 2 NOT STARTED. Phase 7 NOT STARTED.
STOP AFTER PHASE 6 STEP 1.

## Local pre-push validation

Windows retained compiler fallback: agent C++/independent C consumer 2/2 PASS,
259 checks, zero failures. Linux native: the same 2/2 PASS and 259 checks, zero
failures. Windows-applicable retained source suite 102/102 PASS, zero skips.
Workflow lint and git diff --check PASS. Initial malformed-envelope expectation
was corrected to the existing parser status 6; parser semantics were unchanged.
The retained geometry source assertion now includes agent records within the
unchanged 64-handle ceiling. A local overlapping Ninja regeneration failed;
sequential regeneration/build/test passed. Full native/platform/sanitizer and
retained Phase 0–5 execution remains required on the final commit, with actual
SHA/run IDs/results recorded in delivery. No unexecuted check counts as PASS.

## Exact-SHA CI remediation

Candidate `1587245422dd23a46ebece8d8fd31b9fa03dabeb`, Foundation 39104013407:
sanitizer job failed before tests at the new C consumer's link. Clang's C link
driver omitted the C++ UBSan vptr runtime required by the instrumented shared
library (`__ubsan_vptr_type_cache`, `__ubsan_handle_dynamic_type_cache_miss`).
Set this C-compiled consumer's LINKER_LANGUAGE to CXX, matching retained consumers.
No instrumentation, diagnostic, test or production behavior is disabled. This
candidate is not FULL PASS; both complete workflows must certify the fix SHA.
Local Clang ASan/UBSan rebuild of both agent targets then passed 2/2, with 259
checks/zero failures, leak detection and halt-on-error active. No suppression.
