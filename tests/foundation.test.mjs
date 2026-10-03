import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const required = [
  'AGENTS.md', 'PRODUCT_SPEC.md', 'ARCHITECTURE.md', 'AI_ARCHITECTURE.md',
  'PHOTO_ENGINE.md', 'DEVICE_CAPABILITY_STRATEGY.md', 'PRIVACY_SECURITY.md',
  'UX_PRINCIPLES.md', 'MONETIZATION.md', 'TESTING_STRATEGY.md',
  'QUALITY_GATES.md', 'ROADMAP.md', 'DECISIONS.md', 'README.md',
];
function files(directory = root) {
  return fs.readdirSync(directory, { withFileTypes: true }).flatMap(entry => {
    if (['.git', 'build', '.build', '.swiftpm', '.gradle', '.kotlin', '.cxx'].includes(entry.name)) return [];
    const full = path.join(directory, entry.name);
    return entry.isDirectory() ? files(full) : [full];
  });
}

test('authoritative documents and ADRs are present and substantive', () => {
  for (const file of required) {
    const content = fs.readFileSync(path.join(root, file), 'utf8');
    assert.ok(content.startsWith('# '), `${file}: title missing`);
    assert.ok(content.length > 500, `${file}: substantive specification missing`);
  }
  const adrs = files(path.join(root, 'docs/adr'));
  assert.ok(adrs.length >= 6);
  for (const file of adrs) {
    const content = fs.readFileSync(file, 'utf8');
    for (const section of ['Status: Accepted', '## Context', '## Decision', '## Alternatives', '## Consequences', '## Validation']) {
      assert.ok(content.includes(section), `${file}: missing ${section}`);
    }
  }
});

test('relative documentation links resolve inside the repository', () => {
  for (const file of files().filter(file => file.endsWith('.md'))) {
    const content = fs.readFileSync(file, 'utf8');
    for (const match of content.matchAll(/\[[^\]]*\]\(([^)]+)\)/g)) {
      const target = match[1];
      if (/^(https?:|#)/.test(target)) continue;
      const resolved = path.resolve(path.dirname(file), target.split('#')[0]);
      assert.ok(!path.relative(root, resolved).startsWith('..'), `${file}: escaping link`);
      assert.ok(fs.existsSync(resolved), `${file}: broken link ${target}`);
    }
  }
});

test('source hygiene: no conflict markers, trailing spaces or missing newlines', () => {
  for (const file of files()) {
    if (file.endsWith('.jar')) {
      assert.equal(path.relative(root, file).split(path.sep).join('/'),
        'platforms/android/gradle/wrapper/gradle-wrapper.jar', 'Unexpected binary outside generated directories');
      continue;
    }
    const content = fs.readFileSync(file, 'utf8');
    assert.ok(!/^(<<<<<<<|=======|>>>>>>>)( |$)/m.test(content), `${file}: conflict marker`);
    assert.ok(!/[\t ]+\r?$/m.test(content), `${file}: trailing whitespace`);
    assert.ok(content.endsWith('\n'), `${file}: missing final newline`);
  }
});

test('all build systems compile the same core and boundaries have smoke probes', () => {
  const read = file => fs.readFileSync(path.join(root, file), 'utf8');
  assert.match(read('CMakeLists.txt'), /packages\/core\/src\/core\.cpp/);
  assert.match(read('packages/core/Package.swift'), /src\/core\.cpp/);
  assert.match(read('platforms/android/native/CMakeLists.txt'), /add_subdirectory\(\.\.\/\.\.\/\.\.\/ core\)/);
  assert.match(read('platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/CoreProbe.kt'), /package ai\.pixaura\.bridge/);
  assert.match(read('platforms/android/native/bridge.cpp'), /Java_ai_pixaura_bridge_CoreProbe_nativeAbiVersion/);
  assert.match(read('packages/core/swift/Tests/CoreProbeTests.swift'), /XCTAssertEqual/);
});
