import assert from 'node:assert/strict';
import fs from 'node:fs';
import test from 'node:test';

const read = (name) => fs.readFileSync(name, 'utf8');

test('Phase 10.1 records a blocked Mac Studio gate without fabricated host output', () => {
  const agents = read('AGENTS.md');
  const block = agents.split('<!-- phase10.1-history -->')[0];
  assert.match(block, /Phase 10\.1 AUTHORIZED \/ IN PROGRESS \/ LOCAL HARDWARE GATE BLOCKED/);
  assert.match(block, /No Mac Studio command output was collected/);
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
