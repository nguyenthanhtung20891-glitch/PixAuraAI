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
