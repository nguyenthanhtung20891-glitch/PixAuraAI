import assert from 'node:assert/strict';
import fs from 'node:fs';
import crypto from 'node:crypto';
const read = p => fs.readFileSync(p, 'utf8').replaceAll('\r\n', '\n');
const hash = s => crypto.createHash('sha256').update(s).digest('hex');
// CR18-01 authorizes additive Schema4, not alteration of the historical schemas
// or silent migration by the frozen v1 application service.
export function assertLegacySchema3Boundary() {
  const parent = JSON.parse(read('docs/reports/phase-11-source-v19.json'));
  for (const path of ['packages/core/src/storage_schema.hpp', 'packages/core/src/storage_schema_v2.hpp', 'packages/core/src/storage_schema_v3.hpp']) {
    assert.equal(hash(read(path)), parent.protected_native_fingerprints[path], path);
  }
  assert.match(read('DECISIONS.md'), /CR18-01/);
  assert.match(read('packages/core/src/storage.cpp'), /Repository::Repository\(std::string_view root\):Repository\(root,3\)/);
  assert.match(read('packages/core/src/editor_api.cpp'), /make_unique<storage::Repository>\(path\);require\(s->repository->version\(\)==3,4\)/);
  assert.match(read('packages/core/src/storage_schema_v4.hpp'), /Additive to EXACT schema3 objects/);
  assert.doesNotMatch(read('packages/core/src/storage_schema_v4.hpp'), /DROP TABLE|ALTER TABLE/);
}
// Historical fragment hashes are retained; an approved versioned adapter must
// bind an explicit predecessor and the exact complete successor body.
export function phase11HumanApplyHash(path, predecessor) {
  const record = JSON.parse(read('docs/reports/phase-11-source-v20.json'));
  const change = record.fragment_changes?.[path + '#human-apply'];
  if (!change) return predecessor;
  assert.equal(change.before_sha256_lf, predecessor, path + ' legacy HUMAN Apply predecessor');
  assert.equal(hash(change.before_body_lf), predecessor, path + ' complete historical HUMAN Apply body');
  assert.equal(change.approval, 'CR18-02');
  if (path.endsWith('.swift')) {
    const marker = '        var approval: AgentApproval?';
    const historical = change.before_body_lf;
    const boundaries = JSON.parse(read('docs/reports/phase-8-step-4-v1.json')).human_apply_bodies[path];
    const source = read(path); const current = source.slice(source.indexOf(boundaries.start), source.indexOf(boundaries.end));
    assert.ok(historical.includes(marker) && current.includes(marker));
    assert.equal(current.slice(current.indexOf(marker)), historical.slice(historical.indexOf(marker)), path + ' v1 approval/execution tail unchanged');
    assert.match(current.slice(0, current.indexOf(marker)), /versioned\.applyAdvisoryReceiptFromHuman\(native\)/);
  } else if (path.endsWith('.kt')) {
    const marker = '        val approval = gate.withLock {';
    const historical = change.before_body_lf;
    const boundaries = JSON.parse(read('docs/reports/phase-8-step-4-v1.json')).human_apply_bodies[path];
    const source = read(path); const current = source.slice(source.indexOf(boundaries.start), source.indexOf(boundaries.end));
    assert.ok(historical.includes(marker) && current.includes(marker));
    assert.equal(current.slice(current.indexOf(marker)), historical.slice(historical.indexOf(marker)), path + ' v1 approval/execution tail unchanged');
    assert.match(current.slice(0, current.indexOf(marker)), /versioned\.applyAdvisoryReceiptFromHuman\(requireNotNull\(review\.proposal\.v2Advice\)\)/);
  }

  return change.after_sha256_lf;
}
