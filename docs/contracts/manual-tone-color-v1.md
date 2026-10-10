# Manual tone/color numerical contract v1

Scope: Phase 3 Step 3. All seven tool IDs equal their `pixaura.<name>` operation
IDs; operation_version=1 and parameter_version=1. CPU reference is authority.
These recipes are PixAura's initial explicit recipes, not a claim to reproduce
another editor's sliders. No final UI or new GPU kernel is authorized.

## Integer parameters

| Tool | Parameter | Unit | Inclusive range | Exact neutral/default | Step |
| --- | --- | --- | --- | --- | --- |
| Exposure | milli_ev | thousandths EV stop | -5000..5000 | 0 | 1 |
| Brightness | milli_linear | thousandths normalized linear-white offset | -1000..1000 | 0 | 1 |
| Contrast | milli_stops | thousandths log2 contrast slope | -2000..2000 | 0 | 1 |
| Highlights | milli_ev | thousandths highlight EV stop | -2000..2000 | 0 | 1 |
| Shadows | milli_ev | thousandths shadow EV stop | -2000..2000 | 0 | 1 |
| Saturation | milli_ratio | thousandths chroma multiplier | 0..2000 | 1000 | 1 |
| Temperature | kelvin | assumed daylight white correlated color temperature, K | 4000..25000 | 6504 | 1 |

Brightness spans one normalized linear-white unit in either direction. Contrast
and selective exposure span quarter to fourfold strength. Saturation spans zero
chroma through double chroma. Temperature uses the CIE daylight locus's supported
4000..25000 K domain; it is a daylight correction, not a tungsten/blackbody or
arbitrary temperature slider. No tint control or image-dependent estimation.

Each operation contains exactly its listed parameter, a JSON integer in plain
decimal notation. No floating JSON, exponent, numeric strings, missing/extra
parameters, locale parsing or implicit defaults. Invalid parameters reject (7),
unknown tuple rejects (5); never clamp parameters. Shared native descriptors are
the only platform authority. An unversioned recipe/unit/range change is forbidden.

## Working domain and rounding

Input is canonical finite premultiplied RGBA32F linear-sRGB/D65, including finite
negative and above-white RGB. Alpha must be in [0,1], and zero alpha requires zero
RGB. Every operation preserves alpha bits. Neutral operations bypass all math,
preserving all pixel bits. Valid zero-alpha pixels bypass changed color math.
No RGB/output clamp, gamma conversion, auto enhancement or source overwrite.

Read RGB/alpha as binary64 exactly. Decimal constants below mean nearest IEEE
binary64 values. Every primitive multiply/add/subtract/divide rounds to nearest
binary64, with no FMA, reassociation or fast math; dot products evaluate
`((m0*c0)+(m1*c1))+(m2*c2)` in that order. Convert each completed operation's RGB
to nearest binary32 once. No integer conversion occurs in evaluation. Parameter
conversion to binary64 is exact. FE_TONEAREST is required; another mode rejects.
Nonfinite intermediates or a final magnitude above FLT_MAX reject the complete
candidate (7), preserving source and previous displayed preview.

Use the existing CPU reference bound `1e-7 + 2e-6*abs(expected)` against an
independent high-precision per-stage oracle; supported underflow additionally
allows 1e-44. Neutral/alpha identity is bit exact. Ordered composed versus
sequential evaluation with identical per-operation rounding is bit exact on the
same platform. This does not claim universal cross-toolchain bit equality.

## Recipes

Let premultiplied color be C, alpha a, and G(e) the already-frozen exposure table
and exact power-of-two scaling for `2^(e/1000)`. Exposure 1/1 is unchanged.

Brightness: `C' = C + a*(milli_linear/1000)` independently per channel.
Contrast: `p=0.18*a; C' = p + G(milli_stops)*(C-p)` independently per channel.
The fixed pivot is normalized 18% linear gray; no scene statistics or gamma pivot.

For selection and saturation, `Y=((0.2126*R)+(0.7152*G))+(0.0722*B)` on premultiplied
RGB. Exactly equal R/G/B uses Y=R, preserving exact grayscale chroma neutrality.
Let L=Y/a for nonzero alpha. Selection weights use smoothstep
`S(t)=(t*t)*(3-(2*t))`, with t explicitly bounded to [0,1]. Bounding this selector
does not clamp image channels. Highlights: `t=bound((L-0.18)/0.82); w=S(t)`.
Shadows: `t=bound(L/0.18); w=1-S(t)`.
Both: `factor=1+w*(G(milli_ev)-1); C'=C*factor` per channel. The weighting uses
the current operation's input luminance. Highlights do not change pixels below
the pivot; shadows do not change pixels at/above the pivot. Black remains black.

Saturation: `s=milli_ratio/1000; C'=Y+s*(C-Y)` per channel. Zero is grayscale;
1000 is exact bypass; 2000 doubles chroma. Premultiplied luminance/chroma are
linear, so no division by tiny alpha is needed for saturation.

Temperature: calculate daylight white W(T) in XYZ, normalized Y=1:

```
u=1/T
T<=7000: x=(((-4607000000*u)+2967800)*u+99.11)*u+0.244063
T>7000:  x=(((-2006400000*u)+1901800)*u+247.48)*u+0.237040
y=((-3*x)+2.87)*x-0.275
W(T)=(x/y, 1, ((1-x)-y)/y)
```

Use fixed sRGB-to-XYZ matrix M with rows:
`(506752/1228815,87881/245763,12673/70218)`,
`(87098/409605,175762/245763,12673/175545)`,
`(7918/409605,87881/737289,1001167/1053270)`.
Bradford matrix B has rows `(0.8951,0.2664,-0.1614)`,
`(-0.7502,1.7135,0.0367)`, `(0.0389,-0.0685,1.0296)`.
Inverse M/B coefficients are the nearest binary64 values of the exact inverses
of those specified rational/decimal matrices, pinned as hexadecimal constants.

Compute `src=B*W(T)`, `dst=B*W(6504)` and component ratios `dst/src`.
Evaluate `C'=inverse(M)*inverse(B)*diag(dst/src)*B*M*C` through those successive
matrix-vector stages, in that exact order, retaining binary64 until final RGB
conversion. This is assumed-source-white correction toward the reference white:
higher Kelvin settings warm the result, lower settings cool it. T=6504 bypasses
the entire matrix chain exactly, avoiding round-trip drift. No unpremultiply is
needed because the transform is linear. The pipeline's nominal working white
remains D65; this creative operation does not reinterpret stored source profiles.

Daylight equations/domain: [Academy ACES documentation](https://docs.acescentral.com/white-point/).
Linear sRGB/XYZ and Bradford background:
[W3C CSS Color 4](https://www.w3.org/TR/2026/CRD-css-color-4-20260227/).
External standards inform this initial recipe; future standards edits cannot
change this version's pinned constants or operation behavior.

## Ordering, history, preview and storage

Every stack executes in stored order, with binary32 RGB rounding after each
operation. No collapsing repeated operations, removing existing neutral records,
folding exposure sums or reordering geometry/color. Crop/rotation retain outward
rasterization and clockwise quarter-turn semantics. Geometry changes the extent
against which subsequent geometry is evaluated; tone is pointwise on that extent.

Reuse the manual gesture and PRV1 contracts: BEGIN -> UPDATE/PREVIEW* -> one
changed detached revision proposal, or CANCEL without history mutation. Zero
updates, neutral append or unchanged replacement commits do not grow history.
Stale source/session/revision results reject; interruption and tool switching
cancel; admission/allocation failure preserves the last valid retryable pending
state. Preview failure preserves the previous valid display. Application owners
still authorize, persist and publish; a gesture never grants durable approval.

New durable operations require schema 2 through explicit migrate(1,2), never
automatic upgrade. Schema 1 and all old operation meanings remain immutable.
Canonical manifest schema 1, SQLite authority, source hash, bounded history and
all raster/context/memory ceilings are unchanged. No network/plugin/script/UI.
