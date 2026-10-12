# Phase 4 Step 4 — Android Capability Integration

Status: IMPLEMENTATION / VALIDATION IN PROGRESS. No unobserved gate is PASS.
Authorized baseline: `6389750c5ccab3b40d3ee76cb29bbb88b0fb5596`.
Steps 1–3 COMPLETE / FULL PASS; frozen roadmap unchanged; Steps 5–7 NOT STARTED.

## Implementation and acceptance

[Android integration contract](../contracts/android-capability-integration-v1.md)
records exact API mappings, local static/dynamic observations, one application
owner with bounded coalescing, and trusted checked JNI into the existing shared
policy. Additive platform-neutral editor facade retains its private raster owner.
Core alone chooses tier/cap/edge/recovery/fallback; existing PRV1/display lock
remains correctness authority. No policy constants, request semantics or storage
schema change. EXACT remains exact-or-reject; FIT alone adapts.

Require all normalized transitions and unknowns; static/memory overflow validation;
Vulkan available/unavailable/unvalidated; lifetime/recreation/fresh process state;
active parameters, one-revision commit, zero-revision cancel; delayed completion
fence; malformed JNI and synthetic trust restriction; callback/concurrency/churn;
all retained native/Android/Apple regressions and ten same-final-SHA hosted jobs.

## Observed evidence

Baseline HEAD and origin/main match; working tree initially clean; diff check PASS.
ADB device discovery observed no connected devices. Fresh physical Z Fold7
measurement and scope correlation are unavailable, not fabricated or counted PASS.
Historical Phase 2 certification remains unchanged; current adapter grants no
certification. Emulator validation remains mandatory and nonphysical.

Initial strict Android compilation exposed the Kotlin diagnostic for overriding
deprecated onLowMemory. A narrowly scoped compatibility annotation handles the
required older-OS callback; strict warnings remain enabled. Native facade compiled with strict Clang.


Local acceptance evidence: source/docs 88/88 (zero skips); focused contracts 18/18;
workflow actionlint and git diff --check PASS. Windows native 30/30 PASS; Linux
native 31/31 PASS; ASan/UBSan 33/33 PASS with active negative controls. Review
identified and fixed an A integration gap: configure production policy BEFORE
initial decode, via an additive policy-aware open; legacy open is unchanged.
Final rebuilt affected editor regression passed 1/1 on Linux and ASan/UBSan;
Windows final full regression passed 30/30. Initial insufficient budget preserves
context/strategy outputs and creates no document database; retry safely succeeds.
Logs: `build/step4-open-*-tests.log`, `build/step4-sanitize-tests.log` and
`build/step4-source-acceptance.log`. Strict Android debug/release, three ABI builds,
JVM 7/7 per variant, lint and instrumentation APK compilation PASS (151 tasks).
Final Android log: `build/step4-final-android.log`. Runtime instrumentation,
Apple native/Swift/simulator/app/UI and both exact-SHA workflows remain pending.

JVM tests include 10000 concurrent normalized callbacks, one shared registration,
final unsubscribe/close, stale callbacks after resubscribe, unsupported mappings
and overflow-safe budget observations. Android instrumentation declares 26 total
retained/new cases, including real OS probe wiring (nonphysical classification),
1024x512 synthetic source/FIT 360x180 under critical pressure, fixed active values,
late candidate delivery fence, recovery, LOW/MID/HIGH/unknown profiles, malformed
JNI, initial admission, 1000 pressure churn updates, 32 commit/cancel races, close
races, process-style reopen, real activity recreation and blocked-lane coalescing.
No runtime case is counted PASS until observed. Shared native late-kernel
checkpoint tests remain retained, distinct from Android delayed delivery tests.
The historical Step 1 no-facade structural assertion was updated (B) to enforce
pure capability foundation and delegating editor facade without policy decisions.
No semantic assertion, sanitizer, hard ceiling or timeout was weakened.

## Scope and failures

No architecture conflict or new ADR identified. A: Android probes/integration and
all acceptance remediation within Step 4. B: future Apple Step 5 and other frozen
steps require authorization. C: optional dashboards/analytics remain excluded.
Failure classifications and executed counts will be recorded from actual logs.
Schema 3, original fidelity, history/replay, hard ceilings and DH-APPLE-METAL-01
unchanged. No AI/model/Phase 5 work. Stop after Step 4; Step 5 NOT STARTED.
