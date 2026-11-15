# Phase11 native exit and emulator fixture correction

Diagnostic6d206416c28e9028958e40d370b54eefc5487fd9 is NOT certified.
Windows raw log exposes tile native exit112/0x70 after all161parity cases and
12000000pixels complete. It is not a numerical assertion failure or exception
code; exact compiler/CRT cause is not proven. Add explicit return0 after all
assertions rather than rely on implicit main termination. The subprocess wrapper
still propagates every nonzero exit; assertion exit1/exceptions remain failures.
Fresh exact-SHA Windows success is required before claiming portability.

Native source job failed because its fakeADB recognizes only the old exact
logcat filter; extend its exact case to the newly added AndroidRuntime/TestRunner
filters. Preserve all missing-evidence assertions/scenarios. Mac harness cannot
execute GNU realpath-m paths; retained local log documents that unchanged Linux
semantics limitation. Linux CI remains mandatory.

Android unchanged local focused2/2 and full67/67PASS; diagnostic hosted Android
PASS. Earlier daccc1d hosted Compose disposal crash remains a preserved
non-reproduced finding, not a claimed product fix. No assertions or production
code changed here. Prior source manifests immutable; v17 binds this successor.
All integration/store/physical/human acceptance gates remain open; no Phase11
closure, GPU provider promotion, signing changes or Phase12–14 implementation.
