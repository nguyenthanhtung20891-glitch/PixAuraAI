# ADR 0021: Android no-replace asset publication

Status: PROPOSED — requires Product Owner / Architect decision; not implemented.
Date: 2026-10-11
Scope: Android asset publication syscall only, discovered in Phase 3 Step 7.

## Evidence and conflict

At SHA `7dbc8244691becb75f3f94212c837016edc4bf8a`, Android API 35 emulator
instrumentation fails both real-editor imports with native IO_ERROR (12).
[Exact hosted job](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/38120406873/job/114413777511).
The uploaded per-test logcat shows Android SELinux denying `link` on the editor's
private staging file, with `untrusted_app` and `app_data_file` labels. The source
and destination are inside the application's private no-backup project root.
The denied call is the existing `Files::Impl::publish` POSIX `linkat` path.

[Accepted ADR 0010](0010-durable-native-project-storage.md) explicitly specifies
POSIX `linkat` without overwrite. Changing Android's syscall therefore requires
an explicit decision. No storage change implementing this proposal has been made.
The failure is not permission to disable SELinux, alias checks, immutable bytes,
directory sync, destination verification, tests or no-overwrite protection.

## Proposed decision

Supersede only ADR 0010's Android `linkat` choice with Linux/Android
`renameat2(..., RENAME_NOREPLACE)` between the same pinned staging and asset
directory capabilities. Keep file sync/close before publication, read-only bytes,
no replacement, both directory syncs, exact digest/length verification and
asset-before-SQLite-reference ordering. EEXIST still means verified deduplication.
Unsupported syscall/filesystem or any other error still rejects with IO_ERROR;
there is no insecure overwrite fallback. Linux, Darwin and Windows paths remain
as accepted. No new durable schema, dependency, tool or numerical semantics.

Android's API/kernel support and actual SELinux rename permission must be observed
in emulator instrumentation. Add/execute native ingestion, duplicate-digest,
existing-corrupt-digest nonreplacement, concurrent publisher and crash/reopen
regressions, alongside all existing storage and editor gates. Publication tests
must prove the Android branch and durability rather than relying on UI success.

## Alternatives and trade-offs

1. Approve the Android-only no-replace rename backend: preserves the frozen
   storage invariants and permits real image import, subject to validation.
2. Retain the linkat implementation: preserves the syscall decision but blocks
   Android manual editor import and Step 7 acceptance on the observed sandbox.

Ordinary replacing rename, chmod/SELinux changes, platform shadow asset stores and
copying directly into the final digest path are not acceptable alternatives.
