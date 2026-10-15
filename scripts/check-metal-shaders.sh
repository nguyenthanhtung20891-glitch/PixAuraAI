#!/usr/bin/env bash
set -euo pipefail
workspace_path="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$workspace_path"
mkdir -p build/gpu-metal-compile
for sdk in macosx iphoneos iphonesimulator; do
    standard=ios-metal2.4
    if [[ "$sdk" == macosx ]]; then standard=macos-metal2.4; fi
    for shader in exposure segmentation; do
        suffix="$sdk"
        if [[ "$shader" != exposure ]]; then suffix="$sdk-$shader"; fi
        xcrun --sdk "$sdk" metal "-std=$standard" -fno-fast-math -Werror -c \
            "packages/core/swift/Sources/Shaders/$shader.metal" -o "build/gpu-metal-compile/$suffix.air"
        xcrun --sdk "$sdk" metallib "build/gpu-metal-compile/$suffix.air" -o "build/gpu-metal-compile/$suffix.metallib"
    done
done
echo 'Metal shader compilation PASS (3 SDKs); hardware certification not executed'
