# Phase11 v18 private codec compiler successor (source v19)

b47d9b61639a7253bb89fa36e76431eca15965d2 is NOT certified. Foundation43806872858
attempt1 Windows portable failed compiling stream_codec_v2.c: MSVCC4324alignment
padding in Stream, promoted by /WX toC2220. Ubuntuportable passed; other jobs were
still running when diagnosed. Do not rerun this SHA to obtain green. Preserve all
v18 sources/report hashes in phase-11-source-v18.json; source-v19 records this fix.
Raw failed Windows job log build/phase-11-v18/windows-failure.log.

The explicitly aligned jmp_buf is now an automatic C stack object with a pointer
in Stream. Its lifetime covers every synchronous codec callback and longjmp cleanup.
Budget subtracts sizeof(jmp_buf) as well as state size from the same32MiB scratch.
No alignment suppression, warning downgrade, timeout or assertion change. Existing
JPEG Budget/Block layout unchanged; no frozen codec/core source edited. Numerical
output/recipe, installed public ABI, Schema3, runtime selection and shipping app
sources are unchanged by this successor. No UI/instrumentation retest solely for
this private host-test compiler layout change; prior v18 app sources are identical.

Focused native integration/owner/crash5/5PASS28.42s and ASan/UBSan5/5PASS93.85s
after correction; exact corrected GCCLinux5/5PASS21.98s, same export hashes.
Applicable isolated source221/221PASS after synchronizing the
actual committed parent manifest; initial isolated stale-copy mismatch retained
in layout-source-full.log. No historical primary manifest or assertion changed. Fresh full Foundation7/7 and Native3/3 REQUIRED on successor exactSHA;
not yet claimed here. Existing local v18 app/regression evidence retains its exact
scope, not a Windows pass. All prior manifests/reports remain freeze-time history.
Architecture CR18-01/CR18-02, physicalG7/G8, authenticG10 and humanG11 remain OPEN.
DHMetalPASSunchanged; CPUVALIDATED/MetalVulkanprovidersUNKNOWN. Phase11INPROGRESS.
