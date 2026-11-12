# Phase 11 iPhone XR development validation

Status: IN PROGRESS / PHYSICAL EXECUTION BLOCKED. Phase 12 NOT STARTED.
Discovery baseline: 85a8369365b1efae2d7844337656a607d9e0ca96, clean main.
Baseline Foundation 43209508433 7/7 and Native shells 43209509302 3/3,
exact SHA, attempt 1, all required jobs completed SUCCESS.

## Observed compatibility and readiness

Xcode 27.2 detected a paired, wired, booted physical iPhone XR (iPhone11,8,
N841AP), arm64e with arm64 support, iOS 18.7.9 build 22H355. This meets the
application's iOS 16 minimum and architecture conditions only. Metal runtime
capabilities and physical performance have not been measured. Developer Mode
was initially disabled; a later discovery found it enabled with developer
services available. No account/signing changes were performed by the agent.

The Mac has zero valid code-signing identities, zero profiles in the standard
Xcode/MobileDevice provisioning directories, and no project DEVELOPMENT_TEAM.
Automatic signing alone does not supply those prerequisites. No signed app was
installed and no physical XCTest was executed. Metal is NOT RUN, not a failed
parity result. No physical hardware JSON attachment or xcresult exists yet.

Minimal owner action: make an existing authorized Apple Development signing
identity (including its locally managed private key) and matching provisioning
available in Xcode, then supply the authorized Team ID. Never send passwords,
private keys or signing material through chat. Creating or changing accounts,
certificates, teams or profiles requires separate explicit authorization.

## Reproduced controller defect and correction

The clean baseline hardware controller's build settings omitted the Sodium and
Metal target private-extern preservation used by the certified Apple scripts.
An unsigned generic physical-SDK build-for-testing reproduced five undefined
symbols: metal_available, make_metal_worker, sodium_init,
crypto_core_ed25519_is_valid_point and crypto_sign_ed25519_verify_detached.
The controller now includes both missing per-target settings. The otherwise
matching unsigned build-for-testing succeeds. This is build evidence only;
CODE_SIGNING_ALLOWED=NO was a diagnostic command, never hardware certification.

Focused controller/provenance checks: 8/8 PASS. Complete locally portable source
suite: 203/203 PASS, zero skips. The Linux-specific emulator infrastructure
check remains covered by hosted CI. Additive source-v4 binds the changes to
85a8369; all earlier source manifests remain immutable. Product source, frozen
ABI, Schema 3, numerical behavior, signing policy and GPU admission are unchanged.
Both full hosted workflows on the new implementation SHA are required; the
observed final receipt supplies their exact SHA, run IDs, attempts and jobs.

## Evidence and executable handoff

Public sanitized observation: evidence/phase-11-iphone-xr/readiness.json.
Local private raw evidence (ignored, do not publish device identifiers):
`build/phase-11-iphone-xr/device-details-private.json` (initial),
`device-details-current-private.json` (Developer Mode enabled),
`signing-current-private.log`, `destinations-private.log`, `lock-private.json`.
Build evidence in that same directory: `controller-unsigned-preflight.log`
(reproduced link failure), `controller-fixed-preflight.log` (successful unsigned
build), and `source-tests.log` (203/203). Sanitized receipt retains SHA-256 hashes.

After signing readiness, use the existing clean-checkout physical controller
with physical destination and existing Team ID. Retain its commit.txt, complete
xcodebuild.log, hardware.xcresult and exported pixaura-metal-hardware.json
attachment. Independently verify pipeline creation, dispatch, completion,
readback, 14/14 CPU parity, fallback disabled and exact source SHA. A successful
process exit alone is insufficient. Never substitute these unsigned builds.

## G7, G8 and G11 scope

Physical G7 privacy/offline/import/export/lifecycle/pressure: NOT RUN.
Physical G8 latency, memory, repeated-operation trend and thermal measurements:
NOT RUN; there are no physical timing samples, memory values or p95 claims.
No zero measurements or synthetic host numbers stand in for missing device data.
The execution and sampling protocol is docs/plans/phase-11-external-validation.md.
Use synthetic images, app-scoped instrumentation and preserved raw samples.
One XR does not qualify low/mid/high Android and Apple cohorts.

G11 operator walkthrough is ready in that protocol: English/Vietnamese,
normal/maximum text, light/dark, actual VoiceOver labels/values/actions/focus,
Manual Apply/Cancel, Undo/Redo, Assist/Review, import/export, lifecycle, keyboard
or alternative input and contrast observations. Record operator/date/source,
settings, observations and failures for each row. Human observations remain
NOT OBSERVED; no automated success is represented as a VoiceOver walkthrough.

cpu.reference.v1 VALIDATED; apple.metal.v1 UNKNOWN; android.vulkan.v1 UNKNOWN.
DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED. Physical Metal and
independent review remain mandatory. G8 physical cohorts/thermal, G10 binding
commerce policy/implementation/authorized store sandbox and G11 human iOS/Android
assistive-technology evidence remain outstanding. Phase 11 is not accepted or
closed; no closure SHA exists.
