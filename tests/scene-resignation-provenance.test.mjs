import assert from 'node:assert/strict';
import crypto from 'node:crypto';
import fs from 'node:fs';
import test from 'node:test';
import { resultReviewHash } from './helpers/review-edit-hashes.mjs';

const read = (name) => fs.readFileSync(name, 'utf8');
const hash = (name) => crypto.createHash('sha256').update(read(name).replaceAll('\r\n', '\n')).digest('hex');

test('scene resignation extends the source chain without rewriting earlier fingerprints', () => {
  const capture = JSON.parse(read('docs/reports/phase-10.1-swift-capture-v1.json'));
  const locale = JSON.parse(read('docs/reports/phase-10.1-uitest-locale-v1.json'));
  const resigned = JSON.parse(read('docs/reports/phase-10.1-scene-resignation-v1.json'));
  const step4 = JSON.parse(read('docs/reports/phase-8-step-4-v1.json'));
  assert.equal(capture.mac_studio_xcode_27_2_observed, false);
  assert.equal(locale.mac_studio_xcode_27_2_observed, false);
  assert.equal(capture.apple_metal_v1, 'UNKNOWN');
  assert.equal(locale.apple_metal_v1, 'UNKNOWN');
  assert.equal(resigned.subject, 'ios-scene-resignation');
  assert.equal(resigned.predecessor_commit, '760f4afa9b17c80d08f4636857f43780ff33281a');
  assert.equal(resigned.mac_studio_xcode_27_2_observed, true);
  assert.equal(resigned.apple_metal_v1, 'UNKNOWN');
  assert.deepEqual(resigned.historical_records_unchanged, [
    'docs/reports/phase-10.1-swift-capture-v1.json',
    'docs/reports/phase-10.1-uitest-locale-v1.json',
  ]);

  const model = 'platforms/ios/PixAuraAI/ManualEditorModel.swift';
  const screen = 'platforms/ios/PixAuraAI/ManualEditorScreen.swift';
  const ui = 'platforms/ios/PixAuraAIUITests/ShellUITests.swift';
  assert.equal(capture.changes[model].after_sha256_lf, '538b80197a15ed7eb3bd6418a1bb5b657708f5a4457d2d047ad5745f042bc3d8');
  assert.equal(locale.changes[screen].after_sha256_lf, 'b96f998129ea7fee241e82a669124f1240a1e8ebe42af9d987018a42a68b5e9d');
  assert.equal(locale.changes[ui].after_sha256_lf, '90d27f153cc1764c97d4bac2854e126b48609995eb44dabeee9fe6af40f9a3f1');
  assert.equal(resigned.changes[model].before_sha256_lf, capture.changes[model].after_sha256_lf);
  assert.equal(resigned.changes[screen].before_sha256_lf, locale.changes[screen].after_sha256_lf);
  assert.equal(resigned.changes[ui].before_sha256_lf, locale.changes[ui].after_sha256_lf);
  for (const path of [model, screen, ui]) assert.equal(hash(path), resigned.changes[path].after_sha256_lf, path);
  assert.equal(hash(model), resultReviewHash(model, step4.changes[model].after_sha256_lf), model);
  assert.equal(hash(screen), resultReviewHash(screen, step4.changes[screen].after_sha256_lf), screen);
  assert.equal(hash(ui), resultReviewHash(ui, locale.changes[ui].before_sha256_lf), ui);

  const tests = read(ui);
  const background = tests.split('private func backgroundAndReturn')[1].split('private func revealReview')[0];
  assert.match(background, /XCUIApplication\(bundleIdentifier: "com\.apple\.springboard"\)\.activate\(\)/);
  assert.match(background, /app\.state == \.runningBackground \|\| app\.state == \.runningBackgroundSuspended/);
  const integrated = tests.split('func testIntegratedModeBackgroundNavigationAndRestartNeverRestoreReviewAuthority()')[1].split('func testReviewHumanApplyOneOrdinaryRevisionAndManualUndoRedo()')[0];
  assert.ok(integrated.includes('backgroundAndReturn(app)'));
  assert.ok(integrated.includes('XCTAssertFalse(app.buttons["auto-apply"].exists)'));
  assert.ok(integrated.includes('XCTAssertTrue(app.staticTexts["Committed revisions: 1"].exists)'));
  const gesture = tests.split('func testManualEditorGestureHistoryCancelBackgroundAndReopen()')[1];
  assert.ok(gesture.includes('backgroundAndReturn(app)'));
  assert.ok(gesture.includes('XCTAssertFalse(app.buttons["Apply"].isEnabled)'));
  const unit = read('platforms/ios/PixAuraAITests/ShellTests.swift').split('func testInterruptDropsAutoAuthorityAndManualGestureWithoutRevision()')[1].split('model.close()')[0];
  assert.match(unit, /XCTAssertNil\(model\.autoProposal\.proposal\)/);
  assert.match(unit, /XCTAssertNil\(model\.autoProposal\.review\)/);
  assert.match(unit, /XCTAssertEqual\(model\.core\?\.revisionCount, 1\)/);
  assert.match(unit, /model\.action\(2\)/);
  assert.match(unit, /XCTAssertEqual\(model\.core\?\.revisionID, source\)/);
  for (const workflow of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml', 'scripts/check-apple.sh']) {
    assert.match(read(workflow), /tests\/scene-resignation-provenance\.test\.mjs/);
  }
});
