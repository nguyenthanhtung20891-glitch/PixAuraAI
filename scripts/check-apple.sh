#!/usr/bin/env bash
set -euo pipefail

workspace_path="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$workspace_path"
xcodebuild -version
xcrun swift --version
cmake --version
node --test tests/foundation.test.mjs tests/ci-tools.test.mjs tests/document-contract.test.mjs tests/native-document.test.mjs tests/persistence.test.mjs tests/decode.test.mjs tests/working.test.mjs tests/evaluation.test.mjs tests/geometry.test.mjs tests/geometry-execution.test.mjs
cmake -S . -B build/apple-host -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER="$(xcrun --find clang)" \
    -DCMAKE_CXX_COMPILER="$(xcrun --find clang++)"
cmake --build build/apple-host
ctest --test-dir build/apple-host --output-on-failure
swift test --package-path packages/core --scratch-path "$workspace_path/build/apple-swift" \
    -Xswiftc -warnings-as-errors -Xcc -Werror

xcrun simctl list devices available --json > build/apple-devices.json
simulator_id="$(node scripts/select-ios-simulator.mjs build/apple-devices.json)"
source scripts/ios-simulator-architecture.sh
result_directory="$(mktemp -d "$workspace_path/build/apple-results.XXXXXX")"
cd packages/core
# Library/package schemes only: no app, store credentials or signing required.
# Preserve hidden SQLite externs through Xcode's package-target ld -r stages.
xcodebuild -scheme PixAuraCore -destination 'generic/platform=iOS' \
    -derivedDataPath "$workspace_path/build/apple-ios-device" \
    'KEEP_PRIVATE_EXTERNS=$(PIXAURA_PRIVATE_EXTERNS_$(TARGET_NAME))' \
    PIXAURA_PRIVATE_EXTERNS_CPixAuraSQLite=YES PIXAURA_PRIVATE_EXTERNS_CPixAuraCodecs=YES CODE_SIGNING_ALLOWED=NO build
xcodebuild -scheme PixAuraCore -destination "platform=iOS Simulator,id=$simulator_id,arch=$simulator_arch" \
    -derivedDataPath "$workspace_path/build/apple-ios-simulator" \
    -resultBundlePath "$result_directory/ios-tests.xcresult" \
    'KEEP_PRIVATE_EXTERNS=$(PIXAURA_PRIVATE_EXTERNS_$(TARGET_NAME))' \
    PIXAURA_PRIVATE_EXTERNS_CPixAuraSQLite=YES PIXAURA_PRIVATE_EXTERNS_CPixAuraCodecs=YES \
    ARCHS="$simulator_arch" ONLY_ACTIVE_ARCH=YES EXCLUDED_ARCHS= CODE_SIGNING_ALLOWED=NO test
