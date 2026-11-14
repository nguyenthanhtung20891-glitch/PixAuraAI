# Phase 11 resource rejection analysis and bounded resolution proposal

Baseline8387b167514b4a01f331ef05db32f22d5b4f9f5b. Existing XR raw measurements
remain unchanged; no new physical benchmark or Metal certification is required
for this source analysis. CPU/GPU admission and all numerical limits stay frozen.

## Observed result and source trace

XR actual T0,128MiB runtime cap: 6MP import succeeds, preview priming rejects8;
12MP import rejects8; both recover through a real small-image manual Apply.
These are correct failure/recovery results, not successful large-image editing.

working.cpp defaults permit8,388,608pixels and134,217,728bytes for a single
RGBA32F working raster. layout validates both before normalization. A4000x3000
working raster needs192,000,000bytes, exceeding BOTH limits; decode allowing
larger RGBA8 input does not authorize its RGBA32F working representation.

A3000x2000 working raster needs96,000,000bytes. Editing/reference evaluation must
retain the original and reserve full-size output/intermediates, then display and
platform transfer buffers. Even source+one output alone need192,000,000bytes,
above T0's134,217,728byte cap. decode_api.cpp preflight includes retained bytes,
full raster, evaluation scratch and bounded display/transfer capacities under
the existing owner mutex. budget.cpp accounts retained+inflight+new peak; no
allocation may claim a small display as its full-size working buffer.
editor_api.cpp's gesture render uses this path. Therefore reducing preview edge
alone does not make this6MP editing case fit. Resource8 is expected here.

Sources: packages/core/src/working.cpp, budget.cpp, decode_api.cpp, editor_api.cpp;
contracts/budget-admission-v1.md. No new process RSS ceiling is proposed.

## Requirement inconsistency and proposal (NOT APPROVED)

QUALITY_GATES asks for12MP JPEG export within30sT0/10sT1, while the current
end-to-end working representation cannot admit12MP on ANY tier because its
single-raster hard maximum remains8M/128MiB. Raising a tier or successful direct
encoder timing cannot satisfy that end-to-end product requirement. It stays OPEN.

Preferred investigation proposal: bounded CPU streaming/tiled decode/evaluation/
export with aggregate peak admission, immutable original ownership and unchanged
pixel/reference semantics. Stream output instead of materializing a192MB raster;
account tile halos, multi-pass/intermediate lifetimes, geometry coordinate mapping,
ICC/alpha/orientation, cancellation, stale fences, and all caller buffers. A tile
size is chosen only by proven peak accounting under the existing cap, never by
ignoring retained memory. This is NOT authorized for implementation by this report:
it conflicts with the current whole-working-raster contract and needs a versioned
architecture/compatibility proof and full numerical oracle coverage for every
supported tool/stack before adoption. It must preserve ABI/Schema3 or obtain the
required separately approved compatibility path. No GPU/provider substitution.

Alternatives requiring explicit Owner/Architect decisions are a documented change
to the12MP product requirement, or a separately represented imported proxy with
clear original/proxy lineage and truthful export dimensions. Neither is adopted.
User-prepared smaller input is a recovery workaround, not12MP qualification.
No downsampling, clamping, target relaxation or ceiling increase is implemented.

## Additional device acceptance worksheet

For each supported LOW/MID/HIGH Apple AND Android cohort, record the selection
rationale, actual native capability tier and caps (do not infer from model name),
physical model/OS/build, clean source/CI, release/testability/instrumentation flags,
input bytes/dimensions/hash, power/battery/ambient, pressure and thermal states,
concurrent workload and cooling method. Use synthetic assets only.

1. Preserve cold open/import UI-response and decode timings separately. Profile UI
   frame deadlines separately from native operation timing. Document startup/cache
   state; do not label cold timing as warm preview.
2. Run5warmups+100retained independent operation cycles for each admitted size;
   retain ordered raw samples and p50/nearest-rankp95/max. Stop and record denied
   cases; verify smaller Manual Apply/Cancel, Undo/Redo and export recovery.
3. Observe native leases, post-close memory trend and process high-water with method
   overhead disclosed. Pressure/fault injection is distinct from real OS warnings,
   storage-full termination and restart. Never fill unrelated user storage.
4. Record sustained thermal state/throttling/cooling with explicit duration and
   sample chronology. No sustained qualification follows from a short nominal run;
   agree the acceptance protocol where no duration/thermal threshold is specified.
5. Compare only applicable published targets:60Hz framep95<=16.7ms; T0 warm720px
   previewp95<=150ms; import UI response<=100ms excluding decode; UI cancel
   acknowledgement<=250ms.12MP JPEG30sT0/10sT1 remains blocked until the admitted
   end-to-end path is architecturally resolved. No unsupported case becomes PASS.

Do not rerun accepted DH-APPLE-METAL-01 or promote provider trust for these CPU/UI
measurements. Additional physical cohorts and operator/tool permissions remain
external prerequisites; hosted/Simulator measurements do not replace them.
