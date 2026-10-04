# ADR 0011: Shared bounded SDR decoding

Status: Accepted
Date: 2026-10-04
Scope: Phase 2 Step 4; Product Owner explicitly approves shared libjpeg-turbo/libpng/zlib decoding.

## Context

JPEG/PNG SDR first, immutable originals, checked C ABI, native domain authority and bounded memory are frozen. Prior ARCHITECTURE.md and the schema-1 contract assign codecs to platform adapters. The Product Owner explicitly supersedes that ownership for JPEG/PNG SDR only after reviewing portable and platform alternatives.

## Decision

Shared C++ owns admission, bounds, metadata and decode semantics. Libjpeg-turbo 3.2.0, libpng 1.6.59 and zlib 1.3.2 are exact vendored inputs, with unchanged distribution files and SHA-256 provenance. Android/iOS own OS/file integration and background invocation, not an independent JPEG/PNG policy. No HEIF/AVIF/RAW/HDR/animation/export ownership decision is implied. Historical ADRs/contracts remain historical; this ADR supersedes only their JPEG/PNG platform-codec ownership.

Version-1 decode admits 8-bit baseline single-interleaved-scan Huffman JPEG (one or three components; no CMYK/YCCK/progressive/arithmetic/lossless) and still PNG grayscale/RGB/indexed/gray-alpha/RGBA with valid 1/2/4/8-bit combinations, including Adam7 and transparency. Unsupported variants reject explicitly. Native signatures and structural evidence select the format, never filenames. Admission checks structure before output allocation but cannot certify entropy until execution. Errors distinguish unsupported, malformed, truncated, resource limit and output mismatch; unknown random input is malformed. APNG control/frame chunks and JPEG MPF reject.

RGBA8 straight alpha in encoded raster order is transient decoder output, not the frozen linear-sRGB float editing space. Width, height, stride, format and ownership are explicit. The original retains orientation and color metadata; decode performs neither orientation nor color conversion. Orientations 5-8 swap display dimensions; missing orientation means 1; invalid/conflicting explicit orientation rejects. Future evaluation normalizes orientation once and converts color before editing. No source mutation, pixel database, renderer, global cache or GPU objects. This first bounded full-raster decode is deliberately small; tiled evaluation and device-tier admission remain later work, not a claim of full-resolution coverage.

Default hard ceilings: encoded 64 MiB, width/height 16384, pixels 33554432, RGBA 128 MiB, row stride 65536 bytes, non-image metadata 1 MiB, ICC 512 KiB (including inflated PNG ICC), EXIF 256 KiB, 4096 chunks/markers/restarts, one frame, codec scratch 32 MiB, aggregate context payload/reservations 256 MiB. Callers can reduce positive limits, never increase them. Checked arithmetic precedes allocations. TIFF traversal is at most four linked IFDs with 256 entries each; only IFD0 orientation becomes domain metadata. ICC structure/tag bounds are checked; its original bounded bytes/digest/type remain available as untrusted data for future color management. Arbitrary EXIF/XMP/IPTC never becomes trusted domain state; original private source retains those bytes. No full color-profile semantic validation or conversion is claimed.

Libjpeg uses an owned implementation of its documented system-memory hooks, replacing the default backend without modifying vendor code. Libpng/zlib use custom budget allocators. Decoder initialization, pool overhead and scratch are bounded, backing-store requests reject, warning recovery/EOF synthesis never yields partial success. The C adapter isolates longjmp from C++ destructors. Decoder output layout must match verified metadata before publication. Owned code stays warning-clean; vendor warnings are scoped separately. SIMD is disabled for the initial portable reference, PNG encoding and compressed text/profile extraction are disabled in the owned configuration; C++ handles retained profiles.

The separate decode C capability family uses caller-owned explicit contexts, fresh identity per lifetime, context-scoped non-pointer serial handles, at most 64 live source/image handles, no serial reuse and no codec objects/pixel pointers across the ABI. All failures preserve outputs. Context calls serialize; distinct contexts may decode concurrently. Duplicate requests return independently owned images and consume budget. Images retain source/profile ownership after source-handle release. Init/destroy require exclusive caller access and quiescence, as frozen in ADR 0009; a second destroy safely rejects while caller storage remains accessible. No scheduler or hidden registry. ABI 1 and document/storage capabilities remain compatible.

Aggregate accounting covers actual vector capacity for retained encoded/profile/pixel payloads plus worst-case transient profile and codec reservations. Constant bounded parser/control structures and at most 64 registry/object headers are additional fixed overhead; the payload ceiling is not a peak-RSS guarantee. OS allocator bookkeeping and tool instrumentation also add overhead. A pinned verified asset read hashes the exact retained bytes from one no-follow capability, avoiding verify-then-reopen TOCTOU. Discarding every decoded image leaves the durable project unchanged and reopenable.

## Alternatives

Platform codecs reduce bundled size but complicate cross-OS strict bounds and deterministic admission. Restricted stb_image reduces integration cost but has a less suitable security maintenance posture for hostile input. Writing JPEG/PNG codecs internally adds a large unreviewed attack surface. A large codec framework unnecessarily expands format/dependency scope. HDR/CMYK/progressive support would expand this initial policy and requires separately reviewed bounded behavior.

## Consequences

Three dependencies require advisory/provenance maintenance and measured footprint. Decode intentionally rejects some valid JPEG/PNG variants and oversized stills. Whole-raster limits do not replace later tile/device budgets. Embedded profiles are retained rather than transformed; these encoded sample values must not be used as linear editing pixels. Exact source/archive hashes and license obligations are in the [dependency review](../DEPENDENCIES.md) and vendored provenance. SIMD/performance optimization requires subsequent evidence, not silent configuration changes.

## Validation

Independent C/C++/JNI/Swift admission/metadata/decode/lifecycle checks, all orientations, malformed and resource adversaries, allocation/init/mismatch injection, deterministic bounded mutation corpus, context isolation/concurrency/quiescence and original hash preservation. Run actual ASan/UBSan including codec source and negative instrumentation probes. Existing source/native/storage/mobile/Apple/workflow gates remain mandatory. Observed evidence belongs in the [Step 4 report](../reports/phase-2-step-4.md); this ADR alone is not PASS.
