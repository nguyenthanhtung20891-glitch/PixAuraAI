#!/usr/bin/env bash
set -euo pipefail
workspace_path="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$workspace_path"
mkdir -p build/gpu-metal-compile
for sdk in macosx iphoneos iphonesimulator; do
    standard=ios-metal2.4
    if [[ "$sdk" == macosx ]]; then standard=macos-metal2.4; fi
    xcrun --sdk "$sdk" metal "-std=$standard" -fno-fast-math -Werror -c \
        packages/core/swift/Sources/Shaders/exposure.metal \
        -o "build/gpu-metal-compile/$sdk.air"
    xcrun --sdk "$sdk" metallib "build/gpu-metal-compile/$sdk.air" \
        -o "build/gpu-metal-compile/$sdk.metallib"
done
echo 'Metal shader compilation PASS (3 SDKs); hardware certification not executed'
