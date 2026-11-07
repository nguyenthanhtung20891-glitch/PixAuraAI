import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import test from 'node:test';

const read = (name) => fs.readFileSync(name, 'utf8');

function swiftFiles(dir, out = []) {
  for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
    const full = path.join(dir, entry.name);
    if (entry.isDirectory()) swiftFiles(full, out);
    else if (entry.name.endsWith('.swift')) out.push(full.replaceAll('\\', '/'));
  }
  return out;
}

test('worker closures name the existing strong capture and main callbacks stay weak', () => {
  const model = read('platforms/ios/PixAuraAI/ManualEditorModel.swift');
  assert.equal(model.split('send { [self] worker in').length - 1, 5);
  assert.equal(model.split('send { worker in').length - 1, 1);
  assert.equal(model.split('DispatchQueue.main.async { [weak self] in').length - 1, 6);
  assert.doesNotMatch(model, /send \{ worker in[\s\S]*?DispatchQueue\.main\.async \{ \[weak self\]/);
  const suggestion = model.split('func useSuggestion()')[1].split('private func nextAutoEpoch()')[0];
  assert.equal(suggestion.split('self.assistEpoch == stamp').length - 1, 2);
  assert.match(suggestion, /try worker\.useAdvice\(advice\)/);
  assert.match(suggestion, /Manual tool opened\. Set your preferred value, then choose Apply\./);
  const reconcile = model.split('private func reconcileFailedApply')[1].split('func showResult')[0];
  assert.match(reconcile, /Execution cancelled before publication\./);
  assert.match(reconcile, /Outcome unconfirmed\. Ordinary history remains authoritative\./);
  assert.match(reconcile, /self\.autoProposal\.cancelRequested/);
  assert.match(model, /func undoResult\(\) \{ resultHistory\(2\) \}/);
  assert.match(model, /func redoResult\(\) \{ resultHistory\(3\) \}/);
  assert.match(model, /worker\.closeAfterOwnerRelease\(\)/);
  assert.doesNotMatch(model, /ImplicitStrongCapture|swiftFlags|no-warnings/);
  for (const file of swiftFiles('platforms/ios')) {
    const count = read(file).split('DispatchQueue.main.async { [weak self] in').length - 1;
    assert.equal(count, file.endsWith('platforms/ios/PixAuraAI/ManualEditorModel.swift') ? 6 : 0, file);
  }
  const project = read('platforms/ios/PixAuraAI.xcodeproj/project.pbxproj');
  assert.equal(project.split('SWIFT_TREAT_WARNINGS_AS_ERRORS = YES').length - 1, 2);
  assert.doesNotMatch(project, /ImplicitStrongCapture|-Wno-/);
  const report = JSON.parse(read('docs/reports/phase-10.1-swift-capture-v1.json'));
  assert.equal(report.mac_studio_xcode_27_2_observed, false);
  assert.equal(report.apple_metal_v1, 'UNKNOWN');
  for (const workflow of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml']) {
    assert.match(read(workflow), /tests\/swift-capture-compat\.test\.mjs/);
  }
});
