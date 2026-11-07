# Phase 10.1 Step 2 — Local Apple native and iOS validation

Status: LOCAL HARDWARE GATE BLOCKED.

No local Mac Studio build or Simulator test was run. No Xcode version, no
simulator runtime, and no local test count is claimed.

When a Darwin host has the pinned developer directory, the existing commands
are:

```
bash scripts/check-apple.sh
bash scripts/check-ios-shell.sh
```

`check-apple.sh` configures `build/apple-host` with the Xcode `clang` and
`clang++`, runs ctest, runs the Swift package tests, builds `PixAuraCore`
for generic iOS with `CODE_SIGNING_ALLOWED=NO`, and tests that scheme on the
selected iPhone simulator. `check-ios-shell.sh` builds the PixAuraAI app
Debug and Release for generic iOS without signing, scans the release app,
and runs the app tests on the simulator. Those builds are unsigned simulator
and generic-iOS software builds. They are not signed physical-device builds.

Hosted Apple CI remains separate evidence. It does not satisfy this step.
The Mac2 family query now uses raw value 2002 so an Xcode 27.2 build does
not name the deprecated enumerator. That source change does not admit
Apple7-only devices and does not remove the software and virtual name
rejection. This Windows session did not compile it with Xcode 27.2. The Mac
Studio still runs `bash scripts/check-apple.sh` and
`bash scripts/check-ios-shell.sh` after a fast-forward pull.
`f7a61a019295772be700a7771083ffa612c98565` is not certified. Hosted
Xcode rejected the optional Mac2 raw value, and the Android review order
was still 101,102 before the required 102,101 move. That SHA is not rerun.
The Swift package's vendored codec target ignores only
`-Wshorten-64-to-32` on Apple platforms. Vendor JPEG, PNG, and zlib bytes
are unchanged. Owned C++ targets and the CMake `-Werror` gate on
`decode_codec.c` do not receive that exemption. Hosted Xcode 26.6 does not
prove the Xcode 27.2 compile.
