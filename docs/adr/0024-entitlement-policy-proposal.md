# 0024 — Approved commercial policy and entitlement boundaries

Status: Accepted
Decision supplied by Product Owner in Phase11 Final Engineering Mission G8+G10,
2026-11-15. Principal Architect accepted v12 software within its certified scope.
This acceptance is commercial policy/direction, not Phase11 closure or store evidence.
Supersedes the prior proposed policy; original certified proposal is retained in Git
and docs/reports/phase-11-adr0024-proposed-history.md. Schema3/editing ABI unchanged.

## Context

Local photo ownership, native HUMAN Apply, source-bound single-use approval and
ordinary history remain authoritative. Quota or entitlement state never grants native
editing authority. No PixAuraAI account is required solely to edit. Completely offline
anti-abuse is limited; uninstall, storage rollback, rooted devices and lost trusted time
cannot be made fraud-proof without invasive identifiers, which are prohibited.

## Decision

Fourteen calendar days from the first successful committed edit; timestamp anchored
in UTC, expires at the same UTC time14 dates later. Trial removes Free photo/AI quotas.
Preview, failed/cancelled actions and mere requests never start or renew the trial.

Free access: five distinct immutable source identities per UTC calendar day across
opening/importing for editing, editing and ordinary export; three successfully delivered
valid usable AI proposals per day. Reopen/adjust/preview/history/export of an already
admitted source consumes no additional photo. Multi-action proposal counts once.
Rejected/malformed, resource-denied, failed, cancelled-before-result, stale requests
and redisplay consume no AI use. Future voice wake/transcription/clarification does
not independently count; Phase14 is planning only.

Paid subscription reference USD0.99/month and lifetime USD19.99 remove both quotas.
Storefront prices/product IDs require legitimate store configuration. Subscription
paid cache may extend only to the exclusive boundary last verified expiry+7days,
not seven days from a verification failure. Verified revocation/refund supersedes
older facts; renewal cancellation alone is not immediate revocation. Verified lifetime
works offline indefinitely until authoritative revocation/refund is available. No
unverified purchase callback, observation envelope or unsigned cache grants paid access.
StoreKit2 on Apple and Google Play Billing on Android are required platform adapters.
A minimal Android verification backend may handle strictly necessary store transaction
metadata, with authenticated signed results; no photos, conversation, voice, history,
analytics, quotas or PixAuraAI account. No deployment/account/credential invented.

Recovery viewing and export of the last approved saved revision are unconditional
routes outside commercial admission, including corrupt/unavailable ledger/cache,
quota exhaustion, expiry and verification failure. Never delete or lock user content.
An already admitted edit/export finishes; changes affect subsequent admissions only.
Commercial reservation is not document approval, revision publication or export Apply.

### Deterministic time and persistence

UTC00:00 is the daily boundary, labelled to users with local-time equivalent. Timezone
changes do not create a new quota day. Persist logical UTC high-water and last monotonic
uptime under serialized durable transactions. Within one boot, advance by elapsed
monotonic time; ignore untrusted wall-clock jumps/rollback. After reboot/uptime rollback,
retain the previous high-water, restart monotonic accumulation and flag time uncertainty
until authenticated time reconciles forward. Do not count powered-off time as proved.
Never move backward or grant multiple days by changing device date/timezone. Device
boot tokens are local ephemeral OS facts, not server identifiers or fingerprints.

Under uncertain time, new trial/subscription time-based grants fall back to remaining
Free access pending trusted reconciliation; verified lifetime and all recovery remain
available. Already authorized work finishes. This conservative offline limitation must
be presented plainly, not described as fraud-proof. First installation seeds local time;
reinstall/deletion of all local state cannot be securely distinguished offline.

Local protected/no-backup separate commercial database (format1, NOT project Schema3)
holds only current-day source digests/counters, pending leases and bounded recoverable
AI delivery capsules. A single exclusive owner serializes reserve/check/commit with
SQLite FULL durability. Pending reservations count against capacity; concurrent callers
cannot overspend. Native success/provenance proof is required before completion.
Failed/cancelled pending work releases capacity. Pending Free reservations carry
across UTC midnight and consume capacity until terminal; successful delivery is
charged to its actual logical UTC delivery day. A retry of the same native request
identity returns its original reservation, even across midnight. AI delivery backlog
cannot exhaust the separate ordinary-manual reservation path. A valid usable proposal capsule and
its one charge commit together before delivery; a crash before commit charges nothing,
a crash afterward leaves a recoverable usable result. Acknowledge clears the capsule;
old acknowledged tombstones/source hashes are pruned at rollover. Never send these
records to a server. No unbounded user usage history or photo bytes in this ledger.

Secure paid caches use platform Keychain / Keystore authenticated storage, environment,
product and source-store binding and verified generation/version evidence. Cache failure
falls back to Free/recovery; never resets trial as a purchase benefit. Runtime integration
must coordinate the native success/delivery boundary; no UI-only quota enforcement.

## Alternatives

Local timezone resets and action-counting photos are rejected: timezone gaming and
repeated-action charges violate approved policy. Request-time AI charging is rejected.
A broad cloud account/analytics service is rejected. Unsigned Android client purchase
flags are rejected as verification. Minimal metadata-only verification is allowed.

## Consequences

Approved policy is now frozen. Product IDs, exact store price/tax configuration,
store accounts/contracts, signing and authentic sandbox outcomes are external gates.
Family purchase sharing/cross-platform entitlement sharing is not enabled by default;
any extension requires a separate decision. SDK dependency privacy must be reviewed.
Standalone component tests do not establish deployed end-to-end quota enforcement.
New native admission/durable-delivery bridge must preserve the frozen ABI or receive
explicit versioned compatibility approval before production wiring.

## Validation

Require source dedup across all journeys, concurrent cap stress, rollback/cancel/fault
and real process-crash recovery, midnight/timezone/reboot/device-date tests, trial14day
and exclusive7day grace boundaries, lifetime/revocation and unconditional recovery.
Both platforms require authentic signed store sandbox pending/purchase/restore/renewal/
expiry/refund/revoke/offline/cache recovery evidence. Synthetic facts/keys are tests only.
Official APIs: [StoreKit verification](https://developer.apple.com/documentation/storekit/verificationresult),
[Play security](https://developer.android.com/google/play/billing/security),
[Play SDK releases](https://developer.android.com/google/play/billing/release-notes).
