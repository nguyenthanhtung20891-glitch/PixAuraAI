import assert from 'node:assert/strict';
import fs from 'node:fs';
import test from 'node:test';

const read = (name) => fs.readFileSync(name, 'utf8');

test('Mac2 family admission survives the macOS 27 deprecation without becoming Apple7', () => {
  const worker = read('packages/core/src/apple/metal_worker.mm');
  const swift = read('packages/core/swift/Sources/MetalValidation.swift');
  assert.match(worker, /static_cast<MTLGPUFamily>\(2002\)/);
  assert.match(worker, /MTLGPUFamilyApple1/);
  assert.match(worker, /containsString:@"software"/);
  assert.match(worker, /containsString:@"virtual"/);
  assert.match(worker, /TARGET_OS_SIMULATOR/);
  const code = worker.replace(/\/\/[^\n]*/g, '');
  assert.doesNotMatch(code, /MTLGPUFamilyMac2|MTLGPUFamilyApple7|-Wno-deprecated|-Wno-error/);
  const swiftCode = swift.replace(/\/\/[^\n]*/g, '');
  assert.match(swiftCode, /supportsFamily\(\.apple1\)/);
  assert.match(swiftCode, /guard let macFamily = MTLGPUFamily\(rawValue: 2002\) else \{ return evidence \}/);
  assert.match(swiftCode, /!name\.contains\("software"\)/);
  assert.match(swiftCode, /!name\.contains\("virtual"\)/);
  assert.doesNotMatch(swiftCode, /\.mac2|\.apple7/);
  assert.match(read('scripts/check-apple.sh'), /-warnings-as-errors/);
  const report = JSON.parse(read('docs/reports/phase-10.1-metal-v1.json'));
  assert.equal(report.mac_studio_xcode_27_2_observed, false);
  assert.equal(report.apple_metal_v1, 'UNKNOWN');
  for (const workflow of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml']) {
    assert.match(read(workflow), /tests\/metal-family-compat\.test\.mjs/);
  }
});
