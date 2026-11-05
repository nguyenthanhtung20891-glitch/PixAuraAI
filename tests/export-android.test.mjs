import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
const text = p => fs.readFileSync(p, 'utf8').replaceAll('\r\n', '\n');
const read = p => JSON.parse(text(p));
const files = ['AGENTS.md', 'ARCHITECTURE.md', 'DECISIONS.md', 'QUALITY_GATES.md', 'ROADMAP.md', 'docs/plans/phase-8-roadmap.md', 'docs/plans/phase-9-roadmap.md'];

test('Phase 9 Step 3 saves and shares without changing history', () => {
  const m = read('docs/reports/phase-9-step-3-v1.json');
  assert.equal(m.status, 'CLOSURE_CERTIFICATION_PENDING');
  assert.equal(m.certified_implementation_sha, '093f63b7807935f92a3758552dfd60fc79f38d97');
  assert.deepEqual(m.implementation_runs, [41945751521, 41945751568]);
  assert.equal(m.step2_closure_sha, '667bcc3f04c639e41c18f15b4d44f6aee7852107');
  assert.equal(m.step2_implementation_sha, '64d854d7ec2a5e11787cd0b94ff60b0fdfe7a8e3');
  assert.equal(m.runtime_authority, false);
  assert.equal(m.editing_authority, false);
  assert.equal(m.history_authority, false);
  assert.equal(m.step4, 'NOT_STARTED');
  assert.equal(m.share_presentation_is_delivery, false);
  const receipt = read('docs/reports/evidence/phase-9-step-2-closure-certification.json');
  assert.equal(receipt.sha, m.step2_closure_sha);
  assert.equal(receipt.label, 'closure');
  assert.equal(receipt.step2, 'COMPLETE_FULL_PASS');
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
  const bridge = text('platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/ManualEditor.kt');
  assert.ok(bridge.includes('fun exportEncoded'));
  assert.ok(bridge.includes('revisionCount'));
  const destination = text('platforms/android/app/src/main/java/ai/pixaura/app/ExportDestination.kt');
  for (const term of ['IS_PENDING', 'MediaStore', 'FileProvider', 'FLAG_GRANT_READ_URI_PERMISSION', 'ACTION_SEND']) assert.ok(destination.includes(term), term);
  const screen = text('platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorScreen.kt');
  for (const term of ['Save to Photos', 'Save to a file', 'Share', 'Cancel export', 'export-status']) assert.ok(screen.includes(term), term);
  assert.ok(text('platforms/android/app/src/main/AndroidManifest.xml').includes('android:exported="false"'));
  assert.equal(text('platforms/android/app/src/main/java/ai/pixaura/app/ExportDestination.kt').includes('URL('), false);
  for (const p of files) {
    const s = text(p).split('<!-- phase9-step3-current -->')[1]?.split('<!-- phase9-step3-history -->')[0];
    assert.ok(s, p);
    for (const term of ['Step 2 COMPLETE / FULL PASS', '667bcc3f04c639e41c18f15b4d44f6aee7852107', '41935253282', '41935253224', '64d854d7ec2a5e11787cd0b94ff60b0fdfe7a8e3', '41928862538', '41928862545', 'Step 3 CLOSURE CERTIFICATION PENDING', '093f63b7807935f92a3758552dfd60fc79f38d97', '41945751521', '41945751568', 'EXPORT IS NOT A REVISION', 'sharesheet', 'Schema 3 unchanged', 'HUMAN Apply unchanged', 'reference.assistance.v1 ADMITTED', 'reference.deterministic.v1', 'NONE REQUIRED / NONE ADMITTED', 'Vulkan/Metal UNKNOWN', 'NOT WAIVED', 'NOT SATISFIED', 'Steps 4-6 NOT STARTED']) assert.ok(s.includes(term), p + term);
  }
  assert.ok(text('docs/plans/phase-9-roadmap.md').includes('| 3 | Android Save and Share | CLOSURE CERTIFICATION PENDING |'));
  assert.ok(text('docs/plans/phase-9-roadmap.md').includes('| 4 | Apple Save and Share | AUTHORIZED / IN PROGRESS |'));
  const implementation = read('docs/reports/evidence/phase-9-step-3-implementation-certification.json');
  assert.equal(implementation.sha, m.certified_implementation_sha);
  assert.equal(implementation.label, 'implementation');
  assert.equal(implementation.step3, 'CLOSURE_CERTIFICATION_PENDING');
  assert.equal(implementation.step4, 'NOT_STARTED');
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
  for (const p of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml']) assert.ok(text(p).includes('tests/export-android.test.mjs'), p);
});
