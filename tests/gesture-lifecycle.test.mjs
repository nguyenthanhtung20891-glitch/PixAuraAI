import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
const read = path => fs.readFileSync(path, 'utf8');
test('Step 6 contract binds one shared ephemeral lifecycle to existing owners', () => {
  const contract = read('docs/contracts/gesture-lifecycle-integration-v1.md');
  for (const term of ['IDLE', 'ACTIVE', 'COMMITTING', 'COMPLETED', 'CANCELLED', 'INVALIDATED', 'ONE active', 'SHA-256', 'PRV1', 'generation fence', 'explicit retry', 'Schema 3', 'DH-APPLE-METAL-01', 'Step 7 has NOT started']) assert.ok(contract.includes(term), term);
  for (const api of ['manual_edit_begin', 'manual_state', 'manual_preview_ticket', 'manual_preview_current', 'manual_interrupt']) assert.ok(read('packages/core/include/pixaura/manual.h').includes('pixaura_' + api));
  assert.ok(!fs.existsSync('packages/core/src/storage_schema_v4.hpp'));
});
test('Step 6 executes native/race/fault and shared platform consumers in required gates', () => {
  assert.ok(read('CMakeLists.txt').includes('NAME gesture_lifecycle_test'));
  assert.ok(read('CMakeLists.txt').includes('NAME gesture_lifecycle_c_consumer'));
  assert.ok(read('platforms/android/native/bridge.cpp').includes('gesture_lifecycle_boundary_check'));
  assert.ok(read('platforms/android/app/src/androidTest/java/ai/pixaura/app/ManualBoundaryTest.kt').includes('nativeGestureLifecycle'));
  assert.ok(read('packages/core/swift/Tests/GestureLifecycleTests.swift').includes('pixaura_manual_edit_begin'));
  for (const path of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml', 'scripts/check-apple.sh']) assert.ok(read(path).includes('tests/gesture-lifecycle.test.mjs'));
});
test('Step 6 status never authorizes frozen Steps 7 and 8', () => {
  const roadmap = read('ROADMAP.md');
  assert.match(roadmap, /\| 6 \| Gesture Editing Lifecycle Integration \| (IN PROGRESS|COMPLETE \/ FULL PASS) \|/);
  for (const step of [7, 8]) assert.match(roadmap, new RegExp(`\\| ${step} \\|[^\\n]+\\| NOT STARTED \\|`));
  assert.ok(read('QUALITY_GATES.md').includes('Phase 3 Step 6 acceptance'));
  assert.ok(fs.existsSync('docs/reports/phase-3-step-6.md'));
});
