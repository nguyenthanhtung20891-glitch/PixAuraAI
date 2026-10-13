# Phase 4 Step 6 — Cross-tier replay & certification

Status: COMPLETE / FULL PASS. Certified implementation SHA:
`417f127d6018448910886d1760e1f8122bcecc11`.
Authorized baseline HEAD and origin/main
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

## Exact-SHA certification

Both complete workflows observed SUCCESS on SAME implementation SHA:

| Workflow | Run ID | Jobs |
| --- | --- | --- |
| Foundation boundaries | 38414896874 | 7/7 SUCCESS |
| Native application shells | 38414897742 | 3/3 SUCCESS |

All individual jobs SUCCESS: source, workflow-lint, Windows portable, Linux
portable, sanitizer, Apple boundary, Android arm64-v8a, Android armeabi-v7a,
Android application and iOS application. No required skipped/unobserved PASS.
Any documentation closure must ALSO pass both complete workflows on its own SHA;
final delivery records that final accepted SHA and its exact workflow IDs.

| Gate | Observed PASS |
| --- | --- |
| Hosted source/documentation | 126/126, zero skips |
| Portable source per native host | 84/84, zero skips |
| Windows native | 31/31 |
| Linux native | 32/32 |
| ASan/UBSan including active negative controls | 34/34 |
| Apple native | 31/31 |
| Swift macOS / iOS Simulator | 25/25 each |
| iOS debug/release | unsigned builds PASS |
| iOS app / UI | 7/7 / 2/2 |
| Android ABI boundaries | arm64-v8a and armeabi-v7a PASS |
| Android debug/release/JVM/lint | builds/lint PASS; JVM 7/7 each variant |
| Android instrumentation | 27/27, zero failures/errors/skips |
| Workflow lint/shell syntax/diff check | PASS |

All retained Phase 0–3, persistence/reopen and Phase 4 Step 1–5 gates execute.
Logs `build/step6-current-{foundation,shells}-view.txt`,
`build/step6-current-{source,windows,linux,sanitizer,apple,android,ios}-evidence.log`;
Android XML/HTML and per-case logcat artifact
`build/gh-artifacts-38414897742/android-shell-evidence`. Native workflow artifacts
preserve cross-tier canonical, redo, reference SHA and matrix. Ignored artifacts
are not committed; bounded fixture source/golden/provenance files are curated tests.

## Bounded certification matrix

Every row below is SYNTHETIC POLICY EVIDENCE. Native hosts supply HOSTED RUNTIME
EVIDENCE; Android emulator and iOS Simulator supply INTEGRATION EVIDENCE only.
Fixture facts LOW/MID/HIGH: CPU 2/4/8, RAM 2/4/8 GiB, budget 128/256/1024 MiB.
All normal tier/concurrency/cap/edge results come from shared core, not a device
table. Source match and canonical comparison are exact; reference uses full CPU
RGBA32F words with explicit endian encoding. FIT display dimensions are separate.

| Case | Tier/cap/edge | Pressure/backend/trust | Reopen/history | Source/canonical/reference |
| --- | --- | --- | --- | --- |
| HIGH create, seven representative edits | T2 / 256 MiB / 2048 | NORMAL, CPU, unavailable GPU | 8 revisions | PASS / PASS / PASS |
| Same project MID | T1 / 192 MiB / 1280 | NORMAL, CPU, unavailable GPU | exact 8 revisions, redo preserved | PASS / PASS / PASS |
| Same project LOW | T0 / 96 MiB / 720 | NORMAL, CPU, unavailable GPU | exact 8 revisions, redo preserved | PASS / PASS / PASS |
| Peer canonical artifact imported LOW | T0 / 96 MiB / 720 | NORMAL, CPU | same IDs/ordered stacks; redo replay | PASS / PASS / PASS |
| LOW create → HIGH | T0 → T2, bounded baseline | NORMAL, CPU | exact 4 revisions; full T0 flow | PASS / PASS / PASS |
| Six ordered pairs → richer tier | T0 → T2 | NORMAL, CPU | each exact 3-revision graph | PASS / PASS / PASS |
| HIGH severe → recovery → LOW reopen | T2 → T0 ≤64 MiB → T2; fresh LOW | SEVERE then three NORMAL | changed commit 1; pressure 0 | PASS / PASS / PASS |
| CRITICAL fresh reopen | T0 ≤32 MiB / 360 | CRITICAL, CPU | REJECT status 8; envelope/history unchanged | PASS / PASS / not rendered |
| Recovered fresh LOW after rejection | T0 / 96 MiB / 720 | NORMAL, CPU | exact prior revisions; epoch one | PASS / PASS / PASS |
| Synthetic Vulkan validated-state representation | T2, optional backend preference | available/runtime-validated, then loss → CPU | exact same document | PASS / PASS / PASS |
| Vulkan unvalidated or unavailable | T2, CPU | no trust escalation | exact same document | PASS / PASS / PASS |
| Metal available/uncertified | T2, CPU | validation zero | exact same document | PASS / PASS / PASS |
| Explicit historical schema progression | T0 / 96 MiB / 720 | CPU | schema 1/2 open rejection in editor; explicit 1→2→3 | PASS / PASS / PASS |

Runtime TSV has 21 bounded records (≤64, ≤32 KiB) with CPU/memory/pressure,
availability/validation, actual cap/edge, result, count and hashes. Synthetic
Vulkan validation is a trust-state test fixture, NOT physical Z Fold7 scope
correlation or new certification. Production trust rules remain unchanged.

Primary full canonical hash:
`0711b34c325fc5ad10029e8cbda326e303758281d15587160928690fbf1fee05`.
Redo checkpoint hash:
`1177cd3f041821b0fb786274f3c028b88754ce880397b38692e695418d92af77`.
Immutable source hash:
`410e778774cb0ef8e168dfc8aa10c6ad4e0683eede5cd2f4be2aa7590508f3c5`.
Full CPU reference hash:
`779ce293329f4dcb461b800e06af18bc85901034d03416618fee2f0ada5e233f`.
Android per-case logcat matches HIGH/MID/LOW/peer hashes above; Apple/native logs
match the same frozen canonical artifacts and reference, including redo checkpoint.

## Semantic, race, resource and storage conclusions

Representative crop, rotate, exposure, contrast, saturation, blur and sharpen
remain /1/1 with exact integer parameters. Crop→rotate versus rotate→crop,
exposure→contrast versus reverse, and blur/sharpen ordering stay exact. Geometry
and tone reversed pairs also produce distinct reference results. No folding,
reordering, new tool, preset content, model or device-specific semantics.

Changed commit adds exactly one revision; neutral/cancel zero. Multilevel
undo/redo and redo-preserving reopen pass. Branch-after-undo retains prior
graph, clears old redo, and reopens/redo-replays the new branch correctly.
Successful session acquisition intentionally refreshes durable ownership envelope;
session-free document identity remains exact. Runtime policy is never persisted.

1024×512 FIT: HIGH displays 1024×512, LOW 720×360, critical 360×180. Interactive
EXACT is full exact output or status 8; never converted to FIT. Full reference
EXACT remains full-size where actual budget admits. Source is never rewritten
or replaced by preview. Critical rejected reopen creates no partial revision,
source/storage mutation or envelope advance; recovered fresh policy succeeds.

Actual old native render pauses at admission; severe downgrade/newer render
finishes first; old finishes last with status 13 and untouched output sentinels.
Only newest eligible request remains current. Commit persists intended operations,
not display completion order. The same fence survives close/LOW replay. No new
publication owner/generation. Two contexts with different tiers remain independent.
16 pressure/commit/cancel races, checkpoint-controlled stop/join/destruction and
12 fresh HIGH/MID/LOW reopen cycles pass. No UAF, deadlock, double commit or stale
publication observed; ASan/UBSan and active controls PASS. Test state and artifacts
bounded; temporary projects/resources reclaimed. New test controls are debug/test
only, absent from Android release symbols and Swift release product dependency.

Malformed profile/version/trust and existing overflow/hard-boundary corpora remain
active. HIGH cannot exceed 256 MiB context or any raster/scratch/handle/storage
limit; dynamic policy only lowers. Historical schemas 1/2 exact fingerprints and
payloads, explicit migrations and no automatic migration remain certified;
Schema 3 unchanged, no Schema 4. Original immutable assets verify at every tier.

A: proven rejected-reopen durable-epoch mutation fixed minimally with preparation
before acquisition and expected-epoch CAS. B: duplicate SQLite inspector lifetime,
private Windows parser linkage, exact canonical newline and strict GCC indentation
harness corrections. Assertions, sanitizers, resource caps and semantics retained.
C: no infrastructure retry required. No architecture conflict/new ADR.

No physical Android device connected; no current measurement or expanded Z Fold7
scope claim. No physical Apple hardware/certification. Simulator/hosted results
are not physical device evidence. DH-APPLE-METAL-01 remains DEFERRED/unchanged.
No Phase 5/AI/model work. Phase 4 Step 7 NOT STARTED. STOP after Step 6.

No physical Apple hardware/evidence. DH-APPLE-METAL-01 remains DEFERRED; historical
Z Fold7 evidence unchanged and not current measurement. Schema 3 unchanged;
no Phase 5/AI/model work. No unobserved gate is PASS.
