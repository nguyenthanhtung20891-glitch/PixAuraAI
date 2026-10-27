# Phase 7 final closure — AI-assisted MVP

Current Step 8: AUTHORIZED / IN PROGRESS. Final Phase 7 closure is not yet
certified. Phase 8 NOT STARTED. This report is audit evidence, not runtime authority.
Starting clean main == HEAD == origin/main after fetch:
`54dcc9292f43ab49335a99df3a4145d0279c7980`.
Historical ignored research and probe artifacts are preserved. No research reopened.
The [audit changed-file inventory](evidence/phase-7-step-8-audit-files.json) records
the exact documentation/audit-test scope; production source/build inputs are unchanged.

## Certified Steps 1–7

All fourteen historical workflow runs were retrieved again from GitHub. Every run
is completed/SUCCESS, exact closure SHA, attempt 1, with every required job SUCCESS.
The [job-level history](evidence/phase-7-certified-steps.json) records 70 job results.
Earlier reports/contracts describe their original freeze time; their prepared
pending/future-step language does not override these observed closure receipts.

| Step | Certified final closure SHA | Foundation (7/7) | Native shells (3/3) |
| --- | --- | --- | --- |
| 1 Interaction contract | `3640d94e473bddd792a12541a45b6bb36ff8cd50` | 39768943832 | 39768943890 |
| 2 Provider/envelope | `0d73ffc3ec1fa1ac5e1032a4f8f47a2c69e1b990` | 39956128221 | 39956128320 |
| 3 Manual + Assist | `43b2e36f294e4f0d518742b3923027551ec34c1a` | 40056249044 | 40056248966 |
| 4 Auto proposal | `8c185389fddad8b59df202b2f7776e10a46271b0` | 40088844044 | 40088844028 |
| 5 Review/HUMAN Apply | `eef23391fb18c0c9b30cefc632c7a86126e09a60` | 40152878375 | 40152878474 |
| 6 Lifecycle integration | `f2dd3ceac7f9f18bf9033b9e398ac703d491e3c5` | 40198995481 | 40198995512 |
| 7 MVP admission | `54dcc9292f43ab49335a99df3a4145d0279c7980` | 40462182955 | 40462183031 |

Step 7's observed receipt activates **reference.assistance.v1 — ADMITTED**, version 1.
Planner **reference.deterministic.v1**, version 1, remains separate.
PRODUCTION LEARNED MODEL: **NONE REQUIRED / NONE ADMITTED FOR PHASE 7 MVP**.
Future learned provider: DEFERRED / REPLACEABLE. The admission manifest now records
the observed Step 7 closure; it does not grant new runtime capabilities.

## Scope and authority-chain audit

Explicit bounded request -> provider -> UNTRUSTED Envelope -> existing native
Structured Intent/planner or canonical Plan/Registry validation -> exact review ->
explicit HUMAN Apply -> native exact-binding/single-use approval -> existing private
candidate/execution -> deterministic engine -> at most one ordinary revision ->
shared history/undo/redo. This is the AI path; ordinary explicit Manual editing
continues independently through its existing native manual authority.

Inspected production seams: `assistance.hpp`, `assistance_codec.inc`,
`assist_experience_boundary.hpp`, `auto_proposal_boundary.hpp`,
`agent_execution_boundary.hpp`, `editor_api.cpp`, native document/storage;
Android `bridge/.../ManualEditor.kt`, app `ManualEditorModel.kt`, `AssistPanel.kt`,
`AutoProposalPanel.kt`; Swift `Sources/ManualEditor.swift`, app
`ManualEditorModel.swift`, `ManualEditorScreen.swift`. C/JNI/Swift adapters contain
no second tool math, planner or history engine. Provider receives data only.
`execution_plan` repeats canonical and Registry checks; execution compares exact
approval bytes and live source serial, consumes approval before raster work,
rechecks cancellation/source and repository CAS before `Service::accept` publication.
The final cancellation/publication fence decides the winner; UI disappearance
cannot assert cancellation or undo an authoritative publication.

The native host boundary cannot authenticate a physical human tap or sandbox
malicious trusted native host code; this existing threat-model limitation is not
changed or concealed. Application Apply handlers are explicit human controls;
provider completion, appearance and observation callbacks do not invoke approval.

## Architecture invariant matrix

PASS here means inspected existing implementation plus retained regression
coverage; exact Step 8 execution evidence is recorded separately below before closure.

| Invariant | Audit | Evidence / retained executable coverage |
| --- | --- | --- |
| MANUAL independent, original immutable, non-destructive | PASS | native manual/document/storage tests; source SHA checks after Auto and branch |
| One document/history, ordinary undo/redo/branch | PASS | `agent_execution_test.cpp` success/undo/redo/branch cases; Android/Swift/UI history tests |
| Assist non-mutating; Use suggestion navigation-only | PASS | Assist boundary returns presentation only; app selects existing tool without update/Apply; `assist_experience_test.cpp` |
| Auto proposal alone cannot execute | PASS | Auto boundary has no approval/execution call; `auto_proposal_test.cpp`, status NOT_APPROVED tests |
| Explicit HUMAN approval, exact Plan binding | PASS | app Apply controls; Kotlin/Swift AutoReview byte revalidation; `execution_approval`, `execution_plan` |
| Single-use approval, no replay/transfer | PASS | nonreused context/lease serial; state 2 reserved before execution; `agent_execution_test.cpp` double/replay/wrong-owner cases |
| No stale publication, ABA protection | PASS | live serial, repository epoch/CAS and final fences; native source/edit/undo/redo/external-session races |
| One revision or none, no partial history | PASS | native transaction publication, durable-failure injection, atomic multi-action tests |
| No hidden retry/replan/reapproval | PASS | no fallback loop in provider/Apply path; terminal review and consumed approval tests |
| Provider untrusted, bounded, replaceable, non-authoritative | PASS | narrow Provider interface; trusted selected identity; validator/Registry; 117 golden envelopes and negative corpus |
| Platform presentation is not editing/publication authority | PASS | JNI/Swift delegates; authoritative receipt checks and history refresh after Apply outcome |
| Lifecycle stop -> join ALL calls -> destroy | PASS | wrapper call accounting includes cancellation/completion; deterministic native and platform teardown tests |
| No persistent AI authority; Schema 3 unchanged | PASS | no AI fields/handles in document persistence; contract/source freeze and persistence tests |
| Local/offline, no required cloud/network | PASS | provider has no IO capability; no Android INTERNET permission or assistance network client; app source review |
| Request/proposal/action bounds unchanged | PASS | Structured/Advice 2048; Plan 8192 and 1–8 actions; Envelope 12288; IDs 64; malformed/resource tests |
| Frozen resource authorities unchanged | PASS | Phase 4/5 contracts and additive intelligence resource contract exact hash snapshot |
| Free-form Intent remains uninterpreted | PASS | machine-goal table only; app explicitly disclaims image analysis; no chat/NLP path |
| Release/test/research isolation | PASS | native objects and final APK/.app scans, with injected forbidden-content negative fixtures; fresh evidence required below |
| CPU/Vulkan/Metal trust unchanged | PASS | CPU VALIDATED; Vulkan/Metal UNKNOWN; no trust admission changed |
| Physical Metal gate | DEFERRED | DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED; hosted Apple is software only |
| Learned provider/arbitrary-photo diagnosis | DEFERRED | no learned model required/admitted; no claim of production absolute-image intelligence |
| Phase 8 boundary | PASS | Phase 8 NOT STARTED; no feature work authorized |

## Threat / security closure matrix

No unresolved A-class blocker found in the inspected Phase 7 paths. The matrix
does not assert a formal proof against arbitrary malicious native host code.

| Threat | Result | Protection / retained test evidence |
| --- | --- | --- |
| A authority bypass | PASS | provider has no issuer/editor capability; native rejects execution without approval |
| B malformed/forged proposal | PASS | exact parser/version/provider/Registry/bounds; forged ID conveys no authority; assistance negatives |
| C stale proposal | PASS | source/context/serial/epoch checks before presentation, Apply and publication; ABA tests |
| D approval replay/confusion | PASS | exact bytes, owner and source; single-use state; wrong-plan/order/value/document/replay tests |
| E mode leakage | PASS | all six transitions abandon old ephemeral interaction, no commit; platform mode tests |
| F platform semantic mismatch | PASS | same native request/validation/execution, eleven-goal canonical repeat tests on JNI/Swift |
| G destruction race | PASS | provider synchronous; wrapper stop/join includes all in-flight calls; deterministic teardown rendezvous |
| H cancellation/publication race | PASS | native publication fence, CAS, no automatic undo; both deterministic winner orderings |
| I resource/output exhaustion | PASS | input and live-owner bounds, allocation-failure tests, BUSY instead of queue/retry |
| J test-hook/research-model leakage | PASS | actual release binary/package scans plus negative fixtures; no experimental provider registration |
| K privacy/network exposure | PASS | no production network permission/client/provider IO, telemetry or credential authority |

## Concurrency, lifetime, document and platform audit

ReferenceProvider is synchronous, one active call per existing small owner via
try-lock; no provider worker, queue, cache or automatic retry. Epoch fencing
suppresses cancellation/stop results; the application's request epoch is presentation
ownership only. Native source/context checks remain freshness authority.
Android rejects duplicate Apply using exact review identity and admission atomics;
Swift uses its exact owner/review and locked one-shot admission. Completion is
based on validated native receipt, not provider success or execution-start callback.
Both refresh ordinary history after Apply outcomes, including transport failure.
Queued validation/completion and pending cancellations retain admission until drained;
an old cancel cannot target a newer execution. No UI callback revives discarded review.

Activity/view/model recreation does not persist pending proposal, review, approval,
candidate or provider handle. Process restart restores ordinary durable history only.
Native publication may win even during UI departure; that ordinary revision remains
visible when authoritative document/history is refreshed. Cancellation winning before
publication leaves none. Manual edit/undo/redo/source change invalidate old AI work.
Stop -> join ALL calls -> destroy remains ADR 0009, not a UI lifecycle authority.

Retained deterministic seams cover Assist/Auto request against edit, undo/redo,
mode switch, stop and teardown; review/Apply against cancel, duplicate Apply, manual
source change, owner destruction, repository epoch changes and publication. App
tests use semantic accessibility identifiers rather than coordinate-only actions.
The Step 7 final software results were Windows 52/52, Linux/Apple native 53/53,
ASan/UBSan 55/55, Android 47/47, Swift macOS/Simulator 44/44 each, iOS app 9/9,
UI 10/10, source 177/177. These historical counts do not certify new Step 8 SHAs.

## Release, privacy and resource closure

Both retained workflows build real unsigned Android release APK and Apple release
APP and inspect their final contents, native libraries, DEX/strings and model/research
assets. Native production objects are scanned separately. Android/Apple negative
fixtures inject forbidden markers/assets and must fail. Scanning is a bounded
defence, not a claim that arbitrary obfuscated malicious code can always be detected.
Step 7 package evidence: APK 12 native libraries / 2 DEX / 31,870,648 uncompressed
bytes; APP 8 files / 2,940,223 bytes / no model assets. Fresh Step 8 scans are required.

No Phase 7 assistance internet, cloud, upload, remote approval, key, telemetry,
dynamic download, shell/environment or arbitrary filesystem access exists. Native
storage/system photo picking remain the ordinary local document path, not provider
capabilities. No raw user image/request upload path was found.

The deterministic owner is small and lazy within the existing editor service. No
heavyweight intelligence owner/model mapping/session is constructed for it. Existing
bounded resource/handle/concurrency admission remains sufficient; Manual requires no
provider. Phase 4/5 256 MiB authorities and segmentation inference are unchanged;
Intelligence Runtime Resource Admission v1 is additive infrastructure for future
learned providers. RSS is diagnostic only; no guessed production ceiling is introduced.
Step 7 measured allocation/object/timing evidence remains precisely scoped in its
report; it is neither total-process memory nor physical mobile thermal certification.

## Frozen-contract comparison and research disposition

The [closure snapshot](phase-7-closure-v1.json) pins all tracked native/runtime,
public include, Swift/Android/iOS application/adapter sources, platform build inputs,
all contracts and ADRs at the accepted starting SHA. Closure tests reject changed,
added or removed files in these inventories. The eight Phase 6 contract hashes are
also compared to their original Phase 6 closure manifest. No frozen contract is
changed from the eight pinned Phase 7 Step 1–6/Round 1 contract origins either.
The exact origin SHAs and normalized hashes are recorded in the snapshot.
No frozen contract is
rewritten to update a historical status; current authority is layered above it.
Step 8 makes no production behavior, dependency, ABI, Schema 3, history, Manual,
approval/candidate, resource or trust change. The test additions are audit-only.

EmbeddingGemma remains DISQUALIFIED FOR CURRENT MVP; its evidence is preserved.
NIMA/FFCC/MobileIQA NOT SELECTED / NOT INTEGRATED. Absolute-image deterministic
baseline NON-PRODUCTION. Arbitrary-photo learned intelligence is future replaceable
provider research; no new search, conversion, weights or mobile runtime integration.

## Limitations / B and C disposition

No new required B-class defect identified. Known future research is not a silently
waived production requirement. B: any future learned provider still needs independent
provenance, quality, resource and physical-device admission; none is required here.
C: attribution/performance refinement is optional; current footprint and cumulative
allocation diagnostics do not establish exact linked-provider or live-memory deltas.
Physical Metal is a KNOWN DEFERRED ITEM, not satisfied by hosted Apple results.
Local WSL G: v9fs storage permission/rename limitations are retained historical
environment evidence; full native Linux CI, not that local mount, certifies storage.

## Step 8 execution and certification

Local Windows canonical Zig/CMake gate: 53/53 passed (189.91 seconds), including
deterministic provider, native races, storage and release-object isolation.
Focused admission/closure Node checks: 6/6 passed. WSL Python negative scanners:
Android 1/1 and Apple 1/1; retained offline qualification suites 4/4, 6/6, 8/8 passed.
The first full source command on Windows failed Linux fake-SDK/emulator shell
assumptions (EPERM cleanup/symlink and missing fake counter); this is not a full PASS.
The source job is Linux-hosted in CI; its unchanged complete command passed in
WSL 180/180 (185.01 seconds), with repository-contained temporary storage. No platform
test is disabled. The original Windows shell-test failure is retained as an
environment limitation, not relabelled PASS.
Full platform/sanitizer/package evidence requires the new exact-SHA CI runs below;
no local physical Android/Apple, GPU, battery or thermal evidence is claimed.

Audit tests and full retained source checks are executed locally before commit.
Complete Foundation (7 jobs) and Native application shells (3 jobs) must then pass
the exact audit SHA, attempt 1, every job individually inspected. A separate
documentation-only closure records those observed results and must independently
pass BOTH workflows on its own exact SHA. No rerun-until-green, old-SHA substitution
or unobserved PASS. The external observed final engineering receipt activates
Phase 7 CLOSED / COMPLETE / FULL PASS and Steps 1–8 COMPLETE / FULL PASS only
after that final closure certification. Until then Phase 7 closure remains pending.
Phase 8 NOT STARTED. STOP after Step 8.
