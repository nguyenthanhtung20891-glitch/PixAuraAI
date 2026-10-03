#!/usr/bin/env bash
# Source from the repository root. Apply these settings only to simulator builds.
simulator_arch="$(uname -m)"
case "$simulator_arch" in
    arm64|x86_64) ;;
    *) echo "Unsupported macOS simulator host architecture: $simulator_arch" >&2; return 1 ;;
esac
echo "Simulator build architecture: $simulator_arch"
