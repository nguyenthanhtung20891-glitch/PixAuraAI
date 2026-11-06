# Phase 10.1 Step 4 — Mac Studio hardware performance

Status: LOCAL HARDWARE GATE BLOCKED.

No Mac Studio sample was collected. There is no image dimension, sample
count, median, or memory figure for this step. The hosted 2 by 3 export
samples in the Phase 10 Step 5 report remain hosted samples.

A future Mac run measures import/decode, evaluation, preview, review
metadata, Apply, JPEG and PNG export, repeated edit/export, cancellation,
and a Simulator lifecycle on images larger than the 2 by 3 fixture, with
warmup, sample count, median, spread, and allocation notes. The record
includes the SoC string from `sysctl`, the OS from `sw_vers`, and the
compiler from the Xcode `clang` used by `scripts/check-apple.sh`.

Those numbers would be macOS host evidence. They would not be iPhone
battery, thermal, or Metal backend certification. `apple.metal.v1` stays
UNKNOWN. DH-APPLE-METAL-01 stays DEFERRED / NOT WAIVED / NOT SATISFIED.
