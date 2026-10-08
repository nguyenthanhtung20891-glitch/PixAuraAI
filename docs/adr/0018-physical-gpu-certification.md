# ADR 0018: Physical-device first GPU pipeline certification

Status: Accepted
Date: 2026-10-08
Scope: Product Owner/Architect authorization for Phase 2 exit closure, not an additional step.

## Context

Accepted CPU/platform preview tests do not implement or prove the original Phase 2 GPU criterion. Hosted Android uses SwiftShader software rendering; preexisting Apple green tests do not prove Metal execution. The Product Owner selected physical-device certification and explicitly authorized only identity and exposure/1/1.

## Decision

Implement bounded synchronous Vulkan compute Android and Metal compute Apple tile adapters, shared CPU validation/gain/parity/fallback, repository-owned shaders, and dedicated physical-device harnesses. Freeze [gpu-tile-v1](../contracts/gpu-tile-v1.md) tolerance before execution: bit-exact identity/alpha; nonzero-exposure RGB abs error <=4e-37+2e-6*abs(CPU). FP32 only, no hidden clamp. CPU remains semantic authority and any backend/parity failure falls back only when explicitly allowed. Certification prohibits fallback and rejects software/emulated execution. Physical Android Vulkan AND physical Apple Metal actual dispatch/readback/parity evidence are mandatory.

## Alternatives

SwiftShader/API availability, shader compilation and hosted green tests cannot provide physical-device evidence. Additional GPU operations or full preview scheduling expand scope without proving this first pipeline. FP64 shaders are not a portable mobile capability; FP16 weakens the working representation. A bounded single-operation tile isolates driver ownership without allocating additional whole rasters.

## Consequences

Hosted compile/regression gates remain mandatory and independently observable. Hardware certification remains pending until controlled-device evidence exists. No persistent cache, resource ceiling change, document/storage mutation or replacement of the Step 11 publication fence. Context/preview GPU integration is not implied by this standalone tile diagnostic. Existing accepted ADRs are not rewritten. Phase 2 remains NOT CLOSED; Phase 3 is not started.

## Validation

Shared native protocol/actual private-allocation faults, CPU reference gains/corpus, stale-generation rejection, Android native/shader builds and SwiftShader rejection instrumentation, Apple shader/backend compile and simulator rejection, existing source/native/sanitizer/shell gates, then both physical-device dispatch/readback/parity harnesses. Report implementation, hosted regression and hardware certification separately in [exit closure](../reports/phase-2-exit-closure.md).
