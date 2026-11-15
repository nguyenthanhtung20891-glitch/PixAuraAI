# Phase 11 G8/G10 engineering submission v13

Baseline: eb61735be29eb365db76fa57b74170c6656c9efd. Product Owner supplied
independent Principal Architect acceptance of v12 software within its certified
scope, then approved the new commercial policy and tiled architectural direction.
This is engineering evidence, not independent acceptance of v13 or phase closure.
Final successor SHA and its own7+3CI must be recorded in the observed delivery receipt.
Until then the implementation is UNCOMMITTED / NOT CERTIFIED. Baseline CI cannot
certify these changes. Phase11 remains IN PROGRESS; Phases12–14 planning only.

## Approved policy and implementation boundaries

ADR0024 and MONETIZATION now record14 calendar days from first successful committed
edit; Free5 distinct immutable sources/day and3 successfully delivered usable AI
proposals/day; paid/trial remove quotas; reference USD0.99monthly/USD19.99lifetime;
offline subscription grace at most7days after verified expiry; verified lifetime
offline indefinitely subject to authoritative revocation. Recovery inspection and
last approved saved revision export bypass commercial failure while retaining native
ownership checks. No store products, prices, credentials or purchase outcomes invented.
The exact prior proposed ADR is preserved in Git and the hash-checked history snapshot.

packages/commerce implements an independent native SQLite format1, separate from
project Schema3. Exclusive owner + serialized transactions reserve capacity, deduplicate
source identity and native request identity, and atomically store an opaque usable-
delivery test capsule with one charge. Pending work carries across midnight, success
charges its delivery day and retries keep the original token. Failed/cancelled/rejected
work releases capacity; post-delivery cancellation cannot refund a successful use.
A full AI inbox does not consume the separate ordinary-manual reservation path.

Tests cover concurrent cap stress,100 repeated actions on a source, SQLite FULL
rollback, real _Exit/restart before and after successful storage, UTC midnight,
timezone/wall changes, boot/uptime rollback, trial/grace boundaries, verified lifetime
versus expired subscription, verification failure, revocation, in-flight completion,
corrupt clock rejection and unconditional commercial recovery. Time tests use modeled
OS/trusted inputs plus real process restart/TZ changes, not physical clock observations.

Production completion/store-fact methods are private to the proposed native commercial
owner; no such owner is implemented or enabled in the apps. The test probe supplies
synthetic opaque capsules, NOT authentic store transactions or executable native Plans.
The frozen review contract explicitly makes approval/candidate/Plan authority ephemeral.
An atomic native receipt/inbox reconciliation contract is required before real editing
can be coupled safely to quota persistence. See commercial-admission-v1-proposal.md.
No UI-only paywall/enforcement is enabled and this is not a shipped Free-tier claim.

## Store components and privacy

Apple: actual StoreKit2 verified-only transaction/product/purchase/restore/update
adapter; allowlisted products/environment and purchased ownership scope; protected
Keychain snapshot with ThisDeviceOnly accessibility and no synchronization. No catalog
is configured, adapter is not instantiated by editing UI, and no store action was run.
Simulator Keychain tests verify actual protection attributes, bounds and cleanup.
Unsigned tests failed -34018; standard account-free ad-hoc Simulator identity resolves
it. Only Simulator app tests use that identity; generic device builds remain unsigned.
No development team, certificate, provisioning profile or owner project edit changed.

Android: signed P256 metadata verification and app-scoped Keystore AES-GCM atomic
cache, multi-product preservation, stale/tamper/key-loss recovery and pending-state
non-erasure. Real Google Billing9.1 SDK adapter compiles in a non-shipping module;
actual SDK construction/close is tested in the testAPK without transport/location
runtimes. The shipping app does NOT depend on that SDK module. Billing's published
POM and bytecode include a CCT transport-send path; no telemetry-free SDK claim is made.
Pinned artifact SHA256d28286d4e4c18725510980de01211d9b72501e7146d96817fa0c9999af9b0d22;
license Android SDK License, official Google Maven provenance, strict locked checksums.

A non-shipping HTTPS metadata client validates endpoint/TLS/no-redirect behavior,
bounds requests/responses and worker queue, verifies nonce/product/token/package/
environment signatures and stops callbacks on close. The metadata-only Google Publisher
verifier function uses exact allowlists and partial-response masks, never logs/stores
photos, usage histories, identities, tokens or analytics. Operator OAuth/signing key
providers are required; none supplied or generated. Subscription revocation evidence
must be authenticated; unknown/default fails closed rather than confusing expiry with
refund/revoke. RFC3339 fractional expiry is floored to whole seconds without extending
store authority. Four protocol tests use synthetic keys/responses, NOT sandbox evidence.

There is no deployed backend, configured checkout UI, integrated commercial admission,
legitimate product catalog, authenticated store lifecycle result or production activation.
Those boundaries are explicit, not reclassified as PASS. Family/ownership/store billing-
grace configuration and signed sandbox lifecycle qualification remain necessary.

## G8 bounded CPU proof

The versioned tiled proposal preserves the12MP requirement and identifies exact
incompatible whole-raster/original-handle/geometry/export contracts. A normalized
RGBA32F pointwise/local-halo evaluator reuses accepted kernels with strict arithmetic,
cumulative halos, stage-by-stage true-edge clamping, shared aggregate reservation
leases, cancellation/staleness and staged-output abort. It is not linked into production
editing/import/export and cannot authorize a revision. Geometry is explicitly rejected.

161 bitwise parity cases cover seven adversarial extents and pointwise/local stacks,
all six tone kernels at low/neutral/high, alpha/signed-zero/tiny/HDR samples and tile
seams. Independent5x5 binomial impulse, concurrent lease rejection/release, malformed
cancellation binding, allocation/reader/sink failures and late cancellation are checked.
Synthetic4000x3000 traversal validates all12,000,000 output pixels with no full float
raster. Reservation high-water629,440bytes for that test owner is not process RSS,
codec/source occupancy, end-to-end import/export or physical performance qualification.

Original native kernels/public headers113 fingerprints remain unchanged. No cap raised,
no downsample, ABI/schema reinterpretation or GPU/provider admission. A new immutable
reader/stage-plan/stream-writer representation needs explicit compatibility acceptance.
Original XR raw timings/rejections stay at original source; no new physical benchmark
is claimed. Existing v1 import can still reject12MP; the product requirement stays OPEN.

## Observed local validation

Raw logs/results: build/phase-11-final-* (not immutable historical packages).

| Check | Observed result |
| --- | --- |
|Final native host CTest|69/69 PASS,135.07s|
|Final ASan/UBSan CTest|71/71 PASS,236.87s; active negative probes retained|
|Swift ShellTests|18/18 PASS on Simulator; no physical/store transaction claim|
|Generic iOS|Debug/Release builds and release/model scans PASS|
|Android|Full66/66 PASS with original transport preparation; final4/4 component checks, Debug/Release/JVM/lint PASS; new URL test adds one case for hosted full suite|
|Android shipping graph/release scans|No Billing/CCT/location dependency in release runtime; release isolation PASS12native libraries/2DEX|
|Metadata verifier|4/4 synthetic protocol tests PASS|
|Leaks|Tile0bytes leaked, reported peak7472K; final ledger0bytes leaked, reported peak4352K. Host leak-tool observations, no mobile RSS/admission claim|
|Source/provenance|215/215 local source checks PASS including isolated tracked-new-file inventory; additive v13 preserves prior records|
|Cross compilation|Both new C++ components compile for iOS ARM64 and Android ARM64/ARMv7 (6/6); compile-only, not linked mobile execution. Receipt: build/phase-11-final-cross/receipt.json|

Earlier failures are retained: naive multi-pass halo clamping; foreign database
mutation and sqliteX name-filter adversary; unsigned Keychain identity; missing host
transport argument; fractional store expiry. Appropriate before/fix logs are retained;
none is retroactively called PASS. Initial restricted native runs had Mac Metal
status15; unchanged host integration test passes with host GPU access. This is not
DH-APPLE-METAL-01 execution or provider admission.

Read-only device discovery found the paired XR/iOS18.7.9 record with developer mode
enabled but tunnel unavailable. No reachable physical XR session was established.
Same-machine virtual records are not physical qualification. Device identifiers remain
private in build evidence. No signing/account changes or new physical results claimed.

## Acceptance blocker matrix

| Gate | Independently reviewable software evidence | Remaining binding prerequisite |
| --- | --- | --- |
|G7|Source/privacy boundaries, scoped protected caches, foreign-file preservation, negative inputs and sanitizer/release checks|Actual signed-device traffic/permissions/backup/pressure/restart observations and independent acceptance|
|G8|Normalized CPU subset parity, bounded leases, measured host cleanup; exact tiled compatibility proposal|Architect approval of new reader/geometry/stream-output integration; full implementation/compatibility proof then LOW/MID/HIGH Apple/Android real performance/thermal/pressure qualification|
|G10 policy|Approved Owner rules recorded; deterministic native quota component, actual SDK/cache/verifier components|Reviewed native success-receipt/advisory-inbox/clock bridge before application enforcement; legitimate catalogs/store config/accounts; authenticated revocation service/keys/endpoint and real signed sandbox lifecycle evidence. No production activation yet|
|G11|Existing accepted automated UI/localization evidence retained; fresh hosted app regressions required on successor; operator runbook ready|Actual human VoiceOver/TalkBack/focus/contrast/keyboard/switch observations|
|CI/review|Local evidence above; baseline is independently accepted|New clean successor must pass exact Foundation7/7+Native3/3, then independent Principal Architect review; not phase closure|

DH-APPLE-METAL-01 PASS/CLOSED accepted XR scope unchanged and not rerun.
cpu.reference.v1 VALIDATED; apple.metal.v1 UNKNOWN; android.vulkan.v1 UNKNOWN.
No formal closure SHA. Phase11 IN PROGRESS; Phases12–14 NOT AUTHORIZED to implement.
