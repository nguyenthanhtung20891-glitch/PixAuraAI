# Phase 3 Step 3: Tone & Color Tools

Status: IN PROGRESS; not yet FULL PASS. Authorized baseline
`6235737ceb567277584ae287d44ff07a5bf67149`.

## Contracts and architecture

The [numerical contract](../contracts/manual-tone-color-v1.md) freezes six new
/1/1 recipes; exposure /1/1 is unchanged. The compiled registry has nine entries
(crop/rotate and seven tone/color tools), under unchanged 16-descriptor,
8-parameter, 32-KiB registry and all existing history/raster/context ceilings.
No GPU kernels or numerical GPU rules change. No final editor UI.

The storage blocker was resolved by explicit Architect approval of independent
schema 2 and explicit transactional 1->2 migration, Category A.
[ADR 0019](../adr/0019-explicit-storage-schema-evolution.md) records the narrowly
superseding migration protocol; schema 1 SQL/fingerprint and old operation
meanings remain immutable. Ordinary open never migrates.

## Validation evidence

Initial Linux schema foundation: 21/21 native regressions PASS. The independent
Decimal(70) fixture/constant generator ran before CPU implementation; new-tool
contract test failed on unsupported parameters before implementation. After
implementation, the corpus was expanded to 940 reference pixel fixtures,
including signed-zero/minimum-subnormal cases. The established CPU reference
bound, exact neutral/alpha preservation, source invariance and boundary/version
rejection passed. Windows fallback and Linux native suites passed 22/22 each.
Windows/Linux portable source checks passed 59/59; the full Linux source suite
passed 99/99. Independent fixture/matrix reproduction and workflow lint passed.
ASan/UBSan passed 24/24, including active instrumentation probes; final
documentation checks passed 8/8. Platform runtime and exact-SHA hosted closure
remain pending; no pending check is PASS.

## Integration and migration evidence

The shared C family admits exactly the nine registered tools; existing geometry
begin remains crop/rotate only. All seven tone tools are exercised through the
C/Android boundary corpus. Six new tools join real decoded original geometry
stacks, with 1,001 updates -> one detached immutable revision proposal. Returning
to neutral, zero updates and cancellation do not grow history; invalid update,
stale session, prior-sequence rejection and failed preview preserve the required
state. Existing source/revision/tool-switch and combined 64-handle admission
tests remain. Registry limits stay 16 tools/8 parameters/32 KiB, with nine entries.

Each committed new tone operation is approved by the test application owner,
stored in schema 2 and reopened through the existing SQLite authority. Canonical
documents, full ordered operation IDs/types/versions/parameters and original
digest survive restart. Reopened projections equal their prior projections;
their evaluated working pixels match byte-for-byte on the same platform. No
source pixels, source bytes or retained historical operation is modified.

Migration covers empty v1 and populated exposure/rotation/crop fixtures,
ordered retained history, branches and redo. Independent v1/v2 exact SQL
catalogs are compared. The unchanged schema-1 source SHA-256 is
`b3a43dae7e25d8efea564a8f6eeebfab29ea45cfe355d4ba99879e3e1ed84e86`.
v1 new-tool persistence rejects until explicit migration; ordinary open never
upgrades. Repeated 1->2 on v2 fails non-destructively, and 2->2 verifies a no-op.
Wrong application/version/fingerprint and invalid/mixed/NULL/float/new-version
SQL payloads reject. Four pre-commit IO fault points, allocation failure and
simulated SQLite NOMEM/IOERR/FULL/INTERRUPT returns roll back SQL/version and
canonical state. Five migration child-process crash points prove prior-v1
recovery before commit and complete-v2 recovery after commit, alongside nine
existing asset/catalog/checkpoint crash children. Logical rollback is guaranteed;
physical database/WAL byte identity is not claimed.

Windows manual allocation sweeps recovered 1,463 failures; Linux/sanitizer sweeps
recovered 1,768, covering exposure and new brightness update/commit/serialization
and larger registry output. Hosted MSVC remediation retained warnings-as-errors, removed two shadowed test
locals and extended only the new three-operation fault-search phases from 512
to 4,096 attempts; every failure position and retry assertion remains exercised.
Production ceilings are unchanged. Renderer fault sweeps additionally exercise ordered
brightness/contrast/temperature evaluation with and without cancellation.

Android local debug/release assembly, lint and JVM tests passed (2/2 debug,
2/2 release); instrumentation APK compilation passed. No local ADB device was
connected, so runtime instrumentation awaits hosted execution. Apple compilation,
Swift/macOS/iOS simulator execution and both exact-SHA workflows remain pending.
No physical GPU certification is performed or claimed.

## Scope

A: schema evolution/migration, new numerical contracts and shared CPU/manual/
preview/persistence/platform/reference regression integration.
B: final editing flows/UI belong to frozen Steps 6/7, full phase certification
to Step 8, and detail/filter scope to Steps 4/5; none is started here.
C: future acceleration and unapproved tools remain backlog. Resize remains
PROPOSED/unimplemented; whites/blacks/tint/vibrance remain backlog. No AI,
Phase 4, new Phase 2 work or roadmap step. DH-APPLE-METAL-01 is unchanged and
unexecuted. Phase 3 Step 4 has NOT started.

## Files changed

- `.github/workflows/foundation.yml`
- `.github/workflows/native-shells.yml`
- `CMakeLists.txt`
- `DECISIONS.md`
- `docs/adr/0019-explicit-storage-schema-evolution.md`
- `docs/contracts/manual-tone-color-v1.md`
- `docs/contracts/manual-tools-v1.md`
- `docs/contracts/project-storage-v1.md`
- `docs/contracts/project-storage-v2.md`
- `docs/reports/phase-3-step-3.md`
- `packages/core/include/pixaura/manual.h`
- `packages/core/include/pixaura/storage.h`
- `packages/core/src/document.cpp`
- `packages/core/src/document.hpp`
- `packages/core/src/document_api.cpp`
- `packages/core/src/evaluation.cpp`
- `packages/core/src/geometry.cpp`
- `packages/core/src/manual_geometry_preview.cpp`
- `packages/core/src/manual_tools.cpp`
- `packages/core/src/manual_tools.hpp`
- `packages/core/src/storage.cpp`
- `packages/core/src/storage.hpp`
- `packages/core/src/storage_api.cpp`
- `packages/core/src/storage_schema_v2.hpp`
- `packages/core/src/tone.hpp`
- `packages/core/src/tone_constants.hpp`
- `packages/core/src/tone_math.hpp`
- `packages/core/swift/Tests/ManualBoundaryTests.swift`
- `packages/core/swift/Tests/StorageBoundaryTests.swift`
- `packages/core/tests/decode_allocation_test.cpp`
- `packages/core/tests/evaluation_test.cpp`
- `packages/core/tests/manual_geometry_c_consumer.c`
- `packages/core/tests/manual_geometry_test.cpp`
- `packages/core/tests/manual_test.cpp`
- `packages/core/tests/manual_tone_boundary.h`
- `packages/core/tests/storage_c_consumer.c`
- `packages/core/tests/storage_test.cpp`
- `packages/core/tests/tone_fixtures.hpp`
- `packages/core/tests/tone_test.cpp`
- `platforms/android/app/src/androidTest/java/ai/pixaura/app/ManualBoundaryTest.kt`
- `platforms/android/app/src/androidTest/java/ai/pixaura/app/StorageBoundaryTest.kt`
- `platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/CoreProbe.kt`
- `platforms/android/native/bridge.cpp`
- `QUALITY_GATES.md`
- `ROADMAP.md`
- `scripts/check-apple.sh`
- `scripts/tone-reference.py`
- `tests/document-contract.test.mjs`
- `tests/manual-geometry.test.mjs`
- `tests/geometry-execution.test.mjs`
- `tests/preview.test.mjs`
- `tests/support/document-contract.mjs`
- `tests/tone-color.test.mjs`
