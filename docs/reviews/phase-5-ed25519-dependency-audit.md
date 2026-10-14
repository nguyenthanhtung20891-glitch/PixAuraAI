# Phase 5 Step 1 — portable Ed25519 dependency audit

Status: REVIEW ACCEPTED / libsodium 1.0.22 SELECTED AND INCORPORATED.
Reviewed 2026-10-13 against baseline
`098ac7890de5bed61b7c4823fd5ea3b1ef82c904` and Product Owner's Ed25519 decision.
The Product Owner accepted this audit and pinned libsodium 1.0.22 under ADR 0022.
The comparison below preserves selection evidence; it is not a claim that either passes PixAuraAI runtime
or cross-platform gates. Downloaded audit inputs remain ignored under
`build/phase5-crypto-audit`. Reviewed libsodium source is now preserved under
`packages/core/vendor/libsodium` with per-file SHA-256 provenance. Monocypher was
not incorporated. Static portable CMake/Swift builds retain point validation;
full generic source configuration is required because minimal omits its public API.
Compiler-mandated AArch64 NEON initialization closure is included, with no optional
ISA flags/assembly/COMPAT/LTO/native tuning. Vendor symbols hidden, bytes unchanged,
sanitizers retained; public PixAuraAI surface exposes no signing or key generation.

## Accepted recommendation and historical comparison

Recommend **libsodium 1.0.22**, ISC licensed, default strict Ed25519 detached
verification with `ED25519_COMPAT` forbidden. Its supplied rejection checks
better match the package trust contract without owned curve-validation code.
Monocypher 4.0.3 remains materially different: substantially smaller source
integration and straightforward sanitizer testing, but intentionally permissive
point encoding/order checks require an additional reviewed rejection layer.
The required Architect/Product Owner selection was obtained before incorporation:
libsodium 1.0.22 approved; Monocypher prohibited. Runtime/certification evidence is
tracked in the Step 1 report rather than inferred from the comparison.

## Evidence and comparison

| Criterion | Monocypher 4.0.3 | libsodium 1.0.22 |
| --- | --- | --- |
| License/provenance | BSD-2-Clause OR CC0-1.0; propose BSD-2-Clause with retained notices; official Monocypher release | ISC; retain LICENSE and applicable notices; official signed release tag/archive |
| Maintenance | Upstream 4.0.3 released 2026-06-15 with EdDSA/Ed25519 timing fix | Upstream 1.0.22 released 2026-04-10; supported stable branches and platform/security maintenance |
| Maturity | Historical June 2020 Cure53 audit; sanitizer/formal-analysis infrastructure; audit is not of 4.0.3 | Upstream static/dynamic analysis, cross-implementation vectors, documented desktop/mobile packaging; historic external audit is not certification of our build |
| Ed25519 API | Optional `crypto_ed25519_check`, SHA-512; default BLAKE2b EdDSA API is NOT Ed25519 and must not be used | `crypto_sign_ed25519_verify_detached`, pure Ed25519; no incremental Ed25519ph API in package v1 |
| Rejection semantics | Shared equation checker deliberately permits noncanonical A/R encodings and low-order points | Default source checks scalar canonicality, small-order R, canonical/non-small-order public key, curve decode and reconstructed-R equality; do not enable compatibility macro |
| Malformed trusted key | Requires separate strict validation policy before trusting key; stock verifier alone insufficient for proposed strict contract | Validate trust-store key with documented `crypto_core_ed25519_is_valid_point` where available in selected build; detached verifier itself rejects canonicality/small-order/decode failures |
| Portability/ABI | Portable C99, C++ compatible; two C sources and two headers; no OS adapter | C ABI, Windows MSVC/MinGW, Linux, Android and Apple build support documented; larger platform configuration surface |
| Dependency depth | Core plus optional SHA-512/Ed25519 module, standard C headers; no runtime service/network | No required external crypto/network service; platform OS facilities and initialization/RNG support are part of full library; no package key generation or remote lookup |
| Bounds/allocation | Fixed verifier input sizes; caller validates lengths; no implied checked ABI | Crypto operations are allocation-free except unrelated pwhash/guarded allocation APIs; initialization belongs to trusted library owner, never manifest authority |
| Narrowing | No upstream verification-only switch: core includes unrelated crypto; function sections/hidden visibility/dead stripping can reduce binary, must measure | `--enable-minimal` reduces surface but is NOT verification-only; hide symbols and expose only package API; dead stripping must be measured, not assumed |
| Deterministic integration | Preserve four exact upstream files and notices, integrity checked before configure; no source edits | Exact release archive/files/configuration, static build, pinned configuration; avoid floating stable branch and prebuilt platform mismatch |
| Sanitizers | Upstream explicitly runs ASan/MSan/UBSan/Valgrind | Installation guide cautions about sanitizer-induced side channels, while internals documents ASan/UBSan CI; diagnostic public-input verifier tests must stay instrumented, shipping flags separate |
| Network | None in verifier | None in verifier |
| Size evidence | Four source/header files total 136728 bytes; archive 940390 bytes | Archive 2008529 bytes, 814 archive entries; broader source/build system |

These are source/archive footprint observations, NOT linked mobile binary-size
measurements. Windows/Linux/Android/Apple compile/runtime and sanitizer compatibility
are not yet executed for either candidate. Mobile object/link size deltas must be
recorded after approved integration. No arbitrary binary-size claim or sanitizer
exemption is authorized by this review.

## Pinning evidence

Monocypher official archive:
`https://monocypher.org/download/monocypher-4.0.3.tar.gz`

- Locally measured SHA-256:
  `8cc9bc341a66249016db9bd70e9142d8d0aef9945973744b1ac05dbc55d8ee66`.
- Locally matched SHA-512 against upstream checksum sidecar:
  `40904ada5c7ee4f7741733e38b69a30a4b0561cbffba5ffe7c2dce16136d540251ec0d9056ff606510d3b5b708fb8a40db7e0870d4a0b2dc17ba2bfb880f8965`.
- Exact extracted release source hashes (SHA-256):
  `src/monocypher.c`: `57eb914fc88136119bd41655cccb8c250048bf54d470540625186f8ab16f64be`;
  `src/monocypher.h`: `c494da712122da7ff679fdcf318a5317e84972b6c950fe9d896212947797facd`;
  `src/optional/monocypher-ed25519.c`: `60fce3578fb00b00da96490653d993c4cb427b1e1be38183285c66e04d22cc18`;
  `src/optional/monocypher-ed25519.h`: `abc4fad381879f5c29176ebe014b9189956b3dfe0a3e36459b6990bc57212380`.

libsodium official archive:
`https://github.com/jedisct1/libsodium/releases/download/1.0.22-RELEASE/libsodium-1.0.22.tar.gz`

- Locally matched SHA-256 against official release asset digest:
  `adbdd8f16149e81ac6078a03aca6fc03b592b89ef7b5ed83841c086191be3349`.
- Proposed build: static PIC, minimal where required APIs remain available,
  default strict verifier, `ED25519_COMPAT` absent, no LTO, hidden vendor symbols,
  native target compiler/ABI; no `-march=native` for cross-platform artifacts.
  Final per-build-system flags/source list and per-file hashes must be recorded
  when integration is approved. Do not extract a guessed ref10 subset or modify
  third-party source to reduce warnings. No floating `stable` or `latest` input.

Checksum comparison establishes reproducible reviewed bytes, not independent
authentication when archive and checksum share a source. Upstream release
signatures are available for libsodium; no upstream signature verification was
executed in this audit. Monocypher archive inspection extracted source files, but
full Windows extraction reported documentation-link failures; no compile or test
PASS is inferred from partial extraction. No audit files will be committed.

## Security findings and required controls

1. Monocypher's base EdDSA API is BLAKE2b, not the approved SHA-512 Ed25519.
   The optional Ed25519 function delegates to the permissive shared checker.
   A proposed wrapper would need explicit canonical encoding and low-order
   rejection tests, with no owned curve arithmetic or vendor modifications.
   This additional security surface is why small footprint alone does not decide.
2. Monocypher upstream lists a comparison-result timing branch in 4.0.3 for some
   compilers. For public package verification, result/message/signature/key are
   nonsecret; this is assessed as nonblocking for that use only, not a general
   cryptographic safety claim. Earlier Ed25519 timing issues require 4.0.3 pin.
3. libsodium 1.0.21 fixed a subgroup check in `is_valid_point`; pin 1.0.22 and
   independently test malformed/noncanonical/small-order trust anchors. No older
   system library or accidental compatibility mode may satisfy this contract.
4. Public keys originate solely from the trusted versioned store. Trusted keys
   still require structural validation. Package-provided keys never establish trust.
5. Signature/RFC and negative vectors must run through the same shared wrapper
   on all hosts/JNI/Swift. No platform crypto fallback or algorithm negotiation.
6. Retain active ASan/UBSan gates over the verifier and parser. Instrumented
   diagnostic binaries do not become shipping binaries; do not relax gates based
   on upstream sanitizer cautions. A genuine compile/runtime conflict is a decision
   gate rather than permission to disable instrumentation.
7. Vendor source may contain generic signing functions, but runtime has no signing
   API or private keys. Narrow hidden static linking and release symbol inspection
   must verify the reachable/public surface. Test signing uses separate fixture tools.

## Primary sources

- [Monocypher releases](https://monocypher.org/download/),
  [changelog](https://monocypher.org/changelog),
  [known bugs](https://monocypher.org/bugs).
- [Monocypher tagged license](https://raw.githubusercontent.com/LoupVaillant/Monocypher/4.0.3/LICENCE.md),
  [tagged README/build integration](https://raw.githubusercontent.com/LoupVaillant/Monocypher/4.0.3/README.md).
- [Ed25519 API](https://monocypher.org/manual/ed25519),
  [shared equation semantics](https://monocypher.org/manual/eddsa),
  [QA](https://monocypher.org/quality-assurance/),
  [historical audit](https://monocypher.org/quality-assurance/audit).
- [libsodium 1.0.22 release](https://github.com/jedisct1/libsodium/releases/tag/1.0.22-RELEASE),
  [tagged license](https://raw.githubusercontent.com/jedisct1/libsodium/1.0.22-RELEASE/LICENSE),
  [tagged verifier source](https://raw.githubusercontent.com/jedisct1/libsodium/1.0.22-RELEASE/src/libsodium/crypto_sign/ed25519/ref10/open.c).
- [libsodium platform installation](https://doc.libsodium.org/installation),
  [signature API](https://doc.libsodium.org/public-key_cryptography/public-key_signatures),
  [allocation/testing internals](https://doc.libsodium.org/internals).
