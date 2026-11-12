import assert from 'node:assert/strict';
import fs from 'node:fs';
import test from 'node:test';

const read = (name) => fs.readFileSync(name, 'utf8');

test('iOS UI tests launch the English catalog without changing device language', () => {
  const tests = read('platforms/ios/PixAuraAIUITests/ShellUITests.swift');
  const screen = read('platforms/ios/PixAuraAI/ManualEditorScreen.swift');
  const launches = tests.match(/launchArguments = \[[^\]]*\]/g);
  assert.equal(launches[0], 'launchArguments = ["-AppleLanguages", "(en)", "-AppleLocale", "en_US"]');
  assert.equal(launches.length, 2);
  assert.match(launches[1], /AppleLanguages.*language.*AppleLocale.*locale/);
  assert.match(tests, /\("en", "en_US", "Share"\).*\("vi", "vi_VN", "Chia sẻ"\)/);
  assert.equal(tests.match(/let app = launchEnglish\(/g).length, 18);
  assert.equal(tests.match(/startEnglish\(app,/g).length, 3);
  assert.doesNotMatch(tests.slice(tests.indexOf('func selectControl')), /coordinate\(withNormalizedOffset/);
  assert.match(tests, /identifier == %@ OR label == %@/);
  assert.match(tests, /XCTAssertEqual\(app\.buttons\["open-photo"\]\.label, "Open Photo"\)/);
  assert.match(tests, /Committed revisions:/);
  for (const key of ['Apply', 'Cancel', 'Undo', 'Redo']) {
    assert.match(screen, new RegExp(`String\\(localized: "${key}"\\)`));
    assert.match(screen, new RegExp(`accessibilityIdentifier\\("${key}"\\)`));
  }
  const vi = read('platforms/ios/PixAuraAI/vi.lproj/Localizable.strings');
  assert.match(vi, /"Open Photo" = "Mở ảnh";/);
  assert.match(vi, /"Apply" = "Áp dụng";/);
  assert.match(vi, /"Undo" = "Hoàn tác";/);
  assert.match(vi, /"Redo" = "Làm lại";/);
  assert.match(vi, /"Committed revisions: %lld" = "Số bản sửa đã lưu: %lld";/);
  const project = read('platforms/ios/PixAuraAI.xcodeproj/project.pbxproj');
  assert.equal(project.split('SWIFT_TREAT_WARNINGS_AS_ERRORS = YES').length - 1, 2);
  assert.match(read('scripts/check-ios-shell.sh'), /CODE_SIGNING_ALLOWED=NO test/);
  const report = JSON.parse(read('docs/reports/phase-10.1-uitest-locale-v1.json'));
  assert.equal(report.mac_studio_xcode_27_2_observed, false);
  assert.equal(report.apple_metal_v1, 'UNKNOWN');
  for (const workflow of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml']) {
    assert.match(read(workflow), /tests\/uitest-locale\.test\.mjs/);
  }
});
