import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
const read = p => fs.readFileSync(p, 'utf8');

test('capability foundation has bounded versioned data and no execution or persistence authority', () => {
  const header = read('packages/core/include/pixaura/capability.h');
  const core = read('packages/core/src/capability.cpp');
  assert.match(header, /PIXAURA_CAPABILITY_VERSION 1u/);
  assert.match(header, /PIXAURA_CAPABILITY_WIRE_BYTES 76u/);
  for (const field of ['pixaura_capability_static', 'pixaura_capability_dynamic', 'gpu_availability', 'gpu_validation', 'process_budget_bytes', 'sequence']) assert.ok(header.includes(field));
  assert.ok(!/\b(new|malloc|calloc|realloc|thread|mutex|render|sqlite3_|fopen|socket)\b/.test(core));
  assert.ok(!/model_name|serial_number|device_id|uuid|vendor_name|fingerprint/.test(header + core));
  assert.ok(!/capability|Capability/.test(read('packages/core/src/editor_api.cpp')));
  assert.ok(!fs.existsSync('packages/core/src/storage_schema_v4.hpp'));
});

test('synthetic profiles are test-only inputs, not physical evidence or tier selection', () => {
  const fixtures = read('packages/core/tests/capability_fixtures.h');
  for (const label of ['LOW', 'MID', 'HIGH']) assert.ok(fixtures.includes(`"${label}"`));
  assert.match(fixtures, /SYNTHETIC POLICY PROFILE/);
  const boundary = read('packages/core/tests/capability_boundary.h');
  for (const evidence of ['golden[76]', 'PIXAURA_PROVENANCE_SYNTHETIC', 'PIXAURA_PROVENANCE_VIRTUAL', 'PIXAURA_GPU_PHYSICAL_CERTIFIED', 'pixaura_decode_default_limits', 'UINT64_MAX']) assert.ok(boundary.includes(evidence));
  assert.match(read('packages/core/tests/capability_test.cpp'), /10000/);
  assert.match(read('docs/contracts/capability-profile-v1.md'), /not tier thresholds/);
});

test('all build and runtime boundaries execute capability foundation tests', () => {
  assert.ok(read('CMakeLists.txt').includes('NAME capability_c_consumer'));
  assert.ok(read('CMakeLists.txt').includes('NAME capability_test'));
  assert.ok(read('packages/core/Package.swift').includes('src/capability.cpp'));
  assert.ok(read('platforms/android/native/CMakeLists.txt').includes('capability_bridge.cpp'));
  assert.ok(read('platforms/android/app/src/androidTest/java/ai/pixaura/app/CapabilityFoundationTest.kt').includes('nativeCapabilityFoundationBoundary'));
  assert.ok(read('packages/core/swift/Tests/CapabilityFoundationTests.swift').includes('pixaura_capability_normalize'));
  for (const file of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml', 'scripts/check-apple.sh']) assert.ok(read(file).includes('tests/capability.test.mjs'), file);
});

test('finite Phase 4 roadmap stays proposed with only Step 1 authorized', () => {
  const roadmap = read('docs/plans/phase-4-roadmap.md');
  assert.match(roadmap, /PROPOSED — NOT FROZEN/);
  const rows = [...roadmap.matchAll(/^\| (\d+) \|/gm)];
  assert.deepEqual(rows.map(m => Number(m[1])), [1, 2, 3, 4, 5, 6, 7]);
  assert.match(roadmap, /Steps 2\+ are NOT STARTED/);
  assert.match(read('QUALITY_GATES.md'), /Phase 4 Step 1/);
  assert.match(read('docs/contracts/capability-profile-v1.md'), /DH-APPLE-METAL-01/);
});
