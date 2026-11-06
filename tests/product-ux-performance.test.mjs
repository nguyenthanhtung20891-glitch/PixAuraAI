import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import test from 'node:test';

const read = (name) => fs.readFileSync(name, 'utf8');
const walk = (dir) => fs.readdirSync(dir, { withFileTypes: true }).flatMap((entry) => {
  const child = path.join(dir, entry.name);
  return entry.isDirectory() ? walk(child) : [child];
});

test('hosted export samples stay on the existing fixture', () => {
  const native = read('packages/core/tests/export_test.cpp');
  assert.match(native, /pixaura-hosted-v1 fixture=decode_png width=2 height=3 warmup=8 samples=7/);
  assert.match(native, /png-median=%lld jpeg-median=%lld/);
  assert.match(native, /export hosted repeat-us/);
  assert.match(native, /encode\(first,first\.bind\(\),32\)\.code==11/);
  assert.doesNotMatch(native, /process-RSS|rss_ceiling|worker pool/i);
});

test('production surfaces do not log private content', () => {
  const roots = ['platforms/android/app/src/main', 'platforms/ios/PixAuraAI', 'packages/core/src'];
  const forbidden = [/android\.util\.Log/, /\bNSLog\b/, /\bos_log\b/, /content:\/\//, /\bprintln\(/];
  for (const file of roots.flatMap(walk)) {
    const text = read(file);
    for (const pattern of forbidden) assert.doesNotMatch(text, pattern, file);
  }
  const storage = read('packages/core/src/storage_files.cpp');
  assert.match(storage, /PIXAURA_STORAGE_TESTING/);
  assert.match(storage, /storage filesystem failure line=%d status=%d os=%lu/);
  assert.doesNotMatch(storage, /fprintf\([^)]*%s/);
  const metal = read('packages/core/src/inference_metal.hpp');
  assert.match(metal, /PIXAURA_CERTIFICATION_TESTING/);
  assert.match(metal, /pixels=%llu total_us=%lld/);
});

test('Step 5 records the observed Step 4 closure and adds no ceiling', () => {
  const agents = read('AGENTS.md');
  const block = agents.split('<!-- phase10-step5-history -->')[0];
  assert.match(block, /Step 4 COMPLETE \/ FULL PASS/);
  assert.match(block, /9342116c478c0165fe6db4fbc4a440f17761228a/);
  assert.match(block, /42139462323/);
  assert.match(block, /42139462394/);
  assert.match(block, /Step 5 CLOSURE CERTIFICATION PENDING/);
  assert.match(block, /e57f827604b3367c983bd756e6dedf66d5644ef1/);
  assert.match(block, /42164977378/);
  assert.match(block, /42164977392/);
  assert.match(block, /Step 6 NOT STARTED/);
  assert.match(block, /No new process-RSS ceiling/);
  assert.match(agents.split('<!-- phase10-step4-history -->')[0], /Step 4 CLOSURE CERTIFICATION PENDING/);
  const roadmap = read('docs/plans/phase-10-roadmap.md');
  assert.match(roadmap, /\| 5 \| Performance, Resource and Privacy Hardening \| CLOSURE CERTIFICATION PENDING \|/);
  assert.match(roadmap, /\| 4 \| Error Recovery and Lifecycle \| CLOSURE CERTIFICATION PENDING \|/);
  const receipt = JSON.parse(read('docs/reports/evidence/phase-10-step-4-closure-certification.json'));
  assert.equal(receipt.label, 'closure');
  assert.equal(receipt.sha, '9342116c478c0165fe6db4fbc4a440f17761228a');
  assert.equal(receipt.step4, 'COMPLETE_FULL_PASS');
  assert.equal(receipt.runs[0].jobs.length, 7);
  assert.equal(receipt.runs[1].jobs.length, 3);
  const report = JSON.parse(read('docs/reports/phase-10-step-5-v1.json'));
  assert.equal(report.status, 'CLOSURE_CERTIFICATION_PENDING');
  assert.equal(report.implementation_sha, 'e57f827604b3367c983bd756e6dedf66d5644ef1');
  const implementation = JSON.parse(read('docs/reports/evidence/phase-10-step-5-implementation-certification.json'));
  assert.equal(implementation.label, 'implementation');
  assert.equal(implementation.runs[0].jobs.length, 7);
  assert.equal(implementation.runs[1].jobs.length, 3);
  assert.equal(report.new_resource_ceiling, false);
  for (const workflow of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml']) {
    assert.match(read(workflow), /tests\/product-ux-performance\.test\.mjs/);
  }
});
