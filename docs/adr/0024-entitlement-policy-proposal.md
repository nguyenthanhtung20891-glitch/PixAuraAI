# 0024 — Cross-platform entitlement and recovery policy (PROPOSED)

Status: PROPOSED / NOT APPROVED / NOT IMPLEMENTED AS AN ACCESS GATE.
Authority required: Product Owner commercial policy and Principal Architect review.
This document does not approve seven-day grace, family sharing or new cloud services.
No real purchase or sandbox outcome is claimed. Schema 3/editing ABI unchanged.

## Context — existing baseline and architecture

MONETIZATION.md specifies a 14-day local app-access trial beginning at the first
successful edit, reference USD $0.99/month and $19.99 lifetime, no consumable AI
credits, independent platform purchases, recovery viewing/export after expiry,
no interruption of in-progress work, and indefinitely offline verified lifetime.
Prices require approved localized store configuration. There is currently no
StoreKit, BillingClient, verifier or secure entitlement cache in either app.
The native editing/history/approval core remains authoritative for edits; licensing
must never grant Plan approval, publish a preview or interrupt stop/join/destroy.

Smallest design: platform store adapters -> verified immutable observations ->
shared documented access projection -> platform presentation. Keep store identifiers,
signatures and credentials out of image/document storage, logs and exports. No new
native ABI or document schema. A future native edit-admission integration requires
separate architectural review; UI state is never native editing authorization.

## Decision — proposed rules, pending approval

| Topic | Proposal / preserved baseline | Approval or evidence still required |
| --- | --- | --- |
| Trial eligibility/start | First successful committed edit establishes start exactly once, not install/preview/failed Apply; capture UTC and monotonic elapsed evidence; reinstall limitation acknowledged, no invasive device ID | Precise eligible actions (including restore/import), reinstall/backup semantics and clock uncertainty handling |
| Trial expiry | 14-day elapsed eligibility, use monotonic evidence where available; rollback never deletes or locks recovery data | Across reboot/clock rollback policy; do not silently grant a new trial or choose a grace duration |
| Subscription | Only authenticated store evidence grants time-limited entitlement; cancellation of renewal is distinct from immediate expiration | Product IDs, renewal/store billing-grace treatment, transition policy and signed sandbox evidence |
| Offline subscription | Last verified expiry plus an explicit approved grace policy; unverified JSON never extends access | Proposed seven days is NOT adopted. Owner must approve duration, stale/unknown evidence and clock-risk behavior |
| Lifetime | Verified non-consumable entitlement cached securely, works offline indefinitely as baseline requires | Refund/revocation handling when next online and family-sharing policy; cannot promise detection while offline |
| Pending/unverified | No new entitlement from a pending purchase, local boolean, digest, unsigned cache or restore request | Approved recovery messaging; no false purchase success |
| Refund/revoke | Verified store updates supersede older evidence; retire stale generation/cache atomically | Retroactivity/family-sharing/account-switch policy; sandbox refund/revocation observations |
| Restore | Explicit user action, requery authoritative store state, merge by immutable evidence/generation; restore is not purchase | Accounts, IDs, environment/configuration, pending/error/offline behavior acceptance |
| Recovery | Existing project viewing and last approved revision export remain available after expiry and cache/verifier failure | Validate all denial/error paths; never erase images/history, revoke explicit approvals mid-flight or auto-Apply |
| Safe boundaries | Reconcile policy before next new editing admission; already admitted edit/export finishes under its original native authority | Precisely define future admission boundary with native owner, no UI-only permission |

Secure cache proposal: iOS Keychain and Android Keystore-backed authenticated
storage, version/environment/store binding, atomic replacement, no photo content.
A cached verdict is not a store verifier. Handle missing/corrupt/rolled-back cache,
key invalidation/reinstall and unavailable network without losing recovery access.
Never embed App Store/Play service credentials in the application. No platform
secure-storage or signing entitlement is changed by this proposed ADR.

## Store trust boundary and unresolved architecture

Apple StoreKit wraps transactions in verification results; only verified values
can reach a future grant projection. Refund/revoked products are absent from
current entitlements. Restore must requery store truth, not trust a local flag.
See [VerificationResult](https://developer.apple.com/documentation/storekit/verificationresult),
[currentEntitlements](https://developer.apple.com/documentation/storekit/transaction/currententitlements)
and [AppStore.sync](https://developer.apple.com/documentation/storekit/appstore/sync()).

Google recommends secure backend purchase verification, handling PENDING separately
and acknowledging genuine PURCHASED transactions appropriately. See
[Billing integration](https://developer.android.com/google/play/billing/integrate)
and [security guidance](https://developer.android.com/google/play/billing/security).
This creates an unresolved architecture choice against the no-account/server-required
editing baseline: approve a minimal purchase-verification service (store data only,
no photos), or explicitly review a constrained on-device trust model and risks.
Neither option is selected or implemented here. Public-client possession of a
purchase token, hash or signature string alone is not accepted verification.

## Safe pre-approval infrastructure

Development-only bounded observation validation may check transport shape and
produce an explicitly UNTRUSTED record. It must not grant access, contain purchase
credentials, access stores, persist licensing state or change runtime editing.
Synthetic tests of that boundary are not sandbox purchase evidence.

Acceptance after policy approval requires actual signed iOS/Android sandbox
trial/renewal/expiry/refund/revoke/restore/pending/offline/cache-corruption flows,
secure-cache key lifecycle and recovery regressions. Human-owned store accounts,
product configuration, contracts and credentials remain external prerequisites.

## Alternatives

Store-native verification and authenticated local caching reduce server dependence,
but cannot manufacture cross-device revocation freshness while offline. A dedicated
verification service changes the privacy/deployment architecture and requires explicit
approval. Unverified local purchase flags are rejected as entitlement authority.

## Consequences

No proposed policy takes effect before the required decisions. Store integration,
secure caching and runtime access projection remain unimplemented; manual editing
and recovery behavior are unchanged. Hardware certification grants no entitlement.

## Validation

Before adoption, review every unresolved policy above and define deterministic
state-transition tests. After approved implementation, require authentic signed
sandbox purchase, pending, expiry, restore, revocation and offline/cache recovery
evidence on both platforms. The development envelope tests prove rejection and
untrusted status only; they cannot certify store transactions or grant access.
