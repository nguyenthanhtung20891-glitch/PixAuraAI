import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import crypto from 'node:crypto';
const text = p => fs.readFileSync(p, 'utf8').replaceAll('\r\n', '\n');
const read = p => JSON.parse(text(p));
const hash = p => crypto.createHash('sha256').update(text(p)).digest('hex');
const files = ['AGENTS.md', 'ARCHITECTURE.md', 'DECISIONS.md', 'QUALITY_GATES.md', 'ROADMAP.md', 'docs/plans/phase-8-roadmap.md', 'docs/plans/phase-9-roadmap.md', 'docs/plans/phase-10-roadmap.md'];

test('Phase 10 Step 2 explains an empty editor without changing import authority', () => {
  const m = read('docs/reports/phase-10-step-2-v1.json');
  assert.equal(m.status, 'CLOSURE_CERTIFICATION_PENDING');
  assert.equal(m.implementation_sha, 'f9f2df461ecfb63b371982d9cc2ec7f15db14628');
  assert.deepEqual(m.implementation_runs, [42085862758, 42085862581]);
  assert.equal(m.step1_closure_sha, '40f5760b845b9f0ff2bba72228538152eb8a5b0d');
  assert.equal(m.step1_implementation_sha, '5b3d3a15485f159b056861aee49ca56169ce8030');
  assert.equal(m.uncertified_closure_sha, 'af6159de89ec108839e3ecc6e17c13039b0a53ca');
  assert.equal(m.runtime_authority, false);
  assert.equal(m.editing_authority, false);
  assert.equal(m.history_authority, false);
  assert.equal(m.step3, 'NOT_STARTED');
  const step3 = read('docs/reports/phase-10-step-3-v1.json');
  for (const [p, change] of Object.entries(m.changes)) {
    const successor = step3.changes[p];
    if (successor) assert.equal(successor.before_sha256_lf, change.after_sha256_lf, p);
    else assert.equal(hash(p), change.after_sha256_lf, p);
  }
  const receipt = read('docs/reports/evidence/phase-10-step-1-closure-certification.json');
  assert.equal(receipt.sha, m.step1_closure_sha);
  assert.equal(receipt.label, 'closure');
  assert.equal(receipt.step1, 'COMPLETE_FULL_PASS');
  assert.equal(receipt.step2, 'AUTHORIZED_IN_PROGRESS');
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
  const implementation = read('docs/reports/evidence/phase-10-step-2-implementation-certification.json');
  assert.equal(implementation.sha, m.implementation_sha);
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
  const android = text('platforms/android/app/src/main/res/values/strings.xml');
  const apple = text('platforms/ios/PixAuraAI/en.lproj/Localizable.strings');
  for (const term of ['Open Photo', 'PixAuraAI edits a private copy on this device.', 'Open JPEG or PNG', 'Open an image to edit locally']) {
    assert.ok(android.includes(term), term);
    assert.ok(apple.includes(term), term);
  }
  assert.ok(text('platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorScreen.kt').includes('R.string.open_photo'));
  assert.ok(text('platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorScreen.kt').includes('R.string.empty_purpose'));
  assert.ok(text('platforms/ios/PixAuraAI/ManualEditorScreen.swift').includes('String(localized: "Open Photo")'));
  assert.ok(text('platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorModel.kt').includes('Import cancelled. Open JPEG or PNG to try again.'));
  assert.ok(text('platforms/ios/PixAuraAI/ManualEditorModel.swift').includes('Import cancelled. Open JPEG or PNG to try again.'));
  assert.ok(text('platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorModel.kt').includes('owner?.close()'));
  const cancelled = text('platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorModel.kt').split('fun noteImportCancelled()')[1].split('fun importImage')[0];
  assert.equal(cancelled.includes('owner'), false);
  for (const p of files) {
    const s = text(p).split('<!-- phase10-step2-current -->')[1]?.split('<!-- phase10-step2-history -->')[0];
    assert.ok(s, p);
    for (const term of ['Step 1 COMPLETE / FULL PASS', '40f5760b845b9f0ff2bba72228538152eb8a5b0d', '42053284183', '42053284144', '5b3d3a15485f159b056861aee49ca56169ce8030', '42040586837', '42040586934', 'af6159de89ec108839e3ecc6e17c13039b0a53ca', 'Step 2 CLOSURE CERTIFICATION PENDING', 'f9f2df461ecfb63b371982d9cc2ec7f15db14628', '42085862758', '42085862581', 'Steps 3-6 NOT STARTED', 'Schema 3 unchanged', 'HUMAN Apply unchanged', 'reference.assistance.v1 ADMITTED', 'reference.deterministic.v1', 'NONE REQUIRED / NONE ADMITTED', 'Vulkan/Metal UNKNOWN', 'NOT WAIVED', 'NOT SATISFIED', 'Phase 11 is not started', 'Phase 12 is not started']) assert.ok(s.includes(term), p + term);
    assert.ok(text(p).split('<!-- phase10-step1-current -->')[1].includes('Steps 2-6 NOT STARTED'), p);
  }
  assert.ok(text('docs/plans/phase-10-roadmap.md').includes('| 2 | Onboarding, Empty States and Permissions | CLOSURE CERTIFICATION PENDING |'));
  for (const p of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml']) assert.ok(text(p).includes('tests/product-ux-empty.test.mjs'), p);
});
