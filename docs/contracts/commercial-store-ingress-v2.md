# Private commercial store ingress v2 — implementation record

Approved scope: ADR-0024 and the Phase 11 private native commercial owner. This
adds no public v1 ABI, editing authority, approval, schema reinterpretation,
product IDs, store account, checkout, networking or Billing runtime dependency.
Phase 11 remains IN PROGRESS. This record is not independent acceptance.

## Trust boundaries

1. The Apple adapter obtains an actual StoreKit 2 verified transaction through
   its private purchase/current-entitlements/update path. The Google adapter
   verifies the configured backend's P-256 signature, package/catalog/environment,
   fresh request nonce and transaction-token digest. Neither UI paid flags nor
   purchase callbacks, pending results or unsigned cache objects substitute.
2. The application-owned bridge binds that verified result to the configured
   native owner/catalog. A protected installation-key MAC authenticates the local
   handoff/cache. It is **not an Apple/Google signature**, a native revision, a
   Plan approval or proof that a sandbox transaction occurred.
3. Protected-cache restoration reuses the previously authenticated record. It
   never signs arbitrary decoded cached metadata as a new store-verified event.
4. The native commercial owner checks catalog scope, MAC, monotonic verified
   observation ordering and revocation before updating paid projection. It uses
   the same commercial SQLite owner; no second editing publisher/storage owner.
5. Store signed/issued time identifies an authenticated observation. It is not
   automatically authenticated current UTC. Cache replay cannot clear protected
   clock uncertainty or extend a subscription beyond its verified expiry/grace.
   Recovery inspection/export remains unconditional; an authorized operation
   completes under its existing reservation when commercial status changes.

The typed platform adapters remain responsible for real store cryptographic
verification. A synthetic local MAC test does not certify that boundary.

## Bounded metadata representation

The private `store_record_v2.hpp` representation binds application scope, Apple or
Google store, Sandbox or Production, exact allowlisted product/kind, authoritative
active/expired/revoked status, signed observation time, expiry and a SHA-256 digest
of necessary transaction provenance. Product catalogs contain at most eight
entries. Identifiers and times are strictly bounded; lifetime has no expiry;
canonical decoding rejects trailing fields, noncanonical integers and mismatches.
MAC comparison does not short-circuit on secret-dependent byte differences.

No photos, image paths, edits, conversations, voice, account identifiers, analytics
or advertising identifiers belong in this record. Its transaction digest is local
necessary verification metadata, not a tracking identifier or upload permission.

## Current implementation / remaining integration

Implemented: private canonical metadata/cache validation, immutable authenticated
cache wrapper, scope/catalog/key/tamper checks, and deterministic synthetic tests.
This component deliberately grants no entitlement by itself.

Still required: native owner's durable paid-record reconciliation and crash-order
proof; actual typed platform-to-native handoff; previously sealed protected-cache
replay; authoritative revocation/expiry ordering; native clock-policy projection;
application configuration and presentation. Checkout remains disabled. Authentic
StoreKit/Play accounts, catalog and sandbox qualification, verifier deployment and
Android Billing privacy admission remain external prerequisites. Do not label
those prerequisites PASS or treat the component tests as purchase evidence.

## Apple signed-cache verification work in progress

A local verifier is being added using system Security/CryptoKit, with no server or
online certificate fetch. It pins Apple's public Root CA G3, checks App Store
certificate extension OIDs, validates ES256 and device binding, and binds the
payload to the actual app/catalog/environment. It validates historical signed
metadata, not current revocation or authenticated current UTC. It is not yet wired
into native paid admission and has not received genuine sandbox qualification.
No personal certificate, profile, signing team or account trust is modified.

Protocol references: [Apple transaction verification](https://developer.apple.com/documentation/storekit/transaction),
[device binding](https://developer.apple.com/documentation/storekit/appstore/deviceverificationid),
[Apple's chain-verification policy](https://github.com/apple/app-store-server-library-python/blob/main/appstoreserverlibrary/signed_data_verifier.py),
[public Apple PKI](https://www.apple.com/certificateauthority/).
The retained public certificate's SHA256 is
`63343abfb89a6a03ebb57e9b3f5fa7be7c4f5c756f3017b3a8c488c3653e9179`.
Synthetic signature/DER tests are component proofs only, never genuine purchases.

The Android verified-record type now retains the actual verification key's canonical
SPKI SHA-256 and the signed package/environment/kind/expiry/nonce for native-owner
binding. The future broker must compare that key identity with its configured
verifier; a record verified under an arbitrary caller-supplied key is not enough.
Verification requires the complete NIST P-256 parameters, defensively decodes the
bounded SPKI, checks point/parameter consistency, and verifies/fingerprints the same
decoded key. No Billing dependency, endpoint, product configuration or checkout is
activated by this change.

Apple protected signed-cache replacement now rejects signed-generation rollback,
same-generation revocation reversal and ambiguous renewal. These metadata ordering
checks grant nothing themselves. Native durable high-water reconciliation and
cache repair after genuine native acknowledgment remain implementation work.

## Remaining implementation dependency order (not an acceptance receipt)

The following is the engineering plan under the already approved private-owner
architecture. None of these pending stages is implemented merely by documenting it.

1. Add an explicit private commercial-format evolution for configured store
   catalog/pin, per-product observation ordering and native post-COMMIT
   acknowledgments, on the existing commercial SQLite connection. This is distinct
   from project Schema 4. Preserve quota/trial migration and default legacy
   rejection. Do not open another editing repository/publisher or SQLite owner.
2. Persist bounded actual signed store proof alongside its canonical scoped
   metadata and native authentication, not just an unsigned paid flag. Native
   reconciliation must use its actual installation key and configured catalog/pin;
   an AuthenticatedCache verified under an arbitrary caller key is insufficient.
   Revalidate actual store signatures through the trusted platform verifier on
   restoration. The protected local MAC is never described as the store signature.
3. Configure the application-owned broker before exposing its store ingress. Apple
   ingress accepts the immutable result of pinned Apple JWS verification; Google
   accepts the immutable signed result under the actually configured P-256 key,
   package/environment/catalog and request nonce. Raw UI/cache JSON must not be
   routed through a fresh-evidence sealing path. Missing genuine configuration
   leaves the store path unconfigured and checkout off, while native Free/trial
   and recovery paths retain their current authority.
4. Commit observation/high-water/ack atomically. Acknowledgment must be a native
   authenticated post-COMMIT result, not a successful closure return or guessed
   blob. Bind catalog, product, signed generation, transaction provenance and
   event kind. Retried equal facts return the same acknowledgment; rollback,
   same-generation revocation reversal and ambiguous renewal cannot regain paid
   access. Revocation affects only the corresponding verified scope/product.
5. Repair a corrupt platform cache only after genuine fresh verification and that
   durable native acknowledgment. Preserve independently valid products and known
   lifetime facts on unrelated verification failures. Never replace/reset an
   existing native installation key. Cache loss must not start a new trial.
6. Project lifetime, subscription expiry/grace and authoritative revocation in the
   native owner. Historical signedAt never authenticates current UTC; cache replay
   cannot clear reboot uncertainty or extend the verified expiry plus seven-day
   boundary. Do not introduce a raw UI trusted-time boolean. If no authenticated
   current-time evidence is available, retain the approved fail-closed uncertainty
   policy. An already authorized native edit/export finishes its reservation.
7. Prove crash orderings before COMMIT, after COMMIT/before acknowledgment, after
   acknowledgment/before protected cache write, duplicate replay, corrupt/wrong-key
   cache, partial product failure and process restart. Independently prove native
   revision/approval/trial state unchanged by every store operation. Exercise the
   same owner through Swift and Kotlin, with synthetic cryptographic fixtures
   clearly separated from genuine sandbox acceptance.

Completely offline defense against a privileged rollback of *all* local state is
not claimed. Keep OS-protected/no-backup storage and authenticated high-water;
reconcile authoritative store revocation/status when available. Do not add a
tracking identifier, invasive fingerprint, photo upload or unapproved time service
as a shortcut. Actual product configuration, authentic sandbox transactions and
Android Billing privacy admission remain operator/independent acceptance gates.

## Native format3 journal implementation successor (2026-11-25)

`verified_store_v3.hpp` and `store_journal_v3.hpp/.inc` now implement the bounded
native signature-proof/authority boundary and durable observation journal. The
journal shares the commercial ledger connection; configuration, eight-product
manifest, signed proof, high-water metadata and native acknowledgment use explicit
version3 domains. The acknowledgment is an immutable private post-COMMIT store
observation receipt, never a native edit receipt or Apply capability.

`CommercialCoordinatorV2` accepts an explicitly configured format3 authority,
issues one native challenge at a time, consumes it on a matching verification
attempt, and projects reverified native journal facts. This opt-in component is
not yet connected to the shipping Swift/JNI runtime. Missing genuine configuration
keeps checkout disabled. Tests use synthetic signatures, not authentic transactions.

A newer authentically signed authoritative observation can restore status. A more
restrictive signed fact for the same transaction wins an equal-generation tie;
revocation reversal, ambiguous renewal/identity ties and older facts are rejected.
Identical retries retain
the original native acknowledgment. Cached replay never authenticates current UTC,
starts trial, clears reboot uncertainty or extends expiry/grace. Restored lifetime
and subscription projections retain the approved Ledger policy. Invalid raw proof
for one product leaves another independently verified product available. Corrupt
manifest/configuration cannot be silently regenerated from cached metadata.

The journal authenticates local persisted state but does not claim resistance to
an attacker restoring a complete older database AND protected installation key.
No server, invasive device fingerprint or unauthorized clock authority is added.
Remaining work includes actual platform callback binding, protected-cache repair
using the native post-COMMIT receipt, aggregate/lifecycle admission and full shipping
regressions. Historical pending lists above remain chronological implementation
records; they do not constitute acceptance of this successor.

## Version3 application bridge successor (2026-11-25, validation in progress)

The additive `store_ingress_v3.h` family binds one immutable application-owned
cryptographic verifier to the existing v2 runtime/commercial owner. Callback
context retention follows that runtime, with close rejected while verification
holds native work. No v1 handle is accepted. A persistent 262144-byte verifier
lease and a transient 262144-byte verification lease use the existing aggregate
budget/effective tier policy; ceilings are unchanged. Failed calls leave output
receipts untouched. A matching native nonce is single-use even on signature failure.

Native acknowledgments include the exact signed-proof SHA256 for protected-cache
repair. They are store-observation receipts, never edit/Apply authority. Swift's
constructor and Kotlin's native-buffer parser are private; neither parses an
arbitrary cached blob as native success. Native journaling persists actual signed
proof so a missing platform cache is not a missing entitlement authority. Restored
proof is still checked by the actual configured verifier, with explicit cached
origin; it does not regain a fresh nonce/SDK fence or authenticate current UTC.

The optional Apple application adapter verifies Apple's pinned Root G3 chain,
ES256, app/catalog/environment/device binding AND the private actual verified
StoreKit delivery fence. Its stable transaction identity is scoped actual
transactionID, distinct from the JWS hash. It commits natively before replacing
only the acknowledged product's signed cache and finishing the StoreKit transaction.
The pool uses the same application runtime and existing capability owner; the
verifier holds no runtime reference, avoiding a retention cycle. Cache-only raw
JWS cannot create the private fresh fence. Valid cached/SDK facts remain historical
signed observations, not proof of current network revocation status or current UTC.

The optional Android adapter uses the actual configured canonical P-256 SPKI,
package/catalog/environment and nonce through the existing signature verifier.
JNI retains/releases its JVM verifier and performs bounded explicit field checks.
An application request holds a lease on the SAME editor pool, is cancellable and
single-use, and acknowledges COMMIT before repairing an isolated per-product
AES-GCM cache. File corruption repair preserves its existing AES key and never
resets native installation/signing keys. A cached or same-generation weaker snapshot cannot reverse revocation; a newer
actual authoritative signed observation may restore access under the approved policy. No Billing runtime, endpoint, network permission, account,
analytics or photo/voice/edit upload is introduced.

Optional bundle/manifest configuration is absent by default. Its schema is
operator configuration, not invented store products or a checkout activation.
Actual StoreKit/Play sandbox evidence, deployed metadata verifier trust and Android
Billing dependency/privacy admission remain external. Current code and synthetic
signature tests still require final complete platform/source/release certification.
Do not classify any of these component runs as authentic purchases or Phase11 PASS.

Apple latest-transaction selection follows the SDK's
[latest(for:) contract](https://developer.apple.com/documentation/storekit/transaction/latest%28for%3A%29):
the v3 listener re-reads the actual most recent product transaction instead of
guessing order from transaction IDs or treating a historical update as the latest
product state. This is an implementation choice for the product-scoped journal,
not evidence of authentic sandbox qualification or current authenticated UTC.
