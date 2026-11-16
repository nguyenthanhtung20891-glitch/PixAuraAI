import test from 'node:test';
import {phase11PostV18Hash} from './helpers/phase-11-hashes.mjs';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import crypto from 'node:crypto';
const read = p => fs.readFileSync(p, 'utf8').replaceAll('\r\n', '\n');
const hash = p => crypto.createHash('sha256').update(read(p)).digest('hex');
test('v18 preserves immutable native contracts and binds additive integration evidence', () => {
  const m = JSON.parse(read('docs/reports/phase-11-source-v18.json'));
  assert.equal(m.predecessor_commit, '4cf0642119ea39149615b93588068c56a85d48ff');
  assert.equal(hash(m.parent_manifest), m.parent_sha256_lf);
  for (const flag of ['public_abi_changed','project_schema_changed','certified_numerical_kernels_changed','gpu_admission_policy_changed','application_quota_enforcement_enabled','shipping_12mp_editor_enabled','store_product_ids_or_credentials_invented']) assert.equal(m[flag], false, flag);
  for (const [p,c] of Object.entries(m.changes)) assert.equal(hash(p), phase11PostV18Hash(p,c.after_sha256_lf), p);
  for (const [p,h] of Object.entries(m.additions)) assert.equal(hash(p), phase11PostV18Hash(p,h), p);
  for (const [p,h] of Object.entries(m.protected_native_fingerprints)) assert.equal(hash(p), phase11PostV18Hash(p,h), p);
  assert.equal(Object.keys(m.protected_native_fingerprints).length,113);
  const compatibility=read('docs/contracts/phase-11-v18-compatibility-requests.md');
  for (const term of ['CR18-01','CR18-02','Schema3']) assert.ok(compatibility.includes(term),term);
  assert.doesNotMatch(read('platforms/android/app/build.gradle.kts'), /implementation\(project\(.*store-integration/);
});

test('codec layout correction preserves the failed predecessor and all frozen contracts', () => {
  const m=JSON.parse(read('docs/reports/phase-11-source-v19.json'));
  assert.equal(m.predecessor_commit,'b47d9b61639a7253bb89fa36e76431eca15965d2');
  assert.equal(hash(m.parent_manifest),m.parent_sha256_lf);
  assert.equal(m.assertions_weakened,false);
  assert.equal(m.public_abi_changed,false);
  assert.equal(m.project_schema_changed,false);
  for(const [p,c] of Object.entries(m.changes)) assert.equal(hash(p),c.after_sha256_lf,p);
  for(const [p,h] of Object.entries(m.additions)) assert.equal(hash(p),h,p);
  for(const [p,h] of Object.entries(m.protected_native_fingerprints)) assert.equal(hash(p),h,p);
});
