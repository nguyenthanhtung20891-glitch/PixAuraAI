import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
const read = name => fs.readFileSync(new URL(`../${name}`, import.meta.url), 'utf8');
test('fixed transfer tables match the independent sRGB formula at every code', () => {
  const text = read('packages/core/src/srgb_table.hpp');
  const table = name => text.split(`${name}[256]={`)[1].split('};')[0].split(',').map(s => Number(s.trim().replace(/f$/, '')));
  const rgb = table('srgb_table'), alpha = table('alpha_table');
  assert.equal(rgb.length, 256); assert.equal(alpha.length, 256);
  for (let i = 0; i < 256; i++) {
    const s = i / 255, value = s <= 0.04045 ? s / 12.92 : ((s + 0.055) / 1.055) ** 2.4;
    assert.ok(Math.abs(rgb[i] - value) < 3e-8);
    assert.ok(Math.abs(alpha[i] - s) < 3e-8);
    assert.equal(Math.fround(rgb[i]), Math.fround(value));
    assert.equal(Math.fround(alpha[i]), Math.fround(s));
  }
});
test('normalization execution and public boundary have portable platform wiring', () => {
  for (const file of ['CMakeLists.txt', 'packages/core/Package.swift']) assert.ok(read(file).includes('src/working.cpp'));
  for (const file of ['platforms/android/native/bridge.cpp', 'packages/core/swift/Tests/DecodeBoundaryTests.swift', 'packages/core/tests/decode_c_consumer.c']) {
    assert.ok(read(file).includes('pixaura_working_normalize'));
    assert.ok(read(file).includes('pixaura_working_identity'));
  }
  assert.match(read('CMakeLists.txt'), /working_test PROPERTIES TIMEOUT 60/);
});
