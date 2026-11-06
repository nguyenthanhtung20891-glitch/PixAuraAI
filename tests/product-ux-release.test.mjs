import assert from 'node:assert/strict';
import fs from 'node:fs';
import test from 'node:test';

const read = (name) => fs.readFileSync(name, 'utf8');
const android = read('platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorModel.kt');
const androidScreen = read('platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorScreen.kt');
const apple = read('platforms/ios/PixAuraAI/ManualEditorModel.swift');
const appleScreen = read('platforms/ios/PixAuraAI/ManualEditorScreen.swift');

test('Android and Apple share the release journey sentences', () => {
  const shared = [
    'Import cancelled. Open JPEG or PNG to try again.',
    'Share sheet closed. Delivery to another app is not confirmed.',
    'Nothing was saved.',
    'The selected revision is stale. Nothing was saved.',
    'Export stopped. Nothing was saved.',
  ];
  for (const sentence of shared) {
    assert.ok(android.includes(sentence) || androidScreen.includes(sentence), sentence);
    assert.ok(apple.includes(sentence) || appleScreen.includes(sentence), sentence);
  }
  assert.match(android, /Navigation only\. No update, preview, Apply or Agent execution\./);
  assert.match(apple, /Existing manual-tool navigation only; no values, preview or Apply\./);
  assert.match(androidScreen, /ON_STOP/);
  assert.match(androidScreen, /cancelExport\(\); model\.interrupt\(\)/);
  assert.match(appleScreen, /scenePhase/);
  assert.match(appleScreen, /cancelExport\(\); model\.interrupt\(\)/);
  assert.match(appleScreen, /\.disabled\(model\.core == nil\)/);
  assert.match(androidScreen, /if \(ui\.core != null\)/);
  assert.match(android, /fun reopenProject\(\)/);
  assert.match(apple, /func recover\(\)/);
  assert.doesNotMatch(android, /while\s*\(.*approvalPending/);
  assert.doesNotMatch(apple, /while\s+.*approvalPending/);
});

test('release packages keep certification hooks off and permissions narrow', () => {
  const gradle = read('platforms/android/app/build.gradle.kts');
  assert.match(gradle, /release[^\n]*PIXAURA_CERTIFICATION_TESTING=OFF/);
  const destination = read('platforms/android/app/src/main/java/ai/pixaura/app/ExportDestination.kt');
  for (const term of ['IS_PENDING', 'MediaStore', 'FileProvider', 'FLAG_GRANT_READ_URI_PERMISSION']) {
    assert.ok(destination.includes(term), term);
  }
  const project = read('platforms/ios/PixAuraAI.xcodeproj/project.pbxproj');
  assert.match(project, /NSPhotoLibraryAddUsageDescription/);
  assert.match(apple, /authorizationStatus\(for: \.addOnly\)/);
  assert.doesNotMatch(read('packages/core/src/storage_files.cpp'), /fprintf\([^)]*%s/);
});

test('Step 6 records the observed Step 5 closure without closing Phase 10', () => {
  const agents = read('AGENTS.md');
  const closed = agents.split('<!-- phase10-closed-history -->')[0];
  assert.match(closed, /Phase 10 CLOSED \/ COMPLETE \/ FULL PASS/);
  assert.match(closed, /e4835cfcc008fb1e6db276cdcbeca47527e39cd4/);
  assert.match(closed, /42181593096/);
  assert.match(closed, /42181593143/);
  assert.match(closed, /This receipt is not the closure SHA/);
  assert.match(closed, /Phase 10\.1 is not started/);
  assert.match(read('docs/plans/phase-10.1-readiness.md'), /Status: NOT STARTED/);
  assert.match(read('docs/plans/phase-10.1-readiness.md'), /does not change Metal trust/);
  const block = agents.split('<!-- phase10-step6-history -->')[0];
  assert.match(block, /Step 5 COMPLETE \/ FULL PASS/);
  assert.match(block, /47c6503c111feaa5f21c4f732036490a023107f0/);
  assert.match(block, /42168863992/);
  assert.match(block, /42168864184/);
  assert.match(block, /Step 6 CLOSURE CERTIFICATION PENDING/);
  assert.match(block, /c1dfbe2783043530bf9c6f2aaa407b11d2730fe0/);
  assert.match(block, /42173777297/);
  assert.match(block, /42173777251/);
  assert.match(block, /Phase 10 is not closed/);
  assert.match(agents.split('<!-- phase10-step5-history -->')[0], /Step 5 CLOSURE CERTIFICATION PENDING/);
  const roadmap = read('docs/plans/phase-10-roadmap.md');
  assert.match(roadmap, /\| 6 \| Cross-Platform Software Release Readiness and Phase Closure \| CLOSURE CERTIFICATION PENDING \|/);
  assert.match(roadmap, /\| 5 \| Performance, Resource and Privacy Hardening \| CLOSURE CERTIFICATION PENDING \|/);
  const report = JSON.parse(read('docs/reports/phase-10-step-6-v1.json'));
  assert.equal(report.status, 'CLOSURE_CERTIFICATION_PENDING');
  assert.equal(report.implementation_sha, 'c1dfbe2783043530bf9c6f2aaa407b11d2730fe0');
  const implementation = JSON.parse(read('docs/reports/evidence/phase-10-step-6-implementation-certification.json'));
  assert.equal(implementation.label, 'implementation');
  assert.equal(implementation.runs[0].jobs.length, 7);
  assert.equal(implementation.runs[1].jobs.length, 3);
  const receipt = JSON.parse(read('docs/reports/evidence/phase-10-step-5-closure-certification.json'));
  assert.equal(receipt.label, 'closure');
  assert.equal(receipt.step5, 'COMPLETE_FULL_PASS');
  assert.equal(receipt.runs[0].jobs.length, 7);
  assert.equal(receipt.runs[1].jobs.length, 3);
  for (const workflow of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml']) {
    assert.match(read(workflow), /tests\/product-ux-release\.test\.mjs/);
  }
});
