#!/usr/bin/env bash
set -euo pipefail

workspace_path="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$workspace_path"
clang --version
cmake --version
cmake -S . -B build/sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug \
    -DPIXAURA_SANITIZERS=ON -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build build/sanitize
ctest --test-dir build/sanitize --output-on-failure -V
