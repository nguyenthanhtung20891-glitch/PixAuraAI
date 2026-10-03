# Phase 2 Step 1: image document and edit pipeline contracts

Date: 2026-10-03. Authorized scope: Step 1 only. No commit/push; Step 2 not started. Status: **Step 1 contracts and fixture tests complete; locally executable validation PASS, with existing MSVC installation BLOCKED and compiler fallback PASS; new Apple/hosted CI execution pending**. This is not full Phase 2 completion.

## Prior phase evidence and repository audit

At authorization the user reports all Phase 0/1 GitHub Actions gates PASS, including source, Android shell/build/lint/JVM, emulator JNI/Compose, Apple shell and iOS build/test. This final completion is **user-attested**; a final passing run URL was not supplied or independently inspected in this step. Earlier phase reports retain their historical observations. The stale Phase 1-only authorization in root documents is updated to the user's explicit Step 1 boundary without rewriting accepted historical ADRs.

Inspected tracked repository inventory, all thirteen authoritative root documents, README, dependency/toolchain reviews, ADRs 0001-0007, prior reports, both workflows, all core/build/ABI consumers, Kotlin/Swift shell integration and source/native/orchestration scripts/tests. No source document model, SQLite repository, persistence, codec, operation registry or renderer exists. Source asset/history/approval/storage policies are already architectural requirements; shell SavedStateHandle/SceneStorage contains only navigation/mode. ABI 1 exposes only a checked 16-byte probe with zero feature bits. Native UI calls it on background workers. CMake/Android/Swift Package share core.cpp.

## Decisions and delivery

ADR [0008](../adr/0008-document-stacks-and-manifest.md) and the [normative schema/lifecycle contract](../contracts/image-document-v1.md) freeze:

- Ordered linear operation stack per immutable revision; single-parent revision history retains branches. No general multi-input pixel DAG. Full stacks permit replacing/removing/reordering an earlier operation in a new revision without rewriting history.
- Immutable source descriptor (SHA-256, byte length, bounded metadata), project/document identities, versioned operation records and approved revision records. Undo/redo changes current plus explicit contiguous redo path; commit after undo clears only redo navigation, preserving branches. Fresh session identity plus generation rejects stale preview/commit ABA, including after reopen; persisted catalog epoch protects durable publication.
- Stable operation type and separate operation/parameter versions; integer units, exact parameter keys/ranges and fail-closed unknown types/versions. Exposure/crop/quarter-turn schemas are metadata examples only; all other named tools are reserved, not pixel implementations. AI edits use the same registered tools and explicit approval, not a new opaque pixel operation.
- Canonical bounded JSON schema 1 snapshots with a complete synthetic golden fixture. ASCII key sort, preserved arrays, integer decimals, explicit versions, LF terminator; strict duplicate/type/reference/UTF-8/size/depth validation. Unknown projects remain preserved read-only, never silently downgraded or reset.
- SQLite remains live catalog authority. Assets flush before transaction reference; immutable operations/revisions and current/redo/epoch commit atomically. Manifests are derived atomic checkpoints/interchange. Migration/recovery preserves originals and retained history; actual adapters and crash tests remain future work.
- Designed C++ validated immutable values and minimal Result-based lifecycle/transition interfaces. Planned independent C document API family with immutable generation-checked handles, bounded open/command/serialize/release, caller-owned buffers, two-call serialization, typed errors, exception containment and explicit thread/lifetime rules. **No production C++/C ABI changes**; ABI 1, feature bits, JNI/Swift calls and shipping builds remain unchanged.
- Const preview/export request contracts: snapshot/revision/candidate generation, budgets/cancellation, approved-only export and new-output capability. Native result ownership and exactly one terminal state; no renderer, pixel buffers, decoding, GPU, inference, networking, database or editing controls added.

## Focused tests and limits of evidence

`tests/document-contract.test.mjs`, `tests/support/document-contract.mjs` and `tests/fixtures/image-document-v1.json` add nine test groups. They test golden serialization/reload/key order; ordered replay/replacement; undo/redo/branch retention; stale/invalid command nonmutation and ABA; operation types/versions/parameters; malformed roots/refs/cycles/redo/actor; AI provenance using the same stack; UTF-8/duplicate keys/corruption/numeric/depth/byte bounds; resource admission without pruning.

The oracle is **engineering-only**, with no shipping import. Metadata/source descriptor immutability is exercised; actual original-byte hash preservation is not implemented or claimed. Replay returns the ordered metadata recipe, not pixel output. Native parser fuzzing, C document handles, ownership safety, renderer parity, asset hashing and durable crash recovery remain mandatory before those production implementations can pass G2/G4/G5/G6. Existing C ABI/sanitizer safety gates are rerun unchanged.

Both workflows retain all existing tests and add contract tests. Foundation source jobs cover Windows/Linux, and its Apple job adds the same fixture test before its unchanged package/build/XCTest script. Shell source adds the tests while preserving actual emulator/simulator execution, strict dependency verification, lint, bounded readiness, effective 2 GiB hosted userdata proof, KVM preference/fallback and evidence uploads.

## Local validation

| Check | Observed result |
| --- | --- |
| Windows foundation/CI/shell/contract source tests | PASS 25/25, zero skips/failures; rerun after session-ID change and Android completion |
| Linux foundation/CI/shell/emulator orchestration/contract tests | PASS 54/54, zero skips/failures; updated session-ID contract suite additionally rerun 9/9 on Linux |
| Bash syntax; actionlint | PASS; actionlint exit 0 with unavailable shellcheck/pyflakes disabled, independently checked Bash syntax |
| Linux CMake configure/build/CTest | PASS 2/2 independent C/C++ consumers; unchanged compiled objects reused, tests executed |
| Actual ASan/UBSan CTest | PASS 4/4; both positive consumers and negative runtime diagnostic probes, expected exits 86/87 |
| Standard Windows MSVC script | BLOCKED: existing missing vcvarsall; owner host maintainer, remediation repair desktop C++ workload |
| Windows Zig compiler fallback | PASS DLL, independent C/C++ consumers and undefined-behavior trap-mode consumer; not a substitute for actual ASan above |
| Android strict fresh build/lint/JVM/instrumentation APK and connected execution | PASS, exit 0, BUILD SUCCESSFUL in 2m, 147/147 actionable tasks executed |
| Android JVM / JNI-Compose execution | PASS 2/2 JVM and 1/1 connected test, zero skips/failures/errors; fresh XML timestamps 2026-10-03T11:50:45.462Z and 2026-10-03T11:51:28 |
| Android full Debug/Release lint | PASS; both text reports: No issues found. SDK XML-version warning remains nonfatal |
| Final source/whitespace and Git scope | PASS; only 16 intended docs/test/workflow files; HEAD unchanged at 2525b52207ef1f6a3af945032d8190366b177f27 |
| Apple macOS core/C/Swift and iOS build/unit/UI for working tree | BLOCKED locally: no macOS/Xcode; pending new CI, not counted PASS |
| Production document parser/handles/storage/renderer | NOT_APPLICABLE to contract-only Step 1; mandatory later implementation gates, not waived |

Commands executed:

```text
node --test tests/foundation.test.mjs tests/ci-tools.test.mjs tests/shells.test.mjs tests/document-contract.test.mjs
build/tools/actionlint/actionlint.exe -shellcheck= -pyflakes= .github/workflows/foundation.yml .github/workflows/native-shells.yml
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check-native.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/check-native-zig.ps1 -ZigPath G:/PixAuraAI/build/tools/zig-x86_64-windows-0.14.1/zig.exe
wsl --distribution Ubuntu --exec bash -lc 'cd /mnt/g/PixAuraAI && bash -n scripts/*.sh && build/tools/node-v24.14.0-linux-x64/bin/node --test tests/foundation.test.mjs tests/ci-tools.test.mjs tests/shells.test.mjs tests/android-emulator.test.mjs tests/document-contract.test.mjs && cmake -S . -B build/linux-host -DCMAKE_BUILD_TYPE=Debug && cmake --build build/linux-host && ctest --test-dir build/linux-host --output-on-failure && bash scripts/check-sanitizers.sh'
wsl --distribution Ubuntu --exec bash -lc 'cd /mnt/g/PixAuraAI && build/tools/node-v24.14.0-linux-x64/bin/node --test tests/document-contract.test.mjs'
powershell -NoProfile -ExecutionPolicy Bypass -File build/phase-2-step-1/check-android.ps1
git diff --check
```

The ignored local Android harness starts only the owned repository-local pixaura-shell AVD at port 5554, checks boot/package readiness (300s overall, 15s per ADB command), runs the following exact Gradle command, then stops its owned emulator in finally. Local SDK emulator 36.5.11.0/build 15261927 uses the already installed API 36.1 Google Play x86_64 image with WHPX/software GPU; this is local shell regression evidence, **not a reproduction of the hosted API 35 image**. Hosted API 35 x86_64/effective 2 GiB userdata/KVM execution is unchanged and must run in CI on the new revision.

```text
ANDROID_HOME=I:/AndroidStudioSDKdata
ANDROID_USER_HOME=G:/PixAuraAI/build/android-user
ANDROID_AVD_HOME=G:/PixAuraAI/build/android-avd
GRADLE_USER_HOME=G:/PixAuraAI/build/gradle-home
ANDROID_SERIAL=emulator-5554
JAVA_OPTS=-Xmx64m -Xms32m -XX:+UseSerialGC
ANDROID_SDK_HOME unset for Gradle (legacy ADB/emulator helper uses repository-local parent)
platforms/android/gradlew.bat -p platforms/android --no-daemon --offline --dependency-verification strict --no-build-cache --rerun-tasks --max-workers=1 "-Dorg.gradle.jvmargs=-Xmx768m -Xms128m -XX:+UseSerialGC -Dfile.encoding=UTF-8" "-Pkotlin.compiler.execution.strategy=in-process" assembleDebug assembleRelease testDebugUnitTest lintDebug lintRelease assembleDebugAndroidTest connectedDebugAndroidTest
```

Initial temporary-harness attempts failed before application tasks: a process-wrapper exit-code/readiness issue despite boot property 1; conflicting SDK_HOME/USER_HOME effective preference folders confirmed by Gradle stacktrace; and PowerShell splitting an unquoted dotted property. Corrected only the ignored helper, then executed all 147 tasks successfully; no failed attempt is counted PASS. ADB/WSL required sandbox escalation to access their installed services; no package installation, dependency verification relaxation or production source fix was needed. Evidence: build/phase-2-step-1 emulator/probe/Gradle environment logs, app build/reports and test-results/outputs, build/linux-host/Testing and build/sanitize/test-logs. Owned emulator stopped; no validation process remains pending.

macOS/Xcode is unavailable on Windows. Owner repository maintainer; remediation run Foundation boundaries and Native application shells on the reviewed/published revision, retaining Apple result artifacts. Prior user-attested Apple CI completion remains separate evidence. Complete Phase 2 G5 durability and G6 pixel/GPU parity remain future work.

## Risks, next step and human action

Metadata limits are conservative schema policy, not measured decoder/device support. Full-stack snapshots can hit the 8 MiB limit before individual count limits; reject safely and require explicit migration/compaction later, never prune silently. Exact crop rasterization, adjustment kernels/tolerances, profile/codec choice, mask format, GPU/backend coverage and database durability settings must be specified/tested before their implementations. New C-family statuses/error layout are designed but require compiled parity/ownership tests before export. Those details are not silently implemented by the oracle.

Proposed Phase 2 Step 2: implement bounded C++ manifest/registry validation and serialization, immutable revision transitions, and the minimal checked C document API family; port fixed fixtures to independent C/C++/Swift/JNI tests and execute parser fuzz seeds/ownership/allocation failures under sanitizers. Keep codec/rendering/DB/UI work in separately scoped later steps. Step 2 is **not started**.

Human decisions required next: review/accept ADR 0008 and schema 1 limits/semantics, then explicitly authorize the proposed Step 2 scope if desired. No additional product/platform compromise is required for Step 1. Commit/push require separate authorization. Retain final passing Phase 0/1 run URLs for evidence reconciliation and rerun both full workflows on any later published Step 1 revision; unchanged Apple code is not new Apple execution evidence. Do not start Step 2 automatically.

## Files

New: this report; docs/contracts/image-document-v1.md; docs/adr/0008-document-stacks-and-manifest.md; tests/document-contract.test.mjs; tests/support/document-contract.mjs; tests/fixtures/image-document-v1.json.

Updated: AGENTS.md; ARCHITECTURE.md; PHOTO_ENGINE.md; ROADMAP.md; QUALITY_GATES.md; DECISIONS.md; TESTING_STRATEGY.md; README.md; .github/workflows/foundation.yml; .github/workflows/native-shells.yml. No application, production core, build lock, dependency or emulator script changed.
