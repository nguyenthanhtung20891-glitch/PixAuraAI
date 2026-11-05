import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
const text = p => fs.readFileSync(p, 'utf8').replaceAll('\r\n', '\n');
const read = p => JSON.parse(text(p));
const files = ['AGENTS.md', 'ARCHITECTURE.md', 'DECISIONS.md', 'QUALITY_GATES.md', 'ROADMAP.md', 'docs/plans/phase-8-roadmap.md', 'docs/plans/phase-9-roadmap.md'];

test('Phase 9 Step 5 hardens export lifecycle without a new history or resource ceiling', () => {
  const m = read('docs/reports/phase-9-step-5-v1.json');
  assert.equal(m.status, 'CLOSURE_CERTIFICATION_PENDING');
  assert.equal(m.certified_implementation_sha, '6a30b1512c995df511e420945d845a92b85364dc');
  assert.deepEqual(m.implementation_runs, [41976330668, 41976330587]);
  assert.equal(m.step4_closure_sha, 'b04fd291cda9e88e6a03ad20140f634294b35168');
  assert.equal(m.step4_implementation_sha, '6a6b8d30ba69316822dcffafbee6894e4d1f0271');
  assert.equal(m.runtime_authority, false);
  assert.equal(m.editing_authority, false);
  assert.equal(m.history_authority, false);
  assert.equal(m.new_resource_ceiling, false);
  assert.equal(m.hosted_timings_are_device_claims, false);
  assert.equal(m.step6, 'NOT_STARTED');
  const receipt = read('docs/reports/evidence/phase-9-step-4-closure-certification.json');
  assert.equal(receipt.sha, m.step4_closure_sha);
  assert.equal(receipt.label, 'closure');
  assert.equal(receipt.step4, 'COMPLETE_FULL_PASS');
  assert.equal(receipt.step5, 'AUTHORIZED_IN_PROGRESS');
  assert.equal(receipt.runs.length, 2);
  assert.equal(receipt.runs[0].jobs.length, 7);
  assert.equal(receipt.runs[1].jobs.length, 3);
  for (const run of receipt.runs) {
    assert.equal(run.head_sha, receipt.sha);
    assert.equal(run.attempt, 1);
    assert.equal(run.status, 'completed');
    assert.equal(run.conclusion, 'success');
    for (const job of run.jobs) assert.equal(job.conclusion, 'success');
  }
  const native = text('packages/core/tests/export_test.cpp');
  for (const term of ['export hosted repeat-us', 'revisions().size()==1', 'encode(first,first.bind(),32).code==11']) assert.ok(native.includes(term), term);
  for (const p of files) {
    const s = text(p).split('<!-- phase9-step5-current -->')[1]?.split('<!-- phase9-step5-history -->')[0];
    assert.ok(s, p);
    for (const term of ['Step 4 COMPLETE / FULL PASS', 'b04fd291cda9e88e6a03ad20140f634294b35168', '41971443062', '41971442960', '6a6b8d30ba69316822dcffafbee6894e4d1f0271', '41967410493', '41967410497', 'Step 5 CLOSURE CERTIFICATION PENDING', '6a30b1512c995df511e420945d845a92b85364dc', '41976330668', '41976330587', 'EXPORT IS NOT A REVISION', 'No new process-RSS ceiling', 'Schema 3 unchanged', 'HUMAN Apply unchanged', 'reference.assistance.v1 ADMITTED', 'reference.deterministic.v1', 'NONE REQUIRED / NONE ADMITTED', 'Vulkan/Metal UNKNOWN', 'NOT WAIVED', 'NOT SATISFIED', 'Step 6 NOT STARTED']) assert.ok(s.includes(term), p + term);
  }
  assert.ok(text('docs/plans/phase-9-roadmap.md').includes('| 5 | File Lifecycle Hardening | CLOSURE CERTIFICATION PENDING |'));
  const implementation = read('docs/reports/evidence/phase-9-step-5-implementation-certification.json');
  assert.equal(implementation.sha, m.certified_implementation_sha);
  assert.equal(implementation.label, 'implementation');
  assert.equal(implementation.step5, 'CLOSURE_CERTIFICATION_PENDING');
  assert.equal(implementation.step6, 'NOT_STARTED');
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
  assert.ok(text('docs/plans/phase-9-roadmap.md').includes('| 6 | End-to-end Certification and Phase Closure | CLOSURE CERTIFICATION PENDING |'));
  for (const p of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml']) assert.ok(text(p).includes('tests/export-lifecycle.test.mjs'), p);
});
