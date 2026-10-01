# ADR 0001: Native shells and shared core

Status: Accepted
Date: 2026-10-01

## Context
High-performance photo processing needs explicit ownership of platform textures, memory, lifecycle and hardware inference.

## Decision
Use Kotlin/Compose and Swift/SwiftUI shells with portable C++17 domain/processing code through a versioned C ABI. Platform adapters own hardware APIs. No image processing in UI runtime.

## Alternatives
Flutter/React Native plus native plugins would share UI but retain the native engine/integration workload. Fully separate native engines duplicate numerical behavior. Kotlin Multiplatform may be revisited for non-pixel application logic.

## Consequences
Two UIs require shared fixtures and UX parity tests. Boundary overhead must be minimized with handles. Windows host core tests cannot certify iOS.

## Validation
Applicable evidence is required by [QUALITY_GATES.md](../../QUALITY_GATES.md). Implementation status is separate from acceptance of this decision.
