# Phase 11 remaining engineering evidence and blocker matrix

Starting certified main: 8387b167514b4a01f331ef05db32f22d5b4f9f5b.
Phase11 IN PROGRESS; no acceptance/closure SHA. Phase12 NOT AUTHORIZED.
This is an engineering submission, not independent certification review.
Implementation provenance is additive phase-11-source-v10.json; v1-v9 and all
previous reports/artifacts remain immutable. Exact successor CI is required and
its observed SHA/run receipt is delivered separately after completion.

## A/D — Duo defects and accessibility engineering

The preserved v9 Duo run had four behavioral failures, while the conventional
comparison passed those four. Investigation separated actual layout faults from
UI-event transport rather than treating a subsequent run as accidental recovery.
The nested review ScrollView expanded to1565points in a678point app viewport;
review fields were unreachable. Mode toolbar controls could disappear after
result-review transitions. The editor now owns one bounded vertical scroll view,
with stable mode/export navigation outside that scroll and ordinary Back to Home.
The shared native model/approval/history ownership is unchanged.

Transport fixes prepare actual visible controls before sending one normal tap or
slider gesture: controlled left-padding-gutter scrolling, geometric bounded step
count, centering trailing chips in their own horizontal strip, and no editor-scroll
preparation for popup menu rows. Native selected/category/control postconditions,
raw values, preview retirement, single-use human Apply and revision assertions
remain intact. Existing assertion waits remain20seconds; preparation depends on
actual offscreen distance, not longer waits for a broken state.

Falsified hypotheses are preserved: hiding horizontal indicators and direct center/
upper-point touches did not fix the manual category failure, so those changes were
removed. Temporary synthetic Debug category traces showed no Detail action, not a
native category reset; that logging/test flag was removed. Ordinary taps after
horizontal visibility preparation then satisfied the original selected+Blur
postcondition. No direct model selection replaced UI activation.

Preserved local logs/results under build/phase-11-iphone-xr/signing-ready:

| Evidence | Observed result |
| --- | --- |
| all-ui-v9 / conventional-ui-v9 | Original Duo15/19 with4FAIL; original comparison4/4PASS; immutable history |
| duo-single-scroll / duo-fixed-header | Intermediate failures retained; bounded layout/fixed navigation verified progressively |
| duo-review-left-gutter | Complete review edit/remove/restore/explicit Apply PASS |
| duo-popup-transport | All3 newly exposed popup transport cases PASS after helper correction |
| duo-manual-horizontal-preparation | Full manual gesture/history/cancel/background/reopen PASS, selected/control assertions intact |
| remaining-duo-final | Full19/19UI +16/16affected ShellTests PASS,0failures |
| remaining-conventional-final | Same19/19UI +16/16ShellTests PASS,0failures |
| remaining-largest-ax | English/Vietnamese export/failure/manual recovery test PASS at verified largest Dynamic Type; prior setting restored |
| remaining-largest-duo | Same largest-size bilingual test PASS on Duo; previous normal size restored |

The CI controller now explicitly sets and records the largest Simulator content
size for the bilingual test and restores it on exit. A test name alone is not
proof of a large-text environment. Source-level layout checks now require a single
bounded scroll owner and fixed navigation instead of the obsolete nested-scroll
implementation literal. SQLite private-extern confinement checks cover all three
xcodebuild invocations, including the new large-text run; no check was removed.

These are automated software results. Human focus/VoiceOver/TalkBack/contrast and
alternative input acceptance remain outstanding. The operator-ready runbook is
[phase-11-human-accessibility-runbook.md](../plans/phase-11-human-accessibility-runbook.md),
with actual setup/actions and an unfilled observation form.

## B — G7 source and failure boundaries

[Security source audit](phase-11-security-source-audit.md) inventories scoped
import/export, private roots, backup/protection, cleanup, permissions, dependencies,
network/log surfaces and native input/resource boundaries. Existing certified
malformed-input, sanitizers, failure recovery and privacy evidence is reused within
its scope; no stale physical observation is relabelled for changed UI source.
New iOS tests cover all Photos authorization classifications and preservation of
existing user data after failed overwrite. Android tests inject stream-close
failure and require one pending-destination cleanup with no publication.

Affected Android export/security/English-Vietnamese200% text tests passed7/7 on
an owned isolated ARM64 Emulator. A prior shared-emulator run failed Vietnamese
with missing Compose hierarchy; it is retained in remaining-android-focused.log.
Activity loss and a foreign app resumption were observed around failure/teardown;
causality was not proven. A separate repository-local AVD changed the test
environment, with no product-code change or retry loop. Gradle replaces its result
directory; copied isolated XML is not misrepresented as the original failed XML.
The owned emulator was stopped after testing; the shared emulator was untouched.

Unsigned generic iOS Debug/Release builds PASS, and existing model/release scanners
PASS. The final documentation check exposed an accepted-only ADR assumption. The validator
now requires the exact PROPOSED/NOT APPROVED status only for ADR0024, retains accepted
status for every existing ADR and requires all substantive ADR sections.
Source/documentation checks pass208/208 including three untrusted-commerce
boundary tests; exact-source full hosted certification remains mandatory.

## C — G8 conformance versus unsatisfied product target

[Resource analysis](phase-11-resource-rejection-analysis.md) traces actual XR
RESOURCE8 observations to frozen working/admission accounting. Two6MP RGBA32F
rasters already need192MB versus T0's128MiB cap. A12MP working raster needs192MB
and exceeds both8Mpixel and128MiB single-working-raster limits on every tier.
These are conformant denials with verified smaller-image manual recovery, not
large-image performancePASS. The12MP export requirement remains inconsistent
with current end-to-end representation. A bounded CPU streaming/tiled resolution
is proposed for architectural/compatibility review; no ceiling, downsampling,
ABI/schema, numerical behavior or product target is changed here.
Existing XR raw timings/memory evidence stays at its original source and scope.
No new benchmark was run solely to duplicate it. The physical cohort worksheet
specifies actual tier/caps, power/environment, samples/percentiles, UI versus decode,
thermal/pressure/recovery and applicable existing thresholds without invented limits.

## E — G10 proposal and safe infrastructure

[ADR0024](../adr/0024-entitlement-policy-proposal.md) is PROPOSED, not approved.
It preserves MONETIZATION's baseline and explicitly exposes unresolved grace,
clock/trial/family/refund/cache and verification-architecture decisions. Official
Apple/Google primary documentation is linked. No store account or purchase was used.

Development-only entitlement-observation.mjs validates bounded allowlisted
metadata and always returns UNTRUSTED_OBSERVATION/requiresStoreVerification.
Malformed/oversized input, forged trust/secret fields and unknown states reject
without echoing content. PURCHASED/RESTORED strings or a digest never grant access.
It is not imported into the apps, does not persist state, verify receipts, create
SDK integrations or gate editing, and is not sandbox purchase evidence.

## Final acceptance blocker matrix

| Category | Completed independently certifiable software scope | Still required / responsible prerequisite |
| --- | --- | --- |
| Software engineering | Duo layout/navigation/event transport; full Duo/conventional regressions; en/vi largest-text automation; targeted security failure tests; source/release checks; bounded untrusted observation tooling | Exact successor Foundation7/7 and Native3/3 must be observed; independent software acceptance is separate |
| Physical devices / G7-G8 | Source/privacy audit, failure recovery and accounting analysis; original XR evidence preserved | Supported LOW/MID/HIGH Apple and Android cohorts; authorized traffic/OS permissions, real pressure/storage/restart, sustained thermal and UI/frame measurements |
| Human accessibility / G11 | Automated labels/values/targets/localization and operator-ready runbook | Named operators' actual VoiceOver/TalkBack, focus, contrast and keyboard/switch observations for required languages/scales/appearances |
| Product Owner commercial policy / G10 | Proposed binding ADR0024; no commercial choice silently adopted | Approve trial eligibility/clock behavior, subscription grace, lifetime/refund/family/restore rules and cache/recovery policy |
| Architecture / G8-G10 | Bounded12MP resolution and platform entitlement trust boundaries proposed | Approve compatibility proof/path for12MP; choose legitimate Android verification model consistent with local/private editing; no new server authorized here |
| Store accounts / sandbox / G10 | No fake purchase or grant; observation transport remains untrusted | Human-owned products/pricing/configuration/contracts/accounts; authentic signed sandbox transaction/restore/refund/expiration/pending/offline/cache evidence, then approved implementation |

DH-APPLE-METAL-01 remains PASS/CLOSED for its independently accepted XR scope;
it was not rerun or reopened. Metal implementation/contract source is unchanged.
cpu.reference.v1 VALIDATED; apple.metal.v1 UNKNOWN; android.vulkan.v1 UNKNOWN.
No production GPU/model/provider, cloud processing, signing or schema/ABI change.
Historical failures are not erased, and green CI cannot close the external gates.
