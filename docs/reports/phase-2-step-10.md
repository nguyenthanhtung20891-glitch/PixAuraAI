# Phase 2 Step 10: bounded preview and platform consumption

Status: implementation and validation in progress; hosted closure pending. No Step 11 work.

## Audit and scope

Clean main started at accepted Step 9 HEAD 9c03cf152b1358895dbac995b780426e39eaacf0. Audited product/architecture/photo/privacy/quality/roadmap/decision documents, ADRs 0003/0009/0011-0015, Step 1-9 reports and working/evaluation/geometry/cancellation/reference-preview contracts, context registry/accounting, JNI/Swift and Compose/SwiftUI shells. User authorization supersedes stale Step 3 scope in AGENTS.md. No dependency or document/history/storage authority changes.

Exact conversion remains Step 9; additive bounded-fit uses checked rational-floor dimensions, no upscale, scalar binary64 center-mapped bilinear in premultiplied linear RGBA, followed by unchanged straight-alpha sRGB byte quantization. [Contract](../contracts/bounded-preview-resampling-v1.md) and [ADR 0016](../adr/0016-bounded-bilinear-platform-preview.md) freeze formulas, alpha, rounding, lifetime, memory and platform ownership.

## Ownership, resources and failure

One immutable working source plus one private RGBA8 destination. No float resampling raster, heap plan/weights/scratch or cache. Fully count preview and retained capacities/provenance under unchanged 128 MiB working /8388608 pixels/256 MiB context/64 handles. Existing fixed control overhead remains under ADR 0011; payload is not RSS. Admission rejects with 8 without eviction. Independent handle release and source preservation remain intact. Ordinary calls serialize; cancellation signal/release remain separately synchronized; destruction requires quiescence.

Fit validates all source pixels with checkpoints every 1024 pixels, traverses lazy destination 128x128 row-major tiles and checks every 1024 destination pixels plus admission/pre/post allocation/prepublication. Status 13 never publishes partially. All failure paths preserve caller outputs and existing state. Additive request/render API preserves old ABI/API/format; no new status.

## Platform smoke harness

Android fixture instrumentation runs verified local asset -> decode/normalize/evaluate -> FIT -> JNI explicit ARGB word packing -> real Bitmap, checks PNG golden 0xffbcbcff and dimensions, then recycles app-owned copy. Native context/preview are released before return. JNI uses fixed row stack buffers and VM-owned output, no native heap temporary.

Swift macOS/iOS fixture harness runs the same native flow, copies Data and constructs real CGImage with explicit sRGB/RGBA/straight alpha/stride. PNG fit golden is [188,188,255,255]; provider bytes survive native release. An additional iOS application XCTest bundles the same fixture through deterministic Xcode generation and verifies the fit CGImage after native release. Both adapters bound each copied dimension to 1024, each pixel copy <=4 MiB, operate off UI threads and never change shell navigation/document authority. This is the authorized minimal test harness; no production editor display loop.

## Coverage and diagnostics

Step 9 regressions retained. New checkerboard, 2x2->1x1,3x3->2x2,4x2/2x4, thin/odd edges, half-alpha and transparent premultiplied neighbors; independent four-weight/OETF oracle; 2048 deterministic fit properties and 2048 malformed-request adversaries; no-upscale/exact equivalence/source immutability/repeatability; invalid unsampled pixels; all 89 observed fit cancellation checkpoints; aggregate rejection/output sentinels; mixed exact/fit signal and release races; allocation sweeps extended from 14 to16 phases with successful recovery after each injected failure. C/JNI/Swift consumers strengthened. No assertion weakening or new skips.

Approved source-admission tests: both 4032x3024 targets (1440x1080,1024x768) return status8 from real working normalization layout admission before input read/allocation; preview not started. Required canonical payload=195084288 bytes/12192768 pixels. No oversized temporary or diagnostic bypass. These are rejection tests, not timing benchmarks.

Successful Linux debug timing observations (informational, no device/performance claim):

| Source -> output | Input bytes | Output bytes | Destination tiles | Time ms |
| --- | --- | --- | --- | --- |
|4032x2048 ->1440x731|132120576|4210560|72|430.301|
|4032x2048 ->1024x520|132120576|2129920|40|337.922|
|1920x1080 ->1280x720|33177600|3686400|60|219.454|
|1024x768 ->exact|12582912|3145728|48|85.922|
|4032x2048 ->256x130|132120576|133120|4|266.602|

Times include full input validation; exact timings use existing converter. Logs also record ratios/pixels. Sources are admitted working images under frozen limits.

## Validation and closure

| Local gate | Observed result |
| --- | --- |
| Windows applicable source |47/47, zero skips|
| Linux full source |78/78, zero skips (includes SDK provisioning failure/recovery tests)|
| Windows Zig native fallback |15/15; 894362 preview checks; 450 injected failures recovered/16 phases|
| Linux native |15/15; 894362 preview checks; 588 injected failures recovered/16 phases|
| ASan/UBSan |17/17 including actual negative instrumentation probes|
| Android debug/release |builds PASS on ARM64/ARMv7/x86_64|
| Android lint debug/release |zero issues, warnings-as-errors unchanged|
| Android JVM |4/4 (2 debug+2 release); fresh rerun-tasks, 49 tasks executed|
| Android instrumentation |4/4, zero skips; strengthened Bitmap golden/channel/alpha tests|
| Apple source wiring |deterministic project/source tests PASS; execution requires hosted evidence|
| actionlint / Bash syntax / whitespace |PASS; git diff --check clean|

WSL DrvFS cannot enforce POSIX permission-denial fixtures; rerun uses an ephemeral tmpfs mounted beneath repository build directory and unmounted on exit, preserving assertions. The full source suite initially hit one existing fake-emulator subprocess timeout under concurrent DrvFS I/O; repository-local POSIX temporary storage resolves it without changing timeout, assertions or skips. Initial default sandbox blocked Node worker spawning; authorized execution resolves that environment restriction. Generated logs/caches stay ignored under build/phase-2-step-10.

Windows native regression reproduced an existing concurrent immutable-asset publication race: verifier read-open intermittently returns OS sharing violation32 while MoveFileExW's publication handle is closing. Remediation marks the owned private staging file read-only before publication, reclaims read-only staging files on failure/dedup and retries only immutable asset-open sharing conflicts at most64 attempts with63 one-millisecond waits. Read handles still deny write/delete sharing, and reparse/link/hash/no-overwrite checks remain unchanged. Persistent held-handle conflict rejects with6 and recovers after release. Eight complete Windows storage reruns pass, including32 concurrent dedup rounds each. No process-global lock/cache, permission change or test weakening.

Exact changed-file inventory is [phase-2-step-10-files.txt](phase-2-step-10-files.txt). Commit SHA and hosted IDs will be recorded after validation. Real MSVC/Apple evidence pending; Step 10 is not yet ready for FULL PASS.

Initial implementation SHA d8df83ead888c09d6f3221c4ae36cf80e1d8d341: Foundation run37636514502 observes real MSVC19.51.36260.0 job112844048988 PASS15/15,894360 preview checks,1771 recovered allocation failures/16 phases, including the persistent-sharing conflict scenario. Linux/sanitizer/ARM64/ARMv7/lint jobs pass. Native shells run37636514441 Android job112844049728 fails before compilation because Google SDK Manager downloads an invalid API35 system-image zip (`ZipFile unknown archive`). Remediation introduces at most3 SDK installation attempts for the identical pinned package list; persistent failure still stops CI. Isolated fake SDK tests prove transient recovery and persistent3-attempt failure; no local installed SDK mutation, timeout increase, skip or weakened gate. Apple execution and final exact-SHA closure remain pending.

Remediation SHA2f560214e7568b3f152968f03489e2857df5d663 is fully green: [Foundation37637468018](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37637468018) all7 jobs and [Native shells37637468097](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37637468097) all3 jobs. Real MSVC job112847346586, AppleClang21.0.0.21000101 job112847346225 (15/15 native,7/7 macSwift,7/7 iOS simulator Swift), Android job112847346893 (4/4 instrumentation, builds/lint/JVM), iOS job112847346964 (Debug/Release,5/5 app including preview smoke,1/1 UI). GitHub reports macOS runner capacity delays; no timeout change.

Final strengthened tests draw opaque goldens through real CoreGraphics, prove interpolation occurs before display clipping and reject hidden RGB at zero alpha even in unsampled source pixels. A subsequent required sanitizer rerun reproduced Linux's transient two-link dedup publication window. POSIX metadata admission now retains the pinned descriptor, retries exactly-two-link states at most63 times with one-millisecond sleeps and still requires one link before any read and after verification. Persistent aliases reject with6 and recover after explicit unlink; the security condition is unchanged. Linux native15/15 and sanitizer17/17 pass after remediation. Final exact-SHA hosted closure is required for these changes.

## Limitations and next authorization

Reference SDR only; bilinear can alias at strong reductions. No GPU, cache, ICC/P3/HDR/tone mapping, export, editing resize operation, production UI or asynchronous scheduler. Platform copies are one-shot app-owned buffers, not native context payload; system graphics allocation failures remain platform failures. No architecture deviation beyond explicitly authorized preview extension. Proposed Step 11: separately define application preview request/publication lifecycle and cancellation ownership for interactive consumption. Not started.
