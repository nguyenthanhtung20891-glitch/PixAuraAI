# Phase 2 Step 6: bounded CPU operation evaluation and numerical contracts

Status: READY FOR CI; all applicable local gates PASS, hosted execution pending. Accepted Step 5 baseline: 7f1fa96ab0f24b44e4e28a8c6c0aff8e7a4024ce. Step 7 is not started. No PASS is inferred from wiring or previous-step evidence.

## Repository findings and acceptance criteria

Initially clean, exact accepted HEAD. Audited authoritative product, architecture, photo, security, quality, roadmap and decision documents; relevant ADRs 0003/0008/0009/0010/0011; Step 1-5 reports/contracts; document parser/replay/history/storage and working/decode registry. Exposure/1/1, crop/1/1 and rotate/1/1 are frozen metadata tuples; the other adjustment identities are reserved. Integer-only parameters, immutable complete ordered stacks and linear premultiplied working space are frozen. No numerical tolerance for exposure execution existed. No migration is required and no architectural conflict or new dependency was found.

Measurable acceptance: execute the frozen exposure tuple without modifying historical interpretation; reject all other evaluation tuples; prove finite/alpha safety, exact identity, ordered per-operation rounding, bounded parsing/256 operations/two rasters, atomic output and failure recovery, native/C/JNI/Swift lifecycle, fixed-seed properties/fuzz, actual sanitizer runtime and full existing local/hosted matrix.

## Numerical, parameter and version contracts

Implemented scope: exposure/1/1 and empty-stack/existing identity only. No redundant new operation type. Exact formulas, ranges, tolerance, golden vectors and overflow behavior: [CPU evaluation v1](../contracts/cpu-evaluation-v1.md). [ADR 0012](../adr/0012-bounded-cpu-exposure-evaluation.md) records finite unclamped RGB, 256-operation admission and one-private-output buffering. milli_ev integer [-5000,5000]; exact envelope/keys/versions/IDs; no NaN/Inf, fractional/exponent numbers or implicit clamp. RGB can be any finite binary32; alpha finite [0,1], bit preserved; zero-alpha RGB must be zero. No division by alpha. Overflow at any operation aborts. Non-nearest rounding rejects. Historical exposure meaning remains linear gain 2^(milli_ev/1000).

## Architecture, resource and lifecycle

C++ scalar authority, frozen gain constants and exact ldexp scaling; no platform formulas, mutable registry, SIMD, scheduler or thread pool. Shared bounded document parser supplies versioned records. Validate all operations before pixel execution, copy once into a private destination and execute ordered passes. Maximum 256 operations, 64 KiB request and 1024 object bytes per operation. Existing working image limits and 256 MiB/64-handle context budget remain; reserve destination plus 1 MiB parser allowance. Two simultaneous working rasters per request, zero heap kernel scratch. Context serializes calls; independent contexts execute concurrently; owners join/quiesce before destroy. Evaluated images use existing release/query/copy metadata and stale-token semantics.

Additive family-1 validate/evaluate functions in evaluation.h; ABI 1 remains unchanged. Failure outputs are untouched and only complete images register. SQLite, original asset bytes and historical revisions remain authoritative and immutable; working buffers are derived/transient.

## Coverage and performance

Native tests: all 10001 legal gains versus independent exp2; 2x2 human-readable golden vectors; alpha zero/tiny/partial/one; negative, large finite, subnormal, NaN/Inf pixels; invalid parameters/versions/types; duplicate IDs; max/over-limit stacks; order-sensitive overflow at N; composed/sequential exact equality; 2048 deterministic properties and 2048 parser mutations; real verified asset/decode/normalize/evaluate/query/copy boundary; concurrent same-source evaluations and joined destruction. Allocation-site sweeps extend source/normalization/identity coverage through parser storage, destination and output registration, followed by successful recovery. Kernel scratch injection is not applicable because there is no allocation site. Informational 1024x1024/16-operation benchmark reports wall time and estimated 32 MiB source+destination raster payload; Debug informational baseline on this host: Windows Zig 781.906 ms; Linux Clang 614.197 ms; ASan/UBSan 1637.336 ms. These are single observations, not throughput or device RSS certification.

## Validation, commits and CI

Stable implementation evidence is retained under ignored build/phase-2-step-6. Final commands: repository Windows fallback scripts, WSL repository scripts wrapping CMake/CTest and check-sanitizers.sh with a private tmpfs mount under the repository, strict offline Gradle Debug/Release/lint/JVM/connected tasks, Node source tests, actionlint and bash -n.

| Local gate | Exact observed result |
| --- | --- |
| Windows applicable source | 40/40 PASS, zero failures/skips |
| Linux source | 69/69 PASS, zero failures/skips |
| Windows native fallback | Full manual consumers PASS with directly captured process exit 0; final CMake CTest 12/12 PASS, exit 0 |
| Linux Clang native | 12/12 PASS |
| ASan/UBSan | 14/14 PASS including both actual negative instrumentation probes |
| Step 6 kernel/stack/numerical suite | 77822 checks PASS on Windows/Linux/sanitizers; 10001 legal gains, 2048 property cases, 2048 parser mutations |
| C++ allocation sweep | Windows 130 and Linux/sanitizers 157 failures recovered over seven phases; zero leaked sanitizer allocations |
| Android | Strict offline Debug/Release, lintDebug/lintRelease, Debug/Release JVM and instrumentation APK tasks PASS; final Gradle 152 tasks, 23 executed/129 up-to-date, exit 0 |
| Android JVM XML | Debug 2/2 and Release 2/2, zero failures/errors/skips; unchanged JVM tests reused by Gradle |
| Fresh Android emulator XML | 4/4 PASS, zero failures/errors/skips; final connected execution on rebuilt stable native tree |
| Workflow/shell/hygiene | actionlint 1.7.12 PASS (local shellcheck/pyflakes unavailable, integrations disabled); all scripts/*.sh bash -n PASS; whitespace/source hygiene PASS |
| Apple/MSVC local | Runtime unavailable on this Windows host; Apple wiring checks PASS; actual hosted execution remains mandatory |

The final finite-input check uses binary32 exponent bits, including signaling NaNs, before floating comparisons; signaling-NaN rejection returns 7 without FE_INVALID. All input channels are checked even for identity. Tests also prove immutable historical replay/serialization, wrong-kind/stale handles, non-nearest rounding rejection, 64-handle exhaustion/recovery, ordered overflow through the public C API, simultaneous same-source evaluation, concurrent reads of one evaluated image and independent-context evaluation.

Failures were corrected and rerun, never counted PASS: sandbox WSL/Node child-process restrictions; mixed-type owned test initializer; Windows test-only missing internal decode implementation linkage; inline WSL shell quoting; outer PowerShell stderr/exit capture. Read-only GitHub wrapper inspection independently confirms accepted Step 5 SHA workflows 37214191743 and 37214191726 green; these are baseline evidence only.

Workspace-boundary correction: initial inherited Windows fallback/source regressions used system TEMP for self-cleaning test fixtures. Updated both fallback scripts to save/restore process-local TEMP/TMP and use repository build/tmp; subsequent source invocations also explicitly set repository temporary paths. Final bounded fallback exits 0. No unrelated file cleanup or system configuration change was performed.

Exact owned changes: [28-file inventory](phase-2-step-6-files.txt). No vendor bytes, dependency/toolchain pins, generated builds, caches, credentials, user photos or temporary databases are staged. Commit and Step 6 CI evidence pending publication.

## Limitations and next step

No contrast/saturation registration, geometry execution, tile/cancel/device admission, renderer, GPU, export, editing UI or AI. Existing ICC/non-sRGB conversion deferral remains. Resource ceilings are payload bounds rather than peak physical RSS/latency guarantees; legal finite stacks may fail on overflow. Step 6 awaits required hosted Apple/MSVC/mobile/workflow execution before FULL PASS. Proposed Step 7: separately authorize the next bounded engine milestone, with reviewed geometry/tile/cancellation contracts as needed; no Step 7 work has begun.
