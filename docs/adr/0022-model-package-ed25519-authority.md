# ADR 0022: Shared Ed25519 model-package signature authority

Status: Accepted; dependency selection accepted for libsodium 1.0.22 (ISC).
Date: 2026-10-13

## Context

Phase 5 Step 1 requires authenticated bounded local model packages across native,
Android and Apple. Existing SHA-256 storage identity is not signature authentication.
Product Owner explicitly approves one shared portable Ed25519 verifier, but requires
candidate dependency audit and selection before incorporation.

## Decision

Use pure Ed25519 with explicit fixed algorithm ID/version in package v1. Unknown
algorithm/version rejects; no negotiation/fallback. One shared native semantic
authority handles trust lookup and signature verification behind checked C/JNI/
Swift boundaries. UI has no cryptographic authority. Runtime contains public keys
only; production signing/private keys stay outside the app and repository artifacts.

Trusted versioned store resolves exactly the manifest key ID; package-supplied
keys and filenames cannot establish trust. No remote key fetching, unsigned dev
fallback, warning-only acceptance or execution before complete verification.
Rotation uses reviewed store updates without changing package semantics. Test
anchors must be structurally excluded from release runtime.

The exact canonical bytes include all model/task/tensor/provider/license/capability
metadata and every payload size/digest. Repository SHA-256 remains payload identity;
signature validation cannot replace file-set/size/digest checks. Detailed byte
schema and implementation are finalized under model-package-v1 before code work.

## Alternatives

Independent native-platform verification could diverge and is not selected.
Product Owner accepts the [dependency audit](../reviews/phase-5-ed25519-dependency-audit.md)
and selects libsodium 1.0.22, ISC. Approved official archive SHA-256:
`adbdd8f16149e81ac6078a03aca6fc03b592b89ef7b5ed83841c086191be3349`.
Monocypher is not selected. Exact preserved source hashes/build configuration
are recorded in vendor provenance and the frozen model package contract.

## Consequences

One shared verifier prevents platform-specific acceptance semantics. Static,
hidden portable libsodium verification and point-validation supply this authority;
no signing/key-generation API is exposed by PixAuraAI. Full generic configuration
retains point validation, which upstream minimal mode omits. Sanitizers retained.

## Validation

Retain bounded canonical parsing, stable reasons, immutable verified ownership and
cross-platform negative/positive signature parity. Step 1 remains IN PROGRESS;
both complete exact-SHA workflows are required. No provider execution, schema
change, AI UI, Steps 2+ or Phase 6 authorization follows from this decision.
DH-APPLE-METAL-01 unchanged. No historical accepted ADR is rewritten.
