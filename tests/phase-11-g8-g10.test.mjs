import test from 'node:test';
import {phase11PostV13Hash, phase11PostV14Hash, phase11PostV15Hash, phase11PostV16Hash} from './helpers/phase-11-hashes.mjs';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import crypto from 'node:crypto';
const read = p => fs.readFileSync(p, 'utf8').replaceAll('\r\n', '\n');
const hash = p => crypto.createHash('sha256').update(read(p)).digest('hex');
test('G8/G10 source successor preserves native ABI/kernels and all prior records', () => {
  const m = JSON.parse(read('docs/reports/phase-11-source-v13.json'));
  assert.equal(m.predecessor_commit, 'eb61735be29eb365db76fa57b74170c6656c9efd');
  assert.equal(hash(m.parent_manifest), m.parent_sha256_lf);
  for (const flag of ['public_abi_changed', 'project_schema_changed', 'certified_numerical_kernels_changed', 'gpu_admission_policy_changed', 'application_quota_enforcement_enabled', 'full_12mp_pipeline_claimed', 'store_product_ids_or_credentials_invented']) assert.equal(m[flag], false, flag);
  for (const [p, c] of Object.entries(m.changes)) { assert.match(c.before_sha256_lf, /^[a-f0-9]{64}$/); assert.equal(hash(p), phase11PostV13Hash(p, c.after_sha256_lf), p); }
  for (const [p, digest] of Object.entries(m.additions)) assert.equal(hash(p), phase11PostV13Hash(p, digest), p);
  for (const [p, digest] of Object.entries(m.protected_native_fingerprints)) assert.equal(hash(p), phase11PostV13Hash(p, digest), p);
});
test('Owner-approved policy fixes quotas while recovery/native authority and future-phase boundaries stay explicit', () => {
  const p = JSON.parse(read('docs/reports/phase-11-commercial-approval-v1.json'));
  assert.equal(p.trial_calendar_days, 14); assert.equal(p.free_distinct_sources_per_utc_day, 5);
  assert.equal(p.free_successful_usable_ai_proposals_per_utc_day, 3); assert.equal(p.subscription_offline_grace_days, 7);
  assert.equal(p.production_store_ids_or_credentials_supplied, false);
  const adr = read('docs/adr/0024-entitlement-policy-proposal.md');
  for (const term of ['Status: Accepted', 'first successful committed edit', 'UTC00:00', 'monotonic', 'Recovery viewing', 'ephemeral', 'Schema3']) assert.ok(adr.includes(term) || read('docs/contracts/commercial-admission-v1-proposal.md').includes(term), term);
  assert.ok(read('ROADMAP.md').includes('PLANNING ONLY / NOT IMPLEMENTED / NOT AUTHORIZED'));
  assert.ok(read('docs/contracts/commercial-admission-v1-proposal.md').includes('NOT deployed'));
});
test('store SDK and verifier cannot silently become shipping telemetry or a purchase grant', () => {
  const app = read('platforms/android/app/build.gradle.kts'); const sdk = read('platforms/android/store-integration/build.gradle.kts');
  assert.doesNotMatch(app, /implementation\(project\(.*store-integration/);
  assert.match(sdk, /compileOnly\("com.android.billingclient:billing:9\.1\.0"\)/);
  assert.match(sdk, /isTransitive = false/);
  assert.doesNotMatch(read('platforms/android/app/src/main/AndroidManifest.xml'), /uses-permission/);
  assert.ok(app.includes('androidTestImplementation("com.android.billingclient:billing:9.1.0")'));
  assert.ok(read('platforms/ios/PixAuraAI/ShellModel.swift').includes('case .verified(let transaction)'));
  assert.ok(read('packages/commerce/quota.hpp').includes('friend class NativeCommercialOwner'));
});

test('Strict compiler successor binds every corrected file without mutating v13 evidence', () => {
  const m = JSON.parse(read('docs/reports/phase-11-source-v14.json'));
  assert.equal(m.predecessor_commit, '24f7aeb5424bc4f8a3a80ce8cf96a5710f27fbce');
  assert.equal(hash(m.parent_manifest), m.parent_sha256_lf);
  for (const [p, c] of Object.entries(m.changes)) assert.equal(hash(p), phase11PostV14Hash(p, c.after_sha256_lf), p);
  for (const [p, h] of Object.entries(m.additions)) assert.equal(hash(p), phase11PostV14Hash(p, h), p);
});

test('Tile compiler successor preserves all prior manifests and exact corrected sources', () => {
  const m = JSON.parse(read('docs/reports/phase-11-source-v15.json'));
  assert.equal(m.predecessor_commit, '27a875a44a395a59e7e8599e1c001fa67a06ce91');
  assert.equal(hash(m.parent_manifest), m.parent_sha256_lf);
  for (const [p, c] of Object.entries(m.changes)) assert.equal(hash(p), phase11PostV15Hash(p, c.after_sha256_lf), p);
  for (const [p, h] of Object.entries(m.additions)) assert.equal(hash(p), phase11PostV15Hash(p, h), p);
});

test('Native exit and Android first-exception diagnostics preserve failure authority', () => {
  const m = JSON.parse(read('docs/reports/phase-11-source-v16.json'));
  assert.equal(hash(m.parent_manifest), m.parent_sha256_lf);
  for (const [p, c] of Object.entries(m.changes)) assert.equal(hash(p), phase11PostV16Hash(p, c.after_sha256_lf), p);
  for (const [p, h] of Object.entries(m.additions)) assert.equal(hash(p), phase11PostV16Hash(p, h), p);
  assert.match(read('packages/core/tests/run_native_test.py'), /sys.exit\(0 if result.returncode == 0 else 1\)/);
  assert.match(read('scripts/check-android-emulator.sh'), /AndroidRuntime:E TestRunner:I/);
});

test('Explicit successful native exit and exact fake-ADB diagnostic arguments are source-bound', () => {
  const m = JSON.parse(read('docs/reports/phase-11-source-v17.json'));
  assert.equal(hash(m.parent_manifest), m.parent_sha256_lf);
  for (const [p, c] of Object.entries(m.changes)) assert.equal(hash(p), c.after_sha256_lf, p);
  for (const [p, h] of Object.entries(m.additions)) assert.equal(hash(p), h, p);
  assert.match(read('packages/core/tests/tiled_cpu_test.cpp'), /return 0;\s*}\s*$/);
  assert.match(read('tests/android-emulator.test.mjs'), /logcat -v brief -s PixAuraEvaluation:I AndroidRuntime:E TestRunner:I/);
});
