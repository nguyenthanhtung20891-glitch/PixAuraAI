# Phase 2 Step 5: orientation/color normalization and initial CPU evaluation

Status: READY FOR CI; implementation and all applicable local gates PASS; hosted execution pending. Accepted Step 4 baseline d26baf21be2dde5f0c7881e8007658c859572cbe. Step 6 is not started.

## 1. Repository findings

Audited required product/architecture/photo/security/quality/roadmap/decision documents, ADRs 0003/0009/0010/0011, Phase 2 Step 1-4 reports/contracts, current native decode/asset registry and platform boundaries. Linear-sRGB float CPU authority, premultiplied alpha, immutable originals, explicit-context quiescent destruction and Step 4 invalid-orientation rejection are frozen. Normalization was absent. The new contract refines those decisions without superseding historical ADRs. No new dependency or unresolved architecture conflict.

## 2. Orientation contract

All EXIF 1-8 normalize exactly into canonical top-left orientation 1, with dimensions swapped for 5-8. Encoded dimensions/orientation remain queryable separately. Missing defaults to 1 at admission; explicit invalid/conflicting orientation rejects per frozen Step 4 contract. Original bytes never mutate. Exact asymmetric golden mappings, real JPEG/PNG input and checked transformed layouts cover each value.

## 3. Color normalization boundary

No-profile is explicitly assumed sRGB; explicit PNG sRGB is sRGB-like; embedded ICC is profile-present and returns unsupported 16. PNG gAMA/cHRM without explicit sRGB defers, even nominal gamma 45455; contradictory explicit sRGB annotations defer. Original/profile bytes/digest remain retained and queryable. Fixed binary32-rounded lookup tables perform the standard inverse sRGB transfer, followed by linear alpha premultiplication. No runtime libm/dependency, profile conversion, wide gamut or HDR. Table error <3e-8 and normalized component error <6e-8 versus double reference; permutations and identity preserve float bits. Contract: [cpu-working-image-v1](../contracts/cpu-working-image-v1.md).

## 4. Canonical CPU model

Unique owned tightly packed native IEEE RGBA32F linear-sRGB/D65 premultiplied buffer; explicit width/height/byte stride/byte size/orientation/format. Working image retains immutable source provenance after source/decoded release. RAII destruction, no global mutable cache/platform image/GPU object. Alpha zero discards hidden RGB in the derived working image while original data remains immutable.

## 5. CPU evaluation foundation

Identity evaluation validates canonical layout and constructs an independent bit-identical working image. Deterministic bounded traversal serves normalization; no operation stack execution, editing kernel, renderer, resize/crop/composite/export/AI. SQLite/history are untouched and all pixels remain transient.

## 6. Limits

Reducible hard request ceilings: width/height 16384, 8388608 pixels, row 262144 bytes, image 128 MiB. Checked width*height, width*16 and stride*height before allocation, including transposes and size_t capacity. Existing context payload/reservations ceiling 256 MiB and 64 total source/decoded/working handles remain. No heap transform scratch or intermediate raster; one output allocation, constant stack loop state, 2048 read-only table bytes. Fixed object/allocator overhead is additional, not a peak-RSS guarantee.

## 7. C ABI and concurrency

Additive independently versioned working.h family: limits, normalize, identity, metadata query and caller-owned bounded float copy. Reuses decode context/handle lifetime and release/profile query. No codec/C++/platform objects or internal pointers exposed. Exceptions contained; every failure preserves outputs and publishes no partial handle. Kind/context/stale token checks remain. Calls serialize per context; independent contexts run concurrently; duplicate requests own independent buffers. Owners join/quiesce operations before destruction, preserving ADR 0009.

## 8. Security/adversarial coverage

Invalid orientation/layout/zero/extreme dimensions, transpose/stride/product overflow, request bounds, missing/truncated/oversized decoded buffers, profile inconsistency/unsupported transforms, wrong-kind/stale handle/copy range, ownership retention, repeat failure/recovery. Existing hostile encoded corpus/storage protections run unchanged. C++ allocation-site sweep now has five phases including normalization and identity; failure sentinels/recovery tested. Concurrent normalized reads, duplicate requests, independent PNG/JPEG contexts and joined destruction are exercised.

## 9. Property/fuzz coverage

Fixed seed 0x50495835: 1024 small randomized dimension/orientation/permutation cases with pixel count/bijection and canonical identity invariants. Every 8-bit transfer/alpha code independently checked in C++ and Node; exact 2x3 golden mappings all eight orientations. Outer test timeout 60 seconds; no unbounded fuzz run. Step 4 deterministic 4096 mutations/1024 random cases remain, instrumented alongside normalization.

## 10. Files changed

Exact paths: [file inventory](phase-2-step-5-files.txt) (26 owned files). Core working header/implementation/tables; decode internal color hints and context accounting; native/C/allocation tests; CMake/Swift wiring; native JNI and Swift tests; source checks/workflow source wiring; current photo/quality/roadmap/decision documentation; working contract and this report. No vendor/toolchain/dependency bytes changed.

## 11. Commits

Pending focused implementation/validation commit through scripts/codex-git.ps1. No Git history rewrite.

## 12. Local validation

Targeted Windows native 4/4 PASS; working suite 187265 assertions, deterministic properties 1024; targeted ASan/UBSan 4/4 PASS; source working tests 2/2 PASS. Full stable-tree Windows source 38/38, Linux source 67/67, Windows native fallback CTest 11/11, Linux Clang native CTest 11/11, ASan/UBSan 13/13 (including actual negative probes), actionlint 1.7.12 and shell syntax/source hygiene PASS. Native final working suite 187265 checks; decode suite 10495; C++ allocation sweep recovers Windows fallback 41/Linux 20 sites over five phases. Apple wiring is verified but real Apple/MSVC runtime remains hosted-only. Android Debug/Release, lintDebug/lintRelease, JVM tests and instrumentation APK tasks completed on the stable tree (147 tasks, 23 executed/124 up-to-date in the build attempt); unchanged JVM XML 2/2 reused, zero failures/skips. Fresh connected instrumentation PASS 4/4 with zero errors/failures/skips (73 tasks, 7 executed/66 up-to-date). Full builds were not repeated after the infrastructure fixes. Initial owned misleading-indentation warning fixed; sandbox Node child-spawn denial retried with authorized access; Windows-to-WSL inline regex quoting was corrected using a repository script. The full Windows fallback executes all existing manual consumers and CTest successfully; nested PowerShell stderr wrapping reports exit 1 from injected-allocation diagnostics, so direct CTest was independently rerun and returned exit 0. Android first attempt failed: JBR native malloc during C2 compilation, not a test failure; local-only retry uses smaller heap/processor count and TieredStopAtLevel=1/HeapBaseMinAddress. Crash evidence moved into build/phase-2-step-5. Subsequent instrumentation was interrupted by PowerShell native-stderr exception handling and then by the inherited ADB server losing its owner; process-level logging and an explicitly owned ADB server resolved transport lifetime. Zero-test infrastructure attempts are FAIL, not PASS. Final emulator results are fresh. No test or shipping JVM option was weakened. No failed/unexecuted check counted PASS.

## 13. CI

Pending commit/push and exact-SHA Foundation boundaries/Native application shells. Maximum one focused remediation cycle after initial CI failure, per quota-aware user instruction. No unobserved Apple/MSVC execution claimed.

## 14. Limitations

ICC/non-sRGB profile transformation deferred; no production color-management dependency, tile/cancel/device-pressure policy, GPU or editing pipeline. Float copy is diagnostic/native consumption, not per-frame Kotlin/Swift transport. Context destruction requires quiescence; default full-raster working ceiling is 8 MP despite larger decode admission. Ordinary IEEE nearest rounding expected; no caller floating environment change. Physical RSS/power-loss/privacy-flow/device performance not certified by these foundation checks.

## 15. Acceptance

Not ready for FULL PASS until all applicable local checks and exact-SHA hosted workflows pass. Step 6 not started.

## 16. Proposed Step 6

Separately authorize bounded CPU operation evaluation contracts with numerical golden fixtures, cancellation/tile ownership and later renderer parity. Decide ICC transformation dependency before expanding color input support. This proposal is not authorization.
