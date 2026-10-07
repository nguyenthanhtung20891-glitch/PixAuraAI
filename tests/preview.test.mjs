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
  assert.ok(read('packages/core/tests/decode_allocation_test.cpp').includes('phases=17'));
});

test('bounded preview and platform copies share native authority', () => {
  assert.ok(read('packages/core/include/pixaura/preview.h').includes('pixaura_preview_request'));
  assert.ok(read('packages/core/src/preview.cpp').includes('Validate even unsampled pixels'));
  assert.ok(read('platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/CoreProbe.kt').includes('Bitmap.createBitmap(pixels, 2, width'));
  assert.ok(read('packages/core/swift/Sources/ReferencePreview.swift').includes('CGImageAlphaInfo.last'));
  assert.ok(read('packages/core/tests/preview_test.cpp').includes('preview_not_started=true'));
  assert.ok(read('packages/core/tests/preview_test.cpp').includes('fit properties=2048'));
});

test('interactive lifecycle is wired into native and both platform gates', () => {
  for (const p of ['packages/core/include/pixaura/preview.h','packages/core/tests/preview_boundary.h']) assert.ok(read(p).includes('pixaura_preview_render_interactive'));
  assert.ok(read('packages/core/tests/preview_test.cpp').includes('interactive churn=10000'));
  assert.ok(read('packages/core/swift/Sources/ReferencePreview.swift').includes('pixaura_preview_current'));
  assert.doesNotMatch(read('packages/core/swift/Sources/ReferencePreview.swift'), /ObjectIdentifier/);
  assert.ok(read('platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/InteractivePreview.kt').includes('nativeCurrent(storage'));
});
