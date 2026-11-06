import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
const text = p => fs.readFileSync(p, 'utf8').replaceAll('\r\n', '\n');
const read = p => JSON.parse(text(p));
const files = ['AGENTS.md', 'ARCHITECTURE.md', 'DECISIONS.md', 'QUALITY_GATES.md', 'ROADMAP.md', 'docs/plans/phase-8-roadmap.md', 'docs/plans/phase-9-roadmap.md', 'docs/plans/phase-10-roadmap.md'];

test('Phase 10 Step 4 recovers through the existing owner without a new authority', () => {
  const m = read('docs/reports/phase-10-step-4-v1.json');
  assert.equal(m.status, 'CLOSURE_CERTIFICATION_PENDING');
  assert.equal(m.implementation_sha, '359c2f7ffd7e70b0910019ab8c38acafa29848fb');
  assert.deepEqual(m.implementation_runs, [42134548234, 42134548288]);
  assert.equal(m.step3_closure_sha, 'b2e5809aba185f27c4362b5a56c1b448953ed743');
  assert.equal(m.step3_implementation_sha, '8e5a64c51acb87f25b76bafb5bc1106dadfca686');
  assert.equal(m.runtime_authority, false);
  assert.equal(m.editing_authority, false);
  assert.equal(m.history_authority, false);
  assert.equal(m.hidden_retry, false);
  assert.equal(m.hidden_apply, false);
  assert.equal(m.step5, 'NOT_STARTED');
  const receipt = read('docs/reports/evidence/phase-10-step-3-closure-certification.json');
  assert.equal(receipt.sha, m.step3_closure_sha);
  assert.equal(receipt.label, 'closure');
  assert.equal(receipt.step3, 'COMPLETE_FULL_PASS');
  assert.equal(receipt.step4, 'AUTHORIZED_IN_PROGRESS');
  assert.equal(receipt.runs[0].jobs.length, 7);
  assert.equal(receipt.runs[1].jobs.length, 3);
  for (const run of receipt.runs) {
    assert.equal(run.head_sha, receipt.sha);
    assert.equal(run.attempt, 1);
    assert.equal(run.conclusion, 'success');
    for (const job of run.jobs) assert.equal(job.conclusion, 'success');
  }
  const implementation = read('docs/reports/evidence/phase-10-step-4-implementation-certification.json');
  assert.equal(implementation.sha, m.implementation_sha);
  assert.equal(implementation.label, 'implementation');
  assert.equal(implementation.step4, 'CLOSURE_CERTIFICATION_PENDING');
  assert.equal(implementation.step5, 'NOT_STARTED');
  assert.equal(implementation.runs[0].jobs.length, 7);
  assert.equal(implementation.runs[1].jobs.length, 3);
  for (const run of implementation.runs) {
    assert.equal(run.head_sha, implementation.sha);
    assert.equal(run.attempt, 1);
    assert.equal(run.conclusion, 'success');
    for (const job of run.jobs) assert.equal(job.conclusion, 'success');
  }
  const androidScreen = text('platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorScreen.kt');
  const androidModel = text('platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorModel.kt');
  const appleScreen = text('platforms/ios/PixAuraAI/ManualEditorScreen.swift');
  const appleModel = text('platforms/ios/PixAuraAI/ManualEditorModel.swift');
  assert.ok(androidScreen.includes('if (event == Lifecycle.Event.ON_STOP) { model.cancelExport(); model.interrupt() }'));
  assert.ok(appleScreen.includes('if $0 != .active { model.cancelExport(); model.interrupt() }'));
  assert.ok(androidModel.includes('fun interrupt() { invalidateAdvice(AssistStatus.CANCELLED); command { pendingValues.set(null); pendingPreview.set(null); owner?.action(4);'));
  assert.ok(appleModel.includes('func interrupt() { invalidateAdvice(.cancelled); cancelExport(); send { try $0.action(4) } }'));
  assert.ok(androidModel.includes('if (disposed.get()) return'));
  assert.ok(androidModel.includes('fun noteImportCancelled()'));
  assert.ok(androidModel.includes('Nothing was saved.'));
  assert.ok(appleModel.includes('Nothing was saved.'));
  assert.ok(androidModel.includes('fun reopenProject()'));
  assert.ok(appleModel.includes('func recover() { invalidateAdvice(); send { $0.close(); try $0.reopen() } }'));
  assert.ok(androidScreen.includes('if (core?.approvalPending == true) stringResource(R.string.retry_apply)'));
  assert.equal(androidModel.includes('while (true) { model.action(1)'), false);
  assert.equal(appleModel.includes('while true { action(1)'), false);
  for (const p of files) {
    const s = text(p).split('<!-- phase10-step4-current -->')[1]?.split('<!-- phase10-step4-history -->')[0];
    assert.ok(s, p);
    for (const term of ['Step 3 COMPLETE / FULL PASS', 'b2e5809aba185f27c4362b5a56c1b448953ed743', '42114973998', '42114973771', '8e5a64c51acb87f25b76bafb5bc1106dadfca686', '42108036683', '42108036667', 'Step 4 CLOSURE CERTIFICATION PENDING', '359c2f7ffd7e70b0910019ab8c38acafa29848fb', '42134548234', '42134548288', 'Steps 5-6 NOT STARTED', 'Phase 11 is not started', 'Phase 12 is not started']) assert.ok(s.includes(term), p + term);
    assert.ok(text(p).split('<!-- phase10-step3-current -->')[1].includes('Step 3 CLOSURE CERTIFICATION PENDING'), p);
  }
  assert.ok(text('docs/plans/phase-10-roadmap.md').includes('| 4 | Error Recovery and Lifecycle | CLOSURE CERTIFICATION PENDING |'));
  assert.ok(text('docs/plans/phase-10-roadmap.md').includes('| 3 | Localization, Accessibility and Responsive UI | CLOSURE CERTIFICATION PENDING |'));
  for (const p of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml']) assert.equal(text(p).includes('tests/product-ux-recovery.test.mjs'), true, p);
});
