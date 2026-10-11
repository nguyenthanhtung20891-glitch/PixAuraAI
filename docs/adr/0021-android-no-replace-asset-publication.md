# ADR 0021: Android no-replace asset publication

Status: Accepted
Date: 2026-10-11
Scope: Android asset publication syscall only, discovered in Phase 3 Step 7.

## Context

At SHA `7dbc8244691becb75f3f94212c837016edc4bf8a`, Android API 35 emulator
instrumentation fails both real-editor imports with native IO_ERROR (12).
[Exact hosted job](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/38120406873/job/114413777511).
The uploaded per-test logcat shows Android SELinux denying `link` on the editor's
private staging file, with `untrusted_app` and `app_data_file` labels. The source
and destination are inside the application's private no-backup project root.
The denied call is the existing `Files::Impl::publish` POSIX `linkat` path.

[Accepted ADR 0010](0010-durable-native-project-storage.md) explicitly specifies
POSIX `linkat` without overwrite. Changing Android's syscall therefore requires
an explicit decision. The Product Owner / Architect approved this narrow
supersession on 2026-10-11; Step 7 acceptance requires all validation gates to pass.
The failure is not permission to disable SELinux, alias checks, immutable bytes,
directory sync, destination verification, tests or no-overwrite protection.

## Decision

Supersede only ADR 0010's Android `linkat` choice with Linux/Android
`renameat2(..., RENAME_NOREPLACE)` between the same pinned staging and asset
directory capabilities. Keep file sync/close before publication, read-only bytes,
no replacement, both directory syncs, exact digest/length verification and
asset-before-SQLite-reference ordering. EEXIST still means verified deduplication.
Staging stays under the same private managed root. Immutable content-addressed
SHA-256 identity, pinned capability/path protections and NOFOLLOW checks remain
unchanged, including exact existing-object digest/length verification on EEXIST.
Unsupported syscall/filesystem or any other error still rejects with IO_ERROR;
there is no insecure overwrite fallback. Linux, Darwin and Windows paths remain
as accepted. No new durable schema, dependency, tool or numerical semantics.

## Validation

Android's API/kernel support and actual SELinux rename permission must be observed
in emulator instrumentation. Add/execute native ingestion, duplicate-digest,
existing-corrupt-digest nonreplacement, concurrent publisher and crash/reopen
regressions, alongside all existing storage and editor gates. Publication tests
must prove the Android branch and durability rather than relying on UI success.

Observed at implementation SHA `3f3e10e61814cb4b90617443c4b37c7e9d7688b2`:
[Foundation 7/7](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/38129297272)
and [application shells 3/3](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/38129297283)
PASS. Actual Android instrumentation reports 14/14 tests, no failures/errors/skips,
and logcat confirms `ADR0021 Android RENAME_NOREPLACE actual branch: 71 checks PASS`.
The direct native corpus records four real successful no-replace publications;
production ingestion verifies read-only, single-link, exact immutable bytes.
Duplicate/existing-correct dedup retains the inode; existing corruption is rejected
without replacement. Barrier-synchronized same/different digest publishers pass.
Staged-reader failure, failure after file sync before publication, both directory
sync failures and ENOSYS/EOPNOTSUPP reject deterministically with IO_ERROR; no
fallback publication occurs. All nine publication/SQLite crash-reopen boundaries
run on the independent Android branch, alongside every existing storage regression
and ten existing migration crash cases. Linux 29/29 and ASan/UBSan 31/31 PASS.
Android editor import/edit/preview/commit/reopen also passes under actual SELinux.
See the [Step 7 report](../reports/phase-3-step-7.md) for the required-case matrix,
cross-platform parity and final exact-SHA closure procedure.

## Alternatives

1. Approve the Android-only no-replace rename backend: preserves the frozen
   storage invariants and permits real image import, subject to validation.
2. Retain the linkat implementation: preserves the syscall decision but blocks
   Android manual editor import and Step 7 acceptance on the observed sandbox.

Ordinary replacing rename, chmod/SELinux changes, platform shadow asset stores and
copying directly into the final digest path are not acceptable alternatives.

## Consequences

Android alone changes publication syscall. All storage identities, ceilings,
schema-3 authority and failure behavior remain frozen. Unsupported kernels or
filesystems continue to reject; hosted emulator and full storage regression
evidence are required before Step 7 can be marked COMPLETE / FULL PASS.
