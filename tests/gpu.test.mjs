import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
const read = p => fs.readFileSync(new URL(`../${p}`, import.meta.url), 'utf8');

test('hardware certification remains separate from hosted compilation and regression', () => {
  const android = read('platforms/android/app/src/androidTest/java/ai/pixaura/app/GpuHardwareTest.kt');
  assert.match(android, /pixauraHardware/);
  assert.match(android, /UNSUPPORTED/);
  assert.match(android, /assertEquals\(evidence.toString\(\), "PASS"/);
  assert.match(read('scripts/check-android-emulator.sh'), /swiftshader_indirect/);
  const report = read('docs/reports/phase-2-exit-closure.md');
  assert.match(report, /BLOCKED/);
  assert.match(report, /NOT CLOSED/);
  for (const platform of ['android', 'apple']) {
    assert.match(read(`docs/reports/phase-2-gpu-${platform}-hardware.md`), /PENDING REAL HARDWARE EXECUTION/);
  }
});

test('both platform adapters use the shared numerical authority and owned shaders', () => {
  for (const file of ['CMakeLists.txt', 'packages/core/Package.swift']) assert.match(read(file), /src\/gpu.cpp/);
  assert.match(read('platforms/android/native/gpu_vulkan.cpp'), /pixaura_gpu_tile/);
  assert.match(read('packages/core/swift/Sources/MetalValidation.swift'), /pixaura_gpu_tile/);
  assert.match(read('platforms/android/native/CMakeLists.txt'), /exposure.comp/);
  assert.match(read('scripts/check-apple.sh'), /check-metal-shaders.sh/);
  assert.match(read('packages/core/Package.swift'), /\.copy\("Shaders"\)/);
});
