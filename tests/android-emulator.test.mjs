// Linux CI orchestration tests use executable fake SDK and Gradle commands.
import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { spawnSync } from 'node:child_process';
import { fileURLToPath } from 'node:url';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
function runScenario(scenario) {
  const directory = fs.mkdtempSync(path.join(os.tmpdir(), 'pixaura emulator-'));
  const write = (file, source) => {
    const target = path.join(directory, file);
    fs.mkdirSync(path.dirname(target), { recursive: true });
    fs.writeFileSync(target, source, { mode: 0o755 });
  };
  fs.mkdirSync(path.join(directory, 'state'));
  write('scripts/check-android-emulator.sh', fs.readFileSync(path.join(root, 'scripts/check-android-emulator.sh')));
  write('scripts/assert-android-userdata.mjs', fs.readFileSync(path.join(root, 'scripts/assert-android-userdata.mjs')));
  write('sdk/system-images/android-35/google_apis/x86_64/data/empty_data_disk', 'factory-empty userdata marker');
  if (scenario === 'unsupported-factory-data') {
    fs.unlinkSync(path.join(directory, 'sdk/system-images/android-35/google_apis/x86_64/data/empty_data_disk'));
  }
  write('sdk/system-images/android-35/google_apis/x86_64/source.properties', 'Pkg.Revision=9\nAndroidVersion.ApiLevel=35\n');
  write('bin/sudo', '#!/usr/bin/env bash\nexit 1\n'); // Never change a real host's /dev/kvm permissions.
  write('sdk/cmdline-tools/latest/bin/avdmanager', `#!/usr/bin/env bash
mkdir -p "$ANDROID_AVD_HOME/pixaura-ci-shell.avd"
printf 'disk.dataPartition.size=6G\\ndisk.cachePartition.size=66M\\nhw.sdCard=yes\\nsdcard.size=512M\\n' > "$ANDROID_AVD_HOME/pixaura-ci-shell.avd/config.ini"
echo "AVD created: $*"
`);
  write('bin/df', `#!/usr/bin/env bash
count_file="$MOCK_STATE_DIRECTORY/df.count"
count=0; if [[ -f "$count_file" ]]; then read -r count < "$count_file"; fi
count=$((count + 1)); echo "$count" > "$count_file"
available=7069061
if [[ "$MOCK_SCENARIO" == low-disk || "$MOCK_SCENARIO" == local-low-disk || "$MOCK_SCENARIO" == symlink-cleanup ]]; then available=1024; fi
if [[ "$MOCK_SCENARIO" == cleanup-recovers && "$count" == 1 ]]; then available=1024; fi
if [[ "$MOCK_SCENARIO" == boundary-disk ]]; then available=4325376; fi
if [[ "$MOCK_SCENARIO" == invalid-disk ]]; then available=invalid; fi
printf 'Filesystem 1024-blocks Used Available Capacity Mounted on\\nmock 9999999 0 %s 0%% /\\n' "$available"
`);
  write('platforms/android/app/build/tmp/disposable.txt', 'temporary output');
  write('sdk/keep.txt', 'required SDK component');
  if (scenario === 'symlink-cleanup') {
    fs.rmSync(path.join(directory, 'platforms/android/app/build/tmp'), { recursive: true });
    fs.symlinkSync(path.join(directory, 'sdk'), path.join(directory, 'platforms/android/app/build/tmp'));
  }
  write('sdk/emulator/emulator', `#!/usr/bin/env bash
if [[ "$1" == -accel-check ]]; then exit 1; fi
if [[ "$1" == -version ]]; then echo 'fake emulator version'; exit 0; fi
data_path=''; previous=''
for argument in "$@"; do
  if [[ "$previous" == -data ]]; then data_path="$argument"; fi
  previous="$argument"
done
hardware="$ANDROID_AVD_HOME/pixaura-ci-shell.avd/hardware-qemu.ini"
if [[ "$*" == *-check-snapshot-loadable* ]]; then
  echo "resolver: $*"
  echo resolution-start >> "$MOCK_STATE_DIRECTORY/calls.log"
  if [[ "$MOCK_SCENARIO" == resolution-failure ]]; then exit 9; fi
  if [[ "$MOCK_SCENARIO" == resolution-timeout ]]; then sleep 30; fi
  if [[ "$MOCK_SCENARIO" == missing-hardware ]]; then exit 0; fi
  resolved_size=2147483648
  if [[ "$MOCK_SCENARIO" == resolved-6g || "$*" == *-wipe-data* || ! -f "$data_path" ]]; then
    resolved_size=6442450944
    echo disk.dataPartition.size=6442450944 >> "$ANDROID_AVD_HOME/pixaura-ci-shell.avd/config.ini"
  fi
  resolved_path="$data_path"
  if [[ "$MOCK_SCENARIO" == wrong-data-path ]]; then resolved_path="$MOCK_STATE_DIRECTORY/other.img"; fi
  printf 'disk.dataPartition.size = %s\\ndisk.dataPartition.path = %s\\n' "$resolved_size" "$resolved_path" > "$hardware"
  if [[ "$MOCK_SCENARIO" == initdata-recreation ]]; then echo disk.dataPartition.initPath=/sdk/userdata.img >> "$hardware"; fi
  if [[ "$MOCK_SCENARIO" == corrupt-data ]]; then truncate -s 2048 "$data_path"; fi
  exit 0
fi
if [[ "$MOCK_SCENARIO" == launch-size-drift ]]; then
  sed -i 's/2147483648/6442450944/' "$hardware"
fi
echo "emulator launch: $*"
echo "emulator stderr evidence" >&2
echo $$ > "$MOCK_STATE_DIRECTORY/emulator.pid"
if [[ "$MOCK_SCENARIO" == early-exit ]]; then exit 23; fi
trap 'echo emulator-cleanup >> "$MOCK_STATE_DIRECTORY/calls.log"; exit 0' TERM
while true; do sleep 0.1; done
`);
  write('sdk/emulator/qemu-img', `#!/usr/bin/env bash
image="\${!#}"
size=2147483648
if [[ "$MOCK_SCENARIO" == oversized-qemu-data ]]; then size=6442450944; fi
printf '{"filename":"%s","format":"raw","virtual-size":%s}\\n' "$image" "$size"
`);
  write('sdk/platform-tools/adb', `#!/usr/bin/env bash
echo "$*" >> "$MOCK_STATE_DIRECTORY/calls.log"
if [[ "$1" == start-server ]]; then echo server-started; exit 0; fi
if [[ "$1" == devices ]]; then
  echo 'List of devices attached'
  if [[ -f "$MOCK_STATE_DIRECTORY/emulator.pid" ]]; then echo 'emulator-5554 device'; fi
  exit 0
fi
if [[ "$1" != -s || "$2" != emulator-5554 ]]; then exit 90; fi
shift 2
next() {
  local file="$MOCK_STATE_DIRECTORY/$1.count" count=0
  if [[ -f "$file" ]]; then read -r count < "$file"; fi
  count=$((count + 1)); echo "$count" > "$file"; echo "$count"
}
case "$*" in
  get-state)
    if [[ "$MOCK_SCENARIO" == stalled-adb ]]; then sleep 30; fi
    if [[ "$MOCK_SCENARIO" == invisible ]]; then echo offline; exit 1; fi
    if [[ "$(next device)" == 1 ]]; then echo offline; else echo device; fi ;;
  'shell getprop sys.boot_completed')
    if [[ "$MOCK_SCENARIO" == boot-timeout || "$(next boot)" == 1 ]]; then echo 0; else printf '1\\r\\n'; fi ;;
  'shell getprop') echo '[sys.boot_completed]: [1]' ;;
  'shell pm path android')
    if [[ "$MOCK_SCENARIO" == package-timeout || "$(next package)" == 1 ]]; then
      echo 'Error: package manager not ready'
    else echo 'package:/system/framework/framework-res.apk'; fi ;;
  'shell input keyevent 82') echo unlocked ;;
  'logcat -d -t 200') echo synthetic-logcat ;;
  *) exit 91 ;;
esac
`);
  write('platforms/android/gradlew', `#!/usr/bin/env bash
echo instrumentation-start >> "$MOCK_STATE_DIRECTORY/calls.log"
echo "$*" > "$MOCK_STATE_DIRECTORY/gradle-args.txt"
mkdir -p app/build/outputs/androidTest-results/connected
echo '<testsuite tests="1"/>' > app/build/outputs/androidTest-results/connected/test.xml
echo instrumentation-output
if [[ "$MOCK_SCENARIO" == test-failure ]]; then
  kill "$(cat "$MOCK_STATE_DIRECTORY/emulator.pid")"
  sleep 0.2
  exit 7
fi
if [[ "$MOCK_SCENARIO" == test-timeout ]]; then sleep 30; fi
`);
  try {
    const result = spawnSync('bash', ['scripts/check-android-emulator.sh'], {
      cwd: directory,
      env: {
        ...process.env,
        PATH: `${path.join(directory, 'bin')}:${path.dirname(process.execPath)}:${process.env.PATH}`,
        ANDROID_HOME: path.join(directory, 'sdk'),
        MOCK_STATE_DIRECTORY: path.join(directory, 'state'),
        MOCK_SCENARIO: scenario,
        CI: scenario === 'local-low-disk' ? 'false' : 'true',
        GITHUB_ACTIONS: 'true',
        ANDROID_DEVICE_TIMEOUT_SECONDS: scenario === 'invisible' || scenario === 'stalled-adb' ? '1' : '5',
        ANDROID_BOOT_TIMEOUT_SECONDS: scenario === 'boot-timeout' ? '1' : '5',
        ANDROID_PACKAGE_TIMEOUT_SECONDS: scenario === 'package-timeout' ? '1' : '5',
        ANDROID_TEST_TIMEOUT_SECONDS: scenario === 'test-timeout' ? '1' : '5',
        ANDROID_COMMAND_TIMEOUT_SECONDS: '1',
        ANDROID_RESOLUTION_TIMEOUT_SECONDS: scenario === 'resolution-timeout' ? '1' : '5',
        ANDROID_POLL_INTERVAL_SECONDS: '0.02',
      },
      encoding: 'utf8', timeout: 20000,
    });
    assert.ifError(result.error);
    const evidence = path.join(directory, 'build/android-emulator-evidence');
    const read = file => fs.readFileSync(path.join(evidence, file), 'utf8');
    for (const file of ['emulator.stdout.log', 'emulator.stderr.log', 'adb-devices.txt', 'boot-properties.txt',
      'package-manager.txt', 'logcat.txt', 'final-phase.txt', 'exit-code.txt']) {
      assert.ok(fs.existsSync(path.join(evidence, file)), `${scenario}: missing ${file}`);
    }
    assert.equal(Number(read('exit-code.txt')), result.status, result.stderr);
    assert.doesNotMatch(result.stderr, /No such process/);
    if (fs.existsSync(path.join(directory, 'state/emulator.pid'))) {
      const pid = Number(fs.readFileSync(path.join(directory, 'state/emulator.pid'), 'utf8'));
      assert.throws(() => process.kill(pid, 0), { code: 'ESRCH' }, `${scenario}: emulator leaked`);
    }
    assert.equal(fs.readFileSync(path.join(directory, 'sdk/keep.txt'), 'utf8'), 'required SDK component');
    return {
      ...result,
      phase: read('final-phase.txt').trim(),
      calls: fs.readFileSync(path.join(directory, 'state/calls.log'), 'utf8'),
      stdoutLog: read('emulator.stdout.log'), stderrLog: read('emulator.stderr.log'),
      acceleration: fs.existsSync(path.join(evidence, 'acceleration.log')) ? read('acceleration.log') : '',
      diskLog: read('disk-preflight.log'), config: read('avd-config.ini'),
      resolved: fs.existsSync(path.join(evidence, 'userdata-resolved.json')) && read('userdata-resolved.json').trim()
        ? JSON.parse(read('userdata-resolved.json')) : null,
      resolutionLog: fs.existsSync(path.join(evidence, 'emulator-resolution.log')) ? read('emulator-resolution.log') : '',
      temporaryOutputExists: fs.existsSync(path.join(directory, 'platforms/android/app/build/tmp/disposable.txt')),
      testLog: fs.existsSync(path.join(evidence, 'instrumentation.log')) ? read('instrumentation.log') : '',
      gradleArgs: fs.existsSync(path.join(directory, 'state/gradle-args.txt'))
        ? fs.readFileSync(path.join(directory, 'state/gradle-args.txt'), 'utf8') : '',
      testResultsWritten: fs.existsSync(path.join(directory, 'platforms/android/app/build/outputs/androidTest-results/connected/test.xml')),
    };
  } finally {
    const pidFile = path.join(directory, 'state/emulator.pid');
    if (fs.existsSync(pidFile)) {
      try { process.kill(Number(fs.readFileSync(pidFile, 'utf8')), 'SIGTERM'); }
      catch (error) { if (error.code !== 'ESRCH') throw error; }
    }
    fs.rmSync(directory, { recursive: true, force: true });
  }
}

test('emulator orchestration waits for device, boot and package manager before invoking connectedDebugAndroidTest', () => {
  const result = runScenario('success');
  assert.equal(result.status, 0, result.stderr);
  assert.equal(result.phase, 'complete');
  assert.match(result.stdout, /device-visibility ready[\s\S]*boot-completion ready[\s\S]*package-manager ready/);
  assert.match(result.gradleArgs, /--dependency-verification strict connectedDebugAndroidTest/);
  assert.equal(result.testResultsWritten, true);
  assert.match(result.calls, /emulator-cleanup/);
  assert.match(result.acceleration, /Emulator acceleration: off/);
  assert.match(result.stdoutLog, /-port 5554 -accel off/);
});

test('emulator early exit fails before testing and retains both emulator logs', () => {
  const result = runScenario('early-exit');
  assert.equal(result.status, 1);
  assert.match(result.stderr, /Emulator exited with 23 before device-visibility/);
  assert.equal(result.gradleArgs, '');
  assert.match(result.stderrLog, /emulator stderr evidence/);
});

for (const [scenario, phase] of [
  ['invisible', 'device-visibility'], ['stalled-adb', 'device-visibility'],
  ['boot-timeout', 'boot-completion'], ['package-timeout', 'package-manager'],
]) {
  test(`${scenario} fails with a bounded, named readiness timeout and never invokes Gradle`, () => {
    const result = runScenario(scenario);
    assert.equal(result.status, 124, result.stderr);
    assert.equal(result.phase, phase);
    assert.match(result.stderr, new RegExp(`Timed out waiting for ${phase}`));
    assert.equal(result.gradleArgs, '');
  });
}

test('instrumentation failure survives cleanup even if the emulator already exited', () => {
  const result = runScenario('test-failure');
  assert.equal(result.status, 7, result.stderr);
  assert.equal(result.phase, 'instrumentation');
  assert.match(result.testLog, /instrumentation-output/);
});

test('instrumentation timeout is independent from startup readiness and remains a failure', () => {
  const result = runScenario('test-timeout');
  assert.equal(result.status, 124, result.stderr);
  assert.equal(result.phase, 'instrumentation');
  assert.match(result.testLog, /instrumentation-output/);
});

for (const scenario of ['success', 'boundary-disk', 'cleanup-recovers']) {
  test(scenario + ': small disks and sufficient preflight space allow instrumentation', () => {
    const result = runScenario(scenario);
    assert.equal(result.status, 0, result.stderr);
    assert.match(result.stdoutLog, /-partition-size 2048 -cache-size 128/);
    assert.match(result.stdoutLog, /-data .*ci-userdata\..*\.img -datadir /);
    assert.doesNotMatch(result.stdoutLog, /-wipe-data/);
    assert.equal(result.resolved.resolvedBytes, 2147483648);
    assert.equal(result.resolved.ext4VirtualBytes, 2147483648);
    assert.match(result.config, /^disk.dataPartition.size=2048M$/m);
    assert.match(result.config, /^disk.cachePartition.size=128M$/m);
    assert.match(result.config, /^hw.sdCard=no$/m);
    assert.doesNotMatch(result.config, /sdcard.size=|6G|66M/);
    assert.match(result.diskLog, /required: 4224 MiB/);
    assert.equal(result.temporaryOutputExists, scenario !== 'cleanup-recovers');
    assert.equal(result.testResultsWritten, true);
  });
}

for (const scenario of ['resolved-6g', 'wrong-data-path', 'initdata-recreation', 'corrupt-data', 'missing-hardware', 'resolution-failure', 'resolution-timeout', 'oversized-qemu-data']) {
  test(scenario + ': effective resolution must pass before the test VM launches', () => {
    const result = runScenario(scenario);
    assert.notEqual(result.status, 0);
    assert.equal(result.phase, 'userdata-resolution');
    assert.equal(result.stdoutLog, '');
    assert.equal(result.gradleArgs, '');
    assert.match(result.calls, /resolution-start/);
  });
}

test('images without factory-empty userdata fail before resolution or VM launch', () => {
  const result = runScenario('unsupported-factory-data');
  assert.notEqual(result.status, 0);
  assert.equal(result.phase, 'userdata-preparation');
  assert.match(result.stderr, /does not declare empty_data_disk/);
  assert.equal(result.stdoutLog, '');
  assert.equal(result.gradleArgs, '');
});

test('an effective size change during actual launch fails before instrumentation', () => {
  const result = runScenario('launch-size-drift');
  assert.notEqual(result.status, 0);
  assert.equal(result.phase, 'userdata-running-assertion');
  assert.match(result.stderr, /Effective resolved userdata 6442450944/);
  assert.equal(result.gradleArgs, '');
  assert.match(result.calls, /emulator-cleanup/);
});
for (const scenario of ['low-disk', 'local-low-disk', 'symlink-cleanup', 'invalid-disk']) {
  test(scenario + ': preflight fails before launch and preserves SDK and evidence', () => {
    const result = runScenario(scenario);
    assert.notEqual(result.status, 0);
    assert.equal(result.phase, 'disk-preflight');
    assert.equal(result.stdoutLog, '');
    assert.equal(result.gradleArgs, '');
    assert.match(result.stderr, /Insufficient disk space|Cannot determine free disk/);
    if (scenario === 'local-low-disk') assert.equal(result.temporaryOutputExists, true);
    if (scenario === 'symlink-cleanup') assert.match(result.diskLog, /Refusing cleanup/);
  });
}
