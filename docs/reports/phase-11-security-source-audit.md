# Phase 11 source security/privacy audit successor

Baseline8387b16; software audit, not physical OS/traffic acceptance. Prior negative
reproductions/fixes and original raw evidence stay immutable. No new networking,
permissions, telemetry, providers, caches or signing changes are introduced.

| Surface | Source and verified contract | Remaining observation |
| --- | --- | --- |
| iOS import | ManualEditorModel worker balances security-scoped URL access with defer, creates fresh protected/no-backup UUID root, releases failed unpublished owners/roots and preserves current source/history | Real picker/denied-provider UI and device storage pressure |
| Android import | ContentResolver stream copied through64KiB bounded buffer under native import limit; staging finally deleted; fresh unpublished noBackupFilesDir root/owner cleaned on failure; original URI untouched | OS provider permission revocation/process termination |
| Export/share | Native committed revision only. iOS atomic sibling temporary writes, rejects directory target, removes partial; add-only Photos request/denial classifier. Android pending MediaStore insert/write/close/publish with cleanup on failure, CreateDocument and read-only grant-scoped FileProvider cache/export path | OS Photos/picker denial, real destination delivery, storage-full/kill |
| Backup/private storage | iOS complete-until-first-user-auth protection/no-backup roots, including failed import cleanup. Android allowBackup=false and cloud/device-transfer exclusions, private noBackup roots | Physical backup and lock/unlock acceptance beyond existing XR directory metadata |
| Dependencies/network/logging | App sources contain no URLSession/HTTP client or Android INTERNET permission; release scanners and dependency locks remain required. UI error presentation is distinct from uploading/logging user content | Authorized process traffic capture is needed for zero-photo-traffic claims |
| Native input/resources | Existing malformed parser/path/fuzz, allocation/sanitizer/race/cancel contracts; no ABI/Schema3 or native code change | Real OS storage/memory pressure and process restart/cohorts |

Additional focused negative coverage: iOS all Photos authorization classifications
and failed overwrite preserving an existing directory's contents; Android destination
stream-close failure removes only the pending URI and never publishes. These are
software fault injection, not a human OS permission walkthrough. Existing tests
cover invalid cache paths, rejected imports, stale export/history, cancellation,
partial-output cleanup and manual recovery. No new defect is claimed without a
failing reproduction. All applicable tests/release checks must be observed before
this audit's software evidence is certified; final receipt records actual results.

External G7 script: use synthetic images; record installed source and radio state;
operator disables radios for offline flow; authorized OS process traffic tooling
captures import/manual/Assist/Auto/Apply/Cancel/history/export without uploading
content. Record real permission dialogs, cancel/deny recovery, app termination/
restart, scoped destination writes and backup/protection observations. No private
images, credentials or user storage contents in shared evidence. Keep G7 OPEN until
these device/operator observations are accepted.
