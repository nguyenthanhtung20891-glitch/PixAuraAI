# PixAuraAI Autonomous Engineering Contract

PixAuraAI is an AI-native, local-first photo editor.

The repository root is:

`G:\PixAuraAI`

Codex acts as the autonomous engineering team.
The user is Product Owner and final approver for product and architectural decisions.

Before changing code, read the relevant authoritative documents, including:

- `PRODUCT_SPEC.md`
- `ARCHITECTURE.md`
- `QUALITY_GATES.md`
- `ROADMAP.md`
- `DECISIONS.md`
- relevant ADRs
- relevant subsystem contracts
- the current Phase/Step report

User instructions take precedence over repository guidance.

---

## 1. Autonomous operating model

Within the currently authorized Phase/Step, Codex should work autonomously.

Codex may, without routine human approval:

- inspect repository files;
- edit repository files;
- create repository files required by the authorized scope;
- run PowerShell commands;
- run CMake configure/build/test commands;
- run Node tests;
- run Gradle/Android builds and tests;
- use WSL for compilation, sanitizers, Linux validation and diagnostics;
- inspect Git status/diff/log;
- diagnose failures;
- fix failures;
- rerun validation repeatedly;
- update reports and implementation documentation;
- inspect GitHub Actions results when network access is available;
- continue remediation cycles until gates pass or a true human decision is required.

Do not stop merely because an implementation milestone was reached.
Continue within the authorized scope until applicable gates pass or a documented blocker is reached.

Never label an unexecuted check as PASS.

---

## 2. Workspace safety boundary

Autonomous filesystem modifications are restricted to:

`G:\PixAuraAI`

Do not create, modify, move, rename or delete files outside this repository.

Do not autonomously modify:

- Windows system files;
- Windows registry;
- Windows services;
- system-wide environment variables;
- installed applications;
- unrelated drives or repositories;
- the user's personal files;
- shell profiles outside the repository;
- global Git configuration;
- credential stores.

Reading toolchains, SDKs and system metadata required for build diagnostics is allowed.
Writing outside the repository is not.

Temporary files should be placed inside repository-controlled temporary/build directories whenever practical.

---

## 3. WSL boundary

WSL may be used autonomously for:

- Linux builds;
- sanitizers;
- compilation;
- tests;
- shell validation;
- diagnostics.

When accessing Windows files through WSL, writes must remain under:

`/mnt/g/PixAuraAI`

Do not autonomously modify:

- `/etc`;
- Linux package configuration;
- Linux user profiles;
- unrelated Linux home files;
- system services;
- unrelated mounted Windows paths.

Installing or modifying system-wide WSL packages requires explicit human authorization unless the user has separately authorized environment provisioning.

---

## 4. Git safety policy

All autonomous Git mutations must target only the PixAuraAI repository.

The approved repository is:

`G:\PixAuraAI`

The approved remote is:

`github.com/nguyenthanhtung20891-glitch/PixAuraAI`

Use `scripts/codex-git.ps1` for autonomous Git mutations whenever practical.

Allowed autonomous Git activities within an explicitly authorized implementation or CI-remediation task:

- status;
- diff;
- staging intended repository files;
- unstage;
- normal commits;
- push `main` to the existing `origin`;
- read log and current HEAD.

Never autonomously:

- force-push;
- use `--force-with-lease`;
- rewrite published history;
- run `reset --hard`;
- run destructive `git clean`;
- delete branches;
- delete tags;
- change remote URLs;
- change Git credentials;
- modify global Git config;
- bypass hooks with `--no-verify`;
- amend previously published commits;
- push to a repository other than the approved PixAuraAI origin.

Before committing:

1. inspect `git status`;
2. inspect the diff;
3. verify no generated artifacts, credentials, caches, temporary databases or unrelated files are staged;
4. run applicable validation;
5. keep the commit narrowly scoped.

---

## 5. Phase and architecture authority

Codex may work autonomously only inside the currently authorized Phase/Step.

Do not begin the next Step or Phase unless the Product Owner explicitly authorizes it.

If implementation exposes a conflict with a frozen architecture decision, contract or ADR:

1. stop implementation of the conflicting change;
2. describe the conflict;
3. present technically viable options and trade-offs;
4. wait for the Product Owner / Principal Architect decision.

Do not silently redesign frozen architecture.

Architectural changes must be recorded in:

- `DECISIONS.md`, and/or
- a numbered ADR.

Accepted historical ADRs are superseded, not rewritten.

---

## 6. Current authorized scope

Current scope:

**Phase 2 Step 3 — durable persistence and immutable asset storage**

Authorized work includes:

- SQLite project/document/history persistence;
- immutable verified asset storage;
- crash recovery;
- persistence portability;
- C/C++ and platform boundary integration;
- Step 3 CI remediation;
- Step 3 validation and documentation.

Do not begin Phase 2 Step 4.

Do not implement during Step 3:

- image decoding;
- pixel buffers;
- rendering;
- adjustment kernels;
- GPU processing;
- masks;
- AI inference;
- generative editing;
- production editing UI.

---

## 7. Core product invariants

- Original images are immutable.
- Import copies source bytes into private managed storage.
- Export always creates a new destination.
- Manual, AI-assisted manual and AI edits use the same versioned operation graph and project history.
- Planning never mutates the project.
- Validated AI execution produces a candidate.
- Only approved AI candidates become committed project state.
- Core editing must work offline.
- Photos, masks, prompts and embeddings do not leave the device by default.
- AI uses registered deterministic tools before synthesis is considered.
- Generative editing is outside the MVP unless explicitly authorized later.
- Resource pressure may reduce AI capability or preview quality, but must not disable manual editing.

---

## 8. Native architecture boundaries

The application architecture uses:

- Kotlin + Jetpack Compose on Android;
- Swift + SwiftUI on iOS;
- shared C++17 native core;
- checked/versioned C ABI boundaries.

Platform layers own:

- UI;
- OS integration;
- hardware integration;
- platform lifecycle.

Shared native core owns applicable document/edit/persistence semantics.

Do not duplicate native domain semantics independently in Android and iOS.

Do not perform heavy decode, render, inference or persistence work on UI threads.

No C++ exceptions may cross the C ABI.

---

## 9. Security requirements

Treat all imported data and external metadata as untrusted.

Preserve protections for:

- path traversal;
- symlink/reparse-point attacks;
- malformed SQLite content;
- malformed manifests;
- oversized metadata;
- integer overflow;
- unbounded allocation;
- asset substitution;
- hash mismatch;
- stale handles;
- dangling references;
- SQL injection;
- TOCTOU hazards.

Use bound SQL parameters.

Do not weaken security checks merely to make a platform test pass.

Third-party source must not be modified merely to silence compiler warnings unless explicitly approved.

---

## 10. Dependency policy

Do not introduce a production dependency without reviewing:

- version;
- provenance;
- license;
- security implications;
- binary-size impact;
- build reproducibility.

Do not use floating production dependency versions.

Vendored dependencies must retain reviewed checksums where applicable.

Do not commit credentials, user photos, generated builds, caches or model weights.

---

## 11. Validation workflow

Inspect repository and specifications first.

Then:

1. define measurable acceptance criteria;
2. implement narrowly;
3. add meaningful tests;
4. run applicable checks;
5. diagnose failures from evidence;
6. remediate;
7. rerun tests;
8. update documentation/reporting;
9. repeat until gates pass.

Prefer `rg` for repository searches.

Representative gates include:

### Foundation

`node --test tests/foundation.test.mjs`

### Windows native

`powershell -NoProfile -File scripts/check-native.ps1`

Use the documented Windows compiler fallback when local MSVC is incomplete.

### Sanitizers / Linux

`bash scripts/check-sanitizers.sh`

through local WSL or Linux CI where applicable.

### Apple

`bash scripts/check-apple.sh`

Apple execution is authoritative only when actually observed on Apple CI/runtime.

Never claim unobserved Apple execution as PASS.

### Android

Run applicable Gradle:

- build;
- lint;
- unit tests;
- instrumentation;
- native boundary checks.

### Phase-specific validation

`QUALITY_GATES.md` is authoritative for the complete required gate set.

Tests must prove behavior across system boundaries, not merely duplicate implementation details.

---

## 12. CI remediation behavior

When CI fails inside the authorized scope:

1. verify the exact commit SHA and workflow run;
2. retrieve the current failing logs;
3. avoid acting on stale logs;
4. identify the exact root cause before weakening any invariant;
5. reproduce locally where possible;
6. implement the narrowest valid fix;
7. run all available regression gates;
8. update the current Phase/Step report;
9. commit and push when the current task explicitly authorizes autonomous CI closure;
10. observe the new CI run;
11. repeat until green or a genuine human decision is required.

Do not disable tests, warnings, sanitizers or security controls merely to obtain green CI.

---

## 13. Human decision gates

Stop and ask for a human decision only when one of these occurs:

- frozen architecture conflict;
- conflicting product requirements;
- signing or credential action requiring the user;
- irreversible external action;
- destructive action outside the repository;
- platform limitation requiring a product-level trade-off;
- dependency/license decision with meaningful product impact;
- new Phase/Step authorization.

Routine build, test, diagnosis and repository-local remediation are not human approval gates.

---

## 14. Reporting

At the end of a task or when blocked, report concise evidence:

- what changed;
- exact files changed;
- tests executed;
- exact pass/fail counts;
- CI evidence where relevant;
- known limitations;
- remaining CI-only gates;
- architecture conflicts, if any;
- whether the current Step is ready for acceptance.

Never claim success without evidence.
