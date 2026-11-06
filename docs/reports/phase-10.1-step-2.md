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
