# Phase 2 Step 9: bounded preview/reference render ownership

Accepted Step 8 baseline: `86520909fa31372c7926676650873cd105352695`, clean main at audit, Product Owner FULL PASS. Step 9 authorized including local validation, commit/push and hosted CI closure. Step 10 is not started. Status: implementation and required local gates PASS; hosted Step 9 certification pending. No unobserved gate is PASS.

## Audit and approved scope

Product/architecture/photo/security/quality/roadmap/decision documents, relevant ownership/processing ADRs, working/evaluation/geometry/cancellation contracts, Step 8 report and native registry/platform boundaries were inspected. Frozen linear-sRGB premultiplied RGBA32F, exact ordered exposure/crop/quarter turns, immutable history/source, 128 MiB working raster, 256 MiB payload, 64 handles and quiescent destruction remain unchanged. Audit identified aggregate memory conflict: two 120 MiB working buffers plus 30 MiB preview need 270 MiB before provenance. Product Owner approved Option A: 1:1 only, clean status 8 rejection and caller-directed release/retry. [ADR 0015](../adr/0015-bounded-reference-preview.md) records the decision; historical ADRs remain intact.

## Implementation and acceptance criteria

[Reference preview v1](../contracts/reference-preview-v1.md) freezes independently owned immutable RGBA8 straight-alpha sRGB top-left output, exact 1:1 dimensions, binary64 unpremultiply, boundary-only clipping, quantized standard sRGB transfer with frozen thresholds and nearest ties-up alpha quantization. One private destination, no heap scratch, lazy 128x128 row-major tiles, cancellation before/after allocation and before publication. Existing publication lock fences signals. Native owns conversion; C/JNI/Swift tests copy bounded bytes. Preview has no persistence/history/asset mutation capability and retains no working/provenance payload.

Acceptance requires exact bytes and independent formula agreement, unchanged source/output-on-failure, stale/wrong-kind/cap safety, aggregate resource rejection/retry, cancellation and races, allocation-site recovery, all local regression/sanitizer/mobile/source gates and real hosted Linux/MSVC/Apple/Android evidence at exact pushed SHA. Payload accounting follows existing capacity/provenance/control reservation rules; fixed object/allocator overhead remains additional, not a peak RSS promise.

## Validation evidence

Observed local evidence on 2026-10-07, ignored logs under build/phase-2-step-9:

| Gate | Result |
| --- | --- |
| Windows complete applicable source suite | 46/46 PASS, zero skips |
| Linux complete source suite | 75/75 PASS, zero skips |
| Windows Zig fallback native CTest | 15/15 PASS, not MSVC evidence |
| Linux native CTest / C ABI consumers | 15/15 PASS |
| ASan/UBSan / negative instrumentation probes | 17/17 PASS |
| Preview native | 877090 checks in recorded Windows/Linux runs; 2048 numerical properties, 2048 hostile mutations, 8 deterministic checkpoints, 128 signal races and 64 source-release races PASS |
| Allocation sweep | 14 phases, 444 Windows / 582 Linux injected failures recovered |
| Android debug/release and lint | PASS; both lint reports zero issues |
| Fresh Android JVM --rerun-tasks | 4/4 PASS; all 49 tasks executed |
| Android emulator instrumentation | 4/4 PASS, zero skipped/failed |
| Apple source wiring | PASS; execution pending hosted Apple |
| actionlint / Bash syntax / source hygiene / git diff --check | PASS |

Native check count can vary with successful race outcomes; all asserted outcomes are complete success or the documented stale/cancelled error, never partial publication. The aggregate budget test reduces only its test-owned context ceiling after ordinary fixture ingestion to isolate admission; no production tuning API is exposed. Allocation sweep covers preview vector object/buffer and registry node plus debug STL control allocations; no scratch allocation exists.

Commands: scripts/check-decode-zig.ps1 with pinned local Zig; WSL CMake build and verbose CTest; bash scripts/check-sanitizers.sh; node --test applicable source suites; Android Gradle --offline --dependency-verification strict assembleDebug/assembleRelease/lintDebug/lintRelease/testDebugUnitTest/testReleaseUnitTest/connectedDebugAndroidTest, followed by fresh unit --rerun-tasks; actionlint 1.7.12; bash -n scripts/*.sh; git diff --check. Source fake-SDK orchestration temporary files and all local tool caches were confined to repository build paths.

Informational Linux scalar reference diagnostics (opaque black fixture, debug build):

| Dimensions | Input bytes | Output bytes | Tiles | Wall ms |
| --- | --- | --- | --- | --- |
| 256x256 | 1048576 | 262144 | 4 | 7.242 |
| 1024x768 | 12582912 | 3145728 | 48 | 86.525 |
| 4096x2048 | 134217728 | 33554432 | 512 | 898.200 |

No performance threshold, RSS or physical-device claim. Source buffers remain untouched. No new dependency, schema migration, GPU, resampling, production UI, persistent cache, export or AI.

## Files and hosted provenance

Exact file inventory: [phase-2-step-9-files.txt](phase-2-step-9-files.txt). Native preview files implement conversion/ownership and registry admission; C/JNI/Swift fixtures exercise shared bytes; CMake/Swift Package/workflows wire existing gates; contracts/ADR/root documents/report record approved policy. scripts/codex-gh.ps1 run-view now also reports headSha and job IDs for exact certification provenance, retaining existing read-only scope and repository-local log cache.

Accepted baseline SHA independently observed green: Foundation 37335962711 and Native shells 37335962768. Implementation commit 43d86d3029ea13266237f5edbbf243b64f023b05 was pushed to approved origin/main. Exact headSha observed for Foundation 37611252296 and Native shells 37611252203. Foundation real MSVC job 112758750696 passed native 15/15 with MSVC 19.51.36260.0, preview 877090 checks and 1763 recovered allocation failures. Linux, sanitizer and both Android NDK jobs also passed. Foundation Apple job 112758750721 failed an existing storage concurrent-dedup test after preview itself passed; no Apple FULL PASS is claimed for that run.

Root cause: POSIX linkat then staging unlink creates a short two-link interval. A competing dedup verifier correctly rejects st_nlink!=1. Apple publication now uses renameatx_np(RENAME_EXCL), an atomic no-overwrite move followed by existing directory syncs. Strict hard-link/symlink/hash checks remain unchanged; no retries, weakened assertions or disabled gates. Concurrent dedup regression now runs 32 fresh-content synchronized pairs. Windows/Linux native 15/15 and ASan/UBSan 17/17 passed again after this narrow remediation. The contract also clarifies the attainable maximum of 63 preview handles while creation needs a live working handle within the unchanged combined cap of 64. Remediation hosted evidence is pending. Baseline success does not certify Step 9.

## Limitations and next step

Legal retained contexts may reject preview by approved design. No full color-management or physical-device throughput certification. Allocation/lock scheduling remains non-preemptible. Proposed Step 10: separately scope platform preview consumption and any resize semantics after Product Owner review; not authorized or started.
