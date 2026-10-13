# Phase 4 Step 5 — Apple capability integration

Status: IN PROGRESS / VALIDATION. Baseline HEAD and origin/main initially
`40a0c9bef05df328a2b3e89497316c368b81aa24`; working tree and diff check clean.
Steps 1–4 COMPLETE / FULL PASS; frozen seven-step roadmap unchanged.
Steps 6–7 NOT STARTED. [Contract](../contracts/apple-capability-integration-v1.md).

Implementation adds one caller-owned Apple service, trusted static/runtime
observations, bounded observer lifetime and observation coalescing on existing
command lanes. Policy-aware editor open precedes decode; apply/query reuse
existing lifecycle/display gate. Shared policy, PRV1, schema, history, parameters,
reference semantics and hard ceilings remain unchanged. No new architecture
decision or conflict. No Android-only/shared semantics are introduced.

Tests add legitimate probe/mapping checks, synthetic LOW/MID/HIGH pressure and
recovery, large-source FIT resizing, delayed candidate rejection, gesture commit/
cancel, process-style reopen, malformed records and trust rejection, observer
cleanup/reconstruction, 10000 callback churn and concurrent close, 1000 stable
updates per profile and 48 commit/cancel-pressure races. Retained native tests
remain actual late-kernel checkpoint authority and EXACT-vs-FIT proof.

No physical Apple hardware available. Simulator and hosted macOS evidence are
nonphysical target evidence only. Metal availability never certification;
DH-APPLE-METAL-01 remains DEFERRED. Schema 3 unchanged. No Phase 5/AI/model work.

## Observed validation

Local source/docs 89/89 (zero skips), workflow actionlint and git diff check PASS.
Local retained Windows native 30/30 PASS (`build/step5-windows.log`). First hosted
implementation `8bf44312b0c97b4be88fdd0d582624b4c9ff9d84` failed Apple test
compilation (B): Swift concurrency diagnostics in the new race harness. Remediation
moves blocking joins and synchronized immutable captures into a synchronous test
fixture; production policy and strict warnings/assertions remain unchanged.

Corrected implementation `7a0447a4c7815640815fa6e525b6e04262a6c941` observed
Apple boundary SUCCESS: native 30/30, Swift macOS 24/24 and iOS Simulator 24/24,
including all five new Apple cases; Linux and ASan/UBSan SUCCESS. Android app
job SUCCESS. Full workflow closure still pending Windows/iOS application job.
Final test review adds the explicit NORMAL→ELEVATED→SEVERE→NORMAL thermal trace
and UInt64.max budget rejection to the same fixture corpus before acceptance.
Final adapter review found A: coalescing older unavailable backend with newer
unknown must retain UNKNOWN, rather than accidentally create known recovery
evidence. Availability merge now preserves unknown in either pending reading;
tests cover both backend and memory unknown coalescing. Shared policy unchanged.

Pending execution; no unobserved gate is PASS. Final acceptance requires both
Foundation boundaries (7 jobs) and Native application shells (3 jobs) complete
SUCCESS on the same final SHA. Exact run IDs and counts will be recorded after
observation. Failure classification A product, B harness, C infrastructure will
be recorded from exact failing logs; assertions and limits remain intact.
