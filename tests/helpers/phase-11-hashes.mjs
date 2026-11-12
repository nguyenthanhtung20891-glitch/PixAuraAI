import fs from 'node:fs';
import assert from 'node:assert/strict';

// Additive predecessor check: original historical records remain immutable.
// This changes expected fingerprints only; behavioral tests read actual source.
export function phase11Hash(path, predecessor) {
  const record = JSON.parse(fs.readFileSync('docs/reports/phase-11-source-v1.json', 'utf8'));
  const change = record.changes[path];
  if (!change) return predecessor;
  assert.equal(change.before_sha256_lf, predecessor, path + ' Phase 11 predecessor');
  return change.after_sha256_lf;
}
