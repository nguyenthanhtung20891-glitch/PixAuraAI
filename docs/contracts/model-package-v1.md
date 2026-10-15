# Local model package v1 — frozen Step 1 contract

Status: FROZEN for authorized Phase 5 Step 1. Signature architecture: ADR 0022.
Approved dependency: libsodium 1.0.22, ISC, reviewed archive SHA-256
`adbdd8f16149e81ac6078a03aca6fc03b592b89ef7b5ed83841c086191be3349`.
Step 1 verifies packages only; it never loads a graph or executes inference.

Step 2 clarification: the shared verifier retains the already authenticated tensor,
normalization and CPU payload/provider binding internally for prepared-model use.
The inference context wraps successful Step 1 verification in stale-safe tokens
and retains immutable ownership. This does not change package bytes, trust,
signatures, limits, verification order/reasons or the legacy exclusive-pointer ABI.
Only the separate Step 2 provider performs its reviewed graph parsing/execution.

## Local container and immutable ownership

A flat uncompressed local directory contains `manifest.pxm`, `signature.pxs` and
exactly 1..8 listed regular payload files. No archive, decompression, encryption,
recursion, plugin/script execution, URL fetch or package-supplied trust root.
Absolute caller-selected root: valid Windows UTF-8 drive path or POSIX absolute
path, at most 4096 bytes; no NUL or dot/parent components. Relative roots reject.
Windows directory component count is at most 129. Payload leaf names: 1..64 ASCII
bytes, lowercase letters/digits/dot/hyphen/underscore; no leading/trailing dot,
trailing space, separators, colon, NUL, drive/UNC path, Windows reserved device
stem or normalized duplicate. Names never select model semantics.

Native reader walks directories using no-follow handles; Windows holds ancestor
handles with delete/write sharing denied and opens entries as reparse points.
POSIX uses held directory/openat descriptors. All entries must be regular files,
with one hard link, and no symlink/reparse point. Directory enumeration is bounded
before allocation; unsupported/extra entries reject. Exact reads reject truncation
or extra bytes. File bytes are copied into owned private memory; verification hashes
and signs that same snapshot. Replacing/mutating an external source after copying
cannot change the verified snapshot, and no execution provider gets external paths.
No durable model installer/database is introduced. Owned immutable bytes, not an
external filename or cached validation boolean, are the future runtime authority.

Only fully verified snapshots publish opaque package objects. No output pointer
changes on failure. The object exposes bounded metadata only, no writable payload
or model-loading API. Caller exclusively releases the object, must not reuse it
or race destruction/use. It is not a document/context handle. A crash before
admission leaves no durable package/history mutation; copied memory is reclaimed
by process teardown. Schema 3, history and runtime policy persistence unchanged.

## Bounds

| Item | Frozen limit |
| --- | --- |
| Manifest | 65536 bytes |
| Signature envelope | Exactly 80 bytes |
| Files | 8 payloads, 10 directory entries including metadata |
| Individual payload | 1..67108864 bytes (64 MiB) |
| Total payload | 134217728 bytes (128 MiB) |
| Total package data | 134283344 bytes (128 MiB + 65536 + 80) |
| Explicit verification memory ceiling | 65536..268435456 bytes; 65536 reserved for control/parse ownership |
| Strings | Model ID/labels/license 128 bytes; provenance 512; attribution 1024; payload leaf 64 |
| Tensor count / rank | Exactly one input and output; rank exactly 4 |
| Dimensions | 1..1024; batch exactly 1 |
| Elements / bytes per tensor | 4194304 / 16777216 (float32) |
| Providers / operators per provider | 1..3 / 1..64 |
| Trust anchors | At most 16 |

Checked subtraction/product comparisons precede allocation or arithmetic. Excessive
manifest/files/counts/tensors reject. No generic JSON parser, recursion, unbounded
strings or floating point parsing. Copy budgets count all file bytes plus the fixed
control reserve; they are ceilings, never promises of allocation or admission.
No Phase 4 hard ceiling changes. Future inference admission remains Step 6.

## Canonical manifest bytes (all signed)

Exactly one binary encoding: fixed ordered fields below, unsigned big-endian
u32/u64 values; text is u16 big-endian byte length followed by exact UTF-8 bytes.
UTF-8 must be shortest-form Unicode scalar encoding; surrogates, out-of-range
codepoints and ASCII control/DEL bytes reject. No BOM, escaping, terminator,
padding, newline/line-ending conversion, Unicode normalization or alternate
serialization. Text empty only where semantics permit. Digests are raw 32 bytes.
No keys exist in this positional schema: duplicate inserted fields, unknown fields
and trailing bytes reject rather than participate in generic JSON key handling.
Integer widths have no alternate/overlong encoding. Whole-manifest length must
match exactly. Files/providers/operators are strictly increasing in declared order.

| Ordered field | Encoding / meaning |
| --- | --- |
| Magic, package version, exact manifest length | u32 `0x50584d50` (PXMP), 1, byte count |
| Algorithm, signature version, key ID, trust purpose, store generation floor | Five u32 values; algorithm=1 Ed25519, signature version=1; purposes 1 production / 2 test |
| Detached signature location and encoding | text exactly `signature.pxs`; u32 encoding=1 (raw detached signature in v1 envelope) |
| Model ID, model version, task | text; positive u32 version; task u32=1 binary image segmentation |
| Input tensor then output tensor | Each: u32 rank=4, type=1 float32, layout=1 NCHW; four u32 dimensions |
| Input R/G/B and output ranges | Eight u32 values: four (min,max) pairs, exactly (0,1000000); units one millionth |
| Input normalization | Six u32 values: RGB (scale,bias) pairs; scale 1..1000000, bias exactly 0; normalized=input*scale/1000000 stays within the declared [0,1] tensor range |
| Background and foreground labels | text `background`, text `foreground` |
| Provider count and ordered records | u32 1..3; record defined below |
| Capability declarations | u32 minimum tier 0..2, CPU threads 1..1024; u64 estimated peak memory 1..268435456; u32 budget class 0..2, optional accelerator preference 0/1, required validation level 0..2 |
| License/provenance and review | license text, provenance text, use classification u32=1 app redistribution/use reviewed, review u32=1 approved, attribution-required u32 0/1, attribution text |
| Payload count and ordered records | u32 1..8; each leaf-name text, format u32, positive u64 size and raw SHA-256 digest |

Input is `[1,3,H,W]` RGB in [0,1]; output `[1,1,H,W]` foreground probability in
[0,1], same H/W. Negative normalization offsets/dynamic dimensions/quantization
are unsupported in v1; this small baseline does not promise every future model
fits. The parser validates declarations; actual ONNX/model graph matching,
operator inspection and output value validation must precede future provider load
in Step 2. A signed package is not graph/operator/task-quality certification.

Provider record: u32 provider ID, positive runtime version floor, positive API
floor, zero-based payload index, operator-set version=1; raw 32-byte canonical
ONNX source digest; u32 operator count; ordered u32 operator IDs. Provider IDs:
1 CPU ONNX, 2 optional Core ML, 3 optional LiteRT. Exactly one CPU record is
mandatory and first. CPU source digest equals its ONNX payload digest; converted
providers reference that same source digest. Payload formats use the same IDs,
with `.onnx`, `.mlmodel`, `.tflite` suffixes respectively. No format chosen from a
filename alone. Provider compatibility is declarative metadata, not execution or
OS availability certification. Unknown IDs/opset versions reject.

Step 3 approved Option A clarification: the CPU ONNX record authenticates portable
ONNX semantic compatibility, not a requirement for physical CPU execution.
`android.vulkan.v1` may consume that same authenticated record ONLY for the exact
`pixaura.onnx.reference-segmentation.v1` profile, with unchanged version-1 floors
and mandatory CPU reference compatibility. This is not permission for arbitrary
providers or general ONNX execution. Vulkan availability/validation/certification
remain separate. No Vulkan record, field, payload or package-schema extension is
introduced; canonical signed bytes and Core ML/LiteRT records remain unchanged.

Operator registry v1 IDs 1..16: Conv, Relu, Add, Sigmoid, Mul, Resize, MaxPool,
AveragePool, Concat, Reshape, Transpose, Softmax, BatchNormalization, Identity,
Clip, GlobalAveragePool. Actual graph coverage and model format/operator-set
inspection belong to Step 2; declarations cannot bypass them. Runtime/API version
floor numbers are provider-specific monotonic integer identities frozen with that
provider's later contract, not an Android/Apple policy duplicated here.

Budget classes 0 baseline / 1 balanced / 2 quality are declarations. Tier,
backend availability/validation, occupancy and pressure are evaluated later by
shared admission; unknown facts cannot promote. Optional acceleration never removes
the mandatory CPU-compatible baseline; no package grants a backend trust level.

## Detached signature and exact signed bytes

`signature.pxs` is exactly 80 bytes: u32 magic `0x50585347` (PXSG), u32 signature
version=1, u32 algorithm=1, u32 key ID matching manifest, then raw 64-byte signature.
No base64/hex text encoding. The sole signing message is **exactly manifest.pxm's
canonical bytes from byte zero through its declared length**, including metadata
magic/version. No external prefix, prehash, padding or added newline. Ed25519 is
pure detached Ed25519, never Ed25519ph/ctx. Unknown versions/algorithm reject;
no negotiation, fallback or unsigned acceptance. Every payload path/size/digest,
model/task/tensor/provider/capability/license/key/trust field is authenticated.
The detached signature bytes do not sign themselves. SHA-256 still independently
checks all payload bytes and establishes content identity.

## Trust-store representation and rotation

One shared native trusted store, never platform UI or untrusted package authority.
Internal representation version=1, monotonic generation u32, purpose 1/2, count
0..16; each key has u32 ID, purpose, activation generation, minimum model version
and exactly 32 public-key bytes. Unknown IDs, duplicate matches, inactive purpose/
generation or too-old model version reject. No package public key/certificate is
an anchor. Public keys pass libsodium `crypto_core_ed25519_is_valid_point` before
signature verification; malformed/noncanonical/small-order keys reject. Strict
`crypto_sign_ed25519_verify_detached` is used, ED25519_COMPAT absent.

Production store v1 generation 1 currently has ZERO anchors: deliberate fail-closed
until separately approved public-key provisioning. Public verification API does
not accept arbitrary caller keys or test trust. Reviewed compiled store updates
can add/rotate/remove keys with monotonic generations and model-version floors
without changing package semantics. Removal revokes that key; store/version
mismatches have a stable rejection code. Offline revocation is effective after
installing the updated app/store; no online freshness claim or remote key fetch.
No production private key/signing command/key generation exists in runtime.

Certification builds alone compile test purpose/anchor/corpus under
PIXAURA_CERTIFICATION_TESTING. The test seed is the publicly documented RFC 8032
seed, used only by the Node fixture tool. Release source preprocessing omits the
entire corpus and test entry point; actual production-config object scanning checks
for test key bytes, fixture identity and test symbol. Production always rejects
purpose-2 packages and cannot accept test keys through a caller trust argument.

## Fail-closed sequence and diagnostics

A bounded directory/type/count read; B normalized root/leaf paths; C bounded
positional parse; D package/manifest version; E exact canonical length/order/UTF-8;
F key-ID trusted lookup; G trusted-key point validation; H Ed25519 signature;
I exact payload set; J sizes and checked aggregate; K SHA-256 per payload;
L task/tensor/provider/license/capability semantics; M publish owned immutable object.
Structural parse errors needed to bound input occur before authentication; no
semantic authority or usable payload is exposed early. A missing signature may be
reported after trust resolution because the key check precedes signature handling.
All failures preserve package outputs. Diagnostics version=1, reason, bounded
entry ordinal and reserved=0 only; no paths, payloads, metadata, signatures or secrets.

| Code | Stable reason |
| --- | --- |
| 0 | OK |
| 1..5 | ARGUMENT, IO, UNSAFE_FILE, PATH, COUNT |
| 6..11 | MANIFEST_LIMIT, PARSE, VERSION, NONCANONICAL, ALGORITHM, SIGNATURE_VERSION |
| 12..16 | UNKNOWN_KEY, KEY_INVALID, SIGNATURE_SIZE, SIGNATURE_INVALID, TRUST_DOMAIN |
| 17..20 | PAYLOAD_MISSING, PAYLOAD_UNEXPECTED, SIZE, DIGEST |
| 21..24 | TASK, TENSOR, PROVIDER, LICENSE |
| 25..27 | RESOURCE, DUPLICATE, TRUST_VERSION |

License must be ISC, MIT, BSD-2-Clause, BSD-3-Clause, Apache-2.0 or CC0-1.0;
provenance nonempty; explicit reviewed app redistribution/use and review status=1;
attribution flag 0/1, nonempty attribution when required. Missing/unknown/ambiguous
state rejects. Signed metadata attests review; it does not prove actual legal rights.
Production model redistribution/license selection remains a later explicit gate.

## Platform and acceptance evidence

Additive checked C API in model_package.h: verify, metadata-info and exclusive
release. No exceptions/STL across C; explicit ABI/input lengths; no document or
policy authority. Kotlin debug JNI and Swift certification tests call the SAME
native corpus. There is no platform crypto fallback or inference provider yet.

Synthetic corpus covers valid signature, manifest/payload changes, key IDs/wrong
keys/signatures/lengths/scalars/points, unsupported versions/algorithm, duplicate
fields/paths, noncanonical/UTF-8/bounds, missing digests/files/signatures, extra
payload, wrong size/digest, traversal/device names, malformed tensors/overflow,
unknown task/provider/license and test trust in production. RFC 8032 independent
positive/negative vectors supplement Node-generated signatures. Real filesystem
checks exercise regular directories, hard links, symlink/Windows junction,
unsupported entry types and oversized manifest. No copyrighted weights, model
execution, AI UX, network, Phase 6 or new Schema 3 data. DH-APPLE-METAL-01 unchanged.
