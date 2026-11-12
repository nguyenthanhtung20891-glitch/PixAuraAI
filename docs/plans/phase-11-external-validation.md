# Phase 11 external acceptance protocol

Status: NOT OBSERVED. This is a reproducible handoff, not test evidence or a waiver.
Use synthetic images only. Record exact clean implementation SHA, OS/device,
language, text scale, assistive service/version, operator, date, outcome and logs.
Keep signed-store tokens, device IDs and private content outside public reports.
Phase 12 is NOT STARTED.

## Physical Metal — mandatory DH-APPLE-METAL-01

Provide an eligible connected/unlocked physical iPhone/iPad, trusted for development,
an already configured authorized signing team and explicit device/signing access.
Do not modify identities, accounts, profiles or team settings to infer permission.
Read ADR 0018, docs/contracts/gpu-tile-v1.md and the existing hardware script first.
On a clean exact-source tree execute scripts/check-gpu-apple-hardware.sh with the
physical destination and authorized team. Preserve complete xcresult/logs and JSON
XCTest attachments, eligible hardware/OS metadata, real pipeline creation,
dispatch/completion/readback and all 14/14 CPU parity cases with fallback disabled.
Obtain independent review. A host/Simulator/Emulator or successful detection/smoke
run cannot satisfy this gate. Keep Metal UNKNOWN until the formal protocol passes.
Vulkan tile evidence and inference-provider admission remain separate policies.

## G8 physical device cohorts

Supply supported low/mid/high Android and Apple devices and documented cohort
selection. Capture device/OS, source/build/configuration, input dimensions/bytes,
battery/power/ambient conditions, warmup/sample counts and concurrent workloads.
Measure open/import UI latency separately from decode; 720px warm preview;
Manual Apply, cancellation; 12MP JPEG and PNG export; frame timing, peak memory,
post-operation memory, pressure recovery, thermal state/throttling and cooling.
Use adequate independent samples for nearest-rank p95; retain raw ordered samples
and p50/p95/max. Compare only applicable documented QUALITY_GATES targets. Verify
resource leases return after cancellation/close and ordinary Manual still works
after denial. Host timings are not substitute measurements; add no RSS ceiling.

## G7 offline/privacy and reliability device observations

Start in airplane mode with Wi-Fi/data disabled. Use authorized OS traffic tooling
to observe the app UID/process while importing, editing, Assist, Auto Review,
explicit Apply/Cancel, Undo/Redo, reopen and export to selected destinations.
Record zero photo traffic or investigate actual packets; do not upload content.
Check scoped picker/Photos permissions and denied/cancelled recovery, OS backup
exclusion/file protection, temporary-share cleanup, process termination/relaunch,
storage-full and memory-pressure recovery. Never treat Simulator/Emulator evidence
as all OS/device versions or include credentials in traces.

## G11 human assistive-technology walkthrough

Run each row on Android TalkBack and iOS VoiceOver, English and Vietnamese,
normal and largest supported text, light and dark appearances. Use actual screen
reader focus, external keyboard/Switch Access or platform alternative input.
Record focus sequence, spoken label/value/hint, available actions, target reach,
visual contrast evidence and result. Automated semantics existence is insufficient.

| Journey | Required observation |
| --- | --- |
| Home/empty/import | Purpose and privacy understandable; picker reachable; cancel/deny returns focus; errors announced |
| Manual category/tool/value | Category/tool selected announced; label, exact value/unit and bounds understandable; slider alternative actions work |
| Manual Apply/Cancel | Explicit actions distinguish saved revision from disposable preview; cancel does not publish |
| Undo/Redo | Ordinary history state and disabled/enabled actions accurately announced |
| Assist request/cancel/failure | Status/progress/error reachable; cancel announced; Manual remains available |
| Auto request and proposal | Plan provenance/order, exact raw values, range/unit and geometry warnings readable |
| Review edit/remove/move/restore | Each action uniquely labelled; input validation/errors spoken; changed review retires old eligibility |
| Preview comparison | Before/After not confused with committed result; split/zoom/pan accessible; geometry fallback clear |
| Human Apply/denial/stale result | Explicit single-use approval; stale/background cancellation cannot regain authority |
| Result review | Current/previous ordinary revision, Undo/Redo and return to editing correctly announced |
| Export | JPEG/PNG quality, Photos/File/Share reachable at max text; permission/space failure recovers without history mutation |
| Modal/lifecycle/navigation | Entry/exit focus sensible; no trap; background/foreground and reopen preserve authority |

Do not check PASS boxes without a named operator's actual observations. Contrast,
focus order and switch/keyboard operation remain acceptance prerequisites.

## G10 commerce prerequisite

Supply the binding approved entitlement/offline-grace ADR and scope, store product
identifiers/pricing configuration, actual StoreKit/Play Billing implementation and
authorized sandbox accounts/configuration. Current MONETIZATION.md proposes a
subscription grace duration and no matching entitlement implementation exists.
No real-money transactions, new policy, fake licenses or invented outcomes.
Then independently record trial start/end, subscription renewal/expiration/grace,
lifetime persistence, restore, refund/revoke, offline verification and recovery
view/export. Validate $0.99/month, $19.99 lifetime and 14-day trial only against the
approved store configuration; a document is not signed-store evidence.
