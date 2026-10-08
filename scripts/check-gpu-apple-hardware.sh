#!/usr/bin/env bash
set -euo pipefail
workspace_path="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$workspace_path"
destination="${1:?Usage: bash scripts/check-gpu-apple-hardware.sh 'platform=iOS,name=DEVICE_NAME' [EXISTING_TEAM_ID]}"
if [[ "$destination" != platform=iOS,* || "$destination" == *Simulator* ]]; then
    echo 'UNSUPPORTED: connected physical iPhone/iPad destination required' >&2
    exit 2
fi
mkdir -p build/gpu-apple-hardware
result_directory="$(mktemp -d "$workspace_path/build/gpu-apple-hardware/run.XXXXXX")"
signing=()
if [[ -n "${2:-}" ]]; then signing+=("DEVELOPMENT_TEAM=$2"); fi
# Uses existing local signing/provisioning. Does not create credentials or ask
# Xcode to provision/change an account. The owner authorizes signing separately.
git rev-parse HEAD > "$result_directory/commit.txt"
xcodebuild -project platforms/ios/PixAuraAI.xcodeproj -scheme PixAuraAI \
    -configuration Debug -destination "$destination" \
    -derivedDataPath "$workspace_path/build/gpu-apple-hardware/derived" \
    -resultBundlePath "$result_directory/hardware.xcresult" \
    -only-testing:PixAuraAITests/GpuHardwareTests/testPhysicalMetalCertification \
    'KEEP_PRIVATE_EXTERNS=$(PIXAURA_PRIVATE_EXTERNS_$(TARGET_NAME))' \
    PIXAURA_PRIVATE_EXTERNS_CPixAuraSQLite=YES PIXAURA_PRIVATE_EXTERNS_CPixAuraCodecs=YES \
    "${signing[@]}" test | tee "$result_directory/xcodebuild.log"
echo "Evidence: $result_directory (JSON XCTest attachment and human-readable log)"
