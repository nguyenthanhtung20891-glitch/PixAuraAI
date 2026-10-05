# ADR 0012: Bounded CPU operation evaluation and finite numerical semantics

Status: Accepted
Date: 2026-10-05
Scope: Product Owner authorized Phase 2 Step 6; implementation evidence is separate.

## Context

The accepted document schema freezes exposure/1/1 in integer thousandths of a stop. Crop and quarter-turn rotation are registered metadata; other tone tools are reserved identities. Step 5 supplies immutable canonical RGBA32F linear-sRGB premultiplied source images and bounded identity evaluation. Step 6 authorizes the smallest deterministic numerical set and explicitly excludes geometry, rendering and export.

## Decision

Implement exposure/1/1 only, plus the existing empty-stack/identity behavior. Do not invent a redundant identity record or register reserved tools. Keep the document schema, parameter units, historical tuple meanings and ABI 1 unchanged. The new independently versioned evaluation API accepts ordered schema-1 operation envelopes using the authoritative document parser. Registered geometry still parses as metadata but rejects for CPU evaluation.

RGB may be any finite binary32 value, including negative and greater than one. Alpha remains finite in [0,1] and bit-preserved; zero-alpha RGB must be zero. There is no unpremultiplication, intermediate display clamp or gamma calculation. Exposure multiplies premultiplied RGB directly by 2^(milli_ev/1000). Freeze binary64 fractional-stop constants and exact power-of-two scaling, with one binary32 rounding per operation. Overflow rejects the entire candidate before conversion rather than saturating or publishing infinity. Underflow may round to zero. Nearest rounding is required; other rounding modes reject without changing the caller environment. Compiler fast-math and arithmetic contraction are disabled.

Maximum 256 operations matches the existing revision stack bound. Evaluation JSON is at most 64 KiB, each operation object at most 1024 consumed bytes, duplicate IDs forbidden. Kernel temporary memory is constant stack space. Copy once into one private destination and run ordered scalar kernels there; never retain one raster per operation. The live input plus destination are at most two working rasters for a request. Admission reserves destination bytes plus a conservative 1 MiB parser/stack allowance within the existing context payload budget; fixed STL/allocator overhead is additional. All results register only after complete success, including allocation and numerical checks.

Full contract: [CPU evaluation v1](../contracts/cpu-evaluation-v1.md).

## Alternatives

Adding contrast/saturation now would require new parameter tuples and broader tool semantics without strengthening ownership or bounded dispatch proof. Ping-pong is unnecessary for pointwise gain; it would add a full raster. Clamping changes accepted exposure meaning. Runtime exp2 invites libm variation; frozen constants isolate that source. Exact arbitrary-stack agreement with a real-number expression is impossible because each operation rounds separately; reordering/folding is therefore forbidden.

## Consequences

Malicious legal stacks have bounded CPU exposure, but admission is not a latency guarantee. Finite inputs and legal parameters can overflow binary32; this is a documented failure rather than an unconditional finite-success promise. Geometry and reserved tools remain unsupported. Future recipes need new reviewed tuples, and changes to exposure/1/1 semantics need a new operation version. Pixels remain transient; evaluation cannot write SQLite or immutable history.

## Validation

All legal gains against an independent formula, exact integer-stop golden vectors, alpha/transparent/negative/subnormal/overflow cases, maximum and over-limit stacks, failure-at-N atomicity, deterministic parser/property corpus, independent C/JNI/Swift consumers, allocation sweeps, concurrent same-source calls, existing platform regressions and actual sanitizers. Hosted Apple/MSVC execution is mandatory. Evidence: [Step 6 report](../reports/phase-2-step-6.md).
