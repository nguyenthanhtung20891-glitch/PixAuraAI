# Phase 4 Step 4 — Android Capability Integration

Status: COMPLETE / FULL PASS (certified implementation). No unobserved gate is PASS.
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
Final Android log: `build/step4-final-android.log`. Runtime and hosted gates are certified below.

JVM tests include 10000 concurrent normalized callbacks, one shared registration,
final unsubscribe/close, stale callbacks after resubscribe, unsupported mappings
and overflow-safe budget observations. Android instrumentation declares 26 total
retained/new cases, including real OS probe wiring (nonphysical classification),
1024x512 synthetic source/FIT 360x180 under critical pressure, fixed active values,
late candidate delivery fence, recovery, LOW/MID/HIGH/unknown profiles, malformed
JNI, initial admission, 1000 pressure churn updates, 32 commit/cancel races, close
races, process-style reopen, real activity recreation and blocked-lane coalescing.
All 26 runtime cases were observed PASS, zero failures and zero skips. Shared native late-kernel
checkpoint tests remain retained, distinct from Android delayed delivery tests.
The historical Step 1 no-facade structural assertion was updated (B) to enforce
pure capability foundation and delegating editor facade without policy decisions.
No semantic assertion, sanitizer, hard ceiling or timeout was weakened.

Bytecode review found an A JNI linkage defect: Kotlin internal visibility mangled
`nativeVulkanAvailability` while C++ exported its stable name. Explicit @JvmName
preserves internal source scope and fixes debug/release linkage. javap confirmed
the exact stable symbol in both variants; Android build/JVM/lint and source guards
are revalidated. No trust or shared policy change.

First hosted implementation Foundation 38280922829 found MSVC C4456: the new
strategy local shadowed an existing document variable (A compiler portability).
Renamed the strategy local; no behavior or warning settings changed. First SHA
b422d9a3708996170f6fc33d643201fdb859d7d7 is not accepted; final ten-job closure
is required after these narrow fixes.

## Exact-SHA certification

Certified implementation: `16483ff498f8a2ac67152f6a5b8fdac0e6d822fe`.
Foundation boundaries [38281943443](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/38281943443): 7/7 SUCCESS.
Native application shells [38281943346](https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/38281943346): 3/3 SUCCESS.
Both complete workflows passed on that SAME SHA, verified from exact run JSON.
A later documentation-only closure SHA must also pass both complete workflows;
its exact final SHA/run IDs are delivered separately to avoid a self-referencing commit.

| Required job | Observed result |
| --- | --- |
| Foundation Windows native | SUCCESS, 30/30 |
| Foundation Linux native | SUCCESS, 31/31 |
| Foundation ASan/UBSan | SUCCESS, 33/33 including active negative controls |
| Foundation Apple boundary | SUCCESS, native 30/30; Swift macOS 19/19; iOS simulator 19/19 |
| Foundation Android arm64-v8a | SUCCESS, checked C/C++ ABI build |
| Foundation Android armeabi-v7a | SUCCESS, checked C/C++ ABI build |
| Foundation workflow lint | SUCCESS, pinned actionlint |
| Shells source | SUCCESS, 124/124, zero skips; independent fixtures and shell syntax |
| Shells Android app | SUCCESS, strict debug/release/three-ABI/JVM/lint builds; instrumentation 26/26, zero skips |
| Shells iOS app | SUCCESS, package/C ABI gates, debug/release app builds, app tests 6/6, UI tests 2/2 |

Retained Phase 0–3 and Step 1–3 native/capability/admission/dynamic regressions
execute in these complete workflows. Android JVM tests pass 7/7 per variant.
Android actual-probe wiring, initial admission, mappings, LOW/MID/HIGH/unknown,
malformed signed/versioned JNI values, no trust escalation, 1000 pressure churn,
32 commit/cancel races, close/stop lifetime, fixed active parameters, recovery,
late candidate rejection, blocked-lane coalescing and three actual activity
recreations all passed. Native checkpoint-controlled late kernel completion and
80000 update/80000 admission concurrency regressions also passed unchanged.

Pressure preserves 1234 milli-EV while active; latest eligible reduced preview
publishes, delayed older completion cannot install. Critical FIT display is
360x180 from a 1024x512 synthetic original; source/reopen remains full fidelity.
Changed commit adds exactly one revision, cancel adds zero. Policy changes add
none. Three known stable observations recover at most the original static cap;
repeated identical strategy does not churn epoch or schedule redundant previews.
Epoch/PRV1 fences, not cancellation timing, remain publication authority.

Lifecycle uses application activity counts and one shared OS subscription;
configuration replacement preserves ownership, close invalidates callback tokens,
and native stop/join/quiesce precedes destroy. JVM churn verifies one registration
and final unregister without stale callback effects. No Activity reference,
new cache, unbounded policy snapshot list or callback queue is introduced.
Process-style reopen gets a fresh epoch without changing durable history.

EXACT remains exact-or-resource-failure in the retained shared preview corpus;
the Android editor explicitly requests FIT, with no local resampling policy.
Native raster/context/scratch/handle/storage ceilings and Step 2 maxima remain
unchanged. Signed arithmetic and checked JNI conversion cannot raise them.
Vulkan availability remains NOT_VALIDATED; no test seam supplies certification.
The current physical-device gate is unavailable (no connected device), never PASS.
All Android hosted evidence is emulator wiring/lifecycle/synthetic evidence,
not physical thermal/memory/GPU certification. Historical Z Fold7 certification
is preserved without current-runtime scope correlation or broadened trust.

Failure classification: A initial-admission integration gap, JVM-name linkage
and strict MSVC shadowing corrected minimally. First Android run 38280922840
had exactly one failure: UnsatisfiedLinkError for the mangled probe method;
corrected exact-SHA run passes all 26 tests. B old phase-boundary structural guard
updated to enforce delegating authority; legacy callback compatibility annotated
without disabling strict warnings. C no runner retry required for certification.
No assertions, sanitizers, ceilings, semantic checks or timeouts weakened.
No architecture conflict/new ADR. Raw logs/artifacts remain ignored under build.

## Scope and failures

No architecture conflict or new ADR identified. A: Android probes/integration and
all acceptance remediation within Step 4. B: future Apple Step 5 and other frozen
steps require authorization. C: optional dashboards/analytics remain excluded.
Failure classifications and executed counts will be recorded from actual logs.
Schema 3, original fidelity, history/replay, hard ceilings and DH-APPLE-METAL-01
unchanged. No AI/model/Phase 5 work. Stop after Step 4; Step 5 NOT STARTED.
