#!/usr/bin/env bash
set -euo pipefail
workspace_path="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$workspace_path"
mkdir -p build
xcodebuild -version
xcrun simctl list devices available --json > build/ios-shell-devices.json
simulator_id="$(node scripts/select-ios-simulator.mjs build/ios-shell-devices.json)"
for configuration in Debug Release; do
    xcodebuild -project platforms/ios/PixAuraAI.xcodeproj -scheme PixAuraAI \
        -configuration "$configuration" -destination 'generic/platform=iOS' \
        -derivedDataPath "$workspace_path/build/ios-shell-device" CODE_SIGNING_ALLOWED=NO build
done
result_directory="$(mktemp -d "$workspace_path/build/ios-shell-results.XXXXXX")"
xcodebuild -project platforms/ios/PixAuraAI.xcodeproj -scheme PixAuraAI \
    -configuration Debug -destination "platform=iOS Simulator,id=$simulator_id" \
    -derivedDataPath "$workspace_path/build/ios-shell-simulator" \
    -resultBundlePath "$result_directory/tests.xcresult" CODE_SIGNING_ALLOWED=NO test
