import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
const text = p => fs.readFileSync(p, 'utf8').replaceAll('\r\n', '\n');
const read = p => JSON.parse(text(p));
const files = ['AGENTS.md', 'ARCHITECTURE.md', 'DECISIONS.md', 'QUALITY_GATES.md', 'ROADMAP.md', 'docs/plans/phase-8-roadmap.md', 'docs/plans/phase-9-roadmap.md'];

test('Phase 9 Step 6 binds the export journey without closing the phase', () => {
  const m = read('docs/reports/phase-9-step-6-v1.json');
  assert.equal(m.status, 'CLOSURE_CERTIFICATION_PENDING');
  assert.equal(m.certified_implementation_sha, '62d7f2a54907234f92e6c12a2527dd59d6411122');
  assert.deepEqual(m.implementation_runs, [41990040556, 41990040528]);
  assert.equal(m.step5_closure_sha, '7e334b7962b1974bc4f674b9cc6818831dd408de');
  assert.equal(m.step5_implementation_sha, '6a30b1512c995df511e420945d845a92b85364dc');
  assert.equal(m.runtime_authority, false);
  assert.equal(m.editing_authority, false);
  assert.equal(m.history_authority, false);
  assert.equal(m.phase10, 'NOT_STARTED');
  const receipt = read('docs/reports/evidence/phase-9-step-5-closure-certification.json');
  assert.equal(receipt.sha, m.step5_closure_sha);
  assert.equal(receipt.label, 'closure');
  assert.equal(receipt.step5, 'COMPLETE_FULL_PASS');
  assert.equal(receipt.step6, 'AUTHORIZED_IN_PROGRESS');
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
  for (const term of ['pixaura_editor_action(&o.c,1)', 'pixaura_editor_action(&o.c,2)', 'pixaura_editor_action(&o.c,3)', 'actor=="ai"', 'PIXAURA_EXPORT_STALE', 'code==11', 'export hosted repeat-us', '!chunk(png.bytes,"eXIf")', '!jpeg_app1(jpeg.bytes)']) assert.ok(native.includes(term), term);
  const contract = text('docs/contracts/export-file-lifecycle-v1.md');
  for (const id of ['A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N']) assert.ok(contract.includes(`| ${id} |`), id);
  assert.equal(text('platforms/android/app/src/main/java/ai/pixaura/app/ExportDestination.kt').includes('http'), false);
  assert.equal(text('platforms/ios/PixAuraAI/ManualEditorModel.swift').includes('URL('), false);
  assert.ok(text('scripts/check-android-release.py').includes('pixaura_test_'));
  assert.ok(text('scripts/check-apple-release.py').includes('check-android-release.py'));
  for (const p of files) {
    const s = text(p).split('<!-- phase9-step6-current -->')[1]?.split('<!-- phase9-step6-history -->')[0];
    assert.ok(s, p);
    for (const term of ['Step 5 COMPLETE / FULL PASS', '7e334b7962b1974bc4f674b9cc6818831dd408de', '41981405571', '41981405588', '6a30b1512c995df511e420945d845a92b85364dc', '41976330668', '41976330587', 'Step 6 CLOSURE CERTIFICATION PENDING', '62d7f2a54907234f92e6c12a2527dd59d6411122', '41990040556', '41990040528', 'EXPORT IS NOT A REVISION', 'Schema 3 unchanged', 'HUMAN Apply unchanged', 'reference.assistance.v1 ADMITTED', 'reference.deterministic.v1', 'NONE REQUIRED / NONE ADMITTED', 'Vulkan/Metal UNKNOWN', 'NOT WAIVED', 'NOT SATISFIED', 'Phase 10 is not started']) assert.ok(s.includes(term), p + term);
  }
  const implementation = read('docs/reports/evidence/phase-9-step-6-implementation-certification.json');
  assert.equal(implementation.sha, m.certified_implementation_sha);
  assert.equal(implementation.label, 'implementation');
  assert.equal(implementation.step6, 'CLOSURE_CERTIFICATION_PENDING');
  assert.equal(implementation.phase10, 'NOT_STARTED');
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
  for (const p of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml']) assert.ok(text(p).includes('tests/export-e2e.test.mjs'), p);
});

test('observed Step 6 closure activates Phase 9 and leaves Phase 10 unstarted', () => {
  const closed = read('docs/reports/evidence/phase-9-step-6-closure-certification.json');
  assert.equal(closed.sha, '85ec8988a88f482318a3f78b97dd6e5287bd9eda');
  assert.equal(closed.label, 'closure');
  assert.equal(closed.phase9, 'CLOSED_COMPLETE_FULL_PASS');
  assert.equal(closed.step6, 'COMPLETE_FULL_PASS');
  assert.equal(closed.phase10, 'NOT_STARTED');
  assert.equal(closed.runs.length, 2);
  assert.equal(closed.runs[0].jobs.length, 7);
  assert.equal(closed.runs[1].jobs.length, 3);
  for (const run of closed.runs) {
    assert.equal(run.head_sha, closed.sha);
    assert.equal(run.attempt, 1);
    assert.equal(run.status, 'completed');
    assert.equal(run.conclusion, 'success');
    for (const job of run.jobs) assert.equal(job.conclusion, 'success');
  }
  for (const p of files) {
    const s = text(p).split('<!-- phase9-closed-current -->')[1]?.split('<!-- phase9-closed-history -->')[0];
    assert.ok(s, p);
    for (const term of ['Phase 9 CLOSED / COMPLETE / FULL PASS', '85ec8988a88f482318a3f78b97dd6e5287bd9eda', '41995816960', '41995816721', '62d7f2a54907234f92e6c12a2527dd59d6411122', '41990040556', '41990040528', 'EXPORT IS NOT A REVISION', 'Schema 3 unchanged', 'HUMAN Apply unchanged', 'reference.assistance.v1 ADMITTED', 'reference.deterministic.v1', 'NONE REQUIRED / NONE ADMITTED', 'Vulkan/Metal UNKNOWN', 'NOT WAIVED', 'NOT SATISFIED', 'Phase 10 is not started', 'Phase 11 and Phase 12 are not started']) assert.ok(s.includes(term), p + term);
    const pending = text(p).split('<!-- phase9-step6-current -->')[1]?.split('<!-- phase9-step6-history -->')[0];
    assert.ok(pending.includes('Step 6 CLOSURE CERTIFICATION PENDING'), p);
  }
  const proposal = text('docs/plans/phase-10-readiness.md');
  assert.ok(proposal.includes('NOT STARTED'));
  assert.ok(proposal.includes('Phase 10 implementation is not started'));
  assert.ok(text('docs/reports/phase-9-closure.md').includes('85ec8988a88f482318a3f78b97dd6e5287bd9eda'));
  assert.ok(text('AGENTS.md').split('<!-- phase8-closed-current -->')[1].includes('Phase 9 export is not started'));
});
