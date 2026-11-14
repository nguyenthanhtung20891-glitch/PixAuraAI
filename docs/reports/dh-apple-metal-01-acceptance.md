# DH-APPLE-METAL-01 — independent acceptance receipt

Decision date: 2026-11-14. **DH-APPLE-METAL-01 = PASS.**
Phase 11 remains AUTHORIZED / IN PROGRESS, not accepted or closed. Phase 12 NOT STARTED.

## Decision authority and scope

The Product Owner supplied the independent Principal Architect decision after
reviewing the sanitized v9 archive: **ACCEPTED for the physical Apple Metal hardware
certification scope**. Engineering records this external review; it is not an
engineering self-review or an independently invented approval.

Accepted physical target: iPhone XR / iPhone11,8, iOS 18.7.9 (22H355), Apple A12 GPU.
Exact tested source: 33c809f6a4b91300491cc1e211a10dbdbccaf7f9.
Actual pipeline creation, GPU dispatch, command completion/readback, all 14/14
shared CPU–GPU numerical parity cases, fallback disabled and GPU use required;
physical hardware XCTest 1/1 PASS, physical app unit suite 16/16 PASS.
The frozen [GPU tile v1](../contracts/gpu-tile-v1.md) and [ADR 0018](../adr/0018-physical-gpu-certification.md)
requirements and numerical tolerances are unchanged. Aggregate parity was emitted;
per-case max/ULP errors and a separate GPU trace were not emitted or invented.

Acceptance covers this implementation, tested target and 14-case contract. It does
not prove all Apple devices, performance qualification, inference-provider admission,
production readiness, other backends or completion of G7/G8/G10/G11.

## Integrity independently recomputed before recording PASS

Reviewed ZIP SHA-256:
`bd8b8d75622e715a85b457be824c17bc39a156463fa62dc8ef95fcd0efe6ad8a`.
The archive is phase-11-v9-independent-evidence-33c809f-sanitized.zip under the
local ignored build/phase-11-iphone-xr directory. It was extracted unchanged;
verify_integrity.py passed for the complete package, then a separate verification
implementation independently recalculated all 48 payload hashes and the manifest.
No certified test was rerun for acceptance recording.

Every original artifact payload matches provenance/original-artifact-audit.json.
Original hardware.xcresult: **32 files, 297814 bytes**, tree SHA-256
`10b906ee525aa37f05ed31860cdf0d30e398cf95bee225bc2176b50b2c61e411`.
Original controller log SHA-256:
`7443cd0cdadeb9f1fc99ef9ce993f9d5de901fe69ad1933c9b4eaa5b7e1ce688`.
Original hardware JSON attachment equals the reviewed attachment byte-for-byte.
The clean-run commit.txt equals the tested SHA; included source snapshots match
immutable Git blobs at that SHA. Exact attachment hashes and every artifact result
are in [the machine-readable receipt](dh-apple-metal-01-acceptance-v1.json).

Tree hash method: recursively sort file paths, form entries with relative path,
SHA-256 and byte count, serialize with Python json.dumps(entries, sort_keys=True),
then SHA-256 that serialization. Byte counts and file counts are also verified.
The previously disclosed conventional Simulator database.sqlite3 derived index is
excluded only for its original39-file payload comparison, which matches exactly.
No such exclusion applies to hardware.xcresult. No original evidence was changed.

## Source and existing CI independently reverified

Fresh GitHub API reads confirm every required job completed SUCCESS, attempt 1,
exact tested SHA and correct workflow IDs/job sets:

| Workflow | Run | Result |
| --- | --- | --- |
| Foundation boundaries (372386439) | [43539539110](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/43539539110) | 7/7 SUCCESS |
| Native application shells (373262165) | [43539539102](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/43539539102) | 3/3 SUCCESS |

The reviewed archive receipts match the freshly fetched implementation receipts.
The documentation successor must pass its own complete 7+3 workflows on its own
exact SHA; its observed receipt is delivered separately to avoid a self-reference.
No test assertions, production runtime, ABI, schema, numerics, signing identities,
profiles or source-fingerprint evidence are changed by this decision recording.
The primary owner's pre-existing local signing/format project changes stay unstaged.

## Trust and outstanding Phase 11 gates

- cpu.reference.v1 = VALIDATED.
- apple.metal.v1 = UNKNOWN; no production Metal provider/backend enabled or promoted.
- android.vulkan.v1 = UNKNOWN; no cross-backend inference admission follows.

Hardware certification and provider admission are separate. Any future production
Metal validation/enablement must satisfy the existing separate admission policy.
Historical DEFERRED/NOT SATISFIED reports remain immutable freeze-time evidence;
this reviewed successor discharges this hardware gate, without waiving it.

G7 device privacy/traffic/OS-permission observations remain outstanding. G8 needs
Apple/Android cohorts, sustained thermal/pressure/UI targets and resolution of the
12MP target versus frozen admission limitation. G10 needs binding entitlement policy,
implementation and legitimate sandbox evidence. G11 needs human VoiceOver/TalkBack,
focus, contrast and alternative-input observations. Duo/iOS27.2 local UI remains
15/19 PASS with four behavioral failures; conventional same-source comparison is
4/4 PASS. These target-specific findings are not resolved or reclassified.
No Phase 11 acceptance/closure, production readiness or Phase 12 authorization.
