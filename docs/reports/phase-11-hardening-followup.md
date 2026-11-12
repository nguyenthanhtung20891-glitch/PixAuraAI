# Phase 11 hardening successor evidence

This is additive to phase-11-hardening.md and phase-11-source-v1.json at
f585c24a7eec7496454d8fd1f2258bdce5c417ee. Those evidence JSON records remain
unchanged. Phase 11 is IN PROGRESS; mandatory acceptance gates remain BLOCKED.
Phase 12 is NOT STARTED. No independent acceptance or closure is asserted.

## Normal-size Manual control targets

The first recorded accessibility hierarchy also showed Manual button frames of
about 34 points. A stronger version of the existing bilingual UI regression
reproduced the issue at normal text size: Open Photo, Apply, Cancel, Undo, Redo,
Crop and Rotate failed the existing >=44pt target goal. Export checks remain.
This follows Apple's [button guidance](https://developer.apple.com/design/human-interface-guidelines/buttons),
not a new performance or memory admission threshold.

The editor now uses the standard large control size with its existing bordered
button style. No native command, approval, value, history or identity changed.
English/Vietnamese normal and maximum Dynamic Type checks retain both width and
height assertions; the actual Manual gesture/history/cancel/background/reopen
regression and original-target result Undo/Redo/departure are also verified.
Human VoiceOver/TalkBack, contrast, focus and alternative-input acceptance remain
NOT OBSERVED; this automation does not replace them.

## Deterministic Windows compiler correction

Foundation 43202377467, attempt 1, on f585c24 is NOT certified. Its Windows job
131128812609 failed the build with MSVC C4324, treated as error C2220 under /WX:
the test-only aligned allocator header had implicit tail padding. That SHA is not
rerun. The actual job log was retrieved and terminal control sequences sanitized;
log digests are retained in the successor local evidence.

Use an aligned union with an explicit full-size byte extent and compile-time
size/alignment assertions. This preserves default-new and max_align_t alignment,
keeps the same header size/accounting, and avoids implicit padding. No warning
is disabled, no assertion weakened, and no production allocator/ABI changed.
Focused actual ASan/UBSan allocation/fault/lifecycle regression passes. Complete
hosted Windows and sanitizer jobs are required on the successor SHA itself.

## Provenance and acceptance

phase-11-source-v2.json binds each of the five changed source files to its exact
f585c24 predecessor and pins the immutable v1 manifest digest. Legacy fingerprint
checks follow v1 then v2; behavior assertions continue to read actual current
source. No v1 source or local-validation JSON is rewritten. See
 evidence/phase-11-local/validation-v2.json for completed local results and log
fingerprints. Prior unchanged Android, native production, host measurement and
resource-failure evidence is reused within its documented scope.

The final implementation is the commit containing this successor record. It must
receive BOTH complete exact-SHA Foundation 7/7 and Native 3/3 certifications;
future or foreign-SHA success is not asserted here. Final observed receipts are
saved locally in build/phase-11/final-certification.json and reported to the
Product Owner. The failed predecessor is never substituted for that certification.

cpu.reference.v1 VALIDATED; apple.metal.v1 UNKNOWN; android.vulkan.v1 UNKNOWN.
DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED, mandatory before Phase 11
acceptance. Physical cohorts/thermals, human accessibility, binding commerce
contract/implementation/authorized sandbox and independent review remain external
prerequisites. No Phase 11 closure SHA. No Phase 12 work.
