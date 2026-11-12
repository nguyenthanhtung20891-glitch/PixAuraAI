import assert from 'node:assert/strict';
import crypto from 'node:crypto';
import fs from 'node:fs';
import test from 'node:test';
import { phase11Hash } from './helpers/phase-11-hashes.mjs';
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
    assert.equal(phase11Hash(path, change.before_sha256_lf), change.after_sha256_lf, path);
    assert.equal(hash(path), change.after_sha256_lf, path);
  }
  for (const [path, digest] of Object.entries(record.additions)) assert.equal(hash(path), digest, path);
  for (const [path, digest] of Object.entries(record.historical_records)) assert.equal(hash(path), digest, path);
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
