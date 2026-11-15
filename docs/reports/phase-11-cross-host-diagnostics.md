# Phase11 remaining cross-host failure diagnostics

daccc1d170c1a1149130ac8ba03fab3a95b352e2 is NOT certified.
Foundation43701366170 attempt1:6/7SUCCESS, Windows tile test reports all161parity
cases and12000000pixels before nonzero termination; preserved logs lack raw exit
status. Native43701366222: Android crashed after49/67tests in Vietnamese
large-text activity destruction (Compose SlotWriter index-1976/5120).
Do not call either resolved or retry the failed SHA for a green result.

Add a native subprocess wrapper that prints signed/hex exit status and propagates
any nonzero as failure, with the same CTest timeout/assertions. Capture AndroidRuntime
and TestRunner live alongside existing synthetic engineering logs to preserve first
exceptions before activity teardown masks them. No product, SDK, assertion or
warning-policy change is made as a speculative fix.

Unchanged local Android focused accessibility2/2PASS; this does not prove full-suite
or hosted resolution. Full unchanged local sequence67/67PASS with exact synthetic evidence transport
and complete live TestRunner/AndroidRuntime log retained under
build/phase-11-final-android-diagnosis-full. No application/assertion correction
is claimed for the hosted crash; new exact-SHA first-exception observations
remain necessary. Parent source-v15/v14/v13 remain immutable; v16 binds
the diagnostic successor. Owner signing edits untouched; frozen native kernels,
ABI, Schema3, hardware PASS and provider trust unchanged. Phase11 IN PROGRESS.
