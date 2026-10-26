# Absolute-image feasibility v1 — Round 4

OFFLINE engineering evidence only. No provider, model or correction is admitted.
EmbeddingGemma remains DISQUALIFIED FOR THE CURRENT PIXAURAAI MVP; Round 3 is frozen.

## Independent review protocol

Reuse the 32 individually CC0-documented Round 3 bases at their pinned 224 RGB8
derivatives. Sixteen deterministic degraded variants broaden feasibility evidence;
recipes preserve base identity/domain. They are not 16 independent natural scenes.
No new external photos are acquired. Original rights and normalization limitations
remain in ../natural-images-v1/sources.json. Pixel hashes bind every reviewed image.

The reviewer is Codex's visual engineering assessment of the base contact sheet and
all 16 variant images, before measurements/results. This is a SINGLE AI-assisted
engineering review, not independent human consensus, photographic expertise,
Product Owner acceptance or objective ground truth. Recipe identities are visible;
review is NOT blinded to construction. Review is independent of the evaluated
metric outputs and thresholds. No correction labels are derived from those metrics.
Transformation direction alone is insufficient: visibly plausible low-key/sunset/
selective-focus cases remain ambiguous or abstain, even when deliberately altered.

For each image/tool labels.json records positive or negative (native parameter
direction), acceptable (no sufficiently safe technical recommendation; abstain),
or ambiguous (excluded from quality scoring). Confidence and rationale are explicit.
Temperature positive means higher Kelvin; sharpening has no negative recommendation.
Labels concern technical visibility/detail/color plausibility, never a preferred
artistic style. Alternative tools can address similar tonal defects; per-tool missed
counts are coverage diagnostics, not an assertion that every labeled tool must fire.
Parameter targets are NOT labeled: native-unit calibration error is NOT SCORED.
No exact inverse or irrecoverable clipping restoration is promised.

Protocol, labels, recipes, baseline and policy are SHA-bound before first scoring;
an exclusive ignored start marker records those bindings and the execution time.
The final evidence commit retains the marker without claiming a prior commit.
No development/holdout claim: all 48 images are a SMALL provisional feasibility
review, not a new locked natural-photo admission benchmark. The old Round 3 split
is not reused as fresh independent holdout. No threshold tuning after scoring.
Future integration requires separate human reviewers, disagreement/adjudication,
full-resolution context, scene/skin/noise/intent coverage and an untouched test set.

## Absolute baseline

Each call receives ONE fixed-size RGB8 image, never a corresponding reference.
Shared read-only sRGB-to-linear measurements: luminance percentiles/mean/spread,
near-black/white occupancy, saturation, red-blue imbalance and adjacent edge energy.
Policy v1 makes at most ONE typed recommendation, or abstains. Priority is extreme
global darkness, extreme brightness, widespread white, then deep shadows with a
bright region. Exact thresholds are predeclared in policy.json. Confidence is
HEURISTIC_LOW, an uncalibrated evidence tier, not a probability. These are intentionally
inspectable feasibility rules, not production quality/admission thresholds.

Brightness/contrast/saturation/temperature/sharpen decisions abstain: these metrics
alone cannot determine intent, illuminant, noise or focus. Their measurements remain
inspectable. Exposure/highlights/shadows can also be misleading in legitimate scenes;
the experiment measures that failure, rather than treating histogram normalization
as correctness. No blending or edit execution occurs. Fixed bounded numeric values
are checked against existing Registry goldens; no editing math/schema is changed.

Report per-tool direction, misses/wrong direction, false positives, abstention,
excluded ambiguous emissions, invalid output, per-domain and natural/derived strata.
No universal percentage gate or mixed-unit parameter score is invented. Determinism
is checked by reproducing outputs; cost includes Python preprocessing. Working memory
is bounded by 50,176 RGB pixels, but process RSS remains diagnostic only. No mobile
qualification, learned quality, integration or authority is inferred from this run.

## Candidate screening

landscape.json and candidate-landscape.md distinguish original research, actual
implementation/weight identity, code and weight licensing, reproducibility, output
fit, cost, maintenance and unverified deployment paths. At most three learned families
may be shortlisted. Public benchmark scores are not PixAuraAI evidence. No learned
inference is claimed without execution; unknown size/hash/runtime facts stay unknown.
