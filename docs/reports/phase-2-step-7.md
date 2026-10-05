# Phase 2 Step 7: bounded geometry, tiles and cancellation

Accepted baseline: fd545ac545d526e779ca82daf8ac58e20cf39f2f (Product Owner FULL PASS). Scope: Step 7 only; Step 8 not started. Status: **BLOCKED on external GitHub billing/spending-limit infrastructure. Implementation/contracts and local gates PASS; Step 7 is not ready for FULL PASS.** Hosted jobs did not execute; no unexecuted gate is PASS.

## Audit and architecture

Root product/architecture/photo/security/quality/roadmap/decision documents, relevant ADRs and Step 1-6 contracts/reports were inspected. Crop/rotate /1/1 are registered strict integer metadata, with exposure the only executable tool. Crop rasterization was deliberately unspecified in Step 1. Implementation stopped for the required decision; Product Owner approved outward rounding explicitly. [ADR 0013](../adr/0013-bounded-geometry-and-cancellation.md) records that approval without rewriting historical ADRs or operation metadata. Existing top-left orientation, ordered stacks, immutable history/source, transient pixel ownership, 128 MiB raster/256 MiB context ceilings and quiescent destruction remain intact. Earlier contracts describe future preview/cancellation ownership but no concrete tile/token implementation existed.

## Contracts and implementation

[Geometry v1](../contracts/geometry-v1.md) specifies origin/axes, half-open pixel edges, half-integer centers, exact checked outward millionth crop arithmetic, nonempty legal crops, orthogonal forward/inverse mappings and golden vectors. C++ preflight retains one stage per recorded operation in order, without optimization. Crop/rotate plan dimensions only and explicitly remain unsupported for execution, even zero turns. Exposure stages preserve dimensions. Public metadata exposes output dimensions, tile count, raster/metadata bytes, conservative work and executable flag.

128x128 tiles are lazy, bounded, destination-only and row-major; edge tiles never extend beyond the destination. At most 4096 tiles; no tile array allocation. Stage payload <=9216 bytes, parser/plan conservative allowance 1 MiB plus bounded control overhead; stack <=256, request <=64 KiB, object <=1024 bytes. Work estimate <=2147483648 stage-input pixel visits plus one copy/validation pass for executable stacks. Full raster ceilings remain unchanged; tiles do not admit larger images. Maximum two request rasters, no heap kernel scratch.

[Cancellation v1](../contracts/cancellation-v1.md) specifies sticky atomic context tokens, distinct kind registry, never-reused serials and combined 64-handle admission. In-flight evaluation retains token lifetime across release. Signal/release can run while the ordinary context mutex is held; final publication and signal share the separate mutex. Cancellation-before-publication returns existing status 13; publication-first success stays valid. No new status codes, ABI 1/schema/storage migration or dependency. All failures preserve output/source/history and reclaim private allocations. Context destruction requires joined/quiescent calls. No pools, scheduler or parallel tile execution.

Checkpoints cover admission, lock acquisition, bounded parsing, preallocation, every copy/validation/exposure tile, operation boundaries and final publication. Arithmetic work between checks <=16384 pixels. Bounded parsing, allocation/value initialization and locks cannot be preempted; no fixed wall-clock latency or unsafe termination is claimed. Applications still own stale-candidate UI publication.

## Validation evidence

Focused Linux geometry/evaluation: PASS 2/2 on the initial implementation. Final geometry tests contain 51802 assertions, 2048 geometry properties, 2048 parser mutations, 2048 token lifecycles, 64 actual C ABI cancellation/evaluation/source-read races and all 12 observed kernel checkpoints. Explicit 90/180/270 golden vectors, four-turn identity, maximum 256-operation work admission and one-over-limit rejection pass. Existing exposure 77822 assertions, including numerical edge cases, still pass. Allocation sweep expands from seven to ten phases: shared parsing/plan allocation, token object/registration and cancellable destination/registration; 220 Windows and 294 Linux injected allocation failures recover cleanly. Lazy tiles have no metadata allocation to inject; constant stack state is tested instead. Registration failures are included in the allocation-site sweep. Every cancellation/failure leaves source/caller outputs unchanged and permits a subsequent valid request.

| Gate | Executed evidence |
| --- | --- |
| Windows source | PASS 42/42, zero skips/failures |
| Linux source | PASS 71/71, final repetition, zero skips/failures |
| Windows native fallback | PASS 13/13 CTest, plus existing independent consumers/trap checks; actual hosted MSVC BLOCKED |
| Linux native CTest | PASS 13/13 warning-clean Clang build |
| ASan/UBSan | PASS 15/15, leak checks and negative actual instrumentation probes (86/87) |
| Android | PASS Debug/Release, both lint checks (No issues found), four fresh instrumentation cases; JVM debug 2/2 and release 2/2 freshly rerun (49/49 tasks executed), zero errors/failures/skips |
| Apple wiring | Source/build/Swift boundary wiring checked; actual macOS/iOS execution BLOCKED on hosted infrastructure |
| actionlint/shell/hygiene | actionlint 1.7.12 exit 0 (local shellcheck/pyflakes unavailable and disabled); Bash syntax and git diff --check pass |

Repository-local evidence is under ignored build/phase-2-step-7. Commands: check-native-zig.ps1 (delegates full CMake fallback), WSL private mount namespace close.sh (Linux CTest, check-sanitizers.sh, node --test tests/*.test.mjs, Bash syntax), explicit Windows ten-file source command, repository-isolated Android Gradle debug/release/lint/JVM/connected command. Windows TEMP/TMP, Java tmpdir, WSL TMPDIR, caches and emulator state are repository-contained. Initial sandbox Node/WSL child-process denial was resolved through authorized tooling access; initial ADR-template omission was fixed and source gates rerun. Neither failed attempt is counted PASS.

Planning diagnostic: 4096x2048, tile edge 128, 512 tiles, one stage 36 metadata bytes, no tile-array bytes. Windows plan 6.200 us/traversal 36.300 us; Linux 2.965 us/40.916 us; sanitizer 11.660 us/69.860 us. Timings include validation overhead and are informational. Existing 1024x1024/16-exposure diagnostic: Windows 1104.953 ms, Linux 813.387 ms, sanitizer 2297.661 ms; two rasters 32 MiB, scratch zero. No throughput threshold or device latency certification.

Exact source/documentation inventory: [phase-2-step-7-files.txt](phase-2-step-7-files.txt), 27 files. No generated build/cache/database or credentials are intended for staging. No crop/rotate pixels, UI, renderer, export, AI or Step 8 work.

## GitHub Actions and closure blocker

Implementation commit **abaf650bae65e291236f645294861183251435c5**, `feat(core): add bounded geometry plans, tiles and cooperative cancellation`, was committed and pushed to approved origin/main using scripts/codex-git.ps1. Both workflow run identities were obtained with scripts/codex-gh.ps1 run-list filtered to that exact SHA before inspecting their annotations:

| Run | Result | Executed work |
| --- | --- | --- |
| [Foundation boundaries 37319461556](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37319461556) | GitHub failure; infrastructure BLOCKED | All seven jobs refused before starting: Windows/Linux portable, sanitizer, workflow lint, two Android ABIs, Apple boundary |
| [Native application shells 37319461403](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/37319461403) | GitHub failure; infrastructure BLOCKED | All three jobs refused before starting: source, Android app, iOS app |

Exact shared annotation: "The job was not started because recent account payments have failed or your spending limit needs to be increased." GitHub directs the account owner to Billing & plans. Failed-log retrieval reports logs unavailable because no runner work started. This is not a compilation/test failure, and no hosted Apple/MSVC PASS can be inferred from earlier Step 6 results. Repository code cannot fix billing, and changing account settings is outside authorization. Stop condition C applies. Owner remediation: resolve payment/spending-limit restriction, then resume the unchanged Step 7 hosted gates and any evidence-driven remediation; do not weaken workflows or start Step 8.

A focused documentation closure commit records this blocker after the implementation commit; its SHA is reported in the final response. No further implementation change or attempt to alter billing/runner policy is made. Working tree is intended to be clean after that commit/push. Local evidence, including current annotations, remains in ignored build/phase-2-step-7.

## Limitations and next step

Geometry execution, interpolation/resize, preview renderer, GPU, export and AI are absent. Tokens cancel only the new synchronous exposure/identity evaluation capability, not existing decode/normalize/storage calls. Tiling retains full destination storage. Planning timing is informational, not a physical-device latency/RSS claim. Proposed Step 8: separately authorize minimal geometry pixel execution using these frozen mappings and reference fixtures; do not begin it automatically.
