import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import crypto from 'node:crypto';
const text = p => fs.readFileSync(p, 'utf8').replaceAll('\r\n', '\n');
const read = p => JSON.parse(text(p));
const hash = p => crypto.createHash('sha256').update(text(p)).digest('hex');
const files = ['AGENTS.md', 'ARCHITECTURE.md', 'DECISIONS.md', 'QUALITY_GATES.md', 'ROADMAP.md', 'docs/plans/phase-8-roadmap.md', 'docs/plans/phase-9-roadmap.md'];

test('Phase 9 Step 1 freezes export as a projection and leaves the engine unstarted', () => {
  const m = read('docs/reports/phase-9-step-1-v1.json');
  assert.equal(m.baseline, '45d121a2e0dd2fce8e925d3e36b3289a696c136a');
  assert.equal(m.phase8_closure_sha, '029f84eaa86f3e5a8bffc696a6abd10f9477ae2a');
  assert.equal(m.status, 'AUTHORIZED_IN_PROGRESS');
  assert.equal(m.runtime_authority, false);
  assert.equal(m.editing_authority, false);
  assert.equal(m.history_authority, false);
  assert.equal(m.step2, 'NOT_STARTED');
  for (const [p, digest] of Object.entries(m.additions)) assert.equal(hash(p), digest, p);
  assert.equal(m.icc_preserved, false);
  assert.equal(m.location_metadata_copied, false);
  const contract = text('docs/contracts/export-file-lifecycle-v1.md');
  for (const term of ['EXPORT IS NOT A REVISION', 'IDLE', 'VALIDATING', 'RENDERING', 'ENCODING', 'READY_TO_SAVE', 'SAVING', 'SUCCEEDED', 'FAILED', 'CANCELLED', 'STALE', 'JPEG', 'PNG', 'GPS', 'opaque white', 'share sheet is not SUCCEEDED']) assert.ok(contract.includes(term), term);
  assert.ok(text('docs/adr/0023-export-is-a-projection.md').includes('libjpeg-turbo 3.2.0'));
  assert.ok(!text('packages/core/include/pixaura/decode.h').includes('pixaura_export_'));
  for (const p of files) {
    const s = text(p).split('<!-- phase9-step1-current -->')[1]?.split('<!-- phase9-step1-history -->')[0];
    assert.ok(s, p);
    for (const term of ['Phase 8 CLOSED / COMPLETE / FULL PASS', '029f84eaa86f3e5a8bffc696a6abd10f9477ae2a', '41709039261', '41709039298', 'Step 1 AUTHORIZED / IN PROGRESS', 'Steps 2-6 NOT STARTED', 'EXPORT IS NOT A REVISION', 'Schema 3 unchanged', 'HUMAN Apply unchanged', 'reference.assistance.v1 ADMITTED', 'reference.deterministic.v1', 'NONE REQUIRED / NONE ADMITTED', 'Vulkan/Metal UNKNOWN', 'NOT WAIVED', 'NOT SATISFIED']) assert.ok(s.includes(term), p + term);
  }
  const plan = text('docs/plans/phase-9-roadmap.md');
  assert.ok(plan.includes('| 1 | Export Contract and Authority | AUTHORIZED / IN PROGRESS |'));
  assert.ok(plan.includes('| 6 | End-to-end Certification and Phase Closure | NOT STARTED |'));
  for (const p of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml']) assert.ok(text(p).includes('tests/export-contract.test.mjs'), p);
});
