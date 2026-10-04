import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import crypto from 'node:crypto';
const read = file => fs.readFileSync(new URL(`../${file}`, import.meta.url), 'utf8');
test('SQLite vendor bytes match the reviewed official pin', () => {
  const base = 'packages/core/vendor/sqlite/';
  const pin = JSON.parse(read(`${base}provenance.json`));
  assert.equal(pin.version, '3.53.4');
  assert.equal(pin.license, 'blessing');
  for (const [file, hash] of Object.entries(pin.files)) {
    const bytes = fs.readFileSync(new URL(`../${base}${file}`, import.meta.url));
    assert.equal(crypto.createHash('sha256').update(bytes).digest('hex'), hash, file);
  }
  assert.match(read(`${base}sqlite3.h`), /SQLITE_VERSION_NUMBER\s+3053004/);
});
test('persistence and independent C consumers execute in all native host gates', () => {
  for (const file of ['CMakeLists.txt', 'scripts/check-native.ps1', 'scripts/check-native-zig.ps1']) {
    for (const name of ['storage_test', 'storage_c_consumer']) assert.ok(read(file).includes(name), `${file}: ${name}`);
  }
  for (const file of ['CMakeLists.txt', 'packages/core/Package.swift', 'scripts/check-native.ps1', 'scripts/check-native-zig.ps1']) {
    for (const source of ['storage.cpp', 'storage_files.cpp', 'sha256.cpp', 'storage_api.cpp', 'sqlite3.c']) assert.ok(read(file).includes(source), `${file}: ${source}`);
    for (const option of ['SQLITE_OMIT_LOAD_EXTENSION', 'SQLITE_OMIT_SHARED_CACHE', 'SQLITE_MAX_LENGTH', 'SQLITE_DQS']) assert.ok(read(file).includes(option), `${file}: ${option}`);
  }
  assert.match(read('CMakeLists.txt'), /storage_test PROPERTIES TIMEOUT 180/);
  assert.match(read('CMakeLists.txt'), /storage_c_consumer PROPERTIES LINKER_LANGUAGE CXX/);
});
test('platform storage checks use the native checked boundary', () => {
  assert.match(read('platforms/android/native/bridge.cpp'), /pixaura_storage_check/);
  assert.match(read('packages/core/swift/Tests/StorageBoundaryTests.swift'), /pixaura_storage_check/);
  assert.match(read('platforms/android/app/src/androidTest/java/ai/pixaura/app/StorageBoundaryTest.kt'), /nativeStorageVersion/);
});
test('Apple SDK macro exception stays confined to the vendored SQLite target', () => {
  const manifest = read('packages/core/Package.swift');
  const sqlite = manifest.slice(manifest.indexOf('.target(name: "CPixAuraSQLite"'), manifest.indexOf('.target(name: "CPixAuraCore"'));
  const owned = manifest.slice(manifest.indexOf('.target(name: "CPixAuraCore"'));
  assert.match(sqlite, /\.unsafeFlags\(\["-Wno-ambiguous-macro"\], \.when\(platforms: \[\.macOS, \.iOS\]\)\)/);
  assert.doesNotMatch(owned, /-Wno-|-w\b/);
  assert.match(sqlite, /visibility/);
  for (const file of ['scripts/check-apple.sh', 'scripts/check-ios-shell.sh']) {
    const commands = read(file).match(/xcodebuild[^\n]*(?:\\\n[^\n]*)*(?:build|test)/g);
    assert.equal(commands.length, 2, file);
    for (const command of commands) {
      assert.ok(command.includes("'KEEP_PRIVATE_EXTERNS=$(PIXAURA_PRIVATE_EXTERNS_$(TARGET_NAME))'"), file);
      assert.match(command, /PIXAURA_PRIVATE_EXTERNS_CPixAuraSQLite=YES/, file);
      assert.doesNotMatch(command, /\bKEEP_PRIVATE_EXTERNS=YES/, file);
    }
  }
  assert.match(read('scripts/check-apple.sh'), /-Xswiftc -warnings-as-errors -Xcc -Werror/);
  assert.match(read('platforms/ios/PixAuraAI.xcodeproj/project.pbxproj'), /relativePath = \.\.\/\.\.\/packages\/core/);
});
