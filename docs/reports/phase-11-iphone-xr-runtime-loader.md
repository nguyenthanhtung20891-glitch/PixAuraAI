# Phase 11 physical iPhone XR runtime loader correction

Phase 11 IN PROGRESS; Phase 12 NOT STARTED. No acceptance or closure claim.
Predecessor550fd9d79fc761362764a39720f11c79af20e146 is software-certified:
Foundation43264776308 7/7; Native43264777107 3/3, exactSHA, attempt1.

## Controlled physical execution and defect

Existing Apple Development identity and unexpired debuggable app provisioning
were inspected without modification. Profile matches the existing Team, app
bundle and physicalXR. Device: iPhone11,8, iOS18.7.9(22H355), paired/wired,
DeveloperMode enabled. Owner's Xcode Team/project-format edits remain local.
No personal signing settings, profiles or credentials are included in this change.

A detached clean predecessor checkout was used. The original controller first
stopped because the unused UI-test runner had no matching profile. An ignored
local XCODE_XCCONFIG_FILE overlay signs PixAuraAI and PixAuraAITests while leaving
only the unselected PixAuraAIUITests runner unsigned. It does not disable signing
for the actual physical executable or unit bundle, change fallback, alter test
selection, provision profiles or change accounts. The initial signing wait ended;
app signature verified, but launch required owner trust of the development
certificate on iPhone. After owner continuation, launch passed that boundary and
crashed before XCTest connected: dyld could not locate PixAuraCore's framework.
No Metal test executed on the predecessor; this is not a parity failure.

Actual otool inspection showed the framework embedded under app/Frameworks,
but app executable/debug dylib had only loader/executable-root and host build
PackageFrameworks search paths. Simulator/host resolution hid the missing device
runtime path. Unit bundle already had its appropriate framework paths.

## Small correction and evidence boundary

Add $(inherited) @executable_path/Frameworks to both app configurations and the
deterministic project generator. Add a focused regression requiring the path
in Debug and Release. No native algorithm, ABI, Schema3, Metal contract, tolerance,
provider/admission, ownership, Apply or revision behavior changes.
Additive source-v6 preserves all earlier provenance receipts.

Local source suite204/204 PASS. Full hosted Foundation7/7 and Native3/3 are required
on the implementation successor. Both configurations must build; device launch
and Metal controller must then execute from its clean exactSHA. Physical PASS
requires actual JSON attachment, pipeline/dispatch/completion/readback,14/14 parity,
fallback disabled and independent review. No successful process exit alone
changes DH-APPLE-METAL-01 or inference-provider trust.

Raw private artifacts remain ignored under build/phase-11-iphone-xr/signing-ready:
physical-controller-private.log (missing unused runner profile),
physical-controller-local-signing-private.log (untrusted developer certificate),
physical-controller-trust-retry-private.log (dyld crash), source-v6-tests.log,
rpath-unsigned-build-corrected.log and rpath-release-build.log. The first unsigned
build diagnostic incorrectly retained the signing overlay and required a Team;
that invocation is retained as rpath-unsigned-build.log, not counted as a pass.
Controller xcresults/logs/source sidecars remain in the detached checkout's
build/gpu-apple-hardware/run.* directories; no stale result substitutes a new run.
Device identifiers, signing identifiers and raw phone logs are not public evidence.

G7 airplane/traffic/permissions and true OS-pressure observations, G8 representative
latency/memory/thermal cohorts, G10 policy/implementation/store sandbox and G11
human English/Vietnamese VoiceOver/TalkBack/focus/contrast/alternative-input
walkthroughs remain mandatory. Existing operator protocol:
docs/plans/phase-11-external-validation.md. CPU VALIDATED; Metal/Vulkan UNKNOWN;
DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED pending reviewed physical
evidence. No Phase11 closure or Phase12 work.

## First real physical Metal execution and bounded-consumer fixture successor

Implementation23ea6ff556ef7c7fbf94c59aa97218128da47391 launched on the physicalXR.
Controller exited0 and XCTestPassed1/1, zero skips. Exported JSON reports AppleA12
GPU, iPhone11,8, iOS18.7.9(22H355), hardware/pipeline/dispatch true,
expected_test_count=test_count=parity_passed=14, statusPASS. Engineering verification
checked the attachment against the fresh controller source sidecar. Exact source
uses allow_fallback=0, requires used_gpu=1 per case, waits for completed Metal
commands without error before readback and shared CPU parity. Independent review
is pending; this execution result does not promote apple.metal.v1 or discharge
DH-APPLE-METAL-01 automatically. Formal classifications above remain unchanged.

The subsequent broad physical app suite was NOT a pass:14/15 tests succeeded;
testBoundedPreviewPlatformConsumer had6 cascading assertions after decode_open
rejected its root with status6. Its test fixture used Foundation's
resolvingSymlinksInPath, while production ManualEditor uses POSIX realpath to
satisfy native O_NOFOLLOW_ANY storage validation. The fixture successor follows
the existing production canonical-root contract, keeps all preview pixel/alpha/
bounds assertions and adds a path-readiness JSON attachment containing booleans
only. No native storage/security rule is relaxed and no production behavior is
changed by this test correction. A physical rerun must verify this diagnosis.

Local source204/204 PASS and unsigned physical-SDK build-for-testing PASS for the
fixture successor. Additivev7 preservesv1–v6. Its new clean exactSHA still requires
physical execution and full hosted7+3. Raw first Metal JSON, export manifest and
XCTest summary are under signing-ready/metal-attachments,
metal-evidence-sanitized.json, metal-xctest-summary-private.json; the exact fresh
bundle directory is recorded in metal-result-path.txt. First broad-suite bundle
is recorded in device-unit-result-path.txt. No stale result is counted as the
successor's hardware evidence. No Phase11 closure or human walkthrough claim.
