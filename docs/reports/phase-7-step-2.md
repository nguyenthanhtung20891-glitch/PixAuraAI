<!-- phase7-step2-current-authority -->
## Phase 7 Step 2 - current authority

Phase 6 CLOSED / COMPLETE / FULL PASS. Phase 7 AUTHORIZED / IN PROGRESS.
Step 1 COMPLETE / FULL PASS at `3640d94e473bddd792a12541a45b6bb36ff8cd50`.
Foundation 39768943832 SUCCESS 7/7; Native application shells 39768943890 SUCCESS
3/3, exact SHA, completed, attempt 1.
Step 2 AUTHORIZED / IN PROGRESS. Exact-SHA certification pending.
Assistance Provider & Proposal Envelope v1: internal shared-native, bounded,
canonical, ephemeral and UNTRUSTED. Existing native planner/Plan/Registry
validation only; no approval issuer, candidate, execution or history authority.
No new public ABI. Intent v1, Structured Intent v1, Plan v1, Registry v1,
reference planner, approval/candidate, three-mode interaction, Schema 3 and
Phase 5 unchanged. Production model UNRESOLVED. CPU VALIDATED; Vulkan/Metal UNKNOWN.
No model/runtime/weights/NLP/cloud/network AI/AI UX. Local-first.
DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED.
Phase 7 Steps 3-8 NOT STARTED. Phase 8 NOT STARTED. No Step 9.
Full retained source/native/platform, active ASan/UBSan, release isolation,
workflow lint and diff checks required. Both complete exact implementation-SHA
and separate closure-SHA workflows required; every job completed/SUCCESS.
STOP after Step 2. Do not implement Step 3.
See docs/contracts/assistance-provider-proposal-v1.md and docs/reports/phase-7-step-2.md.
<!-- phase7-step2-history -->

# Phase 7 Step 2 — Assistance Provider & Proposal Envelope v1

## Purpose and baseline

Implement the first concrete replaceable intelligence/data boundary, not a model
or product UX. Clean main and origin/main verified at certified Step 1 closure
3640d94e473bddd792a12541a45b6bb36ff8cd50 after fetch before changes.
Step 1 closure: Foundation 39768943832 SUCCESS 7/7; shells 39768943890 SUCCESS 3/3,
attempt 1, exact SHA. Phase 6 remains closed; certified history is retained.

## Architecture and implementation

Internal shared C++ Provider/ReferenceProvider receives data-only Request objects.
No editor, approval issuer, candidate, native authority handle, callback, worker,
network or platform ABI. Reference identity reference.assistance.v1, contract 1,
finite capability mask 7. Static construction; no dynamic registration/plugins.
Reference wrapping does not make output trusted. The separate validator uses the
existing native Parser/Writer, binding, Structured Intent planner and Plan parser,
then manual-derived Registry find/agent_exposable checks. Existing contracts are
not reinterpreted. No Kotlin, Swift, JNI, C ABI or UI changes.

Envelope has eight exact fields: binding, envelope_version, kind, payload,
proposal_id, provider_id, provider_version, registry_version. Kinds advice (1),
structured intent (2), plan candidate (3). Advice validates an explicit one-tool
suggestion but yields no Plan. The other kinds yield ordinary canonical Plan v1,
still unapproved. No approval/candidate creation or execution function is exposed.
Proposal correlation ID is not a capability; changing it cannot confer authority.

## Canonical representation and bounds

Existing UTF-8 sorted-object-key/exact-integer/terminal-LF JSON; exact canonical
input required, strict unknown fields/versions/duplicates. Envelope 12288 bytes,
Plan payload 8192/eight actions, structured/advice payload 2048/one goal, binding
512, IDs 1..64 ASCII. No summary field/list/recursive extensions. All embedded
semantics and parameter ranges remain with existing Phase 6 authority.

Both outer/inner binding must match trusted immutable current snapshot/context:
project/document/revision/session/context/generation/source digest. The selected
provider comes from trusted construction, not self-assertion. Validation does not
replace final live source-serial/ABA/repository-CAS approval/execution checks.
No new native handle, raised ceiling or parallel freshness authority.

## Lifecycle and concurrency

One synchronous active request; competing call RESOURCE/BUSY, no queue/retry.
Nonsticky nonwrapping cancel epoch and stopped state protected by short fence;
completion rechecks before returning bytes. Cancel/stop winner returns CANCELLED
without a result. Completion winner only returns untrusted data. No callbacks or
provider-owned threads. Owner stop -> join ALL calls -> destroy, ADR 0009 intact.
Independent provider instances isolate cancellation/shutdown. Input stays alive
and immutable for each call. Mode/platform integration remains Step 6 future work.

## Security findings and evidence plan

A: strict canonical/version/provider/kind/binding/tool validation, bounded parsing,
non-authority IDs, cancellation/shutdown and independent owners required here.
The provider interface has no mutation/approval/candidate capability. No unresolved
architecture issue. Native tests compare entire document serialization, original
metadata, operations/revisions before/after; no history mutation. Retained Phase 6
tests continue to prove NOT_APPROVED, exactly-once atomic execution, stale/ABA,
manual/undo/redo/cancellation/publication/teardown semantics.

B: later Steps 3–6 product interaction/ownership integration; Step 7 production
provider/model admission. No product provider or NLP selected. Physical Apple
certification remains deferred. C: optional tuning/measurements, not trust.

The positive corpus reuses 39 independently reviewed structured/Plan goldens for
all three kinds (117 cases), ten repeated byte comparisons, an independent exact
envelope golden and existing maximum-eight-action Plan fixtures. Negatives retain
42 structured planner cases and existing Plan cases, malformed UTF-8, duplicates,
unknown fields/versions/kinds/providers, bounds, forged IDs, each binding field,
stale generation/ABA, free-form non-mapping, Registry/tool rejection. 600 competing
requests race cancellation/shutdown with only bounded success/BUSY/CANCELLED.
Release-isolated standalone checkpoints additionally prove cancel/stop winning
before work and before completion, nonsticky next request, and competing BUSY.
Actual editor-produced bindings pass through provider/validation and still return
NOT_APPROVED at the real execution boundary before ordinary human approval.
Allocation-failure sweeps compile production codec independently under the existing
replacement allocator. No replacement allocator/hook ships in production.
The new internal Outcome uses allocation-free empty optional failures, including
MSVC debug-STL failure paths. A Windows test-link attribute error was corrected
with the existing PIXAURA_BUILDING convention on the integration test target;
warnings-as-errors and runtime/public ABI are unchanged.

## Certification

Observed local evidence before the implementation commit:

| Gate | Result |
| --- | --- |
| Windows retained source | 123/123 PASS, no skips |
| Linux retained source | 165/165 PASS, no skips |
| Windows native fallback | 47/47 PASS; final affected assistance/execution/allocation tests 3/3 PASS |
| Linux native | 48/48 PASS; final affected tests 3/3 PASS |
| ASan + UBSan full retained suite | 50/50 PASS; active negative probes exit 86/87 |
| Assistance corpus | 2082 assertions, 117 positive kinds/cases, ten byte repeats, 600 concurrent requests; zero failures |
| Native Agent execution corpus | 1623 assertions, zero failures under sanitizers |
| Android Gradle | Debug/release, JVM, lint and instrumentation APK: BUILD SUCCESSFUL, 151 tasks |
| Actual Android release libraries | All scanned libraries PASS; ARM64, ARMv7 and emulator builds retained |
| Workflow lint / git diff --check | PASS |

Commands: the complete Foundation retained Node list on Windows; Node tests/*.test.mjs
on Linux; CMake/CTest Windows fallback and Linux; scripts/check-sanitizers.sh;
Gradle --no-daemon --dependency-verification strict assembleDebug assembleRelease
testDebugUnitTest testReleaseUnitTest lintDebug lintRelease assembleDebugAndroidTest;
scripts/check-model-release.mjs over actual release libraries; actionlint and diff
check. All temporary/cache writes remain inside the repository. No tests or
warnings/sanitizers were weakened, skipped or made optional. Local title/fixture/
test-link findings were corrected before certification, without CI reruns.

Apple execution and Android emulator instrumentation are CI-only here, not yet
claimed PASS. Exact-SHA certification pending. Both full workflows required on
the implementation SHA; a separate closure SHA requires both independently.
The implementation SHA does not yet exist; observed evidence will be recorded
after execution, without self-referential SHA claims.

Release scans retain all test-key/approval/candidate/platform hook checks and add
assistance fixture/fault marker rejection. No test provider execution authority or
model checkpoint exists. Shared codec compiles through existing native source
lists on Android and Apple; hosted/simulator evidence is software only.

## Frozen state and limitations

Intent v1, Structured Intent v1, Plan v1, Registry v1, deterministic reference
planner, human approval/binding/consumption, candidate lifecycle/publication,
Schema 3, manual/history/undo/redo, Step 1 three-mode contract, Phase 5 unchanged.
Production model UNRESOLVED. U2NETP audited/unsupported/unapproved/unincorporated;
checkpoint rights unresolved. CPU VALIDATED; Vulkan/Metal UNKNOWN; none CERTIFIED.
DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED. No NLP/model/runtime/
weights/cloud/network/AI UX. Steps 3–8 NOT STARTED. Phase 8 NOT STARTED.

## Changed files

AGENTS.md; ARCHITECTURE.md; DECISIONS.md; QUALITY_GATES.md; ROADMAP.md;
docs/plans/phase-7-roadmap.md; docs/contracts/assistance-provider-proposal-v1.md;
docs/reports/phase-7-step-2.md; CMakeLists.txt;
packages/core/src/document.cpp; packages/core/src/assistance.hpp;
packages/core/src/assistance_codec.inc; packages/core/tests/assistance_test.cpp;
packages/core/tests/document_allocation_test.cpp; packages/core/tests/agent_execution_test.cpp;
scripts/check-model-release.mjs;
tests/agent-execution.test.mjs. No production dependency or public ABI addition.
