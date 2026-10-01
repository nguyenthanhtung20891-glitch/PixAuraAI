# ADR 0003: Deterministic image processing

Status: Accepted
Date: 2026-10-01

## Context
Preview and export need consistent color/geometry while supporting low-end CPUs and native GPUs.

## Decision
C++ CPU reference; Metal iOS and measured Vulkan Android adapters; bounded tiles and SDR linear-sRGB working pipeline. JPEG/PNG first. Exact operations/tolerances defined before implementation.

## Alternatives
UI canvas filters cannot guarantee export parity. One portable GPU layer may hide platform capabilities; it remains an evaluation option, not a prerequisite. RAW/HDR expand the initial color/test matrix excessively.

## Consequences
Shaders duplicate numerical work and need golden parity fixtures. CPU fallback always exists. HDR/wide-gamut and codec extension need new validated decisions.

## Validation
Applicable evidence is required by [QUALITY_GATES.md](../../QUALITY_GATES.md). Implementation status is separate from acceptance of this decision.
