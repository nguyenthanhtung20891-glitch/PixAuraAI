import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { selectSimulator } from '../scripts/select-ios-simulator.mjs';

const older = 'AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA';
const newer = 'BBBBBBBB-BBBB-BBBB-BBBB-BBBBBBBBBBBB';

test('Apple CI selects an available iPhone on the newest installed iOS runtime', () => {
  assert.equal(selectSimulator({ devices: {
    'com.apple.CoreSimulator.SimRuntime.iOS-18-0': [
      { name: 'iPhone 16', isAvailable: true, udid: older },
    ],
    'com.apple.CoreSimulator.SimRuntime.tvOS-27-0': [
      { name: 'iPhone invalid runtime', isAvailable: true, udid: older },
    ],
    'com.apple.CoreSimulator.SimRuntime.iOS-26-0': [
      { name: 'iPhone unavailable', isAvailable: false, udid: older },
      { name: 'iPad Pro', isAvailable: true, udid: older },
      { name: 'iPhone 17', isAvailable: true, udid: newer },
    ],
  } }), newer);
});

test('Apple CI fails explicitly when no valid iOS simulator is available', () => {
  assert.throws(() => selectSimulator({ devices: {} }), /No available iPhone/);
  assert.throws(() => selectSimulator({ devices: {
    'com.apple.CoreSimulator.SimRuntime.iOS-26-0': [
      { name: 'iPhone 17', isAvailable: true, udid: 'malformed' },
    ],
  } }), /No available iPhone/);
});

test('workflow scripts and Apple package scheme references resolve', () => {
  const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
  const workflow = fs.readFileSync(path.join(root, '.github/workflows/foundation.yml'), 'utf8');
  const referencedScripts = [...workflow.matchAll(/run: bash (scripts\/[^\s]+\.sh)/g)];
  assert.ok(referencedScripts.length > 0, 'Workflow does not invoke boundary check scripts');
  for (const [, script] of referencedScripts) {
    const contents = fs.readFileSync(path.join(root, script), 'utf8');
    assert.ok(!contents.includes('\r'), `${script}: shell script must use LF`);
    for (const [, reference] of contents.matchAll(/(?:node|bash) ((?:scripts|tests)\/[^\s]+)/g)) {
      assert.ok(fs.existsSync(path.join(root, reference)), `${script}: unresolved ${reference}`);
    }
  }
  const apple = fs.readFileSync(path.join(root, 'scripts/check-apple.sh'), 'utf8');
  const manifest = fs.readFileSync(path.join(root, 'packages/core/Package.swift'), 'utf8');
  const products = [...manifest.matchAll(/\.library\(name: "([^"]+)"/g)].map(match => match[1]);
  for (const [, scheme] of apple.matchAll(/xcodebuild -scheme (\S+)/g)) {
    assert.ok(products.includes(scheme), `Apple scheme ${scheme} is not a package library product`);
  }
});

test('Apple simulator commands share the native host architecture across package, app and tests', () => {
  const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
  const read = file => fs.readFileSync(path.join(root, file), 'utf8');
  const helper = read('scripts/ios-simulator-architecture.sh');
  assert.match(helper, /simulator_arch="\$\(uname -m\)"/);
  assert.match(helper, /arm64\|x86_64\)/);
  for (const file of ['scripts/check-apple.sh', 'scripts/check-ios-shell.sh']) {
    const script = read(file);
    assert.match(script, /source scripts\/ios-simulator-architecture\.sh/);
    assert.match(script, /platform=iOS Simulator,id=\$simulator_id,arch=\$simulator_arch/);
    assert.match(script, /ARCHS="\$simulator_arch" ONLY_ACTIVE_ARCH=YES EXCLUDED_ARCHS= CODE_SIGNING_ALLOWED=NO test/);
    // Device builds retain normal SDK-selected architecture and signing policy.
    const deviceCommand = script.match(/xcodebuild[^\n]*(?:\\\n[^\n]*)*CODE_SIGNING_ALLOWED=NO build/);
    assert.ok(deviceCommand, `${file}: device build missing`);
    assert.doesNotMatch(deviceCommand[0], /ARCHS=|arch=\$simulator_arch/);
  }
  const project = read('platforms/ios/PixAuraAI.xcodeproj/project.pbxproj');
  assert.match(project, /ARCHS = "\$\(ARCHS_STANDARD\)"/);
  assert.doesNotMatch(project, /EXCLUDED_ARCHS|ONLY_ACTIVE_ARCH|ARCHS = "?(?:arm64|x86_64)/);
  const workflow = read('.github/workflows/native-shells.yml');
  assert.match(workflow, /run: bash scripts\/check-ios-shell\.sh/);
  assert.doesNotMatch(workflow, /ARCHS|ONLY_ACTIVE_ARCH|EXCLUDED_ARCHS/);
});

test('Android emulator CI uses bounded staged readiness, separate test timeout and failure evidence', () => {
  const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
  const read = file => fs.readFileSync(path.join(root, file), 'utf8');
  const workflow = read('.github/workflows/native-shells.yml');
  const script = read('scripts/check-android-emulator.sh');
  assert.match(workflow, /timeout-minutes: 35\s+run: bash scripts\/check-android-emulator\.sh/);
  assert.match(workflow, /tests\/android-emulator\.test\.mjs/);
  assert.match(workflow, /if: always\(\)[\s\S]*build\/android-emulator-evidence\//);
  assert.match(workflow, /app\/build\/outputs\/androidTest-results\//);
  assert.doesNotMatch(workflow, /wait-for-device/);
  assert.ok(!script.includes('\r'), 'Emulator script must use LF');
  for (const phase of ['device-visibility', 'boot-completion', 'package-manager']) {
    assert.match(script, new RegExp(`wait_for ${phase} "\\$[a-z_]+"`));
  }
  assert.match(script, /timeout --kill-after=5s "\$\{command_timeout\}s" "\$adb"/);
  assert.match(script, /while \(\( SECONDS < deadline \)\)/);
  assert.match(script, /return 124/);
  assert.match(script, /sys\.boot_completed/);
  assert.match(script, /shell pm path android/);
  assert.match(script, /export ANDROID_SERIAL=emulator-5554/);
  assert.match(script, /-port 5554 -accel "\$acceleration"/);
  assert.match(script, /acceleration=off/);
  assert.match(script, /-accel-check/);
  assert.match(script, /timeout --kill-after=10s "\$\{test_timeout\}s" bash gradlew/);
  assert.match(script, /set -euo pipefail/);
  assert.match(script, /trap cleanup EXIT/);
  assert.match(script, /local result=\$\?/);
  assert.match(script, /kill -0 "\$emulator_pid" 2>\/dev\/null/);
  assert.match(script, /exit "\$result"/);
  for (const file of ['emulator.stdout.log', 'emulator.stderr.log', 'adb-devices.txt', 'boot-properties.txt', 'instrumentation.log']) {
    assert.ok(script.includes(file), `Missing failure evidence ${file}`);
  }
});
