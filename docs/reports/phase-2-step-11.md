# Phase 2 Step 11: interactive preview ownership

Status: implementation and local/hosted certification PASS on functional HEAD 1e9170cdba3a8efb580326be08c833862588f1af. Documentation-only certification publication requires final exact-SHA workflow closure, reported in the final delivery. Step 12 not started.

## Audit and acceptance

Started clean main at accepted Step 10 HEAD 1b451b7e54632040a7df2e257039b78c9f5ac913. Audited product/architecture/photo/privacy/quality/roadmap/decisions, ownership and processing ADRs, Step 1-10 evidence, document session generations, working/evaluation/geometry/cancellation/reference/fit contracts and C/JNI/Swift registry/boundaries. No frozen conflict: preview generation is transient and separate from document authority. ADR 0017 and interactive-preview-lifecycle-v1 freeze measurable race/ownership acceptance. Existing pixel formulas, source immutability, ABI 1 and all ceilings remain unchanged.

## Implementation

Context-scoped uint64 generation with nonwrap exhaustion; latest wins under the existing cancellation fence. Fixed/no-heap request state, stack cancellation view, one native render per ticket and producing-generation binding. Existing resource admission and ordinary mutex protect source lifetime. Stop/join precedes existing quiescent destroy.

Android private direct context storage and Swift stable context owner serialize begin/cancel/install under a short platform lock while synchronous render/copy runs outside. Requested and displayed generations differ. Failure/cancel preserves last valid Bitmap/CGImage; copies survive native release/close. No scheduler, production editor, cache, dependencies or persistence.

## Validation

Local source: Windows 48/48 and Linux 79/79, zero test skips. Linux native 15/15; actual ASan/UBSan 17/17 including negative instrumentation probes. Final Windows Zig native fallback 15/15; Android Debug/Release builds for ARM64/ARMv7/x86_64, lint zero issues, JVM 4/4 and emulator instrumentation 4/4 (zero skipped) PASS. actionlint, Bash syntax, Apple source wiring and git diff --check PASS. Real hosted MSVC/Apple execution and exact-SHA functional closure PASS as recorded below.

Linux diagnostics: begin+eligibility+cancel 100 calls 13.096 us, 1000 calls 121.428 us; 10000 churn 34.299 ms, 2000 successes, 2048 malformed identities, 256 publication races, 90 checkpoint phases. Informational desktop debug observations, not production frame latency. Final Windows preview checks=949689, Linux=949692; totals vary with legal race outcomes, while all 256 races execute. Windows diagnostics: 100 control calls 30.900 us, 1000 calls 284.500 us, churn 73.378 ms. Allocation sweep recovers 453 failures/17 phases on Windows and 591/17 on Linux; fixed begin/cancel/stop succeed even with fail-at-zero. No request metadata allocation exists.

Initial test-only larger fixture inherited a deliberately reduced budget and rejected with 8; restore defaults for that fixture before checkpoint tests. Source-wiring phase expectation extended 16->17. Initial concurrent local Android toolchain failed committing JVM metaspace; crash log reported only 139 MiB available pagefile despite physical memory available. The UTP launcher subsequently failed before tests while committing its default 512 MiB startup heap with only 338 MiB available pagefile. Bounded helper-JVM startup options, 1 GiB local emulator runtime RAM, verified task-owned orphan cleanup and isolated ADB resolve the host constraint. SDK/AVD configuration files and hosted workflows remain unchanged; no OS/pagefile change, timeout increase, test skip or product ceiling change. All final Android gates execute successfully. Restart interrupted final native/source commands; surviving processes/mounts were checked before rerun, preserving the existing implementation.

Exact file inventory: [phase-2-step-11-files.txt](phase-2-step-11-files.txt). 23 files, with no generated artifacts or credentials. Implementation commit: 1e9170cdba3a8efb580326be08c833862588f1af, feat(preview): fence interactive generations and platform publication. Final documentation commit/run identifiers are supplied in the delivery response to avoid a self-referential report commit. Tests include deterministic 10000 churn, supersession/cancel at conversion checkpoints, publication races, source-release/stop, stale/fabricated/cross-context/kind identities, duplicate execution, overflow and independent C/JNI/Swift ownership. Allocation failure sweep adds interactive render and fresh-generation recovery. Existing preview/numerical/security regressions remain mandatory.

## API, bounds and reproducibility

Additive preview.h functions: begin(context,version,ticket), render_interactive(context,working,ticket,request,output), current(context,ticket,optional result), cancel(context,ticket), stop(context). Ticket v1 is 56 bytes with stable kind/context/generation. Old APIs, ABI 1, feature bits and exact/fit pixel conversion remain unchanged. No new status, schema, operation or dependency.

Native lifecycle fields are bounded <=32 bytes excluding existing registry overhead; each preview entry adds one uint64 generation. No heap request state/history. Existing 128 MiB working raster, 8388608 pixels, 256 MiB context payload and 64 combined handles are preserved. Retained capacities/provenance continue fully accounted; fixed control overhead follows ADR 0011. Both app owners bound active render/copy calls to two and retain one requested generation plus one displayed candidate, with <=1024 dimensions/4 MiB per pixel copy. Identity values contain no owner pointer and do not retain native context lifetime.

Native publication checks generation/cancel/stop and inserts under the cancellation mutex while ordinary locking preserves accounting/source access. Source release may win before admission (3) or wait until completion; stop may win before publication (13) or revoke an already completed result's delayed installation eligibility. Destruction follows stop and join, preserving ADR 0009 rather than permitting racing free. Current request failure never changes the last displayed object. Fit checkpoints remain every 1024 pixels/128x128 tiles; exact checkpoints remain <=16384 conversions, with bounded non-preemptible allocation/lock phases and no latency promise.

Executed commands: Windows node --test over all applicable source files; scripts/check-decode-zig.ps1 with repository Zig 0.14.1; WSL full node --test tests/*.test.mjs, CMake build/CTest, scripts/check-sanitizers.sh (Clang ASan/UBSan); Android Gradle offline/strict verification assembleDebug/assembleRelease/lintDebug/lintRelease/testDebugUnitTest/testReleaseUnitTest/assembleDebugAndroidTest/connectedDebugAndroidTest; actionlint; Bash -n over scripts; git diff --check. Test temporary storage/caches/logs remain ignored under build/phase-2-step-11; WSL permission-sensitive tests use repository-mounted ephemeral tmpfs without assertion changes.

## Hosted certification

Both completed/success workflow JSON records identify exactly 1e9170cdba3a8efb580326be08c833862588f1af on main; every required job is green:

- [Foundation boundaries 37655437962](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37655437962): 7/7 jobs, including Windows, Linux, sanitizers, Apple, workflow lint and both Android native ABIs.
- [Native application shells 37655438091](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37655438091): 3/3 jobs, source, Android application and iOS application.
- Real MSVC 19.51.36260.0, job 112909240025: native 15/15; preview checks 949698; 10000 churn/2000 successes/2048 malformed tickets/256 publication races/90 checkpoint phases; 1775 recovered allocation failures/17 phases.
- Real AppleClang 21.0.0.21000101, job 112909240156: native 15/15, macOS Swift 7/7, iOS simulator Swift 7/7, unsigned iOS build. The strengthened shared admission/ownership test executes interactive CGImage lifecycle and post-close data checks.
- Real Android application job 112909241552: Debug/Release, lint/JVM and 4/4 emulator instrumentation pass, including interactive Bitmap generation/display/cancel/failure/close ownership.
- Real iOS application job 112909241408: Debug/Release builds, 5/5 application tests and 1/1 UI test; its package/core gates also execute the interactive Swift boundary tests.

No hosted remediation, architecture deviation, unresolved blocker, weakened assertion, new test skip, timeout increase, ceiling increase or dependency change. Functional certification resumes from the existing tree and all new primitives remain synchronous. Documentation-only publication must receive the same final exact-SHA green closure before delivery. Working tree was clean after implementation commit/push; final status is checked again at delivery.

## Limitations and next authorization

Synchronous off-UI-thread primitives/harness only; applications must not bypass the publication owner. No hard real-time cancellation, background render scheduler or production UI. Resource pressure rejects without eviction under frozen ceilings. Proposed Step 12: separately authorize application scheduling/coalescing and UI lifecycle integration on these primitives; not started.
