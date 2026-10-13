# Phase 4 Step 6 — Cross-tier replay & certification

Status: IN PROGRESS / VALIDATION. Authorized baseline HEAD and origin/main
`ce8d513764d6167aa6756b22ef5b0f81698c11c9` matched with clean working tree/diff.
Steps 1–5 COMPLETE / FULL PASS. Frozen seven-step roadmap unchanged.
Step 7 NOT STARTED. [Contract](../contracts/cross-tier-replay-certification-v1.md).

Acceptance requires bounded HIGH→MID→LOW and LOW→HIGH, T0 complete manual flow,
representative/order-sensitive stacks, exact canonical/history/source identity,
full CPU reference stability, pressure/recovery/rejection, optional backend trust
cases, Android/Apple canonical parity, independent contexts, fresh process-style
policy, security/resource/race gates and both complete exact-final-SHA workflows.

Initial inspection found a sequencing concern to test: editor reopen acquires
a durable session before decode admission. A critical rejected reopen must not
advance storage epoch/session. No product change until failing evidence proves
the defect. Successful reopen still requires fresh ownership under existing ADRs.

Observed A: `build/step6-rejection-test.log` fails before.epoch==after.epoch after
Open resource failure 8. Minimal correction prepares decode/native state before
durable session acquisition and checks expected epoch atomically; session-free
canonical semantics and existing unconditional ownership API remain unchanged.
Rejected critical/malformed reopen now preserves canonical and durable envelope.

Bounded common corpus and frozen independently inspected artifacts are consumed
by native hosts, Android debug instrumentation and Swift macOS/iOS Simulator.
Release Android flag OFF and Swift release product dependencies exclude test
controls. Active inspection uses a test-only owner snapshot/checkpoint seam.
B harness fixes: avoid overlapping separate SQLite amalgamations in an active
project; Windows links its private test parser rather than exporting production
C++ internals; preserve the canonical serializer newline in exact golden bytes.

Local observed source/docs 90/90 (zero skips), fixture CRC/source/graph checker,
workflow actionlint and diff check PASS. Windows native 31/31 PASS. Android
debug/release, three native ABIs, JVM, lint and instrumentation APK build PASS
(151 tasks). Linux retained regression 31/31 PASS; corrected new corpus 1/1 PASS
after schema-inspector lifetime remediation. Final combined and sanitizer gates
remain under validation. Shared corpus completes locally in under eight seconds.
Logs: `build/step6-{source,windows,android,linux-tests,test}.log`.

Certified development corpus includes 8-revision/7-operation HIGH→MID→LOW,
4-revision LOW→HIGH, six order-pair projects, same-artifact peer import with redo,
branch semantics, 12 fresh reopen cycles, two isolated contexts, 16 pressure vs
commit/cancel races, native checkpoint-paused old render completing after newer
eligible render, stop/join/destruction, malformed/trust/budget controls and
explicit schema 1→2→3 with no automatic migration. Matrix bounded to 64 rows /
32 KiB; each canonical document ≤64 KiB; temporary suite resources reclaimed.
Source/hash/reference and pressure-rejection evidence are synthetic, not hardware.
ADB discovery observed zero connected/physical Android devices, no identifiers
printed. No fresh Z Fold7 measurement or scope correlation is claimed.

Final local ASan/UBSan 34/34 PASS including active negative controls; final
affected corpus ASan 1/1, Windows 1/1 and Linux 1/1 PASS. Windows full 31/31
and retained Linux 31/31 remain valid for unchanged production code; final
same-SHA hosted native full totals remain mandatory. Android final strict
rebuild PASS (151 tasks); runtime/Apple execution still pending hosted CI.
Final logs `build/step6-final-{corpus,sanitize-corpus,windows-corpus,android}.log`
and `build/step6-sanitize-tests.log`. Source canonical HIGH/MID/LOW hash
`0711b34c325fc5ad10029e8cbda326e303758281d15587160928690fbf1fee05`;
reference BE32F hash `779ce293329f4dcb461b800e06af18bc85901034d03416618fee2f0ada5e233f`.

First pushed `13b35d4c58baf78ce49a080bba7ed9e6b5581824` exact Linux job failed
GCC misleading-indentation diagnostics in two checkpoint harness lines (B).
Statements separated; strict warnings and all semantic/race checks unchanged.

No physical Apple hardware/evidence. DH-APPLE-METAL-01 remains DEFERRED; historical
Z Fold7 evidence unchanged and not current measurement. Schema 3 unchanged;
no Phase 5/AI/model work. No unobserved gate is PASS.
