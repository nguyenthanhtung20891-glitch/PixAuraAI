import { phase11Hash } from './helpers/phase-11-hashes.mjs';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import test from 'node:test';

const read = (name) => fs.readFileSync(name, 'utf8');

test('Phase 10.1 records a blocked Mac Studio gate without fabricated host output', () => {
  const agents = read('AGENTS.md');
  const block = agents.split('<!-- phase10.1-history -->')[0];
  assert.match(block, /Phase 10\.1 AUTHORIZED \/ IN PROGRESS \/ LOCAL HARDWARE GATE BLOCKED/);
  assert.match(block, /No Mac Studio command output was collected/);
  assert.match(block, /96f5c5697f10c6b5060681135dfab4c11d23dd03 is not certified/);
  assert.match(block, /24466359f9cc6daf89409e08e756d7a6856762f1 is not certified/);
  assert.match(block, /Phase 10\.1 is not closed/);
  assert.match(block, /Phase 11 is not started/);
  assert.match(block, /Phase 12 is not started/);
  assert.match(block, /DH-APPLE-METAL-01 DEFERRED \/ NOT WAIVED \/ NOT SATISFIED/);
  assert.match(agents.split('<!-- phase10-closed-history -->')[0], /Phase 10\.1 is not started/);
  const report = JSON.parse(read('docs/reports/phase-10.1-v1.json'));
  assert.equal(report.status, 'LOCAL_HARDWARE_GATE_BLOCKED');
  assert.equal(report.mac_studio_observed, false);
  assert.equal(report.local_apple_tests_observed, false);
  assert.equal(report.performance_samples_observed, false);
  assert.equal(report.closure_sha, null);
  for (const name of [
    'docs/reports/phase-10.1-step-1.md',
    'docs/reports/phase-10.1-step-2.md',
    'docs/reports/phase-10.1-step-4.md',
    'docs/reports/phase-10.1-closure.md',
  ]) {
    assert.match(read(name), /LOCAL HARDWARE GATE BLOCKED|NOT CLOSED/);
    assert.doesNotMatch(read(name), /ProductName:\s*macOS/);
  }
  const roadmap = read('docs/plans/phase-10.1-roadmap.md');
  assert.match(roadmap, /\| 1 \| Mac Studio development environment \| LOCAL HARDWARE GATE BLOCKED \|/);
  assert.match(roadmap, /\| 5 \| Integrated certification and phase closure \| NOT CLOSED \|/);
  assert.match(read('docs/plans/phase-10.1-readiness.md'), /Status: NOT STARTED/);
  const script = read('scripts/check-mac-studio-host.sh');
  assert.match(script, /Xcode_26\.6\.app\/Contents\/Developer/);
  assert.doesNotMatch(script, /xcode-select -s/);
  assert.match(script, /requires Darwin/);
  for (const workflow of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml']) {
    assert.match(read(workflow), /tests\/phase-10\.1-hardware-gate\.test\.mjs/);
  }
});

test('Mac successor binds observed host evidence to unchanged dual-platform product sources', async () => {
  const crypto = await import('node:crypto');
  const sha = (data) => crypto.createHash('sha256').update(data).digest('hex');
  const base = 'docs/reports/evidence/phase-10.1-local/';
  const manifest = JSON.parse(read(base + 'product-source-manifest.json'));
  assert.equal(manifest.baseline_sha, '9184cf4d6c58ada39ba1b0538d4c32eedf6853d8');
  assert.ok(Object.keys(manifest.files).some((p) => p.startsWith('platforms/android/')));
  assert.ok(Object.keys(manifest.files).some((p) => p.startsWith('platforms/ios/')));
  assert.ok(Object.keys(manifest.files).some((p) => p.startsWith('packages/core/src/')));
  for (const [path, digest] of Object.entries(manifest.files)) {
    assert.equal(sha(read(path).replaceAll('\r\n', '\n')), phase11Hash(path, digest), path);
  }
  const measurements = JSON.parse(read(base + 'host-measurements.json'));
  assert.equal(measurements.source_sha, manifest.baseline_sha);
  assert.equal(measurements.measured_samples, 7);
  assert.equal(measurements.warmups, 2);
  assert.deepEqual(measurements.cases.map((c) => [c.width, c.height]), [[1024, 768], [3000, 2000]]);
  for (const [path, digest] of Object.entries(measurements.harness_sha256)) {
    assert.equal(sha(read(path).replaceAll('\r\n', '\n')), phase11Hash(path, digest), path);
  }
  for (const c of measurements.cases) {
    assert.equal(c.samples.filter((s) => !s.warmup).length, 7);
    assert.ok(c.samples.every((s) => s.peak_rss_bytes > 0 && s.png_bytes > 0 && s.jpeg_bytes > 0));
  }
  const successor = read('docs/plans/phase-10.1-mac-successor.md');
  assert.match(successor, /primary development environment for BOTH platforms/);
  assert.match(successor, /historical two-machine experiment did not occur/);
  assert.match(successor, /DH-APPLE-METAL-01 retains every technical prerequisite/);
  for (const kind of ['ios', 'core-sim', 'core-host']) {
    const summary = JSON.parse(read(base + kind + '-retained-summary.json'));
    assert.equal(summary.result, 'Passed');
    assert.equal(summary.failedTests, 0);
    assert.equal(summary.skippedTests, 0);
  }
  const android = JSON.parse(read(base + 'android-retained-summary.json'));
  assert.equal(Number(android.counts.tests), 58);
  assert.equal(Number(android.counts.failures), 0);
  assert.equal(Number(android.counts.skipped), 0);
});
