# Phase 9 Step 2 — Native deterministic export engine

Step 1 closure `49ddb7fc9ebc8173e716a9cd3f8a25d18564b3ab` is observed:
Foundation `41911149886` SUCCESS 7/7 and Native shells `41911149642` SUCCESS 3/3,
attempt 1, every required job SUCCESS. The Step 1 freeze-time block remains
CLOSURE CERTIFICATION PENDING. This report does not recertify it.

## Engine

`pixaura_export_bind` copies the loaded epoch and current committed revision.
`pixaura_export_encode` accepts only that epoch and a 32-hex revision that
`result_find` can see. It replays that revision, evaluates with
`pixaura_working_evaluate_cancel` when the stack is not empty, and renders with
`pixaura_preview_render`. An empty stack renders the original handle. The
encoded bytes are copied to the caller only after the epoch and revision are
checked again. A mismatch returns stale code 9 and leaves the caller buffer
unchanged.

The editor gate is held for the whole call. A second call returns busy code 8.
There is no queue, retry, or background export. `pixaura_export_cancel` signals
the in-flight cancellation token without taking the editor gate. Stop makes a
later encode return 13. Destroy makes a later encode return 3.

Export does not call apply, approve, or execute. It does not call the Assistance
Provider. Document serialization after a successful encode matches the
serialization from before that encode.

## Encoder

JPEG uses vendored libjpeg-turbo 3.2.0. PNG uses vendored libpng 1.6.59 and
zlib 1.3.2. No new dependency is added. Vendor sources are unchanged. Owned
`pnglibconf.h` enables the write switches the existing write sources need. The
encoder never sets text, EXIF, or ICC chunks.

JPEG is baseline, quality 1–100, integer DCT, `optimize_coding` false. Straight
alpha is flattened over opaque white. PNG is 8-bit straight RGBA, filter none,
compression level 6. Output capacity is 64 bytes through 64 MiB. Codec scratch
is 32 MiB. ICC preservation is not claimed. Location metadata is not copied.

EXACT uses full display dimensions. FIT uses the existing preview fit and does
not upscale.

## Tests

`export_test` covers the imported revision, a manual crop, an AI exposure
revision, a crop-plus-rotate revision, undo/redo selection of an older
committed revision, JPEG quality, PNG identity and chunk privacy, stale epoch,
busy overlap, cancellation, and stop. `export_c_consumer` checks the request
layout and version rejection.

`14330b8057105095a8cf8a9749ecd40af87c0250` is not certified. Foundation
`41924079674` failed: GCC rejected misleading indentation in the export test,
and MSVC did not export the encode symbols while their definitions sat inside
the anonymous namespace. The definitions now match the other boundaries and
stand at file scope. That SHA is not rerun.

`3799fe2b1bff41d8fc4c7cfad034e96306b6c475` is not certified. Foundation
`41926053091` completed attempt 1 with apple-boundary job `126910623261`
failed. Windows, Ubuntu, both Android ABIs, sanitizer and workflow-lint
succeeded. Native shells `41926053135` completed attempt 1 with ios-app job
`126910623388` failed; android-app and source succeeded. Both Apple failures
are the same link: `CPixAuraCodecs` omitted the PNG write and zlib deflate
sources that `codec-sources.cmake` already lists, so `png_write_*` and
`png_free_buffer_list` were undefined. Those existing vendor files are now in
`Package.swift`. That SHA is not rerun.

`64d854d7ec2a5e11787cd0b94ff60b0fdfe7a8e3` is the certified implementation SHA.
Foundation `41928862538` SUCCESS 7/7 and Native shells `41928862545` SUCCESS 3/3,
both attempt 1, exact SHA, every required job SUCCESS. The receipt is
`docs/reports/evidence/phase-9-step-2-implementation-certification.json`.
This documentation commit does not certify itself. Step 2 remains
CLOSURE CERTIFICATION PENDING until that separate SHA is observed.
Steps 3-6 are not started.
