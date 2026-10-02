import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import crypto from 'node:crypto';
import { execFileSync } from 'node:child_process';
import { fileURLToPath } from 'node:url';
const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const read = file => fs.readFileSync(path.join(root, file), 'utf8');

test('native shells prohibit permissions, cloud backup and telemetry dependencies', () => {
  const manifest = read('platforms/android/app/src/main/AndroidManifest.xml');
  assert.doesNotMatch(manifest, /uses-permission/);
  assert.match(manifest, /allowBackup="false"/);
  const extraction = read('platforms/android/app/src/main/res/xml/data_extraction_rules.xml');
  for (const policy of ['cloud-backup', 'device-transfer']) {
    const block = extraction.match(new RegExp(`<${policy}>([\\s\\S]*?)</${policy}>`))[1];
    for (const domain of ['root', 'file', 'database', 'sharedpref', 'external']) {
      assert.ok(block.includes(`exclude domain="${domain}" path="."`));
    }
  }
  assert.doesNotMatch(read('platforms/android/app/build.gradle.kts'), /firebase|analytics|retrofit|okhttp/i);
  const ios = read('platforms/ios/PixAuraAI.xcodeproj/project.pbxproj');
  assert.doesNotMatch(ios, /UsageDescription|XCSwiftPackageReference|XCRemoteSwiftPackageReference/);
  assert.match(ios, /XCLocalSwiftPackageReference; relativePath = ..\/..\/packages\/core/);
  assert.match(read('platforms/ios/PixAuraAI/PrivacyInfo.xcprivacy'), /NSPrivacyTracking<\/key><false\/>/);
});

test('both shells have equivalent destinations/modes and invoke the shared bridge', () => {
  for (const destination of ['Home', 'Editor', 'Projects', 'Settings']) {
    assert.ok(read('platforms/android/app/src/main/java/ai/pixaura/app/navigation/ShellState.kt').includes(destination));
    assert.ok(read('platforms/ios/PixAuraAI/ShellState.swift').includes(destination));
  }
  for (const mode of ['Manual', 'Assisted', 'AI']) {
    assert.ok(read('platforms/android/app/src/main/java/ai/pixaura/app/navigation/ShellState.kt').includes(mode));
    assert.ok(read('platforms/ios/PixAuraAI/ShellState.swift').includes(mode));
  }
  assert.match(read('platforms/android/app/src/main/java/ai/pixaura/app/ShellViewModel.kt'), /withContext\(Dispatchers.Default\)/);
  assert.match(read('platforms/ios/PixAuraAI/ShellModel.swift'), /Task.detached/);
});

test('committed Xcode project is deterministic and all Swift files are assigned to targets', () => {
  const files = ['platforms/ios/PixAuraAI.xcodeproj/project.pbxproj', 'platforms/ios/PixAuraAI.xcodeproj/xcshareddata/xcschemes/PixAuraAI.xcscheme'];
  const before = files.map(read);
  execFileSync(process.execPath, ['scripts/generate-ios-project.mjs'], { cwd: root });
  assert.deepEqual(files.map(read), before);
  for (const directory of ['PixAuraAI', 'PixAuraAITests', 'PixAuraAIUITests']) {
    for (const file of fs.readdirSync(path.join(root, 'platforms/ios', directory)).filter(file => file.endsWith('.swift'))) {
      assert.ok(before[0].includes(`path = ${file};`), `${directory}/${file} not in project`);
    }
  }
  assert.match(before[1], /TestableReference skipped="NO"/);
});

test('Gradle wrapper distribution is checksum pinned and runtime dependencies are locked', () => {
  assert.match(read('platforms/android/gradle/wrapper/gradle-wrapper.properties'), /^distributionSha256Sum=20f1b1176237254a6fc204d8434196fa11a4cfb387567519c61556e8710aed78$/m);
  assert.ok(fs.existsSync(path.join(root, 'platforms/android/gradle/wrapper/gradle-wrapper.jar')));
  // Actual artifact checksums are enforced by Gradle; no floating production versions.
  assert.doesNotMatch(read('platforms/android/app/build.gradle.kts'), /:[+]|SNAPSHOT|latest.release/);
  assert.ok(read('platforms/android/app/gradle.lockfile').includes('androidx.compose'));
  assert.match(read('platforms/android/gradle/verification-metadata.xml'), /<sha256 value="[a-f0-9]{64}"/);
  assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(root, 'platforms/android/gradle/wrapper/gradle-wrapper.jar'))).digest('hex'),
    '81a82aaea5abcc8ff68b3dfcb58b3c3c429378efd98e7433460610fecd7ae45f');
});
