# Phase 4 Step 2 — Budget Admission & Execution Tiers

Status: **COMPLETE / FULL PASS** (technical gates; ready for Product Owner acceptance).
Certified implementation SHA: `cf2026e9e712ddd9e1945ab5e97f9eb7f50c2cc9`.
Baseline: `82fca1f6fe3a565a6704afe18cb5bc80cb9a9823`.
Product Owner freezes the seven-step Phase 4 roadmap and accepts Step 1 FULL PASS.
Only Step 2 is authorized; Steps 3–7 remain NOT STARTED. Phase 0–3 stay closed.

## Delivery

The [budget v1 contract](../contracts/budget-admission-v1.md) freezes shared
allocation-free, immutable-snapshot preflight with versioned request/result and
stable reason codes. No reservation, context access, duplicated counters,
publication owner, worker queue, policy persistence or real platform probe.
Existing context owners remain responsible for truthful occupancy, serialized
allocation and ALL operation-specific checks. No architecture conflict/new ADR.

T0: CPU/manual baseline, 128-MiB policy cap/720 preview edge. T1: at least
4 CPUs, 4 GiB physical RAM and 256 MiB process observation, NORMAL thermal and
pressure, unconstrained; 192 MiB/1280 edge. T2: 8 CPUs, 8 GiB, 512 MiB;
256 MiB/2048 edge. Concurrency stays one. Effective cap additionally minimizes
known RAM/16, process observation minus 32-MiB reserve, current context limit,
and 64 MiB for severe pressure/thermal. Unknown selection inputs cannot promote.
These constants are conservative policy, not physical-device measurements.

Synthetic fixtures LOW/MID/HIGH produce T0/96 MiB, T1/192 MiB, T2/256 MiB.
The same 64×64 two-raster CPU request admits all, peak exactly 1179648 bytes.
HIGH does not admit work over existing limits. Caps and operation-specific native
checks remain absolute authorities; no storage/decode/operation/preset/gesture/
scratch/context/handle/generation ceiling increase.

Work classes: IMPORT/decode, PREVIEW, COMMIT/reference render, optional bounded
GPU_DIAGNOSTIC. Checked 64-bit pixels/BPP/count/scratch/temporary/occupancy/encoded
accounting includes retained and in-flight capacities and 1-MiB control reserve.
Overflow rejects. Preview may fit/halve only RGBA8 display dimensions, with all
full-size working buffers still charged; no source/commit/parameter downgrade.
GPU is considered only for the diagnostic class; available AND runtime-validated
or reviewed physical evidence AND normal state, otherwise explicit CPU fallback
or rejection. Metal availability/runtime validation never claims physical
certification. No validated enum bypasses existing backend/parity checks.

## Tests and evidence

Shared C/C++/JNI corpus exercises fixture outcomes, every CPU/RAM/process tier
threshold minus-one/exact/plus-one, unknowns, contradictory/versioned profiles,
malformed request fields/version/size, multiplication/addition overflow, explicit
hard maxima, reduced context limit, byte/handle/work occupancy, exact-fit/one-byte
exhaustion, aspect ratios/one-pixel fit, preview memory fallback and full-source
failure, GPU-unavailable/unvalidated/Vulkan evidence representation/virtual Metal.
Tests use physical-provenance enum values solely to verify evidence representation;
they are NOT physical measurements or certification of the executing device.
Native stress checks 160000 parallel queries across eight independent callers,
comparing identical complete result records. No mutable state/reservation.
Swift macOS/iOS simulator tests assert independent expected integer outputs.
Android instrumentation invokes the argument-free SAME shared corpus through JNI.
No untrusted-input production injection, new UI or actual OS probe collection.

Local affected C/C++ tests were rebuilt and observed 2/2 PASS under ASan/UBSan.
An initial missing `<cstring>` test include caused compilation failure; it was
corrected and rebuilt before counting tests. No stale binary PASS is accepted.
Local Windows native regression passed 30/30; final affected admission tests
passed 2/2 with 160000 parallel queries. Local source/docs passed 86/86 (zero
skips), workflow lint/diff checks passed. Android strict debug/release/JVM/lint/
instrumentation-APK builds passed all requested tasks, including the final reason
correction rebuild. Real runtime instrumentation and Apple gates require hosted
execution and are not inferred from local builds.

Full local WSL ASan/UBSan passed 31/33, with active negative controls. The two
unchanged storage cases failed on `/mnt/g`'s 9p/DrvFS mount without POSIX metadata:
unwritable-directory rejection in the unprivileged child and the simulated
Android publication branch. No storage/security code or assertion is changed;
these failures are not counted PASS. Final rebuilt admission corpus passed 2/2
under ASan/UBSan. Hosted Linux must pass both unchanged filesystem cases and
all 33 sanitizer tests on its native filesystem before delivery.
The historical local filesystem failures above remain failures; final exact-SHA
hosted Linux and sanitizer execution passed those cases on a native filesystem.
Implementation SHA `0606c4ee934ec89243dabfc1e96ed1fe6ed8073e` passed
Foundation 38162938140 (7/7), source 122/122 and Android runtime 18/18. Shells
38162938172 failed only its existing iOS UI rotation readiness predicate;
debug/release builds, six app tests and Swift 18/18 per runtime passed. The
captured hierarchy shows an enabled on-screen button with the horizontal
scrollbar accessibility region overlapping its default hit point. The coordinate
tap helper now checks enabled/existing plus its actual upper target on screen,
rather than gating that different coordinate on XCTest's default `isHittable`.
Actual tap, mandatory core-backed selection and every full editor/history/reopen
assertion remain. No production UI/domain code changes; no failed run is PASS.
Remediation SHA `bf2b1dfafcb9a16da44f8ab2362849fef5ef55d5` again passed
Foundation 38164350001 (7/7), source and Android. Shells 38164349992 exposed the
same centre-tap issue in category navigation: Geometry remained selected after
the Tone/Color tap, so exposure was absent. Category and tool navigation now use
the same upper-target helper and mandatory selected postcondition. Category
selection is explicitly observed before native tool selection; no direct test
controller action, skipped flow, increased timeout or production UI change.
SHA `b39f26ab2b1b4f169aae4cb271609c4435c6a963` passed Foundation 38165759181
(7/7) and source/Android, but shells 38165759209 failed readiness after Cancel.
The unchanged revision-count label was insufficient cancellation completion
evidence. The UI harness now waits for terminal gesture controls and absence of
busy state after cancel/apply, and resolves current controls on every predicate
poll instead of retaining an element across rebuilt views. All 20-second
deadlines, real taps, selected postconditions and complete flows remain; no
domain/UI production change. Implementation exact-SHA certification is observed below.
Final delivery requires all seven Foundation and three Native application shell
jobs SUCCESS on the same final SHA, including Android runtime and iOS app/UI.
The delivery report supplies final SHA, workflow/job IDs and observed totals.

## Implementation exact-SHA certification observed 2026-10-12

Both completed workflows certify `cf2026e9e712ddd9e1945ab5e97f9eb7f50c2cc9`:
[Foundation 38167080174](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/38167080174)
and [Native application shells 38167080150](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/38167080150).
All 10 jobs SUCCESS; zero failed, running or infrastructure-failed jobs.

| Workflow | Job | Job ID | Classification |
| --- | --- | --- | --- |
| Foundation | workflow-lint | 114551758344 | SUCCESS |
| Foundation | apple-boundary | 114551758431 | SUCCESS |
| Foundation | sanitizer | 114551758446 | SUCCESS |
| Foundation | portable (ubuntu-24.04) | 114551758496 | SUCCESS |
| Foundation | android-boundary (armeabi-v7a) | 114551758504 | SUCCESS |
| Foundation | portable (windows-2025) | 114551758524 | SUCCESS |
| Foundation | android-boundary (arm64-v8a) | 114551758689 | SUCCESS |
| Shells | android-app | 114551758202 | SUCCESS |
| Shells | source | 114551758301 | SUCCESS |
| Shells | ios-app | 114551758390 | SUCCESS |

Observed exact-job logs: source/documentation 122/122, zero skipped; portable
source checks 80/80 on each host; Windows native 30/30; Linux native 31/31;
Apple native 30/30; ASan/UBSan 33/33 with active negative controls. Workflow
lint and both Android ABI builds passed. Android debug/release builds, both
JVM test tasks (4/4 debug and 4/4 release, zero failures/ignored in downloaded
exact-run reports), both lint tasks and instrumentation APK passed; runtime JNI/Compose
instrumentation completed 18/18. Swift macOS and iOS simulator passed 18/18 each;
iOS debug/release app builds, app tests 6/6 and UI tests 2/2 passed, zero failures.
The Linux-only numerical-fixture step is intentionally inapplicable on Windows;
its actual Linux execution passed, rather than treating the skipped step as PASS.

Native logs confirm 160000 identical parallel admission queries across eight
callers. Shared C/C++/JNI boundary corpus and independently expected Swift results
passed. HIGH oversize, hard-limit preservation, threshold +/-1, checked overflow,
unknown facts, occupancy and static fallback cases all passed their containing
gates. These are synthetic policy and platform regression evidence, not physical
device profiling or Apple Metal certification. Schema 3 and document semantics
remain unchanged; admission is not persisted into history.

Final iOS remediation is category B (test harness readiness/input observation).
Cancellation completion requires absence of Editor busy plus an existing disabled
Cancel control before querying rebuilt controls. Fresh predicate queries, actual
visible coordinate taps and mandatory selected/revision/history/reopen assertions
remain. No arbitrary sleeps, production timeout changes or core-error masking.
No category A defect or category C runner failure requires further remediation.
The macOS capacity annotation did not prevent successful execution.

HEAD, cached origin/main and remotely queried main matched the certified SHA;
initial working tree was clean and git diff --check passed. Documentation closure
changes only the report, ROADMAP.md, QUALITY_GATES.md and contract status. The
Product Owner authorizes a documentation-only commit and push; both complete
workflows must independently pass all ten jobs on that closure SHA before final
delivery. Implementation-SHA evidence above does not substitute for closure-SHA
execution. The final delivery records the observed closure SHA and workflow IDs.
Local documentation checks after these edits passed 16/16, zero skipped, using
Node's --test-isolation=none. The initial default-isolation invocation failed to
spawn three test workers with sandbox EPERM before executing tests; that local
environment failure is not a product failure or a PASS. The in-process rerun
executed all checks successfully. Final git diff --check passed.
Closure pre-commit Windows source/documentation checks passed 86/86, zero skipped,
with repository-local temporary storage. An initial attempt to run the Linux CI
source suite on Windows passed 86/122 and failed its 36 Linux-only fake-SDK/Bash
orchestration tests; those failures are not counted PASS. The applicable Windows
suite excludes that Linux-only test file; the complete 122-test suite remains
mandatory in the closure SHA's hosted Linux source job. No tests are changed.
Frozen roadmap structure, DH-APPLE-METAL-01 and all later-step boundaries remain.
Step 2 is ready for acceptance. Step 3 has NOT started; stop here.

## Scope and review boundary

Files: budget.h/budget.cpp; shared budget boundary corpus; existing C/C++ test
consumers/concurrency; JNI diagnostic/CoreProbe and instrumentation; Swift budget
tests and Package source wiring; CMake; capability architecture checks; budget/
capability contracts, frozen roadmap and current ROADMAP/QUALITY_GATES/ARCHITECTURE/
DECISIONS/report entries. Historical Phase 3 closure and ADR 0021 remain unchanged.

A: finish Step 2 technical gates and exact-SHA closure; no known architecture conflict.
B: frozen Steps 3–7 after explicit authorization; DH-APPLE-METAL-01 at its existing
mandatory later hardening/Beta/production/physical-claim gates.
C: optional dashboards, device scores, benchmarks and product polish.
No schema change, Phase 5/inference/AI/model, network/telemetry, new manual tool,
preset catalog, numerical/UI feature or GPU-kernel work. DH-APPLE-METAL-01 remains
unchanged/deferred. STOP after Step 2; do not begin Step 3 automatically.
