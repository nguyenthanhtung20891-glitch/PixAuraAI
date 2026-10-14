# libsodium 1.0.22 — approved verification dependency

Approval: Product Owner accepts the Phase 5 audit and selects libsodium 1.0.22
under ADR 0022. ISC license; all upstream embedded notices and AUTHORS retained.
Official archive SHA-256:
`adbdd8f16149e81ac6078a03aca6fc03b592b89ef7b5ed83841c086191be3349`.
[Audit](../reviews/phase-5-ed25519-dependency-audit.md),
[source provenance](../../packages/core/vendor/libsodium/provenance.json).

No system library, prebuilt substitution, Monocypher, floating branch or runtime
network service. Source bytes are unchanged and verified by CMake and Node gates.
CMake/Swift compile the same preserved source list from upstream Makefile.am:
full portable generic C sources, scalar fe25.5, required AArch64 NEON initialization
closure, SODIUM_STATIC and CONFIGURED=1. No ED25519_COMPAT, Ed25519ph usage, optional
ISA flags/assembly, -march=native or LTO. Static PIC/hidden vendor symbols.
Full configuration retains `crypto_core_ed25519_is_valid_point`, absent from the
upstream minimal configuration. Owned package API calls point validation and pure
detached verification only; runtime exposes no signing, key generation or private
key loading. `sodium_init` initializes trusted dependency state once, never keys
or model/provider policy. Sanitizer builds instrument vendor and owned wrapper.

Swift's private implementation module exposes an empty marker header; vendor API
headers are implementation-only. Xcode retains private extern linkage for the
static vendor object aggregate without publishing vendor symbols. Android and
Swift call shared native authority; neither implements platform crypto fallback.

Distribution: full upstream LICENSE/AUTHORS are incorporated into
`ThirdPartyNotices.txt` shipped as an Android asset and Swift package resource.
Upstream source headers retain their individual ISC/CC0/public-domain notices.
No new license UI is introduced. Build outputs and downloaded archive remain ignored.

Footprint observations are in the Step 1 report and model-footprint JSON evidence;
archive/object size is never represented as an exact linked delta. No tiny/zero-cost
claim. Required release-object and actual release-product scans exclude test keys
and certification symbols. Production trust starts empty pending separate approved
public-key provisioning. RFC test signing seed exists only in the engineering tool.

MSVC upstream common.h automatically enables intrinsic dispatch references. The
shared CMake/Swift source list therefore also retains the twelve unchanged guarded
upstream AESNI/Blake2/Argon2/ChaCha/Salsa implementation files needed to close those
references. Other compilers leave them inactive under the same generic flags.
No optional ISA flags, native tuning, assembly, vendor edits or new PixAuraAI APIs.
