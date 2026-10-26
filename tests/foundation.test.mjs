import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import crypto from 'node:crypto';
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
    if (['.git', '.codex', 'build', '.build', '.swiftpm', '.gradle', '.kotlin', '.cxx'].includes(entry.name)) return [];
    const full = path.join(directory, entry.name);
    return entry.isDirectory() ? files(full) : [full];
  });
}

function validateCorpusImage(relative, bytes) {
  const sources = JSON.parse(fs.readFileSync(path.join(root, 'docs/benchmarks/natural-images-v1/sources.json'), 'utf8'));
  const photo = sources.photos.find(photo => relative === `docs/benchmarks/natural-images-v1/${photo.id}.png`);
  assert.ok(photo && photo.license === 'CC0-1.0', `${relative}: unreviewed corpus binary`);
  assert.equal(crypto.createHash('sha256').update(bytes).digest('hex'), photo.normalized_sha256,
    `${relative}: corpus byte identity mismatch`);
  assert.ok(bytes.length >= 33 && bytes.length <= 200000, `${relative}: corpus image size`);
  assert.equal(bytes.subarray(0, 8).toString('hex'), '89504e470d0a1a0a');
  assert.equal(bytes.readUInt32BE(16), 224);
  assert.equal(bytes.readUInt32BE(20), 224);
  assert.equal(bytes[24], 8);
  assert.equal(bytes[25], 2);
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
    // Exact codec distribution bytes are covered by decode.test.mjs hash checks.
    if (path.relative(root, file).split(path.sep).join('/').startsWith('packages/core/vendor/codecs/')) continue;
    // Exact approved libsodium distribution bytes are covered by model-package integrity checks.
    if (path.relative(root, file).split(path.sep).join('/').startsWith('packages/core/vendor/libsodium/')) continue;
    // Exact upstream bytes are covered by persistence.test.mjs SHA-256 gates.
    if (['packages/core/vendor/sqlite/sqlite3.c', 'packages/core/vendor/sqlite/sqlite3.h']
      .includes(path.relative(root, file).split(path.sep).join('/'))) continue;
    if (file.endsWith('.jar')) {
      assert.equal(path.relative(root, file).split(path.sep).join('/'),
        'platforms/android/gradle/wrapper/gradle-wrapper.jar', 'Unexpected binary outside generated directories');
      continue;
    }
    if (file.endsWith('.png')) {
      // Only pinned CC0 corpus bytes are binary; no blanket image exemption.
      validateCorpusImage(path.relative(root, file).split(path.sep).join('/'), fs.readFileSync(file));
      continue;
    }
    const content = fs.readFileSync(file, 'utf8');
    assert.ok(!/^(<<<<<<<|=======|>>>>>>>)( |$)/m.test(content), `${file}: conflict marker`);
    assert.ok(!/[\t ]+\r?$/m.test(content), `${file}: trailing whitespace`);
    assert.ok(content.endsWith('\n'), `${file}: missing final newline`);
  }
});

test('corpus binary hygiene rejects substituted bytes and unlisted images', () => {
  const relative = 'docs/benchmarks/natural-images-v1/114.png';
  const original = fs.readFileSync(path.join(root, relative));
  validateCorpusImage(relative, original);
  const corrupt = Buffer.from(original);
  corrupt[corrupt.length - 1] ^= 1;
  assert.throws(() => validateCorpusImage(relative, corrupt), /identity mismatch/);
  assert.throws(() => validateCorpusImage('docs/benchmarks/natural-images-v1/unlisted.png', original), /unreviewed corpus binary/);
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
