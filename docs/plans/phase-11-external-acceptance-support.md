# Phase 11 external acceptance support

Status: ENGINEERING COMPLETE — EXTERNAL ACCEPTANCE PENDING. Phase 11 is not
closed. Phases 12–14 are not authorized. No row below is a completed observation.

Certified product source:

`a3551b2e0fb21896ae80fb6ae8fb7fa195cc2f14`

Documentation commits after that SHA do not change the application and are not
a new certification. Build and install from `a3551b2`. The historical sections
of `docs/plans/phase-11-final-operator-runbook.md` that call the implementation
uncommitted, or say the 12 MP editor is unimplemented, describe an earlier
freeze. The accepted application at this SHA includes the 12 MP manual editor.
Checkout stays off. `cpu.reference.v1` is VALIDATED. Metal and Vulkan providers
stay UNKNOWN. DH-APPLE-METAL-01 stays the historical iPhone XR pass at
`33c809f6a4b91300491cc1e211a10dbdbccaf7f9`; it is not a G8 cohort result for
this SHA and it is not provider admission.

Use these existing procedures, with this page as the current operator entry:

- `docs/plans/phase-11-final-operator-runbook.md`
- `docs/plans/phase-11-external-validation.md`
- `docs/plans/phase-11-human-accessibility-runbook.md`
- `QUALITY_GATES.md` budgets and `DEVICE_CAPABILITY_STRATEGY.md` tier caps

The completed-item record is `docs/reports/phase-11-external-acceptance-ledger.md`.
Every mandatory external item is NOT OBSERVED. Do not write PASS from an
automated log.

## Evidence classes

| Class | What it can support | What it cannot support |
| --- | --- | --- |
| Automated CI, host, Simulator, or Emulator | The already accepted engineering scope at `a3551b2` | G7 physical privacy, G8 device cohorts, G10 authentic sandbox, G11 human assistive use |
| `physical_device` | G7 and G8 on the named device and OS | A store transaction or a screen-reader observation |
| `authentic_sandbox` | G10 Apple StoreKit or Google Play Billing outcomes from a real test account | A synthetic signature, a cached flag, or a document that names a price |
| `human_accessibility` | G11 VoiceOver, TalkBack, contrast, focus, keyboard, or switch observations by a named person | An accessibility-node test or a screenshot reviewed only by software |

`scripts/phase-11-external-evidence.py` summarizes operator-supplied sample
numbers and checks that an external record names this source. Its summary is
`NOT_A_GATE`. It does not measure a device.

## Signed test builds

The agent does not create accounts, certificates, profiles, keystores, or
store products, and does not commit the local iOS project file.

1. Use a clean product tree at `a3551b2`. Leave
   `platforms/ios/PixAuraAI.xcodeproj/project.pbxproj` unstaged when it contains
   the owner's existing signing edits. Do not reset or clean the worktree.
   Confirm product files match `a3551b2` before installing. Record the installed
   source as `a3551b2` and record that local signing edits are outside git.
2. iOS development install: open the existing project in Xcode and use the
   development team already configured on this Mac. Archive or install Release
   to a device the owner already registered. Do not change the team, bundle id,
   or entitlements from this procedure, and do not commit the project file.
   A development-signed install can be used for G7, G8, and G11. It is not an
   App Store or TestFlight build and cannot satisfy G10.
3. Android device install for G7, G8, and G11: the repository has no release
   signing configuration, and `*.keystore` is gitignored. `assembleDebug` uses
   the local debug keystore and can be installed on a device the owner
   authorizes. Record the build as debug-signed, not Play-signed. An unsigned
   CI release APK is not a device acceptance build.
4. G10 Android requires a Play-signed build on a test track the owner already
   controls, plus the privacy admission in the operator runbook. G10 Apple
   requires an App Store-signed Sandbox build and a real Sandbox tester. This
   procedure does not create those accounts or enable checkout. Keep the
   shipping checkout path off until authentic catalog, accounts, and privacy
   admission exist.

Record for every install: source SHA, platform, configuration, signing kind
(local development, debug keystore, or store-signed), installer, device model,
and OS. Do not record certificates, passwords, tokens, or account names in the
shared ledger.

## Measurement

Compare samples only with the existing targets. Do not change them.

- Ordinary 60 Hz interaction frame p95 at or below 16.7 ms.
- Warm T0 720 px basic-adjustment preview p95 at or below 150 ms.
- Cancel acknowledgement at or below 250 ms.
- Import and project UI response at or below 100 ms, excluding decode.
- 12 MP JPEG export at or below 10 s on T1 and 30 s on T0.
- Tier working-set targets in `DEVICE_CAPABILITY_STRATEGY.md`: T0 at or below
  128 MiB incremental, T1 at or below 256 MiB incremental. These are profiling
  targets. Do not raise an admission cap or downsample an export to meet them.

On each physical LOW, MID, and HIGH Apple device and each physical LOW, MID,
and HIGH Android device, keep the native tier and cap the app actually reports.
Discard five warmups. Retain 100 ordered admitted samples for each separate
operation: open, decode, preview, Apply, cancel, JPEG export, and PNG export.
Keep quota denials and resource rejections out of the latency series and record
them separately. Record power, ambient conditions, and thermal state in time
order. Host, Simulator, and Emulator timings stay in the automated class.

Feed one sample per line, in the measured unit, to
`scripts/phase-11-external-evidence.py summarize`. Nearest-rank percentile uses
rank `ceil(percent / 100 * count)` on the sorted samples. Keep the raw series
with the summary. A summary without the raw series is not evidence.

## Collection

Copy one record per completed attempt. Leave unfinished items NOT OBSERVED.

| Field | Record |
| --- | --- |
| Gate and journey | |
| Evidence class | `physical_device`, `authentic_sandbox`, or `human_accessibility` |
| Result | PASS, FAIL, or NOT OBSERVED |
| Source SHA | `a3551b2e0fb21896ae80fb6ae8fb7fa195cc2f14` |
| Build kind and signing kind | |
| Device model, OS, cohort | |
| Operator and date | |
| Language, text size, appearance, assistive service, input | G11 only |
| Raw log, sample, or spoken-output path | Stored outside git when it contains device or account identifiers |
| Automated substitute | false |

G11 journeys and the spoken-output form stay in
`docs/plans/phase-11-human-accessibility-runbook.md`. G7 traffic and permission
steps, G8 sample counts, and G10 sandbox cases stay in
`docs/plans/phase-11-final-operator-runbook.md`.

## Closure package, not yet submitted

Submit a closure package only after every mandatory G7, G8, G10, and G11 item
in the ledger is PASS with external provenance. The package cites `a3551b2`,
the CI runs already recorded in `docs/reports/phase-11-engineering-delivery.md`,
and the ledger. It keeps automated results in their own section. It does not
include signing secrets, store tokens, or personal photos. Phase 12 does not
start from this page.
