# Commercial admission v1 — native integration compatibility proposal

Status: PROPOSED native integration; approved business policy is ADR0024. The local
SQLite ledger/store adapters are independently executable components, NOT deployed
quota enforcement. No UI boolean, cache record or commercial token replaces native
editing authority. Existing shipping app is not represented as enforcing new quotas.

## Exact frozen boundary requiring independent acceptance

ai-auto-edit-review-approval-v1.md, Persistence/security: review/provider/approval/
candidate state is ephemeral, never process-restart execution authority; durable
output is only ordinary history. Current store/commit APIs expose no atomic commercial
lease + native-success receipt + durable usable-proposal inbox. Updating two independent
stores after a callback cannot prove exactly-once success across process termination.
Schema3 and the existing public ABI may not be silently reinterpreted to create that
proof. Merely persisting a serialized Plan cannot revive source/session/generation
or single-use approval after restart. The standalone quota test capsule is synthetic
opaque delivery data; it is NOT a re-executable native Plan or approval.

## Proposed versioned replacement/addition for Architect decision

A private native commercial owner v1 would own canonical protected/no-backup ledger,
immutable source SHA/length, current native document/revision/generation and signed
store/cache facts. A bounded reservation is keyed to native request generation, never
UI selection. Public existing editing APIs remain unchanged; any new bridge/export
requires explicit additive ABI review and exact binary/export compatibility evidence.

Before side effects, record durable intent with expected ordinary revision identity,
source binding and commit-event logical timestamp under the native publisher lock.
On successful native publication, seal a receipt; do not allow subsequent owner work
until receipt reconciliation. On restart, prove the expected revision exists using
native ordinary history before starting trial or resolving a successful edit lease.
Failed intents release capacity. The commit-event timestamp semantics and crash
ordering must be accepted and tested at every persistence boundary, including native
commit succeeded but commercial acknowledgment failed. No Schema3 field/table added
by this proposal; if durable proof cannot be reconstructed without one, submit that
precise compatibility change rather than forging a receipt or starting trial late.

For AI, distinguish native validated result, durable inbox receipt, UI display and
human Apply. A new durable *advisory* inbox may contain only bounded untrusted Plan
bytes + source/provenance, never approval/candidate/execution authority. After restart,
revalidate against the current immutable source and native generation; do not revive
old approval. The exact usable-delivery/charge point, nondelivery refund and terminal
idempotency marker need acceptance because the existing frozen contract explicitly
has no durable proposal authority. A crash before this accepted delivery point must
release the lease; after it, recover the same usable result without another charge.
A stale/malformed/rejected result never becomes a successful delivery by being cached.

Recovery inspect/export of last approved saved revision bypasses commercial storage
and paid verification altogether, while retaining ALL native project/revision ownership
checks. Corrupt ledger must not prevent recovery construction. No arbitrary file or
preview export is authorized by this bypass. Existing in-flight native work completes;
commercial changes apply only to future admissions.

## Implemented scope and required proof

Native SQLite component: source dedup/reservations, exclusive owner, atomic counters+
opaque durable delivery, transaction rollback, crash cleanup, trusted-time projection,
trial/Free/paid/recovery semantics. It requires real native success/delivery proof at
its privileged boundary and is not wired to application editing. Store components
verify actual SDK/signed metadata and protect snapshots but do not grant native edits.

Acceptance needs native receipt/inbox contract approval, private cross-platform bridge,
all crash-order/source/staleness/manual/AI publication and recovery tests, actual store
configuration/transactions and physical/operator observations. Do not enable UI-only
enforcement or advertise quotas/paid activation as shipped before that integration.

## Proposed bounded records (not frozen or executable authorization)

| Record | Required binding and bounds | Authority |
| --- | --- | --- |
|NativeLeaseV1|version1, native owner/request nonce, immutable sourceSHA256+encoded length, document/revision/generation, purpose, captured admission/clock facts|Capacity reservation only; never native approval|
|NativeSuccessReceiptV1|lease identity, native success kind, published ordinary revision identity where applicable, commit-event logical UTC and provenance digest|Only the native publisher may seal; restart must prove ordinary history before reconciliation|
|AdvisoryInboxV1|source/revision provenance, Assist or Auto variant, canonical bounded bytes<=8192 and native validation/delivery state|Stored advice only; never persisted approval/candidate/execution. Fresh native validation and explicit Human Apply after restart; no source retargeting|

The current component deliberately has no implemented NativeCommercialOwner friend;
only the synthetic test probe can complete reservations/set test facts. Production
wiring is not enabled. Its constructor currently releases pending component leases
on restart; integrating real native success requires the reviewed reconciliation
interface before that cleanup. Never infer real native success from an opaque blob.
Import is successful for charging only after the source is actually usable for
editing under native resource admission; decoding metadata alone is insufficient.
A resource-denied preview/normalization/import releases its reservation.

Store fact mapping must merge products (verified lifetime outranks an expired
subscription); pending/unverified failures do not erase prior verified facts. Online
expired subscription is Free; offline cached expiry may have at most7days with
trusted time, while verified revoke/hold disables that grant. Authenticated store
refund/revocation reconciliation, store billing-grace configuration, ownership/family
settings and a real trusted-time source remain qualification prerequisites; synthetic
clock/provider flags do not prove those observations.
