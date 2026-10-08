#!/usr/bin/env bash
set -euo pipefail
workspace_path="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$workspace_path"
mkdir -p build/gpu-metal-compile
for sdk in macosx iphoneos iphonesimulator; do
    xcrun --sdk "$sdk" metal -std=metal2.4 -fno-fast-math -Werror -c \
        packages/core/swift/Sources/Shaders/exposure.metal \
        -o "build/gpu-metal-compile/$sdk.air"
    xcrun --sdk "$sdk" metallib "build/gpu-metal-compile/$sdk.air" \
        -o "build/gpu-metal-compile/$sdk.metallib"
done
echo 'Metal shader compilation PASS (3 SDKs); hardware certification not executed'
