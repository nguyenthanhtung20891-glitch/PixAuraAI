# System architecture

## Phase 5 Step 7 current authorization

Step 6 COMPLETE / FULL PASS at `5592cd839c2607379c6cb8c02a4b42a44e80311d`,
accepted by Product Owner. Step 7 AUTHORIZED / IN PROGRESS under approved Option A:
software parity/security evidence is separate from release trust. CPU remains
VALIDATED; Vulkan and Metal remain UNKNOWN. No hosted-evidence promotion policy.
[Certification contract](docs/contracts/inference-provider-certification-v1.md)
and [Step 7 report](docs/reports/phase-5-step-7.md) govern the complete retained
suite and both complete same-final-SHA workflows. Physical evidence stays separate;
DH-APPLE-METAL-01 unchanged and DEFERRED. Production model UNRESOLVED, profile and
Schema 3 unchanged. No weights, external runtime or AI UX. Step 8 and Phase 6
NOT STARTED. Stop after Step 7. Supersedes historical authorization text below.


## Phase 5 Step 6 current authorization

Step 5 COMPLETE / FULL PASS at `59d846967fffef7e735597f6b65301841659336f`:
Foundation 38999701451 (7/7), Native application shells 38999701458 (3/3).
Step 6 AUTHORIZED / IN PROGRESS: deterministic capability-aware inference
admission and PRE-EXECUTION fallback only. Reuse Phase 4 policy authority and
existing provider/package semantics. Accelerated UNKNOWN trust remains ineligible
for automatic selection. Production model selection UNRESOLVED. Require retained
Steps 1-5, malformed/resource/epoch/concurrency tests, sanitizers and both complete
same-final-SHA workflows. [Admission contract](docs/contracts/inference-admission-v1.md),
[Step 6 report](docs/reports/phase-5-step-6.md). Schema 3, provider profiles and
DH-APPLE-METAL-01 unchanged. No AI UX, runtime/model expansion, Step 7 or Phase 6.
Stop after Step 6. This authority supersedes historical authorization text below.

## Phase 5 Step 5 current authorization

Step 4 COMPLETE / FULL PASS at `a3ad7e7a61030992d933b3ca68572f83604dd5a9`:
Foundation 38730904780 (7/7), Native application shells 38730904823 (3/3).
Step 5 AUTHORIZED / IN PROGRESS under approved Option A: bounded synthetic
segmentation evaluation only. [Evaluation contract](docs/contracts/segmentation-evaluation-v1.md),
[Step 5 report](docs/reports/phase-5-step-5.md). Production selection UNRESOLVED;
U2NETP audited/unsupported, no weights/runtime/profile expansion. Require metrics,
independent goldens, corpus, retained Steps 1-4/sanitizers and both complete
same-final-SHA workflows; no unobserved PASS. Schema 3 and DH-APPLE-METAL-01
unchanged. No AI UX, Step 6+ or Phase 6. Stop after Step 5. This supersedes
historical authorization text below without rewriting prior closure.

## Phase 5 Step 4 current authorization

Step 3 COMPLETE / FULL PASS at `bf31eb33d5b5027236d71cf56da4c648d6b56255`:
Foundation 38685139496 (7/7), Native application shells 38685139454 (3/3).
Step 4 AUTHORIZED / IN PROGRESS. Apple-specific Option A explicitly permits
`apple.metal.v1` to consume the authenticated portable CPU ONNX record ONLY
for the frozen reference profile. Mandatory CPU compatibility, separate Metal
trust, explicit selection, no package-schema extension or Core ML execution.
[Apple contract](docs/contracts/apple-inference-provider-v1.md),
[Step 4 report](docs/reports/phase-5-step-4.md). Both complete same-final-SHA
workflows govern closure. Schema 3 and DH-APPLE-METAL-01 unchanged; no physical
Apple claim. Steps 5+ and Phase 6 NOT STARTED. Stop after Step 4.
This supersedes historical Step 3 stop text below without reopening closure.

## Phase 5 Step 3 current authorization

Step 2 COMPLETE / FULL PASS at `58202caf1fd86215abe4650b208b2134cdca9b58`:
Foundation 38605609607 (7/7), Native application shells 38605609516 (3/3).
Step 3 AUTHORIZED / IN PROGRESS. Approved Option A permits `android.vulkan.v1`
to consume the authenticated portable CPU ONNX compatibility record ONLY for the
exact frozen reference profile. No package schema extension or new dependency.
[Android contract](docs/contracts/android-inference-provider-v1.md),
[Step 3 report](docs/reports/phase-5-step-3.md). CPU reference remains authority;
Vulkan availability is separate from validation/certification. Explicit selection;
Step 6 owns admission/fallback. Both complete same-final-SHA workflows required.
Schema 3 and DH-APPLE-METAL-01 unchanged. Steps 4+ and Phase 6 NOT STARTED.
This supersedes historical Step 2 stop text below. Stop after Step 3.

## Phase 5 Step 2 current authorization

Step 1 COMPLETE / FULL PASS at `8d5a30338c97a05e987ecf1324b86a2afb795356`: Foundation
38491704459 (7/7), Native application shells 38491704500 (3/3).
Step 2 AUTHORIZED / IN PROGRESS. Product Owner approves dependency-free
`cpu.reference.v1` with a narrowly bounded genuine ONNX reference profile.
[Inference contract](docs/contracts/inference-provider-v1.md),
[CPU profile](docs/contracts/cpu-reference-provider-v1.md),
[Step 2 report](docs/reports/phase-5-step-2.md). No general ONNX execution or
external runtime. Complete same-final-SHA Foundation/application-shell workflows
are required for closure. Schema 3 and DH-APPLE-METAL-01 unchanged.
Steps 3+ and Phase 6 NOT STARTED; stop after Step 2. This current authority
supersedes historical Step 1-only stop text below, without rewriting closure evidence.

## Phase 5 Step 1 current authority

Only the model-package foundation is authorized at baseline
`098ac7890de5bed61b7c4823fd5ea3b1ef82c904`. [ADR 0022](docs/adr/0022-model-package-ed25519-authority.md)
and the [frozen package contract](docs/contracts/model-package-v1.md) establish
shared pure Ed25519/libsodium 1.0.22 verification, bounded canonical metadata,
no-follow local directory acquisition and owned immutable verified snapshots.
No model/provider execution, inference admission, AI UX or project-schema change.
Phase 4 capability/policy ownership retained; Schema 3 and DH-APPLE-METAL-01 unchanged.
The [eight-step Phase 5 roadmap](docs/plans/phase-5-roadmap.md) is FROZEN;
Step 1 IN PROGRESS through exact-SHA certification, Steps 2+ and Phase 6 NOT STARTED.
This supersedes historical Phase 5 stop points below without reopening Phase 0–4.

## Phase 4 final closure

**Phase 4 — CLOSED / COMPLETE / FULL PASS.** All Steps 1–7 COMPLETE / FULL PASS.
Authorized closure baseline `3d5db65b82e16bb18cca7b906b5108b70fc95956` passed
Foundation 38419049842 (7/7) and Native application shells 38419049786 (3/3).
[Consolidated Step 7 report](docs/reports/phase-4-step-7.md) freezes retained
contracts, limits, evidence classification and exit certification. Effective
closure requires BOTH complete workflows passing on the final commit containing
this status; delivery records its exact SHA/run IDs. Prior-SHA green is historical.
Adapters provide observations; shared capability/admission/dynamic policy and
existing PRV1/document/storage owners remain authoritative. Schema 3 and hard
ceilings unchanged. No unresolved Category A or architecture conflict.
DH-APPLE-METAL-01 remains DEFERRED at its existing hardening/Beta/release/physical
claim boundaries. No physical Apple certification claim. No Step 8 or Phase 4.5.
Phase 5 NOT STARTED; no AI/model/NPU work. STOP after Phase 4 closure.
This current status supersedes historical authorization/stop points below.


## Phase 4 Step 6 authorized certification

Authorized baseline `ce8d513764d6167aa6756b22ef5b0f81698c11c9`; Steps 1–5 FULL PASS.
Step 6 COMPLETE / FULL PASS at `417f127d6018448910886d1760e1f8122bcecc11`:
Foundation 38414896874 (7/7), Native application shells 38414897742 (3/3), both
complete SUCCESS. [Contract](docs/contracts/cross-tier-replay-certification-v1.md).
Observed A: critical rejected reopen advanced durable session/epoch before decode.
Preparation now precedes durable session acquisition, with existing-epoch CAS to
reject intervening changes. No schema/history/policy or architecture redesign.
Test-only owner snapshots avoid duplicate live SQLite copies; release products
exclude certification controls. Step 7 NOT STARTED. DH-APPLE-METAL-01 deferred.
This supersedes historical step authorization below; no Phase 5/AI/model work.

## Phase 4 Step 5 accepted Apple integration

Step 5 COMPLETE / FULL PASS at `944517952252cf1c9e6970bb3dfb2c62fe7f908a`:
Foundation 38355260289 (7/7), Native application shells 38355259361 (3/3), both
complete SUCCESS. [Apple contract](docs/contracts/apple-capability-integration-v1.md)
composes one caller-owned observation service with existing shared policy and
editor lifecycle/display/PRV1 owner. No new policy engine, publication authority,
dependency or architecture conflict. Schema 3/hard ceilings unchanged; Metal
availability is not certification. DH-APPLE-METAL-01 remains DEFERRED.
Frozen roadmap unchanged; Steps 6–7 NOT STARTED, no Phase 5/AI/model work.
This supersedes historical authorization sections below. Stop after Step 5.

## Phase 4 Step 4 current authorization — Android capability integration

Product Owner authorizes Step 4 at `6389750c5ccab3b40d3ee76cb29bbb88b0fb5596`.
Steps 1–3 COMPLETE / FULL PASS; seven-step roadmap remains FROZEN.
[Android contract](docs/contracts/android-capability-integration-v1.md) and
[Step 4 report](docs/reports/phase-4-step-4.md) govern trusted local Android
observations into shared capability/admission/dynamic policy and existing
editor/PRV1 ownership. Step 4 COMPLETE / FULL PASS at `16483ff498f8a2ac67152f6a5b8fdac0e6d822fe`:
Foundation 38281943443 (7/7), Native application shells 38281943346 (3/3),
all SUCCESS on the same implementation SHA.
All retained source/native/sanitizer/Android/Apple gates and both complete
same-final-SHA hosted workflows must pass; unobserved gates are not PASS.
No Android tier table, schema change, UI, telemetry, new dependency or trust
expansion. Schema 3, hard ceilings and DH-APPLE-METAL-01 unchanged.
Steps 5–7 NOT STARTED; no Step 8+ or Phase 5/AI/model authorization.
Stop after Step 4. This supersedes historical Step 3-only authorization below.

## Phase 4 Step 3 dynamic execution authority

Authorized baseline `d118cd26f9384c2f1735a2fbc57682c24a5990e7`; Steps 1/2 FULL PASS.
[Dynamic policy v1](docs/contracts/dynamic-capability-policy-v1.md) composes the
existing raster cancellation mutex, native registry caps, PRV1 generation fence
and active gesture owner. Fixed context-owned policy, immutable static profile,
normalized observations and deterministic recovery add no scheduler, publication
owner, platform business semantics or persistence. Runtime caps only tighten
Step 2/native ceilings. Dynamic enum family preserves Capability v1 bytes.
Step 3 alone is authorized; Steps 4–7 NOT STARTED, no Phase 5. DH-APPLE-METAL-01
unchanged. This supersedes historical Step 2-only authorization below.


## Phase 4 Step 2 shared admission authority

Product Owner freezes the seven-step roadmap and accepts Step 1 at
`82fca1f6fe3a565a6704afe18cb5bc80cb9a9823`. Only Step 2 is authorized.
[Budget v1](docs/contracts/budget-admission-v1.md) adds pure shared preflight over
immutable capability + truthful owner occupancy snapshots. No resource reservation,
second scheduler, platform business rules, context owner, profile persistence or
render/publication integration. Existing native checks stay authoritative and
runtime caps only tighten them. Static preview/CPU choices do not orchestrate
transitions. Steps 3–7 remain NOT STARTED; no Phase 5. DH-APPLE-METAL-01 unchanged.
This supersedes historical roadmap-not-frozen/Step-1-review-pending wording below.


## Phase 4 Step 1 capability foundation

Authorized baseline: `56cd60560b6271b7d27bf5e03fd1d720a3aad2fa`.
The additive [capability v1 contract](docs/contracts/capability-profile-v1.md)
is pure caller-owned normalized metadata: static facts separate from dynamic
observations; GPU availability separate from validation; bounded local diagnostic
bytes; conservative JNI/Swift probe stubs. No admission, device-tier selection,
dynamic downgrade, scheduler, editor ownership or persistence integration lands
in Step 1. Existing hard ceilings and PRV1/document/storage authorities remain.
The [seven-step plan](docs/plans/phase-4-roadmap.md) is PROPOSED, not frozen;
Steps 2+ await Product Owner freeze and Step 1 acceptance. Phase 0–3 stay closed.
DH-APPLE-METAL-01 is unchanged; Phase 5 has NOT started.

## Phase 3 Step 8 certification and closure

**Phase 3 — CLOSED / COMPLETE / FULL PASS.** Steps 1–8 and all frozen exit criteria are certified at implementation SHA `e73605501019afb1aca2a12c521539b68176618e`: Foundation 38144341158 (7/7) and Native application shells 38144341145 (3/3), all SUCCESS. Android teardown and iOS UI input-targeting remediation are validated. This closure supersedes historical not-started wording below. The documentation-only closure commit must also pass both complete exact-SHA workflows before delivery. No Step 9 is created and Phase 4 has NOT started. DH-APPLE-METAL-01 remains DEFERRED and mandatory at its existing later hardening/Beta/release boundaries. No new tool, numerical, AI, GPU, network, telemetry or storage-schema scope was added.

Evidence: [Step 8 report](docs/reports/phase-3-step-8.md) and [certification matrix](docs/contracts/phase-3-certification-v1.md).

Phase 3 Step 7 is COMPLETE / FULL PASS (technical gates), authorized at baseline `116c4efc65f55025f717241a01bc6056ed27d042`.
[Manual editor integration](docs/contracts/manual-editor-integration-v1.md) composes existing gesture/PRV1/document/schema-3 authority; no new numerical semantics or storage schema.
Native, Android controller/Compose, Swift/macOS/iOS app/UI, persistence/reopen, parity and exact-SHA hosted gates are observed PASS. Accepted ADR 0021 changes only Android's immutable asset publication syscall. Step 8 has NOT started; DH-APPLE-METAL-01 unchanged.
Evidence: [Step 7 report](docs/reports/phase-3-step-7.md). Historical statuses below do not override this authorization.


Phase 3 Step 6 is authorized: [gesture lifecycle integration](docs/contracts/gesture-lifecycle-integration-v1.md)
binds the existing manual controller to verified originals and PRV1. Context
serialization owns transient pending state; application services retain approval,
SQLite authority and the existing platform check+display lock. No new engine,
scheduler, durable gesture schema or final editor UI. Steps 7/8 remain NOT STARTED;
historical phase statuses below do not supersede this authorization.

Phase 2 is CLOSED by the explicit [Product Owner/Architect decision](DECISIONS.md#phase-2-closure-decision-2026-10-08). Accepted Steps 1-11 remain FULL PASS; original first GPU pipeline implementation and hosted compile/regression are complete, and physical Android Vulkan certification is accepted PASS 14/14. Apple physical Metal certification is deferred because no real hardware is available, not waived or removed, and remains mandatory under [DH-APPLE-METAL-01](QUALITY_GATES.md#dh-apple-metal-01-deferred-physical-apple-metal-certification) before hardening acceptance, Beta readiness completion, production release or any physical Apple GPU certification claim. This supersedes only the Phase 2 closure timing in historical ADR 0018, not its technical criteria. See the [exit-closure report](docs/reports/phase-2-exit-closure.md). Existing ceilings and CPU authority remain unchanged. Phase 3 has not started; no additional Phase 2 step or work is authorized.

Phase 2 Step 11 is authorized: [interactive preview request generation and publication ownership](docs/contracts/interactive-preview-lifecycle-v1.md) under [ADR 0017](docs/adr/0017-interactive-preview-generation-fence.md). Latest-wins native/platform fences, cooperative supersession and failed-request display preservation keep synchronous execution, existing ceilings and quiescent destruction. All existing local/hosted gates plus churn/races/overflow/platform ownership remain mandatory. [Step 11 report](docs/reports/phase-2-step-11.md); Step 12 is not started.

Phase 2 Step 10 adds [bounded-fit bilinear reference preview and platform-owned copies](docs/contracts/bounded-preview-resampling-v1.md) under [ADR 0016](docs/adr/0016-bounded-bilinear-platform-preview.md). Exact Step 9 bytes remain unchanged; shared C++ filters premultiplied linear RGBA with ordered binary64 arithmetic. Existing resource ceilings remain unchanged, and oversized diagnostic sources intentionally reject before rendering. Bounded one-shot Android Bitmap and Apple CGImage smoke consumers add no persistence or edit authority.


Phase 2 Step 9 adds a bounded CPU 1:1 disposable reference preview under [ADR 0015](docs/adr/0015-bounded-reference-preview.md) and [reference-preview-v1](docs/contracts/reference-preview-v1.md): shared C++ converts evaluated premultiplied linear-sRGB RGBA32F to independently owned RGBA8 straight-alpha sRGB. Existing context ceiling/cap remain unchanged; aggregate rejection preserves all handles and permits explicit release/retry. No resize, GPU, persistent preview cache or production display pipeline.

For JPEG/PNG SDR, accepted [ADR 0011](docs/adr/0011-shared-bounded-sdr-decode.md) supersedes the earlier platform codec ownership below: shared C++ owns admission, bounds, decode semantics and transient image ownership using pinned portable decoders. Platforms retain OS/file integration. This exception does not extend to other formats, HDR, animation or encoding/export.

## Stack and rationale
Native Kotlin/Jetpack Compose Android shell and Swift/SwiftUI iOS shell, shared C++17 processing/domain primitives behind a versioned C ABI. Two native UIs cost more than one cross-platform shell but give direct ownership of GPU surfaces, memory pressure, background tasks, accessibility and inference providers. Flutter and React Native are viable shells but are not selected: the core must already be native and this product prioritizes platform profiling and integration over shared presentation.

Android uses ViewModel + immutable StateFlow and unidirectional events. iOS uses MainActor observable state with async domain services. No domain command executes in a view. Shared serialized contracts and cross-platform fixtures prevent the two UIs from diverging. See [Compose architecture](https://developer.android.com/develop/ui/compose/architecture) and [SwiftUI UIKit integration](https://developer.apple.com/documentation/swiftui/uikit-integration).

## Layers
Presentation -> application services (project/session/approval/jobs) -> domain contracts (operations, revisions, plans) -> native engine and local repositories. Platform adapters implement picker, codec, GPU surface, secure storage, billing, capability and inference interfaces. JNI is Android boundary; Swift imports a C module, with Objective-C++ only where Apple APIs need it. C++ owns numerical semantics and engine scheduling; Kotlin/Swift own lifecycle and permissions.

No image buffers cross bridges as JSON or per-frame byte arrays. Exchange opaque handles, metadata, bounded commands and completion events; native texture surfaces display previews. Handles have explicit release, generation IDs and cancellation lifetime rules. C ABI uses fixed-width types, status codes and caller-owned bounded output; no C++ exceptions or STL containers cross it. Validate every boundary; reject unknown ABI versions. Phase 0 exposes only version negotiation, not a renderer.

## Processing and persistence

Phase 2 Step 1 freezes the [image document contract](docs/contracts/image-document-v1.md) under ADR [0008](docs/adr/0008-document-stacks-and-manifest.md): ordered operation stacks per immutable revision, retained single-parent revision branches, explicit redo navigation, bounded canonical JSON checkpoints and SQLite live authority. Step 2 implements bounded native schema-1 parsing/canonical serialization, immutable metadata/history transitions and a separate versioned document C API. ADR [0009](docs/adr/0009-explicit-document-context.md) specifies explicit caller-owned registry contexts and context-scoped non-pointer handles. Snapshots retain a nonserialized session/generation envelope; application services own approval, persistence and active publication. ABI 1 remains unchanged, including zero feature bits. No SQLite adapter, source importer, decoder or renderer is implemented by Step 2.
CPU reference kernels in C++; Metal compute on iOS; Vulkan compute Android where validated, with OpenGL ES 3.1 backend considered only if device coverage requires it, and tiled CPU fallback always present. Native codecs decode into bounded tiles. Linear-light float working space; color management and export conversion defined in PHOTO_ENGINE.md. No dependency on a UI framework renderer for editing pixels.

SQLite per-project metadata and append-only operation/revision records; originals are content-addressed files in app-private storage. Authorized Step 3 implements direct shared C++ SQLite services and small native filesystem adapters under [ADR 0010](docs/adr/0010-durable-native-project-storage.md), avoiding duplicated platform database semantics. Transactional approval commits operation batch, revision/current/redo/session/generation and durable epoch. Streaming SHA-256 ingestion flushes and publishes without overwrite before any reference. Recovery verifies committed references and ignores untrusted staging/checkpoints; it retains orphan assets rather than introducing garbage collection here. Never delete an asset referenced by retained history. Storage version/migration admission and backup/restore exclusion rules remain explicit. Canonical JSON is derived interchange/checkpoint metadata, not a second authority. Minimal JNI/Swift checks prove SQLite schema creation/reopen through the checked C API; no import UI exists.

## Build and dependencies
Windows: MSVC host reference tests, Android Gradle/NDK when available. macOS: Swift Package tests, Xcode iOS builds, Metal compilation. CMake builds portable core/Android library; Swift Package wraps identical source for Apple. Android baseline API 26, iOS 16 provisionally; hardware support is capability-based. Reassess minimum OS before beta using real-device evidence. Pin shipping toolchains, NDK, SDK and libraries in Phase 1; the Phase 0 skeleton has no third-party runtime dependencies. Node built-in tests are engineering checks only, never app runtime.

Dependency review requires exact version, SPDX license, redistribution/model license, source provenance, vulnerabilities, binary-size effect and offline behavior. No unlicensed weights or copyleft dependency silently incorporated. Lockfiles/checksums in repository; upgrades are isolated with regression evidence. Releases include SBOM and reproducible inputs; signing is a human gate. Foundation CI uses pinned action commit SHAs; runner images remain hosted mutable inputs and are recorded by logs.

Observed host tools and the optional repository-local C/C++ test compiler fallback are recorded in [docs/TOOLCHAIN.md](docs/TOOLCHAIN.md). That fallback does not change the shipping mobile stack.

## Phase 1 shell implementation

Android's app module isolates Compose presentation, an immutable destination/mode reducer, ViewModel state and the existing bridge source. CMake packages pixaura_core and pixaura_bridge for ARM64/ARMv7/x86_64. iOS's real Xcode application uses SwiftUI NavigationStack, MainActor state and a local Swift package dependency on the same core. Probe loading/calling runs on background executors; unavailable status leaves placeholder navigation reachable. No image decode/render/inference/persistence command exists in a view.

ADR [0007](docs/adr/0007-shell-navigation-and-build-inputs.md) defines minimal coordinators and pinned inputs; [dependency review](docs/DEPENDENCIES.md) records licenses/security/size. Shell route/mode restoration is presentation state, not project history. SceneStorage holds no content; future project storage adapters must set Apple backup exclusion/data protection at creation. Android backup/transfer domains are excluded now. Import/Compare/Export extend future typed paths, with no editing feature implemented in Phase 1.

## Security and future expansion
Untrusted decode and model parsing have allocation bounds, cancellation and fuzz tests. Planner receives tool descriptions, never raw filesystem authority. Executor verifies budgets and base revision. Cloud support requires a new opt-in data-flow ADR. Future advanced models fit the runtime/plan interfaces without bypassing approval or history.
