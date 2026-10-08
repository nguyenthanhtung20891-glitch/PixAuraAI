# Phase 3 Step 1 — Manual Tool Contract & Registry Foundation

Baseline: 67454642c3f918ef67618d78ace41cef150c1c03 (Phase 2 CLOSED).
Scope: explicitly authorized Step 1 only. Local acceptance evidence is below;
hosted closure requires observed green runs on the delivery commit. Exact-SHA
hosted results are supplied in the delivery response; unobserved gates are not PASS.
No later Phase 3 sequence is frozen, numbered or assigned.

## Implementation

Audited PRODUCT_SPEC, ARCHITECTURE, ROADMAP, QUALITY_GATES, DECISIONS, ADR 0008/0009,
Phase 2 document/storage/evaluation/geometry/preview contracts and Step 11 report.
No architecture conflict was found; no new ADR is needed. Immutable originals,
full ordered stacks, detached candidates, approval/persistence/publication owners,
CPU authority, GPU recipes and resource ceilings remain unchanged.

The [contract](../contracts/manual-tools-v1.md) defines geometry, tone_color, detail,
filters_presets. Only crop/rotate/exposure tuples 1/1 are registered. Parameters:
crop x/y 0..999999 and width/height 1..1000000 millionths of current extent
(defaults 0/1000000; sum <=1000000), rotation 0..3 clockwise quarter-turns
(default 0), exposure -5000..5000 milli-EV (default 0). All integer step 1, reject;
no clamp, locale parsing or platform rounding.

Registry limits: 16 descriptors, 8 parameters each, 64-byte text, 32 KiB output,
sorted IDs/names;
duplicate/unknown tuples reject. Compiled immutable storage exposes versioned C
descriptor JSON and canonicalizes a bounded single operation through the shared
parser. JNI and Swift read the same metadata. No network, runtime discovery,
configuration, executable blobs or dependencies are introduced.

BEGIN captures the immutable base; UPDATE replaces one pending operation;
PREVIEW is detached; COMMIT once yields one immutable revision proposal for a
change; CANCEL changes no history. Neutral append/identical replacement is no-op;
neutral replacement removes only the new stack reference. Ordering and old history
remain intact. 1000 updates coalesce without history growth. Stale bindings reject;
validation/admission/allocation failures preserve valid pending state for retry.
Tool switch/background/disposal must cancel and revoke owner PRV1 publication.
This is a shared-native reference, not UI, scheduler, durable approval or publication
authority. Same source + ordered operations + integers + versions replays the same
logical state; canonical round trips are stable. No new GPU numerical claim.

## Validation

- Windows native fallback (repository-local Zig 0.14.1): 19/19 CTests PASS.
- Linux Node source/documentation/shell regressions: 93/93 PASS, zero skipped.
  Windows-compatible source/documentation suite: 53/53 PASS, zero skipped.
  The initial all-file Windows invocation failed POSIX-only emulator shell fixtures;
  the same complete suite passed on Linux, its authoritative execution host.
- Android offline strict debug/release builds, lint, both JVM suites and test APK:
  BUILD SUCCESSFUL (151 tasks); 2/2 JVM cases per variant, no failures/skips.
  Selected local JNI execution could not run: no connected device. Hosted Android
  emulator execution is mandatory; this is not physical GPU certification.
- Linux ASan/UBSan POSIX rerun: 21/21 PASS, including active instrumentation probes.
  Earlier drvfs runs were 20/21 because permission fixtures need POSIX temporary
  storage. The passing run mounts tmpfs inside the repository for the live session.
- Linux standard native: 19/19 CTests PASS in the authoritative Debug configuration. An exploratory
  Release test build failed existing assert-based storage consumer warnings under
  NDEBUG; tests were rerun in their required Debug configuration without weakening
  warnings/assertions or editing unrelated consumers.
- Actionlint and git diff --check PASS. Hosted Windows/MSVC, Apple compile/Swift
  and Android instrumentation results must be observed at the exact delivery SHA.

Hosted remediation: foundation run 37802462220 at
9612e3d0e2ce9c0a741ff8545a895a119a97f4f9 exposed missing C++ UBSan vptr runtime
linkage for the new C consumer under hosted Clang 18. The narrow fix adds that
consumer to the existing C++ sanitizer linker rule while preserving compilation
as C, and applies existing strict sanitizer environment settings to both new tests.
The new source gate checks this integration. No sanitizer or security gate is waived.

The initial hosted MSVC manual fault sweep timed out while the other 18 native
tests passed. Native JSON builders now append into the existing fallible String
container and format integers with allocation-free, locale-independent to_chars;
they avoid temporary standard-string concatenations and MSVC debug allocating
noexcept moves. Fault tests disable interactive CRT reports and emit path/index
and terminate diagnostics; their 90-second guard is retained. Canonical C output
also receives an exhaustive allocation sweep. Canonical bytes remain unchanged.

New C/native tests cover canonical bytes, preserved error outputs, parameter
edges/invalid notation, versions, duplicate/excess registry counts, defaults,
coalescing, cancellation/stale source/session/revision results, the 256-operation
ceiling, replacement ordering and immutable replay. Exhaustive one-shot allocation
sweeps prove unchanged outputs/base/pending state and successful retry for registry,
update, commit and canonical C output (882 recovered injected failures on the
Linux fallible-container build).
Android and Swift tests consume shared descriptor identities/ranges.

## Observed hosted closure and infrastructure retry

Implementation SHA: 9bb2460a686026dd0e6539e2ee9cd6d7752f59a0.
[Foundation run 37803753967](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37803753967)
is SUCCESS: all 7 jobs passed, including Linux/Windows 19/19 native tests,
21/21 ASan/UBSan, Android arm64-v8a/armeabi-v7a builds, workflow lint and Apple
native/Swift/iOS simulator boundaries. MSVC recovered 2304 injected allocation
failures with 17526 manual checks. The Swift suite passed 8/8 on macOS and 8/8 on
iOS simulator, including ManualBoundaryTests; unsigned iOS package build passed.

[Application-shell run 37803753812](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37803753812)
passed source and Android jobs, including debug/release, lint/JVM and 7/7 Android
instrumentation tests. Its iOS job was CANCELLED with zero steps executed:
GitHub reported that the job was not acquired by a hosted runner after multiple
attempts and annotated macOS arm64 capacity constraints. This is infrastructure
BLOCKED, not an iOS test PASS or implementation failure. Full hosted acceptance
still requires the application-shell iOS job. This report-only delivery update
triggers a fresh exact-SHA attempt; its observed outcome is supplied in the final
delivery response. No runner selection, gate, implementation or timeout is changed
to bypass this blocker. No physical Apple certification gate was executed.

## Scope and limits

PROPOSED MVP: resize; brightness, contrast, highlights, shadows, saturation,
temperature; sharpen, blur; basic filters/presets (no frozen names or recipes).
None is registered or implemented. Category A is this foundation and acceptance
validation. Future MVP controls await separate sequencing review; no later number
or scope is assigned. Whites/blacks/tint/vibrance/free-angle rotation and optional
extensibility are category C backlog, not shipping commitments.
No final editor UI, AI, GPU implementation, Phase 2 or Phase 4 work is introduced.
DH-APPLE-METAL-01 is untouched and unexecuted. Phase 3 Step 2 has NOT started.
Stop after Step 1 delivery for Product Owner review.
