#!/usr/bin/env bash
# Print the Mac host toolchain. This script does not install software, change
# xcode-select, clone a repository, or copy a Windows build directory.
set -euo pipefail

if [[ "$(uname -s)" != "Darwin" ]]; then
  printf 'PixAuraAI Mac host check requires Darwin. This process reports: %s %s\n' "$(uname -s)" "$(uname -m)"
  exit 2
fi

expected_developer_dir="/Applications/Xcode_26.6.app/Contents/Developer"
selected_developer_dir="$(xcode-select -p)"

printf 'sw_vers:\n'
sw_vers
printf 'uname: %s\n' "$(uname -m)"
printf 'hardware: %s\n' "$(sysctl -n machdep.cpu.brand_string)"
printf 'xcode-select: %s\n' "$selected_developer_dir"
if [[ "$selected_developer_dir" != "$expected_developer_dir" ]]; then
  printf 'Selected developer directory differs from the CI pin %s. This script does not change it.\n' "$expected_developer_dir"
fi
xcodebuild -version
xcrun swift --version
cmake --version
git --version
node --version
printf 'simctl devices:\n'
xcrun simctl list devices available
printf 'disk:\n'
df -h .
