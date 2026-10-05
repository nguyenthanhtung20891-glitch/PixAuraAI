# Bounded CPU operation evaluation, version 1

## Frozen operation and representation

Step 6 implements only `pixaura.exposure / operation_version 1 / parameter_version 1`. Parameters contain exactly `milli_ev`, a plain decimal JSON integer in [-5000,5000]. The original schema-1 operation envelope is unchanged: exact `id`, `type`, `operation_version`, `parameter_version`, `parameters` fields. IDs remain 32 lowercase hex digits, unique within a stack. No migration, fractional numeric serialization or new registered identity tool is introduced. Canonical document serialization remains shortest decimal integers, ASCII sorted keys and LF. Floating parameter NaN/Inf, fractional/exponent notation, negative zero, strings, missing/extra keys, duplicate/escape-equivalent keys and invalid UTF-8 reject using the shared parser. Unsupported tuples return 5, invalid parameters 7, malformed envelopes 6, resource excess 8. Geometry tuples remain valid document metadata but reject evaluation, never skip.

Request family 1 is negotiated separately through the `version` C argument. Its JSON is exactly `{"operations":[OPERATION,...]}`. Arrays preserve the selected complete stack order; the service must use document `replay` order rather than concatenate ancestor stacks. Empty stack is identity. Equal parameter values with distinct IDs may repeat. A new operation meaning requires a new operation version. Evaluation grants no approval/persistence capability.

## Exact numerical recipe

For each pixel `(r,g,b,a)` and integer `e=milli_ev`:

`a' = a`, `r' = RN32(RN64(double(r) * G(e)))`, likewise g and b.

`q=floor(e/1000)`, `t=e-1000*q` (0..999), `G(e)=ldexp(T[t],q)`.

`T` is the frozen binary64 constant array [exposure_table.hpp](../../packages/core/src/exposure_table.hpp), approximating `2^(t/1000)` with relative error <=1e-14. `ldexp` scales exactly within this domain. Multiplication uses binary64, then one nearest binary32 conversion per channel per operation. e=0 bypasses arithmetic and preserves all bits. Integer-stop results with representable products are exact, including e=-5000/5000. Alpha bits never change. No unpremultiply/division/pivot/clamp/gamma transform occurs. Zero alpha requires zero RGB and gain cannot create hidden color. Tiny nonzero alpha has the same recipe without thresholding.

RGB inputs/intermediates/outputs may be finite outside [0,1], including negative values; alpha must be finite in [0,1]. Before kernels, every channel is validated, including empty stacks. Non-finite pixels and hidden zero-alpha RGB reject. Each double product must be finite and have magnitude <=FLT_MAX before float conversion; otherwise INVALID_PARAMETERS aborts the whole stack. Underflow rounds to a subnormal or zero. Arbitrarily large legal gain stacks cannot guarantee successful finite output for every finite input; overflow is explicit failure. Display/export clamps belong to later stages.

Ordinary IEEE nearest rounding is required; `fegetround()!=FE_TONEAREST` rejects. The core does not change rounding/control modes; ordinary arithmetic may set floating exception status flags. Input finite classification uses binary32 exponent bits, rejecting signaling NaNs without floating comparisons or FE_INVALID. Cross-platform comparison to the independent real formula per operation uses `abs(error)<=1e-7+2e-6*abs(expected)`; comparisons involving underflow additionally allow 1e-44 absolute error for supported platform denormal behavior. Empty/e=0 identity and alpha preservation are bit exact; composed-vs-sequential evaluation with identical per-operation rounding is bit exact on the same platform. Stack output is defined by ordered per-operation rounding, never a folded sum of exposure parameters. No thread reductions, scheduling dependence, locale parsing or platform color APIs. CMake uses `/fp:strict` for MSVC and `-fno-fast-math -ffp-contract=off` for Clang/GCC/Android NDK/Apple Clang; Swift Package applies the same Clang flags. No SIMD or hidden registry mutation.

## Ownership, resources and failure

[evaluation.h](../../packages/core/include/pixaura/evaluation.h) adds validate/evaluate to the existing explicit decode context; output uses existing working metadata/query/copy and decode release. ABI 1/probe remains unchanged. No raw owning pointer, pixel bridge JSON, internal kernel pointer or platform image is exposed. C++ exceptions map to statuses. Source provenance stays retained; source working raster/history/original asset stay immutable. A caller publishes a candidate only after success.

Max stack 256 operations; request 65536 bytes; each operation consumed object bytes 1024 (outside-object separator whitespace counts toward total request). These limits reject before raster execution. Source/destination each respect the reducible working ceilings: 16384 dimensions, 8388608 pixels, 262144 row bytes, 128 MiB raster. Context aggregate payload remains 256 MiB, at most 64 handles. Admission reserves destination plus 1 MiB for bounded parser/stack storage. The parser structurally limits integer/field/string/array/depth work before constructing pixels; STL node/debug/allocator/control overhead is additional bounded overhead, not a peak RSS promise. Standalone validation has the same parser limits. Total request payload admission includes existing context-retained source/provenance plus destination plus parser allowance; input JSON is borrowed only for the call and bounded separately to 64 KiB.

Maximum two simultaneous working rasters per request: immutable input and private destination. Other context-owned decoded/working images remain counted independently. One copy, in-place pointwise passes on the private destination; no heap kernel scratch, ping-pong or raster per operation. Failed parsing/validation/allocation/kernel/handle registration destroys local candidates and leaves caller output untouched. There is no partial result. Subsequent valid calls recover. Allocation injection sweeps include existing source buffer, destination, parser storage and registry-node allocation. A kernel scratch allocation cannot be injected because no such allocation exists; scratch=0 is an architectural property, not a waived failure site.

Calls serialize within one context, including reads/evaluations of the same immutable source; independent contexts can run concurrently. There is no scheduler/global thread pool. Destroy/init/free require the owner to quiesce and join all calls, per ADR 0009. Stale/cross-context/wrong-kind handles reject; serials never reuse/wrap; repeated release/destroy safely reject. Evaluated pixels never become authoritative SQLite state, and evaluation never mutates retained revisions.

## Human-reviewable golden vectors

| Premultiplied input | milli_ev | Expected output |
| --- | --- | --- |
| (0,0,0,0) | 5000 | (0,0,0,0) |
| (1,0,0,1) | 1000 | (2,0,0,1) |
| (.25,-.5,2,.5) | 1000 | (.5,-1,4,.5) |
| (.25,.25,.25,1) | -1000 | (.125,.125,.125,1) |
| (1e-30,-1e-30,0,1e-30) | 0 | exact input bits |

Native tests use 2x2 versions plus real 2x3 JPEG/PNG pipeline fixtures, all 10001 legal gains, 2048 fixed-seed numerical/property cases and 2048 bounded parser mutations. Failure/order vectors use FLT_MAX followed by +1000/-1000 versus the reverse: overflow in the former aborts rather than algebraically cancelling it. Benchmarks report dimensions, operations, elapsed time and estimated raster bytes; no Step 6 throughput threshold or physical-device performance claim.
