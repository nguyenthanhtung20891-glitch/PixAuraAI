# Encoded image admission and bounded decode, version 1

Status: frozen for authorized Step 4 under [ADR 0011](../adr/0011-shared-bounded-sdr-decode.md), 2026-10-04. Implementation evidence is separate.

The authoritative contract is ADR 0011 plus [decode.h](../../packages/core/include/pixaura/decode.h). Shared C++ admission consumes only bytes retained by a digest/length-verified private asset read. Persisted caller metadata alone does not authorize allocation. Source/image handles belong to the decode family, not the document registry. Every successful source has immutable bounded structural metadata; entropy is validated when decoding. No metadata-only success is a successful pixel decode.

Orientation values follow EXIF: 1 identity; 2 horizontal reflection; 3 180 degrees; 4 vertical reflection; 5 transpose; 6 clockwise 90 degrees; 7 transverse; 8 clockwise 270 degrees. Encoded dimensions/raster never change in Step 4; display dimensions swap for 5-8. No platform auto-orientation. Invalid/duplicate explicit orientation rejects, absent defaults to 1. Future normalization must implement these transforms once before evaluation; no geometry tool is implemented here.

Profile type 0 means absent/default sRGB; 1 means retained ICC; 2 means explicit PNG sRGB. PNG gAMA/cHRM are structurally checked and retained in the verified immutable original; no color transform is applied. Unsupported HDR signaling rejects. Profile bytes remain untrusted data even after structural admission. Query exposes only fixed fields and a SHA-256, not arbitrary text. No metadata is logged.

Calls are synchronous/background-only. Init/destroy must not race calls or release/free context storage. Platform owners join/quiesce workers before destruction; live calls/release on one context synchronize. Different contexts execute independently. Failure preserves source state, pixels, handles and caller outputs; releasing a source does not invalidate a retained image. Double release/destroy rejects deterministically. No serial wrapping/reuse or process-global mutable decoder state.

Bounds, supported variants, errors, allocation model, profile/EXIF limits and non-goals are specified in ADR 0011. C API pointer accessibility and truthful lengths remain caller obligations; forged dangling pointers are not memory-safe inputs to any C interface. ABI 1 remains unchanged.
