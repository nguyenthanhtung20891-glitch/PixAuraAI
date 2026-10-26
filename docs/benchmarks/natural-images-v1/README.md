# Round 3 natural-image relative-signal qualification v1

Offline engineering only; no production provider, model admission or absolute
auto-correction claim. Corpus acquisition and scene review preceded all candidate
inference. Thirty-two individually CC0-labelled WordPress Photo Directory sources
are recorded in sources.json with author/page/asset/license URLs, acquisition date,
original and normalized hashes. Large originals and captured page evidence remain
ignored under build/phase7-step7-round3. Compact RGB8 PNG derivatives are committed.
CC0 photo licensing does not imply model/property releases or absolute legal
guarantees for every depicted object. No scraping of search-result images occurred.

Eight scene strata have four bases each: indoor architecture, landscape, urban,
food/product objects, forest/nature, water, low light and flowers. There is no
portrait-focused set; skin-tone/portrait generalization is not established. Photos
were visually reviewed as real photographs, not labelled for artistic correction.
One near-duplicate cupcake and unsuitable keyword matches were excluded before
the deterministic split. SHA256(PixAuraAI-Round3-split-v1:source-id) orders each
stratum: two development and two holdout, 16/16. Derivatives never cross splits.

The source normalization is a fixed GDI+ bicubic center-square crop to 224x224.
Pinned derivative bytes, not cross-decoder equivalence, are authoritative for the
experiment. Color-profile/orientation reproduction outside that derivative is not
claimed. Transform calculations use decoded sRGB to linear RGB and quantize back
to RGB8. Config and runner define exact transformations and native parameter bins.

Seven levels per tool/base: -1, -0.6, -0.2, 0, 0.2, 0.6, 1. Positive means the
known imposed condition needs the frozen positive correction, not necessarily a
positive parameter. Exposure scales intensity; brightness adds an offset; contrast
scales around 0.18; highlights/shadows scale fixed luminance regions; saturation
scales distance from luminance; temperature applies opposing red/blue gains;
softness blends with three 3x3 blurs, opposite adds high-frequency residual. Clipping
and quantization are recorded limitations, and inverse edits are not claimed
perceptually optimal. Sharpen's opposite is a relative negative signal, with no
authorized opposite automatic action. Blur/crop/rotation remain excluded.

Two preselected bases in each split also receive eight opposed-half conflicts
(expected abstention) and eight two-condition combinations (primary-tool direction
expected). Stress counts are small and do not establish broad conflict robustness.

This is a PAIRED experiment: both methods compare a transformed image against the
same unmodified base. Neither method sees expected labels at prediction time.
This tests RELATIVE SIGNAL RECOGNITION. Original-reference access is not generally
available to an arbitrary-photo production provider. An unchanged pair has exactly
zero evidence by construction: its false-positive rate is NOT an absolute neutral
photo false-positive estimate. ABSOLUTE AUTO-CORRECTION STILL UNQUALIFIED.

The cheap baseline measures paired changes in eight documented read-only scalar
statistics. The candidate measures paired change along one contrastive semantic
axis per tool, using the existing Round 2 positive/opposite prototype texts. No
prototype/prompt search or other experimental policy variants are planned. Each
method fits one zero-intercept least-squares slope per tool on regular DEVELOPMENT
cases only, mapping evidence to signed perturbation strength. Stress labels are
excluded from fitting. Predictions below 0.10 magnitude abstain; fixed strength
bins 0.25/0.75/1.25 select bounded native-unit recommendations. All emitted values
are checked against frozen golden Registry bounds. No embedding dimension becomes
an edit value. This policy is experimental and non-authoritative.

Development results, exact runner/config/source bindings and fitted slopes must be
committed BEFORE holdout execution. The holdout runner verifies those bindings and
creates an exclusive started marker before runtime load. It cannot silently repeat
the holdout into the same output directory. No holdout-dependent tuning is allowed.
The existing explicit reference provider runs on the identical explicit requests;
it is context, not image understanding. Development scores are in-sample fitting
results, never independent qualification evidence.

Report per-tool direction, weak/strong accuracy, missing recommendations, paired
reference false positives, invalid output, native-unit errors, domain/stress results
and repeat consistency. Confidence intervals resample 16 BASE photographs with
replacement, not their correlated derivatives; 2,000 deterministic replicates,
seed 73003, percentile intervals. Small stratum/stress counts limit inference.
No aggregate parameter error across incompatible units or statistical-significance
claim is authorized. Cost is host CPU engineering evidence, not a resource lease or
physical-device admission. Production model remains UNRESOLVED.

The decision gate is the Principal Architect's Round 3 incremental-value rule:
continue only for robust useful advantage beyond the cheap metric baseline, such
as multiple useful categories, substantially better abstention/conflict behavior
or clear complementary information. Marginal numerical differences do not justify
the candidate's artifact/runtime cost. Equal/better metric results, fragile tuning
or no robust incremental information imply DISQUALIFY_CANDIDATE. A hold requires a
specific remaining evidence question, not reluctance to reject. No universal pass
percentage or production threshold is invented here.
