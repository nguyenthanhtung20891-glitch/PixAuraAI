import assert from 'node:assert/strict';
import crypto from 'node:crypto';
import fs from 'node:fs';
import test from 'node:test';
import { phase11Hash, phase11SuccessorHash, phase11PostV2Hash, phase11PostV3Hash, phase11PostV4Hash, phase11PostV5Hash, phase11PostV6Hash, phase11PostV7Hash } from './helpers/phase-11-hashes.mjs';
const read = p => fs.readFileSync(p, 'utf8').replaceAll('\r\n', '\n');
const hash = p => crypto.createHash('sha256').update(read(p)).digest('hex');

test('Phase 11 source successor binds current hardening without rewriting historical evidence', () => {
  const record = JSON.parse(read('docs/reports/phase-11-source-v1.json'));
  const baseline = JSON.parse(read('docs/reports/evidence/phase-10.1-local/product-source-manifest.json'));
  assert.equal(record.base_commit, '874b6645c75cd227fbb974b891ff41991397acc2');
  assert.equal(record.public_abi_changed, false);
  assert.equal(record.schema_changed, false);
  assert.equal(record.gpu_admission_policy_changed, false);
  for (const [path, change] of Object.entries(record.changes)) {
    if (baseline.files[path]) assert.equal(change.before_sha256_lf, baseline.files[path], path);
    assert.equal(phase11Hash(path, change.before_sha256_lf), phase11SuccessorHash(path, change.after_sha256_lf), path);
    assert.equal(hash(path), phase11SuccessorHash(path, change.after_sha256_lf), path);
  }
  for (const [path, digest] of Object.entries(record.additions)) assert.equal(hash(path), phase11SuccessorHash(path, digest), path);
  for (const [path, digest] of Object.entries(record.historical_records)) assert.equal(hash(path), digest, path);
  const successor = JSON.parse(read('docs/reports/phase-11-source-v2.json'));
  assert.equal(successor.predecessor_commit, 'f585c24a7eec7496454d8fd1f2258bdce5c417ee');
  assert.equal(hash(successor.parent_manifest), successor.parent_manifest_sha256_lf);
  assert.equal(successor.public_abi_changed, false);
  assert.equal(successor.schema_changed, false);
  assert.equal(successor.gpu_admission_policy_changed, false);
  for (const [path, change] of Object.entries(successor.changes)) {
    assert.equal(change.before_sha256_lf, record.changes[path]?.after_sha256_lf ?? record.additions[path], path);
    assert.equal(hash(path), phase11PostV2Hash(path, change.after_sha256_lf), path);
  }
  const localization = JSON.parse(read('docs/reports/phase-11-source-v3.json'));
  assert.equal(localization.predecessor_commit, 'ee7f8cefc96f26bb3024d749d3f3bd52742f2a02');
  assert.equal(hash(localization.parent_manifest), localization.parent_manifest_sha256_lf);
  for (const flag of ['public_abi_changed', 'schema_changed', 'gpu_admission_policy_changed']) assert.equal(localization[flag], false);
  const previouslyUnchanged = { 'tests/product-ux-locale.test.mjs': 'b5bbe3c61074366d6ca642688ea59a3adfd2fc06c8e917e7fd96cd1af785a976' };
  assert.deepEqual(localization.unchanged_predecessor_files, previouslyUnchanged);
  for (const [path, change] of Object.entries(localization.changes)) {
    assert.equal(change.before_sha256_lf, successor.changes[path]?.after_sha256_lf ?? record.changes[path]?.after_sha256_lf ?? record.additions[path] ?? previouslyUnchanged[path], path);
    assert.equal(hash(path), phase11PostV3Hash(path, change.after_sha256_lf), path);
  }
  const controller = JSON.parse(read('docs/reports/phase-11-source-v4.json'));
  assert.equal(controller.predecessor_commit, '85a8369365b1efae2d7844337656a607d9e0ca96');
  assert.equal(hash(controller.parent_manifest), controller.parent_manifest_sha256_lf);
  for (const flag of ['public_abi_changed', 'schema_changed', 'gpu_admission_policy_changed']) assert.equal(controller[flag], false);
  for (const [path, change] of Object.entries(controller.changes)) {
    assert.equal(change.before_sha256_lf, localization.changes[path]?.after_sha256_lf ?? successor.changes[path]?.after_sha256_lf ?? record.changes[path]?.after_sha256_lf ?? record.additions[path] ?? controller.unchanged_predecessor_files[path], path);
    assert.equal(hash(path), phase11PostV4Hash(path, change.after_sha256_lf), path);
  }
  const retirement = JSON.parse(read('docs/reports/phase-11-source-v5.json'));
  assert.equal(retirement.predecessor_commit, '828152f8673001c2ae38bda2c489dc2d7fd5251f');
  assert.equal(hash(retirement.parent_manifest), retirement.parent_manifest_sha256_lf);
  for (const flag of ['public_abi_changed', 'schema_changed', 'gpu_admission_policy_changed']) assert.equal(retirement[flag], false);
  for (const [path, change] of Object.entries(retirement.changes)) {
    assert.equal(change.before_sha256_lf, controller.changes[path]?.after_sha256_lf ?? localization.changes[path]?.after_sha256_lf ?? successor.changes[path]?.after_sha256_lf ?? record.changes[path]?.after_sha256_lf ?? record.additions[path], path);
    assert.equal(hash(path), phase11PostV5Hash(path, change.after_sha256_lf), path);
  }
  const loader = JSON.parse(read('docs/reports/phase-11-source-v6.json'));
  assert.equal(loader.predecessor_commit, '550fd9d79fc761362764a39720f11c79af20e146');
  assert.equal(hash(loader.parent_manifest), loader.parent_manifest_sha256_lf);
  for (const flag of ['public_abi_changed', 'schema_changed', 'gpu_admission_policy_changed']) assert.equal(loader[flag], false);
  for (const [path, change] of Object.entries(loader.changes)) {
    assert.equal(change.before_sha256_lf, retirement.changes[path]?.after_sha256_lf ?? controller.changes[path]?.after_sha256_lf ?? localization.changes[path]?.after_sha256_lf ?? successor.changes[path]?.after_sha256_lf ?? record.changes[path]?.after_sha256_lf ?? record.additions[path] ?? baseline.files[path] ?? ({ 'scripts/generate-ios-project.mjs': '8c1ed80ab608ada826757296657006d4704ae79d6fb54be101ac4f10f7c597e5' })[path], path);
    assert.equal(hash(path), phase11PostV6Hash(path, change.after_sha256_lf), path);
  }
  const fixture = JSON.parse(read('docs/reports/phase-11-source-v7.json'));
  assert.equal(fixture.predecessor_commit, '23ea6ff556ef7c7fbf94c59aa97218128da47391');
  assert.equal(hash(fixture.parent_manifest), fixture.parent_manifest_sha256_lf);
  for (const flag of ['public_abi_changed', 'schema_changed', 'gpu_admission_policy_changed']) assert.equal(fixture[flag], false);
  for (const [path, change] of Object.entries(fixture.changes)) {
    assert.equal(change.before_sha256_lf, loader.changes[path]?.after_sha256_lf ?? retirement.changes[path]?.after_sha256_lf ?? controller.changes[path]?.after_sha256_lf ?? localization.changes[path]?.after_sha256_lf ?? successor.changes[path]?.after_sha256_lf ?? record.changes[path]?.after_sha256_lf ?? record.additions[path], path);
    assert.equal(hash(path), phase11PostV7Hash(path, change.after_sha256_lf), path);
  }
  const measurement = JSON.parse(read('docs/reports/phase-11-source-v8.json'));
  assert.equal(measurement.predecessor_commit, '82f847859397e5217f4a68b511b526524a1ab7d6');
  assert.equal(hash(measurement.parent_manifest), measurement.parent_manifest_sha256_lf);
  for (const flag of ['public_abi_changed', 'schema_changed', 'gpu_admission_policy_changed']) assert.equal(measurement[flag], false);
  for (const [path, change] of Object.entries(measurement.changes)) {
    assert.equal(change.before_sha256_lf, fixture.changes[path]?.after_sha256_lf ?? loader.changes[path]?.after_sha256_lf ?? retirement.changes[path]?.after_sha256_lf ?? controller.changes[path]?.after_sha256_lf ?? localization.changes[path]?.after_sha256_lf ?? successor.changes[path]?.after_sha256_lf ?? record.changes[path]?.after_sha256_lf ?? record.additions[path], path);
    assert.equal(hash(path), change.after_sha256_lf, path);
  }

});

test('Darwin sanitizer portability retains Linux leak checks and active error probes', () => {
  const cmake = read('CMakeLists.txt');
  assert.match(cmake, /set\(PIXAURA_ASAN_ENV "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1"\)/);
  assert.match(cmake, /if\(APPLE\)\s+set\(PIXAURA_ASAN_ENV "ASAN_OPTIONS=detect_leaks=0:halt_on_error=1"\)/);
  const verify = read('scripts/verify-sanitizer.cmake');
  assert.match(verify, /CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin"/);
  assert.match(verify, /ERROR: AddressSanitizer: heap-buffer-overflow/);
  assert.match(verify, /runtime error: signed integer overflow/);
  assert.match(verify, /pixaura_get_core_info/);
  assert.match(cmake, /add_test\(NAME phase11_security_test/);
  assert.match(read('packages/core/tests/gesture_lifecycle_test.cpp'), /alignas\(__STDCPP_DEFAULT_NEW_ALIGNMENT__\)/);
});

test('Phase 11 does not turn software success into mandatory hardware or commerce acceptance', () => {
  const active = read('AGENTS.md').split('<!-- phase11-history -->')[0];
  assert.match(active, /Phase 11 AUTHORIZED \/ IN PROGRESS/);
  assert.match(active, /MANDATORY before Phase 11 hardening acceptance/);
  assert.match(active, /Phase 12 NOT STARTED/);
  const plan = read('docs/plans/phase-11-roadmap.md');
  assert.match(plan, /human.*walkthrough/i);
  assert.match(plan, /offline subscription grace requiring|offline grace still proposed/);
  for (const path of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml', 'scripts/check-apple.sh']) {
    assert.ok(read(path).includes('tests/phase-11-hardening.test.mjs'), path);
  }
});


test('hosted iOS job budget preserves the complete package and app suites', () => {
  const ios = read('.github/workflows/native-shells.yml').split('  ios-app:')[1];
  assert.match(ios, /timeout-minutes: 60/);
  assert.match(ios, /run: bash scripts\/check-apple\.sh/);
  assert.match(ios, /run: bash scripts\/check-ios-shell\.sh/);
  assert.match(ios, /if: always\(\)/);
});
