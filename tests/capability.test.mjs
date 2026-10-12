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
  // Step 4 authorizes only a delegating editor facade; foundation stays pure.
  const editor = read('packages/core/src/editor_api.cpp');
  for (const call of ['pixaura_policy_configure(version,&s.raster', 'pixaura_policy_update(version,&s.raster', 'pixaura_policy_query(version,&s.raster']) assert.ok(editor.includes(call));
  assert.ok(!/PIXAURA_TIER_|PIXAURA_POLICY_(NORMAL|SEVERE|CRITICAL)|physical_memory_bytes\s*[<>=]/.test(editor));
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

test('finite Phase 4 roadmap is frozen with only Step 4 currently authorized', () => {
  const roadmap = read('docs/plans/phase-4-roadmap.md');
  assert.match(roadmap, /FROZEN by Product Owner/);
  const rows = [...roadmap.matchAll(/^\| (\d+) \|/gm)];
  assert.deepEqual(rows.map(m => Number(m[1])), [1, 2, 3, 4, 5, 6, 7]);
  assert.match(roadmap, /Steps 5–7 remain NOT STARTED/);
  assert.match(read('QUALITY_GATES.md'), /Phase 4 Step 1/);
  assert.match(read('docs/contracts/capability-profile-v1.md'), /DH-APPLE-METAL-01/);
});

test('budget authority is pure shared preflight and cannot bypass existing owners', () => {
  const core = read('packages/core/src/budget.cpp');
  const header = read('packages/core/include/pixaura/budget.h');
  assert.match(header, /PIXAURA_BUDGET_VERSION 1u/);
  assert.ok(!/\b(new|malloc|calloc|realloc|thread|mutex|sqlite3_|fopen|socket)\b/.test(core));
  for (const authority of ['pixaura_decode_default_limits', 'pixaura_working_default_limits', 'PIXAURA_GPU_MAX_PIXELS']) assert.ok(core.includes(authority));
  assert.ok(!/budget|Budget/.test(read('packages/core/src/editor_api.cpp')));
  assert.ok(!/Budget|budget_admit/.test(read('platforms/ios/PixAuraAI/ManualEditorScreen.swift')));
  assert.ok(!fs.existsSync('packages/core/src/storage_schema_v4.hpp'));
  assert.match(read('docs/contracts/budget-admission-v1.md'), /not a\s+reservation/);
});

test('shared budget corpus runs at native JNI and Swift boundaries', () => {
  for (const file of ['packages/core/tests/capability_c_consumer.c', 'packages/core/tests/capability_test.cpp', 'platforms/android/native/capability_bridge.cpp']) assert.ok(read(file).includes('budget_boundary_check'), file);
  assert.ok(read('platforms/android/app/src/androidTest/java/ai/pixaura/app/CapabilityFoundationTest.kt').includes('nativeBudgetBoundary'));
  assert.ok(read('packages/core/swift/Tests/BudgetAdmissionTests.swift').includes('pixaura_budget_admit'));
  const tests = read('packages/core/tests/budget_boundary.h');
  for (const evidence of ['delta = -1', 'UINT64_MAX', 'PIXAURA_BUDGET_OVERFLOW', 'PIXAURA_BUDGET_OCCUPANCY', 'PIXAURA_BUDGET_PREVIEW_MEMORY', 'PIXAURA_GPU_PHYSICAL_CERTIFIED']) assert.ok(tests.includes(evidence));
  assert.match(read('packages/core/tests/capability_test.cpp'), /160000 parallel admission/);
  assert.ok(read('packages/core/Package.swift').includes('src/budget.cpp'));
});

test('dynamic policy is shared runtime authority with existing publication ownership', () => {
  const contract = read('docs/contracts/dynamic-capability-policy-v1.md');
  const header = read('packages/core/include/pixaura/dynamic_policy.h');
  assert.match(header, /PIXAURA_DYNAMIC_POLICY_VERSION 1u/);
  assert.match(contract, /THREE distinct monotonically increasing/);
  assert.match(contract, /Schema 3/);
  assert.match(contract, /Step 4 NOT STARTED/);
  assert.ok(read('packages/core/Package.swift').includes('src/dynamic_policy.cpp'));
  for (const file of ['packages/core/tests/capability_c_consumer.c', 'packages/core/tests/capability_test.cpp', 'platforms/android/native/capability_bridge.cpp']) {
    assert.ok(read(file).includes('dynamic_policy_boundary_check'), file);
  }
  assert.ok(read('packages/core/swift/Tests/DynamicPolicyTests.swift').includes('pixaura_policy_update'));
  assert.ok(read('platforms/android/app/src/androidTest/java/ai/pixaura/app/CapabilityFoundationTest.kt').includes('nativeDynamicPolicyBoundary'));
  assert.ok(!/policy|Policy/.test(read('packages/core/src/storage_schema_v3.hpp')));
  assert.ok(!/dynamic_policy|policy_update/.test(read('platforms/ios/PixAuraAI/ManualEditorScreen.swift')));
});

test('Android capability adapter delegates strategy and cannot escalate backend trust', () => {
  const owner = read('platforms/android/app/src/main/java/ai/pixaura/app/AndroidCapabilityOwner.kt');
  const mapping = read('platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/AndroidCapability.kt');
  const jni = read('platforms/android/native/editor_bridge.cpp');
  for (const api of ['availableProcessors', 'memoryClass', 'totalMem', 'currentThermalStatus', 'addThermalStatusListener', 'removeThermalStatusListener', 'registerComponentCallbacks', 'unregisterComponentCallbacks']) assert.ok(owner.includes(api), api);
  assert.ok(!/Build\.(MODEL|FINGERPRINT|SERIAL)|getImei|AdvertisingId|\bT[012]\b|PHYSICAL_CERTIFIED/.test(owner + mapping));
  assert.match(jni, /gpu_validation=PIXAURA_GPU_NOT_VALIDATED/);
  assert.match(jni, /output\[i\]<0/);
  assert.ok(read('platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/ManualEditor.kt').includes('nativePolicyUpdate'));
  assert.ok(read('platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorModel.kt').includes('pendingCapability'));
  for (const file of ['AndroidCapabilityIntegrationTest.kt', 'AndroidCapabilityLifecycleTest.kt']) assert.ok(read(`platforms/android/app/src/androidTest/java/ai/pixaura/app/${file}`).includes('revisionCount'));
  assert.match(read('docs/contracts/android-capability-integration-v1.md'), /EXACT.*exact-or-resource-failure/);
  assert.match(read('docs/reports/phase-4-step-4.md'), /Steps 5–7 NOT STARTED/);
});
