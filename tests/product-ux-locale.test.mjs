import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
const text = p => fs.readFileSync(p, 'utf8').replaceAll('\r\n', '\n');
const names = xml => [...xml.matchAll(/<string name="([^"]+)">/g)].map(m => m[1]);
const value = (xml, name) => xml.match(new RegExp(`<string name="${name}">([^<]*)</string>`))[1];
const pairs = source => Object.fromEntries([...source.matchAll(/^"((?:\\.|[^"])*)" = "((?:\\.|[^"])*)";$/gm)].map(m => [m[1], m[2]]));

test('English and Vietnamese catalogs present the same journey without becoming authority', () => {
  const en = text('platforms/android/app/src/main/res/values/strings.xml');
  const vi = text('platforms/android/app/src/main/res/values-vi/strings.xml');
  assert.deepEqual(names(vi), names(en));
  assert.equal(value(en, 'committed_revisions'), 'Committed revisions: %1$d');
  assert.equal(value(vi, 'committed_revisions').includes('%1$d'), true);
  assert.equal(value(en, 'open_jpeg_or_png'), 'Open JPEG or PNG');
  assert.equal(value(en, 'apply'), 'Apply');
  assert.equal(value(en, 'undo'), 'Undo');
  assert.equal(value(en, 'redo'), 'Redo');
  assert.equal(value(en, 'cancel'), 'Cancel');
  for (const name of names(en)) {
    if (name === 'app_name') continue;
    assert.notEqual(value(vi, name), value(en, name), name);
  }
  assert.notEqual(value(vi, 'after_preview'), value(vi, 'committed_result'));
  assert.notEqual(value(vi, 'apply_reviewed_plan'), value(vi, 'undo_document'));
  assert.notEqual(value(vi, 'restore_review'), value(vi, 'undo_document'));
  assert.equal(value(vi, 'share_closed').includes('chưa được xác nhận'), true);
  const appleEn = pairs(text('platforms/ios/PixAuraAI/en.lproj/Localizable.strings'));
  const appleVi = pairs(text('platforms/ios/PixAuraAI/vi.lproj/Localizable.strings'));
  assert.deepEqual(Object.keys(appleVi), Object.keys(appleEn));
  assert.equal(appleEn['Committed revisions: %lld'], 'Committed revisions: %lld');
  assert.equal(appleVi['Committed revisions: %lld'].includes('%lld'), true);
  assert.equal(appleEn.Apply, 'Apply');
  assert.equal(appleEn['Open JPEG or PNG'], 'Open JPEG or PNG');
  for (const key of Object.keys(appleEn)) assert.notEqual(appleVi[key], appleEn[key], key);
  assert.equal(text('platforms/android/app/src/main/java/ai/pixaura/app/Presentation.kt').includes('else -> 0'), true);
  const applePresentation = text('platforms/ios/PixAuraAI/ManualEditorScreen.swift');
  assert.equal(applePresentation.includes('String(localized: String.LocalizationValue(meaning), bundle: bundle)'), true);
  assert.equal(applePresentation.includes('String(body[$0.lowerBound...])'), true); // Raw native suffix stays verbatim.
  assert.equal(applePresentation.includes('return heading +'), true);
  assert.equal(fs.existsSync('packages/core/src') && text('platforms/android/app/src/main/java/ai/pixaura/app/Presentation.kt').includes('Plan('), false);
  const screen = text('platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorScreen.kt');
  assert.equal(screen.includes('Text("JPEG")'), true);
  assert.equal(screen.includes('Text("PNG")'), true);
  assert.equal(screen.includes('maxLines'), false);
  assert.equal(text('platforms/android/app/src/main/java/ai/pixaura/app/ReviewEditPanel.kt').includes('"EDITED"'), true);
  for (const p of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml']) assert.equal(text(p).includes('tests/product-ux-locale.test.mjs'), true, p);
  const files = ['AGENTS.md', 'ARCHITECTURE.md', 'DECISIONS.md', 'QUALITY_GATES.md', 'ROADMAP.md', 'docs/plans/phase-8-roadmap.md', 'docs/plans/phase-9-roadmap.md', 'docs/plans/phase-10-roadmap.md'];
  for (const p of files) {
    const s = text(p).split('<!-- phase10-step3-current -->')[1]?.split('<!-- phase10-step3-history -->')[0];
    assert.ok(s, p);
    for (const term of ['Step 2 COMPLETE / FULL PASS', 'e077092c5f714831f73b704edf473a64a096dff0', '42092651581', '42092651461', 'f9f2df461ecfb63b371982d9cc2ec7f15db14628', '42085862758', '42085862581', 'Step 3 CLOSURE CERTIFICATION PENDING', '8e5a64c51acb87f25b76bafb5bc1106dadfca686', '42108036683', '42108036667', 'Steps 4-6 NOT STARTED', 'Phase 11 is not started', 'Phase 12 is not started']) assert.ok(s.includes(term), p + term);
    assert.ok(text(p).split('<!-- phase10-step1-current -->')[1].includes('Steps 2-6 NOT STARTED'), p);
  }
  assert.ok(text('docs/plans/phase-10-roadmap.md').includes('| 3 | Localization, Accessibility and Responsive UI | CLOSURE CERTIFICATION PENDING |'));
  const receipt = JSON.parse(text('docs/reports/evidence/phase-10-step-2-closure-certification.json'));
  assert.equal(receipt.sha, 'e077092c5f714831f73b704edf473a64a096dff0');
  assert.equal(receipt.label, 'closure');
  assert.equal(receipt.step2, 'COMPLETE_FULL_PASS');
  assert.equal(receipt.step3, 'AUTHORIZED_IN_PROGRESS');
  assert.equal(receipt.runs[0].jobs.length, 7);
  assert.equal(receipt.runs[1].jobs.length, 3);
  for (const run of receipt.runs) {
    assert.equal(run.head_sha, receipt.sha);
    assert.equal(run.attempt, 1);
    assert.equal(run.conclusion, 'success');
    for (const job of run.jobs) assert.equal(job.conclusion, 'success');
  }
  const implementation = JSON.parse(text('docs/reports/evidence/phase-10-step-3-implementation-certification.json'));
  assert.equal(implementation.sha, '8e5a64c51acb87f25b76bafb5bc1106dadfca686');
  assert.equal(implementation.label, 'implementation');
  assert.equal(implementation.step3, 'CLOSURE_CERTIFICATION_PENDING');
  assert.equal(implementation.step4, 'NOT_STARTED');
  assert.equal(implementation.runs[0].jobs.length, 7);
  assert.equal(implementation.runs[1].jobs.length, 3);
  for (const run of implementation.runs) {
    assert.equal(run.head_sha, implementation.sha);
    assert.equal(run.attempt, 1);
    assert.equal(run.conclusion, 'success');
    for (const job of run.jobs) assert.equal(job.conclusion, 'success');
  }
});
