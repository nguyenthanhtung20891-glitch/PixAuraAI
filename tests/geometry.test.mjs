import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
const read = p => fs.readFileSync(new URL(`../${p}`, import.meta.url), 'utf8');

test('approved outward rasterization golden vectors use exact integer edges', () => {
  const edge = (dimension, start, length) => {
    const m = 1000000n, d = BigInt(dimension), s = BigInt(start), n = BigInt(length);
    assert.ok(s >= 0 && n > 0 && s + n <= m);
    return [Number(d * s / m), Number((d * (s + n) + m - 1n) / m)];
  };
  assert.deepEqual(edge(3, 250000, 500000), [0, 3]);
  assert.deepEqual(edge(3, 999999, 1), [2, 3]);
  assert.deepEqual(edge(1, 999999, 1), [0, 1]);
  assert.deepEqual(edge(16384, 0, 1000000), [0, 16384]);
  assert.throws(() => edge(3, 999999, 2));
});

test('geometry and cancellation are wired through authoritative native and platform boundaries', () => {
  for (const file of ['CMakeLists.txt', 'packages/core/Package.swift']) assert.ok(read(file).includes('src/geometry.cpp'));
  assert.match(read('CMakeLists.txt'), /geometry_test PROPERTIES TIMEOUT 90/);
  const header = read('packages/core/include/pixaura/geometry.h');
  assert.match(header, /PIXAURA_CANCELLED 13/);
  assert.match(header, /PIXAURA_TILE_EDGE 128u/);
  assert.match(header, /PIXAURA_MAX_TILES 4096u/);
  for (const file of ['packages/core/tests/decode_c_consumer.c', 'platforms/android/native/bridge.cpp']) assert.ok(read(file).includes('geometry_boundary_check'));
  assert.ok(read('packages/core/swift/Tests/DecodeBoundaryTests.swift').includes('pixaura_working_evaluate_cancel'));
  for (const file of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml', 'scripts/check-apple.sh']) assert.ok(read(file).includes('tests/geometry.test.mjs'));
  for (const file of ['docs/contracts/geometry-v1.md', 'docs/contracts/cancellation-v1.md', 'docs/adr/0013-bounded-geometry-and-cancellation.md']) assert.ok(read(file).length > 1000);
});
