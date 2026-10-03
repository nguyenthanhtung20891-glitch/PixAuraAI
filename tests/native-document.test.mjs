import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';

const read = file => fs.readFileSync(new URL(`../${file}`, import.meta.url), 'utf8');

test('Apple package resource is byte-identical to the approved cross-boundary fixture', () => {
  assert.equal(read('packages/core/swift/Tests/Fixtures/image-document-v1.json'), read('tests/fixtures/image-document-v1.json'));
  assert.match(read('packages/core/Package.swift'), /resources: \[\.copy\("Fixtures"\)\]/);
  assert.match(read('packages/core/swift/Tests/DocumentBoundaryTests.swift'), /Bundle.module/);
  assert.match(read('platforms/android/app/build.gradle.kts'), /sourceSets\["androidTest"\]\.assets\.srcDir\("\.\.\/\.\.\/\.\.\/tests\/fixtures"\)/);
});

test('all shipping builds include the native document implementation and existing ABI 1 stays unchanged', () => {
  for (const file of ['CMakeLists.txt', 'packages/core/Package.swift', 'scripts/check-native.ps1', 'scripts/check-native-zig.ps1']) {
    for (const source of ['document.cpp', 'document_api.cpp']) assert.ok(read(file).includes(source), `${file}: ${source}`);
  }
  assert.match(read('packages/core/src/core.cpp'), /\*output = \{PIXAURA_ABI_VERSION, sizeof\(pixaura_core_info\), 0u, 0u\}/);
  assert.match(read('packages/core/include/pixaura/core.h'), /#define PIXAURA_ABI_VERSION 1u/);
  assert.match(read('packages/core/include/pixaura/document.h'), /#define PIXAURA_DOCUMENT_API_VERSION 1u/);
});

test('Linux Windows Apple and sanitizers execute document ownership and parser tests', () => {
  const cmake = read('CMakeLists.txt');
  for (const target of ['document_test', 'document_c_consumer', 'document_allocation_test']) {
    assert.match(cmake, new RegExp(`add_test\\(NAME ${target} COMMAND ${target}`));
    assert.ok(read('scripts/check-native.ps1').includes(`${target}.exe`));
    assert.ok(read('scripts/check-native-zig.ps1').includes(`${target}.exe`));
  }
  assert.match(read('scripts/check-apple.sh'), /ctest --test-dir build\/apple-host --output-on-failure/);
  assert.match(read('scripts/check-apple.sh'), /swift test --package-path packages\/core/);
  assert.match(read('scripts/check-sanitizers.sh'), /ctest --test-dir build\/sanitize/);
  assert.match(read('.github/workflows/foundation.yml'), /tests\/native-document.test.mjs/);
  assert.match(read('.github/workflows/native-shells.yml'), /tests\/native-document.test.mjs/);
});

test('allocation failure injection is confined to an independent test executable', () => {
  const shipping = read('packages/core/src/document.cpp') + read('packages/core/src/document_api.cpp');
  assert.doesNotMatch(shipping, /operator new|fail_after|thread_local/);
  assert.match(read('packages/core/tests/document_allocation_test.cpp'), /operator new/);
  assert.match(read('CMakeLists.txt'), /add_executable\(document_allocation_test/);
});
