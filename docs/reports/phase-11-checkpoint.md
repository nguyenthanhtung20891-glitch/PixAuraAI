# Phase 11 resumable engineering checkpoint

Status: AUTHORIZED / IN PROGRESS. No Phase 11 acceptance or closure.
Starting certified SHA: 874b6645c75cd227fbb974b891ff41991397acc2, main.
Origin/main fetched and equal; working tree initially clean; GitHub authenticated.
Baseline Foundation 43127870126 7/7 and Native 43127870204 3/3 reverified against
GitHub: exact SHA, completed, every job SUCCESS, attempt 1.

Plan before source edits: docs/plans/phase-11-roadmap.md, workstreams A–F.
Initial audit: latest AGENTS closure successor applies; historical phase status
is frozen. G7–G11 and mandatory DH-APPLE-METAL-01 acceptance requirements read.
MONETIZATION.md has no matching StoreKit/Play Billing/entitlement implementation
in current app sources and still proposes offline subscription grace requiring
an ADR; no billing or pricing policy may be invented by this hardening task.

Device discovery: no Android entries; paired iPad8,9 unavailable; connected iPhone
17 is simulated. No physical-device test or signing attempted. Project uses
Automatic signing with no DEVELOPMENT_TEAM; credentials/keychain not inspected.
Raw discovery stays ignored under build/phase-11; exported evidence omits IDs.

Known external acceptance blockers: physical Metal with authorized signing/device,
physical G8 cohort/thermal measurements, human assistive-technology walkthroughs,
and G10 missing implementation/final policy/authorized sandbox evidence.
Continue executable software work despite these blockers. No Phase 12.

Next executable action: reproduce platform temporary/private-root cleanup and
path-validation gaps; finish complete accessibility/source audit, add focused
fuzz/stress coverage, then test/correct and proceed through all A–F workstreams.

A/B progress: added two real iOS negative tests and a new Android instrumentation
class (Phase11SecurityTest). iOS before-fix run FAILED both new tests, confirming
cache-path escape/current-export deletion on invalid names, and three unpublished
project-root leaks on rejected imports. Raw result: build/phase-11/security-before.xcresult
and .log (synthetic application data only). iOS model fixes are applied; focused
post-fix run started (build/phase-11/security-after.xcresult/.log).
Android production sources still unchanged while before-fix instrumentation runs
on repository-local ARM64 emulator emulator-5556; log android-security-before.log.
Do not patch Android production until that baseline reproduction finishes.

New native phase11_security_test is registered in CMake/CTest, test-only: seed
0x50483131, 4096 mutations + 64 random 8192-byte payloads + 2 boundary cases +
valid seed = 4163 requests. Debug PASS, accepted 6/rejected 4157, 33328 checks,
no state/history mutation and manual cancel recovery. ASan/UBSan full host build
and suite started under build/phase-11/sanitize; inspect logs before claiming PASS.

Pending: Android smallest fixes/retests; full UI accessibility/bilingual automation;
source-fingerprint successor for modified files (prior records MUST remain immutable);
new adequate-sample host p50/p95/memory-trend instrumentation; G7/G9/G10 audit report;
blocker matrix/human protocols; source/platform/CI certification. No commits yet.
All product invariants and GPU trust states unchanged. Async question for physical
Apple device/signing authorization is pending; no answer is assumed.

Security regression update: Android before-fix 2/2 FAILED (private-root leak and
invalid-cache-name deletion); after narrow fixes 2/2 PASSED. iOS after-fix 2/2
PASSED. Current Android model handles prepared owner cleanup before removing only
a fresh unpublished root; both export helpers validate bounded basenames before
cache cleanup. Android also removes an owned partial cache file on write failure.
Raw logs/xcresults and before XML summary remain under build/phase-11.

Local sanitizers: Ninja unavailable, so fresh Make generator used. Initial Apple
ASan suite aborted because detect_leaks=1 is unsupported by Apple runtime. Added
Darwin-only detect_leaks=0 environment, preserving Linux detect_leaks=1 and all
ASan/UBSan negative probes. Full corrected suite is running; inspect sanitize-after.log.
No sanitizer is disabled. Apple leaks measurements remain separately required.

D work: inventory found untranslated static UI/screen-reader labels and Android
mojibake in proposal text. Added Phase11AccessibilityTest for en/vi, 200% text,
SetProgress alternative input, 48dp export targets and Vietnamese descriptions;
before-fix run started. Plan: presentation-only translations and scrolling fixes,
plus equivalent Simulator large-text/bilingual UI check. Human AT walkthrough is
still external, never inferred from automation.
C work: new successor host benchmark sources created under scripts/benchmarks/
phase-11-host.{cpp,py}; old harness immutable. 5 warmups + 100 measured lifetimes
per size; individual previews, nearest-rank p95, live Mach RSS and peak RSS.
Run after heavy sanitizer work completes; measurements not collected yet.

Sanitizer progress: corrected full suite passed 67/68 including both active
negative probes. Remaining UBSan failure was actual misalignment in the
TEST-ONLY gesture allocator: Darwin max_align_t is weaker than C++17 default-new
alignment, so its accounting header offset broke Registry alignment. Header now
preserves both alignments with compile-time assertions. Focused corrected
ASan/UBSan gesture_lifecycle_test PASS (4.73s), no diagnostics suppressed. Full
final suite will be rerun after the benchmark to avoid CPU-load contamination.

The first accessibility test attempt exposed a HARNESS issue, not a product fail:
localized Context lost ActivityResultRegistryOwner. Explicitly forwarding the
existing owner fixes that setup; before2 run now targets real UI behavior.
Host benchmark is sequenced after that short run and before further heavy tests;
raw measurements under build/phase-11/host, exact harness/native source digests.

Current guarded UI replay: build/phase-11/replay-ui-baseline.py temporarily tests
ONLY the old ManualEditorScreen.kt against the corrected locale/Activity owner
harness, then restores current bytes in finally. Backup:
build/phase-11/ManualEditorScreen.current.kt. If interrupted mid-replay, verify and
restore that owned backup before further Android builds. This is not a whole
baseline build: adapters already have security fixes; current catalogs are present.
Metadata/log: android-ui-replay-metadata.json and android-accessibility-before3.log.
Earlier before2 failed test compilation due nullable registry; corrected checkNotNull.
No product failure is claimed from harness setup/compile errors.

Presentation fixes prepared: 92 bilingual canonical labels/tool/navigation strings,
localized screen-reader labels/value units, corrected Android mojibake; scrollable
Android action rows and iOS export sheet. Native identifiers, raw Plan inspection
values, tool ranges and authority remain unchanged. Prior catalog records immutable.
Benchmark first summary failed on an array-valued preview field; no PASS assigned.
Corrected scalar/individual-preview summary is sequenced after UI replay.

Update 2026-11-12: controlled Android UI replay restored current source; real
before3 failures were untranslated Vietnamese Tone/Color and clipped Share at
200% text. Corrected Android accessibility 2/2 PASS. iOS bilingual max Dynamic
Type export PASS in light AND dark (two separate xcresults). No human AT claim.
Host benchmark completed: 5 warmups +100 lifetimes for 0.8/6 MP, 400 individual
preview samples per size; measurements.json and JSONL under build/phase-11/host.
Apple leaks small workload: 0 leaks /0 bytes; large workload running.
Node full local suite:202/202 PASS (Linux-specific android-emulator test retained
in CI; documented macOS GNU realpath limitation). Predecessor hashes extended
additively;109 historical JSON records verified LF-identical to baseline.
Android Debug/Release,7+7 JVM unit, lintDebug/lintRelease builds successful.
Full offline instrumentation running on owned ARM64 Emulator, airplane mode1.
Release scanner correctly rejected migrated Windows resource paths in stale
compiled resources; diagnose/rebuild clean owned intermediates after test logs
are preserved, do not weaken scanner. Native sanitizer67/68 PASS after alignment
fix: remaining Apple inference status15 under sandbox, focused host-device-access
check now running. Report exact cause rather than claim a full green suite.
iOS full generic builds/scanners/app regression script running. No Phase11 commit
or CI yet. Current committed HEAD remains874b6645c75cd227fbb974b891ff41991397acc2.
Fresh OSV audit289 pinned Maven coordinates (all current locks),20 flagged only
build/test/Swift-export configurations;107 release-runtime coordinates0 flagged.
This is advisory database scope, not proof of absence. G10 policy/sandbox and
physical signing questions remain pending; no access assumed.

Update: Android final clean native-macOS Gradle transform rebuild PASSED Debug/
Release,7/7+7/7 JVM and both strict lint tasks. Root cause of rejected APK: cached
AAPT .flat inputs themselves held Windows backslash paths; project clean alone
could not fix them. Repository-local transform cache quarantined, no cache or
scanner policy weakened. Final APK isolation PASS:12 native libraries,2 DEX,
33,021,570 uncompressed bytes. Offline instrumentation corrected missing host
transport fixture (not a product defect), then62/62 PASS,0 skipped,airplane1;
fresh TRANSPORT_TEST_ONLY artifact read after uninstall, overwrite defense intact.
Full iOS initial sweep exposed normal-text export target20–22pt and a result-back
mode-selector assertion; do not claim green. Export labels enlarged to48pt (44pt
boundary comparison had floating-coordinate rounding); assertions remain >=44.
Fresh isolated Simulator focus run uses final format labels,13/13 app unit PASS,
normal target and result-back diagnosis in progress. Initial full sweep still
finishing old compiled snapshot; final affected checks and hosted complete suites
remain required. Hardware/G10 questions unanswered. No Phase11 commit yet.

Continuation: automatic approval review temporarily failed due usage limit; the
blocked Simulator action was NOT executed. Product Owner requested continuation;
approved tool execution is available again. The prior six-test original-target
run finished5/6 PASS with result-back still failing on the pre-toolbar-fix build.
Failure hierarchy confirms Manual controls/revision2 but no navigation buttons.
Removed mode-dependent .id from toolbar principal item, keeping stable identity;
original-target result-back + integrated lifecycle tests now running on this fix.
Final max-text light/dark and generic Debug/Release/scanners still pending.
Android62/62 and full source202/202 remain verified. No commit/push yet.

Local engineering completed after continuation: final stable-toolbar original-target
result Undo/Redo/return and integrated background/navigation/restart2/2 PASS.
Final bilingual export targets normal text PASS and maximum Dynamic Type light/
dark PASS; actual48pt labels preserve >=44pt tests. Final unsigned generic iOS
Debug/Release builds and release scanners PASS. New source202/202 PASS.
Detailed completed test scopes, initial failures and log SHA256:
docs/reports/evidence/phase-11-local/validation.json and local-log-index.json.
Phase11 hardening report and external handoff are ready for implementation CI.
The implementation is the commit containing this checkpoint; resolve its exact
SHA with git rev-parse HEAD. Observed final CI/HEAD receipt will be written locally
to build/phase-11/final-certification.json after pushing and completing both full
workflows. No future CI success or independent acceptance asserted here.
Next executable action: final diff/secrets/provenance review, commit/push main,
verify exact-SHA Foundation7/7 and Native3/3 attempts, correct deterministic
failures on a new commit. Then stop only at remaining external acceptance gates.
