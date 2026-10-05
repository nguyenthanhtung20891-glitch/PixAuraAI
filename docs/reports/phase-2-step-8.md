# Phase 2 Step 8: minimal geometry pixel execution

Accepted Step 7 HEAD: 2e0f975ca894372684c3febbc391dc3ac92d5407, clean main at audit. Product Owner attests Step 7 FULL PASS; baseline hosted Foundation run 37319777904 and Native shells run 37319778089 now show success. Earlier billing failures remain historical. Scope: Step 8 only; Step 9 not started. Status: local gates PASS; hosted Step 8 certification pending.

## Repository findings and frozen semantics

Audited product/architecture/photo/security/quality/roadmap/decision documents, relevant ADRs and Step 1-7 reports/contracts, shared operation parser/history/storage, canonical working images, scalar exposure and geometry/token registry. Frozen crop millionth units/outward rational edges, quarter-turn clockwise direction, canonical top-left linear-sRGB RGBA32F premultiplied representation and exact stack ordering remain unchanged. No architecture conflict requiring a new product decision. The explicitly authorized buffering extension is recorded in [ADR 0014](../adr/0014-two-raster-geometry-execution.md), preserving two rasters rather than adding a third ping-pong raster.

## Execution and resource contracts

Only exposure/crop/rotate /1/1 and empty identity execute. Reserved/future tuples reject. [Execution contract](../contracts/geometry-execution-v1.md) freezes bit-preserving planned crop selection and exact quarter-turn movement; no interpolation/resampling/channel arithmetic in geometry. Every intermediate stage is preflighted; the same plan drives execution. Crop compacts ascending rows using overlap-safe byte movement; rotation processes inverse-mapping cycles with a saved 16-byte pixel and reused visited bitmap. No historical operation reordering/folding. An exposure overflow before later crop still fails even if that crop would discard the overflowing pixel.

Maximum simultaneous request rasters: immutable input + one private source-sized candidate. Logical crop shrinks but capacity stays source-sized and is counted fully. Bitmap <=1 MiB, admitted separately from the 1 MiB parser/plan allowance. Existing 128 MiB raster, 256 MiB context, width/height/pixel/stride, 64 handles, 256 operations and request bounds remain unchanged. Internal preflight computes maximum intermediate dimensions/pixels/raster and exact bitmap demand. No raster per operation or alternate full-raster buffer.

Lazy 128x128 copy/exposure/root tiles remain row-major; crop uses dependency-safe increasing scanlines, each <=16384 pixels. Rotation may follow a cycle outside a root tile but checks every 1024 moves. Checkpoints also cover admission, allocation, each stage, bitmap reset and publication. Existing sticky tokens, status 13, publication mutex fencing, serialized ordinary context calls, independent contexts and quiescent destruction remain unchanged. All failures/cancellations preserve outputs/source/history and reclaim private scratch. Byte-copy is non-throwing; malformed layout and checked offset boundaries cover invalid copy requests. No C ABI layouts/functions/statuses change; ordinary working handles expose final metadata/bounded pixel copy.

## Tests and evidence

New geometry_execution_test uses an independent allocating reference, human-readable 1x1/2x2/2x3/3x2/3x3 matrices, edge tiles/skinny images, transparent/tiny alpha, signed zero/subnormal/negative/out-of-range finite RGB and bit-exact comparisons. Covers all four turns, inverse/four-turn/two-half-turn/full-crop properties, all eight mixed order examples, differing crop/rotate order, discarded-pixel exposure overflow, whole-stack intermediate rejection, maximum 256 and one-over-limit, 2048 deterministic mixed-stack properties and cancellation at every observed checkpoint. Existing Step 6/7 parser fuzz/adversarial/numerical and concurrency suites remain active.

Allocation sweep expands to 12 phases, including geometry planning, first candidate, reusable bitmap and registration with recovery after each failure. No alternate raster or intermediate pixel metadata allocation exists to inject. C/JNI shared native probes execute crop and rotate/crop/exposure with output dimensions/pixel checks, cancellation and stale lifecycle. Swift independently exercises the same native calls and golden selected pixel bits; no Kotlin/Swift geometry implementation.

Final local validation on 2026-10-05 (evidence under ignored build/phase-2-step-8):

| Gate | Observed result |
| --- | --- |
| Windows source tests | 44/44 PASS, zero skipped |
| Linux complete source tests | 73/73 PASS, zero skipped |
| Windows Zig fallback native CTest | 14/14 PASS; not MSVC evidence |
| Linux native CTest, including C ABI consumers | 14/14 PASS |
| ASan/UBSan | 16/16 PASS, including expected-failure sanitizer probes |
| Geometry execution | 11093 checks, 2048 mixed properties, 164 cancellation checkpoints PASS |
| Geometry planning/lifecycle | 51809 checks, 2048 properties, 2048 parser mutations and 2048 lifecycle cases PASS |
| Existing exposure evaluation | 77822 checks, 2048 properties and 2048 fuzz cases PASS |
| Allocation sweep | 12 phases; 438 Windows / 576 Linux injected failures recovered |
| Android debug/release build and lint | PASS; both lint XML reports contain zero issues |
| Android fresh JVM rerun | 4/4 tests PASS; 49/49 Gradle tasks executed |
| Android emulator instrumentation | 4/4 PASS, including native geometry boundary |
| Apple source wiring | PASS in source suite; actual execution requires hosted Apple |
| actionlint, Bash syntax, whitespace | PASS |

Informational final Linux scalar diagnostics (1024x768 source; no performance gate):

| Stack | Output | Root tiles visited | Work checkpoints | Wall ms | Two raster bytes | Bitmap bytes |
| --- | --- | --- | --- | --- | --- | --- |
| Full crop | 1024x768 | 48 | 816 | 42.687 | 25165824 | 0 |
| Rotate 90 | 768x1024 | 96 | 864 | 175.599 | 25165824 | 98304 |
| Crop + rotate | 768x512 | 72 | 1224 | 98.761 | 25165824 | 49152 |
| Exposure + crop + rotate | 768x512 | 120 | 1272 | 134.100 | 25165824 | 49152 |

Counts include initial copy tiles; crop uses scanline checkpoints and rotation also checks cycle groups. Times are unoptimized host diagnostics, not device/RSS claims. Exact 26-file inventory: [phase-2-step-8-files.txt](phase-2-step-8-files.txt). Hosted run/commit provenance will be appended after execution. No dependencies, migrations or production UI were added.

## Limitations and next step

No arbitrary-angle rotation, resize/interpolation, masks, filters, GPU, preview renderer, export, AI or cloud. Full-raster admission and retained crop capacity remain deliberate limits. Non-preemptible bounded parser/allocation/bitmap initialization and lock scheduling prevent a fixed cancellation latency guarantee. Plans/rasters/bitmap remain transient, never SQLite authority. Proposed Step 9: separately authorize a bounded preview/reference-render ownership milestone, after reviewing exact scope; do not begin automatically.
