import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
const read = p => fs.readFileSync(p, 'utf8');
test('manual screens consume descriptor metadata, never duplicate native tool contracts', () => {
  const kotlin = read('platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorScreen.kt');
  const swift = read('platforms/ios/PixAuraAI/ManualEditorScreen.swift');
  for (const ui of [kotlin, swift]) {
    for (const field of ['minimum', 'maximum', 'step', 'parameters']) assert.ok(ui.includes(field));
    assert.ok(!/pixaura\.(crop|rotate|exposure|blur|sharpen)/.test(ui));
    assert.ok(!/1000000|6504|25000|5000/.test(ui));
    for (const action of ['Apply', 'Cancel', 'Undo', 'Redo', 'Filters/Presets', 'Reset to default']) assert.ok(ui.includes(action));
  }
  assert.ok(read('platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/ManualEditor.kt').includes('nativeManualRegistry'));
  assert.ok(read('packages/core/swift/Sources/ManualEditor.swift').includes('pixaura_manual_registry_json'));
});
test('application service composes frozen owners and authority without schema expansion', () => {
  const service = read('packages/core/src/editor_api.cpp');
  for (const call of ['pixaura_manual_edit_begin', 'pixaura_manual_update', 'pixaura_manual_commit', 'pixaura_manual_interrupt', 'pixaura_manual_render', 'repository->apply', 'open_session']) assert.ok(service.includes(call));
  assert.ok(!fs.existsSync('packages/core/src/storage_schema_v4.hpp'));
  assert.ok(read('CMakeLists.txt').includes('NAME editor_test'));
  assert.ok(read('packages/core/Package.swift').includes('src/editor_api.cpp'));
});
test('Step 7 authority leaves Step 8 frozen and tracks exact parity', () => {
  assert.match(read('ROADMAP.md'), /\| 7 \| Cross-platform Manual Editor Integration \| (IN PROGRESS|COMPLETE \/ FULL PASS) \|/);
  assert.match(read('ROADMAP.md'), /\| 8 \|[^\n]+\| NOT STARTED \|/);
  const report = read('docs/reports/phase-3-step-7.md');
  for (const name of ['crop', 'rotate', 'exposure', 'brightness', 'contrast', 'highlights', 'shadows', 'saturation', 'temperature', 'sharpen', 'blur']) assert.ok(report.includes('pixaura.' + name));
  assert.ok(read('QUALITY_GATES.md').includes('Phase 3 Step 7 acceptance'));
});

test('accepted ADR 0021 gates Android publication and has no overwrite fallback', () => {
  assert.match(read('docs/adr/0021-android-no-replace-asset-publication.md'), /Status: Accepted/);
  const files = read('packages/core/src/storage_files.cpp');
  assert.match(files, /#elif defined\(__ANDROID__\) \|\| defined\(PIXAURA_TEST_ANDROID_PUBLICATION\)/);
  assert.ok(files.includes('linkat(stage_dir.handle')); // retained non-Android Linux
  const backend = read('packages/core/src/android_asset_publication.hpp');
  assert.match(backend, /syscall\(SYS_renameat2, source, name, destination, digest, RENAME_NOREPLACE\)/);
  assert.ok(!/\blinkat\(|\brename\(|\brenameat\(|\bchmod\(/.test(backend));
  assert.ok(backend.includes('errno != EEXIST'));
  assert.ok(read('CMakeLists.txt').includes('NAME storage_android_test'));
  assert.ok(read('packages/core/tests/android_publication_boundary.hpp').includes('ENOSYS'));
  assert.ok(read('platforms/android/app/src/androidTest/java/ai/pixaura/app/AssetPublicationTest.kt').includes('nativeAndroidAssetPublication'));
});
