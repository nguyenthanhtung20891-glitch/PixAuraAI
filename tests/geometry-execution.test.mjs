import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
const read = p => fs.readFileSync(new URL(`../${p}`, import.meta.url), 'utf8');
test('minimal geometry execution retains the frozen pixel movement and bounded two-raster contract', () => {
  const source = read('packages/core/src/evaluation.cpp');
  assert.ok(source.includes('geometry::plan(extent,stack,limits)'));
  assert.ok(source.includes('geometry::inverse(stage.input,stage.turns,destination)'));
  assert.ok(source.includes('std::memmove'));
  assert.ok(source.includes('uint8_t saved[16]'));
  assert.ok(source.includes('moves++%1024'));
  assert.ok(source.includes('visited.capacity()<=1048576'));
  assert.ok(read('packages/core/src/decode_api.cpp').includes('evaluation::reservation'));
  assert.ok(read('docs/contracts/geometry-execution-v1.md').includes('Maximum request working rasters: two'));
});
test('geometry execution fixtures and platform probes are included in all required gates', () => {
  assert.ok(read('CMakeLists.txt').includes('geometry_execution_test'));
  for (const file of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml', 'scripts/check-apple.sh']) assert.ok(read(file).includes('tests/geometry-execution.test.mjs'));
  assert.ok(read('packages/core/tests/geometry_boundary.h').includes('float original[24],cropped[12]'));
  assert.ok(read('packages/core/swift/Tests/DecodeBoundaryTests.swift').includes('mixedPixel.map'));
  assert.ok(read('packages/core/tests/decode_allocation_test.cpp').includes('phases=12'));
});
