# Quality gates and promotion policy

Status vocabulary: PASS (executed with evidence), FAIL (executed and failed), BLOCKED (mandatory environment unavailable), NOT_APPLICABLE (out of milestone scope with reason). Skips are never PASS. A milestone advances only when all its mandatory gates pass and advancement is authorized. Current scope ends at Phase 1 regardless. No unobserved CI run is evidence.

| Gate | Required evidence | Applies |
| --- | --- | --- |
| G0 specification | Required docs, ADRs, link validation, measurable milestone criteria, no product contradiction | Every phase |
| G1 source quality | Whitespace/conflict checks; warning-clean native build; platform lint/format/type checking when source exists | Every phase |
| G2 behavior | Unit/integration tests for changed behavior and safety/error paths | Every phase |
| G3 build | Host core build; Android native adapter compile; Apple Swift/C boundary compile; full app debug/release builds once shells exist | Phase 0 boundaries; Phase 1+ apps |
| G4 native safety | ASan/UBSan and malformed-input fixtures; fuzz regression seeds when parsers exist | Phase 0+ native code |
| G5 persistence | Migrations, interruption recovery, source hash preservation, atomic revision commits | Phase 2+ |
| G6 parity | CPU/GPU numeric tolerance and preview/export geometry consistency | Phase 2+ renderers |
| G7 offline/privacy | Core flows airplane-mode; zero photo traffic; permissions/backup/log checks | Phase 1 policies, Phase 3+ flows |
| G8 device/performance | Physical low/mid/high cohort; p50/p95, peak RSS and thermal data; bounded fallback | Phase 4+ |
| G9 AI quality/control | Allowlist/ranges/stale plans, model provenance/parity, suggestion evaluation, explicit approval | Phase 5+ as applicable |
| G10 commerce | Store sandbox lifecycle, offline entitlement, restore/refund tests; pricing/policy verification | Phase 10+ |
| G11 accessibility | Assistive-tech walkthrough, dynamic text, contrast, input alternatives | Shell baseline Phase 1; full Phase 11 |
| G12 release | Both platforms signed builds, SBOM/license/security review, rollback/recovery/support checklist; no critical defects | Phase 12 |

## Phase 0 acceptance
All thirteen requested authoritative root documents; README and source/build instructions; docs/adr decisions with accepted/proposed status and consequences; skeleton portable C ABI with independent C/C++ consumers, Android JNI and Swift bindings; automated foundation check; local warning-clean native build/tests; platform/sanitizer build automation. G0/G1/G2 must execute locally. G3 includes Android native compile/link, macOS core/C/Swift tests, unsigned iOS package build and iOS simulator Swift/C ABI tests. G4 requires actual execution of the shared core tests under ASan/UBSan plus negative probes whose runtime diagnostics prove instrumentation; compiler flags alone never qualify. Unavailable host prerequisites leave Phase 0 validation incomplete even if automation is configured. Do not proceed to Phase 1 on partial status or without the user's authorization to advance.

Phase 0 reports separately classify evidence as locally validated, CI validated, pending CI execution, or legitimately unavailable on the current host. The last category describes host availability and does not waive a mandatory gate. READY FOR CI means all mandatory local checks have passed and remaining platform jobs are executable but unobserved. FULLY PASSED requires every mandatory gate to have executed successfully, with a run URL/logs for CI evidence. No unobserved workflow receives CI validated status.

## Phase 1 acceptance
Phase 1 acceptance: native Compose/SwiftUI apps render Home/Editor/Projects/Settings and show ABI 1 through the actual shared boundary; mode selection survives navigation without project mutation; Android recreation and iOS scene restoration preserve shell state; debug/unsigned release builds; strict Android lint, JVM reducer tests, Android emulator JNI/Compose tests; Apple package/core tests, iOS simulator Swift/C ABI/UI tests; workflow/source checks and existing sanitizers; no dangerous/media/network permissions, analytics, authentication or content networking. Native labels, scrolling and system text/color/touch defaults supply the accessibility baseline; physical TalkBack/VoiceOver walkthrough evidence is tracked separately and never inferred from compilation.

CI executes both Foundation boundaries and Native application shells. Apple app/package execution remains mandatory even when Windows gates pass. Signing is outside Phase 1; no store credentials are needed for simulator tests or unsigned builds.

## Budgets to validate

Initial targets on recorded reference devices: UI frame p95 <=16.7 ms for ordinary interaction at 60 Hz; T0 720px basic adjustment preview p95 <=150 ms warm; cancel acknowledgement <=250 ms; import/project UI response <=100 ms excluding decode; 12 MP JPEG export <=10 s T1 and <=30 s T0; no unbounded allocations and tier caps per DEVICE_CAPABILITY_STRATEGY.md. Cold model loading separately measured. Targets are refined only via ADR with data, never quietly relaxed. Phase 2 sets numerical tolerances per operation before implementation; Phase 5 sets task-quality thresholds before model selection.

## Reporting and exceptions
Each report names criteria, exact commands/tool versions, results, affected files, failed/unrun gates, blockers and next milestone. Required environmental blocker includes remediation and owner. Never waive source overwrite, approval or privacy invariants. Temporary noncritical exception requires explicit documented owner, expiry and bounded exposure; user authorization cannot be inferred for a product-level compromise.
