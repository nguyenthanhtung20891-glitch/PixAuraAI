# ADR 0007: Minimal native shell coordinators and build inputs

Status: Accepted
Date: 2026-10-02

## Context
Phase 1 must prove two real native applications call the existing C ABI, without introducing an image engine or final visual identity. The four destinations and three mode entries require presentation state, not a persisted project domain.

## Decision
Keep ADRs 0001-0006 unchanged. Android uses a single application module, Compose, ViewModel/SavedStateHandle and immutable StateFlow. A typed shell reducer owns destination/mode, including Home/back behavior. iOS uses SwiftUI NavigationStack, a MainActor observable coordinator and SceneStorage of destination/mode. Navigation preserves mode; this placeholder session is not an operation graph or project revision. Both invoke the same ABI 1 probe on background executors.

Keep route mutation inside coordinators. The current graph is Home -> Editor/Projects/Settings -> Home. Future Import -> Editor -> Compare -> Export routes will extend coordinator state with a typed path and real project identity after the engine contracts exist. No feature route, media picker or processing is implemented now.

Pin Gradle 8.13, AGP 8.13.2, Kotlin/Compose compiler 2.2.21, Compose BOM 2025.12.00, SDK 36, build tools 35.0.0, NDK 28.2.13676358, CMake 3.22.1 and JDK 21.0.10 (Java bytecode 17). Retain the Phase 0 Xcode 26.6/macOS 26 CI baseline and iOS 16 deployment target. Commit dependency locks/checksums. Generate the committed Xcode project using a deterministic Node script instead of adding a generator dependency.

## Alternatives
AndroidX Navigation is useful when deep links and nested graphs arrive; the current four shallow destinations do not need it. XcodeGen/Tuist would introduce extra build tooling for a tiny project. Cross-platform UI or duplicated engines contradict ADR 0001.

## Consequences
Two small coordinators intentionally duplicate presentation semantics and have parity tests. They cannot claim durable project restoration; that belongs to Phase 2. Growing flow complexity requires revisiting typed paths rather than adding commands in views. The Xcode generator and checked project must remain identical. Dependency upgrades are reviewed separately; lint upgrade notices are disabled, while correctness/accessibility checks remain strict. Hosted SDK/emulator image revisions and runner images are mutable; CI must log execution evidence.

## Validation
See [Phase 1 report](../reports/phase-1.md) and [dependency review](../DEPENDENCIES.md). JVM/Swift reducer tests check mode preservation and back behavior; emulator/simulator tests load the real core. Debug and unsigned release builds, strict lint, C ABI tests and sanitizers remain mandatory. Windows cannot validate Xcode.
