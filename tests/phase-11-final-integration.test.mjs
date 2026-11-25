import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import crypto from 'node:crypto';
const read = p => fs.readFileSync(p, 'utf8').replaceAll('\r\n', '\n');
const hash = p => crypto.createHash('sha256').update(read(p)).digest('hex');
test('versioned integration retains every protected predecessor except approved storage extensions', () => {
  const m = JSON.parse(read('docs/reports/phase-11-source-v20.json'));
  const predecessor = JSON.parse(read(m.parent_manifest));
  assert.equal(m.predecessor_commit, '2f823a55c36271b19847fade7134aa9b460b715d');
  assert.equal(m.parent_sha256_lf, hash(m.parent_manifest));
  assert.equal(m.public_v1_abi_changed, false);
  assert.equal(m.schema3_reinterpreted, false);
  assert.equal(m.schema4_opt_in_added, true);
  assert.equal(m.certified_numerical_kernels_changed, false);
  assert.equal(m.gpu_admission_policy_changed, false);
  assert.equal(m.phase_11_closed, false);
  assert.deepEqual(m.approved_protected_changes, ['packages/core/src/storage.cpp', 'packages/core/src/storage.hpp']);
  assert.equal(Object.keys(predecessor.protected_native_fingerprints).length, 113);
  for (const [path, digest] of Object.entries(predecessor.protected_native_fingerprints)) {
    if (m.approved_protected_changes.includes(path)) assert.equal(m.changes[path].before_sha256_lf, digest, path);
    else assert.equal(hash(path), digest, path);
    assert.equal(hash(path), m.protected_native_fingerprints[path], path);
  }
  for (const [path, change] of Object.entries(m.changes)) assert.equal(hash(path), change.after_sha256_lf, path);
  for (const [path, digest] of Object.entries(m.additions)) assert.equal(hash(path), digest, path);
  assert.ok(!('platforms/ios/PixAuraAI.xcodeproj/project.pbxproj' in m.changes));
  assert.doesNotMatch(read('platforms/android/app/build.gradle.kts'), /implementation\(project\(.*store-integration/);
});
