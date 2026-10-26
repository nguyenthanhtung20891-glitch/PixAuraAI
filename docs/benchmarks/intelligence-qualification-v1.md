# Offline intelligence qualification v1

Engineering evidence only; no production admission, provider registration, editor
authority, mobile integration or model acquisition. Expected semantics, prototype
text, metric bins and policy were fixed before scoring. The JSON specification is
the exact executable policy. Source and corpus identities are bound by
`intelligence-qualification-v1-corpus.json`; decoded RGB identities are separately
bound by `intelligence-qualification-v1-pixels.json` for zlib-independent tests.

Specification SHA-256:
`e85003aa98caa91f09d498f768e1fe91df059c6835db99abbccf8cd90c3bbd06`.
Executed frozen corpus SHA-256:
`f12417d5ec208192eebf0af5620d1bee676fe0369ca2146ca79984ad9c3760bb`.

## Corpus and scoring limitations

Project-owned procedural chart and illustrated scene, 224x224 RGB sRGB, eight
authorized intelligence goals, six variants each: positive, opposite, neutral,
weak boundary, combination and opposed-half conflict. Total 96 cases, 46 positive
and 50 abstention targets. Sharpen's opposite/noise case expects abstention, not a
new blur action. Blur, quarter rotation and crop are NOT SUITABLE FOR THIS PROVIDER
FAMILY in this qualification policy and are not tested; explicit/manual tools stay
unchanged. No natural photos or external dataset were acquired.

Perturbation direction is objectively known, but these synthetic bases are not
perceptually reviewed neutral photographs. Expected native-unit values are fixed
conservative bins, not exact inverse edits or perceptual optimality labels.
Exposure/brightness overlap, highlight occupancy and scene color can confound
these fixtures. Results establish behavior on this corpus only.
**INSUFFICIENT REALISTIC QUALITY EVIDENCE** remains the final semantic admission
limitation. No category is production-qualified.

For each explicit goal, three frozen local prototypes represent positive,
opposite and neutral evidence. Cosine minimum 0.15, top-two margin 0.02, agreement
with independent bounded read-only image metrics and a conflict veto are required.
The predeclared sensitivity margins 0, 0.01, 0.02 are reported without selecting a
post-hoc winning policy. Embedding components never become editing parameters.
Each output is a validated existing integer parameter or abstention; maximum one
action. These are experimental policy constants, not admission thresholds.

The existing `deterministicBaseline` receives the same explicit tool/value request
for each image, never expected labels. Its contract is request echo, not image
analysis; false positives here describe lack of automatic abstention, not a bug in
that certified baseline. A metric-only ablation distinguishes image metrics from
embedding contribution. Candidate adapters never receive fixture expectations.

Per tool, report selected positive count, correct direction, true abstention,
false positives/negatives, invalid output, repeatability and absolute parameter
error in that tool's native units. Missing emitted-positive samples have null
calibration error, never zero. No cross-unit parameter aggregate is defined.

## Actual execution

Existing local artifact only, LiteRT-LM API 0.18.0, CPU text/vision one thread,
audio backend disabled, 70 vision tokens, normalized 768-dimensional embeddings.
Two image inferences per case, 24 static prototype embeddings. Runtime-owned image
preprocessing is not claimed equivalent to upstream processor configuration.
One initial engine setup failed because its cache child directory did not exist,
before any semantic inference. The sole correction created that directory and
resolved the model path. Corpus pixels, labels, prototypes and policy remained
identical. No completed semantic run was rerun or tuned after seeing results.

Reproduction with already legally acquired local assets (no downloads):

```sh
python3 scripts/qualify-intelligence.py --output build/qualification \
  --model build/phase7-step7-audit/embeddinggemma-2-text-vision-440m.litertlm \
  --runtime-root build/phase7-step7-audit/runtime --node node
python3 tests/intelligence-qualification.test.py
```

The runner requires Linux `/proc` and `resource` for host diagnostics. CI executes
the regression without model dependencies, reproducing pixels, calibration,
recorded score decisions and summaries. That regression does not claim to rerun
the actual model. Model bytes, caches and input PNGs stay ignored under build/.
Raw bounded score triples and actual timings are retained as JSON evidence.
