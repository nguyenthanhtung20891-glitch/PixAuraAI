# System architecture

## Stack and rationale
Native Kotlin/Jetpack Compose Android shell and Swift/SwiftUI iOS shell, shared C++17 processing/domain primitives behind a versioned C ABI. Two native UIs cost more than one cross-platform shell but give direct ownership of GPU surfaces, memory pressure, background tasks, accessibility and inference providers. Flutter and React Native are viable shells but are not selected: the core must already be native and this product prioritizes platform profiling and integration over shared presentation.

Android uses ViewModel + immutable StateFlow and unidirectional events. iOS uses MainActor observable state with async domain services. No domain command executes in a view. Shared serialized contracts and cross-platform fixtures prevent the two UIs from diverging. See [Compose architecture](https://developer.android.com/develop/ui/compose/architecture) and [SwiftUI UIKit integration](https://developer.apple.com/documentation/swiftui/uikit-integration).

## Layers
Presentation -> application services (project/session/approval/jobs) -> domain contracts (operations, revisions, plans) -> native engine and local repositories. Platform adapters implement picker, codec, GPU surface, secure storage, billing, capability and inference interfaces. JNI is Android boundary; Swift imports a C module, with Objective-C++ only where Apple APIs need it. C++ owns numerical semantics and engine scheduling; Kotlin/Swift own lifecycle and permissions.

No image buffers cross bridges as JSON or per-frame byte arrays. Exchange opaque handles, metadata, bounded commands and completion events; native texture surfaces display previews. Handles have explicit release, generation IDs and cancellation lifetime rules. C ABI uses fixed-width types, status codes and caller-owned bounded output; no C++ exceptions or STL containers cross it. Validate every boundary; reject unknown ABI versions. Phase 0 exposes only version negotiation, not a renderer.

## Processing and persistence

Phase 2 Step 1 freezes the [image document contract](docs/contracts/image-document-v1.md) under ADR [0008](docs/adr/0008-document-stacks-and-manifest.md): ordered operation stacks per immutable revision, retained single-parent revision branches, explicit redo navigation, bounded canonical JSON checkpoints and SQLite live authority. Step 2 implements bounded native schema-1 parsing/canonical serialization, immutable metadata/history transitions and a separate versioned document C API. ADR [0009](docs/adr/0009-explicit-document-context.md) specifies explicit caller-owned registry contexts and context-scoped non-pointer handles. Snapshots retain a nonserialized session/generation envelope; application services own approval, persistence and active publication. ABI 1 remains unchanged, including zero feature bits. No SQLite adapter, source importer, decoder or renderer is implemented by Step 2.
CPU reference kernels in C++; Metal compute on iOS; Vulkan compute Android where validated, with OpenGL ES 3.1 backend considered only if device coverage requires it, and tiled CPU fallback always present. Native codecs decode into bounded tiles. Linear-light float working space; color management and export conversion defined in PHOTO_ENGINE.md. No dependency on a UI framework renderer for editing pixels.

SQLite per-app catalog/project metadata, append-only operations and revision records; large originals/masks/checkpoints as content-addressed files in app-private storage. Platform SQLite adapters initially; Room/other wrappers require dependency review. Transactional approval commits operation batch and revision pointer. Write asset to temporary file, flush and rename before transaction refers to it; recovery removes unreferenced assets and incomplete temp files. Never delete an asset referenced by retained history. Explicit migrations and backup/restore exclusion rules.

## Build and dependencies
Windows: MSVC host reference tests, Android Gradle/NDK when available. macOS: Swift Package tests, Xcode iOS builds, Metal compilation. CMake builds portable core/Android library; Swift Package wraps identical source for Apple. Android baseline API 26, iOS 16 provisionally; hardware support is capability-based. Reassess minimum OS before beta using real-device evidence. Pin shipping toolchains, NDK, SDK and libraries in Phase 1; the Phase 0 skeleton has no third-party runtime dependencies. Node built-in tests are engineering checks only, never app runtime.

Dependency review requires exact version, SPDX license, redistribution/model license, source provenance, vulnerabilities, binary-size effect and offline behavior. No unlicensed weights or copyleft dependency silently incorporated. Lockfiles/checksums in repository; upgrades are isolated with regression evidence. Releases include SBOM and reproducible inputs; signing is a human gate. Foundation CI uses pinned action commit SHAs; runner images remain hosted mutable inputs and are recorded by logs.

Observed host tools and the optional repository-local C/C++ test compiler fallback are recorded in [docs/TOOLCHAIN.md](docs/TOOLCHAIN.md). That fallback does not change the shipping mobile stack.

## Phase 1 shell implementation

Android's app module isolates Compose presentation, an immutable destination/mode reducer, ViewModel state and the existing bridge source. CMake packages pixaura_core and pixaura_bridge for ARM64/ARMv7/x86_64. iOS's real Xcode application uses SwiftUI NavigationStack, MainActor state and a local Swift package dependency on the same core. Probe loading/calling runs on background executors; unavailable status leaves placeholder navigation reachable. No image decode/render/inference/persistence command exists in a view.

ADR [0007](docs/adr/0007-shell-navigation-and-build-inputs.md) defines minimal coordinators and pinned inputs; [dependency review](docs/DEPENDENCIES.md) records licenses/security/size. Shell route/mode restoration is presentation state, not project history. SceneStorage holds no content; future project storage adapters must set Apple backup exclusion/data protection at creation. Android backup/transfer domains are excluded now. Import/Compare/Export extend future typed paths, with no editing feature implemented in Phase 1.

## Security and future expansion
Untrusted decode and model parsing have allocation bounds, cancellation and fuzz tests. Planner receives tool descriptions, never raw filesystem authority. Executor verifies budgets and base revision. Cloud support requires a new opt-in data-flow ADR. Future advanced models fit the runtime/plan interfaces without bypassing approval or history.
