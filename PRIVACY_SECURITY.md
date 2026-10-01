# Privacy and security boundaries

## Default data flows
Photo picker -> private immutable original -> local engine/model -> local project -> user-selected new export. No remote image processing; no analytics SDK in MVP foundation. Photos, thumbnails, masks, prompts, embeddings, filenames and EXIF never enter telemetry/crash breadcrumbs. Any future diagnostics are opt-in, redacted and separately documented. Billing receipt verification/model downloads are distinct network flows and must never receive project content.

## Storage and permissions
Use system pickers and scoped access, no broad media permission for import. Request add-only/export permission just in time on iOS; use scoped MediaStore destination on Android. Application-private files and database use OS sandbox and data-protection facilities; secrets only in Keychain/Keystore. Exclude originals, masks, previews, prompts and project databases from automatic cloud backups by default, explicitly configure Android backup/data-extraction rules and Apple backup exclusion attributes. OS encryption protects locked-device data; it is not protection against an unlocked compromised device. No custom crypto substitute for OS facilities.

## Threats and controls
| Boundary/threat | Required control | Evidence |
| --- | --- | --- |
| Malformed image/decompression bomb | Decode limits, overflow checks, cancellation, fuzz corpus | Decoder fuzz/sanitizer jobs; oversized fixtures |
| Native handle/tensor misuse | Checked lengths, ownership, version checks; no exceptions over ABI | Boundary tests + ASan/UBSan |
| Prompt/metadata injection | Data cannot expand tool allowlist or privileges | Adversarial plan tests |
| Malicious/tampered model | Approved signed manifest, digest and operator/size validation | Tamper rejection test |
| Accidental overwrite | Read-only source, managed copy, new output identity | Source hash unchanged across export/failure |
| Stale AI execution | Base revision check and atomic approval | Concurrency/recovery tests |
| Cache/project leakage | Private storage, backup exclusion, redacted diagnostics | Platform policy/device inspection |
| Supply chain | Pinned reviewed dependencies, SBOM, license/security scan | Release review artifact |

Validate imported project references for traversal and cross-project ownership before opening paths. Sensitive content never appears in logs. Preview assets and cancelled candidates are cleaned under project-aware retention; deletion cannot touch outside managed roots. User deletion includes originals, history, masks and caches, with explicit warning before removing sole imported copy. Secure erasure on flash is not promised. Temporary export files are reconciled after interruption.

Before beta: airplane-mode tests and network capture prove editing sends no requests; permission and backup inspection; dependency/model license audit; threat-model review and unresolved risk list. Store networking is isolated from editing. No credential or signing material in repository or CI logs.
