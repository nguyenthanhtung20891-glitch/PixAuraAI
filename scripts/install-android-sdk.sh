#!/usr/bin/env bash
set -euo pipefail

# Hosted provisioning only. The same pinned packages remain mandatory;
# transient download/archive failures never permit later gates to be skipped.
manager="${ANDROID_HOME:?ANDROID_HOME required}/cmdline-tools/latest/bin/sdkmanager"
for attempt in 1 2 3; do
  if "$manager" "platforms;android-36" "build-tools;35.0.0" "ndk;28.2.13676358" "cmake;3.22.1" "system-images;android-35;google_apis;x86_64"; then
    exit 0
  fi
  printf 'SDK provisioning attempt %s failed\n' "$attempt" >&2
  if (( attempt < 3 )); then sleep 2; fi
done
printf 'SDK provisioning failed after three attempts\n' >&2
exit 1
