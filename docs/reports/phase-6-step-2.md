# Phase 6 Step 2 — intent and editing plan schemas

> Historical Step 2 engineering report. Accepted final closure evidence and current
> integrated Phase 6 status are recorded in [Phase 6 closure](phase-6-closure.md).
> Pending/future-step language below describes the original report time only.


Baseline `f07016da62556b0634fde8ed081b1361b8c9eb57`, clean at audit. Step 1
COMPLETE / FULL PASS: Foundation 39104299393 SUCCESS 7/7; shells 39104299376
SUCCESS 3/3. Step 2 AUTHORIZED / IN PROGRESS through exact-SHA certification.
Step 3 NOT STARTED. Phase 7 NOT STARTED. No Step 9+.

Audit: product/architecture/quality/roadmap/decision authority; Step 1 contract,
report, ABI and registry; document/revision/session/actor and ADR 0009 ownership;
existing JSON Parser/Writer, integer and UTF-8 defense; manual registry/operation
versions/parameters; Schema 3 persistence. Reuse is sufficient, no new ADR or
architecture conflict. Accepted Step 1 and Phase 5 contracts are unchanged.

[Intent/plan v1](../contracts/editing-intent-plan-v1.md) freezes separate semantic
objects and checked handle-free byte-copy API. One intent category `edit`, whole
document scope, <=256-byte UTF-8 goal. Plan actions preserve array order and
contiguous indices; exact existing IDs/operation/parameter versions/integers,
including repeated tools. No invented semantic UUID/hash identity.

Both carry context/project/document/source/revision/session/generation binding,
derived/checked against native snapshot authority. Shared Parser/Writer and
operation validation are reused; unknown/duplicate fields/versions and malformed
data fail closed. Canonical UTF-8 JSON uses sorted keys, exact integers, ordered
arrays and one LF, max 8192 input/canonical bytes and eight actions. No coercion
or parameter/Unicode normalization. Strict canonical-input mode compares full bytes.

One parse slot per context, no new object handles, queue or persistent payload.
Context-local cancellation epoch revokes work before output publication; private
snapshot protects source lifetime. Checkpoint injection is test-only and actual
production objects are scanned. Schema cancellation does not create a candidate
lifecycle, approval mechanism or execution bridge. All output copying is fenced;
failures preserve output/size except accepted short-buffer size queries.

Goldens: intent, exposure, crop/rotate, seven tone/color actions, detail pair,
eight-action sequence with repeated exposure, rejected nine-action sequence.
Canonical bytes/digests are small reviewable repo fixtures. Shared native corpus
executes on Windows/Linux/Apple and Android instrumentation via argument-free
debug JNI; no platform semantics or production app flow added.

Security tests cover empty/truncated/oversized/unknown/missing/duplicate payloads,
invalid indices/parameters/unsigned numbers/overflow/UTF-8/trailing garbage,
strict mode, cross-context bytes/handles, native release, pre-cancel and precise
post-parse cancellation, busy rejection, recovery and cancellation isolation.
Complete serialized document/history bytes remain unchanged. Retained Step 1
259 checks and all Phase 0–5/manual gates remain mandatory, including active
ASan/UBSan, Apple hosted gates, Android two ABIs/debug/release/JVM/lint/instrumentation,
workflow lint and diff check. No pending/unobserved job counts as PASS.

Completion requires BOTH full workflows SUCCESS on the same final Step 2 SHA.
Observed final SHA/run IDs/results/counts are recorded in delivery and bounded
engineering evidence after execution. No self-referential or premature SHA claim.

A: schema/canonical/binding/security and retained evidence. B: Step 3 binding,
Step 4 planner, Step 5 approval/execution, Step 6 candidate lifecycle, Step 7
integration/certification and Step 8 closure. C: optional research/UX/model work.
No architecture blocker identified. Schema 3/manual semantics unchanged;
Production model UNRESOLVED; no runtime, weights, AI UX. Phase 5 semantics and
DH-APPLE-METAL-01 unchanged (DEFERRED, NOT WAIVED, NOT SATISFIED).
STOP AFTER STEP 2. Step 3 NOT STARTED. Phase 7 NOT STARTED.

## Local pre-push validation

Windows retained compiler fallback: 4/4 affected native tests PASS, 319633 schema
checks/zero failures, retained Step 1 259 checks/zero failures, C consumer and
production-object isolation PASS. Linux initial affected suite 4/4 PASS; final
Linux Clang ASan/UBSan affected suite 4/4 PASS with the same 319633/259 checks.
Both active-instrumentation probes PASS (ASan exit 86; UBSan exit 87), no suppression.
Windows-applicable retained source suite 104/104 PASS, zero skips. Workflow lint
and git diff --check PASS. Initial source consistency check caught lowercase
report marker wording, corrected before full rerun. A local sanitizer test filter
was misquoted by the shell; separate filters executed and passed. No check was
disabled or loosened. Final full retained native/platform/sanitizer results must
still be observed on the final commit before Step 2 closes.

## CI provisioning retry

Candidate `db30dc5e25cd39c918708b6e7ad2c8a1bca86ee5`, Foundation 39106320127:
armeabi-v7a job 117612820196 failed BEFORE configure/compilation while SDK Manager
installed the pinned NDK 28.2.13676358: `Error on ZipFile unknown archive`.
The same run's arm64-v8a job installed that revision and passed. No source,
toolchain pin, security check or ABI gate is changed to address this transient
provisioning failure. This report-only retry commit requests fresh complete
workflows on a new SHA; no result from this failed candidate certifies Step 2.
