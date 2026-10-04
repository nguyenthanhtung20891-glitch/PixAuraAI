# Phase 2 Step 4: bounded image decoding foundation

Status: READY FOR CI; implementation and applicable local gates PASS; hosted CI closure pending. Step 5 is not started. Updated 2026-10-04.

## 1. Repository findings and approved decision

Audited accepted Step 3 HEAD `94159bcc071ba1ae6832bd0cd0012017d54d09ad`, initially clean. Read required product/architecture/photo/security/quality/roadmap/decision documents, relevant ADRs, Step 1-3 contracts/reports, dependency policy, native document/storage implementation and Android/Swift boundaries. JPEG/PNG SDR, immutable hashed originals, SQLite authority, orientation normalization before evaluation, future linear working space and explicit non-pointer context handles were frozen. No decoder dependency or implementation existed. Storage accepted caller metadata without extracting image metadata.

Initial audit stopped on platform-codec ownership and dependency choice. The Product Owner explicitly approved portable libjpeg-turbo/libpng/zlib and shared C++ ownership for JPEG/PNG SDR only. [ADR 0011](../adr/0011-shared-bounded-sdr-decode.md) records that superseding decision without rewriting historical ADRs. No remaining architecture conflict is known.

Read-only baseline inspection through scripts/codex-gh.ps1 confirms Step 3 [Foundation 37191218419](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37191218419) and [Shells 37191218459](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37191218459) successful at the exact accepted SHA. These are baseline evidence, not Step 4 PASS.

## 2. Decoder and admission architecture

Private digest/length descriptor -> one pinned no-follow asset read -> hash the exact retained encoded bytes -> bounded signature/structure admission -> immutable Source -> checked decode layout -> privately owned output -> strict codec execution/output check -> publish image handle. Rehash and decode share the same retained bytes, avoiding verify-then-reopen substitution. Failure never publishes a partial image or replaces caller output. SQLite/document/history state is untouched; clearing every decode context leaves projects reopenable.

Shared C++ owns policy; a small owned C adapter isolates codec longjmp and uses per-operation memory hooks. JPEG's documented system-memory backend replaces jmemnobs without changing vendor source. Libpng/zlib custom allocators enforce scratch budgets, including overhead. No backing files, global cache, registry singleton, network, renderer, export API or GPU objects. Codec code is included in sanitizers; owned code stays warning-clean.

## 3. Supported format scope

Content-based baseline 8-bit single-interleaved-scan Huffman JPEG, grayscale or three-component SDR; still PNG grayscale/RGB/indexed/gray-alpha/RGBA with valid 1/2/4/8-bit combinations, tRNS and Adam7. Synthetic fixtures prove grayscale, RGB, gray-alpha, palette transparency and Adam7 alongside baseline JPEG/RGBA PNG. Filename extension is ignored, including ingestion deliberately mislabeled .jpeg.

Recognized unsupported formats/variants return 16; malformed returns 17; truncated returns 18; resource limit returns 8; output mismatch returns 19. Header admission is structural, not a claim that compressed entropy is valid. Progressive/arithmetic/lossless/CMYK JPEG, PNG depth 16, APNG, JPEG MPF, HDR signaling and other codecs reject. No HEIF/AVIF/RAW, animation playback or encoding/export support.

## 4. Verified metadata contract

[Version-1 contract](../contracts/encoded-image-v1.md) and [decode.h](../../packages/core/include/pixaura/decode.h) define encoded/display dimensions, codec, orientation, depth/channel/alpha information, profile presence/type/size/SHA-256, frame count, RGBA format and checked row/output estimates. Parsed Source is immutable. Arbitrary EXIF/XMP/IPTC never becomes domain data or logs. ICC bytes remain untrusted retained data despite bounded header/tag-range validation. Original immutable bytes retain other color annotations and private metadata. PNG ICC inflation is output/scratch bounded; codec text/ICC extraction is disabled to avoid duplicate unbounded parsing.

## 5. Orientation contract

EXIF 1 identity, 2 horizontal mirror, 3 180 degrees, 4 vertical mirror, 5 transpose, 6 clockwise 90, 7 transverse, 8 clockwise 270. Missing orientation defaults to 1; invalid/duplicate/conflicting explicit orientation rejects. Encoded dimensions and pixel raster are preserved; display dimensions swap for 5-8. Tests cover all eight values for both formats and prove identical encoded-raster pixels. No color/geometry normalization is executed in Step 4; future evaluation normalizes once before kernels.

## 6. Decode limit policy

| Bound | Default hard ceiling (caller may reduce) |
| --- | --- |
| Encoded source | 64 MiB |
| Width / height | 16384 each |
| Pixels | 33554432 |
| RGBA output | 128 MiB |
| Row stride | 65536 bytes |
| Non-image metadata | 1 MiB |
| ICC / EXIF | 512 KiB / 256 KiB |
| Markers/chunks/restarts | 4096 |
| Frames | 1 |
| Codec scratch | 32 MiB |
| Aggregate context payload/reservations | 256 MiB |

TIFF traversal: four linked IFDs, 256 entries each; ICC tag table: 1024 entries. Checked arithmetic proves width*height, width*4, stride*height and pixel_count*4 before output allocation. Aggregate accounting tracks retained vector capacities and conservative transient reservations. Fixed parser/control/object overhead is bounded separately; these are payload ceilings, not a measured peak-RSS/device-tier guarantee. Future tiled/device-tier work remains separate.

## 7. Decoded memory model

Transient owned RGBA8 with straight alpha, explicit encoded width/height and stride. Encoded samples retain source color interpretation and orientation; this is not the linear-float editing space. Unique ownership avoids large buffer copies, including MSVC debug-container moves; immutable shared Sources keep image-associated encoded/profile state alive after source-handle release. RAII destroys buffers deterministically. No pixels are persisted in SQLite and no original is overwritten.

## 8. C/C++ and C ABI changes

New independently versioned decode family exposes defaults, explicit context init/destroy, managed source open, metadata query, bounded decode, bounded pixel/profile copies and release. Existing ABI 1/probe feature flags and document/storage families remain unchanged. Handles are context identity + monotonic serial, not pointers; at most 64 live handles. Cross-context, released/destroyed and wrong-kind handles reject; serials never reuse/wrap. C entry points contain all exceptions. Context storage, truthful pointer lengths and nonoverlapping buffers remain caller obligations. Android JNI and Swift tests exercise the actual checked boundary off UI threads; no codec object is exposed.

## 9. Dependencies and supply chain

Exact pins: libjpeg-turbo 3.2.0, libpng 1.6.59, zlib 1.3.2. [Dependency review](../DEPENDENCIES.md) records official source/archive sizes, licenses, hashes, security history, platform/build policy and footprint implications. [Per-file provenance](../../packages/core/vendor/codecs/provenance.json) records every unchanged vendored file; CMake and source tests verify them. Published libpng/zlib archive hashes match; JPEG archive hash is official HTTPS trust-on-first-use, not independent signature attestation. Owned configuration headers are separate; no vendor warning-silencing edits. Portable SIMD configuration is explicit on all platforms.

Recorded prechange three-ABI APK totals: Debug 18898487 bytes, unsigned Release 14858259 bytes. Final local totals: Debug 24006511 bytes (+5108024), unsigned Release 15961987 bytes (+1103728). These observations are not a clean baseline-SHA or isolated per-codec measurement. Apple size remains unmeasured locally.

## 10. Security/adversarial coverage

Zero/extreme/bomb dimensions, checked multiplication overflow, row/pixel/output/encoded limits, every truncation of both fixtures, malformed lengths/CRC/entropy, empty and random bytes, unsupported GIF/progressive/depth/HDR/animation/critical chunks, mislabeled extension, duplicate/conflicting orientation/profile data, invalid TIFF type/count/cycle, ICC corruption/size limits, metadata/marker/EXIF budgets and trailing garbage. Original hashes reverify after decode. All failure paths are bounded and followed by valid requests where applicable. Existing storage traversal/link/hash/crash/SQLite defenses and their tests remain intact.

## 11. Bounded fuzz/property coverage

Fixed seed 0x50495834: 4096 signature-preserving single-byte mutations of synthetic JPEG/PNG plus 1024 64-byte random/property cases, including checked multiplication. Encoded/pixel/scratch policy bounds every attempted decode; outer CTest timeout is 90 seconds. Corpus duration and count are fixed, not an unbounded CI fuzz job. Native and codec code run under actual ASan/UBSan. No claim of exhaustive codec fuzzing.

## 12. Failure injection

Codec allocation-site sweeps for JPEG and PNG; decoder initialization and output-mismatch hooks compiled only in tests; malformed entropy/EOF/resource rejection; independent replacement-new sweep through context init, source open and decode publication. Failures preserve output sentinels and subsequently valid requests work. Native assertions check source/image ownership, no leaked scratch allocations, handle budget exhaustion/recovery and repeat release/destroy. Windows fallback C++ sweep recovers 35 allocation sites; Linux recovers 14 (allocator/STL implementation differences). Hosted MSVC execution must independently prove its Debug paths.

## 13. Concurrency policy

Calls serialize within a context, with parallel independent contexts. Tests exercise concurrent metadata reads, simultaneous PNG/JPEG decode, duplicate image requests, source/image release ownership, and query/release races. Owners join in-flight workers before destruction; racing free/destroy against calls is outside the frozen caller-exclusive destruction contract (ADR 0009). No generalized scheduler or implicit shared decoder registry is introduced.

## 14. Exact changed files

Owned changes: root architecture/photo/quality/roadmap/decision documentation; dependency review; new ADR/contract/report; CMake/Swift package and codec source/integrity manifests/configuration; decode C header, C++ policy/context implementation and C codec adapter; pinned verified asset reading; native adversarial/C/allocation tests and shared synthetic fixtures; Kotlin/JNI/Swift boundary tests; native/Apple gate scripts and both workflow source checks; vendor preservation/source hash tests. Exact intended paths are in [file inventory](phase-2-step-4-files.txt). Vendor paths are individually enumerated and hashed by provenance. No generated build, downloaded archive, user image, credential, cache or database is committed.

## 15. Commits

Created/pushed through scripts/codex-git.ps1: `b8449b484e1f080d444cb9fa9bd7f56081e572cf` (bounded decode implementation, dependency pins, contracts and tests). CI remediation/evidence commits follow below.

## 16. Local validation

Evidence is ignored under build/phase-2-step-4 and normal build/test-result folders. Final observed local results on the implementation tree (zero skips/failures where counts are given):

| Gate | Observed result |
| --- | --- |
| Windows-compatible source | 36/36 PASS, zero failures/skips |
| Linux source | 65/65 PASS, zero failures/skips |
| Windows Zig fallback native CTest | 10/10 PASS; final documented fallback also executes existing manual-script ABI/document/allocation/storage gates |
| Linux Clang Debug CTest | 10/10 PASS |
| ASan/UBSan CTest | 12/12 PASS, including both actual negative instrumentation probes |
| Android final | PASS strict offline Debug/Release, lintDebug/lintRelease, JVM and instrumented APK/connected gates; 147 tasks, 28 executed/119 up-to-date; JVM XML 2/2 and fresh instrumentation XML 4/4, errors/failures/skips zero |
| Workflow/shell checks | PASS actionlint 1.7.12 both workflows (local shellcheck/pyflakes integrations unavailable and disabled); all scripts/*.sh pass bash -n; git diff --check/source hygiene PASS |
| MSVC local | BLOCKED: existing installation lacks vcvarsall; hosted MSVC is mandatory |
| Apple local | BLOCKED: Windows host has no Apple runtime; wiring is source evidence only |

Initial failures were diagnosed rather than counted PASS: WSL RPC/service access recovered without system modifications; sandbox child-spawn restrictions required authorized tooling access; stale fallback archive discovery required a fresh repository-local toolchain; libpng's unused encoding configuration and ARM auto-SIMD selection were corrected in owned configuration; orientation parsed before JPEG SOF was inadvertently reset and fixed; relative test root was corrected to absolute. WSL DrvFS cannot reproduce POSIX DAC tests, so the established private tmpfs mount namespace under the repository is used and vanishes on process exit. An all-source Windows invocation attempted Linux-only fake-emulator scenarios and failed; final Windows uses its applicable 36 checks, while all 65 execute on Linux. Temporary output is redirected to repository directories.

## 17. GitHub Actions

Initial Step 4 workflows on exact SHA `b8449b484e1f080d444cb9fa9bd7f56081e572cf`: Foundation run 37208103131 failed on two host compiler issues; Native application shells run 37208103134 remains in progress at this observation. Both Foundation boundaries and Native application shells must pass on the exact pushed SHA, including real MSVC, Apple package/app/simulator and Android hosted emulator. Use scripts/codex-gh.ps1, verify SHA before retrieving current failed logs and repeat narrow remediation until green. Baseline successes in section 1 do not substitute.

## 18. Remaining limitations

Only baseline 8-bit JPEG and specified still PNG subsets; progressive/depth/CMYK/HDR/animation/other codecs are unsupported. Full-raster output is bounded; tile/device pressure policy is future work. Profiles are structurally admitted/retained, not fully semantically validated or converted. Pixel output is encoded sample data, not ready for editing/rendering. Context destroy needs caller quiescence. No physical-device RSS/power-loss guarantee, UI/import flow, renderer, color transform, adjustment or export pipeline is claimed. Hosted platform gates remain pending.

## 19. Step 4 acceptance

Not FULL PASS until final local regressions and exact-SHA CI workflows are green. Approved ownership/dependency conflict is resolved; no Step 5 work is underway.

## 20. Proposed Step 5

Separately review/authorize orientation and color normalization into the frozen linear working space, then initial CPU reference evaluation with explicit numerical/resource contracts. This proposal is not authorization and no Step 5 implementation has started.

## CI remediation round 1

Exact current run/job logs were retrieved through scripts/codex-gh.ps1 after commit-filtered listing. Foundation 37208103131: real MSVC C compiler lacks the C max_align_t declaration used by the allocation header; Ubuntu GCC rejects copied pair loop variables under -Werror=range-loop-construct. Corrected only owned code: a portable union aligned for the scalar codec types, and const-reference test loop bindings. A final scratch-budget review also moves the fixed decoder-control size check before its allocation; both codecs test scratch=1 rejection. No vendor bytes, warning gates or security policy were changed. Local GCC is absent and its attempted configure is BLOCKED; hosted GCC must execute the corrected tree.

First-run Foundation Apple, sanitizers, workflow lint and both Android native jobs independently pass. Retrieved Apple job 111453460725 checkout logs verify b8449b4 and actual macOS Swift 7/7, iOS simulator package 7/7 and successful unsigned device build. This is genuine initial implementation evidence, not final corrected-SHA acceptance. Fresh Windows fallback and Android regressions pass on the remediation tree (10/10 native; strict Android 147 tasks, 23 executed/124 up-to-date; instrumentation 4/4), with refreshed Clang 10/10, ASan/UBSan 12/12, Linux source 65/65 and Windows source 36/36 passing before publishing the fix. Local actionlint 1.7.12 passes through its Windows executable; an initial invocation incorrectly targeted its containing directory and was corrected.
