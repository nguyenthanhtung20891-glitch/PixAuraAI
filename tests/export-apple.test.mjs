import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
const text = p => fs.readFileSync(p, 'utf8').replaceAll('\r\n', '\n');
const read = p => JSON.parse(text(p));
const files = ['AGENTS.md', 'ARCHITECTURE.md', 'DECISIONS.md', 'QUALITY_GATES.md', 'ROADMAP.md', 'docs/plans/phase-8-roadmap.md', 'docs/plans/phase-9-roadmap.md'];

test('Phase 9 Step 4 saves and shares on Apple without changing history', () => {
  const m = read('docs/reports/phase-9-step-4-v1.json');
  assert.equal(m.status, 'CLOSURE_CERTIFICATION_PENDING');
  assert.equal(m.certified_implementation_sha, '6a6b8d30ba69316822dcffafbee6894e4d1f0271');
  assert.deepEqual(m.implementation_runs, [41967410493, 41967410497]);
  assert.equal(m.step3_closure_sha, 'df49c9a77bddf2f0a8b45d5bb0f12120ebe4bc3b');
  assert.equal(m.step3_implementation_sha, '093f63b7807935f92a3758552dfd60fc79f38d97');
  assert.equal(m.runtime_authority, false);
  assert.equal(m.editing_authority, false);
  assert.equal(m.history_authority, false);
  assert.equal(m.step5, 'NOT_STARTED');
  assert.equal(m.share_presentation_is_delivery, false);
  const receipt = read('docs/reports/evidence/phase-9-step-3-closure-certification.json');
  assert.equal(receipt.sha, m.step3_closure_sha);
  assert.equal(receipt.label, 'closure');
  assert.equal(receipt.step3, 'COMPLETE_FULL_PASS');
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
  const bridge = text('packages/core/swift/Sources/ManualEditor.swift');
  assert.ok(bridge.includes('func exportEncoded'));
  assert.ok(bridge.includes('revisionCount'));
  const destination = text('platforms/ios/PixAuraAI/ManualEditorModel.swift');
  for (const term of ['PHPhotoLibrary', 'addOnly', 'NSPhotoLibraryAddUsageDescription', 'writeAtomically', 'share_presentation_is_delivery']) {
    if (term === 'NSPhotoLibraryAddUsageDescription') assert.ok(text('platforms/ios/PixAuraAI.xcodeproj/project.pbxproj').includes(term), term);
    else if (term !== 'share_presentation_is_delivery') assert.ok(destination.includes(term), term);
  }
  assert.equal(destination.includes('URL('), false);
  const screen = text('platforms/ios/PixAuraAI/ManualEditorScreen.swift');
  for (const term of ['Save to Photos', 'Save to a file', 'Share', 'Cancel export', 'export-status', 'UIActivityViewController']) assert.ok(screen.includes(term), term);
  for (const p of files) {
    const s = text(p).split('<!-- phase9-step4-current -->')[1]?.split('<!-- phase9-step4-history -->')[0];
    assert.ok(s, p);
    for (const term of ['Step 3 COMPLETE / FULL PASS', 'df49c9a77bddf2f0a8b45d5bb0f12120ebe4bc3b', '41950759628', '41950759555', '093f63b7807935f92a3758552dfd60fc79f38d97', '41945751521', '41945751568', 'Step 4 CLOSURE CERTIFICATION PENDING', '6a6b8d30ba69316822dcffafbee6894e4d1f0271', '41967410493', '41967410497', 'EXPORT IS NOT A REVISION', 'share sheet', 'Schema 3 unchanged', 'HUMAN Apply unchanged', 'reference.assistance.v1 ADMITTED', 'reference.deterministic.v1', 'NONE REQUIRED / NONE ADMITTED', 'Vulkan/Metal UNKNOWN', 'NOT WAIVED', 'NOT SATISFIED', 'Steps 5-6 NOT STARTED']) assert.ok(s.includes(term), p + term);
  }
  assert.ok(text('docs/plans/phase-9-roadmap.md').includes('| 4 | Apple Save and Share | CLOSURE CERTIFICATION PENDING |'));
  const implementation = read('docs/reports/evidence/phase-9-step-4-implementation-certification.json');
  assert.equal(implementation.sha, m.certified_implementation_sha);
  assert.equal(implementation.label, 'implementation');
  assert.equal(implementation.step4, 'CLOSURE_CERTIFICATION_PENDING');
  assert.equal(implementation.step5, 'NOT_STARTED');
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
  assert.ok(text('docs/plans/phase-9-roadmap.md').includes('| 5 | File Lifecycle Hardening | AUTHORIZED / IN PROGRESS |'));
  for (const p of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml']) assert.ok(text(p).includes('tests/export-apple.test.mjs'), p);
});
