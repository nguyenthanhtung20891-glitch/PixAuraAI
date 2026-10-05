import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
const read = p => fs.readFileSync(new URL(`../${p}`, import.meta.url), 'utf8');

test('frozen exposure constants satisfy the formula across every legal integer parameter', () => {
  const table = read('packages/core/src/exposure_table.hpp').split('exposure_fraction[1000]={')[1].split('};')[0].split(',').map(Number);
  assert.equal(table.length, 1000);
  for (let e = -5000; e <= 5000; e++) {
    const q = Math.floor(e / 1000), r = e - 1000 * q;
    const actual = table[r] * 2 ** q, expected = 2 ** (e / 1000);
    assert.ok(Math.abs(actual - expected) <= 1e-14 * expected);
  }
  assert.equal(table[0], 1);
});

test('evaluation boundary is wired into native, C, JNI, Swift and sanitizer builds', () => {
  for (const file of ['CMakeLists.txt', 'packages/core/Package.swift']) {
    assert.ok(read(file).includes('src/evaluation.cpp'));
    assert.ok(read(file).includes('-fno-fast-math'));
    assert.ok(read(file).includes('-ffp-contract=off'));
  }
  for (const file of ['packages/core/tests/decode_c_consumer.c', 'platforms/android/native/bridge.cpp', 'packages/core/swift/Tests/DecodeBoundaryTests.swift']) {
    assert.ok(read(file).includes('pixaura_evaluation_validate'));
    assert.ok(read(file).includes('pixaura_working_evaluate'));
  }
  assert.match(read('CMakeLists.txt'), /evaluation_test PROPERTIES TIMEOUT 90/);
  assert.match(read('CMakeLists.txt'), /\/fp:strict/);
  assert.match(read('packages/core/include/pixaura/evaluation.h'), /MAX_OPERATIONS 256u/);
});
