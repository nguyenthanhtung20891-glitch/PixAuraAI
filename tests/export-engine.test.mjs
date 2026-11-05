import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import crypto from 'node:crypto';
const text = p => fs.readFileSync(p, 'utf8').replaceAll('\r\n', '\n');
const read = p => JSON.parse(text(p));
const hash = p => crypto.createHash('sha256').update(text(p)).digest('hex');
const files = ['AGENTS.md', 'ARCHITECTURE.md', 'DECISIONS.md', 'QUALITY_GATES.md', 'ROADMAP.md', 'docs/plans/phase-8-roadmap.md', 'docs/plans/phase-9-roadmap.md'];

test('Phase 9 Step 2 encodes one committed revision and leaves history untouched', () => {
  const m = read('docs/reports/phase-9-step-2-v1.json');
  assert.equal(m.status, 'CLOSURE_CERTIFICATION_PENDING');
  assert.equal(m.certified_implementation_sha, '64d854d7ec2a5e11787cd0b94ff60b0fdfe7a8e3');
  assert.deepEqual(m.implementation_runs, [41928862538, 41928862545]);
  assert.equal(m.step1_closure_sha, '49ddb7fc9ebc8173e716a9cd3f8a25d18564b3ab');
  assert.equal(m.step1_implementation_sha, '64c01c1fba688ff266951ce2eb608982331323f8');
  assert.equal(m.runtime_authority, false);
  assert.equal(m.editing_authority, false);
  assert.equal(m.history_authority, false);
  assert.equal(m.step3, 'NOT_STARTED');
  assert.equal(m.icc_preserved, false);
  assert.equal(m.location_metadata_copied, false);
  assert.equal(m.encoder.new_dependency, false);
  assert.deepEqual(m.formats, ['jpeg', 'png']);
  for (const [p, digest] of Object.entries(m.additions)) assert.equal(hash(p), digest, p);
  for (const [p, change] of Object.entries(m.changes)) assert.equal(hash(p), change.after_sha256_lf, p);
  const header = text('packages/core/include/pixaura/export.h');
  for (const term of ['pixaura_export_bind', 'pixaura_export_encode', 'pixaura_export_cancel', 'PIXAURA_EXPORT_STALE', 'EXPORT IS NOT A REVISION']) assert.ok(header.includes(term), term);
  const boundary = text('packages/core/src/export_boundary.hpp');
  for (const term of ['pixaura_working_evaluate_cancel', 'pixaura_preview_render', 'try_lock', 'PIXAURA_EXPORT_STALE']) assert.ok(boundary.includes(term), term);
  for (const term of ['repository->apply', 'pixaura_editor_agent_approve', 'pixaura_editor_agent_execute']) assert.equal(boundary.includes(term), false, term);
  assert.equal(text('packages/core/src/editor_api.cpp').includes('budget'), false);
  assert.ok(text('packages/core/codec-sources.cmake').includes('pngwrite.c'));
  assert.ok(text('packages/core/codec-sources.cmake').includes('deflate.c'));
  for (const source of ['pngwrite.c', 'pngwio.c', 'pngwtran.c', 'pngwutil.c', 'deflate.c', 'trees.c']) {
    assert.ok(text('packages/core/Package.swift').includes(source), source);
  }
  assert.ok(text('docs/DEPENDENCIES.md').includes('Owned pnglibconf enables PNG write'));
  const receipt = read('docs/reports/evidence/phase-9-step-1-closure-certification.json');
  assert.equal(receipt.sha, m.step1_closure_sha);
  assert.equal(receipt.label, 'closure');
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
  for (const p of files) {
    const s = text(p).split('<!-- phase9-step2-current -->')[1]?.split('<!-- phase9-step2-history -->')[0];
    assert.ok(s, p);
    for (const term of ['Step 1 COMPLETE / FULL PASS', '49ddb7fc9ebc8173e716a9cd3f8a25d18564b3ab', '41911149886', '41911149642', 'Step 2 CLOSURE CERTIFICATION PENDING', '64d854d7ec2a5e11787cd0b94ff60b0fdfe7a8e3', '41928862538', '41928862545', 'EXPORT IS NOT A REVISION', 'Schema 3 unchanged', 'HUMAN Apply unchanged', 'reference.assistance.v1 ADMITTED', 'reference.deterministic.v1', 'NONE REQUIRED / NONE ADMITTED', 'Vulkan/Metal UNKNOWN', 'NOT WAIVED', 'NOT SATISFIED', 'Steps 3-6 NOT STARTED']) assert.ok(s.includes(term), p + term);
  }
  const plan = text('docs/plans/phase-9-roadmap.md');
  assert.ok(plan.includes('| 1 | Export Contract and Authority | CLOSURE CERTIFICATION PENDING |'));
  assert.ok(plan.includes('| 2 | Native Deterministic Export Engine | CLOSURE CERTIFICATION PENDING |'));
  const implementation = read('docs/reports/evidence/phase-9-step-2-implementation-certification.json');
  assert.equal(implementation.sha, m.certified_implementation_sha);
  assert.equal(implementation.label, 'implementation');
  assert.equal(implementation.step2, 'CLOSURE_CERTIFICATION_PENDING');
  assert.equal(implementation.step3, 'NOT_STARTED');
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
  assert.ok(plan.includes('| 3 | Android Save and Share | CLOSURE CERTIFICATION PENDING |'));
  for (const p of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml']) assert.ok(text(p).includes('tests/export-engine.test.mjs'), p);
});
