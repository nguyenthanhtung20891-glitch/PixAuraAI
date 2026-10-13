# Cross-tier replay certification v1 — Phase 4 Step 6

Status: COMPLETE / FULL PASS. Authorized baseline `ce8d513764d6167aa6756b22ef5b0f81698c11c9`.
Certified implementation `417f127d6018448910886d1760e1f8122bcecc11`:
Foundation 38414896874 (7/7), Native application shells 38414897742 (3/3), both
complete SUCCESS. [Evidence and matrix](../reports/phase-4-step-6.md).
Steps 1–5 COMPLETE / FULL PASS; seven frozen steps unchanged. Step 7 NOT STARTED.
No schema, policy, numerical or publication-contract redesign is authorized.

## Comparison authority

Durable document comparison uses exact shared canonical manifest bytes: project/
document/source identity, all operation IDs/types/versions/integer parameters,
ordered complete revision stacks/parents/actors, current revision and redo path.
Successful reopen intentionally acquires a fresh session and advances the storage
envelope epoch under the accepted storage contract. These ownership fields are
not document identity; runtime policy epoch is a separate nonpersisted namespace.
Failed admission must preserve BOTH canonical document and durable envelope
(storage epoch, session, generation). Physical SQLite/WAL byte identity is not
claimed; logical persistent rows and verified immutable assets are authoritative.

Reference comparison uses full-source deterministic CPU working results under
accepted geometry/tone/detail contracts. FIT dimensions never identify a document.
EXACT remains exact-or-resource-rejection; no platform resampling or tier-dependent
parameters. Backend policy never introduces a manual GPU renderer or new kernels.

## Bounded certification corpus

Use accepted capability_fixtures.h SYNTHETIC LOW/MID/HIGH: CPU 2/4/8, RAM 2/4/8 GiB,
budget 128/256/1024 MiB; derived normal T0/96, T1/192, T2/256 MiB. No measured
device or physical certification is implied. Diagnostics record only normalized
facts, tier/cap/edge, availability/validation/pressure, reopen result and hashes.

Required flows: HIGH create/import + crop/rotate/exposure/contrast/saturation/
blur/sharpen commits → MID reopen → LOW reopen; reverse LOW → HIGH. Verify neutral/
cancel zero, changed commit one, multilevel undo/redo, redo-preserving reopen,
retained branches with redo cleared on branch commit. Order pairs crop/rotate,
exposure/contrast and blur/sharpen remain exactly ordered. T0 import/preview/edit/
commit/cancel/undo/redo/reopen uses CPU without network, GPU or AI.

Pressure during gesture preserves accepted values, fences stale candidate,
commits once and permits fresh LOW reopen without document changes. CRITICAL
32 MiB fresh decode can reject; scratch/metadata/control may not fit. Project,
source and durable envelope stay valid/unchanged; recovered fresh policy reopens.
Recovery requires the accepted three distinct stable normal observations and
cannot exceed static baseline. Repeated updates do not churn material epoch.

Backend scenarios include synthetic Vulkan validated scope representation,
available/unvalidated Vulkan, unavailable and available/uncertified Metal.
These test-only trust inputs DO NOT grant real physical/runtime certification;
production adapters retain accepted scope restrictions. CPU semantic results
and document validity stay independent of optional backend selection.

Two independent project/context policies, bounded repeated fresh reopen cycles,
pressure vs commit/cancel/preview, stopped context/stale requests, malformed
profile/version/trust values and hard-ceiling checks are mandatory. Destruction
is stop → join/quiesce → destroy; never race actual freed storage. Cancellation
saves work; existing generation/policy publication fences guarantee correctness.

## Cross-platform evidence

One test-only shared corpus runs in native hosts, Android instrumentation and
Swift macOS/iOS Simulator. Bounded canonical artifacts/goldens verify that both
Android/shared-core and Apple/shared-core consume the SAME persisted model.
Canonical reconstruction/import is explicit through the existing verified asset
and schema-3 repository APIs, not a platform format or untrusted production loader.
Fixture injection/support remains test-only; no production exported control UI.

Separate SYNTHETIC POLICY EVIDENCE, EMULATOR/SIMULATOR INTEGRATION EVIDENCE,
HOSTED RUNTIME EVIDENCE and PHYSICAL DEVICE EVIDENCE. No physical Apple hardware
available; DH-APPLE-METAL-01 stays DEFERRED. Historical Z Fold7 Phase 2 evidence
does not become current physical measurement or expanded trust.

Schema 1/2 historical and explicit 1→2→3 migration gates remain mandatory;
ordinary open never migrates. Schema 3 unchanged; no Schema 4, capability/tier/
pressure/epoch persistence, product UI, dependency, telemetry or Phase 5/AI/model.
Require all retained local/hosted gates and both complete workflows on the SAME
final accepted SHA, including later documentation closure. No skipped/unobserved
required gate is PASS. Stop after Step 6; Step 7 needs separate authorization.
