# CPU reference provider v1

Status: FROZEN under the Product Owner's Option A decision for Phase 5 Step 2.
`cpu.reference.v1` supports only `pixaura.onnx.reference-segmentation.v1`.
PixAuraAI does NOT support general ONNX execution in Phase 5 Step 2.
No external inference or protobuf runtime is incorporated.

## Genuine ONNX profile

Smallest graph: one standard-domain Conv (ONNX opset 13, IR 9), with no attributes,
bias, subgraphs or other nodes. Conv's default group/stride/dilation/padding and
inferred 1x1 kernel reduce RGB `[1,3,H,W]` to `[1,1,H,W]`. One FLOAT initializer
`weights`, dimensions `[1,3,1,1]`, contains exactly 12 raw little-endian bytes.
Each parsed coefficient is 0, 0.25, 0.5, 0.75 or 1 and their sum must equal 1.
Weights are interpreted from the verified payload, not from its filename/digest.
One graph named `pixaura-reference-v1`; input `rgb`; output `probability`;
node consumes `rgb,weights`, produces `probability`, op_type `Conv`.
The signed CPU metadata must declare exactly operator registry ID 1 (Conv),
provider/runtime/API/opset version 1; graph dimensions match the signed manifest.

Allowed protobuf fields only: Model 1/7/8; Graph 1/2/5/11/12; Node 1/2/4;
Tensor 1/2/8/9; ValueInfo 1/2; TypeProto 1; tensor type 1/2; shape 1;
dimension 1; opset import 2. Singular duplicates reject. Repeated node/input/
output/initializer counts reject beyond the profile. Dimensions use unpacked
positive int64 varints; weights use raw_data only. Minimal varints, wire types
0/2 only, field-order independence; unknown fields/attributes/semantics reject.
No training/functions/external/sparse/custom/dynamic types or alternative storage.
Malformed/truncated/overflow wire input rejects even when its package is signed.

Schema authority: [ONNX 1.16.2 schema](https://github.com/onnx/onnx/blob/v1.16.2/onnx/onnx.proto),
[Conv operator](https://github.com/onnx/onnx/blob/v1.16.2/docs/Operators.md#Conv).
These are format references, not runtime dependencies or general compatibility.

## Bounds and arithmetic

Payload <=4096 bytes; <=128 fields per message; fixed nesting <=7; no recursive
parser. One node/input/output/initializer, zero attributes/temporary tensors.
Tensor rank 4, dimensions 1..1024, batch 1, channels 3/1; <=4194304 elements.
Parser stores bounded spans on the stack and a fixed prepared plan, never allocates
from an untrusted length. Checked products/length subtraction precede reads.

Normalize each binary32 RGB value using Step 1's signed `(scale,bias=0)`:
binary64 multiplication by scale, division by 1000000, round to binary32.
Execute pixels in row-major order, channels R then G then B. Binary64 ordered
weighted sum of normalized binary32 inputs and parsed binary32 weights, then
round once to binary32 output. FE_TONEAREST and IEEE binary32/binary64 required;
other environments reject. No fast-math, contraction, transcendentals or alternate
SIMD semantics. Input/output nonfinite or outside [0,1] reject, never clamp.
Binary32 decode and round-to-nearest/ties-even conversion explicitly manipulate
IEEE words, including subnormals, so ambient binary32 flush modes cannot change
semantics. Intermediate binary64 values stay normal or zero. Negative zero is
accepted as zero; negative subnormals reject by their bits. No FTZ/DAZ mutation.
Reference parity is exact binary32 words. Mask threshold is probability >=0.5.
Future provider comparisons must preserve shape/type/range; any tolerance needs
its own reviewed provider/evaluation contract. No accelerated parity certification.

Fixtures include coefficients A=(0.25,0.5,0.25), B=(1,0,0), zeros/ones/gradient/
checkerboard, 1x1 and 1024x1024, and malformed signed graphs. Independent dyadic
integer arithmetic produces compact IEEE-754 goldens; no interpreter-generated
golden authority. Synthetic models attest correctness only, not subject quality.
