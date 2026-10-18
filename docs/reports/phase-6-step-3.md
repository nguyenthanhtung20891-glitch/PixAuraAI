# Phase 6 Step 3 — tool capability registry and agent binding

Baseline `578958979f2fc4b5fea4b53dfc3ecfee7aafe34f`, clean at audit. Step 2
COMPLETE / FULL PASS: Foundation 39106599021 SUCCESS 7/7 and Native application
shells 39106599002 SUCCESS 3/3. Step 3 AUTHORIZED / IN PROGRESS through final-SHA
certification. Step 4 NOT STARTED. Phase 7 NOT STARTED. No Step 9+.

[Registry contract v1](../contracts/editing-tool-registry-v1.md) freezes immutable
manual-derived metadata and exact canonical plan binding. Audit covered product,
architecture, decisions/ADR 0004/0009, manual geometry/tone/detail/preset contracts,
Step 1–2 contracts/reports, document snapshots/JSON/ownership, native C ABI and
Android/Swift consumers. Existing authority suffices; no architecture change/ADR.

The SAME native manual descriptor formatter supplies eleven tools, all exact
operation/parameter version 1/1. Parameters, units, default/neutral values, ranges,
step and categories are reused. Fixed trusted allowlist controls exposure; a
future manual tool is denied by default. No separate tool semantic table,
registration, plugin, scripting, inference tool or preset operation is created.
The hand-reviewable golden table includes every field and is checked against
compiled native output and source derivation. No fingerprint/semantic UUID needed.

Canonical plan binding reuses Step 2 parsing, exact integer parameter validation,
8-action/8192-byte limits and native snapshot binding. Private validated operations
resolve to authoritative entries and the exposure policy. Receipt embeds exact
unchanged canonical plan with approval not_approved, binding_version 1 and
registry_version 1. Receipt <=9216 bytes; registry <=32768 bytes. No new handles,
approval issuer, execution bridge, planner, candidate or persistent state.
Step 2's private parse slot and cancellation/publication fence are retained.

Native corpus covers complete table equality, all tool parameter edges/defaults,
invalid neighbors, malformed types/IDs/UTF-8/NUL/keys/versions/overflow, duplicates,
unknown/removed/future tools, denied descriptor policy, exact ordered multi-action
and repeated-action goldens, valid eight/rejected nine, malformed ABI/ranges/
overlap, context isolation, release, cancellation/recovery, concurrent enumeration
and unchanged serialized document/history. Test descriptor copies are never
installed. Existing shared Android golden boundary now also exercises complete
registry bytes and plan binding; no JNI/Kotlin/Swift semantic implementation added.
Production object scans retain test-fixture and injection exclusion. Step 1
approval-bypass tests and all manual/Phase 0–5/Steps 1–2 gates remain required.

## Certification status

PENDING_EXACT_SHA until both complete workflows succeed on the final Step 3 SHA.
Observed SHA, run/job results and counts will be recorded in bounded engineering
receipt and final delivery after execution. No unexecuted gate is PASS.
Apple hosted/simulator results are software evidence only, never physical proof.

A: current registry/binding/security, consistency and regression evidence.
B: Step 4 planner, Step 5 approval/execution, Step 6 lifecycle, Step 7 integration
and Step 8 closure. C: optional research/measurements. Fix A only.
No architecture blocker identified. Production model UNRESOLVED. Schema 3,
manual editing and Phase 5 provider/profile/trust/admission semantics unchanged.
No external runtime, weights, AI UX or network dependency.
DH-APPLE-METAL-01 unchanged: DEFERRED, NOT WAIVED, NOT SATISFIED.
Step 4 NOT STARTED. Phase 7 NOT STARTED. STOP AFTER STEP 3.

## Local pre-push validation

Windows existing compiler fallback: five agent/native C/release-isolation tests
PASS; final registry corpus 655017 assertions, zero failures. Linux full retained
native suite 43/43 PASS; finalized registry corpus 655017 assertions, zero failures;
expanded normal-production object scan PASS. Retained Step 1 and Step 2/manual
corpora PASS. Windows-applicable source gate 107/107 PASS; supported full Linux
source gate 149/149 PASS, zero skips. Workflow lint and git diff --check PASS.
Final registry corpus under ASan/UBSan: 655017 assertions, zero failures; active
negative probes PASS (ASan exit 86, UBSan exit 87), no suppression. The full
retained sanitizer suite is running; its final result remains pending until observed.

Initial malformed-case expectations were corrected to existing shared statuses,
without parser/semantic changes. Windows test-only metadata objects were separated
from DLL-import consumers to preserve normal symbol ownership; no warning was
suppressed. An initial all-source run on Windows produced 36 Linux-shell test
failures (113/149 PASS); the exact Windows-applicable gate and complete supported
Linux gate subsequently passed unchanged. An initial WSL regex was misquoted and
a shell script had CRLF; repository-local LF scripts supplied exact commands.
An initial sanitizer probe filter matched no tests; the actual two named probes
were then executed and passed. None of these attempts is substituted for evidence.

Apple/Android and final complete retained gates still require observed final-SHA
CI results. This source report remains a certification candidate until both full
workflows pass; final receipt and delivery record actual SHA/run results.

## Exact-SHA CI remediation

Candidate `06b3ff88dd3597b286b487ca6fcaa3d4b2bc3d33`, Foundation 39131709820,
Linux job 117688056207 failed GCC -Werror=misleading-indentation at two new test
statements (lines 50 and 120). Explicit braces separate the diagnostic from its
assertion and the join loop from its final check. No semantic change, warning
suppression, test removal or weakened gate. Both full workflows must recertify
the correction on the new final SHA; old-candidate results cannot close Step 3.
