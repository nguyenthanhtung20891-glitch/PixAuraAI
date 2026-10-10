# Manual detail numerical contract v1

Authority: Phase 3 Step 4 authorization. CPU reference owns the following initial
recipes. No platform image API, GPU kernel, mask or additional control is used.

| Tool/operation ID | Operation/parameter version | Integer parameter | Unit | Inclusive range | Default/neutral | Step |
| --- | --- | --- | --- | --- | --- | --- |
| pixaura.blur | 1/1 | milli_strength | thousandths fixed-kernel blend | 0..1000 | 0 | 1 |
| pixaura.sharpen | 1/1 | milli_amount | thousandths fixed-kernel unsharp amount | 0..1000 | 0 | 1 |

Exactly one plain decimal integer parameter is required; missing/extra keys,
fractional/exponent/string/bool/null/negative-zero values reject. Invalid values
return 7, unknown operation/version 5, malformed envelopes 6, resources 8,
cancellation/stale generation 13. No parameter clamp or locale parsing.
Changing any recipe, range, unit or rounding requires a new version.

## Kernel, domain and edges

Input/output is finite RGBA32F premultiplied linear-sRGB/D65. RGB may be negative
or above white; alpha is in [0,1], with no hidden RGB at alpha zero. Both tools
have a fixed radius of ONE pixel in the current operation's input raster.
Strength never changes radius. There is no resolution scaling or image analysis.
Existing PRV1 evaluation precedes display resampling; preview does not substitute
a downsampled-input kernel. Crop/rotation before detail change that input raster.

Use the nonseparable 3x3 binomial kernel K = [[1,2,1],[2,4,2],[1,2,1]]/16.
Clamp each neighbor coordinate independently to the nearest valid edge pixel;
1x1 and one-row/one-column images follow the same rule. No wrap/mirror/zero pad.
For each premultiplied RGB channel and alpha, G is the weighted sum of the NINE
stage-input samples. Visit dy=-1..1 then dx=-1..1, accumulate from binary64 +0,
multiply each sample by its integer weight, add in that order, divide by 16 once.
There is no rounded intermediate horizontal/vertical pass or vendor library.

## Exact recipes and alpha

Let C be each premultiplied RGB channel, a center alpha, GC/GA the kernel values,
and t = integer_parameter/1000 in binary64.

Blur: C'=(1-t)*C+t*GC; a'=(1-t)*a+t*GA. Blur filters coverage and premultiplied
color together, so transparent pixels contribute zero color, and neighboring
visible coverage may spread into an initially transparent pixel. If rounded
output alpha is zero, set all output RGB to +0 to maintain canonical transparency.
Otherwise RGB remains unclamped. This is coverage blur, not an alpha-preserving
RGB-only blur. No unpremultiply or tiny-alpha threshold.

Sharpen: preserve alpha bits. For a=0 preserve the entire center pixel. Otherwise
GA>0 because the kernel center has positive weight. Compute estimate=a*(GC/GA),
then C'=C+t*(C-estimate), independently per channel. This alpha-normalized
binomial unsharp reference avoids sharpening a coverage edge as a color edge.
It does not sharpen alpha or create hidden zero-alpha color. It is not an
edge-aware/bilateral filter. Negative finite overshoot is defined and unclamped.

Parameter zero bypasses all math and scratch allocation, preserving every pixel
bit. Flat finite uniform RGBA is preserved within the reference contract.
Every primitive rounds to nearest binary64 in the stated order, with no FMA,
reassociation or fast math. Read binary32 exactly; round completed channel to
nearest binary32 once per operation. FE_TONEAREST is required. Nonfinite or
abs(RGB)>FLT_MAX results reject the whole candidate before publication.
Use the established independent-reference tolerance 1e-7+2e-6*abs(expected),
plus 1e-44 for supported underflow; neutral and sharpen alpha identity are exact.
No universal cross-toolchain bit-equality claim. Same-platform ordered replay
and composed versus sequential per-operation rounding are bit exact.

## Bounded execution and lifecycle

Retain the immutable source plus one private candidate, never a third raster.
Keep at most three original input rows in one reusable byte buffer: 3*width*16,
at most 786432 bytes. Reuse the same buffer capacity for rotation's visitation
bitmap; planned scratch is the maximum, not their sum, at most the existing
1 MiB ceiling. Checked admission includes exact scratch plus existing parser/
plan allowance under the unchanged 256 MiB context and 128 MiB raster ceilings.
No raster or row-buffer chain per operation. Read all required original rows
before overwriting a destination row; copy the next input row before reusing its
ring slot. Nine bounded neighborhood accesses per pixel, at most 256 operations.
Poll cancellation before stage/allocation/row copy, each row and each 128 outputs,
and before publication. Allocation/copy remain bounded non-preemptible phases;
no unmeasured latency or throughput claim.

Reuse BEGIN -> UPDATE/PREVIEW* -> COMMIT once, or CANCEL. A changed commit yields
one detached revision proposal; neutral append, return-to-neutral, unchanged
replacement and cancel do not grow history. Existing replacement-to-neutral
removal semantics remain unchanged. Stale session/source/revision results reject.
Failures preserve the last valid pending/display state where the existing
contract permits retry. No new scheduler or durable mutation before approval.

Canonical schema-1 operation envelopes, stable IDs/order/versions, immutable
source hashes and revision/redo semantics remain. Durable detail needs explicit
schema 2->3 migration; ordinary opens never upgrade and 1->3 is unsupported.
No operation normalization/folding/reordering. Exact fixtures must independently
cover impulses, edges, checkerboards, flat/saturated/grayscale, borders, alpha,
repeats, geometry/order, boundaries, failures, restart and platform parity.
