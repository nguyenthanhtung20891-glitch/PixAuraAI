#!/usr/bin/env bash
set -euo pipefail

workspace_path="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$workspace_path"
: "${ANDROID_HOME:?Set ANDROID_HOME to the installed Android SDK}"
evidence="$workspace_path/build/android-emulator-evidence"
mkdir -p "$evidence"
adb="$ANDROID_HOME/platform-tools/adb"
emulator="$ANDROID_HOME/emulator/emulator"
avdmanager="$ANDROID_HOME/cmdline-tools/latest/bin/avdmanager"
export ANDROID_AVD_HOME="$workspace_path/build/android-ci-avd"
export ANDROID_SERIAL=emulator-5554
mkdir -p "$ANDROID_AVD_HOME"

device_timeout="${ANDROID_DEVICE_TIMEOUT_SECONDS:-180}"
boot_timeout="${ANDROID_BOOT_TIMEOUT_SECONDS:-600}"
package_timeout="${ANDROID_PACKAGE_TIMEOUT_SECONDS:-120}"
test_timeout="${ANDROID_TEST_TIMEOUT_SECONDS:-900}"
command_timeout="${ANDROID_COMMAND_TIMEOUT_SECONDS:-10}"
resolution_timeout="${ANDROID_RESOLUTION_TIMEOUT_SECONDS:-120}"
poll_interval="${ANDROID_POLL_INTERVAL_SECONDS:-2}"
for budget in "$device_timeout" "$boot_timeout" "$package_timeout" "$test_timeout" "$command_timeout" "$resolution_timeout"; do
    if [[ ! "$budget" =~ ^[1-9][0-9]*$ ]]; then
        echo "Timeout budgets must be positive integer seconds" >&2
        exit 2
    fi
done
if [[ ! "$poll_interval" =~ ^([01]([.][0-9]+)?|2([.]0+)?)$ ]] || [[ "$poll_interval" =~ ^0+([.]0+)?$ ]]; then
    echo "Polling interval must be greater than 0 and at most 2 seconds" >&2
    exit 2
fi

emulator_pid=""
phase=setup
touch "$evidence/emulator.stdout.log" "$evidence/emulator.stderr.log"
adb_command() { timeout --kill-after=5s "${command_timeout}s" "$adb" "$@"; }
diagnostics() {
    if [[ -n "${avd_config:-}" && -f "$avd_config" ]]; then cp "$avd_config" "$evidence/avd-config.final.ini"; fi
    if [[ -n "${avd_directory:-}" && -f "$avd_directory/hardware-qemu.ini" ]]; then
        cp "$avd_directory/hardware-qemu.ini" "$evidence/hardware-qemu.final.ini"
    fi
    echo "$phase" > "$evidence/final-phase.txt"
    adb_command devices -l > "$evidence/adb-devices.txt" 2>&1
    adb_command -s "$ANDROID_SERIAL" shell getprop > "$evidence/boot-properties.txt" 2>&1
    adb_command -s "$ANDROID_SERIAL" shell pm path android > "$evidence/package-manager.txt" 2>&1
    adb_command -s "$ANDROID_SERIAL" logcat -d -t 200 > "$evidence/logcat.txt" 2>&1
    if [[ -n "$emulator_pid" ]]; then
        ps -p "$emulator_pid" -o pid=,stat=,args= > "$evidence/emulator-process.txt" 2>&1
    fi
}
cleanup() {
    local result=$?
    trap - EXIT
    set +e
    diagnostics
    # Preserve the build/test result even when the process already exited.
    if [[ -n "$emulator_pid" ]] && kill -0 "$emulator_pid" 2>/dev/null; then
        kill "$emulator_pid" 2>/dev/null
        local deadline=$((SECONDS + 10))
        while kill -0 "$emulator_pid" 2>/dev/null && (( SECONDS < deadline )); do sleep 1; done
        if kill -0 "$emulator_pid" 2>/dev/null; then kill -KILL "$emulator_pid" 2>/dev/null; fi
    fi
    if [[ -n "$emulator_pid" ]]; then wait "$emulator_pid" 2>/dev/null; fi
    printf '%s\n' "$result" > "$evidence/exit-code.txt"
    echo "Android emulator phase '$phase' finished with exit $result; evidence: $evidence"
    if (( result != 0 )); then
        if [[ -f "$evidence/tool-preflight.log" ]]; then cat "$evidence/tool-preflight.log" >&2; fi
        if [[ -f "$evidence/emulator-version.stderr.log" ]]; then cat "$evidence/emulator-version.stderr.log" >&2; fi
        if [[ -f "$evidence/emulator-resolution.log" ]]; then tail -n 40 "$evidence/emulator-resolution.log" >&2; fi
        tail -n 40 "$evidence/emulator.stdout.log" "$evidence/emulator.stderr.log" >&2
        cat "$evidence/adb-devices.txt" "$evidence/boot-properties.txt" >&2
    fi
    exit "$result"
}
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

wait_for() {
    phase="$1"
    local budget="$2"
    shift 2
    local started=$SECONDS
    local deadline=$((started + budget))
    local attempt=0
    echo "Waiting for $phase (budget ${budget}s)" | tee -a "$evidence/readiness.log"
    while (( SECONDS < deadline )); do
        if ! kill -0 "$emulator_pid" 2>/dev/null; then
            local emulator_result=0
            wait "$emulator_pid" || emulator_result=$?
            echo "Emulator exited with $emulator_result before $phase; inspect emulator stdout/stderr" >&2
            return 1
        fi
        attempt=$((attempt + 1))
        if "$@"; then
            echo "$phase ready after $attempt probe(s)" | tee -a "$evidence/readiness.log"
            return 0
        fi
        echo "$phase: probe $attempt not ready ($((SECONDS - started))s elapsed)" | tee -a "$evidence/readiness.log"
        if (( SECONDS >= deadline )); then break; fi
        sleep "$poll_interval"
    done
    echo "Timed out waiting for $phase after ${budget}s" >&2
    return 124
}
device_visible() {
    local state
    state="$(adb_command -s "$ANDROID_SERIAL" get-state 2>> "$evidence/adb-probes.stderr.log")" || return 1
    printf '%s\n' "$state" > "$evidence/device-state.txt"
    [[ "${state//$'\r'/}" == device ]]
}
boot_complete() {
    local property
    property="$(adb_command -s "$ANDROID_SERIAL" shell getprop sys.boot_completed 2>> "$evidence/adb-probes.stderr.log")" || return 1
    printf '%s\n' "$property" > "$evidence/boot-completed.txt"
    [[ "${property//$'\r'/}" == 1 ]]
}
package_ready() {
    local package_path
    package_path="$(adb_command -s "$ANDROID_SERIAL" shell pm path android 2>> "$evidence/adb-probes.stderr.log")" || return 1
    printf '%s\n' "$package_path" > "$evidence/package-probe.txt"
    [[ "$package_path" == package:* ]]
}

phase=tool-preflight
echo "PATH=$PATH" | tee "$evidence/tool-preflight.log"
for required_command in timeout node mke2fs df awk mktemp ls; do
    if ! resolved_command="$(command -v "$required_command")" || [[ ! -x "$resolved_command" ]]; then
        echo "Required tool '$required_command' is unavailable on PATH; disk preparation cannot proceed" | tee -a "$evidence/tool-preflight.log" >&2
        exit 2
    fi
    echo "$required_command=$resolved_command" | tee -a "$evidence/tool-preflight.log"
done
node_command="$(command -v node)"
mke2fs_command="$(command -v mke2fs)"
for sdk_command in "$adb" "$avdmanager" "$emulator" "$ANDROID_HOME/emulator/qemu-img"; do
    if [[ ! -x "$sdk_command" ]]; then
        echo "Required SDK tool '$sdk_command' is missing or not executable; disk preparation cannot proceed" | tee -a "$evidence/tool-preflight.log" >&2
        exit 2
    fi
    echo "SDK tool=$sdk_command" | tee -a "$evidence/tool-preflight.log"
done
# Select the SDK's headless engine even for version inspection. The GUI engine
# requires libpulse.so.0 on hosts where the headless test engine does not.
version_result=0
timeout --kill-after=5s 10s "$emulator" -no-window -version \
    > "$evidence/emulator-version.stdout.log" 2> "$evidence/emulator-version.stderr.log" || version_result=$?
cat "$evidence/emulator-version.stdout.log" "$evidence/emulator-version.stderr.log" > "$evidence/emulator-version.txt"
if (( version_result != 0 )); then
    echo "Required SDK emulator '$emulator' failed headless version inspection (exit $version_result); inspect emulator-version.stderr.log for missing runtime libraries" | tee -a "$evidence/tool-preflight.log" >&2
    exit 1
fi

phase=adb-server-start
timeout --kill-after=5s 30s "$adb" start-server > "$evidence/adb-server.log" 2>&1
phase=device-list-before-launch
adb_command devices -l > "$evidence/adb-devices-before.txt" 2>&1
if grep -q "^${ANDROID_SERIAL}[[:space:]]" "$evidence/adb-devices-before.txt"; then
    echo "Refusing to use an existing $ANDROID_SERIAL; the CI emulator must own port 5554" >&2
    exit 1
fi
phase=avd-creation
printf 'no\n' | timeout --kill-after=5s 60s "$avdmanager" create avd --force --name pixaura-ci-shell \
    --package 'system-images;android-35;google_apis;x86_64' > "$evidence/avd-create.log" 2>&1

# Explicit writable disk sizes; do not inherit the SDK userdata default.
userdata_mib=2048
cache_mib=128
reserve_mib=2048
avd_config="$ANDROID_AVD_HOME/pixaura-ci-shell.avd/config.ini"
avd_directory="$(dirname "$avd_config")"
cp "$avd_config" "$evidence/avd-config.created.ini"
sed -i -E '/^(disk\.dataPartition\.size|disk\.cachePartition\.(size|yes)|hw\.sdCard|sdcard\.(size|path))=/d' "$avd_config"
printf '%s\n' "disk.dataPartition.size=${userdata_mib}M" \
    "disk.cachePartition=yes" "disk.cachePartition.size=${cache_mib}M" "hw.sdCard=no" >> "$avd_config"
cp "$avd_config" "$evidence/avd-config.ini"

phase=disk-input-evidence
image_directory="$ANDROID_HOME/system-images/android-35/google_apis/x86_64"
for metadata in source.properties package.xml advancedFeatures.ini; do
    if [[ -f "$image_directory/$metadata" ]]; then cp "$image_directory/$metadata" "$evidence/system-image.$metadata"; fi
done
if [[ -f "$ANDROID_HOME/emulator/source.properties" ]]; then
    cp "$ANDROID_HOME/emulator/source.properties" "$evidence/emulator.source.properties"
fi
cat "$avd_config" "$evidence/emulator-version.txt"
if [[ -f "$avd_directory/hardware-qemu.ini" ]]; then
    cp "$avd_directory/hardware-qemu.ini" "$evidence/hardware-qemu.before.ini"
else
    echo 'hardware-qemu.ini not yet generated' > "$evidence/hardware-qemu.before.txt"
fi
if [[ -f "$image_directory/userdata.img" ]]; then
    timeout --kill-after=5s 10s "$ANDROID_HOME/emulator/qemu-img" info --output=json "$image_directory/userdata.img" \
        > "$evidence/initial-userdata.json"
else
    echo 'No initial userdata.img; recording system image data directory' > "$evidence/initial-userdata.txt"
    timeout --kill-after=5s 10s ls -la "$image_directory/data" >> "$evidence/initial-userdata.txt"
fi

phase=disk-preflight
required_mib=$((userdata_mib + cache_mib + reserve_mib))
check_disk() {
    local available_kib
    available_kib="$(timeout --kill-after=5s 10s df -Pk "$ANDROID_AVD_HOME" | awk 'NR == 2 {print $4}')"
    if [[ ! "$available_kib" =~ ^[0-9]+$ ]]; then
        echo "Cannot determine free disk space for $ANDROID_AVD_HOME" >&2
        return 2
    fi
    free_mib=$((available_kib / 1024))
    echo "Free disk: ${free_mib} MiB; configured userdata: ${userdata_mib} MiB; cache: ${cache_mib} MiB; reserve: ${reserve_mib} MiB; required: ${required_mib} MiB" | tee -a "$evidence/disk-preflight.log"
}
echo "AVD disk settings ($avd_config):" | tee -a "$evidence/disk-preflight.log"
grep -E '^(disk\.|hw.sdCard|sdcard\.)' "$avd_config" | tee -a "$evidence/disk-preflight.log"
check_disk
if (( free_mib < required_mib )); then
    # One bounded cleanup of disposable workspace output on CI only. Keep SDKs,
    # APKs, reports and dependency caches; reject redirected/symlinked parents.
    if [[ "${CI:-}" == true && "${GITHUB_ACTIONS:-}" == true ]]; then
        cleanup_path="$(realpath -m "$workspace_path/platforms/android/app/build/tmp")"
        if [[ "$cleanup_path" == "$workspace_path/platforms/android/app/build/tmp" ]]; then
            echo "Low disk: cleaning CI workspace temporary output: $cleanup_path" | tee -a "$evidence/disk-preflight.log"
            timeout --kill-after=5s 30s rm -rf -- "$cleanup_path"
        else
            echo "Refusing cleanup outside the expected workspace path: $cleanup_path" | tee -a "$evidence/disk-preflight.log"
        fi
        check_disk
    fi
    if (( free_mib < required_mib )); then
        echo "Insufficient disk space before emulator launch: ${free_mib} MiB free, ${required_mib} MiB required; SDK components preserved" >&2
        exit 1
    fi
fi

phase=userdata-preparation
# API 24+ firstTimeSetup (wipe-data OR missing runtime image) clamps userdata
# to 6 GiB inside QEMU2, after command-line parsing. Use documented -data with
# a newly created factory-empty ext4 image, not -wipe-data or a missing file.
# Only images that explicitly declare factory-empty data are supported here.
if [[ ! -f "$image_directory/data/empty_data_disk" ]]; then
    echo 'System image does not declare empty_data_disk; refusing to invent factory userdata' >&2
    exit 1
fi
cp "$image_directory/data/empty_data_disk" "$evidence/empty_data_disk.txt"
timeout --kill-after=5s 10s "$mke2fs_command" -V > "$evidence/mke2fs-version.txt" 2>&1
userdata_path="$(mktemp "$avd_directory/ci-userdata.XXXXXX.img")"
timeout --kill-after=5s 60s "$mke2fs_command" -q -F -t ext4 -b 4096 -m 0 \
    -L data "$userdata_path" "$((userdata_mib * 256))" > "$evidence/userdata-create.log" 2>&1
"$node_command" scripts/assert-android-userdata.mjs "$userdata_path" "$userdata_mib" > "$evidence/userdata-prepared.json"
cat "$evidence/userdata-prepared.json"

phase=acceleration-probe
acceleration=off
if [[ -c /dev/kvm ]]; then
    if [[ ! -r /dev/kvm || ! -w /dev/kvm ]]; then
        timeout --kill-after=5s 10s sudo -n chmod a+rw /dev/kvm >> "$evidence/acceleration.log" 2>&1 || true
    fi
    if [[ -r /dev/kvm && -w /dev/kvm ]] && \
        timeout --kill-after=5s 10s "$emulator" -accel-check >> "$evidence/acceleration.log" 2>&1; then
        acceleration=on
    fi
fi
echo "Emulator acceleration: $acceleration (software fallback when KVM is unavailable)" | tee -a "$evidence/acceleration.log"
emulator_arguments=(-avd pixaura-ci-shell -port 5554 -accel "$acceleration" -memory 2048 -cores 2
    -partition-size "$userdata_mib" -cache-size "$cache_mib" -data "$userdata_path" -datadir "$avd_directory"
    -no-window -no-audio -no-boot-anim -no-snapshot -gpu swiftshader_indirect -verbose)
printf '%q ' "$emulator" "${emulator_arguments[@]}" > "$evidence/emulator-arguments.txt"
printf '\n' >> "$evidence/emulator-arguments.txt"
cat "$evidence/emulator-arguments.txt"

phase=userdata-resolution
# Documented SDK diagnostic option resolves hardware and exits without running
# the guest CPU. The absent snapshot is intentional, not a test skip.
printf '%q ' "$emulator" "${emulator_arguments[@]}" -check-snapshot-loadable pixaura-ci-resolution \
    > "$evidence/emulator-resolution-arguments.txt"
printf '\n' >> "$evidence/emulator-resolution-arguments.txt"
timeout --kill-after=5s "${resolution_timeout}s" "$emulator" "${emulator_arguments[@]}" \
    -check-snapshot-loadable pixaura-ci-resolution > "$evidence/emulator-resolution.log" 2>&1
cp "$avd_directory/hardware-qemu.ini" "$evidence/hardware-qemu.resolved.ini"
cp "$avd_config" "$evidence/avd-config.resolved.ini"
timeout --kill-after=5s 10s "$ANDROID_HOME/emulator/qemu-img" info --output=json "$userdata_path" \
    > "$evidence/userdata-qemu-info.json"
"$node_command" scripts/assert-android-userdata.mjs "$userdata_path" "$userdata_mib" "$evidence/hardware-qemu.resolved.ini" "$evidence/userdata-qemu-info.json" \
    > "$evidence/userdata-resolved.json"
cat "$evidence/userdata-resolved.json"

phase=launch
"$emulator" "${emulator_arguments[@]}" \
    > "$evidence/emulator.stdout.log" 2> "$evidence/emulator.stderr.log" &
emulator_pid=$!

wait_for device-visibility "$device_timeout" device_visible
phase=userdata-running-assertion
running_image="$userdata_path"
if [[ -f "$userdata_path.qcow2" ]]; then running_image="$userdata_path.qcow2"; fi
timeout --kill-after=5s 10s "$ANDROID_HOME/emulator/qemu-img" info -U --output=json "$running_image" \
    > "$evidence/userdata-running-qemu-info.json"
"$node_command" scripts/assert-android-userdata.mjs "$userdata_path" "$userdata_mib" "$avd_directory/hardware-qemu.ini" \
    "$evidence/userdata-running-qemu-info.json" \
    > "$evidence/userdata-running.json"
wait_for boot-completion "$boot_timeout" boot_complete
wait_for package-manager "$package_timeout" package_ready
phase=unlock
adb_command -s "$ANDROID_SERIAL" shell input keyevent 82
phase=instrumentation
echo "Executing JNI/Compose tests (independent budget ${test_timeout}s)"
cd platforms/android
timeout --kill-after=10s "${test_timeout}s" bash gradlew --no-daemon --dependency-verification strict \
    connectedDebugAndroidTest 2>&1 | tee "$evidence/instrumentation.log"
phase=complete
