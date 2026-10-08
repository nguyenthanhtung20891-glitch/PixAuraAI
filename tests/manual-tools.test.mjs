import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
const read = p => fs.readFileSync(new URL(`../${p}`, import.meta.url), 'utf8');

test('manual foundation is shared, bounded and compiled across platforms', () => {
  for (const file of ['CMakeLists.txt', 'packages/core/Package.swift']) assert.ok(read(file).includes('src/manual_tools.cpp'));
  const header = read('packages/core/include/pixaura/manual.h');
  assert.match(header, /MAX_DESCRIPTORS 16u/);
  assert.match(header, /MAX_PARAMETERS 8u/);
  const implementation = read('packages/core/src/manual_tools.cpp');
  assert.match(implementation, /parse_evaluation\(request\)/);
  assert.match(implementation, /transition\(\*live, candidate\)/);
  assert.match(implementation, /sequence_ != UINT64_MAX/);
  for (const file of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml', 'scripts/check-apple.sh']) assert.ok(read(file).includes('tests/manual-tools.test.mjs'));
  assert.ok(read('platforms/android/app/src/androidTest/java/ai/pixaura/app/ManualBoundaryTest.kt').includes('nativeManualRegistry'));
  assert.ok(read('packages/core/swift/Tests/ManualBoundaryTests.swift').includes('pixaura_manual_registry_json'));
});

test('manual contract freezes only accepted parameter semantics and separates proposals', () => {
  const contract = read('docs/contracts/manual-tools-v1.md');
  for (const term of ['PROPOSED', '-5000..5000', '0..999999', '1..1000000', '1000 coalesced', 'CANCEL', 'PRV1', 'STALE_BASE']) assert.ok(contract.includes(term));
  assert.match(contract, /No subsequent step sequence is frozen/);
  assert.match(contract, /No clamp/);
});
