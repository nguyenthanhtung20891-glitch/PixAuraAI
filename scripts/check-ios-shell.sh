#!/usr/bin/env bash
set -euo pipefail
workspace_path="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$workspace_path"
mkdir -p build
xcodebuild -version
xcrun simctl list devices available --json > build/ios-shell-devices.json
simulator_id="$(node scripts/select-ios-simulator.mjs build/ios-shell-devices.json)"
source scripts/ios-simulator-architecture.sh
for configuration in Debug Release; do
    xcodebuild -project platforms/ios/PixAuraAI.xcodeproj -scheme PixAuraAI \
        -configuration "$configuration" -destination 'generic/platform=iOS' \
        -derivedDataPath "$workspace_path/build/ios-shell-device" \
        'KEEP_PRIVATE_EXTERNS=$(PIXAURA_PRIVATE_EXTERNS_$(TARGET_NAME))' \
        PIXAURA_PRIVATE_EXTERNS_CPixAuraSQLite=YES PIXAURA_PRIVATE_EXTERNS_CPixAuraCodecs=YES PIXAURA_PRIVATE_EXTERNS_CPixAuraSodium=YES PIXAURA_PRIVATE_EXTERNS_CPixAuraMetal=YES CODE_SIGNING_ALLOWED=NO build
done
node scripts/check-model-release.mjs "$workspace_path/build/ios-shell-device/Build/Products/Release-iphoneos/PixAuraAI.app/PixAuraAI"
python3 tests/apple-release-scanner.test.py
python3 scripts/check-apple-release.py "$workspace_path/build/ios-shell-device/Build/Products/Release-iphoneos/PixAuraAI.app"
result_directory="$(mktemp -d "$workspace_path/build/ios-shell-results.XXXXXX")"
xcodebuild -project platforms/ios/PixAuraAI.xcodeproj -scheme PixAuraAI \
    -configuration Debug -destination "platform=iOS Simulator,id=$simulator_id,arch=$simulator_arch" \
    -derivedDataPath "$workspace_path/build/ios-shell-simulator" \
    -resultBundlePath "$result_directory/tests.xcresult" \
    'KEEP_PRIVATE_EXTERNS=$(PIXAURA_PRIVATE_EXTERNS_$(TARGET_NAME))' \
    PIXAURA_PRIVATE_EXTERNS_CPixAuraSQLite=YES PIXAURA_PRIVATE_EXTERNS_CPixAuraCodecs=YES PIXAURA_PRIVATE_EXTERNS_CPixAuraSodium=YES PIXAURA_PRIVATE_EXTERNS_CPixAuraMetal=YES \
    ARCHS="$simulator_arch" ONLY_ACTIVE_ARCH=YES EXCLUDED_ARCHS= CODE_SIGNING_ALLOWED=NO test

# A test name is not proof of the actual Dynamic Type environment. Preserve the
# selected simulator's setting and explicitly exercise the largest supported size.
previous_content_size="$(xcrun simctl ui "$simulator_id" content_size)"
restore_content_size() {
    xcrun simctl ui "$simulator_id" content_size "$previous_content_size"
}
trap restore_content_size EXIT
xcrun simctl ui "$simulator_id" content_size accessibility-extra-extra-extra-large
xcrun simctl ui "$simulator_id" content_size > "$result_directory/large-text-setting.txt"
xcodebuild -project platforms/ios/PixAuraAI.xcodeproj -scheme PixAuraAI \
    -configuration Debug -destination "platform=iOS Simulator,id=$simulator_id,arch=$simulator_arch" \
    -derivedDataPath "$workspace_path/build/ios-shell-simulator" \
    -resultBundlePath "$result_directory/large-text.xcresult" \
    -only-testing:PixAuraAIUITests/ShellUITests/testPhase11BilingualExportAtAccessibilityTextSize \
    'KEEP_PRIVATE_EXTERNS=$(PIXAURA_PRIVATE_EXTERNS_$(TARGET_NAME))' \
    PIXAURA_PRIVATE_EXTERNS_CPixAuraSQLite=YES PIXAURA_PRIVATE_EXTERNS_CPixAuraCodecs=YES PIXAURA_PRIVATE_EXTERNS_CPixAuraSodium=YES PIXAURA_PRIVATE_EXTERNS_CPixAuraMetal=YES \
    ARCHS="$simulator_arch" ONLY_ACTIVE_ARCH=YES EXCLUDED_ARCHS= CODE_SIGNING_ALLOWED=NO test
