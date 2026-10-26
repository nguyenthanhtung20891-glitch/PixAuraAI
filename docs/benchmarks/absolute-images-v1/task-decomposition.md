# Absolute-image task decomposition — engineering evidence v1

Phase 7 Step 7 Round 4. No tool/schema semantics or provider capabilities change.
A = deterministic evidence plausible; B = learned context likely beneficial;
C = unsuitable for automatic absolute recommendation in this MVP. A does NOT mean
safe production correction: observability of a condition is not desired user intent.

| Existing tool | Class | Observable evidence | Ambiguity / failure modes | Single-photo sufficiency / learned role |
|---|---|---|---|---|
| adjust.exposure | A, limited | Linear luminance percentiles, clipping, dynamic range | Night, low-key, snow, white background, silhouettes; clipped data cannot be recovered | Detect extremes; cannot uniquely infer desired exposure. Scene/subject context and abstention needed |
| adjust.brightness | B | Midtone occupancy and local readability | Duplicates exposure evidence; scene reflectance and style confound mean intensity | No unique exposure-versus-brightness decision; a contextual defect classifier could help avoid stacking |
| adjust.contrast | B | Histogram spread and local contrast | Fog, flat illumination, backlight, deliberate high contrast | Metrics observe spread, not its desirability. Context likely valuable; no generic histogram target |
| adjust.highlights | A, limited | Near-white occupancy and spatial extent | Lamps, specular reflections, white objects; sensor clipping versus rendered clipping | Warn about broad lost detail; cannot infer recovery or whether highlights should be lowered |
| adjust.shadows | A, limited | Near-black occupancy with bright regions and local texture | Silhouette, night, dark material; noise amplified by lifting | Can describe deep shadows; correction needs intent/subject/noise context |
| adjust.saturation | B | Chroma distribution / channel clipping | Colored objects, monochrome style, neon and vivid flowers | Neither low nor high chroma proves a defect; learned content may reduce false normalization |
| adjust.temperature | B | Channel balance, neutral-region candidates, illuminant statistics | Sunset/stage light, mixed light, pigment; camera-rendered sRGB has unknown prior white balance | Single global illuminant can be ill-defined; context/neutral references helpful. RGB estimate is NOT Kelvin tool value |
| detail.sharpen | B | Edge/high-frequency energy, spatial focus, noise proxies | Low texture, shallow depth, deliberate softness, noise, resize artifacts | Need subject/focus/noise discrimination; thumbnail cannot certify native-resolution sharpness. Sharpen cannot repair defocus |
| detail.blur | C | Blur itself can be measured | Whether additional blur is desirable is stylistic and spatial | Explicit/manual goal only; remains in Registry, excluded from automatic intelligence |
| transform.rotate_quarter | C | Some horizon/text/face cues | Unreliable orientation, intentional composition, no semantic ground truth | Preserve explicit/manual flow; no automatic inference authorized |
| transform.crop_normalized | C | Saliency/composition cues | Subjective framing and irreversible field-of-view choice | Preserve explicit/manual flow; no automatic inference authorized |

## Feasibility boundary

Deterministic measurements are useful read-only CONDITION EVIDENCE for UI advice.
An all-abstaining policy can avoid false positives but does not demonstrate useful
correction coverage. A darker image is not proof of underexposure; a low-frequency
image is not proof of blur. No statistic or global MOS supplies an action direction
or trustworthy parameter by itself. Learned assessment is optional, and must earn
its role by reducing these specific confusions against a human-reviewed baseline.

This round's one-image baseline does not consume Round 3 paired references. It uses
fixed heuristic confidence plus abstention, emits at most one ordinary typed record,
and never constructs executable Plan/Envelope or touches document/history authority.
The recommended future stack is measurements + optional calibrated contextual
defect evidence -> uncertainty/abstention -> bounded shared deterministic parameter
policy -> UNTRUSTED Envelope v1 -> existing validation/Plan/Registry -> HUMAN review
and approval -> existing execution/publication. This is a research direction only.
