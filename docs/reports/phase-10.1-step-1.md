# Phase 10.1 Step 1 — Mac Studio development environment

Status: LOCAL HARDWARE GATE BLOCKED.

The preparing session was Windows. `sw_vers`, `xcodebuild`, and
`xcrun simctl` were not executed here. No Mac Studio hostname, user path, or
credential was recorded. A script in the repository is not evidence that a
Mac host has run it.

## Commands for the Mac clone

Choose a macOS path outside any Windows checkout. Clone from GitHub. Do not
copy `build/` or a Windows dependency cache.

```
git clone https://github.com/nguyenthanhtung20891-glitch/PixAuraAI.git
cd PixAuraAI
git checkout main
bash scripts/check-mac-studio-host.sh
```

The host check prints `sw_vers`, `uname -m`, the CPU brand string,
`xcode-select -p`, `xcodebuild -version`, `swift --version`, `cmake --version`,
`git --version`, `node --version`, available simulators, and `df -h .`.
It exits 2 on a non-Darwin process. If the selected developer directory is
not `/Applications/Xcode_26.6.app/Contents/Developer`, it reports the
difference and leaves `xcode-select` unchanged.

Install the Xcode 26.6 bundle used by the hosted jobs before expecting that
path to exist. CMake must satisfy `cmake_minimum_required(VERSION 3.22)`.
The repository does not pin a Node version. Select the newest available
iPhone simulator with `scripts/select-ios-simulator.mjs` when the existing
Apple scripts run. Do not install an unreviewed newer Xcode over that pin
just to begin this phase.
