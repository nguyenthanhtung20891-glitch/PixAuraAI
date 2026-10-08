import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import { spawnSync } from 'node:child_process';
import { fileURLToPath } from 'node:url';
import { validateEvidence } from '../scripts/gpu-android-evidence.mjs';
const read = p => fs.readFileSync(new URL(`../${p}`, import.meta.url), 'utf8');
const run = 'a'.repeat(32), sha = 'b'.repeat(40);
const current = { validation_run: run, source_sha: sha, status: 'PASS', hardware: true,
  controlled_hardware_gate: true, pipeline: true, dispatch: true,
  parity_passed: 14, test_count: 14, expected_test_count: 14, gpu: 'synthetic regression fixture' };

test('A: current invocation evidence is readable through the actual validator CLI', () => {
  const root = new URL('../build/gpu-evidence-tests/', import.meta.url);
  fs.mkdirSync(root, { recursive: true });
  const dir = fs.mkdtempSync(path.join(fileURLToPath(root), 'run-'));
  try {
    const file = path.join(dir, 'evidence.json');
    fs.writeFileSync(file, JSON.stringify(current));
    const result = spawnSync(process.execPath, ['scripts/gpu-android-evidence.mjs', file, run, sha, '0'], { encoding: 'utf8' });
    assert.equal(result.status, 0, result.stderr);
    assert.match(result.stdout, /14\/14/);
  } finally { fs.rmSync(dir, { recursive: true }); }
});
test('B: stale evidence from a prior invocation is rejected', () => {
  assert.throws(() => validateEvidence(JSON.stringify(current), 'c'.repeat(32), sha, 0), /validation_run/);
});
test('C: mismatched validation_run is rejected', () => {
  assert.throws(() => validateEvidence(JSON.stringify({ ...current, validation_run: 'd'.repeat(32) }), run, sha, 0), /validation_run/);
});
test('D: mismatched source SHA is rejected', () => {
  assert.throws(() => validateEvidence(JSON.stringify({ ...current, source_sha: 'e'.repeat(40) }), run, sha, 0), /source SHA/);
});
test('E: missing evidence fails even with a successful instrumentation process', () => {
  assert.throws(() => validateEvidence('', run, sha, 0), /Missing/);
  const result = spawnSync(process.execPath, ['scripts/gpu-android-evidence.mjs', 'build/gpu-evidence-tests/nonexistent.json', run, sha, '0']);
  assert.equal(result.status, 1);
});
test('F: transport selects only an exclusive current-run artifact, never a previous installed test', () => {
  const wrapper = read('scripts/check-gpu-android-hardware.ps1');
  assert.match(wrapper, /Guid\]::NewGuid/);
  assert.match(wrapper, /mkdir '\$deviceRunPath'/);
  assert.match(wrapper, /head -c 16385 '\$deviceRunPath\/evidence.json'/);
  assert.doesNotMatch(wrapper, /run-as|cat files\/gpu-hardware|Get-ChildItem/);
  const producer = read('platforms/android/app/src/androidTest/java/ai/pixaura/app/GpuHardwareTest.kt');
  assert.match(read('scripts/publish-gpu-android-evidence.sh'), /set -C/);
  assert.match(producer, /pixaura-gpu-\$validationRun\/publish.sh/);
  assert.match(producer, /executeShellCommand/);
  assert.match(producer, /bytes.size <= 16384/);
  const root = new URL('../build/gpu-evidence-tests/', import.meta.url);
  fs.mkdirSync(root, { recursive: true });
  const dir = fs.mkdtempSync(path.join(fileURLToPath(root), 'previous-install-'));
  try {
    // Even a valid-looking previous installation cannot satisfy a missing current artifact.
    fs.mkdirSync(path.join(dir, 'previous-app-files'));
    fs.writeFileSync(path.join(dir, 'previous-app-files', 'gpu-hardware.json'), JSON.stringify(current));
    const result = spawnSync(process.execPath, ['scripts/gpu-android-evidence.mjs',
      path.join(dir, 'current-run-evidence.json'), run, sha, '0']);
    assert.equal(result.status, 1);
  } finally { fs.rmSync(dir, { recursive: true }); }
});
test('incomplete parity, software/fallback, failed tests and oversized diagnostics cannot certify hardware', () => {
  for (const change of [{ parity_passed: 13 }, { test_count: 13 }, { expected_test_count: 13 },
    { hardware: false }, { controlled_hardware_gate: false }, { pipeline: false }, { dispatch: false }, { status: 'UNSUPPORTED' }]) {
    assert.throws(() => validateEvidence(JSON.stringify({ ...current, ...change }), run, sha, 0), /did not PASS/);
  }
  assert.throws(() => validateEvidence(JSON.stringify(current), run, sha, 1), /did not PASS/);
  assert.throws(() => validateEvidence(' '.repeat(16385), run, sha, 0), /oversized/);
});

test('hardware certification remains separate from hosted compilation and regression', () => {
  const android = read('platforms/android/app/src/androidTest/java/ai/pixaura/app/GpuHardwareTest.kt');
  assert.match(android, /pixauraHardware/);
  assert.match(android, /UNSUPPORTED/);
  assert.match(android, /assertEquals\(evidence.toString\(\), "PASS"/);
  assert.match(read('scripts/check-android-emulator.sh'), /swiftshader_indirect/);
  const report = read('docs/reports/phase-2-exit-closure.md');
  assert.match(report, /^Status: PHASE 2 CLOSED/m);
  assert.match(report, /DH-APPLE-METAL-01/);
  assert.match(report, /not waived or removed/);
  assert.match(report, /before Beta readiness completion, production release or any claim of physical Apple GPU certification/);
  assert.match(read('docs/reports/phase-2-gpu-android-hardware.md'), /^Status: ACCEPTED PHYSICAL-DEVICE ANDROID VULKAN CERTIFICATION/m);
  assert.match(read('docs/reports/phase-2-gpu-apple-hardware.md'), /^Status: DEFERRED \/ PENDING REAL HARDWARE EXECUTION/m);
  assert.match(read('QUALITY_GATES.md'), /Promotion rule.*remain blocked while this gate is DEFERRED/);
});

test('both platform adapters use the shared numerical authority and owned shaders', () => {
  for (const file of ['CMakeLists.txt', 'packages/core/Package.swift']) assert.match(read(file), /src\/gpu.cpp/);
  assert.match(read('platforms/android/native/gpu_vulkan.cpp'), /pixaura_gpu_tile/);
  assert.match(read('packages/core/swift/Sources/MetalValidation.swift'), /pixaura_gpu_tile/);
  assert.match(read('platforms/android/native/CMakeLists.txt'), /exposure.comp/);
  assert.match(read('scripts/check-apple.sh'), /check-metal-shaders.sh/);
  assert.match(read('packages/core/Package.swift'), /\.copy\("Shaders"\)/);
});
