# ADR 0023: Export is a projection of a committed revision

Status: Accepted
Date: 2026-11-04
Scope: Phase 9. Export of an ordinary committed revision.

## Context

Phase 8 closed the review journey and the MVP gap audit found no export, save, or share path. ADR 0011 admits vendored libjpeg-turbo 3.2.0, libpng 1.6.59, and zlib 1.3.2 for bounded SDR decode and leaves export undecided. The owned PNG configuration disables writing. JPEG encoder sources remain inside the common library and are not exposed.

## Decision

Export is a deterministic projection of one authoritative committed revision. It is not an edit, an approval, a candidate, or a revision. It does not change undo, redo, or Schema 3.

The raster is the existing evaluation of that revision followed by the existing preview render. No second editing implementation is added. Encoded bytes use the already vendored JPEG and PNG libraries. Vendor files stay unchanged. A later implementation step may enable PNG writing only in the owned `pnglibconf.h`. No new codec dependency is admitted.

Output is the display-oriented 8-bit sRGB raster. Orientation is already applied by working normalization, so the file does not carry a second orientation transform. GPS, EXIF, XMP, and IPTC are not copied. ICC bytes are not embedded, and this step does not claim profile preservation. JPEG has no alpha and flattens straight alpha over opaque white. PNG writes the evaluated straight RGBA samples and no ancillary text.

Android and Apple choose the user destination. Native code returns caller-owned encoded bytes and does not publish them.

## Alternatives

A new codec dependency was rejected because libjpeg-turbo and libpng are already vendored with reviewed licenses and checksums. Platform encoders were rejected because they would split pixel and metadata policy. Writing a new encoder was rejected because it would add an unreviewed attack surface. Copying original metadata, including location, was rejected because the export must not leak GPS through an unexamined container copy.

## Consequences

ADR 0011 remains the decode authority. Historical documents stay historical. Physical device, Vulkan, and Metal trust are unchanged. The owned PNG configuration may later enable writing. Vendor sources stay unchanged.

## Validation

Contract tests must reject a preview, a proposal, or a missing revision as an export source. Later native tests must show JPEG and PNG bytes, stripped location metadata, no history mutation, and release of encoded buffers after failure or cancellation. Both exact-SHA workflows remain mandatory. This decision does not certify an encoder.
