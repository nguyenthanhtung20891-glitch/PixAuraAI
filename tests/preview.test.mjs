import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
const read = p => fs.readFileSync(new URL(`../${p}`, import.meta.url), 'utf8');
test('frozen reference sRGB thresholds agree with independent standard transfer', () => {
  const values = read('packages/core/src/preview_thresholds.hpp').split('srgb_half_byte[255]={')[1].split('};')[0].split(',').map(Number);
  assert.equal(values.length, 255);
  for (let k = 0; k < 255; k++) {
    const linear = values[k];
    const encoded = linear <= .0031308 ? 12.92 * linear : 1.055 * Math.pow(linear, 1 / 2.4) - .055;
    assert.ok(Math.abs(encoded - (k + .5) / 255) < 2e-15);
    assert.ok(linear > (values[k - 1] ?? 0));
  }
});
test('preview is wired into C, JNI, Swift, host and sanitizer gates', () => {
  for (const file of ['CMakeLists.txt', 'packages/core/Package.swift']) assert.ok(read(file).includes('src/preview.cpp'));
  for (const file of ['packages/core/tests/decode_c_consumer.c', 'platforms/android/native/bridge.cpp']) assert.ok(read(file).includes('preview_boundary_check'));
  assert.ok(read('packages/core/swift/Tests/DecodeBoundaryTests.swift').includes('pixaura_preview_copy'));
  for (const file of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml', 'scripts/check-apple.sh']) assert.ok(read(file).includes('tests/preview.test.mjs'));
  assert.match(read('CMakeLists.txt'), /preview_test PROPERTIES TIMEOUT 90/);
  assert.ok(read('packages/core/tests/decode_allocation_test.cpp').includes('phases=14'));
});
