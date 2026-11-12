# Phase 11 hardening engineering evidence

Phase 11 AUTHORIZED / IN PROGRESS. Acceptance BLOCKED by mandatory external gates.
No Phase 11 closure or independent acceptance is asserted. Phase 12 NOT STARTED.
Authority: Product Owner's Phase 11 master mission, 2026-11-12. Certified starting
SHA: 874b6645c75cd227fbb974b891ff41991397acc2, main. Baseline exact-SHA Foundation
43127870126 7/7 and Native 43127870204 3/3, both completed SUCCESS, attempt 1,
were rechecked. Phase 10.1 stays CLOSED within its independently approved scope.
The final implementation SHA and observed GitHub runs are delivered separately
so this commit cannot claim its own future certification.

## Gate matrix

| Gate | Engineering evidence | Acceptance status / remaining dependency |
| --- | --- | --- |
| G7 security/offline/privacy | Reproduced/fixed both-platform invalid export basename and failed-import orphan cleanup; negative regressions; scoped permissions/backup/log/network audit; bounded parser fuzz; release scanners; offline ARM64 Emulator flows | Software evidence; physical OS backup/protection and observed photo-traffic walkthrough remain external. Dated tooling advisories recorded, no zero-vulnerability assertion |
| G8 performance | Host CPU 5 warmups +100 lifetimes per size, 400 individual previews, peak/live RSS; actual leaks diagnostics; allocation/fault/manual-recovery suite | BLOCKED: physical supported low/mid/high cohorts, thermal/frame/pressure/12MP observations absent |
| G9 AI control | Existing native authority/allowlist/range/stale/approval/cancel/manual recovery regressions retained; new 4,163 malformed proposals; no learned model/provider/backend admitted | Software checks verified; independent integrated acceptance pending; no model-quality evidence invented for reference deterministic provider |
| G10 commerce | Policy/source inventory: local trial/pricing policy documented; no app StoreKit/Play Billing/entitlement state machine or store sandbox outcomes | BLOCKED: binding offline-grace/entitlement contract, actual implementation and authorized sandbox/store configuration needed; no policy invented |
| G11 accessibility | Fixed bilingual static/generated control labels, Android large-text clipped actions, slider labels/values, scrollable iOS export; Android en/vi 200% and iOS max Dynamic Type light/dark automation | BLOCKED: actual human TalkBack/VoiceOver, focus/contrast and alternative-input walkthroughs not supplied |
| DH-APPLE-METAL-01 | Device discovery and existing physical protocol reviewed; no physical execution | DEFERRED / NOT WAIVED / NOT SATISFIED; mandatory PASS before Phase 11 hardening acceptance |

## Verified defects and smallest fixes

Before-fix tests failed on BOTH platforms: path-like share filenames could escape
the owned export directory and invalid names could remove an existing share;
rejected image imports left a fresh unpublished project directory. Now reject
empty/dot/path/control/oversized basenames before cleanup and remove only the
fresh unpublished import root on preparation failure. Android closes a prepared
owner on stream-close failure and removes owned partial share output on failure.
Existing owner/source/history survive; subsequent ordinary Manual remains usable.
No user-selected source, retained project or committed revision is deleted.
Focused security regressions: Android 2/2 PASS, iOS 2/2 PASS, after recorded red
runs. Fixtures are synthetic; raw before/after logs and xcresults are local.

Controlled Android old-screen replay (with corrected test harness/current catalogs
and security-fixed adapters) reproduced Vietnamese Tone/Color missing and Share
clipping at 200% text. It is not a whole baseline build. Current tests pass 2/2:
English/Vietnamese, SetProgress alternative semantics, no unintended publication,
48dp export targets and translated Share label. iOS max Dynamic Type export test
passes in both light and dark, each covering English/Vietnamese with stable native
revision count and >=44pt Share control. Added localized generated review captions
and action descriptions; native tool IDs, raw canonical parameters and units stay
verbatim. No reordering, rounding, approval or native authority moved into UI.
The full normal-text iOS run exposed export buttons only20–22pt tall; that
new assertion failed rather than being weakened. Export format/destination/cancel
labels now have48pt content hit regions (>=44pt assertions retained) and the regression also checks
Photos/File/JPEG/PNG. The original Simulator result-back failure was inspected through its recorded
accessibility hierarchy: Manual controls/revision2 were present, but the entire
navigation toolbar was empty. A mode-dependent .id replaced the principal item
during review departure; retain stable toolbar identity instead. Existing UI
assertions for returning to Manual and leaving result review remain unchanged.
Human AT/contrast/focus acceptance is explicitly NOT OBSERVED.

## Native reliability, sanitizer and source provenance

New public-C-ABI fuzz test uses deterministic seed 0x50483131: valid seed,
4,096 byte mutations,64 random 8,192-byte inputs and empty/8,193-byte boundaries.
4,163 calls:6 accepted,4,157 rejected;33,328 checks, zero failures. Failed outputs
preserve sentinel bytes; every probe leaves ordinary history unchanged; no human
approval occurs; Manual select/update/cancel still works. Actual ASan/UBSan run
also passes with halt-on-error. This bounded mutation test is not exhaustive fuzzing.

Apple ASan cannot enable LeakSanitizer. Darwin-only detect_leaks=0 preserves
ASan/UBSan instrumentation and active negative probes; Linux keeps detect_leaks=1.
The active probes must report the real shared-core heap overflow (exit86) and
signed overflow (exit87). UBSan exposed a TEST-ONLY allocator header misalignment
on Darwin; alignas preserves C++17 default-new and max_align_t alignment with
compile-time checks. No production allocator/numerical code changed.
Final full local sanitizer suite:67/68 PASS inside sandbox; unchanged Apple Metal
inference corpus returned provider unavailable (15). With authorized host Metal
access that same test PASS:105 vectors,36,727,185 checks,0 failures,release_trust=0.
Thus all68 tests verified across explicit environment scopes, not a fabricated
single green sandbox run. New fuzz environment subsequently checked separately.
Apple leaks independently reports0 leaks/0 bytes for each105-lifetime workload.

The additive phase-11-source-v1.json binds before/after LF-normalized hashes,
new sources and109 historical JSON evidence digests. Historical manifests remain
unchanged; legacy fingerprint tests follow explicit successor links and behavioral
assertions continue reading current code. Public C ABI, Schema3, shared production
editing code/numerics, approval/history and GPU admission policy remain unchanged.
Local complete Node suite202/202 PASS; Linux-only emulator orchestration test stays
in both hosted workflows (GNU realpath portability limitation recorded previously).

## Host performance and memory observations

Mac Studio M5 Max, macOS27.2, Xcode27.2 Clang, Release/O3 core and O2 harness,
strict floating point. Synthetic 1024x768 and3000x2000 PNGs;5 warmup then100 measured
complete lifetimes,4 previews each. p50 median, p95 nearest-rank ceil(.95*n)-1;
400 individual preview samples per size. Raw ordered samples/source/harness hashes
and all summaries: evidence/phase-11-local/host-measurements.json and host JSONL.
Normal development-host conditions, not isolated mobile/thermal certification.

| Operation | 0.8MP p50 / p95 ms | 6MP p50 / p95 ms |
| --- | --- | --- |
| Open/import | 47.346 /49.471 |87.390 /125.151 |
| Individual preview |13.977 /15.244 |49.085 /86.067 |
| Manual Apply |10.642 /11.233 |16.262 /23.579 |
| PNG export |31.865 /34.196 |174.038 /322.354 |
| JPEG export |17.457 /18.831 |136.684 /244.690 |

Cumulative peak RSS maxima116,916,224 and332,759,040 bytes. Post-destruction live
RSS first/last25 medians116,867,072/116,883,456 bytes and328,876,032/298,811,392
bytes respectively. Allocator/OS caching influences RSS; finite observations plus
zero exit leaks do not prove arbitrary-lifetime boundedness. Resource leases,
fault injection, cancellation and Manual recovery are checked by retained native
suites; there is no new total-process RSS cap. Host preview dimensions/workloads
are not the T0 720px or physical12MP/frame/thermal gate. No mobile target PASS.

## Security/dependency and commerce audit

Android app manifest requests no Internet, camera, microphone or broad storage
permissions. FileProvider is private, grants selected read access; backup and
transfer rules exclude all private domains. iOS picker security-scope handling,
private project backup exclusion and completeUntilFirstUserAuthentication
protection retained; privacy manifest declares no tracking/collected data.
No production logging/network calls found in owned app/native paths reviewed.
Release isolation scans cover test hooks/unadmitted models and assets. These
source observations do not claim a physical packet capture or OS backup execution.

Fresh public OSV query covers289 current pinned Maven coordinates;107 release
runtime coordinates returned0 findings,20 coordinates flagged only upstream build,
test or Swift-export tooling. evidence/phase-11-local/dependency-advisories.json
records exact coordinates, configurations and advisory IDs/date. No lock/artifact
verification weakening or blind transitive override. Tooling risks remain tracked
for supported upstream maintenance; synthetic tests use no private photo/account
material. This is not exhaustive advisory clearance or independent security review.

Pinned libpng1.6.59 includes the compressed-ancillary fix described by its
[maintainers](https://www.libpng.org/pub/png/libpng.html). New
[zlib1.4 security notes](https://zlib.net/) were reviewed against pinned1.3.2:
gz/minizip/inflateBack paths are not compiled into our codec subset; owned paths do
not call inflateGetDictionary, inflateSync, inflatePrime, deflatePrime or
 deflateResetKeep. No verified reachable new flaw was reproduced; future codec
maintenance remains required. [SQLite's advisory guidance](https://www.sqlite.org/cves.html)
is evaluated against owned SQL/private project storage, not arbitrary SQL import.
See docs/DEPENDENCIES.md for immutable pinned versions/provenance/license rules.
Do not interpret lack of a reproduced reachable flaw as absence of vulnerabilities.

G10 source inventory found no StoreKit/Play Billing integration or entitlement
state machine. MONETIZATION.md documents14-day local trial,$0.99/month,$19.99
lifetime, but offline subscription grace is proposed pending ADR. No sandbox
accounts/configuration or signed-store results supplied. Implementing a guessed
commercial contract would change product policy; external binding contract and
sandbox prerequisites were requested. No purchase, refund, restore or offline
entitlement outcome is claimed.

## Integration and remaining external actions

Platform results and local artifact digests are recorded in
evidence/phase-11-local/validation.json. Initial failures are preserved, not hidden;
focused corrections are distinguished from the complete hosted final-source suites. Final implementation must
pass Foundation workflow-lint/portable Ubuntu/portable Windows/sanitizer/
apple-boundary/android-boundary arm64-v8a/armeabi-v7a and Native source/android-app/
ios-app: exact SHA, completed SUCCESS, attempt recorded. Prior SHA is not substituted.

No physical Android is connected. Paired iPad8,9 is unavailable and Apple iPhone17
is simulated; project has no specified development team. No credential/keychain,
provisioning or account changes performed. Provide eligible physical hardware and
explicit existing signing/device authorization, physical G8 cohorts, human G11
walkthroughs and approved G10 contract/implementation/sandbox. The executable
handoff is docs/plans/phase-11-external-validation.md. Independent Principal
Architect review is pending; this engineering report does not certify itself.
CPU reference VALIDATED; apple.metal.v1 UNKNOWN; android.vulkan.v1 UNKNOWN.
DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED. Phase11 remains open.
