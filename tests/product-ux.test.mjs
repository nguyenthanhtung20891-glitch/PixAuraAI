import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import crypto from 'node:crypto';
const text = p => fs.readFileSync(p, 'utf8').replaceAll('\r\n', '\n');
const read = p => JSON.parse(text(p));
const hash = p => crypto.createHash('sha256').update(text(p)).digest('hex');
const files = ['AGENTS.md', 'ARCHITECTURE.md', 'DECISIONS.md', 'QUALITY_GATES.md', 'ROADMAP.md', 'docs/plans/phase-8-roadmap.md', 'docs/plans/phase-9-roadmap.md', 'docs/plans/phase-10-roadmap.md'];

test('Phase 10 Step 1 freezes product UX without changing native authority', () => {
  const m = read('docs/reports/phase-10-step-1-v1.json');
  assert.equal(m.baseline, '54e7be1c910cde31bacc3474f7ab0e8fdcc2b954');
  assert.equal(m.phase9_closure_sha, '85ec8988a88f482318a3f78b97dd6e5287bd9eda');
  assert.equal(m.status, 'CLOSURE_CERTIFICATION_PENDING');
  assert.equal(m.certified_implementation_sha, '5b3d3a15485f159b056861aee49ca56169ce8030');
  assert.deepEqual(m.implementation_runs, [42040586837, 42040586934]);
  assert.equal(m.runtime_authority, false);
  assert.equal(m.editing_authority, false);
  assert.equal(m.history_authority, false);
  assert.equal(m.step2, 'NOT_STARTED');
  assert.deepEqual(m.languages, ['en', 'vi']);
  for (const [p, digest] of Object.entries(m.additions)) assert.equal(hash(p), digest, p);
  const contract = text('docs/contracts/product-ux-release-readiness-v1.md');
  for (const term of ['UI STATE != NATIVE AUTHORITY', 'Open JPEG or PNG', 'After Preview — not committed', 'Apply reviewed plan', 'Restore original review content — not document undo', 'Undo document', 'Redo document', 'Committed result', 'Share sheet closed. Delivery to another app is not confirmed.', 'Save cancelled', 'Export failed', 'Cancelled', 'Failed', 'Stale', 'English', 'Vietnamese']) assert.ok(contract.includes(term), term);
  for (const id of ['A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P']) assert.ok(contract.includes(`| ${id} |`), id);
  for (const p of files) {
    const s = text(p).split('<!-- phase10-step1-current -->')[1]?.split('<!-- phase10-step1-history -->')[0];
    assert.ok(s, p);
    for (const term of ['Phase 9 CLOSED / COMPLETE / FULL PASS', '85ec8988a88f482318a3f78b97dd6e5287bd9eda', '41995816960', '41995816721', 'Phase 10 AUTHORIZED / IN PROGRESS', 'Step 1 CLOSURE CERTIFICATION PENDING', '5b3d3a15485f159b056861aee49ca56169ce8030', '42040586837', '42040586934', 'Steps 2-6 NOT STARTED', 'UI STATE != NATIVE AUTHORITY', 'EXPORT IS NOT A REVISION', 'Schema 3 unchanged', 'HUMAN Apply unchanged', 'reference.assistance.v1 ADMITTED', 'reference.deterministic.v1', 'NONE REQUIRED / NONE ADMITTED', 'Vulkan/Metal UNKNOWN', 'NOT WAIVED', 'NOT SATISFIED', 'Phase 11 is not started', 'Phase 12 is not started']) assert.ok(s.includes(term), p + term);
    const prior = text(p).split('<!-- phase9-closed-current -->')[1];
    if (prior) assert.ok(prior.includes('Phase 10 is not started'), p);
  }
  const plan = text('docs/plans/phase-10-roadmap.md');
  assert.ok(plan.includes('| 1 | Product UX Contract and Information Architecture | CLOSURE CERTIFICATION PENDING |'));
  assert.ok(plan.includes('| 2 | Onboarding, Empty States and Permissions | CLOSURE CERTIFICATION PENDING |'));
  assert.ok(plan.includes('| 6 | Cross-Platform Software Release Readiness and Phase Closure | NOT STARTED |'));
  assert.ok(text('docs/plans/phase-10-readiness.md').includes('Phase 10 implementation is not started'));
  const implementation = read('docs/reports/evidence/phase-10-step-1-implementation-certification.json');
  assert.equal(implementation.sha, m.certified_implementation_sha);
  assert.equal(implementation.label, 'implementation');
  assert.equal(implementation.step1, 'CLOSURE_CERTIFICATION_PENDING');
  assert.equal(implementation.step2, 'NOT_STARTED');
  assert.equal(implementation.runs.length, 2);
  assert.equal(implementation.runs[0].jobs.length, 7);
  assert.equal(implementation.runs[1].jobs.length, 3);
  for (const run of implementation.runs) {
    assert.equal(run.head_sha, implementation.sha);
    assert.equal(run.attempt, 1);
    assert.equal(run.status, 'completed');
    assert.equal(run.conclusion, 'success');
    for (const job of run.jobs) assert.equal(job.conclusion, 'success');
  }
  for (const p of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml']) assert.ok(text(p).includes('tests/product-ux.test.mjs'), p);
});
