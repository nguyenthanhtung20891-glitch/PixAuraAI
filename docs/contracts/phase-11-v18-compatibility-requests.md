# Phase11 v18 — exact remaining compatibility requests

Status: PROPOSED; not a waiver or authorization to modify frozen contracts.
The supplied Principal Architect decision ACCEPTED v17 software and authorized
original-reader-v2/stage-plan-v2/stream-writer-v2 plus private commercial coordination.
Those internal components are implemented additively. This record concerns the
remaining native publication and platform ABI boundaries, not reopening that approval.

## CR18-01 — atomic committed-edit receipt and logical timestamp

Frozen Schema3 revisions contains document,id,seq,parent,actor,plan, with no trusted
commit timestamp. storage::Repository::apply appends ordinary history and updates
current/generation/epoch in a private SQLite WAL transaction. It exposes neither
its connection nor a receipt hook. A separate commercial SQL transaction cannot
prove the successful native commit event's UTC time across process death. In
particular, a pre-publication intent at23:59:59 followed by commit after00:00:00
cannot safely start trial at either guessed time after a crash. Ordinary history
proves the revision, not its timestamp. This subtask is STOPPED; no trial is started
by import, proposal delivery, preview, intent records or a recovered opaque blob.

Requested minimal change: explicit opt-in storage Schema4 + publisher-v2 extension
that seals an immutable NativeCommitReceiptV2 in THE SAME SQLite transaction as
its ordinary revision. Bind source SHA/length, project/document/revision/session,
kind, logical commit-event UTC, native clock provenance and a unique native event
identity. Define that timestamp at the native publication linearization stage;
rollback removes BOTH receipt and revision. Readback/reconciliation validates the
actual immutable ordinary revision and source before acknowledging the local
commercial lease. Every crash ordering must be tested, including native COMMIT
succeeded but commercial acknowledgment did not. Trial uses the committed receipt,
never reopening time, UI state, an arbitrary revision ID or serialized Plan.

Preserve Schema3 reading/recovery/export and require explicit separately approved
migration; no automatic migration or reinterpretation of actor/plan/id columns.
Do not claim ATTACH across WAL databases makes a revision+commerce commit atomic.
The commercial SQLite format remains separate from project schema. Approval must
explicitly cover the exact native storage source fingerprints changed; numerical
kernels, source ownership, approval single-use and human publication remain frozen.

## CR18-02 — versioned application editor context / private bridge

Frozen editor.h pixaura_editor_open/open_policy and editor_api.cpp Service own a
v1 pixaura_decode_handle original obtained through pixaura_working_normalize.
Manual gesture, preview and approval/execution paths require that contiguous v1
working owner, max8388608pixels/128MiB. A4000x3000 RGBA32F original is192000000B.
The new reader cannot masquerade as that handle or silently reinterpret its kind,
layout, ABI version, geometry admission or approval source binding.

Requested explicit new editor-context-v2 bridge contract (separate opaque family,
old installed C ABI unchanged): native v2 original+revision owner; immutable source
identity; per-call epoch/session/generation/request serial; resource-profile snapshot
and aggregate leases; bounded explicit FIT preview; staged full JPEG/PNG export;
manual gesture begin/update/Cancel/HUMAN Apply; ordinary Undo/Redo/reopen; stop,
join then destroy. Use existing Registry/tool validators/kernels and native document
history. A commercial token is eligibility only and never a gesture/approval/Plan
capability. Carry verified platform-store events and actual OS clock observations
through private lifetime-fenced channels, never a paid UI boolean or arbitrary time.

The v2 application bridge must also define native AI approval/execution for a v2
original while preserving all existing exact-source, single-use approval and private
candidate/publication contracts. Current native commercial coordinator deliberately
rejects >8MP AI delivery without charging because the frozen shipping Human Apply
path could not execute that result. No UI promise of unsupported Auto editing.

Linkage must retain ONE native editing/SQLite authority. In particular, a second
statically embedded SQLite copy beside a legacy shared-core publisher must not be
used to claim an in-process writer fence. Current native proof uses one SQLite
instance for its real Repository and fence; the legacy shared-core link is used
only for pure capability/budget queries. The app bridge must use the same native
publisher/storage instance and adopt actual retained source/revision payload leases
BEFORE new image allocation, including large-history/reopen transient ownership.
Snapshot payload capacities are counted by the new projection, but that does not
retroactively instrument a frozen legacy Repository::read allocation.

This requires explicit compatibility acceptance before installing new bridge symbols
or replacing the shipping editor backend. Swift/Kotlin clock/store handoff components
can compile and be tested independently; no incomplete checkout, quota enforcement
or 12MP UI path is enabled. The accepted internal v2 pipeline and real native
Schema3 history tests are not represented as completed application integration.

## Already executable without those changes

SQLite BEGIN IMMEDIATE writer reservation can fence source/advisory validation and
commercial acknowledgment without modifying Schema3 or native source fingerprints.
WAL recovery reads remain available. Tested native writer contention, actual stored
source/planner bindings, source dedup, three delivered AI uses, cancellation winner,
UTC delivery rollover, stale/dismissed restoration and real _Exit orderings are
independent of committed-edit timestamp receipts. Verified trial/paid projection in
the existing ledger remains separately tested; it is not enabled purchase evidence.

G7 physical/operator, G8 LOW/MID/HIGH performance/thermal/pressure, authentic G10
store setup/sandbox and human G11 acceptance remain external gates. No Phase11
closure or Phase12–14 implementation, no GPU admission/promotion or Metal rerun.
