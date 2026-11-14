# Phase 11 iOS certification-controller reconciliation

Phase11 remains IN PROGRESS. No Phase12 authorization or hardware trust change.
The remaining-engineering submission and all predecessor evidence remain intact.

## Observed predecessor certifications

Implementation244bfeead60dc47bb909866d8a497b8e236db84c was not certified:
Foundation43615800385 and Native43615800342 failed tracked source inventory for
new proposed ADR0024. The explicit hash-bound inventory successor corrected this
without changing the historical Phase7 manifest or approving commerce policy.

Successor d5b5e156147b9cd558e44701b9b23b0c112b4bab:
Foundation43616011252 SUCCESS7/7, attempt1, exact SHA.
Native43616011177 FAILED, attempt1, exact SHA: source and Android SUCCESS,
iOS FAILED after its actual tests passed. It is not certified and is not rerun.
Android instrumentation63/63, no failures/skips; release scanner PASS.
Hosted iOS app tests17/17 and UI19/19 PASS. The failing command occurred afterward:
simctl ui could not access the selected Simulator in Shutdown state (SimError405),
exit149. No behavioral assertion failure is claimed for that run.

## Narrow correction and focused verification

Xcode can execute tests using a clone and leave the selected destination shut down.
The additional large-text controller must explicitly establish boot readiness before
querying/changing UI settings. It now calls simctl bootstatus with its documented
-b option before setup and before restoration. The focused run disables parallel
test cloning so the configured destination is the test destination. No test is
excluded from the normal suite, assertion changed, timeout extended or failure ignored.
Original Dynamic Type settings are still restored on exit.

A dedicated local accessibility Simulator is deliberately shut down before the
corrected controller fragment. Existing derived data is reused; all actual boot,
setting, focused xcodebuild and restoration commands execute. Raw evidence lives at
build/phase-11-iphone-xr/signing-ready/remaining-boot-controller/ (execution.log,
methodology.json, large-text-setting.txt and large-text.xcresult).
Both focused shutdown runs PASS. The distinct-size run records initial size large,
requested accessibility-extra-extra-extra-large, successful bilingual XCTest and
restored size large; the dedicated Simulator original size is then restored too.
Its raw evidence is in remaining-boot-controller-distinct-size beside the first run.
The final source suite passes208/208; final successor CI is delivered separately. Additive phase-11-source-v12.json
binds this controller correction to the immutable v11 inventory and v10 source chain.

Full fresh Foundation7/7 and Native3/3 are required on the corrected successor.
Shared native core, ABI, Schema3, numerics, GPU contracts, production app source,
test behavior assertions and signing assets are unchanged by this correction.
DH-APPLE-METAL-01 PASS/CLOSED accepted scope unchanged; CPU VALIDATED and
Metal/Vulkan provider UNKNOWN. G7/G8/G10/G11 external gates remain open as listed
in phase-11-remaining-engineering.md; this is not phase acceptance or closure.
